/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_SCOPETRANSFORMATIONS_H
#define HERMES_OPTIMIZER_SCALAR_SCOPETRANSFORMATIONS_H

#include "hermes/Optimizer/PassManager/Pass.h"

#include "hermes/IR/IR.h"
#include "llvh/ADT/DenseSet.h"

namespace hermes {

/// Merges scopes when it is safe to do so.
class ScopeMerger : public FunctionPass {
 public:
  explicit ScopeMerger() : FunctionPass("ScopeMerger") {}
  ~ScopeMerger() override = default;

  bool runOnFunction(Function *F) override;

 private:
  /// Merges the contents of \p child into \p parent. \p child should be a
  /// direct inner scope of \p parent (i.e.,
  /// \p child->getParent() == \p parent).
  void mergeInto(Function *F, ScopeDesc *parent, ScopeDesc *child);

  /// Recursively optimizes \p scopeDesc's inner scopes that are part of \p F.
  /// After each inner scope optimization, non-dynamic scopes (or dynamic scopes
  /// that don't have any escaping vars) are merged into \p scopeDesc.
  enum class HasEscapingVars { No, Yes };
  HasEscapingVars optimizeScope(Function *F, ScopeDesc *scopeDesc);

  /// \return \p scopeDesc if \p scopeDesc was not merged during scope merging;
  /// or the scope where \p scopeDesc's contents were merged to. Used to update
  /// the SourceLevelScope field of the IR after scope merging completes.
  ScopeDesc *mergedScope(ScopeDesc *scopeDesc);

  /// Updates the SourceLevelScope of \p F's instructions after scope merging
  /// completes.
  void updateSourceLevelScopes(Function *F);

  /// Map of original scope to merged scope.
  llvh::DenseMap<ScopeDesc *, ScopeDesc *> mergedMap_;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_SCOPETRANSFORMATIONS_H
