/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "scopehoisting"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#include "llvh/ADT/SetOperations.h"

/// This pass tries to hoist VariableScopes to the nearest ancestor scope that
/// actually need to be able to reach. This flattens the tree of VariableScopes,
/// making it faster to resolve ancestor scopes and reducing the memory
/// transitively retained by a scope.

namespace hermes {

/// Given the set of ancestor scopes \p uses that are resolved by this scope or
/// any of its descendents, try hoisting \p VS to the nearest ancestor scope
/// that it actually uses. Update users to reflect the new parent.
/// \return true if the scope was hoisted, false otherwise.
static bool tryHoistScope(
    VariableScope *VS,
    const llvh::DenseSet<VariableScope *> &uses,
    IRBuilder &builder) {
  // Identify the nearest ancestor that occurs in \p uses.
  auto *oldParent = VS->getParentScope();
  auto *newParent = oldParent;
  while (newParent && !uses.count(newParent))
    newParent = newParent->getParentScope();

  // If the current parent is used, there is nothing to do.
  if (newParent == oldParent)
    return false;

  // Copy the users before we start modifying them.
  auto users = VS->getUsers();

  // Insert a ResolveScope before each creation to go to the new parent.
  for (auto *U : users) {
    if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(U)) {
      assert(
          CSI->getVariableScope() == VS &&
          "No other usage in CreateScopeInst.");
      Value *newParentVal = builder.getEmptySentinel();
      if (newParent) {
        builder.setInsertionPoint(CSI);
        newParentVal = builder.createResolveScopeInst(
            newParent,
            oldParent,
            llvh::cast<Instruction>(CSI->getParentScope()));
      }
      CSI->setParentScope(newParentVal);
    }
  }
  VS->setParentScope(newParent);
  return true;
}

static bool runScopeHoisting(Module *M) {
  bool changed = false;
  IRBuilder builder{M};

  // Map from a scope to the set of its ancestor scopes that are resolved by it
  // or any of its children. If an ancestor is never resolved, we can hoist a
  // scope past that ancestor.
  llvh::DenseMap<VariableScope *, llvh::DenseSet<VariableScope *>> scopeUses;

  // Stack of a pair of <scope, childrenVisited>. This is used to visit all
  // scopes in post-order.
  llvh::SmallVector<std::pair<VariableScope *, bool>, 8> stack;

  // Start with the top-level scopes.
  for (auto &VS : M->getVariableScopes())
    if (!VS.getParentScope())
      stack.push_back({&VS, false});

  // Iterate in post-order over all the scopes. We have to visit child scopes
  // first to collect all of their usage information and propagate that to
  // parents.
  while (!stack.empty()) {
    auto [VS, childrenVisited] = stack.back();
    if (!childrenVisited) {
      // Children have not been visited, push them and move on.
      stack.back().second = true;
      for (auto &child : VS->getChildren())
        stack.push_back({&child, false});
      continue;
    }
    stack.pop_back();
    assert(!scopeUses.count(VS) && "Scope already populated");
    auto &uses = scopeUses[VS];

    for (auto *U : VS->getUsers()) {
      if (auto *RSI = llvh::dyn_cast<ResolveScopeInst>(U)) {
        // Record any scope that is resolved starting from this one. We cannot
        // hoist past scopes that are used.
        if (RSI->getStartVarScope() == VS)
          uses.insert(RSI->getVariableScope());
      }
    }

    // Record any scope that is resolved by children of this scope.
    for (auto &child : VS->getChildren()) {
      auto it = scopeUses.find(&child);
      assert(it != scopeUses.end() && "Child scope not visited yet");
      llvh::set_union(uses, it->second);
      scopeUses.erase(it);
    }

    // Try to hoist the scope to the highest used parent.
    changed |= tryHoistScope(VS, uses, builder);
  }
  return changed;
}

Pass *createScopeHoisting() {
  class ThisPass : public ModulePass {
   public:
    ThisPass() : ModulePass("ScopeHoisting") {}

    bool runOnModule(Module *M) override {
      return runScopeHoisting(M);
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
