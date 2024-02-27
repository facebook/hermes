/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/Analysis.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#define DEBUG_TYPE "lowerscopes"

namespace hermes {

/// Lower LoadFrameInst, StoreFrameInst and CreateFunctionInst.
class LowerScopes {
  /// The module being lowered.
  Module *M_;

  /// Scope analysis to use when determining the depth of resolved scopes.
  FunctionScopeAnalysis scopeAnalysis_;

 public:
  LowerScopes(Module *M) : M_{M}, scopeAnalysis_{M->getTopLevelFunction()} {}

  bool run() {
    bool changed = false;
    for (auto &F : *M_)
      changed |= runOnFunction(&F);
    return changed;
  }

  bool runOnFunction(Function *F) {
    IRBuilder builder(F);
    bool changed = false;

    for (BasicBlock &BB : *F) {
      for (auto I = BB.begin(), E = BB.end(); I != E; /* nothing */) {
        // Keep the reference and increment iterator first.
        Instruction *Inst = &*I;
        ++I;

        if (auto *RSI = llvh::dyn_cast<ResolveScopeInst>(&*Inst)) {
          // Lower ResolveScopeInst into LIRResolveScopeInst by computing the
          // depth delta.
          llvh::Optional<int32_t> resDepth =
              scopeAnalysis_.getScopeDepth(RSI->getVariableScope());
          auto *startScope = llvh::cast<BaseScopeInst>(RSI->getStartScope());
          llvh::Optional<int32_t> startDepth =
              scopeAnalysis_.getScopeDepth(startScope->getVariableScope());

          // If either depth is not found, this is unreachable. Just use 0 as a
          // dummy depth.
          int32_t diff = !resDepth || !startDepth ? 0 : *startDepth - *resDepth;
          assert(diff >= 0 && "Variable is from an inner scope.");

          builder.setInsertionPoint(Inst);
          builder.setLocation(Inst->getLocation());
          auto *newInst = builder.createLIRResolveScopeInst(
              RSI->getVariableScope(),
              startScope,
              builder.getLiteralNumber(diff));
          Inst->replaceAllUsesWith(newInst);
          Inst->eraseFromParent();
          changed = true;
        }
      }
    }
    return changed;
  }
};

Pass *createLowerScopes() {
  class ThisPass : public ModulePass {
   public:
    explicit ThisPass() : ModulePass("LowerScopes") {}

    bool runOnModule(Module *M) override {
      return LowerScopes(M).run();
    }
  };

  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
