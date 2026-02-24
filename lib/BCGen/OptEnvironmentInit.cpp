/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/SmallPtrSet.h"

#define DEBUG_TYPE "OptEnvironmentInit"

STATISTIC(
    NumStoreUndefinedRemoved,
    "Number of store undefined instructions removed");

namespace hermes {

/// Eliminate any stores of undefined to newly created environments, since we
/// know the VM will always initialize slots in an environment to undefined.
/// WARNING: This pass violates IR invariants, because it removes
/// initializing stores to Variables. It should be run very late in the pipeline
/// after any passes that depend on the stores being present (e.g.
/// StackPromotion, TypeInference, etc.). (TODO: T204084366)
static bool optEnvironmentInit(Function *F) {
  bool changed = false;

  for (auto &BB : *F) {
    IRBuilder::InstructionDestroyer destroyer{};

    // Environments created in the current BB.
    llvh::SmallPtrSet<Value *, 2> createdEnvs{};

    // Environment slots that have already been written to.
    llvh::SmallPtrSet<Variable *, 8> writtenSlots{};

    for (auto &I : BB) {
      auto *inst = &I;

      if (auto *CSI = llvh::dyn_cast<CreateScopeInst>(inst)) {
        createdEnvs.insert(CSI);
        continue;
      }

      // Note that in practice we don't currently generate code to exercise
      // these checks below.
      if (auto *SFI = llvh::dyn_cast<StoreFrameInst>(inst)) {
        // Are we storing in one of the environments created in this BB?
        // If not, we could be storing anywhere, including in the created
        // envs, so unfortunately we have to abort. This could happen if the
        // same environment is resolved with a separate instructtion.
        if (!createdEnvs.count(SFI->getScope()))
          break;

        // If we are not storing undefined, mark the slot as written.
        if (!llvh::isa<LiteralUndefined>(SFI->getValue())) {
          writtenSlots.insert(SFI->getVariable());
          continue;
        }

        // If that slot has already been written to, ignore it.
        if (writtenSlots.count(SFI->getVariable()))
          continue;

        // Success! We can eliminate this store.
        ++NumStoreUndefinedRemoved;
        changed = true;
        destroyer.add(SFI);
        continue;
      }

      // If we encounter an instruction that can execute arbitrary code,
      // stop scanning this BB.
      if (inst->getSideEffect().getExecuteJS())
        break;
    }
  }

  return changed;
}

Pass *createOptEnvironmentInit() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("OptEnvironmentInit") {}
    ~ThisPass() override = default;
    bool runOnFunction(Function *F) override {
      return optEnvironmentInit(F);
    }
  };
  return new ThisPass();
}
} // namespace hermes

#undef DEBUG_TYPE
