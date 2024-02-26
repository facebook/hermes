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

  /// Decide the correct scope to use when dealing with given variable.
  Instruction *getScope(
      IRBuilder &builder,
      Variable *var,
      HBCCreateFunctionEnvironmentInst *captureScope) {
    llvh::Optional<int32_t> varDepth =
        scopeAnalysis_.getScopeDepth(var->getParent());
    llvh::Optional<int32_t> curDepth =
        scopeAnalysis_.getScopeDepth(builder.getFunction()->getFunctionScope());
    if (!varDepth || !curDepth) {
      // If we cannot identify the depth, this is unreachable, insert a dummy.
      return builder.createHBCResolveParentEnvironmentInst(
          var->getParent(),
          builder.getLiteralPositiveZero(),
          builder.getFunction()->getParentScopeParam());
    }
    if (int32_t diff = *curDepth - *varDepth) {
      assert(diff > 0 && "Variable is from an inner scope.");
      // If the variable is from an outer scope, resolve to it. Subtract one
      // from diff because the instruction will start from the parent of the
      // current function.
      return builder.createHBCResolveParentEnvironmentInst(
          var->getParent(),
          builder.getLiteralNumber(diff - 1),
          builder.getFunction()->getParentScopeParam());
    }
    // Now we know that the variable belongs to the current scope.
    // We are going to conservatively assume the variable might get
    // captured. Hence we use the newly created scope.
    // This will not cause performance issue as long as optimization
    // is enabled, because every variable will be moved to stack
    // if not being captured.
    return captureScope;
  }

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

    // Insert the environment creation at the beginning of the first basic
    // block, after any FirstInBlock instructions.
    auto insertAt = F->begin()->begin();
    while (insertAt->getSideEffect().getFirstInBlock())
      ++insertAt;
    builder.setInsertionPoint(&*insertAt);

    // All local captured variables will be stored in this scope (or
    // "environment").
    // It will also be used by all closures created in this function, even if
    // there are no captured variables in this function.
    // Closures need a new environment even without captured variables because
    // we currently use only the lexical nesting level to determine which parent
    // environment to use - we don't account for the case when an environment
    // may not be needed somewhere along the chain.
    HBCCreateFunctionEnvironmentInst *captureScope =
        builder.createHBCCreateFunctionEnvironmentInst(
            F->getFunctionScope(), F->getParentScopeParam());

    for (BasicBlock &BB : F->getBasicBlockList()) {
      for (auto I = BB.begin(), E = BB.end(); I != E; /* nothing */) {
        // Keep the reference and increment iterator first.
        Instruction *Inst = &*I;
        ++I;

        builder.setLocation(Inst->getLocation());

        switch (Inst->getKind()) {
          case ValueKind::LoadFrameInstKind: {
            auto *LFI = cast<LoadFrameInst>(Inst);
            auto *var = LFI->getLoadVariable();

            builder.setInsertionPoint(Inst);
            Instruction *scope = getScope(builder, var, captureScope);
            Instruction *newInst =
                builder.createHBCLoadFromEnvironmentInst(scope, var);

            Inst->replaceAllUsesWith(newInst);
            Inst->eraseFromParent();
            changed = true;
            break;
          }
          case ValueKind::StoreFrameInstKind: {
            auto *SFI = cast<StoreFrameInst>(Inst);
            auto *var = SFI->getVariable();
            auto *val = SFI->getValue();

            builder.setInsertionPoint(Inst);
            Instruction *scope = getScope(builder, var, captureScope);
            builder.createHBCStoreToEnvironmentInst(scope, val, var);

            Inst->eraseFromParent();
            changed = true;
            break;
          }
          case ValueKind::CreateFunctionInstKind: {
            auto *CFI = cast<CreateFunctionInst>(Inst);

            builder.setInsertionPoint(Inst);
            auto *newInst = builder.createHBCCreateFunctionInst(
                CFI->getFunctionCode(), captureScope);

            Inst->replaceAllUsesWith(newInst);
            Inst->eraseFromParent();
            changed = true;
            break;
          }
          case ValueKind::CreateGeneratorInstKind: {
            auto *CFI = cast<CreateGeneratorInst>(Inst);

            builder.setInsertionPoint(Inst);
            auto *newInst = builder.createHBCCreateGeneratorInst(
                CFI->getFunctionCode(), captureScope);

            Inst->replaceAllUsesWith(newInst);
            Inst->eraseFromParent();
            changed = true;
            break;
          }
          default:
            break;
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
