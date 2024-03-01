/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "scopeelimination"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Eliminate creation of \p VS if it does not contain any variables. Users of
/// VS and created scopes are replaced with their parent.
static bool eliminateScopeIfEmpty(VariableScope *VS) {
  if (!VS->getVariables().empty())
    return false;

  IRBuilder::InstructionDestroyer destroyer;
  VariableScope *parentVarScope = nullptr;
  for (auto *U : VS->getUsers()) {
    if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(U)) {
      if (llvh::isa<EmptySentinel>(CSI->getParentScope())) {
        // This is a root scope, and cannot be eliminated.
        assert(!parentVarScope && "VS has multiple parent scopes");
        return false;
      }

      // Check that there is a parent scope that we can use to replace this
      // with.
      auto *parentScope = llvh::cast<BaseScopeInst>(CSI->getParentScope());
      if (!parentVarScope)
        parentVarScope = parentScope->getVariableScope();
      assert(
          parentVarScope == parentScope->getVariableScope() &&
          "VS has multiple parent scopes");

      // Eliminate this scope creation by just replacing it with its parent.
      CSI->replaceAllUsesWith(parentScope);
      destroyer.add(CSI);
    }
  }

  if (!parentVarScope)
    return false;

  // Update any scope retrieval instructions (like GetParentScope and
  // ResolveScope) to now reference the parent scope directly.
  VS->replaceAllUsesWith(parentVarScope);
  return true;
}

static bool runScopeElimination(Module *M) {
  bool changed = false;
  for (auto &F : *M)
    changed |= eliminateScopeIfEmpty(F.getFunctionScope());
  return changed;
}

Pass *createScopeElimination() {
  class ThisPass : public ModulePass {
   public:
    ThisPass() : ModulePass("ScopeElimination") {}

    bool runOnModule(Module *M) override {
      return runScopeElimination(M);
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
