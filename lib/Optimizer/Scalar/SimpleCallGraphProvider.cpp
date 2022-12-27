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

/// The main function that computes caller-callee relationships.
void SimpleCallGraphProvider::initCallRelationships(Function *F) {
  // (a) Initialize the callsites map.
  llvh::DenseSet<BaseCallInst *> callSites;
  if (getCallSites(F, callSites)) {
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
      if (getCallees(CI, funcs)) {
        callees_.insert(std::make_pair(CI, funcs));
      }
    }
  }
}
