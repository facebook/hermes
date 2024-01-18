/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/PassManager/PassManager.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Support/Timer.h"
#include "llvh/Support/Debug.h"

#define DEBUG_TYPE "passmanager"

namespace hermes {
PassManager::~PassManager() = default;

void PassManager::addPass(Pass *P) {
  pipeline_.emplace_back(P);
}

void PassManager::run(Function *F) {
  if (F->isLazy())
    return;

  // Optionally dump the IR after every pass if the flag is set.
  bool dumpBetweenPasses =
      F->getContext().getCodeGenerationSettings().dumpIRBetweenPasses;
  // TODO(T156009366): Run the IRVerifier between passes once we can run it on
  // individual functions.

  if (dumpBetweenPasses) {
    llvh::dbgs() << "*** INITIAL STATE\n\n";
    F->dump(llvh::dbgs());
  }

  // For each pass:
  for (const std::unique_ptr<Pass> &P : pipeline_) {
    auto *FP = llvh::dyn_cast<FunctionPass>(P.get());
    assert(FP && "Invalid pass kind");
    LLVM_DEBUG(llvh::dbgs() << "Running the pass " << FP->getName() << "\n");
    LLVM_DEBUG(
        llvh::dbgs() << "Optimizing the function " << F->getInternalNameStr()
                     << "\n");
    FP->runOnFunction(F);

    if (dumpBetweenPasses) {
      llvh::dbgs() << "\n*** AFTER " << P->getName() << "\n\n";
      F->dump(llvh::dbgs());
    }
  }
}

void PassManager::run(Module *M) {
  llvh::SmallVector<Timer, 32> timers;
  std::unique_ptr<TimerGroup> timerGroup{nullptr};
  if (AreStatisticsEnabled()) {
    timerGroup.reset(new TimerGroup("", "PassManager Timers"));
  }

  // Optionally dump the IR after every pass if the flag is set.
  bool dumpBetweenPasses =
      M->getContext().getCodeGenerationSettings().dumpIRBetweenPasses;
  // Run the IRVerifier after every pass if the flag is set.
  bool verifyBetweenPasses =
      M->getContext().getCodeGenerationSettings().verifyIRBetweenPasses;

  if (dumpBetweenPasses) {
    llvh::dbgs() << "*** INITIAL STATE\n\n";
    M->dump(llvh::dbgs());
  }

  // For each pass:
  for (std::unique_ptr<Pass> &P : pipeline_) {
    TimeRegion timeRegion(
        timerGroup ? timers.emplace_back("", P->getName(), *timerGroup),
        &timers.back()
                   : nullptr);

    /// Handle function passes:
    if (auto *FP = llvh::dyn_cast<FunctionPass>(P.get())) {
      LLVM_DEBUG(
          llvh::dbgs() << "Running the function pass " << FP->getName()
                       << "\n");

      for (auto &I : *M) {
        Function *F = &I;
        if (F->isLazy())
          continue;
        LLVM_DEBUG(
            llvh::dbgs() << "Optimizing the function "
                         << F->getInternalNameStr() << "\n");
        FP->runOnFunction(F);
      }
    } else {
      auto *MP = llvh::cast<ModulePass>(P.get());
      LLVM_DEBUG(
          llvh::dbgs() << "Running the module pass " << MP->getName() << "\n");
      MP->runOnModule(M);
    }

    if (dumpBetweenPasses) {
      llvh::dbgs() << "\n*** AFTER " << P->getName() << "\n\n";
      M->dump(llvh::dbgs());
    }

    if (verifyBetweenPasses) {
      if (!verifyModule(*M, &llvh::errs())) {
        M->getContext().getSourceErrorManager().error(
            {},
            {},
            llvh::Twine("IRVerifier failed after pass ") + P->getName());
        return;
      }
    }
  }
}
} // namespace hermes

#undef DEBUG_TYPE
