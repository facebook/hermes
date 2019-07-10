/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "uncalledmethodopts"

#include "hermes/Optimizer/Scalar/UncalledMethodOpts.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/CLACallGraphProvider.h"
#include "hermes/Optimizer/Scalar/ClosureAnalysis.h"
#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"
#include "hermes/Support/Statistic.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;
using llvm::isa;

STATISTIC(NumUMO, "Number of uncalled functions whose bodies were removed");

bool UncalledMethodOpts::isUncalled(Function *F) {
  if (cgp_->hasUnknownCallsites(F))
    return false;

  if (cgp_->getKnownCallsites(F).empty())
    return true;

  return false;
}

bool UncalledMethodOpts::replaceFunctionBodyWithReturn(Function *F) {
  bool changed = true;
  IRBuilder::InstructionDestroyer destroyer;

  LLVM_DEBUG(
      dbgs() << "Replacing the body of Function " << F->getInternalName()
             << "\n");
  NumUMO++;

  // Destroy all existing basic blocks, without relying on DCE.
  // However, if we first insert a Return statement at the entry
  // (see below), then DCE could clean this up.
  while (F->begin() != F->end()) {
    F->begin()->replaceAllUsesWith(nullptr);
    F->begin()->eraseFromParent();
  }

  // Restore the invariant of the Function: it needs at least
  // one basic block in it.  Insert a ReturnInst statement in a
  // freshly-created block attached to the Function entry.
  IRBuilder B(F);
  BasicBlock *BB = B.createBasicBlock(F);
  B.setInsertionBlock(BB);
  B.createThrowInst(B.getLiteralString(F->getInternalName()));
  // B.createReturnInst(B.getLiteralUndefined());

  return changed;
}

bool UncalledMethodOpts::runOnFunction(Function *F) {
  if (isUncalled(F)) {
    return replaceFunctionBodyWithReturn(F);
  }
  return false;
}

bool UncalledMethodOpts::runOnModule(Module *M) {
  if (!M->getContext().getOptimizationSettings().uncalledMethodOptimizations)
    return false;

  bool changed = false;

  LLVM_DEBUG(
      dbgs() << "\nStart uncalled methods opts on module "
             << "\n");

  // Perform the closure analysis based call graph.

  ClosureAnalysis CA;
  CA.analyzeModule(M);

  // Here we need to do any work only if CLA results are available.

  // Process the function nests nested in the top level.
  for (auto &R : CA.analysisRoots_) {
    LLVM_DEBUG(
        dbgs() << "Working with root " << R->getInternalName().c_str() << "\n");

    auto analysis = CA.analysisMap_.find(R);
    assert(analysis != CA.analysisMap_.end());

    auto nested = CA.nestedMap_.find(R);
    assert(nested != CA.nestedMap_.end());

    if (analysis->second != nullptr) {
      CLACallGraphProvider cgp(analysis->second);
      cgp_ = &cgp;
      for (auto &G : nested->second) {
        changed |= runOnFunction(G);
      }
    }
  }

  return changed;
}

Pass *hermes::createUncalledMethodOpts() {
  return new UncalledMethodOpts();
}
