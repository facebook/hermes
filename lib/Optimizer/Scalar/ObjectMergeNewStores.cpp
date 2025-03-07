/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization collects DefineNewOwnPropertyInsts and merges them into a
/// single AllocObjectLiteral.
//===----------------------------------------------------------------------===//

#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

#define DEBUG_TYPE "objectmergenewstores"

namespace hermes {
namespace {

/// Define a type for managing lists of DefineNewOwnPropertyInsts.
using StoreList = llvh::SmallVector<DefineNewOwnPropertyInst *, 4>;
/// Define a type for mapping a given basic block to the stores to a given
/// AllocObjectLiteralInst in that basic block.
using BlockUserMap = llvh::DenseMap<BasicBlock *, StoreList>;

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
  // Iterate over the sorted blocks to collect DefineNewOwnPropertyInst users
  // until we encounter a nullptr indicating we should stop.
  StoreList instrs;
  for (BasicBlock *BB : sortedBlocks) {
    for (DefineNewOwnPropertyInst *I : userBasicBlockMap.find(BB)->second) {
      // If I is null, we cannot consider additional stores.
      if (!I)
        return instrs;
      instrs.push_back(I);
    }
  }
  return instrs;
}

/// Merge DefineNewOwnPropertyInsts into a single AllocObjectLiteralInst.
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
    DefineNewOwnPropertyInst *I = users[i];
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
        builder.createDefineOwnPropertyInst(
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
        // encountered a non-define new user of A in this block. Ignore this
        // user.
        if (!stores.empty() && !stores.back())
          return;
        auto *newDef = llvh::dyn_cast<DefineNewOwnPropertyInst>(U);
        if (!newDef || newDef->getStoredValue() == A ||
            !newDef->getIsEnumerable()) {
          // A user that's not a DefineNewOwnPropertyInst storing into the
          // object created by allocInst. We have to stop processing here. Note
          // that we check the stored value instead of the target object so that
          // we omit the case where an object is stored into itself. While it
          // should technically be safe, this maintains the invariant that stop
          // as soon the allocated object is used as something other than the
          // target of a DefineNewOwnPropertyInst.
          stores.push_back(nullptr);
        } else {
          assert(
              newDef->getObject() == A &&
              "DefineNew using allocInst must use it as object or value");
          stores.push_back(newDef);
        }
      };

  // For each basic block, collect an ordered list of stores into
  // AllocObjectInsts that should be considered for lowering into a buffer.
  llvh::DenseMap<AllocObjectLiteralInst *, BlockUserMap> allocUsers;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      // Skip Phis for now, we will revisit them later.
      if (llvh::isa<PhiInst>(&I))
        continue;
      for (size_t i = 0; i < I.getNumOperands(); ++i) {
        if (auto *A = llvh::dyn_cast<AllocObjectLiteralInst>(I.getOperand(i))) {
          // For now, we only consider merging DefineNewOwnPropertyInsts that
          // are writing into an empty object to start.
          if (A->getKeyValuePairCount() == 0)
            tryAdd(A, &I, allocUsers[A][&BB]);
        }
      }
    }
  }

  // Phi users of the object allocation are special, they are not necessarily
  // dominated by the AllocObjectLiteralInst, but they do "leak" the object. To
  // reflect this, if a Phi uses an AllocObjectLiteral, the block associated
  // with it should terminate the traversal. Note that we only want this to
  // happen after that block has been traversed, which is why we do it in a
  // separate loop.
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      auto *phiInst = llvh::dyn_cast<PhiInst>(&I);
      // If we have visited all the Phis, proceed to the next block.
      if (!phiInst)
        break;
      for (unsigned i = 0, e = phiInst->getNumEntries(); i < e; ++i) {
        auto [val, block] = phiInst->getEntry(i);
        if (auto *AOL = llvh::dyn_cast<AllocObjectLiteralInst>(val)) {
          auto it = allocUsers.find(AOL);
          // If this is an object we are going to merge, it must not proceed
          // past the block that feeds into the Phi.
          if (it != allocUsers.end())
            it->second[block].push_back(nullptr);
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
