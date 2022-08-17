/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/OptEnvironmentInit.h"

#include "hermes/IR/IRBuilder.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/SmallPtrSet.h"

#define DEBUG_TYPE "OptEnvironmentInit"

STATISTIC(
    NumStoreUndefinedRemoved,
    "Number of store undefined instructions removed");

namespace hermes {
namespace hbc {

bool OptEnvironmentInit::runOnFunction(Function *F) {
  IRBuilder builder{F};
  bool changed = false;

  for (auto &BB : *F) {
    IRBuilder::InstructionDestroyer destroyer{};

    // Environments created in the current BB.
    llvh::SmallPtrSet<Value *, 2> createdEnvs{};

    // Environment slots that have already been written to.
    llvh::SmallPtrSet<Variable *, 8> writtenSlots{};

    for (auto &I : BB) {
      auto *inst = &I;

      if (auto *CE = llvh::dyn_cast<HBCCreateEnvironmentInst>(inst)) {
        createdEnvs.insert(CE);
        continue;
      }

      // Note that in practice we don't currently generate code to exercise
      // these checks below.
      if (auto *SE = llvh::dyn_cast<HBCStoreToEnvironmentInst>(inst)) {
        // Are we storing in one of the environments created in this BB?
        // If not, we could be storing anywhere, including in the created
        // envs, so unfortunately we have to abort. This could happen if the
        // same environment is resolved with a separate instructtion.
        if (!createdEnvs.count(SE->getEnvironment()))
          break;

        // If we are not storing undefined, mark the slot as written.
        if (!llvh::isa<LiteralUndefined>(SE->getStoredValue())) {
          writtenSlots.insert(SE->getResolvedName());
          continue;
        }

        // If that slot has already been written to, ignore it.
        if (writtenSlots.count(SE->getResolvedName()))
          continue;

        // Success! We can eliminate this store.
        ++NumStoreUndefinedRemoved;
        changed = true;
        destroyer.add(SE);
        continue;
      }

      // If we encounter an instruction that can execute arbitrary code,
      // stop scanning this BB.
      if (inst->mayExecute())
        break;
    }
  }

  return changed;
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
