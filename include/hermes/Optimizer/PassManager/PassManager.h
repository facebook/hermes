/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
#define HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H

#include "hermes/Optimizer/PassManager/Pass.h"

#include "hermes/Support/Timer.h"

#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"

#include <memory>

namespace hermes {

/// The pass manager is responsible for running the transformation passes on the
/// whole module and on the functions in the module. The pass manager determines
/// the order of the passes, the order of the functions to be processed and the
/// invalidation of analysis.
class PassManager {
  friend class FixedPointLoopPass;

  /// The name of the PassManager.
  llvh::StringRef pmName_;

  using PassSeq = std::vector<std::unique_ptr<Pass>>;
  std::vector<std::unique_ptr<Pass>> pipeline_;

  /// Whether the pipeline contains a loop.
  bool pipelineContainsLoop_ = false;

  /// A stack of the Pass sequences currently being constructed.
  std::vector<PassSeq *> curPassSeqStack_;

  /// If we take the hash of the Module after a pass, we record it here.
  /// If \p moduleHashAfterLastPass_ is not llvh::None, it is a hash
  /// value that was computed after the last pass was run.  If we run
  /// a pass, we set \p moduleHashAfterLastPass_ to llvh::None.  But
  /// if need want to compute a hash, and there have been no
  /// intervening passes run, we can re-use this cache.  This is an
  /// optimization when there are nested fixed-point loops, and the
  /// inner one shares a boundary with the outer one.  For example,
  /// say the outer loop runs one pass, then an inner loop.  At the
  /// end of an iteration of the inner loop, we will compute the
  /// module hash.  If that terminates the inner loop, we will return
  /// to the outer loop, which has also finished an iteration.
  /// Without this caching optimization, the outer loop would
  /// recompute the hash.
  llvh::Optional<llvh::hash_code> moduleHashAfterLastPass_;

  /// Returns the current PassSeq to append passes to.
  PassSeq *getCurrentPassSeq();

  /// Returns the current hash of \p M.  (We assume that all passes
  /// are run on \p M, or functions within \p M.)
  llvh::hash_code getModuleHash(const Module &M);

  /// Information used while running a pipeline.  Forward decl here;
  /// full decl in PassManager.cpp
  struct DynamicInfo;

  /// Run the pass \p P on the module \p M, using the given \p dynInfo.
  /// Returns false if the pass (or, if the pass is a FixedPointLoop, any
  /// pass returned in the loop) fails verification.
  bool runPassOnModule(Module *M, Pass *P, DynamicInfo &dynInfo);

 public:
  PassManager(llvh::StringRef pmName = "") : pmName_(pmName) {
    curPassSeqStack_.emplace_back(&pipeline_);
  }
  ~PassManager();

/// Add a pass by appending its name.
#define PASS(ID, NAME, DESCRIPTION) \
  void add##ID() {                  \
    addPass(hermes::create##ID());  \
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

  llvh::StringRef getName() const {
    return pmName_;
  }

  /// Add a pass by reference.
  void addPass(Pass *P);

  /// Starts a fixed-point loop.  New passes, and loops, will
  /// be added to this loop until a matching \p endFixedPointLoop()
  /// call is made.
  void beginFixedPointLoop(llvh::StringRef name, unsigned maxIters = 20);

  /// Terminate the current fixed-point loop.  Requires that there is one.
  /// The innermost unterminated fixed-point loop, if there is one, or else
  /// the original pipeline, become the new pipeline to which passes are
  /// appended.
  void endFixedPointLoop();

  void run(Function *F);

  /// Run all the passes added.
  /// If IR verification is enabled:
  ///  Verify the IR between every pass. On the first failure, stop running any
  ///  more passes and \return false. If all passes verified correctly, \return
  ///  true.
  /// If IR verification is not enabled, \return true.
  bool run(Module *M);
};
} // namespace hermes
#undef DEBUG_TYPE
#endif // HERMES_OPTIMIZER_PASSMANAGER_PASSMANAGER_H
