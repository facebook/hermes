/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/PassManager/PassManager.h"

#include "hermes/IR/IR.h"
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
  Pass *lastPass = nullptr;
  auto dumpLastPass = [&lastPass, F](Pass *newPass) {
    if (!F->getContext().getCodeGenerationSettings().dumpIRBetweenPasses)
      return;

    if (!lastPass) {
      llvh::dbgs() << "*** INITIAL STATE\n\n";
    } else {
      llvh::dbgs() << "\n*** AFTER " << lastPass->getName() << "\n\n";
    }

    F->dump(llvh::dbgs());
    lastPass = newPass;
  };

  // For each pass:
  for (const std::unique_ptr<Pass> &P : pipeline_) {
    dumpLastPass(P.get());

    auto *FP = llvh::dyn_cast<FunctionPass>(P.get());
    assert(FP && "Invalid pass kind");
    LLVM_DEBUG(llvh::dbgs() << "Running the pass " << FP->getName() << "\n");
    LLVM_DEBUG(
        llvh::dbgs() << "Optimizing the function " << F->getInternalNameStr()
                     << "\n");
    FP->runOnFunction(F);
  }
  dumpLastPass(nullptr);
}

void PassManager::run(Module *M) {
  llvh::SmallVector<Timer, 32> timers;
  std::unique_ptr<TimerGroup> timerGroup{nullptr};
  if (AreStatisticsEnabled()) {
    timerGroup.reset(new TimerGroup("", "PassManager Timers"));
  }

  // Optionally dump the IR after every pass if the flag is set.
  Pass *lastPass = nullptr;
  auto dumpLastPass = [&lastPass, M](Pass *newPass) {
    if (!M->getContext().getCodeGenerationSettings().dumpIRBetweenPasses)
      return;

    if (!lastPass) {
      llvh::dbgs() << "*** INITIAL STATE\n\n";
    } else {
      llvh::dbgs() << "\n*** AFTER " << lastPass->getName() << "\n\n";
    }

    M->dump(llvh::dbgs());
    lastPass = newPass;
  };

  // For each pass:
  for (std::unique_ptr<Pass> &P : pipeline_) {
    dumpLastPass(P.get());

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

      // Move to the next pass.
      continue;
    }

    /// Handle module passes:
    if (auto *MP = llvh::dyn_cast<ModulePass>(P.get())) {
      LLVM_DEBUG(
          llvh::dbgs() << "Running the module pass " << MP->getName() << "\n");
      MP->runOnModule(M);
      // Move to the next pass.
      continue;
    }

    llvm_unreachable("Unknown pass kind");
  }
  dumpLastPass(nullptr);
}
} // namespace hermes
