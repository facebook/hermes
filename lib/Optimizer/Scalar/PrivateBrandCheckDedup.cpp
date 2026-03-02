/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "private-brand-check-dedup"
#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Support/Statistic.h"

#include <variant>

STATISTIC(
    NumPrivateBrandCheckDedup,
    "Number of PrivateBrandCheck instructions eliminated");

namespace hermes {

namespace {

class BrandCheckDedupVisitor;

/// The key is a (object, brand) pair identifying a brand check.
using BrandCheckKey = std::pair<Value *, Value *>;

/// Maintains state per dominator tree node that's needed to check for
/// redundancy between private brand checks.
class StackNode : public DomTreeDFS::StackNode {
 public:
  inline StackNode(BrandCheckDedupVisitor *ctx, const DominanceInfoNode *n);

 private:
  /// Update the ScopedHashTable's scope on entry/exit of StackNode.
  hermes::ScopedHashTableScope<BrandCheckKey, std::monostate> scope_;
};

/// Simple depth-first walk of the dominator tree, eliminating redundant
/// PrivateBrandCheckInst instructions.
class BrandCheckDedupVisitor
    : public DomTreeDFS::Visitor<BrandCheckDedupVisitor, StackNode> {
 public:
  BrandCheckDedupVisitor(const DominanceInfo &DI)
      : DomTreeDFS::Visitor<BrandCheckDedupVisitor, StackNode>(DI) {}

  bool run() {
    return DFS();
  }

  bool processNode(StackNode *SN);

 private:
  friend StackNode;

  /// This is functionally a scoped set which keeps track of what pairs of
  /// objects and private brands have been verified so far.
  hermes::ScopedHashTable<BrandCheckKey, std::monostate> checkedBrands_{};
};

inline StackNode::StackNode(
    BrandCheckDedupVisitor *ctx,
    const DominanceInfoNode *n)
    : DomTreeDFS::StackNode(n), scope_{ctx->checkedBrands_} {}

bool BrandCheckDedupVisitor::processNode(StackNode *SN) {
  BasicBlock *BB = SN->node()->getBlock();
  bool changed = false;

  IRBuilder::InstructionDestroyer destroyer;

  for (auto &inst : *BB) {
    auto *PBC = llvh::dyn_cast<PrivateBrandCheckInst>(&inst);
    if (!PBC)
      continue;

    BrandCheckKey key{PBC->getObject(), PBC->getBrand()};

    // If a dominating check with the same key exists, this one is redundant.
    if (checkedBrands_.count(key)) {
      destroyer.add(PBC);
      changed = true;
      ++NumPrivateBrandCheckDedup;
      continue;
    }

    // Otherwise, record that this (object, brand) pair has been checked.
    checkedBrands_.try_emplace(key, {});
  }

  return changed;
}

} // namespace

Pass *createPrivateBrandCheckDedup() {
  class PrivateBrandCheckDedup : public FunctionPass {
   public:
    explicit PrivateBrandCheckDedup()
        : FunctionPass("PrivateBrandCheckDedup") {}
    ~PrivateBrandCheckDedup() override = default;

    bool runOnFunction(Function *F) override {
      DominanceInfo DI{F};
      BrandCheckDedupVisitor v{DI};
      return v.run();
    }
  };
  return new PrivateBrandCheckDedup();
}

} // namespace hermes

#undef DEBUG_TYPE
