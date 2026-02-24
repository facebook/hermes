/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/LowerScopes.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#define DEBUG_TYPE "lowerscopes"

namespace hermes {

/// Lower ResolveScopeInst into LIRResolveScopeInst by analysing scopes to
/// determine their depth.
class LowerScopes {
  /// The module being lowered.
  Module *M_;

  /// Map recording the depth of scopes that we have already computed.
  llvh::DenseMap<VariableScope *, uint32_t> scopeDepthMap_;

 public:
  LowerScopes(Module *M) : M_{M} {}

  /// Get the scope depth of the given VariableScope \p VS.
  uint32_t getScopeDepth(VariableScope *VS) {
    // Chain of scopes whose depth has not been computed yet.
    llvh::SmallVector<VariableScope *, 8> chain;

    // The depth of one past the last element in the chain.
    uint32_t baseDepth = 0;

    for (; VS; VS = VS->getParentScope()) {
      // If we have already computed the depth of this scope, end the loop.
      auto it = scopeDepthMap_.find(VS);
      if (it != scopeDepthMap_.end()) {
        baseDepth = it->second;
        break;
      }

      // Push this scope onto the chain so we will populate its depth later.
      chain.push_back(VS);
    }

    for (auto *cur : llvh::reverse(chain))
      scopeDepthMap_[cur] = ++baseDepth;
    return baseDepth;
  }

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
          uint32_t resDepth = getScopeDepth(RSI->getVariableScope());
          uint32_t startDepth = getScopeDepth(RSI->getStartVarScope());

          assert(
              startDepth >= resDepth &&
              "Start must be deeper than the target.");
          uint32_t diff = startDepth - resDepth;

          builder.setInsertionPoint(Inst);
          builder.setLocation(Inst->getLocation());
          auto *newInst = builder.createLIRResolveScopeInst(
              RSI->getVariableScope(),
              RSI->getStartScope(),
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
