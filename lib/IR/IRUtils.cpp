/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IRUtils.h"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

namespace hermes {

/// Process the deletion of the basic block, and erase it.
static void deleteBasicBlock(BasicBlock *B) {
  // Remove all uses of this basic block.

  // Copy the uses of the block aside because removing the users invalidates the
  // iterator.
  Value::UseListTy users(B->getUsers().begin(), B->getUsers().end());

  // Remove the block from all Phi instructions referring to it. Note that
  // reachable blocks could end up with Phi instructions referring to
  // unreachable blocks.
  for (auto *I : users) {
    if (auto *Phi = llvh::dyn_cast<PhiInst>(I)) {
      Phi->removeEntry(B);
      continue;
    }
  }

  // There may still be uses of the block from other unreachable blocks.
  B->replaceAllUsesWith(nullptr);
  // Erase this basic block.
  B->eraseFromParent();
}

bool deleteUnreachableBasicBlocks(Function *F) {
  bool changed = false;

  // Visit all reachable blocks.
  llvh::SmallPtrSet<BasicBlock *, 16> visited;
  llvh::SmallVector<BasicBlock *, 32> workList;

  workList.push_back(&*F->begin());
  while (!workList.empty()) {
    auto *BB = workList.pop_back_val();
    // Already visited?
    if (!visited.insert(BB).second)
      continue;

    for (auto *succ : successors(BB))
      workList.push_back(succ);
  }

  // Delete all blocks that weren't visited.
  for (auto it = F->begin(), e = F->end(); it != e;) {
    auto *BB = &*it++;
    if (!visited.count(BB)) {
      deleteBasicBlock(BB);
      changed = true;
    }
  }

  return changed;
}

} // namespace hermes
