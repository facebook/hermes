/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

/// Auxiliary method to figure out the Functions that a given CallInst may
/// be calling. Returns true if we have a complete set, false if there are
/// unknown callees.
static bool identifyCallees(
    BaseCallInst *CI,
    llvh::DenseSet<Function *> &callees) {
  Value *callee = CI->getCallee();

  if (llvh::isa<NormalFunction>(callee) ||
      llvh::isa<GeneratorFunction>(callee)) {
    auto *F = cast<Function>(callee);
    callees.insert(F);
    return true;
  }
  if (auto *CFI = llvh::dyn_cast<BaseCreateCallableInst>(callee)) {
    callees.insert(CFI->getFunctionCode());
    return true;
  }
  if (auto LFI = llvh::dyn_cast<LoadFrameInst>(callee)) {
    Variable *V = LFI->getLoadVariable();
    if (V->getParent()->isGlobalScope()) {
      return false;
    }
    for (auto *U : V->getUsers()) {
      if (llvh::isa<LoadFrameInst>(U)) {
        // Skip over a load frame using that address
        continue;
      }
      auto *SF = llvh::dyn_cast<StoreFrameInst>(U);
      if (!SF) {
        // Unknown inst using that address ... bail out.
        return false;
      }
      auto *CFI = llvh::dyn_cast<BaseCreateCallableInst>(SF->getValue());
      if (!CFI) {
        // Currently look only direct stores of created functions.
        return false;
      }
      callees.insert(CFI->getFunctionCode());
    }
    return true;
  }

  // If callee is any other ValueKind, we don't know.
  return false;
}

/// Auxiliary method to figure out the call sites at which F may be
/// invoked.  Returns true if the complete set of call sites is known.
static bool identifyCallsites(
    Function *F,
    llvh::DenseSet<BaseCallInst *> &callSites) {
  for (auto *CU : F->getUsers()) {
    if (auto *CI = llvh::dyn_cast<BaseCallInst>(CU)) {
      if (!isDirectCallee(F, CI))
        return false;
      callSites.insert(CI);
    } else if (auto *CFI = llvh::dyn_cast<BaseCreateCallableInst>(CU)) {
      for (auto *CL : CFI->getUsers()) {
        auto *CI = llvh::dyn_cast<BaseCallInst>(CL);
        if (!CI)
          return false;

        if (!isDirectCallee(CFI, CI))
          return false;
        callSites.insert(CI);
      }
    } else {
      return false;
    }
  }
  return true;
}

/// The main function that computes caller-callee relationships.
void SimpleCallGraphProvider::initCallRelationships(Function *F) {
  // (a) Initialize the callsites map.
  llvh::DenseSet<BaseCallInst *> callSites;
  if (identifyCallsites(F, callSites)) {
    callsites_.insert(std::make_pair(F, callSites));
  }

  // (b) Initialize the callees map.
  for (auto &bbit : *F) {
    for (auto &it : bbit) {
      Instruction *I = &it;

      auto *CI = llvh::dyn_cast<BaseCallInst>(I);
      if (!CI)
        continue;

      llvh::DenseSet<Function *> funcs;
      if (identifyCallees(CI, funcs)) {
        callees_.insert(std::make_pair(CI, funcs));
      }
    }
  }
}
