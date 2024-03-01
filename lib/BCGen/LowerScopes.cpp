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

/// Lower ResolveScopeInst into LIRResolveScopeInst by analysing scopes to
/// determine their depth.
class LowerScopes {
  /// The module being lowered.
  Module *M_;

  /// Map recording the depth of scopes that we have already computed.
  /// Unreachable scopes are represented with UINT32_MAX.
  llvh::DenseMap<VariableScope *, uint32_t> scopeDepthMap_;

 public:
  LowerScopes(Module *M) : M_{M} {}

  /// Get the scope depth of the given VariableScope \p VS, or None if the scope
  /// is unreachable.
  OptValue<uint32_t> getScopeDepth(VariableScope *VS) {
    // Chain of scopes whose depth has not been computed yet.
    llvh::SmallVector<VariableScope *, 8> chain;

    // The depth of one past the last element in the chain. Initialize it to
    // UINT32_MAX which represents the depth of an unreachable scope.
    uint32_t baseDepth = UINT32_MAX;

    for (;;) {
      // If we have already computed the depth of this scope, end the loop.
      auto it = scopeDepthMap_.find(VS);
      if (it != scopeDepthMap_.end()) {
        baseDepth = it->second;
        break;
      }

      // Push this scope onto the chain so we will populate its depth later.
      chain.push_back(VS);

      // Find the instruction that created this scope, so we can determine its
      // parent.
      CreateScopeInst *creator = nullptr;
      for (Instruction *user : VS->getUsers()) {
        if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(user)) {
          creator = CSI;
          break;
        }
      }

      // This scope is never created, it must be dead.
      if (!creator)
        break;

      // This is a root scope, so set the baseDepth to 0.
      if (llvh::isa<EmptySentinel>(creator->getParentScope())) {
        baseDepth = 0;
        break;
      }

      // Advance to the parent scope.
      VS = llvh::cast<BaseScopeInst>(creator->getParentScope())
               ->getVariableScope();
    }

    // Populate the depth of the chain, and return.
    if (baseDepth == UINT32_MAX) {
      for (auto *cur : chain)
        scopeDepthMap_[cur] = UINT32_MAX;
      return llvh::None;
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
          OptValue<uint32_t> resDepth = getScopeDepth(RSI->getVariableScope());
          auto *startScope = llvh::cast<BaseScopeInst>(RSI->getStartScope());
          OptValue<uint32_t> startDepth =
              getScopeDepth(startScope->getVariableScope());

          // If either depth is not found, this is unreachable. Just use 0 as a
          // dummy depth.
          uint32_t diff = 0;
          if (resDepth && startDepth) {
            assert(
                *startDepth >= *resDepth &&
                "Start must be deeper than the target.");
            diff = *startDepth - *resDepth;
          }

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
