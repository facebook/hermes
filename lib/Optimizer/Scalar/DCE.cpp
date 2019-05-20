/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "dce"

#include "hermes/Optimizer/Scalar/DCE.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/Statistic.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;
using llvm::isa;

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
      if (I->mayWriteMemory() || isa<TerminatorInst>(I))
        continue;

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

  // A list of unused functions to delete.
  llvm::SmallVector<Function *, 16> toDelete;

  // Perform per-function DCE.
  for (auto &F : *M) {
    LLVM_DEBUG(
        dbgs() << "\tDCE: running on function \"" << F.getInternalName()
               << "\"\n");
    changed |= performFunctionDCE(&F);
  }

  bool localChanged = false;
  do {
    localChanged = false;
    for (auto &F : *M) {
      // Try to remove unused functions. Notice that the top-level-function has
      // no external users so we must check for it explicitly.
      if (M->findCJSModule(&F)) {
        // If the function is a top-level module.
        continue;
      }
      if (!F.isGlobalScope() && !F.hasUsers()) {
        toDelete.push_back(&F);
        changed = true;
        localChanged = true;
        NumFuncDCE++;
      }
    }

    for (auto *F : toDelete) {
      LLVM_DEBUG(
          dbgs() << "\tDCE: Erasing function \"" << F->getInternalName()
                 << "\"\n");
      F->eraseFromParent();
    }
    toDelete.clear();

  } while (localChanged);

  return changed | localChanged;
}

Pass *hermes::createDCE() {
  return new DCE();
}
