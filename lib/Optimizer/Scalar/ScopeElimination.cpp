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

  VariableScope *parentVarScope = VS->getParentScope();
  // We cannot eliminate root scopes, since they are created with EmptySentinel
  // as the parent operand to CreateScopeInst, which cannot propagate into the
  // rest of the IR.
  if (!parentVarScope)
    return false;

  IRBuilder::InstructionDestroyer destroyer;

  for (auto *U : VS->getUsers()) {
    if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(U)) {
      // Eliminate this scope creation by just replacing it with its parent.
      CSI->replaceAllUsesWith(CSI->getParentScope());
      destroyer.add(CSI);
    }
  }

  // Update any scope retrieval instructions (like GetParentScope and
  // ResolveScope) to now reference the parent scope directly.
  VS->replaceAllUsesWith(parentVarScope);
  VS->removeFromScopeChain();
  return true;
}

static bool runScopeElimination(Module *M) {
  bool changed = false;
  for (auto &VS : M->getVariableScopes())
    changed |= eliminateScopeIfEmpty(&VS);
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
