/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IRUtils.h"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
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

void updateIncomingPhiValues(
    BasicBlock *blockToFix,
    BasicBlock *previousBlock,
    BasicBlock *newBlock) {
  for (auto &inst : *blockToFix) {
    auto *phi = llvh::dyn_cast<PhiInst>(&inst);
    if (!phi)
      return;

    for (int i = 0, e = phi->getNumEntries(); i < e; i++) {
      auto entry = phi->getEntry(i);
      if (entry.second == previousBlock) {
        phi->updateEntry(i, entry.first, newBlock);
      }
    }
  }
}

void movePastFirstInBlock(IRBuilder &builder, BasicBlock *BB) {
  auto it = BB->begin();
  auto end = BB->end();
  // Skip all FirstInBlock insts.
  while (it != end && it->getSideEffect().getFirstInBlock())
    ++it;
  builder.setInsertionPoint(&*it);
}

std::pair<Instruction *, VariableScope *> getResolveScopeStart(
    Instruction *startScope,
    VariableScope *startVarScope,
    VariableScope *targetVarScope) {
  while (auto *CSI = llvh::dyn_cast<CreateScopeInst>(startScope)) {
    // We have found the target scope, exit.
    if (startVarScope == targetVarScope)
      break;

    // Advance to the parent of the CreateScopeInst. We know that it must be an
    // instruction, since we cannot resolve past the root scope.
    startScope = llvh::cast<Instruction>(CSI->getParentScope());
    startVarScope = startVarScope->getParentScope();
  }
  return {startScope, startVarScope};
}

/// Makes an entry block for \p F which only contains UnreachableInst.
/// Deletes the rest of the body of the function.
void replaceBodyWithUnreachable(Function *F) {
  IRBuilder builder(F);

  // Do nothing if the first instruction is already Unreachable.
  if (llvh::isa<UnreachableInst>(F->begin()->begin())) {
    return;
  }

  for (auto it = F->begin(), e = F->end(); it != e;) {
    auto *BB = &*it++;
    // No need to handle Phis because the whole body will be deleted.
    // There may still be uses of the block from other unreachable blocks.
    BB->replaceAllUsesWith(nullptr);
    // Erase this basic block.
    BB->eraseFromParent();
  }
  auto *unreachableBB = builder.createBasicBlock(F);
  builder.setInsertionBlock(unreachableBB);
  builder.createUnreachableInst();
}

void deleteBodyExceptEvalData(Function *F) {
  IRBuilder builder(F);

  // New block will contain Eval and UnreachableInst.
  auto *newFirstBB = builder.createBasicBlock(F);
  builder.setInsertionBlock(newFirstBB);
  auto *unreachableInst = builder.createUnreachableInst();

  // Delete all blocks, preserving the EvalCompilationDataInst.
  EvalCompilationDataInst *evalDataInst = nullptr;
  for (auto it = F->begin(), e = F->end(); it != e;) {
    auto *BB = &*it++;
    // Skip the new block.
    if (BB == newFirstBB)
      continue;
    // Move the EvalCompilationDataInst to the new block if we found it.
    // There's only one, so don't look in the block if we already found it.
    if (!evalDataInst) {
      for (auto &I : *BB) {
        if (llvh::isa<EvalCompilationDataInst>(&I)) {
          evalDataInst = llvh::cast<EvalCompilationDataInst>(&I);
          evalDataInst->moveBefore(unreachableInst);
          break;
        }
      }
    }
    // Delete the block.
    BB->replaceAllUsesWith(nullptr);
    BB->eraseFromParent();
  }
}

} // namespace hermes
