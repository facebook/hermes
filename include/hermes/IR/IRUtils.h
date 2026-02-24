/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IRUTILS_H
#define HERMES_IR_IRUTILS_H

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"

namespace hermes {

class BasicBlock;
class Function;
class IRBuilder;

/// Delete all unreachable basic blocks from \p F.
/// \return true if anything was deleted, false otherwise.
bool deleteUnreachableBasicBlocks(Function *F);

/// In blockToFix, change all incoming Phi values from previousBlock to instead
/// come from newBlock.
void updateIncomingPhiValues(
    BasicBlock *blockToFix,
    BasicBlock *previousBlock,
    BasicBlock *newBlock);

/// Update the insertion point of the builder to the beginning of \p BB,
/// skipping past any initial FirstInBlock instructions.
void movePastFirstInBlock(IRBuilder &builder, BasicBlock *BB);

/// Try walking up the scope chain from \p startScope by traversing the parents
/// of CreateScopeInsts. If it can obtain the scope described by
/// \p targetVarScope, return that. Otherwise, walk past as many scope
/// creation instructions as possible, and return the last scope. This should be
/// used to ensure that a the start and target VariableScopes of a ResolveScope
/// are as close as possible.
std::pair<Instruction *, VariableScope *> getResolveScopeStart(
    Instruction *startScope,
    VariableScope *startVarScope,
    VariableScope *targetVarScope);

/// Makes an entry block for \p F which only contains UnreachableInst.
/// Deletes the rest of the body of the function.
void replaceBodyWithUnreachable(Function *F);

/// Deletes all instructions except for EvalCompilationDataInst,
/// ensuring it is the first instruction in the function.
/// Terminates the first block with UnreachableInst.
/// Moves the function to the compiled function list in the Module.
void deleteBodyExceptEvalData(Function *F);

/// Starting from the given \p entry block, use the given DominanceInfo to
/// examine all blocks that satisfy \p pred and attempt to construct the longest
/// possible ordered chain of blocks such that each block dominates the block
/// after it. This is done by traversing the dominance tree, until we encounter
/// two blocks that satisfy pred and do not have a dominance relationship. Note
/// that the last block in the chain will dominate all remaining blocks that
/// satisfy \p pred.
/// \return the longest ordered chain of blocks that satisfy \p pred.
template <typename Func>
static llvh::SmallVector<BasicBlock *, 4> orderBlocksByDominance(
    const DominanceInfo &DI,
    BasicBlock *entry,
    Func &&pred) {
  class OrderBlocksContext
      : public DomTreeDFS::Visitor<OrderBlocksContext, DomTreeDFS::StackNode> {
    /// The given predicate to determine whether a block should be considered.
    Func pred_;

    /// When we encounter branching, i.e. for a given basic block, if multiple
    /// of the basic blocks dominated by that basic block all contain users of
    /// allocInst_, we cannot append any of those basic blocks to
    /// sortedBasicBlocks_. Furthermore, we cannot append any other basic
    /// blocks to sortedBasicBlocks_ because the branch already exists.
    bool stopAddingBasicBlock_{false};

    /// List of basic blocks that satisfy the predicate, ordered by dominance
    /// relationship.
    llvh::SmallVector<BasicBlock *, 4> sortedBasicBlocks_{};

   public:
    OrderBlocksContext(
        const DominanceInfo &DI,
        BasicBlock *entryBlock,
        Func &&pred)
        : DomTreeDFS::Visitor<OrderBlocksContext, DomTreeDFS::StackNode>(DI),
          pred_(std::forward<Func>(pred)) {
      // Perform the DFS to populate sortedBasicBlocks_.
      this->DFS(this->DT_.getNode(entryBlock));
    }

    llvh::SmallVector<BasicBlock *, 4> get() && {
      return std::move(sortedBasicBlocks_);
    }

    /// Called by DFS recursively to process each node. Note that the return
    /// value isn't actually used.
    bool processNode(DomTreeDFS::StackNode *SN) {
      BasicBlock *BB = SN->node()->getBlock();
      // If BB does not satisfy the predicate, proceed to the next block.
      if (!pred_(BB))
        return false;

      while (!sortedBasicBlocks_.empty() &&
             !this->DT_.properlyDominates(sortedBasicBlocks_.back(), BB)) {
        // If the last basic block in the list does not dominate BB,
        // it means BB and that last basic block are in parallel branches
        // of previous basic blocks. We cannot doing any lowering into
        // any of these basic blocks. So we roll back one basic block,
        // and mark the fact that we can no longer append any more basic blocks
        // afterwards because of the existence of basic blocks.
        // The DFS process needs to continue, as we may roll back even more
        // basic blocks.
        sortedBasicBlocks_.pop_back();
        stopAddingBasicBlock_ = true;
      }
      if (!stopAddingBasicBlock_) {
        sortedBasicBlocks_.push_back(BB);
        return true;
      }
      return false;
    }
  };

  return OrderBlocksContext(DI, entry, std::forward<Func>(pred)).get();
}

} // namespace hermes

#endif
