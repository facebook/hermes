/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "functionanalysis"

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/Support/Debug.h"

namespace hermes {

namespace {
/// Registers the call by setting the target/env operands if possible,
/// if they haven't been set yet.
/// \param call the Call instruction being analyzed.
/// \param callee the expected callee of the call instruction.
void registerCallsite(BaseCallInst *call, BaseCreateCallableInst *callee) {
  // Set the target/env operands if possible.
  if (llvh::isa<EmptySentinel>(call->getTarget())) {
    call->setTarget(callee->getFunctionCode());
    if (auto *create = llvh::dyn_cast<HBCCreateFunctionInst>(callee)) {
      // TODO: This can be done unconditionally once we store environments along
      // with all CreateFunctionInsts as well.
      call->setEnvironment(create->getEnvironment());
    }
  }
}

/// Find all callsites that could call a function via the closure created
/// by the \p create instruction and register them.
/// Looks at calls that use \p create as an operand themselves as well as
/// calls that load \p create via a variable which is stored to once.
void analyzeCreateCallable(BaseCreateCallableInst *create) {
  Function *F = create->getFunctionCode();
  for (Instruction *createUser : create->getUsers()) {
    // Closure is used as the callee operand.
    if (auto *call = llvh::dyn_cast<BaseCallInst>(createUser)) {
      if (!isDirectCallee(create, call)) {
        // F potentially escapes.
        F->getAttributes()._allCallsitesKnownInStrictMode = false;
      }
      if (call->getCallee() == create) {
        registerCallsite(call, create);
      }
      continue;
    }

    // Construction setup instructions can't leak the closure on their own,
    // but don't contribute to the call graph.
    if (isConstructionSetup(createUser, create)) {
      continue;
    }

    // Closure is stored to a variable, look at corresponding loads
    // to find callsites.
    if (auto *store = llvh::dyn_cast<StoreFrameInst>(createUser)) {
      Variable *var = store->getVariable();
      if (!isStoreOnceVariable(var)) {
        // Multiple stores to the variable, give up.
        F->getAttributes()._allCallsitesKnownInStrictMode = false;
        continue;
      }
      for (Instruction *varUser : var->getUsers()) {
        auto *load = llvh::dyn_cast<LoadFrameInst>(varUser);
        if (!load) {
          // Skip all stores, because they'll all be storing the same closure.
          assert(
              llvh::isa<StoreFrameInst>(varUser) &&
              "only Store and Load can use variables");
          continue;
        }
        // Find any calls using the load.
        for (Instruction *loadUser : load->getUsers()) {
          // Construction setup instructions can't leak the closure on their
          // own, but don't contribute to the call graph.
          if (isConstructionSetup(createUser, create)) {
            continue;
          }
          auto *call = llvh::dyn_cast<BaseCallInst>(loadUser);
          if (!call) {
            // Unknown instruction using the load, skip over the instruction
            // because it's not a call, but it could lead to an unknown call.
            F->getAttributes()._allCallsitesKnownInStrictMode = false;
            continue;
          }
          // Check if F potentially escapes via arguments to the call.
          if (!isDirectCallee(load, call)) {
            F->getAttributes()._allCallsitesKnownInStrictMode = false;
          }
          // Make sure the function is actually used as the callee operand.
          if (call->getCallee() == load) {
            registerCallsite(call, create);
          }
        }
      }
      continue;
    }

    // Unknown user, F could escape somewhere.
    F->getAttributes()._allCallsitesKnownInStrictMode = false;
  }
}

/// Find and register any callsites that can be found which call \p F.
void analyzeFunctionCallsites(Function *F) {
  if (F->getAttributes()._allCallsitesKnownInStrictMode) {
    return;
  }

  // Attempt to start from a position of knowing all callsites.
  F->getAttributes()._allCallsitesKnownInStrictMode = true;

  if (auto *newTargetParam = F->getNewTargetParam()) {
    // Uses of new.target can be used to leak the closure.
    // TODO: Allow certain instructions to use new.target.
    if (newTargetParam->hasUsers())
      F->getAttributes()._allCallsitesKnownInStrictMode = false;
  }

  for (Instruction *user : F->getUsers()) {
    if (auto *create = llvh::dyn_cast<BaseCreateCallableInst>(user)) {
      assert(
          create->getFunctionCode() == F &&
          "Function can only be used as the FunctionCode operand");
      analyzeCreateCallable(create);
      continue;
    }

    if (auto *call = llvh::dyn_cast<BaseCallInst>(user)) {
      // Ignore uses as call target.
      assert(
          call->getTarget() == F &&
          "invalid use of Function as operand of call");
      continue;
    }

    // Unknown user of Function.
    LLVM_DEBUG(
        llvh::dbgs() << "Unknown function user: " << user->getKindStr()
                     << '\n');
    F->getAttributes()._allCallsitesKnownInStrictMode = false;
  }
}

} // namespace

Pass *createFunctionAnalysis() {
  /// Analyze Function call graphs to update `target` operands and attributes.
  class FunctionAnalysis : public ModulePass {
   public:
    explicit FunctionAnalysis() : hermes::ModulePass("FunctionAnalysis") {}
    ~FunctionAnalysis() override = default;

    /// Create the call graph for \p mod by analyzing all potential callsites
    /// for all functions and populating the maps.
    /// If a callee is definitely known, populate the target/env operands on the
    /// \c BaseCallInst.
    bool runOnModule(Module *M) override {
      for (Function &F : *M) {
        analyzeFunctionCallsites(&F);
      }
      return true;
    }
  };

  return new FunctionAnalysis();
}

} // namespace hermes
#undef DEBUG_TYPE
