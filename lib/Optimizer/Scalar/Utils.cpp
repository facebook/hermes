/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "utils"

#include "hermes/Optimizer/Scalar/Utils.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

using namespace hermes;

Value *hermes::isStoreOnceVariable(Variable *V) {
  Value *res = nullptr;

  for (auto *U : V->getUsers()) {
    if (llvh::isa<LoadFrameInst>(U)) {
      continue;
    }
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U)) {
      auto *val = SF->getValue();

      // We found a stored value. Make sure that there is only one stored value.
      if (res && val != res)
        return nullptr;

      res = val;
      continue;
    }

    llvm_unreachable("invalid user!");
  }

  return res;
}

Value *hermes::isStoreOnceStackLocation(AllocStackInst *AS) {
  Value *res = nullptr;

  for (auto *U : AS->getUsers()) {
    if (llvh::isa<LoadStackInst>(U)) {
      continue;
    }
    if (auto *SS = llvh::dyn_cast<StoreStackInst>(U)) {
      // Theoretically someone might try storing a stack address in another
      // stack location. That is not allowed, but we might as well be correct
      // here.
      if (SS->getPtr() != AS)
        continue;

      auto *val = SS->getValue();

      // We found a stored value. Make sure that there is only one stored value.
      if (res && val != res)
        return nullptr;

      res = val;
      continue;
    }

    llvm_unreachable("invalid user!");
  }

  return res;
}

bool hermes::isDirectCallee(Instruction *C, BaseCallInst *CI) {
  if (CI->getCallee() != C)
    return false;

  for (int i = 0, e = CI->getNumArguments(); i < e; i++) {
    // Check if C is captured.
    if (C == CI->getArgument(i))
      return false;
  }

  return true;
}

bool hermes::isConstructionSetup(Value *V, Value *closure) {
  // Constructor invocations access the closure, to read the "prototype"
  // property and to create the "this" argument. However, in the absence of
  // other accesses (like making "prototype" a setter), these operations
  // cannot leak the closure.
  if (auto *LPI = llvh::dyn_cast<LoadPropertyInst>(V))
    if (LPI->getObject() == closure)
      if (auto *LS = llvh::dyn_cast<LiteralString>(LPI->getProperty()))
        if (LS->getValue().str() == "prototype")
          return true;

  if (auto *CTI = llvh::dyn_cast<CreateThisInst>(V)) {
    (void)CTI;
    assert(
        CTI->getClosure() == closure &&
        "Closure must be closure argument to CreateThisInst");
    return true;
  }
  return false;
}

bool hermes::getCallSites(
    Function *F,
    llvh::DenseSet<BaseCallInst *> &callsites) {
  // Skip global and module functions.
  if (F->isGlobalScope() || F->getParent()->findCJSModule(F))
    return false;

  for (auto *CU : F->getUsers()) {
    // Target field must be ignored here because we'll find this call
    // via the BaseCreateCallableInst in another iteration of the loop.
    // We have to ignore it because the function is only going to return `true`
    // if _all_ callsites are found and it'll bail on unknown insts.
    if (auto *call = llvh::dyn_cast<BaseCallInst>(CU)) {
      if (call->getTarget() == F) {
        continue;
      }
    }

    auto *CFI = llvh::dyn_cast<BaseCreateCallableInst>(CU);
    if (!CFI) {
      // Used in an instruction that returns a non-callable. Bail.
      return false;
    }

    // Collect direct calls.
    for (auto *U : CFI->getUsers()) {
      auto *CI = llvh::dyn_cast<BaseCallInst>(U);
      if (CI && isDirectCallee(CFI, CI)) {
        callsites.insert(CI);
        continue;
      }

      // Check if the closure is only being used to set up for construction.
      if (isConstructionSetup(U, CFI))
        continue;

      // Check if the variable is stored somewhere.
      auto *SFI = llvh::dyn_cast<StoreFrameInst>(U);
      if (!SFI)
        return false;

      // If the variable is analyzable then try to see where the closure
      // goes.
      Variable *V = SFI->getVariable();
      if (!isStoreOnceVariable(V))
        return false;

      for (auto *VU : V->getUsers()) {
        if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(VU)) {
          for (Value *loadUser : LFI->getUsers()) {
            if (auto *loadUserCI = llvh::dyn_cast<BaseCallInst>(loadUser)) {
              if (isDirectCallee(LFI, loadUserCI)) {
                callsites.insert(loadUserCI);
                continue;
              }
            }

            if (isConstructionSetup(loadUser, LFI))
              continue;

            // Unknown load used.
            return false;
          }
        }
      }
    }
  }
  return true;
}

llvh::SmallVector<BaseCallInst *, 2> hermes::getKnownCallsites(Function *F) {
  llvh::SmallVector<BaseCallInst *, 2> result{};
  for (Instruction *user : F->getUsers()) {
    if (auto *call = llvh::dyn_cast<BaseCallInst>(user)) {
      assert(
          call->getTarget() == F &&
          "invalid usage of Function as operand of BaseCallInst");
      result.push_back(call);
    }
  }
  return result;
}

/// Delete all incoming arrows from \p incoming in PhiInsts in \p blockToModify.
bool hermes::deleteIncomingBlockFromPhis(
    BasicBlock *blockToModify,
    BasicBlock *incoming) {
  bool changed = false;
  for (auto &I : *blockToModify) {
    auto *phi = llvh::dyn_cast<PhiInst>(&I);
    if (!phi)
      break;

    for (signed i = (signed)phi->getNumEntries() - 1; i >= 0; i--) {
      auto entry = phi->getEntry(i);
      if (entry.second != incoming)
        continue;

      phi->removeEntry(i);
      changed = true;
    }
  }
  return changed;
}

void hermes::splitCriticalEdge(
    IRBuilder *builder,
    BasicBlock *from,
    BasicBlock *to) {
  // Special case: If the target block is Catch block, there's only one
  // possible arrow and we can't insert anything between them. Just
  // start writing after the Catch statement.
  if (auto *tryStart = llvh::dyn_cast<TryStartInst>(from->getTerminator())) {
    if (tryStart->getCatchTarget() == to) {
      builder->setInsertionPointAfter(&to->front());
      return;
    }
  }

  // General case: insert a new block and rewrite Phis
  auto *newBlock = builder->createBasicBlock(from->getParent());
  builder->setInsertionBlock(newBlock);
  auto *branch = builder->createBranchInst(to);
  int updates = 0;

  Instruction *terminator = from->getTerminator();
  for (int i = 0, e = terminator->getNumOperands(); i < e; i++) {
    if (terminator->getOperand(i) == to) {
      terminator->setOperand(newBlock, i);
      for (auto &I : *to) {
        auto *phi = llvh::dyn_cast<PhiInst>(&I);
        if (!phi)
          break;
        for (int j = 0, f = phi->getNumEntries(); j < f; j++) {
          auto entry = phi->getEntry(j);
          if (entry.second != from)
            continue;
          phi->updateEntry(j, entry.first, newBlock);
        }
      }
      updates++;
    }
  }

  if (!updates) {
    llvm_unreachable("There were no current transitions between blocks");
  }
  builder->setInsertionPoint(branch);
}

bool hermes::deleteUnusedVariables(Module *M) {
  bool changed = false;
  for (Function &F : *M) {
    llvh::SmallVectorImpl<Variable *> &vars =
        F.getFunctionScope()->getVariables();
    // Delete variables without any users. Do this in a separate loop since we
    // are putting vars in an invalid state.
    for (Variable *&var : vars) {
      if (!var->hasUsers()) {
        Value::destroy(var);
        var = nullptr;
        changed = true;
      }
    }
    // Clean up the variable list to remove destroyed entries.
    llvh::erase_if(vars, [](Variable *var) { return !var; });
  }

  return changed;
}

bool hermes::deleteUnusedFunctionsAndVariables(Module *M) {
  // A list of unused functions to deallocate from memory.
  // We need to destroy the memory at the very end of this function because a
  // dead function may have a variable that is referenced by an inner function
  // (which will also become dead once the outer function is removed). However,
  // we cannot destroy the outer function right away until we destroy the inner
  // function.
  llvh::SmallVector<Function *, 16> toDestroy;

  bool changed = false, localChanged = false;
  do {
    // A list of unused functions to remove from the module without being
    // destroyed. We have to collect these separately since we can't remove the
    // functions as we iterate over the module.
    llvh::SmallVector<Function *, 16> toRemove;
    localChanged = false;
    for (auto &F : *M) {
      // Delete any functions that are unused. The top level function does not
      // have an explicit user, so check for it directly.
      if (&F != M->getTopLevelFunction() && !F.hasUsers()) {
        toRemove.push_back(&F);
        toDestroy.push_back(&F);
        changed = true;
        localChanged = true;
      }
    }

    // We erase the basic blocks and instructions from each function in
    // toRemove, and also remove the function from the module. However, the
    // memory of the function remains alive.
    for (auto *F : toRemove)
      F->eraseFromParentNoDestroy();
  } while (localChanged);

  // Now that all instructions have been destroyed from each dead function, it's
  // now safe to destroy them including the variables in them.
  for (auto *F : toDestroy) {
    assert(F->empty() && "All basic blocks should have been deleted.");
    Value::destroy(F);
  }

  // Deleting functions will make some variables unused, delete them.
  changed |= deleteUnusedVariables(M);
  return changed;
}

bool hermes::isSimpleSideEffectFreeInstruction(Instruction *I) {
  if (I->hasSideEffect()) {
    return false;
  }
  if (llvh::isa<UnaryOperatorInst>(I))
    return true;
  if (llvh::isa<BinaryOperatorInst>(I))
    return true;
  switch (I->getKind()) {
    case ValueKind::GetNewTargetInstKind:
    case ValueKind::HBCResolveEnvironmentKind:
    case ValueKind::HBCLoadConstInstKind:
    case ValueKind::HBCGetGlobalObjectInstKind:
      return true;
    default:
      return false;
  }
  llvm_unreachable("unreachable");
}

#undef DEBUG_TYPE
