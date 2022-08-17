/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes.h"

#include "hermes/BCGen/BCOpt.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/ISel.h"
#include "hermes/BCGen/Lowering.h"

#include "llvh/ADT/SetVector.h"

#define DEBUG_TYPE "hbc-backend"

namespace hermes {
namespace hbc {

namespace {
/// In blockToFix, change all incoming Phi values from previousBlock to instead
/// come from newBlock.
void updateIncomingPhiValues(
    BasicBlock *blockToFix,
    BasicBlock *previousBlock,
    BasicBlock *newBlock) {
  for (auto &inst : *blockToFix) {
    auto *phi = llvh::dyn_cast<PhiInst>(&inst);
    if (!phi)
      return;

    for (int i = 0, e = phi->getNumEntries(); i < e; i++) {
      auto entry = phi->getEntry(i);
      if (entry.second == previousBlock) {
        phi->updateEntry(i, entry.first, newBlock);
      }
    }
  }
}
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

/// Update the insertion point of the builder to the "entry insertion point"
/// of the function, which is where we insert new lowered instructions that must
/// execute on entry.
void updateToEntryInsertionPoint(IRBuilder &builder, Function *F) {
  auto &BB = F->front();
  auto it = BB.begin();
  auto end = BB.end();
  // Skip all HBCCreateEnvironmentInst.
  while (it != end && llvh::isa<HBCCreateEnvironmentInst>(*it))
    ++it;

  builder.setInsertionPoint(&*it);
}

} // namespace

bool LoadConstants::operandMustBeLiteral(Instruction *Inst, unsigned opIndex) {
  // HBCLoadConstInst is meant to load a constant
  if (llvh::isa<HBCLoadConstInst>(Inst))
    return true;

  // The operand of HBCLoadParamInst is a literal index.
  if (llvh::isa<HBCLoadParamInst>(Inst))
    return true;

  if (llvh::isa<HBCAllocObjectFromBufferInst>(Inst))
    return true;

  // All operands of AllocArrayInst are literals.
  if (llvh::isa<AllocArrayInst>(Inst))
    return true;

  if (llvh::isa<AllocObjectInst>(Inst)) {
    // The AllocObjectInst::SizeIdx is a literal.
    if (opIndex == AllocObjectInst::SizeIdx)
      return true;
    // AllocObjectInst::ParentObjectIdx is a literal if it is the EmptySentinel.
    if (opIndex == AllocObjectInst::ParentObjectIdx &&
        llvh::isa<EmptySentinel>(Inst->getOperand(opIndex)))
      return true;

    return false;
  }

  // SwitchInst's rest of the operands are case values,
  // hence they will stay as constant.
  if (llvh::isa<SwitchInst>(Inst) && opIndex > 0)
    return true;

  // StoreOwnPropertyInst and StoreNewOwnPropertyInst.
  if (auto *SOP = llvh::dyn_cast<StoreOwnPropertyInst>(Inst)) {
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
  if (llvh::isa<StorePropertyInst>(Inst) &&
      opIndex == StorePropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  // If LoadPropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually.
  if (llvh::isa<LoadPropertyInst>(Inst) &&
      opIndex == LoadPropertyInst::PropertyIdx &&
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

  /// CallBuiltin's callee and "this" should always be literals.
  if (llvh::isa<CallBuiltinInst>(Inst) &&
      (opIndex == CallBuiltinInst::CalleeIdx ||
       opIndex == CallBuiltinInst::ThisIdx))
    return true;

  /// GetBuiltinClosureInst's builtin index is always literal.
  if (llvh::isa<GetBuiltinClosureInst>(Inst) &&
      opIndex == GetBuiltinClosureInst::BuiltinIndexIdx)
    return true;

#ifdef HERMES_RUN_WASM
  /// CallIntrinsic's IntrinsicIndexIdx should always be literals.
  if (llvh::isa<CallIntrinsicInst>(Inst) &&
      (opIndex == CallIntrinsicInst::IntrinsicIndexIdx))
    return true;
#endif

  if (llvh::isa<IteratorCloseInst>(Inst) &&
      opIndex == IteratorCloseInst::IgnoreInnerExceptionIdx) {
    return true;
  }

  return false;
}

bool LoadConstants::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  llvh::SmallDenseMap<Literal *, Instruction *, 8> constMap{};

  // This is a bit counter-intuitive because the logic appears reversed.
  // We only want to unique the generated literals if optimization is disabled.
  // That is the case when it causes too many registers to be generated (one
  // per literal).
  // If optimization is enabled, that is not necessary because of CSE and
  // because doing this now interefers with code motion.
  const bool uniqueLiterals = !optimizationEnabled_;

  auto createLoadLiteral = [&builder](Literal *literal) -> Instruction * {
    return llvh::isa<GlobalObject>(literal)
        ? cast<Instruction>(builder.createHBCGetGlobalObjectInst())
        : cast<Instruction>(builder.createHBCLoadConstInst(literal));
  };

  updateToEntryInsertionPoint(builder, F);

  for (BasicBlock &bbit : F->getBasicBlockList()) {
    for (auto &it : bbit.getInstList()) {
      for (unsigned i = 0, n = it.getNumOperands(); i < n; i++) {
        if (operandMustBeLiteral(&it, i))
          continue;

        auto *operand = llvh::dyn_cast<Literal>(it.getOperand(i));
        if (!operand)
          continue;

        Instruction *load = nullptr;
        if (uniqueLiterals) {
          auto &entry = constMap[operand];
          if (!entry)
            entry = createLoadLiteral(operand);
          load = entry;
        } else {
          load = createLoadLiteral(operand);
        }

        it.setOperand(load, i);
        changed = true;
      }
    }
  }
  return changed;
}

bool LoadParameters::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  updateToEntryInsertionPoint(builder, F);

  // Index of 0 is the "this" parameter.
  unsigned index = 1;
  for (Parameter *p : F->getParameters()) {
    auto *load =
        builder.createHBCLoadParamInst(builder.getLiteralNumber(index));
    p->replaceAllUsesWith(load);
    index++;
    changed = true;
  }

  // Lower accesses to "this".
  auto *thisParam = F->getThisParameter();
  if (thisParam && thisParam->hasUsers()) {
    // In strict mode just use param 0 directly. In non-strict, we must coerce
    // it to an object.
    Value *getThisInst = F->isStrictMode()
        ? cast<Value>(
              builder.createHBCLoadParamInst(builder.getLiteralNumber(0)))
        : cast<Value>(builder.createHBCGetThisNSInst());
    thisParam->replaceAllUsesWith(getThisInst);
    changed = true;
  }
  return changed;
}

Instruction *LowerLoadStoreFrameInst::getScope(
    IRBuilder &builder,
    Variable *var,
    HBCCreateEnvironmentInst *captureScope) {
  if (var->getParent()->getFunction() != builder.getFunction()) {
    // If the variable is neither from the current scope,
    // we should get the proper scope for it.
    return builder.createHBCResolveEnvironment(var->getParent());
  } else {
    // Now we know that the variable belongs to the current scope.
    // We are going to conservatively assume the variable might get
    // captured. Hence we use the newly created scope.
    // This will not cause performance issue as long as optimization
    // is enabled, because every variable will be moved to stack
    // if not being captured.
    return captureScope;
  }
}

bool LowerLoadStoreFrameInst::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  updateToEntryInsertionPoint(builder, F);

  // All local captured variables will be stored in this scope (or
  // "environment").
  // It will also be used by all closures created in this function, even if
  // there are no captured variables in this function.
  // Closures need a new environment even without captured variables because
  // we currently use only the lexical nesting level to determine which parent
  // environment to use - we don't account for the case when an environment may
  // not be needed somewhere along the chain.
  HBCCreateEnvironmentInst *captureScope =
      builder.createHBCCreateEnvironmentInst();

  for (BasicBlock &BB : F->getBasicBlockList()) {
    for (auto I = BB.begin(), E = BB.end(); I != E; /* nothing */) {
      // Keep the reference and increment iterator first.
      Instruction *Inst = &*I;
      ++I;

      builder.setLocation(Inst->getLocation());

      switch (Inst->getKind()) {
        case ValueKind::LoadFrameInstKind: {
          auto *LFI = cast<LoadFrameInst>(Inst);
          auto *var = LFI->getLoadVariable();

          builder.setInsertionPoint(Inst);
          Instruction *scope = getScope(builder, var, captureScope);
          Instruction *newInst =
              builder.createHBCLoadFromEnvironmentInst(scope, var);

          Inst->replaceAllUsesWith(newInst);
          Inst->eraseFromParent();
          changed = true;
          break;
        }
        case ValueKind::StoreFrameInstKind: {
          auto *SFI = cast<StoreFrameInst>(Inst);
          auto *var = SFI->getVariable();
          auto *val = SFI->getValue();

          builder.setInsertionPoint(Inst);
          Instruction *scope = getScope(builder, var, captureScope);
          builder.createHBCStoreToEnvironmentInst(scope, val, var);

          Inst->eraseFromParent();
          changed = true;
          break;
        }
        case ValueKind::CreateFunctionInstKind: {
          auto *CFI = cast<CreateFunctionInst>(Inst);

          builder.setInsertionPoint(Inst);
          auto *newInst = builder.createHBCCreateFunctionInst(
              CFI->getFunctionCode(), captureScope);

          Inst->replaceAllUsesWith(newInst);
          Inst->eraseFromParent();
          changed = true;
          break;
        }
        case ValueKind::CreateGeneratorInstKind: {
          auto *CFI = cast<CreateGeneratorInst>(Inst);

          builder.setInsertionPoint(Inst);
          auto *newInst = builder.createHBCCreateGeneratorInst(
              CFI->getFunctionCode(), captureScope);

          Inst->replaceAllUsesWith(newInst);
          Inst->eraseFromParent();
          changed = true;
          break;
        }
        default:
          break;
      }
    }
  }
  return changed;
}

CreateArgumentsInst *LowerArgumentsArray::getCreateArgumentsInst(Function *F) {
  // CreateArgumentsInst is always in the first block in normal functions,
  // but is in the second block in GeneratorInnerFunctions.
  if (llvh::isa<GeneratorInnerFunction>(F)) {
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
  updateToEntryInsertionPoint(builder, F);

  CreateArgumentsInst *createArguments = getCreateArgumentsInst(F);
  if (!createArguments) {
    return false;
  }

  builder.setInsertionPoint(createArguments);
  AllocStackInst *lazyReg = builder.createAllocStackInst("arguments");
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
    auto *load = llvh::dyn_cast<LoadPropertyInst>(user);
    if (load && load->getObject() == createArguments) {
      builder.setInsertionPoint(load);
      builder.setLocation(load->getLocation());
      auto *propertyString = llvh::dyn_cast<LiteralString>(load->getProperty());
      if (propertyString && propertyString->getValue().str() == "length") {
        // For `arguments.length`, get the length.
        auto *length = builder.createHBCGetArgumentsLengthInst(lazyReg);
        load->replaceAllUsesWith(length);
        load->eraseFromParent();
      } else {
        // For all other property loads, get by index.
        auto *get = builder.createHBCGetArgumentsPropByValInst(
            load->getProperty(), lazyReg);
        load->replaceAllUsesWith(get);
        load->eraseFromParent();
      }
    }
  }

  uniqueUsers.clear();
  uniqueUsers.insert(
      createArguments->getUsers().begin(), createArguments->getUsers().end());
  for (Value *user : uniqueUsers) {
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
        builder.createHBCReifyArgumentsInst(lazyReg);
        auto *reifiedValue = builder.createLoadStackInst(lazyReg);
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
    } else if (auto *inst = llvh::dyn_cast<Instruction>(user)) {
      // For other users, insert a reification so we can replace
      // the usage with this array.
      builder.setInsertionPoint(inst);
      builder.setLocation(inst->getLocation());
      builder.createHBCReifyArgumentsInst(lazyReg);
      auto *array = builder.createLoadStackInst(lazyReg);
      for (int i = 0, n = inst->getNumOperands(); i < n; i++) {
        if (inst->getOperand(i) == createArguments) {
          inst->setOperand(array, i);
        }
      }
    } else {
      llvm_unreachable("CreateArguments used for a non-Instruction.");
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
  PostOrderAnalysis PO(F);
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

bool LowerConstruction::runOnFunction(Function *F) {
  IRBuilder builder(F);
  auto *prototypeString = builder.getLiteralString("prototype");

  for (BasicBlock &BB : F->getBasicBlockList()) {
    IRBuilder::InstructionDestroyer destroyer;
    for (Instruction &I : BB) {
      if (auto *constructor = llvh::dyn_cast<ConstructInst>(&I)) {
        builder.setInsertionPoint(constructor);
        builder.setLocation(constructor->getLocation());
        auto closure = constructor->getCallee();
        auto prototype =
            builder.createLoadPropertyInst(closure, prototypeString);
        auto thisObject = builder.createHBCCreateThisInst(prototype, closure);

        llvh::SmallVector<Value *, 8> args;
        for (int i = 1, n = constructor->getNumArguments(); i < n; i++) {
          args.push_back(constructor->getArgument(i));
        }
        auto newConstructor =
            builder.createHBCConstructInst(closure, thisObject, args);
        auto finalValue = builder.createHBCGetConstructedObjectInst(
            thisObject, newConstructor);
        constructor->replaceAllUsesWith(finalValue);
        destroyer.add(constructor);
      }
    }
  }
  return true;
}

bool LowerCalls::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  for (auto &BB : *F) {
    for (auto &I : BB) {
      auto *call = llvh::dyn_cast<CallInst>(&I);
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

      for (auto index : I.getChangedOperands()) {
        auto *operand = I.getOperand(index);
        unsigned reg = RA_.getRegister(cast<Instruction>(operand)).getIndex();
        regToInstMap.erase(reg);
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
  if (I->getType().isNoType())
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
    case ValueKind::CallInstKind:
    case ValueKind::ConstructInstKind:
    case ValueKind::CallBuiltinInstKind:
    case ValueKind::HBCConstructInstKind:
    case ValueKind::HBCCallDirectInstKind:
      return op == 0;
    default:
      return true;
  }
}

bool SpillRegisters::modifiesOperandRegister(Instruction *I, int op) {
  return I->getChangedOperands().at((unsigned)op);
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
          auto temp = getReserved(tempReg++);

          builder.setInsertionPoint(&inst);
          auto *load = builder.createHBCSpillMovInst(op);
          RA_.updateRegister(load, temp);
          inst.setOperand(load, i);

          if (modifiesOperandRegister(&inst, i)) {
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
