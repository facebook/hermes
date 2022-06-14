/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

namespace hermes {
namespace hbc {

Instruction *InsertProfilePoint::findInsertionPoint(BasicBlock &BB) const {
  for (auto &inst : BB.getInstList()) {
    /// CatchInst/TryEndInst/PhiInst must be the first instructions
    /// in a basic block.
    if (!llvh::isa<CatchInst>(inst) && !llvh::isa<TryEndInst>(inst) &&
        !llvh::isa<PhiInst>(inst)) {
      return &inst;
    }
  }
  llvm_unreachable("terminator instruction is missing in basic block.");
  return nullptr;
}

bool InsertProfilePoint::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;
  uint16_t basicBlockIndex = 1;
  // Iterate from backward so that the first basic block will have largest
  // index. This enables the VM to size the counter table correctly on entry.
  for (auto it = F->rbegin(), e = F->rend(); it != e; ++it) {
    builder.setInsertionPoint(findInsertionPoint(*it));
    builder.createHBCProfilePointInst(basicBlockIndex);
    // If there are 2^16 basic blocks use index zero for all the overflowed
    // blocks.
    if (basicBlockIndex != 0) {
      ++basicBlockIndex;
    }
    changed = true;
  }
  return changed;
}

} // namespace hbc
} // namespace hermes
