/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/ISel.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/IR/Analysis.h"
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/Optional.h"

#define DEBUG_TYPE "hbc-backend-isel"

using namespace hermes;
using namespace hbc;

using llvh::dbgs;
using llvh::Optional;

#define INCLUDE_HBC_INSTRS

STATISTIC(NumJumpPass, "Number of passes to resolve all jump targets");
STATISTIC(
    NumUncachedNodes,
    "Number of put/get property instructions with property caching disabled");
STATISTIC(
    NumCachedNodes,
    "Number of put/get property instructions with property caching enabled");
STATISTIC(
    NumCacheSlots,
    "Number of cache slots allocated for all put/get property instructions");

/// Given a list of basic blocks \p blocks linearized into the order they will
/// be generated, \return the set of those basic blocks containing backwards
/// successors. Note a jump from a block to itself is necessarily backwards.
static DenseSet<const BasicBlock *> basicBlocksWithBackwardSuccessors(
    ArrayRef<BasicBlock *> blocks) {
  llvh::SmallDenseSet<const BasicBlock *, 16> seen;
  DenseSet<const BasicBlock *> result;
  for (const BasicBlock *BB : blocks) {
    seen.insert(BB);
    // Add all successors that are in our seen set.
    for (const auto *successor : successors(BB)) {
      if (seen.count(successor))
        result.insert(successor);
    }
  }
  return result;
}

void HVMRegisterAllocator::handleInstruction(Instruction *I) {
  if (auto *CI = llvh::dyn_cast<BaseCallInst>(I)) {
    return allocateCallInst(CI);
  }
}

bool HVMRegisterAllocator::hasTargetSpecificLowering(Instruction *I) {
  return llvh::isa<BaseCallInst>(I);
}

void HVMRegisterAllocator::allocateCallInst(BaseCallInst *I) {
  allocateParameterCount(I->getNumArguments() + CALL_EXTRA_REGISTERS);
}

unsigned HBCISel::encodeValue(Value *value) {
  if (llvh::isa<Instruction>(value)) {
    return RA_.getRegister(value).getIndex();
  } else if (auto *var = llvh::dyn_cast<Variable>(value)) {
    return var->getIndexInVariableList();
  } else {
    llvm_unreachable("Do not support other value types");
  }
}

void HBCISel::registerLongJump(offset_t loc, BasicBlock *target) {
  relocations_.push_back(
      {loc, Relocation::RelocationType::LongJumpType, target});
}

void HBCISel::registerSwitchImm(offset_t loc, SwitchImmInst *inst) {
  relocations_.push_back(
      {loc, Relocation::RelocationType::JumpTableDispatch, inst});
}

void HBCISel::resolveRelocations() {
  bool changed;
  do {
    int totalShift = 0;
    changed = false;
    for (auto &relocation : relocations_) {
      auto loc = relocation.loc;
      auto *pointer = relocation.pointer;
      auto type = relocation.type;
      loc -= totalShift;
      relocation.loc = loc;
      switch (type) {
        case Relocation::LongJumpType: {
          int targetLoc = basicBlockMap_[cast<BasicBlock>(pointer)].first;
          int jumpOffset = targetLoc - loc;
          if (-128 <= jumpOffset && jumpOffset < 128) {
            // The jump offset can fit into one byte.
            totalShift += 3;
            BCFGen_->shrinkJump(loc + 1);
            BCFGen_->updateJumpTarget(loc + 1, jumpOffset, 1);
            relocation.type = Relocation::JumpType;
            changed = true;
          } else {
            BCFGen_->updateJumpTarget(loc + 1, jumpOffset, 4);
          }
          break;
        }
        case Relocation::BasicBlockType:
          basicBlockMap_[cast<BasicBlock>(pointer)].first = loc;
          break;
        case Relocation::JumpType: {
          int targetLoc = basicBlockMap_[cast<BasicBlock>(pointer)].first;
          int jumpOffset = targetLoc - loc;
          BCFGen_->updateJumpTarget(loc + 1, jumpOffset, 1);
          break;
        }
        case Relocation::CatchType:
          catchInfoMap_[cast<CatchInst>(pointer)].catchLocation = loc;
          break;
        case Relocation::DebugInfo:
          // Nothing, just keep track of the location.
          break;
        case Relocation::JumpTableDispatch:
          auto &switchImmInfo = switchImmInfo_[cast<SwitchImmInst>(pointer)];
          // update default target jmp
          BasicBlock *defaultBlock = switchImmInfo.defaultTarget;
          int defaultOffset = basicBlockMap_[defaultBlock].first - loc;
          BCFGen_->updateJumpTarget(loc + 1 + 1 + 4, defaultOffset, 4);
          switchImmInfo_[cast<SwitchImmInst>(pointer)].offset = loc;
          break;
      }

      NumJumpPass++;
    }
  } while (changed);
}

void HBCISel::resolveExceptionHandlers() {
  if (catchInfoMap_.empty()) {
    // No exception handling, no need to do anything.
    return;
  }

  BasicBlockInfoMap bbMap;
  for (auto it : basicBlockMap_) {
    bbMap[it.first] =
        std::make_pair(it.second.first, basicBlockMap_[it.second.second].first);
  }
  auto exceptionEntries = generateExceptionHandlers(catchInfoMap_, bbMap, F_);
  for (auto entry : exceptionEntries) {
    BCFGen_->addExceptionHandler(
        HBCExceptionHandlerInfo{entry.start, entry.end, entry.target});
  }
}

void HBCISel::generateJumpTable() {
  using SwitchInfoEntry =
      llvh::DenseMap<SwitchImmInst *, SwitchImmInfo>::iterator::value_type;

  if (switchImmInfo_.empty())
    return;

  std::vector<uint32_t> res{};

  // Sort the jump table entries so iteration order is deterministic.
  llvh::SmallVector<SwitchInfoEntry, 1> infoVector{
      switchImmInfo_.begin(), switchImmInfo_.end()};
  std::sort(
      infoVector.begin(),
      infoVector.end(),
      [](SwitchInfoEntry &a, SwitchInfoEntry &b) {
        return a.second.offset < b.second.offset;
      });

  // Fix up all SwitchImm instructions with correct offset.
  for (auto &tuple : infoVector) {
    auto entry = tuple.second;
    uint32_t startOfTable = res.size();
    for (uint32_t jmpIdx = 0; jmpIdx < entry.table.size(); jmpIdx++) {
      res.push_back(basicBlockMap_[entry.table[jmpIdx]].first - entry.offset);
    }

    BCFGen_->updateJumpTableOffset(
        // Offset is located two bytes from begining of instruction.
        entry.offset + 1 + 1,
        startOfTable,
        entry.offset);
  }

  BCFGen_->setJumpTable(std::move(res));
}

bool HBCISel::getDebugSourceLocation(
    SourceErrorManager &manager,
    SMLoc loc,
    DebugSourceLocation *out) {
  SourceErrorManager::SourceCoords coords{};
  if (!manager.findBufferLineAndLoc(loc, coords)) {
    return false;
  }

  auto ids = obtainFileAndSourceMapId(manager, coords.bufId);

  out->line = coords.line;
  out->column = coords.col;
  out->filenameId = ids.filenameId;
  out->sourceMappingUrlId = ids.sourceMappingUrlId;

  return true;
}

inline FileAndSourceMapId HBCISel::obtainFileAndSourceMapId(
    SourceErrorManager &sm,
    unsigned bufId) {
  if (LLVM_LIKELY(
          lastFoundFileSourceMapId_ &&
          lastFoundFileSourceMapId_->first == bufId)) {
    return lastFoundFileSourceMapId_->second;
  }

  auto it = fileAndSourceMapIdCache_.find(bufId);
  if (LLVM_LIKELY(it != fileAndSourceMapIdCache_.end())) {
    lastFoundFileSourceMapId_ = &*it;
    return it->second;
  }

  llvh::StringRef filename = sm.getSourceUrl(bufId);

  uint32_t currentFilenameId = BCFGen_->addFilename(filename);
  uint32_t currentSourceMappingUrlId;

  llvh::StringRef sourceMappingUrl{};
  // Only fetch the source map URL if we are not stripping it and if we are
  // generating full debug info.
  if (!bytecodeGenerationOptions_.stripSourceMappingURL &&
      F_->getContext().getDebugInfoSetting() >= DebugInfoSetting::ALL) {
    sourceMappingUrl = sm.getSourceMappingUrl(bufId);
  }

  // Lazily compiled functions ask to strip the source mapping URL because
  // it was already encoded in the top level module, and it could be a 1MB+
  // data url that we don't want to duplicate once per function.
  if (sourceMappingUrl.empty()) {
    currentSourceMappingUrlId = facebook::hermes::debugger::kInvalidBreakpoint;
  } else {
    // NOTE: this is potentially a very expensive operation, since the source
    // mapping URL could be many megabytes (when it is a data URL). Looking it
    // up in the hash table potentially requires scanning it three times.
    currentSourceMappingUrlId = BCFGen_->addFilename(
        F_->getParent()->getContext().getIdentifier(sourceMappingUrl).str());
  }

  it = fileAndSourceMapIdCache_
           .try_emplace(
               bufId,
               FileAndSourceMapId{currentFilenameId, currentSourceMappingUrlId})
           .first;
  lastFoundFileSourceMapId_ = &*it;
  return it->second;
}

void HBCISel::addDebugSourceLocationInfo(SourceMapGenerator *outSourceMap) {
  bool needDebugStatementNo =
      F_->getContext().getDebugInfoSetting() == DebugInfoSetting::ALL ||
      F_->getContext().getDebugInfoSetting() == DebugInfoSetting::SOURCE_MAP;
  auto &manager = F_->getContext().getSourceErrorManager();
  IRBuilder builder(F_);

  DebugSourceLocation info{};

  bool hasDebugInfo = false;
  for (auto &reloc : relocations_) {
    if (reloc.type != Relocation::DebugInfo)
      continue;
    hasDebugInfo = true;
    auto *inst = cast<Instruction>(reloc.pointer);

    assert(inst->hasLocation() && "Missing location");
    auto location = inst->getLocation();

    if (LLVM_UNLIKELY(!getDebugSourceLocation(manager, location, &info))) {
      hermes_fatal("Unable to get source location");
    }
    info.address = reloc.loc;
    info.statement = needDebugStatementNo ? inst->getStatementIndex() : 0;
    BCFGen_->addDebugSourceLocation(info);
  }

  // If there's no debug info, don't set the function location.
  // This avoids polluting the string table with source file names.
  if (hasDebugInfo) {
    getDebugSourceLocation(manager, F_->getSourceRange().Start, &info);
    info.address = 0;
    info.statement = 0;
    BCFGen_->setSourceLocation(info);
  }
}

void HBCISel::addDebugLexicalInfo() {
  // Only emit if debug info is enabled.
  if (F_->getContext().getDebugInfoSetting() != DebugInfoSetting::ALL)
    return;

  // Set the lexical parent by finding the function in which a closure is
  // created with F_, if any.
  for (auto *U : F_->getUsers()) {
    if (llvh::isa<BaseCreateLexicalChildInst>(U)) {
      BCFGen_->setLexicalParentID(BCFGen_->getFunctionID(U->getFunction()));
      break;
    }
  }

  std::vector<Identifier> names;
  for (const Variable *var : F_->getFunctionScope()->getVariables())
    names.push_back(var->getName());
  BCFGen_->setDebugVariableNames(std::move(names));
}

void HBCISel::populatePropertyCachingInfo() {
  BCFGen_->setHighestReadCacheIndex(lastPropertyReadCacheIndex_);
  BCFGen_->setHighestWriteCacheIndex(lastPropertyWriteCacheIndex_);
}

void HBCISel::generateDirectEvalInst(DirectEvalInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto evalText = encodeValue(Inst->getEvalText());
  BCFGen_->emitDirectEval(dst, evalText, Inst->getStrictCaller());
}

void HBCISel::generateDeclareGlobalVarInst(
    DeclareGlobalVarInst *inst,
    BasicBlock *next) {
  BCFGen_->emitDeclareGlobalVar(BCFGen_->getIdentifierID(inst->getName()));
}

void HBCISel::generateAddEmptyStringInst(
    AddEmptyStringInst *Inst,
    BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  BCFGen_->emitAddEmptyString(dst, src);
}

void HBCISel::generateAsNumberInst(AsNumberInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  BCFGen_->emitToNumber(dst, src);
}

void HBCISel::generateAsNumericInst(AsNumericInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  BCFGen_->emitToNumeric(dst, src);
}

void HBCISel::generateAsInt32Inst(AsInt32Inst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  BCFGen_->emitToInt32(dst, src);
}

void HBCISel::emitMovIfNeeded(param_t dest, param_t src) {
  if (dest == src)
    return;
  if (dest <= UINT8_MAX && src <= UINT8_MAX) {
    BCFGen_->emitMov(dest, src);
  } else {
    BCFGen_->emitMovLong(dest, src);
  }
}

void HBCISel::emitUnreachableIfDebug() {
#ifndef NDEBUG
  BCFGen_->emitUnreachable();
#endif
}

void HBCISel::verifyCall(BaseCallInst *Inst) {
#ifndef NDEBUG
  const auto lastArgReg = RA_.getLastRegister().getIndex() -
      HVMRegisterAllocator::CALL_EXTRA_REGISTERS;

  const bool isBuiltin = llvh::isa<CallBuiltinInst>(Inst);
  const bool isCallN = llvh::isa<HBCCallNInst>(Inst);

  for (unsigned i = 0, max = Inst->getNumArguments(); i < max; i++) {
    Value *argument = Inst->getArgument(i);
    // The first argument (thisArg) of CallBuiltin LiteralUndefined.
    if (isBuiltin && i == 0) {
      assert(
          llvh::isa<LiteralUndefined>(argument) && !RA_.isAllocated(argument) &&
          "Register for 'this' argument is misallocated");
    } else if (isCallN) {
      // CallN may take arguments from anywhere except for the last N registers
      // of the frame, because the bytecode instruction overwrites those
      // registers. Note that <= is correct because lastArgReg is the index of
      // the last register, not the count of registers.
      assert(
          llvh::isa<Instruction>(argument) &&
          RA_.getRegister(argument).getIndex() <= lastArgReg - max);
    } else {
      // Calls require that the arguments be at the end of the frame, in reverse
      // order.
      assert(
          llvh::isa<Instruction>(argument) &&
          RA_.getRegister(argument).getIndex() == lastArgReg - i &&
          "Register is misallocated");
    }
  }
#endif
}

void HBCISel::generateLoadStackInst(LoadStackInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  emitMovIfNeeded(dst, src);
}
void HBCISel::generateMovInst(MovInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  emitMovIfNeeded(dst, src);
}

void HBCISel::generateImplicitMovInst(ImplicitMovInst *Inst, BasicBlock *next) {
  // ImplicitMovs produce no bytecode, they only express that a subsequent
  // instruction will perform the equivalent of a 'Mov'.
}

void HBCISel::generateUnaryOperatorInst(
    UnaryOperatorInst *Inst,
    BasicBlock *next) {
  auto opReg = encodeValue(Inst->getSingleOperand());
  auto resReg = encodeValue(Inst);

  switch (Inst->getKind()) {
    case ValueKind::UnaryTypeofInstKind: { // typeof
      BCFGen_->emitTypeOf(resReg, opReg);
      break;
    }
    case ValueKind::UnaryMinusInstKind: { // -
      BCFGen_->emitNegate(resReg, opReg);
      break;
    }
    case ValueKind::UnaryTildeInstKind: { // ~
      BCFGen_->emitBitNot(resReg, opReg);
      break;
    }
    case ValueKind::UnaryBangInstKind: { // !
      BCFGen_->emitNot(resReg, opReg);
      break;
    }
    case ValueKind::UnaryVoidInstKind: { // Void operator.
      BCFGen_->emitLoadConstUndefined(resReg);
      break;
    }
    case ValueKind::UnaryIncInstKind: { // ++
      BCFGen_->emitInc(resReg, opReg);
      break;
    }
    case ValueKind::UnaryDecInstKind: { // --
      BCFGen_->emitDec(resReg, opReg);
      break;
    }
    default:
      llvm_unreachable("Can't handle this operation");
      break;
  }
}
void HBCISel::generateLoadFrameInst(
    hermes::LoadFrameInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  Variable *var = Inst->getLoadVariable();
  auto envReg = encodeValue(Inst->getScope());
  auto varIdx = encodeValue(var);
  if (varIdx <= UINT8_MAX) {
    BCFGen_->emitLoadFromEnvironment(dstReg, envReg, varIdx);
  } else {
    BCFGen_->emitLoadFromEnvironmentL(dstReg, envReg, varIdx);
  }
}
void HBCISel::generatePhiInst(PhiInst *Inst, BasicBlock *next) {
  // PhiInst has been translated into a sequence of MOVs in RegAlloc
  // Nothing to do here.
}
void HBCISel::generateBinaryOperatorInst(
    BinaryOperatorInst *Inst,
    BasicBlock *next) {
  auto left = encodeValue(Inst->getLeftHandSide());
  auto right = encodeValue(Inst->getRightHandSide());
  auto res = encodeValue(Inst);

  bool isBothNumber = Inst->getLeftHandSide()->getType().isNumberType() &&
      Inst->getRightHandSide()->getType().isNumberType();

  switch (Inst->getKind()) {
    case ValueKind::BinaryEqualInstKind: // ==
      // TODO: optimize the case for null check.
      BCFGen_->emitEq(res, left, right);
      break;
    case ValueKind::BinaryNotEqualInstKind: // !=
      // TODO: optimize the case for null check.
      BCFGen_->emitNeq(res, left, right);
      break;
    case ValueKind::BinaryStrictlyEqualInstKind: // ===
      BCFGen_->emitStrictEq(res, left, right);
      break;
    case ValueKind::BinaryStrictlyNotEqualInstKind: // !===
      BCFGen_->emitStrictNeq(res, left, right);
      break;
    case ValueKind::BinaryLessThanInstKind: // <
      BCFGen_->emitLess(res, left, right);
      break;
    case ValueKind::BinaryLessThanOrEqualInstKind: // <=
      BCFGen_->emitLessEq(res, left, right);
      break;
    case ValueKind::BinaryGreaterThanInstKind: // >
      BCFGen_->emitGreater(res, left, right);
      break;
    case ValueKind::BinaryGreaterThanOrEqualInstKind: // >=
      BCFGen_->emitGreaterEq(res, left, right);
      break;
    case ValueKind::BinaryLeftShiftInstKind: // <<  (<<=)
      BCFGen_->emitLShift(res, left, right);
      break;
    case ValueKind::BinaryRightShiftInstKind: // >>  (>>=)
      BCFGen_->emitRShift(res, left, right);
      break;
    case ValueKind::BinaryUnsignedRightShiftInstKind: // >>> (>>>=)
      BCFGen_->emitURshift(res, left, right);
      break;
    case ValueKind::BinaryAddInstKind: // +   (+=)
      if (isBothNumber) {
        BCFGen_->emitAddN(res, left, right);
      } else {
        BCFGen_->emitAdd(res, left, right);
      }
      break;
    case ValueKind::BinarySubtractInstKind: // -   (-=)
      if (isBothNumber) {
        BCFGen_->emitSubN(res, left, right);
      } else {
        BCFGen_->emitSub(res, left, right);
      }
      break;
    case ValueKind::BinaryMultiplyInstKind: // *   (*=)
      if (isBothNumber) {
        BCFGen_->emitMulN(res, left, right);
      } else {
        BCFGen_->emitMul(res, left, right);
      }
      break;
    case ValueKind::BinaryDivideInstKind: // /   (/=)
      if (isBothNumber) {
        BCFGen_->emitDivN(res, left, right);
      } else {
        BCFGen_->emitDiv(res, left, right);
      }
      break;
    case ValueKind::BinaryExponentiationInstKind: // ** (**=)
      llvm_unreachable("ExponentiationKind emits a HermesInternal call");
      break;
    case ValueKind::BinaryModuloInstKind: // %   (%=)
      BCFGen_->emitMod(res, left, right);
      break;
    case ValueKind::BinaryOrInstKind: // |   (|=)
      BCFGen_->emitBitOr(res, left, right);
      break;
    case ValueKind::BinaryXorInstKind: // ^   (^=)
      BCFGen_->emitBitXor(res, left, right);
      break;
    case ValueKind::BinaryAndInstKind: // &   (^=)
      BCFGen_->emitBitAnd(res, left, right);
      break;
    case ValueKind::BinaryInInstKind: // "in"
      BCFGen_->emitIsIn(res, left, right);
      break;
    case ValueKind::BinaryInstanceOfInstKind: // instanceof
      BCFGen_->emitInstanceOf(res, left, right);
      break;

    default:
      break;
  }
}
void HBCISel::generateStorePropertyLooseInst(
    StorePropertyLooseInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  if (auto *Lit = llvh::dyn_cast<LiteralString>(prop)) {
    // Property is a string
    auto id = BCFGen_->getIdentifierID(Lit);
    if (id <= UINT16_MAX)
      BCFGen_->emitPutByIdLoose(
          objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
    else
      BCFGen_->emitPutByIdLooseLong(
          objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
    return;
  }

  auto propReg = encodeValue(prop);
  BCFGen_->emitPutByValLoose(objReg, propReg, valueReg);
}

void HBCISel::generateStorePropertyStrictInst(
    StorePropertyStrictInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  if (auto *Lit = llvh::dyn_cast<LiteralString>(prop)) {
    // Property is a string
    auto id = BCFGen_->getIdentifierID(Lit);
    if (id <= UINT16_MAX)
      BCFGen_->emitPutByIdStrict(
          objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
    else
      BCFGen_->emitPutByIdStrictLong(
          objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
    return;
  }

  auto propReg = encodeValue(prop);
  BCFGen_->emitPutByValStrict(objReg, propReg, valueReg);
}

void HBCISel::generateTryStoreGlobalPropertyLooseInst(
    TryStoreGlobalPropertyLooseInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  auto *Lit = cast<LiteralString>(prop);

  auto id = BCFGen_->getIdentifierID(Lit);
  if (id <= UINT16_MAX)
    BCFGen_->emitTryPutByIdLoose(
        objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
  else
    BCFGen_->emitTryPutByIdLooseLong(
        objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
}

void HBCISel::generateTryStoreGlobalPropertyStrictInst(
    TryStoreGlobalPropertyStrictInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  auto *Lit = cast<LiteralString>(prop);

  auto id = BCFGen_->getIdentifierID(Lit);
  if (id <= UINT16_MAX)
    BCFGen_->emitTryPutByIdStrict(
        objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
  else
    BCFGen_->emitTryPutByIdStrictLong(
        objReg, valueReg, acquirePropertyWriteCacheIndex(id), id);
}

void HBCISel::generateStoreOwnPropertyInst(
    StoreOwnPropertyInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  Value *prop = Inst->getProperty();
  bool isEnumerable = Inst->getIsEnumerable();

  // If the property is a LiteralNumber, the property is enumerable, and it is a
  // valid array index, it is coming from an array initialization and we will
  // emit it as PutByIndex.
  auto *numProp = llvh::dyn_cast<LiteralNumber>(prop);
  if (numProp && isEnumerable) {
    if (auto arrayIndex = numProp->convertToArrayIndex()) {
      uint32_t index = arrayIndex.getValue();
      if (index <= UINT8_MAX) {
        BCFGen_->emitPutOwnByIndex(objReg, valueReg, index);
      } else {
        BCFGen_->emitPutOwnByIndexL(objReg, valueReg, index);
      }

      return;
    }
  }

  // It is a register operand.
  auto propReg = encodeValue(Inst->getProperty());
  BCFGen_->emitPutOwnByVal(objReg, valueReg, propReg, Inst->getIsEnumerable());
}

void HBCISel::generateStoreNewOwnPropertyInst(
    StoreNewOwnPropertyInst *Inst,
    BasicBlock *next) {
  auto valueReg = encodeValue(Inst->getStoredValue());
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();
  bool isEnumerable = Inst->getIsEnumerable();

  if (auto *numProp = llvh::dyn_cast<LiteralNumber>(prop)) {
    assert(
        isEnumerable &&
        "No way to generate non-enumerable indexed StoreNewOwnPropertyInst.");
    uint32_t index = *numProp->convertToArrayIndex();
    if (index <= UINT8_MAX) {
      BCFGen_->emitPutOwnByIndex(objReg, valueReg, index);
    } else {
      BCFGen_->emitPutOwnByIndexL(objReg, valueReg, index);
    }
    return;
  }

  auto strProp = cast<LiteralString>(prop);
  auto id = BCFGen_->getIdentifierID(strProp);

  if (isEnumerable) {
    if (id > UINT16_MAX) {
      BCFGen_->emitPutNewOwnByIdLong(objReg, valueReg, id);
    } else if (id > UINT8_MAX) {
      BCFGen_->emitPutNewOwnById(objReg, valueReg, id);
    } else {
      BCFGen_->emitPutNewOwnByIdShort(objReg, valueReg, id);
    }
  } else {
    if (id > UINT16_MAX) {
      BCFGen_->emitPutNewOwnNEByIdLong(objReg, valueReg, id);
    } else {
      BCFGen_->emitPutNewOwnNEById(objReg, valueReg, id);
    }
  }
}

void HBCISel::generateStoreGetterSetterInst(
    StoreGetterSetterInst *Inst,
    BasicBlock *next) {
  auto objReg = encodeValue(Inst->getObject());
  auto ident = encodeValue(Inst->getProperty());
  BCFGen_->emitPutOwnGetterSetterByVal(
      objReg,
      ident,
      encodeValue(Inst->getStoredGetter()),
      encodeValue(Inst->getStoredSetter()),
      Inst->getIsEnumerable());
}
void HBCISel::generateDeletePropertyLooseInst(
    DeletePropertyLooseInst *Inst,
    BasicBlock *next) {
  auto objReg = encodeValue(Inst->getObject());
  auto resultReg = encodeValue(Inst);
  auto prop = Inst->getProperty();

  if (auto *Lit = llvh::dyn_cast<LiteralString>(prop)) {
    auto id = BCFGen_->getIdentifierID(Lit);
    if (id <= UINT16_MAX)
      BCFGen_->emitDelByIdLoose(resultReg, objReg, id);
    else
      BCFGen_->emitDelByIdLooseLong(resultReg, objReg, id);
    return;
  }

  auto propReg = encodeValue(prop);
  BCFGen_->emitDelByValLoose(resultReg, objReg, propReg);
}
void HBCISel::generateDeletePropertyStrictInst(
    DeletePropertyStrictInst *Inst,
    BasicBlock *next) {
  auto objReg = encodeValue(Inst->getObject());
  auto resultReg = encodeValue(Inst);
  auto prop = Inst->getProperty();

  if (auto *Lit = llvh::dyn_cast<LiteralString>(prop)) {
    auto id = BCFGen_->getIdentifierID(Lit);
    if (id <= UINT16_MAX)
      BCFGen_->emitDelByIdStrict(resultReg, objReg, id);
    else
      BCFGen_->emitDelByIdStrictLong(resultReg, objReg, id);
    return;
  }

  auto propReg = encodeValue(prop);
  BCFGen_->emitDelByValStrict(resultReg, objReg, propReg);
}
void HBCISel::generateLoadPropertyInst(
    LoadPropertyInst *Inst,
    BasicBlock *next) {
  auto resultReg = encodeValue(Inst);
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  if (auto *Lit = llvh::dyn_cast<LiteralString>(prop)) {
    auto id = BCFGen_->getIdentifierID(Lit);
    if (id > UINT16_MAX) {
      BCFGen_->emitGetByIdLong(
          resultReg, objReg, acquirePropertyReadCacheIndex(id), id);
    } else if (id > UINT8_MAX) {
      BCFGen_->emitGetById(
          resultReg, objReg, acquirePropertyReadCacheIndex(id), id);
    } else {
      BCFGen_->emitGetByIdShort(
          resultReg, objReg, acquirePropertyReadCacheIndex(id), id);
    }
    return;
  }

  auto propReg = encodeValue(prop);
  BCFGen_->emitGetByVal(resultReg, objReg, propReg);
}

void HBCISel::generateTryLoadGlobalPropertyInst(
    TryLoadGlobalPropertyInst *Inst,
    BasicBlock *next) {
  auto resultReg = encodeValue(Inst);
  auto objReg = encodeValue(Inst->getObject());
  auto prop = Inst->getProperty();

  auto *Lit = cast<LiteralString>(prop);

  auto id = BCFGen_->getIdentifierID(Lit);
  if (id > UINT16_MAX) {
    BCFGen_->emitTryGetByIdLong(
        resultReg, objReg, acquirePropertyReadCacheIndex(id), id);
  } else {
    BCFGen_->emitTryGetById(
        resultReg, objReg, acquirePropertyReadCacheIndex(id), id);
  }
}

void HBCISel::generateStoreStackInst(StoreStackInst *Inst, BasicBlock *next) {
  llvm_unreachable("StoreStackInst should have been lowered.");
}
void HBCISel::generateStoreFrameInst(StoreFrameInst *Inst, BasicBlock *next) {
  Variable *var = Inst->getVariable();
  auto valueReg = encodeValue(Inst->getValue());
  auto envReg = encodeValue(Inst->getScope());
  auto varIdx = encodeValue(var);
  if (Inst->getValue()->getType().isNonPtr()) {
    if (varIdx <= UINT8_MAX) {
      BCFGen_->emitStoreNPToEnvironment(envReg, varIdx, valueReg);
    } else {
      BCFGen_->emitStoreNPToEnvironmentL(envReg, varIdx, valueReg);
    }
  } else {
    if (varIdx <= UINT8_MAX) {
      BCFGen_->emitStoreToEnvironment(envReg, varIdx, valueReg);
    } else {
      BCFGen_->emitStoreToEnvironmentL(envReg, varIdx, valueReg);
    }
  }
}
void HBCISel::generateAllocStackInst(AllocStackInst *Inst, BasicBlock *next) {
  // This is a no-op.
}
void HBCISel::generateAllocObjectInst(AllocObjectInst *Inst, BasicBlock *next) {
  auto result = encodeValue(Inst);
  // TODO: Utilize sizeHint.
  if (llvh::isa<EmptySentinel>(Inst->getParentObject())) {
    BCFGen_->emitNewObject(result);
  } else {
    auto parentReg = encodeValue(Inst->getParentObject());
    BCFGen_->emitNewObjectWithParent(result, parentReg);
  }
}
void HBCISel::generateAllocObjectLiteralInst(
    AllocObjectLiteralInst *,
    BasicBlock *) {
  // This instruction should not have reached this far.
  llvm_unreachable("AllocObjectLiteralInst should have been lowered.");
}
void HBCISel::generateAllocArrayInst(AllocArrayInst *Inst, BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  auto elementCount = Inst->getElementCount();
  uint32_t sizeHint =
      std::min((uint32_t)UINT16_MAX, Inst->getSizeHint()->asUInt32());

  if (elementCount == 0) {
    BCFGen_->emitNewArray(dstReg, sizeHint);
  } else {
    SmallVector<Literal *, 8> elements;
    for (unsigned i = 0, e = Inst->getElementCount(); i < e; ++i) {
      elements.push_back(cast<Literal>(Inst->getArrayElement(i)));
    }
    auto bufIndex =
        BCFGen_->BMGen_.addArrayBuffer(ArrayRef<Literal *>{elements});
    if (bufIndex <= UINT16_MAX) {
      BCFGen_->emitNewArrayWithBuffer(
          encodeValue(Inst), sizeHint, elementCount, bufIndex);
    } else {
      BCFGen_->emitNewArrayWithBufferLong(
          encodeValue(Inst), sizeHint, elementCount, bufIndex);
    }
  }
}
void HBCISel::generateGetTemplateObjectInst(
    GetTemplateObjectInst *inst,
    BasicBlock *next) {
  hermes_fatal("GetTemplateObjectInst unsupported in HBC");
}
void HBCISel::generateCreateFunctionInst(
    CreateFunctionInst *Inst,
    BasicBlock *) {
  auto env = encodeValue(Inst->getScope());
  auto output = encodeValue(Inst);
  auto code = BCFGen_->getFunctionID(Inst->getFunctionCode());
  bool isGen = llvh::isa<GeneratorFunction>(Inst->getFunctionCode());
  bool isAsync = llvh::isa<AsyncFunction>(Inst->getFunctionCode());
  if (LLVM_LIKELY(code <= UINT16_MAX)) {
    // Most of the cases, function index will be less than 2^16.
    if (isAsync) {
      BCFGen_->emitCreateAsyncClosure(output, env, code);
    } else if (isGen) {
      BCFGen_->emitCreateGeneratorClosure(output, env, code);
    } else {
      BCFGen_->emitCreateClosure(output, env, code);
    }
  } else {
    if (isAsync) {
      BCFGen_->emitCreateAsyncClosureLongIndex(output, env, code);
    } else if (isGen) {
      BCFGen_->emitCreateGeneratorClosureLongIndex(output, env, code);
    } else {
      BCFGen_->emitCreateClosureLongIndex(output, env, code);
    }
  }
}

void HBCISel::generateHBCAllocObjectFromBufferInst(
    HBCAllocObjectFromBufferInst *Inst,
    BasicBlock *next) {
  auto result = encodeValue(Inst);
  int e = Inst->getKeyValuePairCount();
  SmallVector<Literal *, 8> objKeys;
  SmallVector<Literal *, 8> objVals;
  for (int ind = 0; ind < e; ind++) {
    auto keyValuePair = Inst->getKeyValuePair(ind);
    objKeys.push_back(cast<Literal>(keyValuePair.first));
    objVals.push_back(cast<Literal>(keyValuePair.second));
  }

  // size hint operand of NewObjectWithBuffer opcode is 16-bit.
  uint32_t sizeHint =
      std::min((uint32_t)UINT16_MAX, Inst->getSizeHint()->asUInt32());

  auto buffIdxs = BCFGen_->BMGen_.addObjectBuffer(
      llvh::ArrayRef<Literal *>{objKeys}, llvh::ArrayRef<Literal *>{objVals});
  if (buffIdxs.first <= UINT16_MAX && buffIdxs.second <= UINT16_MAX) {
    BCFGen_->emitNewObjectWithBuffer(
        result, sizeHint, e, buffIdxs.first, buffIdxs.second);
  } else {
    BCFGen_->emitNewObjectWithBufferLong(
        result, sizeHint, e, buffIdxs.first, buffIdxs.second);
  }
}

void HBCISel::generateCatchInst(CatchInst *Inst, BasicBlock *next) {
  auto loc = BCFGen_->emitCatch(encodeValue(Inst));
  relocations_.push_back({loc, Relocation::CatchType, Inst});
  catchInfoMap_[Inst] = CatchCoverageInfo(loc);
}
void HBCISel::generateDebuggerInst(DebuggerInst *Inst, BasicBlock *next) {
  BCFGen_->emitDebugger();
}
void HBCISel::generateCreateRegExpInst(
    CreateRegExpInst *Inst,
    BasicBlock *next) {
  const auto &pattern = Inst->getPattern()->getValue();
  const auto &flags = Inst->getFlags()->getValue();
  auto &ctx = F_->getParent()->getContext();
  auto &regexp = ctx.getCompiledRegExp(
      pattern.getUnderlyingPointer(), flags.getUnderlyingPointer());
  uint32_t reBytecodeID = BCFGen_->addRegExp(&regexp);
  auto patternStrID = BCFGen_->getStringID(Inst->getPattern());
  auto flagsStrID = BCFGen_->getStringID(Inst->getFlags());
  BCFGen_->emitCreateRegExp(
      encodeValue(Inst), patternStrID, flagsStrID, reBytecodeID);
}
void HBCISel::generateTryEndInst(TryEndInst *Inst, BasicBlock *next) {
  // This is a no-op.
  // TryEndInst is used to mark the end of a try region to construct
  // the list of basic blocks covered by a catch.
  // The range of try regions are stored in exception handlers,
  // and are therefore not encoded in the instruction stream.
}
void HBCISel::generateBranchInst(BranchInst *Inst, BasicBlock *next) {
  auto *dst = Inst->getBranchDest();
  if (dst == next)
    return;

  auto loc = BCFGen_->emitJmpLong(0);
  registerLongJump(loc, dst);
}
void HBCISel::generateReturnInst(ReturnInst *Inst, BasicBlock *next) {
  auto value = encodeValue(Inst->getValue());
  Function *F = Inst->getParent()->getParent();
  if (llvh::isa<GeneratorInnerFunction>(F)) {
    // Generator inner functions must complete before `return`,
    // unlike when they yield.
    BCFGen_->emitCompleteGenerator();
  }
  BCFGen_->emitRet(value);
}
void HBCISel::generateThrowInst(ThrowInst *Inst, BasicBlock *next) {
  BCFGen_->emitThrow(encodeValue(Inst->getThrownValue()));
}
void HBCISel::generateThrowIfInst(
    hermes::ThrowIfInst *Inst,
    hermes::BasicBlock *next) {
  assert(
      Inst->getInvalidTypes()->getData().isEmptyType() &&
      "Only Empty supported");
  BCFGen_->emitThrowIfEmpty(
      encodeValue(Inst), encodeValue(Inst->getCheckedValue()));
}
void HBCISel::generateSwitchInst(SwitchInst *Inst, BasicBlock *next) {
  llvm_unreachable("SwitchInst should have been lowered");
}
void HBCISel::generateSaveAndYieldInst(
    SaveAndYieldInst *Inst,
    BasicBlock *next) {
  auto result = encodeValue(Inst->getResult());
  auto loc = BCFGen_->emitSaveGeneratorLong(0);
  registerLongJump(loc, Inst->getNextBlock());
  BCFGen_->emitRet(result);
}
void HBCISel::generateCreateGeneratorInst(
    CreateGeneratorInst *Inst,
    BasicBlock *next) {
  auto env = encodeValue(Inst->getScope());
  auto output = encodeValue(Inst);
  auto code = BCFGen_->getFunctionID(Inst->getFunctionCode());
  if (LLVM_LIKELY(code <= UINT16_MAX)) {
    // Most of the cases, function index will be less than 2^16.
    BCFGen_->emitCreateGenerator(output, env, code);
  } else {
    BCFGen_->emitCreateGeneratorLongIndex(output, env, code);
  }
}
void HBCISel::generateStartGeneratorInst(
    StartGeneratorInst *Inst,
    BasicBlock *next) {
  BCFGen_->emitStartGenerator();
}
void HBCISel::generateResumeGeneratorInst(
    ResumeGeneratorInst *Inst,
    BasicBlock *next) {
  auto value = encodeValue(Inst);
  auto isReturn = encodeValue(Inst->getIsReturn());
  BCFGen_->emitResumeGenerator(value, isReturn);
}

void HBCISel::generateCondBranchInst(CondBranchInst *Inst, BasicBlock *next) {
  auto condReg = encodeValue(Inst->getCondition());

  BasicBlock *trueBlock = Inst->getTrueDest();
  BasicBlock *falseBlock = Inst->getFalseDest();

  // Emit a conditional jump to the 'False' destination and a fall-through to
  // the 'True' side.
  if (next == trueBlock) {
    auto loc = BCFGen_->emitJmpFalseLong(0, condReg);
    registerLongJump(loc, falseBlock);
    return;
  }

  // Emit a conditional jump to the 'True' destination and a fall-through to the
  // 'False' side.
  auto loc = BCFGen_->emitJmpTrueLong(0, condReg);
  registerLongJump(loc, trueBlock);

  // Try to eliminate the branch by using a fall-through. If the destination
  // is also the basic block we'll generate next then no need to emit the
  // jmp.
  if (next == falseBlock) {
    return;
  }

  loc = BCFGen_->emitJmpLong(0);
  registerLongJump(loc, falseBlock);
}

void HBCISel::generateCompareBranchInst(
    CompareBranchInst *Inst,
    BasicBlock *next) {
  auto left = encodeValue(Inst->getLeftHandSide());
  auto right = encodeValue(Inst->getRightHandSide());
  auto res = encodeValue(Inst);

  bool isBothNumber = Inst->getLeftHandSide()->getType().isNumberType() &&
      Inst->getRightHandSide()->getType().isNumberType();

  BasicBlock *trueBlock = Inst->getTrueDest();
  BasicBlock *falseBlock = Inst->getFalseDest();

  bool invert = false;

  // If we need to fall-through to the "true" case, invert the condition and
  // the jump targets.
  if (next == trueBlock) {
    invert = true;
    std::swap(trueBlock, falseBlock);
  }

  offset_t loc;
  switch (Inst->getKind()) {
    case ValueKind::CmpBrLessThanInstKind: // <
      loc = invert
          ? (isBothNumber ? BCFGen_->emitJNotLessNLong(res, left, right)
                          : BCFGen_->emitJNotLessLong(res, left, right))
          : (isBothNumber ? BCFGen_->emitJLessNLong(res, left, right)
                          : BCFGen_->emitJLessLong(res, left, right));
      break;
    case ValueKind::CmpBrLessThanOrEqualInstKind: // <=
      loc = invert
          ? (isBothNumber ? BCFGen_->emitJNotLessEqualNLong(res, left, right)
                          : BCFGen_->emitJNotLessEqualLong(res, left, right))
          : (isBothNumber ? BCFGen_->emitJLessEqualNLong(res, left, right)
                          : BCFGen_->emitJLessEqualLong(res, left, right));
      break;
    case ValueKind::CmpBrGreaterThanInstKind: // >
      loc = invert
          ? (isBothNumber ? BCFGen_->emitJNotGreaterNLong(res, left, right)
                          : BCFGen_->emitJNotGreaterLong(res, left, right))
          : (isBothNumber ? BCFGen_->emitJGreaterNLong(res, left, right)
                          : BCFGen_->emitJGreaterLong(res, left, right));
      break;
    case ValueKind::CmpBrGreaterThanOrEqualInstKind: // >=
      loc = invert
          ? (isBothNumber ? BCFGen_->emitJNotGreaterEqualNLong(res, left, right)
                          : BCFGen_->emitJNotGreaterEqualLong(res, left, right))
          : (isBothNumber ? BCFGen_->emitJGreaterEqualNLong(res, left, right)
                          : BCFGen_->emitJGreaterEqualLong(res, left, right));
      break;

    case ValueKind::CmpBrEqualInstKind:
      loc = invert ? BCFGen_->emitJNotEqualLong(res, left, right)
                   : BCFGen_->emitJEqualLong(res, left, right);
      break;

    case ValueKind::CmpBrNotEqualInstKind:
      loc = invert ? BCFGen_->emitJEqualLong(res, left, right)
                   : BCFGen_->emitJNotEqualLong(res, left, right);
      break;

    case ValueKind::CmpBrStrictlyEqualInstKind:
      loc = invert ? BCFGen_->emitJStrictNotEqualLong(res, left, right)
                   : BCFGen_->emitJStrictEqualLong(res, left, right);
      break;

    case ValueKind::CmpBrStrictlyNotEqualInstKind:
      loc = invert ? BCFGen_->emitJStrictEqualLong(res, left, right)
                   : BCFGen_->emitJStrictNotEqualLong(res, left, right);
      break;

    default:
      llvm_unreachable("invalid compare+branch operator");
      break;
  }

  registerLongJump(loc, trueBlock);

  // Try to eliminate the branch by using a fall-through. If the destination
  // is also the basic block we'll generate next then no need to emit the
  // jmp.
  if (next == falseBlock) {
    return;
  }

  loc = BCFGen_->emitJmpLong(res);
  registerLongJump(loc, falseBlock);
}
void HBCISel::generateGetPNamesInst(GetPNamesInst *Inst, BasicBlock *next) {
  auto itrReg = encodeValue(Inst->getIterator());
  BCFGen_->emitGetPNameList(
      itrReg,
      encodeValue(Inst->getBase()),
      encodeValue(Inst->getIndex()),
      encodeValue(Inst->getSize()));
  // If the returned iterator is empty value, quit the iteration directly.
  registerLongJump(
      BCFGen_->emitJmpUndefinedLong(0, itrReg), Inst->getOnEmptyDest());

  auto *onSomeBlock = Inst->getOnSomeDest();
  if (next != onSomeBlock) {
    auto loc = BCFGen_->emitJmpLong(0);
    registerLongJump(loc, onSomeBlock);
  }
}
void HBCISel::generateGetNextPNameInst(
    GetNextPNameInst *Inst,
    BasicBlock *next) {
  auto indexReg = encodeValue(Inst->getIndexAddr());
  auto sizeReg = encodeValue(Inst->getSizeAddr());
  auto propReg = encodeValue(Inst->getPropertyAddr());

  BCFGen_->emitGetNextPName(
      propReg,
      encodeValue(Inst->getIteratorAddr()),
      encodeValue(Inst->getBaseAddr()),
      indexReg,
      sizeReg);

  // If the returned property is empty, terminate the iteration.
  registerLongJump(
      BCFGen_->emitJmpUndefinedLong(0, propReg), Inst->getOnLastDest());

  auto *onSomeBlock = Inst->getOnSomeDest();
  if (next != onSomeBlock) {
    auto loc = BCFGen_->emitJmpLong(0);
    registerLongJump(loc, onSomeBlock);
  }
}
void HBCISel::generateTryStartInst(TryStartInst *Inst, BasicBlock *next) {
  // TryStartInst is the same as BranchInst in bytecode gen.
  BasicBlock *destination = Inst->getTryBody();

  assert(
      Inst->getParent()->getTerminator() == Inst &&
      "BI is not the terminator!");

  // Try to eliminate the branch by using a fall-through. If the destination
  // is also the basic block we'll generate next then no need to emit the
  // jmp.
  if (next == destination) {
    return;
  }

  // Emit the jump and remember to update the relocation location.
  auto loc = BCFGen_->emitJmpLong(0);
  registerLongJump(loc, destination);
}
void HBCISel::generateCallInst(CallInst *Inst, BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto function = encodeValue(Inst->getCallee());
  bool newTargetIsUndefined = llvh::isa<LiteralUndefined>(Inst->getNewTarget());
  assert(
      Inst->getNumArguments() <= UINT8_MAX &&
      "CallInst doesn't support large number of args");
  verifyCall(Inst);

  if (newTargetIsUndefined) {
    BCFGen_->emitCall(output, function, Inst->getNumArguments());
  } else {
    auto newTarget = encodeValue(Inst->getNewTarget());
    if (function == newTarget) {
      BCFGen_->emitConstruct(output, function, Inst->getNumArguments());
    } else {
      BCFGen_->emitCallWithNewTarget(
          output, function, newTarget, Inst->getNumArguments());
    }
  }
}

void HBCISel::generateHBCCallWithArgCountInst(
    HBCCallWithArgCountInst *Inst,
    BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto function = encodeValue(Inst->getCallee());
  auto newTarget = encodeValue(Inst->getNewTarget());
  auto argCount = encodeValue(Inst->getNumArgumentsLiteral());
  BCFGen_->emitCallWithNewTargetLong(output, function, newTarget, argCount);
}

void HBCISel::generateHBCCallNInst(HBCCallNInst *Inst, BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto function = encodeValue(Inst->getCallee());
  verifyCall(Inst);

  static_assert(
      HBCCallNInst::kMinArgs == 1 && HBCCallNInst::kMaxArgs == 4,
      "Update generateHBCCallNInst to reflect min/max arg range");

  switch (Inst->getNumArguments()) {
    case 1:
      BCFGen_->emitCall1(output, function, encodeValue(Inst->getArgument(0)));
      break;
    case 2:
      BCFGen_->emitCall2(
          output,
          function,
          encodeValue(Inst->getArgument(0)),
          encodeValue(Inst->getArgument(1)));
      break;
    case 3:
      BCFGen_->emitCall3(
          output,
          function,
          encodeValue(Inst->getArgument(0)),
          encodeValue(Inst->getArgument(1)),
          encodeValue(Inst->getArgument(2)));
      break;
    case 4:
      BCFGen_->emitCall4(
          output,
          function,
          encodeValue(Inst->getArgument(0)),
          encodeValue(Inst->getArgument(1)),
          encodeValue(Inst->getArgument(2)),
          encodeValue(Inst->getArgument(3)));
      break;
    default:
      llvm_unreachable("Unexpected argument count");
  }
}

void HBCISel::generateCallBuiltinInst(CallBuiltinInst *Inst, BasicBlock *next) {
  auto output = encodeValue(Inst);
  verifyCall(Inst);

  if (Inst->getNumArguments() <= UINT8_MAX) {
    BCFGen_->emitCallBuiltin(
        output, Inst->getBuiltinIndex(), Inst->getNumArguments());
  } else {
    BCFGen_->emitCallBuiltinLong(
        output, Inst->getBuiltinIndex(), Inst->getNumArguments());
  }
}

void HBCISel::generateGetBuiltinClosureInst(
    GetBuiltinClosureInst *Inst,
    BasicBlock *next) {
  auto output = encodeValue(Inst);
  BCFGen_->emitGetBuiltinClosure(output, Inst->getBuiltinIndex());
}

void HBCISel::generateGetParentScopeInst(
    GetParentScopeInst *Inst,
    hermes::BasicBlock *next) {
  BCFGen_->emitGetParentEnvironment(encodeValue(Inst), 0);
}

void HBCISel::generateResolveScopeInst(
    ResolveScopeInst *Inst,
    BasicBlock *next) {
  llvm_unreachable("ResolveScopeInst should have been lowered.");
}

void HBCISel::generateLIRResolveScopeInst(
    hermes::LIRResolveScopeInst *Inst,
    hermes::BasicBlock *next) {
  auto delta = Inst->getNumLevels()->asUInt32();
  if (std::numeric_limits<uint8_t>::max() < delta) {
    F_->getContext().getSourceErrorManager().error(
        Inst->getLocation(), "Variable environment is out-of-reach");
  }
  BCFGen_->emitGetEnvironment(
      encodeValue(Inst), encodeValue(Inst->getStartScope()), delta);
}

void HBCISel::generateHBCResolveParentEnvironmentInst(
    HBCResolveParentEnvironmentInst *Inst,
    BasicBlock *next) {
  auto delta = Inst->getNumLevels()->asUInt32();
  if (std::numeric_limits<uint8_t>::max() < delta) {
    F_->getContext().getSourceErrorManager().error(
        Inst->getLocation(), "Variable environment is out-of-reach");
  }
  BCFGen_->emitGetParentEnvironment(encodeValue(Inst), delta);
}

void HBCISel::generateGetClosureScopeInst(
    hermes::GetClosureScopeInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto closure = encodeValue(Inst->getClosure());
  BCFGen_->emitGetClosureEnvironment(output, closure);
}

void HBCISel::generateHBCLoadConstInst(
    hermes::HBCLoadConstInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  Literal *literal = Inst->getConst();
  switch (literal->getKind()) {
    case ValueKind::LiteralEmptyKind:
      BCFGen_->emitLoadConstEmpty(output);
      break;
    case ValueKind::LiteralUndefinedKind:
      BCFGen_->emitLoadConstUndefined(output);
      break;
    case ValueKind::LiteralNullKind:
      BCFGen_->emitLoadConstNull(output);
      break;
    case ValueKind::LiteralBoolKind: {
      auto *litBool = cast<LiteralBool>(literal);
      if (litBool->getValue()) {
        BCFGen_->emitLoadConstTrue(output);
      } else {
        BCFGen_->emitLoadConstFalse(output);
      }
      break;
    }
    case ValueKind::LiteralNumberKind: {
      auto *litNum = cast<LiteralNumber>(literal);
      // Check for +0.0.
      if (litNum->isPositiveZero()) {
        BCFGen_->emitLoadConstZero(output);
      } else {
        if (litNum->isUInt8Representible()) {
          BCFGen_->emitLoadConstUInt8(output, litNum->asUInt8());
        } else if (litNum->isInt32Representible()) {
          BCFGen_->emitLoadConstInt(output, litNum->asInt32());
        } else {
          // param_t is int64_t, we cannot directly convert a double into that.
          // Instead we are going to copy it as if it is binary.
          BCFGen_->emitLoadConstDoubleDirect(output, litNum->getValue());
        }
      }
      break;
    }
    case ValueKind::LiteralBigIntKind: {
      auto parsedBigInt = bigint::ParsedBigInt::parsedBigIntFromNumericValue(
          cast<LiteralBigInt>(literal)->getValue()->str());
      assert(parsedBigInt && "should be valid");
      if (bigint::tooManyBytes(parsedBigInt->getBytes().size())) {
        // TODO: move this to the semantic analysis so we can get a proper
        // warning (i.e., with the correct location for the literal).
        std::string sizeStr;
        {
          llvh::raw_string_ostream OS(sizeStr);
          OS << parsedBigInt->getBytes().size();
        }
        F_->getContext().getSourceErrorManager().warning(
            Inst->getLocation(),
            Twine("BigInt literal has too many bytes (") + sizeStr +
                ") and a RangeError will be raised at runtime time if it "
                "is referenced.");
      }
      auto idx = BCFGen_->addBigInt(std::move(*parsedBigInt));
      if (idx <= UINT16_MAX) {
        BCFGen_->emitLoadConstBigInt(output, idx);
      } else {
        BCFGen_->emitLoadConstBigIntLongIndex(output, idx);
      }
      break;
    }
    case ValueKind::LiteralStringKind: {
      auto idx = BCFGen_->getStringID(cast<LiteralString>(literal));
      if (idx <= UINT16_MAX) {
        BCFGen_->emitLoadConstString(output, idx);
      } else {
        BCFGen_->emitLoadConstStringLongIndex(output, idx);
      }
      break;
    }
    default:
      llvm_unreachable("Invalid literal type");
  }
}
void HBCISel::generateLoadParamInst(
    hermes::LoadParamInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  uint32_t value = Inst->getParam()->getIndexInParamList();
  if (value <= UINT8_MAX) {
    BCFGen_->emitLoadParam(output, value);
  } else {
    BCFGen_->emitLoadParamLong(output, value);
  }
}

void HBCISel::generateCreateScopeInst(
    hermes::CreateScopeInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  Value *parent = Inst->getParentScope();
  // Check if this is the top level scope.
  if (llvh::isa<EmptySentinel>(parent)) {
    BCFGen_->emitCreateFunctionEnvironment(dstReg);
  } else {
    BCFGen_->emitCreateEnvironment(
        dstReg,
        encodeValue(parent),
        Inst->getVariableScope()->getVariables().size());
  }
}

void HBCISel::generateHBCCreateFunctionEnvironmentInst(
    hermes::HBCCreateFunctionEnvironmentInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  BCFGen_->emitCreateFunctionEnvironment(dstReg);
}

void HBCISel::generateHBCProfilePointInst(
    hermes::HBCProfilePointInst *Inst,
    hermes::BasicBlock *next) {
  BCFGen_->emitProfilePoint(Inst->getPointIndex());
}

void HBCISel::generateHBCGetGlobalObjectInst(
    hermes::HBCGetGlobalObjectInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  BCFGen_->emitGetGlobalObject(dstReg);
}

void HBCISel::generateGetNewTargetInst(
    hermes::GetNewTargetInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  BCFGen_->emitGetNewTarget(dstReg);
}

void HBCISel::generateLIRGetThisNSInst(
    hermes::LIRGetThisNSInst *Inst,
    hermes::BasicBlock *next) {
  auto dstReg = encodeValue(Inst);
  BCFGen_->emitLoadThisNS(dstReg);
}
void HBCISel::generateCoerceThisNSInst(
    CoerceThisNSInst *Inst,
    BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  BCFGen_->emitCoerceThisNS(dst, src);
}
void HBCISel::generateHBCGetArgumentsLengthInst(
    hermes::HBCGetArgumentsLengthInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto reg = encodeValue(Inst->getLazyRegister());
  BCFGen_->emitGetArgumentsLength(output, reg);
}
void HBCISel::generateHBCGetArgumentsPropByValLooseInst(
    hermes::HBCGetArgumentsPropByValLooseInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto index = encodeValue(Inst->getIndex());
  auto reg = encodeValue(Inst->getLazyRegister());
  BCFGen_->emitGetArgumentsPropByValLoose(output, index, reg);
}
void HBCISel::generateHBCGetArgumentsPropByValStrictInst(
    hermes::HBCGetArgumentsPropByValStrictInst *Inst,
    hermes::BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto index = encodeValue(Inst->getIndex());
  auto reg = encodeValue(Inst->getLazyRegister());
  BCFGen_->emitGetArgumentsPropByValStrict(output, index, reg);
}
void HBCISel::generateHBCReifyArgumentsLooseInst(
    hermes::HBCReifyArgumentsLooseInst *Inst,
    hermes::BasicBlock *next) {
  auto reg = encodeValue(Inst->getLazyRegister());
  BCFGen_->emitReifyArgumentsLoose(reg);
}
void HBCISel::generateHBCReifyArgumentsStrictInst(
    hermes::HBCReifyArgumentsStrictInst *Inst,
    hermes::BasicBlock *next) {
  auto reg = encodeValue(Inst->getLazyRegister());
  BCFGen_->emitReifyArgumentsStrict(reg);
}
void HBCISel::generateCreateThisInst(CreateThisInst *Inst, BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto proto = encodeValue(Inst->getPrototype());
  auto closure = encodeValue(Inst->getClosure());
  BCFGen_->emitCreateThis(output, proto, closure);
}
void HBCISel::generateGetConstructedObjectInst(
    GetConstructedObjectInst *Inst,
    BasicBlock *next) {
  auto output = encodeValue(Inst);
  auto thisValue = encodeValue(Inst->getThisValue());
  auto constructed = encodeValue(Inst->getConstructorReturnValue());
  BCFGen_->emitSelectObject(output, thisValue, constructed);
}
void HBCISel::generateHBCSpillMovInst(HBCSpillMovInst *Inst, BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getValue());
  emitMovIfNeeded(dst, src);
}
void HBCISel::generateUnreachableInst(
    hermes::UnreachableInst *Inst,
    hermes::BasicBlock *next) {
  emitUnreachableIfDebug();
}

void HBCISel::generateIteratorBeginInst(
    hermes::IteratorBeginInst *Inst,
    hermes::BasicBlock *next) {
  auto iter = encodeValue(Inst);
  auto src = encodeValue(Inst->getSourceOrNext());
  BCFGen_->emitIteratorBegin(iter, src);
}

void HBCISel::generateIteratorNextInst(
    hermes::IteratorNextInst *Inst,
    hermes::BasicBlock *next) {
  auto dst = encodeValue(Inst);
  auto iter = encodeValue(Inst->getIterator());
  auto src = encodeValue(Inst->getSourceOrNext());
  BCFGen_->emitIteratorNext(dst, iter, src);
}

void HBCISel::generateIteratorCloseInst(
    hermes::IteratorCloseInst *Inst,
    hermes::BasicBlock *next) {
  auto iter = encodeValue(Inst->getIterator());
  bool ignoreInnerException = Inst->getIgnoreInnerException();
  BCFGen_->emitIteratorClose(iter, ignoreInnerException);
}

void HBCISel::generateSwitchImmInst(
    hermes::SwitchImmInst *Inst,
    hermes::BasicBlock *next) {
  uint32_t min = Inst->getMinValue();
  uint32_t size = Inst->getSize();

  uint32_t max = min + size - 1;

  std::vector<BasicBlock *> jmpTable;
  jmpTable.resize(size);

  // fill jump table entries in use.
  for (uint32_t caseIdx = 0; caseIdx < Inst->getNumCasePair(); caseIdx++) {
    auto casePair = Inst->getCasePair(caseIdx);
    uint32_t val = casePair.first->asUInt32();
    BasicBlock *target = casePair.second;
    jmpTable[val - min] = target;
  }

  for (uint32_t idx = 0; idx < size; idx++) {
    if (jmpTable[idx] == nullptr)
      jmpTable[idx] = Inst->getDefaultDestination();
  }

  registerSwitchImm(
      BCFGen_->emitSwitchImm(
          encodeValue(Inst->getInputValue()), 0, 0, min, max),
      Inst);
  switchImmInfo_[Inst] = {0, Inst->getDefaultDestination(), jmpTable};
}

void HBCISel::generatePrLoadInst(PrLoadInst *inst, BasicBlock *) {
  F_->getContext().getSourceErrorManager().error(
      inst->getLocation(), inst->getKindStr() + " not implemented");
}
void HBCISel::generatePrStoreInst(PrStoreInst *inst, BasicBlock *) {
  F_->getContext().getSourceErrorManager().error(
      inst->getLocation(), inst->getKindStr() + " not implemented");
}

void HBCISel::generateFCompareInst(FCompareInst *Inst, BasicBlock *) {
  auto left = encodeValue(Inst->getLeft());
  auto right = encodeValue(Inst->getRight());
  auto res = encodeValue(Inst);
  switch (Inst->getKind()) {
    case ValueKind::FEqualInstKind:
      BCFGen_->emitStrictEq(res, left, right);
      break;
    case ValueKind::FNotEqualInstKind:
      BCFGen_->emitStrictNeq(res, left, right);
      break;
    case ValueKind::FLessThanInstKind:
      BCFGen_->emitLess(res, left, right);
      break;
    case ValueKind::FLessThanOrEqualInstKind:
      BCFGen_->emitLessEq(res, left, right);
      break;
    case ValueKind::FGreaterThanInstKind:
      BCFGen_->emitGreater(res, left, right);
      break;
    case ValueKind::FGreaterThanOrEqualInstKind:
      BCFGen_->emitGreaterEq(res, left, right);
      break;
    default:
      hermes_fatal("invalid kind for FCompareInst");
      break;
  }
}
void HBCISel::generateFBinaryMathInst(FBinaryMathInst *Inst, BasicBlock *) {
  auto left = encodeValue(Inst->getLeft());
  auto right = encodeValue(Inst->getRight());
  auto res = encodeValue(Inst);
  switch (Inst->getKind()) {
    case ValueKind::FAddInstKind:
      BCFGen_->emitAddN(res, left, right);
      break;
    case ValueKind::FSubtractInstKind:
      BCFGen_->emitSubN(res, left, right);
      break;
    case ValueKind::FMultiplyInstKind:
      BCFGen_->emitMulN(res, left, right);
      break;
    case ValueKind::FDivideInstKind:
      BCFGen_->emitDivN(res, left, right);
      break;
    case ValueKind::FModuloInstKind:
      BCFGen_->emitMod(res, left, right);
      break;
    default:
      hermes_fatal("invalid kind for FBinaryMathInst");
      break;
  }
}
void HBCISel::generateFUnaryMathInst(FUnaryMathInst *Inst, BasicBlock *) {
  auto opReg = encodeValue(Inst->getArg());
  auto resReg = encodeValue(Inst);
  switch (Inst->getKind()) {
    case ValueKind::FNegateKind:
      BCFGen_->emitNegate(resReg, opReg);
      break;
    default:
      hermes_fatal("invalid kind for FUnaryMathInst");
      break;
  }
}
void HBCISel::generateLoadParentInst(LoadParentInst *, BasicBlock *) {
  hermes_fatal("LoadParentInst not supported.");
}
void HBCISel::generateNativeCallInst(NativeCallInst *, BasicBlock *) {
  hermes_fatal("NativeCallInst not supported.");
}
void HBCISel::generateGetNativeRuntimeInst(
    hermes::GetNativeRuntimeInst *Inst,
    hermes::BasicBlock *next) {
  hermes_fatal("GetNativeRuntimeInst not supported.");
}
void HBCISel::generateStoreParentInst(StoreParentInst *, BasicBlock *) {
  hermes_fatal("StoreParentInst not supported.");
}
void HBCISel::generateLIRDeadValueInst(LIRDeadValueInst *, BasicBlock *) {
  hermes_fatal("LIRDeadValueInst not supported.");
}
void HBCISel::generateFastArrayLoadInst(FastArrayLoadInst *, BasicBlock *) {
  hermes_fatal("FastArrayLoadInst not supported.");
}
void HBCISel::generateFastArrayPushInst(FastArrayPushInst *, BasicBlock *) {
  hermes_fatal("FastArrayPushInst not supported.");
}
void HBCISel::generateAllocFastArrayInst(AllocFastArrayInst *, BasicBlock *) {
  hermes_fatal("AllocFastArrayInst not supported.");
}
void HBCISel::generateFastArrayStoreInst(FastArrayStoreInst *, BasicBlock *) {
  hermes_fatal("FastArrayStoreInst not supported.");
}
void HBCISel::generateThrowTypeErrorInst(ThrowTypeErrorInst *, BasicBlock *) {
  hermes_fatal("ThrowTypeErrorInst should have been lowered.");
}
void HBCISel::generateCheckedTypeCastInst(CheckedTypeCastInst *, BasicBlock *) {
  hermes_fatal("CheckedTypeCastInst not supported.");
}
void HBCISel::generateFastArrayAppendInst(FastArrayAppendInst *, BasicBlock *) {
  hermes_fatal("FastArrayAppendInst not supported.");
}
void HBCISel::generateFastArrayLengthInst(FastArrayLengthInst *, BasicBlock *) {
  hermes_fatal("FastArrayLengthInst not supported.");
}
void HBCISel::generateUnionNarrowTrustedInst(
    UnionNarrowTrustedInst *Inst,
    BasicBlock *) {
  auto dst = encodeValue(Inst);
  auto src = encodeValue(Inst->getSingleOperand());
  emitMovIfNeeded(dst, src);
}
void HBCISel::generateCreateArgumentsLooseInst(
    CreateArgumentsLooseInst *,
    BasicBlock *) {
  hermes_fatal("CreateArgumentsLooseInst not supported.");
}
void HBCISel::generateCreateArgumentsStrictInst(
    CreateArgumentsStrictInst *,
    BasicBlock *) {
  hermes_fatal("CreateArgumentsStrictInst not supported.");
}
void HBCISel::generateStringConcatInst(StringConcatInst *inst, BasicBlock *) {
  hermes_fatal("StringConcatInst not supported.");
}

void HBCISel::generate(BasicBlock *BB, BasicBlock *next) {
  // Register the address of the current basic block.
  auto begin_loc = BCFGen_->getCurrentLocation();

  // It is important to register the basic block before processing it,
  // as we require the relocations to be sorted by their offset.
  relocations_.push_back(
      {begin_loc, Relocation::RelocationType::BasicBlockType, BB});
  basicBlockMap_[BB] = std::make_pair(begin_loc, next);

  if (BB == &F_->front()) {
    initialize();
  }

  // Emit an async break check before the terminator if necessary.
  // We do this at the end of the block so that we come after any
  // CreateEnvironment instruction.
  const Instruction *asyncBreakCheckLoc =
      asyncBreakChecks_.count(BB) ? BB->getTerminator() : nullptr;
  for (auto &I : *BB) {
    if (&I == asyncBreakCheckLoc) {
      BCFGen_->emitAsyncBreakCheck();
    }
    generate(&I, next);
  }
  auto end_loc = BCFGen_->getCurrentLocation();
  if (!next) {
    // When next is nullptr, we are hitting the last BB.
    // We should also register that null BB with it's location.
    assert(
        basicBlockMap_.find(nullptr) == basicBlockMap_.end() &&
        "Multiple nullptr BBs encountered");
    basicBlockMap_[nullptr] = std::make_pair(end_loc, nullptr);
  }

  LLVM_DEBUG(
      dbgs() << "Generated the block " << BB << " from " << begin_loc << " .. "
             << end_loc << "\n");
}

void HBCISel::generate(Instruction *ii, BasicBlock *next) {
  LLVM_DEBUG(dbgs() << "Generating the instruction " << ii->getName() << "\n");

  // Generate the debug info.
  switch (F_->getContext().getDebugInfoSetting()) {
    case DebugInfoSetting::NONE:
      break;
    case DebugInfoSetting::THROWING:
      if (!ii->getSideEffect().getThrow()) {
        break;
      }
    // Falls through - if ii can execute.
    case DebugInfoSetting::SOURCE_MAP:
    case DebugInfoSetting::ALL:
      if (ii->hasLocation()) {
        relocations_.push_back(
            {BCFGen_->getCurrentLocation(),
             Relocation::RelocationType::DebugInfo,
             ii});
      }
      break;
  }

  switch (ii->getKind()) {
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return generate##CLASS(cast<CLASS>(ii), next);
#define DEF_TAG(NAME, PARENT) \
  case ValueKind::NAME##Kind: \
    return generate##PARENT(cast<PARENT>(ii), next);
#include "hermes/IR/Instrs.def"

    default:
      llvm_unreachable("Invalid kind");
  }
}

void HBCISel::generate(SourceMapGenerator *outSourceMap) {
  auto PO = postOrderAnalysis(F_);

  /// The order of the blocks is reverse-post-order, which is a simply
  /// topological sort.
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());

  // If we are compiling with debugger or otherwise need async break checks,
  // decide which blocks need runtime async break checks: blocks with backwards
  // jumps, and the first block if this is a function (i.e. not the
  // global scope).
  if (F_->getContext().getDebugInfoSetting() == DebugInfoSetting::ALL ||
      F_->getContext().getEmitAsyncBreakCheck()) {
    asyncBreakChecks_ = basicBlocksWithBackwardSuccessors(order);
    asyncBreakChecks_.insert(order.front());
  }

  for (int i = 0, e = order.size(); i < e; ++i) {
    BasicBlock *BB = order[i];
    BasicBlock *next = ((i + 1) == e) ? nullptr : order[i + 1];
    LLVM_DEBUG(dbgs() << "Generating bytecode for basic block " << BB << "\n");
    generate(BB, next);
  }

  resolveRelocations();
  resolveExceptionHandlers();
  addDebugSourceLocationInfo(outSourceMap);
  generateJumpTable();
  addDebugLexicalInfo();
  populatePropertyCachingInfo();
  BCFGen_->bytecodeGenerationComplete();
}

uint8_t HBCISel::acquirePropertyReadCacheIndex(unsigned id) {
  const bool reuse = F_->getContext().getOptimizationSettings().reusePropCache;
  // Zero is reserved for indicating no-cache, so cannot be a value in the map.
  uint8_t dummyZero = 0;
  auto &idx = reuse ? propertyReadCacheIndexForId_[id] : dummyZero;
  if (idx) {
    ++NumCachedNodes;
    return idx;
  }

  if (LLVM_UNLIKELY(
          lastPropertyReadCacheIndex_ == std::numeric_limits<uint8_t>::max())) {
    ++NumUncachedNodes;
    return PROPERTY_CACHING_DISABLED;
  }

  ++NumCachedNodes;
  ++NumCacheSlots;
  idx = ++lastPropertyReadCacheIndex_;
  return idx;
}

uint8_t HBCISel::acquirePropertyWriteCacheIndex(unsigned id) {
  const bool reuse = F_->getContext().getOptimizationSettings().reusePropCache;
  // Zero is reserved for indicating no-cache, so cannot be a value in the map.
  uint8_t dummyZero = 0;
  auto &idx = reuse ? propertyWriteCacheIndexForId_[id] : dummyZero;
  if (idx) {
    ++NumCachedNodes;
    return idx;
  }

  if (LLVM_UNLIKELY(
          lastPropertyWriteCacheIndex_ ==
          std::numeric_limits<uint8_t>::max())) {
    ++NumUncachedNodes;
    return PROPERTY_CACHING_DISABLED;
  }

  ++NumCachedNodes;
  ++NumCacheSlots;
  idx = ++lastPropertyWriteCacheIndex_;
  return idx;
}

#undef DEBUG_TYPE
