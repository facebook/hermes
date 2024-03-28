/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeGenerator.h"

#include "ISel.h"
#include "LoweringPipelines.h"
#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/BCGen/LowerBuiltinCalls.h"
#include "hermes/BCGen/LowerStoreInstrs.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/Analysis.h"
#include "hermes/Optimizer/PassManager/PassManager.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/Format.h"

#include <unordered_map>

namespace hermes {
namespace hbc {

// If we have less than this number of instructions in a Function, and we're
// not compiling in optimized mode, take shortcuts during register allocation.
// 250 was chosen so that registers will fit in a single byte even after some
// have been reserved for function parameters.
const unsigned kFastRegisterAllocationThreshold = 250;

// If register allocation is expected to take more than this number of bytes of
// RAM, and we're not trying to optimize, use a simpler pass to reduce compile
// time memory usage.
const uint64_t kRegisterAllocationMemoryLimit = 10L * 1024 * 1024;

unsigned BytecodeFunctionGenerator::getStringID(LiteralString *value) const {
  return BMGen_.getStringID(value->getValue().str());
}

unsigned BytecodeFunctionGenerator::getIdentifierID(
    LiteralString *value) const {
  return BMGen_.getIdentifierID(value->getValue().str());
}

uint32_t BytecodeFunctionGenerator::addBigInt(bigint::ParsedBigInt bigint) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addBigInt(std::move(bigint));
}

uint32_t BytecodeFunctionGenerator::addRegExp(CompiledRegExp *regexp) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addRegExp(regexp);
}

uint32_t BytecodeFunctionGenerator::addFilename(llvh::StringRef filename) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addFilename(filename);
}

void BytecodeFunctionGenerator::addExceptionHandler(
    HBCExceptionHandlerInfo info) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  exceptionHandlers_.push_back(info);
}

void BytecodeFunctionGenerator::addDebugSourceLocation(
    const DebugSourceLocation &info) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  // If an address is repeated, it means no actual bytecode was emitted for the
  // previous source location.
  if (!debugLocations_.empty() &&
      debugLocations_.back().address == info.address) {
    debugLocations_.back() = info;
  } else {
    debugLocations_.push_back(info);
  }
}

void BytecodeFunctionGenerator::setJumpTable(
    std::vector<uint32_t> &&jumpTable) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  assert(!jumpTable.empty() && "invoked with no jump table");

  jumpTable_ = std::move(jumpTable);
}

std::unique_ptr<BytecodeFunction>
BytecodeFunctionGenerator::generateBytecodeFunction(
    Function::ProhibitInvoke prohibitInvoke,
    bool strictMode,
    uint32_t paramCount,
    uint32_t nameID) {
  if (!complete_) {
    bytecodeGenerationComplete();
  }

  FunctionHeader header{
      bytecodeSize_,
      paramCount,
      frameSize_,
      nameID,
      highestReadCacheIndex_,
      highestWriteCacheIndex_};

  switch (prohibitInvoke) {
    case Function::ProhibitInvoke::ProhibitNone:
      header.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitNone;
      break;
    case Function::ProhibitInvoke::ProhibitConstruct:
      header.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitConstruct;
      break;
    case Function::ProhibitInvoke::ProhibitCall:
      header.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitCall;
      break;
  }

  header.flags.strictMode = strictMode;
  header.flags.hasExceptionHandler = exceptionHandlers_.size();

  return std::make_unique<BytecodeFunction>(
      std::move(opcodes_), std::move(header), std::move(exceptionHandlers_));
}

unsigned BytecodeFunctionGenerator::getFunctionID(Function *F) {
  return BMGen_.addFunction(F);
}

void BytecodeFunctionGenerator::shrinkJump(offset_t loc) {
  // We are shrinking a long jump into a short jump.
  // The size of operand reduces from 4 bytes to 1 byte, a delta of 3.
  opcodes_.erase(opcodes_.begin() + loc, opcodes_.begin() + loc + 3);

  // Change this instruction from long jump to short jump.
  longToShortJump(loc - 1);
}

void BytecodeFunctionGenerator::updateJumpTarget(
    offset_t loc,
    int newVal,
    int bytes) {
  // The jump target is encoded in little-endian. Update it correctly
  // regardless of host byte order.
  for (; bytes; --bytes, ++loc) {
    opcodes_[loc] = (opcode_atom_t)(newVal);
    newVal >>= 8;
  }
}

void BytecodeFunctionGenerator::updateJumpTableOffset(
    offset_t loc,
    uint32_t jumpTableOffset,
    uint32_t instLoc) {
  assert(opcodes_.size() > instLoc && "invalid switchimm offset");

  // The offset is not aligned, but will be aligned when read in the
  // interpreter.
  updateJumpTarget(
      loc,
      opcodes_.size() + jumpTableOffset * sizeof(uint32_t) - instLoc,
      sizeof(uint32_t));
}

void BytecodeFunctionGenerator::bytecodeGenerationComplete() {
  assert(!complete_ && "Can only call bytecodeGenerationComplete once");
  complete_ = true;
  bytecodeSize_ = opcodes_.size();

  // Add the jump tables inline with the opcodes, as a 4-byte aligned section at
  // the end of the opcode array.
  if (!jumpTable_.empty()) {
    uint32_t alignedOpcodes = llvh::alignTo<sizeof(uint32_t)>(bytecodeSize_);
    uint32_t jumpTableBytes = jumpTable_.size() * sizeof(uint32_t);
    opcodes_.reserve(alignedOpcodes + jumpTableBytes);
    opcodes_.resize(alignedOpcodes, 0);
    const opcode_atom_t *jumpTableStart =
        reinterpret_cast<opcode_atom_t *>(jumpTable_.data());
    opcodes_.insert(
        opcodes_.end(), jumpTableStart, jumpTableStart + jumpTableBytes);
  }
}

unsigned BytecodeModuleGenerator::addFunction(Function *F) {
  auto [it, inserted] = functionIDMap_.insert({F, bm_->getNumFunctions()});
  if (inserted) {
    bm_->addFunction();
    bm_->getBytecodeOptionsMut().hasAsync |= llvh::isa<AsyncFunction>(F);
  }
  return it->second;
}

unsigned BytecodeModuleGenerator::getStringID(llvh::StringRef str) const {
  return bm_->getStringID(str);
}

unsigned BytecodeModuleGenerator::getIdentifierID(llvh::StringRef str) const {
  return bm_->getIdentifierID(str);
}

void BytecodeModuleGenerator::initializeStringTable(
    StringLiteralTable stringTable) {
  bm_->initializeStringTable(std::move(stringTable));
}

uint32_t BytecodeModuleGenerator::addBigInt(bigint::ParsedBigInt bigint) {
  return bm_->addBigInt(std::move(bigint));
}

void BytecodeModuleGenerator::initializeSerializedLiterals(
    LiteralBufferBuilder::Result &&bufs) {
  assert(
      bm_->getArrayBuffer().empty() && bm_->getObjectBuffer().first.empty() &&
      bm_->getObjectBuffer().second.empty() && literalOffsetMap_.empty() &&
      "serialized literals already initialized");
  bm_->initializeSerializedLiterals(
      std::move(bufs.arrayBuffer),
      std::move(bufs.keyBuffer),
      std::move(bufs.valBuffer));
  literalOffsetMap_ = std::move(bufs.offsetMap);
}

uint32_t BytecodeModuleGenerator::addRegExp(CompiledRegExp *regexp) {
  return bm_->addRegExp(regexp);
}

uint32_t BytecodeModuleGenerator::addFilename(llvh::StringRef filename) {
  return debugInfoGenerator_.addFilename(filename);
}

void BytecodeModuleGenerator::addCJSModule(
    uint32_t functionID,
    uint32_t nameID) {
  bm_->addCJSModule(functionID, nameID);
}

void BytecodeModuleGenerator::addCJSModuleStatic(
    uint32_t moduleID,
    uint32_t functionID) {
  bm_->addCJSModuleStatic(moduleID, functionID);
}

void BytecodeModuleGenerator::addFunctionSource(
    uint32_t functionID,
    uint32_t stringID) {
  bm_->addFunctionSource(functionID, stringID);
}

std::unique_ptr<BytecodeModule> BytecodeModuleGenerator::generate() && {
  assert(
      valid_ &&
      "BytecodeModuleGenerator::generate() cannot be called more than once");
  valid_ = false;

  BytecodeOptions &bytecodeOptions = bm_->getBytecodeOptionsMut();
  bytecodeOptions.cjsModulesStaticallyResolved =
      !bm_->getCJSModuleTableStatic().empty();

  // Allow reusing the debug cache between functions
  FileAndSourceMapIdCache debugCache{};

  const uint32_t strippedFunctionNameId =
      options_.stripFunctionNames ? bm_->getStringID(kStrippedFunctionName) : 0;
  for (auto [F, functionID] : functionIDMap_) {
    if (F->isLazy()) {
      hermes_fatal("lazy compilation not supported");
    }

    // Run register allocation.
    HVMRegisterAllocator RA(F);
    if (!options_.optimizationEnabled) {
      RA.setFastPassThreshold(kFastRegisterAllocationThreshold);
      RA.setMemoryLimit(kRegisterAllocationMemoryLimit);
    }
    auto PO = postOrderAnalysis(F);
    /// The order of the blocks is reverse-post-order, which is a simply
    /// topological sort.
    llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
    RA.allocate(order);

    if (options_.format == DumpRA)
      RA.dump();

    lowerAllocatedFunctionIR(F, RA, options_);

    if (options_.format == DumpLRA)
      RA.dump();

    uint32_t functionNameId = options_.stripFunctionNames
        ? strippedFunctionNameId
        : bm_->getStringID(F->getOriginalOrInferredName().str());

    // Use the register allocated IR to make a BytecodeFunctionGenerator and
    // run ISel.
    std::unique_ptr<BytecodeFunctionGenerator> funcGen =
        BytecodeFunctionGenerator::create(*this, RA.getMaxRegisterUsage());
    runHBCISel(F, &*funcGen, RA, options_, debugCache, sourceMapGen_);

    if (funcGen->hasEncodingError()) {
      F->getParent()->getContext().getSourceErrorManager().error(
          F->getSourceRange().Start, "Error encoding bytecode");
      return nullptr;
    }

    std::unique_ptr<BytecodeFunction> func = funcGen->generateBytecodeFunction(
        F->getProhibitInvoke(),
        F->isStrictMode(),
        F->getExpectedParamCountIncludingThis(),
        functionNameId);

    if (funcGen->hasDebugInfo()) {
      uint32_t sourceLocOffset = debugInfoGenerator_.appendSourceLocations(
          funcGen->getSourceLocation(),
          functionID,
          funcGen->getDebugLocations());
      uint32_t lexicalDataOffset = debugInfoGenerator_.appendLexicalData(
          funcGen->getLexicalParentID(), funcGen->getDebugVariableNames());
      func->setDebugOffsets({sourceLocOffset, lexicalDataOffset});
    }
    bm_->setFunction(functionID, std::move(func));
  }

  bm_->setDebugInfo(debugInfoGenerator_.serializeWithMove());
  return std::move(bm_);
}

} // namespace hbc
} // namespace hermes
