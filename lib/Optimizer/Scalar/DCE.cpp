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
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;

STATISTIC(NumDCE, "Number of instructions DCE'd");

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

      // Skip if the instruction has side effects that prevent us from deleting
      // it, or if it is a terminator.
      if (I->getSideEffect().hasSideEffect() || llvh::isa<TerminatorInst>(I))
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

  // Perform per-function DCE.
  for (auto &F : *M) {
    LLVM_DEBUG(
        dbgs() << "\tDCE: running on function \"" << F.getInternalName()
               << "\"\n");
    changed |= performFunctionDCE(&F);
  }

  changed |= deleteUnusedFunctionsAndVariables(M);

  return changed;
}

Pass *hermes::createDCE() {
  return new DCE();
}

#undef DEBUG_TYPE
