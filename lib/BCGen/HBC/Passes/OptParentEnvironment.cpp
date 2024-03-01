/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#define DEBUG_TYPE "OptParentEnvironment"

namespace hermes {
namespace hbc {

/// Optimize scope creation and resolution that operate on the parent
/// environment of the current function to smaller HBC instructions that read
/// the parent environment implicitly. This means that the resulting
/// instructions are smaller, and the read of the parent environment may be
/// eliminated.
static bool runOptParentEnvironment(Function *F) {
  bool changed = false;
  IRBuilder builder{F};
  IRBuilder::InstructionDestroyer destroyer;
  for (auto &BB : *F) {
    for (auto &I : BB) {
      if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(&I)) {
        // Convert a CreateScopeInst that creates a child of the current
        // function's parent scope to HBCCreateFunctionEnvironmentInst.
        if (llvh::isa<GetParentScopeInst>(CSI->getParentScope())) {
          builder.setInsertionPoint(CSI);
          builder.setLocation(CSI->getLocation());
          auto *newInst = builder.createHBCCreateFunctionEnvironmentInst(
              CSI->getVariableScope(), F->getParentScopeParam());
          CSI->replaceAllUsesWith(newInst);
          destroyer.add(CSI);
          changed = true;
        }
      } else if (auto *LRSI = llvh::dyn_cast<LIRResolveScopeInst>(&I)) {
        // Convert a ResolveScopeInst that starts from the parent scope
        // to HBCResolveParentEnvironmentInst.
        if (llvh::isa<GetParentScopeInst>(LRSI->getStartScope())) {
          builder.setInsertionPoint(LRSI);
          builder.setLocation(LRSI->getLocation());
          auto *newInst = builder.createHBCResolveParentEnvironmentInst(
              LRSI->getVariableScope(),
              LRSI->getNumLevels(),
              F->getParentScopeParam());
          LRSI->replaceAllUsesWith(newInst);
          destroyer.add(LRSI);
          changed = true;
        }
      }
    }
  }
  return changed;
}

Pass *createOptParentEnvironment() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("OptParentEnvironment") {}

    bool runOnFunction(Function *F) override {
      return runOptParentEnvironment(F);
    }
  };
  return new ThisPass();
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
