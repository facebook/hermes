/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"

namespace hermes {
namespace hbc {

bool InsertProfilePoint::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;
  uint16_t basicBlockIndex = 1;
  // Iterate from backward so that the first basic block will have largest
  // index. This enables the VM to size the counter table correctly on entry.
  for (auto it = F->rbegin(), e = F->rend(); it != e; ++it) {
    movePastFirstInBlock(builder, &*it);
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
