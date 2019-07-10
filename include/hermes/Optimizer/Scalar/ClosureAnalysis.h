/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_OPTIMIZER_SCALAR_CLOSURE_ANALYSIS_H
#define HERMES_OPTIMIZER_SCALAR_CLOSURE_ANALYSIS_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/BundlerUtils.h"
#include "hermes/Optimizer/Scalar/SetConstraintAnalysisProblem.h"
#include "llvm/ADT/SetVector.h"

namespace hermes {

using FunctionSet = llvm::SetVector<Function *>;

class ClosureAnalysis {
  /// Given a non-inlineable JSmodule, generate constraints
  /// for it and the inlineable JSmodules that it requires.
  /// Note: this only works for a tree of dependencies, not a DAG
  /// Identification of inlineable modules must ensure we only
  /// present trees here, not DAGs.
  void analyzeJSModuleWithDependents(
      Function *,
      unsigned int,
      SetConstraintAnalysisProblem *,
      bool);

  /// Generate constraints for the root of the
  /// JSmodule and nested functions.
  /// If this is a dependent module, then, module.exports is captured.
  /// If require'ing a dependent module, then require(...) is captured.
  void generateConstraintsJSModule(
      Function &R,
      SetConstraintAnalysisProblem *ap,
      FunctionSet &nested,
      unsigned int modId,
      llvm::SetVector<unsigned int> &deps,
      bool isIndependent);

  BundlerUtils bundlerUtils_;

 public:
  /// Set containing roots of the analysis. SetVector rather than
  /// DenseSet is used to preserve iteration order.
  FunctionSet analysisRoots_;

  /// Map of each function to its analysis root in which it is nested.
  llvm::DenseMap<Function *, Function *> rootMap_;

  /// Map of analysis roots to the solved analysis representation.
  llvm::DenseMap<Function *, SetConstraintAnalysisProblem *> analysisMap_;

  /// Map of analysis roots to the nested functions (including itself!)
  llvm::DenseMap<Function *, FunctionSet> nestedMap_;

  /// Set of SetConstraintAnalysisProblems created (needed for cleanup)
  llvm::DenseSet<SetConstraintAnalysisProblem *> analyses_;

  bool analyzeModule(Module *M);

  /// If an analysis root contains more than this number of functions,
  /// do not analyze it
  static const unsigned int MAX_NEST_SIZE = 100;

  ~ClosureAnalysis() {
    for (auto *a : analyses_) {
      // This deletes each analysis object. Recall that these may be
      // shared between multiple analysis roots.
      delete a;
    }
    analyses_.clear();
    analysisRoots_.clear();
    rootMap_.clear();
    analysisMap_.clear();
    nestedMap_.clear();
  }
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_CLOSURE_ANALYSIS_H
