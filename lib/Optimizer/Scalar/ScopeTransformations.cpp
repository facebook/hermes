/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "scopetransformations"

#include "hermes/Optimizer/Scalar/ScopeTransformations.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

namespace hermes {

void ScopeMerger::mergeInto(Function *F, ScopeDesc *parent, ScopeDesc *child) {
  // Transfer over variables.
  auto &newHome = parent->getMutableVariables();
  auto &oldHome = child->getMutableVariables();

  newHome.reserve(newHome.size() + oldHome.size());
  for (Variable *V : oldHome) {
    newHome.push_back(V);
    V->setParent(parent);
  }
  oldHome.clear();

  // Transfer over the inner scopes.
  auto &innerScopesChild = child->getMutableInnerScopes();
  auto &innerScopesParent = parent->getMutableInnerScopes();

  innerScopesParent.reserve(innerScopesParent.size() + innerScopesChild.size());
  for (ScopeDesc *grandchild : innerScopesChild) {
    innerScopesParent.push_back(grandchild);
    grandchild->relocateTo(parent);
  }
  innerScopesChild.clear();

  /// Finds the ScopeCreationInst that creates the given scope \p S.
  auto findCreatorOf = [](ScopeDesc *S) -> ScopeCreationInst * {
    ScopeCreationInst *creator{};
    for (Instruction *user : S->getUsers()) {
      if (auto *SCI = llvh::dyn_cast<ScopeCreationInst>(user)) {
        assert(!creator && "Multiple instructions creating scope");
        creator = SCI;
      }
    }
    return creator;
  };

  ScopeCreationInst *createParent = findCreatorOf(parent);
  ScopeCreationInst *createChild = findCreatorOf(child);

  assert(
      (createParent || !createChild) &&
      "Inner scope can't be created without parent scope.");
  if (createParent && createChild) {
    assert(!llvh::isa<CreateScopeInst>(createChild));
    createChild->replaceAllUsesWith(createParent);
    createChild->eraseFromParent();
  }

  // Replace all uses of child with parent.
  child->replaceAllUsesWith(parent);

  // Add a child -> parent mapping in mergedMap_. There will still be references
  // to child in F's instructions' SourceLevelScope, which will be fixed up
  // before the optimization completes.
  assert(mergedMap_.count(child) == 0);
  mergedMap_[child] = parent;
}

/// \return trus if \p var has any users outside of \p F.
static bool escapes(Function *F, Variable *var) {
  assert(
      F == var->getParent()->getFunction() &&
      "Variable should have been transferred.");

  for (Value *User : var->getUsers()) {
    if (auto *instr = llvh::dyn_cast<Instruction>(User)) {
      if (instr->getParent()->getParent() != F) {
        return true;
      }
    }
  }

  return false;
}

/// \return true is \p scopeDesc has any variables that are used outside of
/// \p F.
static bool hasAtLeastOneEscapingVar(Function *F, ScopeDesc *scopeDesc) {
  for (Variable *var : scopeDesc->getVariables()) {
    if (escapes(F, var)) {
      return true;
    }
  }

  return false;
}

ScopeMerger::HasEscapingVars ScopeMerger::optimizeScope(
    Function *F,
    ScopeDesc *scopeDesc) {
  assert(
      scopeDesc->getFunction() == F &&
      "should not try to optimize other function's scopes.");

  // See if any of scopeDesc' variables escape.
  bool scopeDescHasEscapingVar = hasAtLeastOneEscapingVar(F, scopeDesc);

  auto &childrenScopes = scopeDesc->getMutableInnerScopes();
  std::vector<bool> merged(childrenScopes.size());
  // mergeInto (below) will append to S's inner scopes array (aka
  // childrenScopes); thus this loop can't use iterators (because the array
  // could be reallocated). It should also "cache" the initial number of
  // scopeDesc' inner scopes to avoid iterating over scopes that were transfered
  // during mergeInto.
  for (size_t i = 0, end = childrenScopes.size(); i < end; ++i) {
    ScopeDesc *childScope = childrenScopes[i];
    // childScope is a scope that belongs to another function, so it can't be
    // optimized here.
    if (childScope->getFunction() != F) {
      continue;
    }

    // Optimize the inner scope childScope first. This will merge childScope's
    // inner scopes recursively.
    HasEscapingVars childScopeHasEscapingVars = optimizeScope(F, childScope);

    // Scopes can safely be merged if they are not "dynamic" with escaping vars.
    // Non-dynamic scopes are statically produced during compilation; and
    // dynamic scopes without escaping vars are not needed at runtime as none of
    // their variables need to be stored in Environments. Note that optimizing
    // childScope first means that childScope will be optimized by the time
    // we're deciding if it should be merged with scopeDesc. For example:
    //
    // function f() {
    //   arr = []
    //   for (...) {
    //     if (...) {}
    //       let a = 20;
    //       arr.push(() => a)
    //     }
    //   }
    //   return arr;
    // }
    //
    // The scope of the if statement has an escaping var, but it is not dynamic.
    // Thus, it is merged with the for body scope. After merging, the if
    // statement scope is removed, and the for body scope (which is dynamic)
    // contains a (which escapes). This means the for body scope can't be merged
    // with F's scope, and the optimization completes.
    if (!(childScope->getDynamic() &&
          childScopeHasEscapingVars == HasEscapingVars::Yes)) {
      mergeInto(F, scopeDesc, childScope);
      // Now that childScope is merged into scopeDesc, scopeDesc could have
      // escaping vars (if childScope used to have escaping vars).
      if (childScopeHasEscapingVars == HasEscapingVars::Yes) {
        scopeDescHasEscapingVar = true;
      }
      // Mark the scope as merged.
      merged[i] = true;
    }
  }

  // Now erase the merged scopes. Do so by compacting the childrenScopes array,
  // overwriting scopes that have been merged.
  size_t curr = 0;
  for (size_t i = 0, end = childrenScopes.size(); i < end; ++i) {
    if (i >= merged.size() || !merged[i]) {
      childrenScopes[curr++] = childrenScopes[i];
    }
  }
  childrenScopes.resize(curr);

  return scopeDescHasEscapingVar ? HasEscapingVars::Yes : HasEscapingVars::No;
}

ScopeDesc *ScopeMerger::mergedScope(ScopeDesc *scopeDesc) {
  auto it = mergedMap_.find(scopeDesc);
  if (it == mergedMap_.end()) {
    return scopeDesc;
  }

  // Update mergedMap_ so the next queries for scopeDesc can be answered fast.
  assert(it->second != scopeDesc);
  it->second = mergedScope(it->second);
  return it->second;
}

void ScopeMerger::updateSourceLevelScopes(Function *F) {
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      if (auto *S = I.getSourceLevelScope()) {
        I.setSourceLevelScope(mergedScope(S));
      }
    }
  }

  for (auto [childScope, mergedScope] : mergedMap_) {
    (void)mergedScope;
    Value::destroy(childScope);
  }
}

bool ScopeMerger::runOnFunction(Function *F) {
  mergedMap_.clear();
  optimizeScope(F, F->getFunctionScopeDesc());
  updateSourceLevelScopes(F);
  return !mergedMap_.empty();
}
} // namespace hermes
