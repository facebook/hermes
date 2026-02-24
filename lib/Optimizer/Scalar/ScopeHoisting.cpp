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
#include "llvh/ADT/SetVector.h"

/// This pass tries to hoist VariableScopes to the nearest ancestor scope that
/// actually need to be able to reach. This flattens the tree of VariableScopes,
/// making it faster to resolve ancestor scopes and reducing the memory
/// transitively retained by a scope. Similarly, it identifies the set of scopes
/// that actually need to be reachable from a function, and updates the scope
/// captured in its closure to the nearest parent that is actually used.

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

/// Hoist the function \p F which is known to never use its parent scope. The
/// function will be hoisted to the top level so it no longer has a parent
/// scope.
/// \return false if the function was already hoisted, true otherwise.
static bool hoistFunctionToTopLevel(Function *F) {
  bool changed = false;
  IRBuilder builder{F->getParent()};
  IRBuilder::InstructionDestroyer destroyer;

  // Update all the users of the function to reflect the new parent scope.
  for (auto *U : F->getUsers()) {
    if (auto *BCLI = llvh::dyn_cast<BaseCreateLexicalChildInst>(U)) {
      // If the scope is already empty, there is nothing left to do.
      if (llvh::isa<EmptySentinel>(BCLI->getScope())) {
        assert(!changed && "Mismatched parent scopes for function");
        return false;
      }
      BCLI->setScope(builder.getEmptySentinel());
      BCLI->setVarScope(builder.getEmptySentinel());
      changed = true;
    } else if (auto *BCI = llvh::dyn_cast<BaseCallInst>(U)) {
      if (llvh::isa<EmptySentinel>(BCI->getEnvironment()))
        continue;
      BCI->setEnvironment(builder.getEmptySentinel());
      changed = true;
    } else if (llvh::isa<CreateThisInst>(U)) {
      // Do nothing, CreateThisInst is not affected by the parent scope.
    } else {
      assert(llvh::isa<GetClosureScopeInst>(U) && "Unknown user of function.");
      assert(!U->hasUsers() && "Trying to eliminate parent that is used.");
      // Any users of GetClosureScopeInst must be dead, because the function's
      // scope is known to be unused.
      destroyer.add(U);
      changed = true;
    }
  }

  // Delete the unused users of the parent scope parameter.
  for (auto *U : F->getParentScopeParam()->getUsers()) {
    assert(llvh::isa<GetParentScopeInst>(U) && "Unexpected user of parent.");
    assert(!U->hasUsers() && "Trying to eliminate parent that is used.");
    destroyer.add(U);
    changed = true;
  }
  return changed;
}

/// Try to hoist the function \p F.
static bool tryHoistFunction(Function *F) {
  llvh::DenseSet<VariableScope *> usedVarScopes;
  VariableScope *oldParentVarScope = nullptr;

  /// Given an instruction \p fnScopeInst that is known to produce the scope
  /// stored in a function's closure, traverse its users to determine whether it
  /// is only used to access ancestor scopes, and if so, populate them in
  /// \p usedVarScopes. Also populate oldParentVarScope if it isn't already.
  /// \return true if the instruction is used only to resolve scopes, false
  /// otherwise. If false is returned usedVarScopes should be ignored.
  auto collectUsedScopes = [&usedVarScopes,
                            &oldParentVarScope](BaseScopeInst *fnScopeInst) {
    auto *varScope = fnScopeInst->getVariableScope();
    assert(
        (!oldParentVarScope || oldParentVarScope == varScope) &&
        "Parent scope mismatch.");
    oldParentVarScope = varScope;
    for (auto *U : fnScopeInst->getUsers()) {
      // If the scope is used for something other than resolving, bail.
      if (!llvh::isa<ResolveScopeInst>(U))
        return false;

      // We are currently accessing this scope via the function's parent. Record
      // that we need to maintain the ability to get to this scope from any new
      // parent we choose.
      usedVarScopes.insert(llvh::cast<ResolveScopeInst>(U)->getVariableScope());
    }
    return true;
  };

  // If the parent scope is only used to resolve other scopes, record all the
  // scopes it is used to resolve.
  for (auto *paramUser : F->getParentScopeParam()->getUsers()) {
    // Unknown user of the parent scope, bail.
    if (!llvh::isa<GetParentScopeInst>(paramUser))
      return false;

    if (!collectUsedScopes(llvh::cast<GetParentScopeInst>(paramUser)))
      return false;
  }

  // The parent scope may also be accessed through GetClosureScopeInst, find
  // them and check how they are used.
  for (auto *funUser : F->getUsers()) {
    if (llvh::isa<GetClosureScopeInst>(funUser))
      if (!collectUsedScopes(llvh::cast<GetClosureScopeInst>(funUser)))
        return false;
  }

  // If the parent scopes are completely unused, hoist to the top level.
  if (usedVarScopes.empty())
    return hoistFunctionToTopLevel(F);

  VariableScope *newParentVarScope = oldParentVarScope;
  // Walk up until we find the first scope that is actually used.
  while (!usedVarScopes.count(newParentVarScope))
    newParentVarScope = newParentVarScope->getParentScope();
  assert(newParentVarScope && "Uses scopes that are not parents.");

  if (newParentVarScope == oldParentVarScope)
    return false;

  // We have identified a new parent, replace the function's parent with it.
  IRBuilder builder{F};
  for (auto *U : F->getUsers()) {
    if (auto *BCLI = llvh::dyn_cast<BaseCreateLexicalChildInst>(U)) {
      builder.setInsertionPoint(BCLI);
      BCLI->setVarScope(newParentVarScope);
      BCLI->setScope(builder.createResolveScopeInst(
          newParentVarScope,
          oldParentVarScope,
          llvh::cast<Instruction>(BCLI->getScope())));
    } else if (auto *BCI = llvh::dyn_cast<BaseCallInst>(U)) {
      // If the call does not have an environment set, there is nothing to do.
      if (llvh::isa<EmptySentinel>(BCI->getEnvironment()))
        continue;
      // Update the call environment by introducing a ResolveScopeInst.
      auto *env = llvh::cast<Instruction>(BCI->getEnvironment());
      builder.setInsertionPoint(BCI);
      BCI->setEnvironment(builder.createResolveScopeInst(
          newParentVarScope, oldParentVarScope, env));
    } else if (llvh::isa<CreateThisInst>(U)) {
      // Do nothing, CreateThisInst is not affected by the parent scope.
    } else {
      // The only other known user is GetClosureScopeInst. Update it to produce
      // the new parent.
      auto *GCSI = llvh::cast<GetClosureScopeInst>(U);
      GCSI->setVariableScope(newParentVarScope);
      // Update all the users (which must be ResolveScopeInsts) to reflect the
      // new incoming scope.
      for (auto *gcsiUser : GCSI->getUsers()) {
        llvh::cast<ResolveScopeInst>(gcsiUser)->setStartVarScope(
            newParentVarScope);
      }
    }
  }

  for (auto *paramUser : F->getParentScopeParam()->getUsers()) {
    // Update the GetParentScopeInst to point to the new parent.
    auto *GPSI = llvh::cast<GetParentScopeInst>(paramUser);
    GPSI->setVariableScope(newParentVarScope);
    // Update all the users (which must be ResolveScopeInsts) to reflect the
    // new incoming scope.
    for (auto *U : GPSI->getUsers())
      llvh::cast<ResolveScopeInst>(U)->setStartVarScope(newParentVarScope);
  }

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

    // Collect the functions that have this scope as their parent so we can
    // hoist them.
    llvh::SmallSetVector<Function *, 4> functionsToHoist;
    for (auto *U : VS->getUsers()) {
      if (auto *RSI = llvh::dyn_cast<ResolveScopeInst>(U)) {
        // Record any scope that is resolved starting from this one. We cannot
        // hoist past scopes that are used.
        if (RSI->getStartVarScope() == VS)
          uses.insert(RSI->getVariableScope());
      } else if (auto *BCLI = llvh::dyn_cast<BaseCreateLexicalChildInst>(U)) {
        // If the scope is the parent of some function, try hoisting that
        // function, since inner scopes and functions have already been hoisted.
        // This also increases the likelihood that we can hoist this scope.
        functionsToHoist.insert(BCLI->getFunctionCode());
      }
    }

    // While these functions all have the same parent scope, we do not know how
    // their creation might be nested. This is important because hoisting a
    // function that is created inside another function may make the latter
    // function eligible for hoisting. To address this, we iterate until we
    // reach a fixed point.
    bool localChanged;
    do {
      localChanged = false;
      for (auto *F : functionsToHoist)
        localChanged |= tryHoistFunction(F);
      changed |= localChanged;
    } while (localChanged);

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
