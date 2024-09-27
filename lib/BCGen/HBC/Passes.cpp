/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes.h"

#include "BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/BCGen/RegAlloc.h"
#include "hermes/IR/IRUtils.h"

#include "llvh/ADT/SetVector.h"

#define DEBUG_TYPE "hbc-backend"

namespace hermes {
namespace hbc {

namespace {
// Get the next instruction(s) after this Instruction. Creates branches as
// necessary.
llvh::SmallVector<Instruction *, 4> getInsertionPointsAfter(
    IRBuilder &builder,
    Instruction *inst) {
  llvh::SmallVector<Instruction *, 4> points;

  if (!llvh::isa<TerminatorInst>(inst)) {
    // Easy case for non-terminators. Just return the next inst.
    auto pos = inst->getIterator();
    pos++;
    points.push_back(&*pos);
    return points;
  }

  for (int i = 0, e = inst->getNumOperands(); i < e; i++) {
    BasicBlock *target = llvh::dyn_cast<BasicBlock>(inst->getOperand(i));
    if (!target)
      continue;

    auto *detour = builder.createBasicBlock(target->getParent());
    builder.setInsertionBlock(detour);
    auto *bounce = builder.createBranchInst(target);
    inst->setOperand(detour, i);
    updateIncomingPhiValues(target, inst->getParent(), detour);
    points.push_back(bounce);
  }
  return points;
}

} // namespace

bool LoadConstants::operandMustBeLiteral(Instruction *Inst, unsigned opIndex) {
  // HBCLoadConstInst is meant to load a constant
  if (llvh::isa<HBCLoadConstInst>(Inst))
    return true;

  // The operand of LoadParamInst is a literal index.
  if (llvh::isa<LoadParamInst>(Inst))
    return true;

  if (llvh::isa<HBCAllocObjectFromBufferInst>(Inst))
    return true;

  // All operands of AllocArrayInst are literals.
  if (llvh::isa<AllocArrayInst>(Inst))
    return true;

  if (llvh::isa<AllocFastArrayInst>(Inst))
    return true;

  // SwitchInst's rest of the operands are case values,
  // hence they will stay as constant.
  if (llvh::isa<SwitchInst>(Inst) && opIndex > 0)
    return true;

  // StoreOwnPropertyInst and StoreNewOwnPropertyInst.
  if (auto *SOP = llvh::dyn_cast<BaseStoreOwnPropertyInst>(Inst)) {
    if (opIndex == StoreOwnPropertyInst::PropertyIdx) {
      if (llvh::isa<StoreNewOwnPropertyInst>(Inst)) {
        // In StoreNewOwnPropertyInst the property name must be a literal.
        return true;
      }

      // If the propery is a LiteralNumber, the property is enumerable, and it
      // is a valid array index, it is coming from an array initialization and
      // we will emit it as PutByIndex.
      if (auto *LN = llvh::dyn_cast<LiteralNumber>(Inst->getOperand(opIndex))) {
        if (SOP->getIsEnumerable() && LN->convertToArrayIndex().hasValue())
          return true;
      }
    }

    // StoreOwnPropertyInst's isEnumerable is a boolean constant.
    if (opIndex == StoreOwnPropertyInst::IsEnumerableIdx)
      return true;

    return false;
  }

  // If StorePropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually.
  if (llvh::isa<BaseStorePropertyInst>(Inst) &&
      opIndex == BaseStorePropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  // If LoadPropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually.
  if (llvh::isa<BaseLoadPropertyInst>(Inst) &&
      opIndex == BaseLoadPropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  // If DeletePropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually.
  if (llvh::isa<DeletePropertyInst>(Inst) &&
      opIndex == DeletePropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  // StoreGetterSetterInst's isEnumerable is a boolean constant.
  if (llvh::isa<StoreGetterSetterInst>(Inst) &&
      opIndex == StoreGetterSetterInst::IsEnumerableIdx)
    return true;

  // Both pattern and flags operands of the CreateRegExpInst
  // are literal strings.
  if (llvh::isa<CreateRegExpInst>(Inst))
    return true;

  if (llvh::isa<SwitchImmInst>(Inst) &&
      (opIndex == SwitchImmInst::MinValueIdx ||
       opIndex == SwitchImmInst::SizeIdx ||
       opIndex >= SwitchImmInst::FirstCaseIdx))
    return true;

  /// CallBuiltin's callee, new.target, "this" should always be literals.
  if (llvh::isa<CallBuiltinInst>(Inst) &&
      (opIndex == CallBuiltinInst::CalleeIdx ||
       opIndex == CallBuiltinInst::NewTargetIdx ||
       opIndex == CallBuiltinInst::ThisIdx))
    return true;

  // CallInst's NewTarget should only be a literal if it's undefined.
  if (auto *CI = llvh::dyn_cast<CallInst>(Inst);
      CI && opIndex == CallInst::NewTargetIdx) {
    return llvh::isa<LiteralUndefined>(CI->getNewTarget());
  }

  // HBCCallNInst does not use its NewTarget operand because it is always
  // undefined.
  if (auto *HCNI = llvh::dyn_cast<HBCCallNInst>(Inst);
      HCNI && opIndex == HBCCallNInst::NewTargetIdx) {
    return true;
  }

  /// GetBuiltinClosureInst's builtin index is always literal.
  if (llvh::isa<GetBuiltinClosureInst>(Inst) &&
      opIndex == GetBuiltinClosureInst::BuiltinIndexIdx)
    return true;

  if (llvh::isa<IteratorCloseInst>(Inst) &&
      opIndex == IteratorCloseInst::IgnoreInnerExceptionIdx) {
    return true;
  }

  if (llvh::isa<DeclareGlobalVarInst>(Inst) &&
      opIndex == DeclareGlobalVarInst::NameIdx) {
    return true;
  }

  if (llvh::isa<DirectEvalInst>(Inst) &&
      opIndex == DirectEvalInst::StrictCallerIdx) {
    return true;
  }

  if (llvh::isa<GetTemplateObjectInst>(Inst) &&
      (opIndex == GetTemplateObjectInst::TemplateObjIDIdx ||
       opIndex == GetTemplateObjectInst::DupIdx)) {
    return true;
  }

  if (llvh::isa<PrLoadInst>(Inst) &&
      (opIndex == PrLoadInst::PropIndexIdx ||
       opIndex == PrLoadInst::PropNameIdx)) {
    return true;
  }
  if (llvh::isa<PrStoreInst>(Inst) &&
      (opIndex == PrStoreInst::PropIndexIdx ||
       opIndex == PrStoreInst::PropNameIdx ||
       opIndex == PrStoreInst::NonPointerIdx)) {
    return true;
  }

  if (llvh::isa<NativeCallInst>(Inst) &&
      (opIndex == NativeCallInst::CalleeIdx ||
       opIndex == NativeCallInst::SignatureIdx)) {
    return true;
  }

  if (llvh::isa<CheckedTypeCastInst>(Inst) &&
      opIndex == CheckedTypeCastInst::SpecifiedTypeIdx) {
    return true;
  }
  if (llvh::isa<ThrowIfInst>(Inst) && opIndex == ThrowIfInst::InvalidTypesIdx) {
    return true;
  }

  if (llvh::isa<HBCResolveParentEnvironmentInst>(Inst) &&
      opIndex == HBCResolveParentEnvironmentInst::NumLevelsIdx) {
    return true;
  }
  if (llvh::isa<LIRResolveScopeInst>(Inst) &&
      opIndex == LIRResolveScopeInst::NumLevelsIdx) {
    return true;
  }

  if (llvh::isa<BaseCallInst>(Inst) &&
      opIndex == BaseCallInst::CalleeIsAlwaysClosure) {
    return true;
  }

  // For properties that uint8_t literals, there's a GetByIndex variant
  // that encodes the property as an immediate.
  if (auto *loadPropInst = llvh::dyn_cast_or_null<LoadPropertyInst>(Inst)) {
    if (opIndex == LoadPropertyInst::PropertyIdx) {
      if (auto *litNum =
              llvh::dyn_cast<LiteralNumber>(loadPropInst->getOperand(1));
          litNum && litNum->isUInt8Representible()) {
        return true;
      }
    }
  }

  if (llvh::isa<EvalCompilationDataInst>(Inst)) {
    return true;
  }

  return false;
}

bool LoadConstants::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  /// Inserts and returns a load instruction for \p literal before \p where.
  auto createLoadLiteral = [&builder](Literal *literal, Instruction *where) {
    builder.setInsertionPoint(where);
    return llvh::isa<GlobalObject>(literal)
        ? cast<Instruction>(builder.createHBCGetGlobalObjectInst())
        : cast<Instruction>(builder.createHBCLoadConstInst(literal));
  };

  for (BasicBlock &BB : *F) {
    for (auto &I : BB) {
      if (auto *phi = llvh::dyn_cast<PhiInst>(&I)) {
        // Since PhiInsts must always be at the start of a basic block, we have
        // to insert the load instruction in the predecessor. This lowering is
        // sub-optimal: for conditional branches, the load constant operation
        // will be performed before the branch decides which path to take.
        for (unsigned i = 0, e = phi->getNumEntries(); i < e; ++i) {
          auto [val, bb] = phi->getEntry(i);
          if (auto *literal = llvh::dyn_cast<Literal>(val)) {
            auto *load = createLoadLiteral(literal, bb->getTerminator());
            phi->updateEntry(i, load, bb);
            changed = true;
          }
        }
        continue;
      }

      // For all other instructions, insert load constants right
      // before the they are needed.  (They are not needed if the
      // corresponding HBC instruction always expects a literal at the
      // given operand position, or if there exists a variant of the
      // HBC instruction that does.)
      //
      // This minimizes their live range and therefore reduces
      // register pressure. CodeMotion and CSE can later hoist and
      // deduplicate them.
      for (unsigned i = 0, e = I.getNumOperands(); i < e; ++i) {
        if (auto *literal = llvh::dyn_cast<Literal>(I.getOperand(i))) {
          if (!operandMustBeLiteral(&I, i)) {
            auto *load = createLoadLiteral(literal, &I);
            I.setOperand(load, i);
            changed = true;
          }
        }
      }
    }
  }
  return changed;
}

CreateArgumentsInst *LowerArgumentsArray::getCreateArgumentsInst(Function *F) {
  // CreateArgumentsInst is always in the first block in normal functions,
  // but is in the second block in unlowered inner generator functions.
  bool gensLowered = F->getParent()->areGeneratorsLowered();
  if (!gensLowered && F->isInnerGenerator()) {
    for (BasicBlock *succ : F->front().getTerminator()->successors()) {
      for (auto &inst : *succ) {
        if (auto *target = llvh::dyn_cast<CreateArgumentsInst>(&inst)) {
          return target;
        }
      }
    }
  } else {
    for (auto &inst : F->front()) {
      if (auto *target = llvh::dyn_cast<CreateArgumentsInst>(&inst)) {
        return target;
      }
    }
  }
  return nullptr;
}

bool LowerArgumentsArray::runOnFunction(Function *F) {
  IRBuilder builder(F);
  movePastFirstInBlock(builder, &*F->begin());

  CreateArgumentsInst *createArguments = getCreateArgumentsInst(F);
  if (!createArguments) {
    return false;
  }
  bool isStrict = llvh::isa<CreateArgumentsStrictInst>(createArguments);

  builder.setInsertionPoint(createArguments);
  AllocStackInst *lazyReg = builder.createAllocStackInst(
      "arguments",
      Type::unionTy(Type::createObject(), Type::createUndefined()));
  builder.createStoreStackInst(builder.getLiteralUndefined(), lazyReg);

  // Process all LoadPropertyInst's first because they may add another user
  // to the list of users of createArguments.
  // Specifically the case when `arguments[arguments]` is accessed.
  // Note that in such a case, a single LoadPropertyInst will appear twice in
  // the use list. Use a set so we only remove it once.
  llvh::SmallSetVector<Instruction *, 16> uniqueUsers;
  uniqueUsers.insert(
      createArguments->getUsers().begin(), createArguments->getUsers().end());
  for (Value *user : uniqueUsers) {
    auto *load = llvh::dyn_cast<BaseLoadPropertyInst>(user);
    if (load && load->getObject() == createArguments) {
      builder.setInsertionPoint(load);
      builder.setLocation(load->getLocation());
      auto *propertyString = llvh::dyn_cast<LiteralString>(load->getProperty());
      if (propertyString && propertyString->getValue().str() == "length") {
        // For `arguments.length`, get the length.
        auto *length = builder.createHBCGetArgumentsLengthInst(
            builder.createLoadStackInst(lazyReg));
        load->replaceAllUsesWith(length);
        load->eraseFromParent();
      } else {
        // For all other property loads, get by index.
        HBCGetArgumentsPropByValInst *get;
        if (isStrict)
          get = builder.createHBCGetArgumentsPropByValStrictInst(
              load->getProperty(), lazyReg);
        else
          get = builder.createHBCGetArgumentsPropByValLooseInst(
              load->getProperty(), lazyReg);
        load->replaceAllUsesWith(get);
        load->eraseFromParent();
      }
    }
  }

  uniqueUsers.clear();
  uniqueUsers.insert(
      createArguments->getUsers().begin(), createArguments->getUsers().end());
  for (Instruction *user : uniqueUsers) {
    if (auto *phi = llvh::dyn_cast<PhiInst>(user)) {
      // We have to insert another branch where we can reify the value.
      for (int i = 0, n = phi->getNumEntries(); i < n; i++) {
        auto entry = phi->getEntry(i);
        if (entry.first != createArguments)
          continue;

        auto *previousBlock = cast<BasicBlock>(entry.second);
        auto *thisBlock = phi->getParent();

        auto *newBlock = builder.createBasicBlock(F);
        builder.setInsertionBlock(newBlock);
        if (isStrict)
          builder.createHBCReifyArgumentsStrictInst(lazyReg);
        else
          builder.createHBCReifyArgumentsLooseInst(lazyReg);
        auto *reifiedValue = builder.createUnionNarrowTrustedInst(
            builder.createLoadStackInst(lazyReg), Type::createObject());
        builder.createBranchInst(thisBlock);

        phi->updateEntry(i, reifiedValue, newBlock);
        // Update all other PHI nodes in thisBlock that currently reference
        // previousBlock so they instead reference newBlock.
        updateIncomingPhiValues(thisBlock, previousBlock, newBlock);

        auto *branch = previousBlock->getTerminator();
        for (int j = 0, m = branch->getNumOperands(); j < m; j++)
          if (branch->getOperand(j) == thisBlock)
            branch->setOperand(newBlock, j);
      }
    } else {
      // For other users, insert a reification so we can replace
      // the usage with this array.
      builder.setInsertionPoint(user);
      builder.setLocation(user->getLocation());
      if (isStrict)
        builder.createHBCReifyArgumentsStrictInst(lazyReg);
      else
        builder.createHBCReifyArgumentsLooseInst(lazyReg);
      auto *array = builder.createUnionNarrowTrustedInst(
          builder.createLoadStackInst(lazyReg), Type::createObject());
      for (int i = 0, n = user->getNumOperands(); i < n; i++) {
        if (user->getOperand(i) == createArguments) {
          user->setOperand(array, i);
        }
      }
    }
  }

  createArguments->eraseFromParent();
  return true;
}

bool DedupReifyArguments::runOnFunction(Function *F) {
  bool changed = false;

  // Check if there are any HBCReifyArgumentsInst in the function before
  // calculating dominator tree and reverse post order to save compile time.
  bool hasRAI = false;
  for (auto &BB : *F) {
    for (auto &inst : BB.getInstList()) {
      if (llvh::isa<HBCReifyArgumentsInst>(&inst)) {
        hasRAI = true;
        break;
      }
    }
    if (hasRAI)
      break;
  }

  if (!hasRAI)
    return false;

  DominanceInfo domInfo{F};
  auto PO = postOrderAnalysis(F);
  IRBuilder::InstructionDestroyer destroyer;

  llvh::SmallVector<BasicBlock *, 16> reversePO(PO.rbegin(), PO.rend());
  llvh::SmallVector<HBCReifyArgumentsInst *, 4> reifications;

  for (auto *BB : reversePO) {
    for (auto &inst : BB->getInstList()) {
      if (auto *reify = llvh::dyn_cast<HBCReifyArgumentsInst>(&inst)) {
        HBCReifyArgumentsInst *dominator = nullptr;
        for (auto *possibleParent : reifications) {
          if (domInfo.properlyDominates(possibleParent, reify)) {
            dominator = possibleParent;
            break;
          }
        }

        if (dominator) {
          destroyer.add(reify);
          changed = true;
        } else {
          reifications.push_back(reify);
        }
      }
    }
  }
  return changed;
}

bool InitCallFrame::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  for (auto &BB : *F) {
    for (auto &I : BB) {
      auto *call = llvh::dyn_cast<BaseCallInst>(&I);
      // This also matches constructors.
      if (!call)
        continue;
      builder.setInsertionPoint(call);
      changed = true;

      auto reg = RA_.getLastRegister().getIndex() -
          HVMRegisterAllocator::CALL_EXTRA_REGISTERS;

      for (int i = 0, e = call->getNumArguments(); i < e; i++, --reg) {
        // If this is a Call instruction, emit explicit Movs to the argument
        // registers. If this is a CallN instruction, emit ImplicitMovs
        // instead, to express that these registers get written to by the CallN,
        // even though they are not the destination.
        // Lastly, if this is argument 0 of CallBuiltinInst emit ImplicitMov to
        // encode that the "this" register is implicitly set to undefined.
        Value *arg = call->getArgument(i);
        if (llvh::isa<HBCCallNInst>(call) ||
            (i == 0 && llvh::isa<CallBuiltinInst>(call))) {
          auto *imov = builder.createImplicitMovInst(arg);
          RA_.updateRegister(imov, Register(reg));
        } else {
          auto *mov = builder.createMovInst(arg);
          RA_.updateRegister(mov, Register(reg));
          call->setArgument(mov, i);
        }
      }
    }
  }
  return changed;
}

bool RecreateCheapValues::runOnFunction(Function *F) {
  IRBuilder builder(F);
  llvh::SmallPtrSet<Instruction *, 4> potentiallyUnused;
  bool changed = false;

  for (auto &BB : *F) {
    IRBuilder::InstructionDestroyer destroyer;
    for (auto &I : BB) {
      auto *mov = llvh::dyn_cast<MovInst>(&I);
      if (!mov)
        continue;
      auto *load = llvh::dyn_cast<HBCLoadConstInst>(mov->getSingleOperand());
      if (!load)
        continue;
      Literal *literal = load->getConst();

      switch (literal->getKind()) {
        case ValueKind::LiteralUndefinedKind:
        case ValueKind::LiteralNullKind:
        case ValueKind::LiteralBoolKind:
          break;
        case ValueKind::LiteralNumberKind:
          if (cast<LiteralNumber>(literal)->isPositiveZero()) {
            break;
          }
          continue;
        default:
          continue;
      }

      builder.setInsertionPoint(mov);
      auto *recreation = builder.createHBCLoadConstInst(literal);
      RA_.updateRegister(recreation, RA_.getRegister(mov));
      mov->replaceAllUsesWith(recreation);
      destroyer.add(mov);
      potentiallyUnused.insert(load);
      changed = true;
    }
  }

  {
    IRBuilder::InstructionDestroyer destroyer;
    for (auto *inst : potentiallyUnused) {
      if (!inst->hasUsers()) {
        destroyer.add(inst);
      }
    }
  }
  return changed;
}

bool LoadConstantValueNumbering::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  for (auto &BB : *F) {
    IRBuilder::InstructionDestroyer destroyer;
    // Maps a register number to the instruction that last modified it.
    // Every Instruction is either an HBCLoadConstInst or a Mov whose
    // operand is a HBCLoadConstInst
    llvh::DenseMap<unsigned, Instruction *> regToInstMap{};
    for (auto &I : BB) {
      HBCLoadConstInst *loadI{nullptr};
      // Value numbering currently only tracks the values of registers that
      // have a constant in them, or that have had a constant moved in them.
      if (!(loadI = llvh::dyn_cast<HBCLoadConstInst>(&I))) {
        if (auto *movI = llvh::dyn_cast<MovInst>(&I)) {
          loadI = llvh::dyn_cast<HBCLoadConstInst>(movI->getSingleOperand());
        }
      }

      if (RA_.isAllocated(&I)) {
        unsigned reg = RA_.getRegister(&I).getIndex();
        if (loadI) {
          auto it = regToInstMap.find(reg);
          if (it != regToInstMap.end()) {
            auto prevI = it->second;
            HBCLoadConstInst *prevLoad{nullptr};
            // If the key is found, the instruction must be either an
            // HBCLoadConstInst, or a Mov whose operand is an HBCLoadConstInst.
            if (!(prevLoad = llvh::dyn_cast<HBCLoadConstInst>(prevI))) {
              prevLoad = llvh::dyn_cast<HBCLoadConstInst>(prevI->getOperand(0));
            }
            if (prevLoad->isIdenticalTo(loadI)) {
              I.replaceAllUsesWith(prevI);
              destroyer.add(&I);
              changed = true;
              continue;
            }
          }
          regToInstMap[reg] = &I;
          continue;
        }
        regToInstMap.erase(reg);
      }

      // If the instruction has writable stack operands, invalidate them. Note
      // that read-only stack operands are prohibited (they should go through
      // LoadStackInst), so all stack operands must be writable.
      if (I.getSideEffect().getWriteStack()) {
        for (size_t i = 0, e = I.getNumOperands(); i < e; ++i) {
          if (auto *operand = llvh::dyn_cast<AllocStackInst>(I.getOperand(i))) {
            unsigned reg =
                RA_.getRegister(cast<Instruction>(operand)).getIndex();
            regToInstMap.erase(reg);
          }
        }
      }
    }
  }
  return changed;
}

bool SpillRegisters::requiresShortOutput(Instruction *I) {
  if (llvh::isa<TerminatorInst>(I)) {
    // None of our terminators produce results at all
    // (though GetNextPNameInst modifies operands).
    return false;
  }

  // Instructions that produce no output, don't use the register, even when
  // allocated.
  if (!I->hasOutput())
    return false;

  switch (I->getKind()) {
    // Some instructions become Movs or other opcodes with long variants:
    case ValueKind::HBCSpillMovInstKind:
    case ValueKind::LoadStackInstKind:
    case ValueKind::MovInstKind:
    case ValueKind::PhiInstKind:
    // Some instructions aren't actually encoded at all:
    case ValueKind::AllocStackInstKind:
    case ValueKind::TryEndInstKind:
    case ValueKind::TryStartInstKind:
      return false;
    default:
      return true;
  }
}

bool SpillRegisters::requiresShortOperand(Instruction *I, int op) {
  switch (I->getKind()) {
    case ValueKind::PhiInstKind:
    case ValueKind::MovInstKind:
    case ValueKind::HBCSpillMovInstKind:
    case ValueKind::LoadStackInstKind:
    case ValueKind::StoreStackInstKind:
      return false;
    // For all the call instructions, ensure that the *arguments* are not moved
    // around, because they are going to be placed in the stack and not directly
    // emitted in the bytecode.
    case ValueKind::CallInstKind:
    case ValueKind::CallBuiltinInstKind:
      return op == CallInst::CalleeIdx || op == CallInst::NewTargetIdx;
    case ValueKind::HBCCallWithArgCountInstKind:
      return op == CallInst::CalleeIdx || op == CallInst::NewTargetIdx ||
          op == HBCCallWithArgCountInst::NumArgLiteralIdx;
    default:
      return true;
  }
}

bool SpillRegisters::modifiesOperandRegister(Instruction *I, int op) {
  // Check if the operand is a stack location that this instruction may write
  // to. Note that read-only stack operands are prohibited (they should go
  // through LoadStackInst), so any stack operands may be writable.
  return I->getSideEffect().getWriteStack() &&
      llvh::isa<AllocStackInst>(I->getOperand(op));
}

bool SpillRegisters::runOnFunction(Function *F) {
  if (RA_.getMaxRegisterUsage() < boundary_) {
    return false;
  }
  reserveLowRegisters(F);

  IRBuilder builder(F);
  llvh::SmallVector<std::pair<Instruction *, Register>, 2> toSpill;

  for (BasicBlock &BB : F->getBasicBlockList()) {
    for (Instruction &inst : BB) {
      if (!RA_.isAllocated(&inst)) {
        // This instruction is dead. Don't bother spilling.
        continue;
      }

      int tempReg = 0;
      toSpill.clear();
      bool replaceWithFirstSpill = false;
      builder.setLocation(inst.getLocation());

      auto myRegister = RA_.getRegister(&inst);
      if (requiresShortOutput(&inst) && !isShort(myRegister)) {
        auto temp = getReserved(tempReg++);
        RA_.updateRegister(&inst, temp);
        toSpill.push_back(
            std::pair<Instruction *, Register>(&inst, myRegister));
        replaceWithFirstSpill = true;
      }

      for (int i = 0, e = inst.getNumOperands(); i < e; i++) {
        auto *op = llvh::dyn_cast<Instruction>(inst.getOperand(i));
        if (!op || !RA_.isAllocated(op)) {
          // This is either not an instruction, or a dead instruction.
          // Either way, we don't have to do anything.
          continue;
        }
        auto opRegister = RA_.getRegister(op);

        if (requiresShortOperand(&inst, i) && !isShort(opRegister)) {
          // The check for if an instruction modifies an operand depends on the
          // kind of the instruction the operand is. So, we need to compute this
          // value before we replace the operand.
          bool modifiesOp = modifiesOperandRegister(&inst, i);
          auto temp = getReserved(tempReg++);

          builder.setInsertionPoint(&inst);
          auto *load = builder.createHBCSpillMovInst(op);
          RA_.updateRegister(load, temp);
          inst.setOperand(load, i);

          if (modifiesOp) {
            toSpill.push_back(
                std::pair<Instruction *, Register>(load, opRegister));
          }
        }
      }

      if (toSpill.size()) {
        auto spillPoints = getInsertionPointsAfter(builder, &inst);

        assert(
            (!replaceWithFirstSpill || spillPoints.size() <= 1) &&
            "Asked to spill the value of a TerminatorInst. Our terminators "
            "shouldn't produce values. It wouldn't have mattered, but it "
            "also has multiple branches so users might need PhiInsts.");

        for (auto *point : spillPoints) {
          builder.setInsertionPoint(point);
          for (auto store : toSpill) {
            auto *storeInst = builder.createHBCSpillMovInst(store.first);
            RA_.updateRegister(storeInst, store.second);

            if (!replaceWithFirstSpill)
              continue;
            // Replace all uses of the inst with the spilling inst
            inst.replaceAllUsesWith(storeInst);
            // Except for the actual spill of course
            storeInst->setOperand(&inst, 0);
            // Disable now that the job is done.
            replaceWithFirstSpill = false;
          }
        }
      }
    }
  }
  return true;
}

bool LowerSwitchIntoJumpTables::runOnFunction(Function *F) {
  bool changed = false;
  llvh::SmallVector<SwitchInst *, 4> switches;
  // Collect all switch instructions.
  for (BasicBlock &BB : *F)
    for (auto &it : BB) {
      if (auto *S = llvh::dyn_cast<SwitchInst>(&it))
        switches.push_back(S);
    }

  for (auto *S : switches) {
    if (lowerIntoJumpTable(S))
      changed = true;
  }

  return changed;
}

bool LowerSwitchIntoJumpTables::lowerIntoJumpTable(SwitchInst *switchInst) {
  // if its a constant switch don't bother
  if (llvh::isa<Literal>(switchInst->getInputValue())) {
    return false;
  }
  IRBuilder builder(switchInst->getParent()->getParent());
  unsigned numCases = switchInst->getNumCasePair();
  uint32_t minValue = 0;
  uint32_t maxValue = 0;

  llvh::SmallVector<LiteralNumber *, 8> values;
  llvh::SmallVector<BasicBlock *, 8> blocks;

  for (unsigned i = 0; i != numCases; ++i) {
    auto casePair = switchInst->getCasePair(i);
    auto *lit = casePair.first;
    auto *num = llvh::dyn_cast<LiteralNumber>(lit);
    if (!num)
      return false;

    // Check whether it is representable as uint32.
    if (auto ival = num->isIntTypeRepresentible<uint32_t>()) {
      values.push_back(num);
      blocks.push_back(casePair.second);

      if (i == 0) {
        minValue = maxValue = ival.getValue();
      } else {
        minValue = std::min(minValue, ival.getValue());
        maxValue = std::max(maxValue, ival.getValue());
      }
    } else {
      return false;
    }
  }

  assert(minValue <= maxValue && "Minimum cannot exceed maximum");
  uint32_t range = maxValue - minValue;
  // We can't generate a table for a zero-sized range.
  if (range == 0)
    return false;

  // The number of cases is range + 1, which must fit in a uint32.
  if (range == std::numeric_limits<uint32_t>::max())
    return false;

  // Check the "denseness" of the cases.
  // Don't convert small switches.
  if (range / numCases > 5 || numCases < 10)
    return false;

  builder.setInsertionPoint(switchInst);
  auto *switchImmInst = builder.createSwitchImmInst(
      switchInst->getInputValue(),
      switchInst->getDefaultDestination(),
      builder.getLiteralNumber(minValue),
      builder.getLiteralNumber(range + 1),
      values,
      blocks);

  switchInst->replaceAllUsesWith(switchImmInst);
  switchInst->eraseFromParent();
  return true;
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
