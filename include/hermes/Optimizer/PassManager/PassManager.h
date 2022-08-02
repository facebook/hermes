/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
#define HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H

#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Support/Timer.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/ADT/StringSwitch.h"
#include "llvh/Support/Debug.h"

#define DEBUG_TYPE "passmanager"

namespace hermes {

/// The pass manager is responsible for running the transformation passes on the
/// whole module and on the functions in the module. The pass manager determines
/// the order of the passes, the order of the functions to be processed and the
/// invalidation of analysis.
class PassManager {
  std::vector<Pass *> pipeline;

 public:
  ~PassManager() {
    for (auto *p : pipeline) {
      delete p;
    }
  }

/// Add a pass by appending its name.
#define PASS(ID, NAME, DESCRIPTION) \
  void add##ID() {                  \
    Pass *P = hermes::create##ID(); \
    pipeline.push_back(P);          \
  }
#include "Passes.def"

  /// Add a pass by name.
  bool addPassForName(llvh::StringRef name) {
#define PASS(ID, NAME, DESCRIPTION) \
  if (name == NAME) {               \
    add##ID();                      \
    return true;                    \
  }
#include "Passes.def"
    return false;
  }

  static std::string getCustomPassText() {
    return
#define PASS(ID, NAME, DESCRIPTION) NAME ": " DESCRIPTION "\n"
#include "Passes.def"
        ;
  }

  /// Add a pass by reference.
  void addPass(Pass *P) {
    pipeline.push_back(P);
  }

  void run(Function *F) {
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
    for (auto *P : pipeline) {
      dumpLastPass(P);

      auto *FP = llvh::dyn_cast<FunctionPass>(P);
      assert(FP && "Invalid pass kind");
      LLVM_DEBUG(llvh::dbgs() << "Running the pass " << FP->getName() << "\n");
      LLVM_DEBUG(
          llvh::dbgs() << "Optimizing the function " << F->getInternalNameStr()
                       << "\n");
      FP->runOnFunction(F);
    }
    dumpLastPass(nullptr);
  }

  void run(Module *M) {
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
    for (auto *P : pipeline) {
      dumpLastPass(P);

      TimeRegion timeRegion(
          timerGroup ? timers.emplace_back("", P->getName(), *timerGroup),
          &timers.back()
                     : nullptr);

      /// Handle function passes:
      if (auto *FP = llvh::dyn_cast<FunctionPass>(P)) {
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
      if (auto *MP = llvh::dyn_cast<ModulePass>(P)) {
        LLVM_DEBUG(
            llvh::dbgs() << "Running the module pass " << MP->getName()
                         << "\n");
        MP->runOnModule(M);
        // Move to the next pass.
        continue;
      }

      llvm_unreachable("Unknown pass kind");
    }
    dumpLastPass(nullptr);
  }
};
} // namespace hermes
#undef DEBUG_TYPE
#endif // HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
