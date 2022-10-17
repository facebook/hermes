/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "dce"

#include "hermes/Optimizer/Scalar/DCE.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;

STATISTIC(NumDCE, "Number of instructions DCE'd");
STATISTIC(NumFuncDCE, "Number of functions DCE'd");

static bool performFunctionDCE(Function *F) {
  bool changed = false;
  PostOrderAnalysis PO(F);

  // Scan the function in post order (from end to start). We want to visit the
  // uses of the instruction before we visit the instruction itself in order to
  // allow the optimization to delete long chains of dead code.
  for (auto *BB : PO) {
    // Scan the instructions in the block from end to start.
    for (auto it = BB->rbegin(), e = BB->rend(); it != e; /* nothing */) {
      Instruction *I = &*it;
      // Move the iterator to the next instruction in the block. This will
      // allow us to delete the current instruction.
      ++it;

      // If the instruction writes to memory then we can't remove it. Notice
      // that it is okay to delete instructions that only read memory and are
      // unused.
      //
      // Terminators don't have any uses but are never supposed to be removed
      // as dead code.
      //
      // CreateScopeInst may not have any users, but it is lowered to
      // HBCCreateEnvironmentInst which should always be emitted and DCE'd if
      // appropriate.
      if (I->mayWriteMemory() || llvh::isa<TerminatorInst>(I) ||
          llvh::isa<CreateScopeInst>(I)) {
        continue;
      }

      // If some other instruction is using the result of this instruction then
      // we can't delete it.
      if (I->getNumUsers())
        continue;

      LLVM_DEBUG(
          dbgs() << "\t\tDCE: Erasing instruction \"" << I->getName()
                 << "\"\n");
      I->eraseFromParent();
      ++NumDCE;
      changed = true;
    }
  }
  return changed;
}

bool DCE::runOnModule(Module *M) {
  bool changed = false;

  // A list of unused functions to deallocate from memory.
  // We need to destroy the memory at the very end of this function because
  // a dead function may have a variable that is referrenced by an inner
  // function (which will also become dead once the outer function is removed).
  // However we cannot destroy the outer function right away until we destroy
  // the inner function.
  llvh::SmallVector<Function *, 16> toDestroy;

  // Perform per-function DCE.
  for (auto &F : *M) {
    LLVM_DEBUG(
        dbgs() << "\tDCE: running on function \"" << F.getInternalName()
               << "\"\n");
    changed |= performFunctionDCE(&F);
  }

  bool localChanged = false;
  do {
    // A list of unused functions to remove from the module without being
    // destroyed.
    llvh::SmallVector<Function *, 16> toRemove;
    localChanged = false;
    for (auto &F : *M) {
      // Try to remove unused functions. Notice that the top-level-function has
      // no external users so we must check for it explicitly.
      if (M->findCJSModule(&F)) {
        // If the function is a top-level module.
        continue;
      }
      // Don't delete the function if it is at global scope, or if it is the
      // entry point of a module.
      if (!F.isGlobalScope() && &F != M->getTopLevelFunction() &&
          !F.hasUsers()) {
        toRemove.push_back(&F);
        toDestroy.push_back(&F);
        changed = true;
        localChanged = true;
        NumFuncDCE++;
      }
    }

    // We erase the basic blocks and instructions from each function in
    // toRemove, and also remove the function from the module. However
    // the memory of the function remain alive.
    for (auto *F : toRemove) {
      LLVM_DEBUG(
          dbgs() << "\tDCE: Erasing function \"" << F->getInternalName()
                 << "\"\n");
      F->eraseFromParentNoDestroy();
    }
  } while (localChanged);

  // Now that all instructions have been destroyed from each dead function,
  // it's now safe to destroy them including the variables in them.
  for (auto *F : toDestroy) {
    assert(F->empty() && "All basic blocks should have been deleted.");
    Value::destroy(F);
  }

  return changed | localChanged;
}

Pass *hermes::createDCE() {
  return new DCE();
}

#undef DEBUG_TYPE
