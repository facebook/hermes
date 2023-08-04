/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "simplestackpromotion"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/Support/Debug.h"

STATISTIC(NumConstProm, "Number of loads of known constants promoted");
STATISTIC(NumVarCopy, "Number of variables copied to the stack");
STATISTIC(NumStoreOnlyVarDel, "Number of store only variables deleted");

namespace hermes {
namespace {
/// If \p var is only ever stored to with a single literal value, replace all
/// loads from it with that value, and delete all stores.
/// \return true if \p var was promoted.
bool tryPromoteConstVariable(Variable *var) {
  Value *storedVal = isStoreOnceVariable(var);

  // Not a single-store variable with a literal value, give up.
  if (!storedVal || !llvh::isa<Literal>(storedVal))
    return false;

  LLVM_DEBUG(
      llvh::dbgs() << "Promoting const variable " << var->getName()
                   << " with literal value " << storedVal->getKindStr()
                   << "\n");
  IRBuilder::InstructionDestroyer destroyer;

  // Delete all stores to the variable, and replace all loads with the literal.
  for (auto *U : var->getUsers()) {
    if (llvh::isa<LoadFrameInst>(U)) {
      U->replaceAllUsesWith(storedVal);
    } else {
      assert(llvh::isa<StoreFrameInst>(U) && "Access must be a load or store.");
    }
    destroyer.add(U);
  }
  NumConstProm++;
  return true;
}

/// If \p var is only ever stored to in its owning function \p func, create a
/// copy of \p var on the stack. Use this stack variable for any loads from \p
/// var in \p func. Note that the stores to the frame are not deleted, which
/// means that loads in inner functions will continue to work correctly.
/// \return true if the variable was promoted.
bool tryCopyToStack(Variable *var) {
  auto *func = var->getParent()->getFunction();
  bool hasLoadInOwningFunction = false;
  bool hasStoreInInnerFunction = false;
  for (auto *U : var->getUsers()) {
    bool owningFunc = U->getParent()->getParent() == func;
    if (llvh::isa<LoadFrameInst>(U))
      hasLoadInOwningFunction |= owningFunc;
    else
      hasStoreInInnerFunction |= !owningFunc;
  }

  // If the variable is stored to from an inner function, we cannot attempt to
  // create a copy of it on the stack. Note that this applies even if all stores
  // are in the same inner function, because recursive invocations of that
  // function need to modify the same frame variable.
  // Likewise, if it doesn't have any loads that can use the new stack value, we
  // don't need to create it. The latter prevents us from repeatedly creating
  // new stack variables every time the pass runs.
  if (hasStoreInInnerFunction || !hasLoadInOwningFunction)
    return false;

  LLVM_DEBUG(llvh::dbgs() << "Promoting Variable: " << var->getName() << "\n");

  IRBuilder builder(func->getParent());

  // AllocStack will be inserted at the very start of the function.
  builder.setInsertionPoint(&*func->begin()->begin());
  auto *stackVar = builder.createAllocStackInst(var->getName());

  IRBuilder::InstructionDestroyer destroyer;

  for (auto *U : var->getUsers()) {
    // Replace all loads in the owning function with loads from the stack.
    if (llvh::isa<LoadFrameInst>(U)) {
      if (U->getParent()->getParent() == func) {
        builder.setInsertionPoint(U);
        auto *loadStack = builder.createLoadStackInst(stackVar);
        U->replaceAllUsesWith(loadStack);
        destroyer.add(U);
      }
    } else {
      // Duplicate all stores so they also store to the stack.
      auto *store = llvh::cast<StoreFrameInst>(U);
      builder.setInsertionPoint(store);
      builder.createStoreStackInst(store->getValue(), stackVar);
    }
  }

  NumVarCopy++;
  return true;
}

/// Delete all stores to \p var if there are no loads from it.
/// \return true if stores to the variable were deleted.
bool tryDeleteStoreOnlyVariable(Variable *var) {
  if (!var->hasUsers())
    return false;
  for (auto *U : var->getUsers())
    if (!llvh::isa<StoreFrameInst>(U))
      return false;
  LLVM_DEBUG(
      llvh::dbgs() << "Deleting stores to store-only Variable: "
                   << var->getName() << "\n");
  IRBuilder::InstructionDestroyer destroyer;
  for (auto *U : var->getUsers())
    destroyer.add(llvh::cast<StoreFrameInst>(U));
  NumStoreOnlyVarDel++;
  return true;
}

/// Run SimpleStackPromotion on a single function.
/// Iterates all variables of \p func, replacing them usage of them with SSA and
/// stack values.
bool runOnFunction(Function *F) {
  bool changed = false;
  LLVM_DEBUG(
      llvh::dbgs() << "Promoting variables in " << F->getInternalNameStr()
                   << "\n");

  llvh::SmallVectorImpl<Variable *> &vars =
      F->getFunctionScopeDesc()->getMutableVariables();
  for (Variable *var : vars) {
    // Attempt to replace var with a literal if possible. Note that this removes
    // all loads and stores from var, so we can skip the rest of the pass.
    if (tryPromoteConstVariable(var)) {
      changed = true;
      continue;
    }
    // Try creating a copy of the variable on the stack if it is only ever
    // stored to in the owning function. This allows us to eliminate all loads
    // from the variable in the owning function.
    changed |= tryCopyToStack(var);
    // If the variable no longer has any loads at all, delete any remaining
    // stores. This may happen because the program never loads from the
    // variable, or because all loads from the variable were in the owning
    // function as well, and were therefore eliminated by the stack copy.
    changed |= tryDeleteStoreOnlyVariable(var);
  }

  // Delete variables without any users.
  for (Variable *&var : vars) {
    if (!var->hasUsers()) {
      Value::destroy(var);
      var = nullptr;
      changed = true;
    }
  }
  // Clean up the variable list to remove destroyed entries.
  llvh::erase_if(vars, [](Variable *var) { return !var; });

  return changed;
}

/// Promotes all non-captured variables into stack allocations.
/// Replaces usage of variables with known literal values.
/// Creates stack copies of variables that are only stored to from the owning
/// function, to allow local values to be forwarded.
bool runSimpleStackPromotion(Module *M) {
  bool changed = false;
  for (Function &func : *M)
    changed |= runOnFunction(&func);
  return changed;
}

} // namespace

std::unique_ptr<Pass> createSimpleStackPromotion() {
  class ThisPass : public ModulePass {
   public:
    explicit ThisPass() : ModulePass("SimpleStackPromotion") {}
    ~ThisPass() override = default;

    bool runOnModule(Module *M) override {
      return runSimpleStackPromotion(M);
    }
  };
  return std::make_unique<ThisPass>();
}
} // namespace hermes
#undef DEBUG_TYPE
