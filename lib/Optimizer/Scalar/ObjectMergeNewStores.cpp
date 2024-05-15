/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization collects StoreNewOwnPropertyInsts and merges them into a
/// single AllocObjectLiteral.
//===----------------------------------------------------------------------===//

#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#define DEBUG_TYPE "objectmergenewstores"

namespace hermes {
namespace {

/// Define a type for managing lists of StoreNewOwnPropertyInsts.
using StoreList = llvh::SmallVector<StoreNewOwnPropertyInst *, 4>;
/// Define a type for mapping a given basic block to the stores to a given
/// AllocObjectLiteralInst in that basic block.
using BlockUserMap = llvh::DenseMap<BasicBlock *, StoreList>;

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

/// \return an ordered list of stores to \p allocInst that are known to
/// always execute without any other intervening users.
StoreList collectStores(
    AllocObjectLiteralInst *allocInst,
    const BlockUserMap &userBasicBlockMap,
    const DominanceInfo &DI) {
  // Sort the basic blocks that contain users of allocInst by dominance.
  auto sortedBlocks = orderBlocksByDominance(
      DI, allocInst->getParent(), [&userBasicBlockMap](BasicBlock *BB) {
        return userBasicBlockMap.find(BB) != userBasicBlockMap.end();
      });
  // Iterate over the sorted blocks to collect StoreNewOwnPropertyInst users
  // until we encounter a nullptr indicating we should stop.
  StoreList instrs;
  for (BasicBlock *BB : sortedBlocks) {
    for (StoreNewOwnPropertyInst *I : userBasicBlockMap.find(BB)->second) {
      // If I is null, we cannot consider additional stores.
      if (!I)
        return instrs;
      instrs.push_back(I);
    }
  }
  return instrs;
}

/// Merge StoreNewOwnPropertyInsts into a single AllocObjectLiteralInst.
/// Non-literal values are set with placeholders and later patched with the
/// correct value.
/// \p allocInst the instruction to transform
/// \p users the ordered list of stores guaranteed to execute before any other
///    users of the allocInst. Any stores to a numeric property key should
///    appear *after* nonnumeric stores
bool mergeStoresToObjectLiteral(
    AllocObjectLiteralInst *allocInst,
    const StoreList &users) {
  uint32_t size = users.size();
  // Skip processing for objects that have no new stores.
  if (size == 0) {
    return false;
  }

  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);
  AllocObjectLiteralInst::ObjectPropertyMap propMap;
  // Keep track of if we have encountered a numeric key yet.
  bool hasSeenNumericKey = false;
  for (uint32_t i = 0; i < size; ++i) {
    StoreNewOwnPropertyInst *I = users[i];
    Literal *propKey = llvh::cast<Literal>(I->getProperty());
    auto *propVal = I->getStoredValue();
    hasSeenNumericKey |= llvh::isa<LiteralNumber>(propKey);
    if (auto *literalVal = llvh::dyn_cast<Literal>(propVal)) {
      propMap.push_back({propKey, literalVal});
      I->eraseFromParent();
    } else {
      // Place the correct key with a placeholder value in the buffer.
      propMap.push_back({propKey, builder.getLiteralNull()});
      builder.setLocation(I->getLocation());
      builder.setInsertionPoint(I);
      // Patch in the correct value to the object.
      if (hasSeenNumericKey) {
        builder.createStoreOwnPropertyInst(
            propVal, allocInst, propKey, IRBuilder::PropEnumerable::Yes);
      } else {
        builder.createPrStoreInst(
            propVal,
            I->getObject(),
            i,
            cast<LiteralString>(propKey),
            propVal->getType().isNonPtr());
      }
      I->eraseFromParent();
    }
  }

  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *alloc = builder.createAllocObjectLiteralInst(
      propMap, allocInst->getParentObject());
  allocInst->replaceAllUsesWith(alloc);
  allocInst->eraseFromParent();

  return true;
}

bool mergeNewStores(Function *F) {
  /// If we can still append to \p stores, check if the user \p U is an eligible
  /// store to \p A. If so, append it to \p stores, if not, append nullptr to
  /// indicate that subsequent users in the basic block should not be
  /// considered.
  auto tryAdd =
      [](AllocObjectLiteralInst *A, Instruction *U, StoreList &stores) {
        // If the store list has been terminated by a nullptr, we have already
        // encountered a non-SNOP user of A in this block. Ignore this user.
        if (!stores.empty() && !stores.back())
          return;
        auto *SI = llvh::dyn_cast<StoreNewOwnPropertyInst>(U);
        if (!SI || SI->getStoredValue() == A || !SI->getIsEnumerable()) {
          // A user that's not a StoreNewOwnPropertyInst storing into the object
          // created by allocInst. We have to stop processing here. Note that we
          // check the stored value instead of the target object so that we omit
          // the case where an object is stored into itself. While it should
          // technically be safe, this maintains the invariant that stop as soon
          // the allocated object is used as something other than the target of
          // a StoreNewOwnPropertyInst.
          stores.push_back(nullptr);
        } else {
          assert(
              SI->getObject() == A &&
              "SNOP using allocInst must use it as object or value");
          stores.push_back(SI);
        }
      };

  // For each basic block, collect an ordered list of stores into
  // AllocObjectInsts that should be considered for lowering into a buffer.
  llvh::DenseMap<AllocObjectLiteralInst *, BlockUserMap> allocUsers;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      for (size_t i = 0; i < I.getNumOperands(); ++i) {
        if (auto *A = llvh::dyn_cast<AllocObjectLiteralInst>(I.getOperand(i))) {
          // For now, we only consider merging StoreNewOwnPropertyInsts that are
          // writing into an empty object to start.
          if (A->getKeyValuePairCount() == 0)
            tryAdd(A, &I, allocUsers[A][&BB]);
        }
      }
    }
  }

  bool changed = false;
  DominanceInfo DI(F);
  for (const auto &[A, userBasicBlockMap] : allocUsers) {
    // Collect the stores that are guaranteed to execute before any other user
    // of this object.
    auto stores = collectStores(A, userBasicBlockMap, DI);
    changed |= mergeStoresToObjectLiteral(A, stores);
  }

  return changed;
}

} // namespace

Pass *createObjectMergeNewStores() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : hermes::FunctionPass("ObjectMergeNewStores") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *F) override {
      return mergeNewStores(F);
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
