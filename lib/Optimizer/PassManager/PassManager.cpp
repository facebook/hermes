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
#include "llvh/Support/Debug.h"

#include <deque>

#define DEBUG_TYPE "passmanager"

namespace hermes {

/// Information used while running a pipeline.
struct PassManager::DynamicInfo {
  std::unique_ptr<TimerGroup> timerGroup;
  std::deque<Timer> timers;
  bool dumpBetweenPasses;
  bool verifyBetweenPasses;
};

/// This is a "pseudo-pass" -- it's not a real pass, but we use it
/// to represent a fixed-point loop.
class FixedPointLoopPass : public Pass {
 public:
  /// The passes executed in the loop.
  PassManager::PassSeq passes;

 private:
  /// The PassManager that owns this loop.
  PassManager *pm_;

  /// If the FixedPointLoop has not reached a fixed point
  /// after this number of iterations, it will print a warning
  /// and exit the loop.
  unsigned maxIters_;

 public:
  FixedPointLoopPass(PassManager *pm, llvh::StringRef name, unsigned maxIters)
      : Pass(PassKind::FixedPointLoop, name), pm_(pm), maxIters_(maxIters) {}

  /// Execute the loop.
  /// Returns false only if IR verification is enabled, and fails.
  bool run(Module *M, PassManager::DynamicInfo &dynInfo);

  static bool classof(const Pass *S) {
    return S->getKind() == PassKind::FixedPointLoop;
  }
};

PassManager::~PassManager() = default;

PassManager::PassSeq *PassManager::getCurrentPassSeq() {
  assert(
      !curPassSeqStack_.empty() &&
      "There should always be the original pipeline_");
  return curPassSeqStack_.back();
}

void PassManager::addPass(Pass *P) {
  assert(
      !llvh::isa<FixedPointLoopPass>(P) &&
      "FixedPointLoopPass is only for use by the implementation.");
  getCurrentPassSeq()->emplace_back(P);
}

void PassManager::beginFixedPointLoop(llvh::StringRef name, unsigned maxIters) {
  auto *fixedPointLoop = new FixedPointLoopPass(this, name, maxIters);
  getCurrentPassSeq()->emplace_back(fixedPointLoop);
  curPassSeqStack_.push_back(&fixedPointLoop->passes);
  pipelineContainsLoop_ = true;
}

void PassManager::endFixedPointLoop() {
  // There must be a current FixedPointLoop pass to close.
  assert(curPassSeqStack_.size() > 1 && "No FixedPointLoop to end.");
  // We end the current FixedPointLoop by returning to its parent sequence
  // as the current sequence.
  curPassSeqStack_.pop_back();
#ifndef NDEBUG
  PassSeq *ownerPassSeq = curPassSeqStack_.back();
  assert(
      !ownerPassSeq->empty() &&
      llvh::isa<FixedPointLoopPass>(ownerPassSeq->back().get()) &&
      "Should be ending FixedPointLoop");
#endif
}

void PassManager::run(Function *F) {
  assert(
      !pipelineContainsLoop_ &&
      "Pipelines with loops may only be run with Modules.");

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

llvh::hash_code PassManager::getModuleHash(const Module &M) {
  if (!moduleHashAfterLastPass_) {
    moduleHashAfterLastPass_ = M.hash();
  } else {
    // We're going to use the cached value below; make sure it's correct.
    assert(*moduleHashAfterLastPass_ == M.hash());
  }
  return *moduleHashAfterLastPass_;
}

bool FixedPointLoopPass::run(Module *M, PassManager::DynamicInfo &dynInfo) {
  // If we didn't return from within the loop, we reached maxIters_.
  llvh::hash_code loopModHashPre = pm_->getModuleHash(*M);
  for (unsigned iters = 0; iters < maxIters_; iters++) {
    for (const auto &pass : passes) {
      if (!pm_->runPassOnModule(M, pass.get(), dynInfo)) {
        return false;
      }
    }
    llvh::hash_code loopModHashPost = pm_->getModuleHash(*M);
    if (loopModHashPost == loopModHashPre) {
      return true;
    }
    loopModHashPre = loopModHashPost;
  }
  // If we didn't return from within the loop, we reached maxIters_.
  M->getContext().getSourceErrorManager().warning(
      SMLoc{},
      Twine("Opt-to-fixed-point loop ") + getName() +
          " reached max iterations (" + Twine(maxIters_) + ").  " +
          "This is likely a bug.");

  return true;
}

bool PassManager::runPassOnModule(Module *M, Pass *P, DynamicInfo &dynInfo) {
  // If the current pass is a loop, run that.
  if (auto *fixedPointLoop = llvh::dyn_cast<FixedPointLoopPass>(P)) {
    return fixedPointLoop->run(M, dynInfo);
  }

  // We're going to run a pass, so invalidate the cached module hash.
  moduleHashAfterLastPass_.reset();

  TimeRegion timeRegion(
      dynInfo.timerGroup
      ? dynInfo.timers.emplace_back("", P->getName(), *dynInfo.timerGroup),
      &dynInfo.timers.back()
      : nullptr);

  /// Handle function passes:
  if (auto *FP = llvh::dyn_cast<FunctionPass>(P)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Running the function pass " << FP->getName() << "\n");

    for (auto &I : *M) {
      Function *F = &I;
      if (F->isLazy())
        continue;
      LLVM_DEBUG(
          llvh::dbgs() << "Optimizing the function " << F->getInternalNameStr()
                       << "\n");
      FP->runOnFunction(F);
    }
  } else {
    auto *MP = llvh::cast<ModulePass>(P);
    LLVM_DEBUG(
        llvh::dbgs() << "Running the module pass " << MP->getName() << "\n");
    MP->runOnModule(M);
  }

  if (dynInfo.dumpBetweenPasses) {
    llvh::dbgs() << "\n*** AFTER " << P->getName() << "\n\n";
    M->dump(llvh::dbgs());
  }

  if (dynInfo.verifyBetweenPasses) {
    if (!verifyModule(*M, &llvh::errs())) {
      M->getContext().getSourceErrorManager().error(
          {}, {}, llvh::Twine("IRVerifier failed after pass ") + P->getName());
      return false;
    }
  }
  return true;
}

bool PassManager::run(Module *M) {
  assert(curPassSeqStack_.size() == 1 && "FixedPointLoop left unterminated");
  DynamicInfo dynInfo;
  if (M->getContext().getCodeGenerationSettings().timeCompiler) {
    if constexpr (kTimerEnabled) {
      llvh::StringRef nm = getName();
      if (nm.empty())
        nm = "PassManager Timers";
      dynInfo.timerGroup.reset(new TimerGroup("", nm));
    } else {
      llvh::errs() << "Warning: compiler timing requested, "
                   << "but build does not have timers.\n";
    }
  }

  // Optionally dump the IR after every pass if the flag is set.
  dynInfo.dumpBetweenPasses =
      M->getContext().getCodeGenerationSettings().dumpIRBetweenPasses;
  // Run the IRVerifier after every pass if the flag is set.
  dynInfo.verifyBetweenPasses =
      M->getContext().getCodeGenerationSettings().verifyIRBetweenPasses;

  if (dynInfo.dumpBetweenPasses) {
    llvh::dbgs() << "*** INITIAL STATE\n\n";
    M->dump(llvh::dbgs());
  }
  // For each pass:
  for (auto &pass : pipeline_) {
    if (!runPassOnModule(M, pass.get(), dynInfo)) {
      return false;
    }
  }
  return true;
}

} // namespace hermes

#undef DEBUG_TYPE
