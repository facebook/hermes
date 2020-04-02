/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "tdzdedup"
#include "hermes/Optimizer/Scalar/TDZDedup.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/RecyclingAllocator.h"

STATISTIC(NumTDZFrameDedup, "Number of TDZ frame checks eliminated");
STATISTIC(NumTDZStackDedup, "Number of TDZ stack checks eliminated");
STATISTIC(NumTDZOtherDedup, "Number of TDZ other checks eliminated");
STATISTIC(NumTDZDedup, "Number of TDZ instructions eliminated");

namespace hermes {

using llvm::dbgs;
using llvm::isa;

namespace {

class TDZDedupContext;

using TDZDedupValueHTType = llvm::ScopedHashTableVal<Value *, Value *>;
using AllocatorTy =
    llvm::RecyclingAllocator<llvm::BumpPtrAllocator, TDZDedupValueHTType>;
using ScopedHTType = llvm::
    ScopedHashTable<Value *, Value *, llvm::DenseMapInfo<Value *>, AllocatorTy>;

// StackNode - contains all the needed information to create a stack for doing
// a depth first traversal of the tree. This includes scopes for values and
// loads as well as the generation. There is a child iterator so that the
// children do not need to be store spearately.
class StackNode : public DomTreeDFS::StackNode<TDZDedupContext> {
 public:
  inline StackNode(TDZDedupContext *ctx, DominanceInfoNode *n);

 private:
  /// RAII to create and pop a scope when the stack node is created and
  /// destroyed.
  ScopedHTType::ScopeTy scope_;
};

/// TDZDedupContext - This pass does a simple depth-first walk of the dominator
/// tree, eliminating trivially redundant instructions.
class TDZDedupContext : public DomTreeDFS::Visitor<TDZDedupContext, StackNode> {
 public:
  TDZDedupContext(Function *F, DominanceInfo &DT)
      : DomTreeDFS::Visitor<TDZDedupContext, StackNode>(DT), F_(F) {}

  bool run();

  bool processNode(StackNode *SN);

 private:
  friend StackNode;

  Function *const F_;

  /// All TDZ state variables are collected here.
  llvm::DenseSet<Value *> tdzState_{};

  /// AvailableValues - This scoped hash table contains the TDZ flags that are
  /// known to be true. It is instances of Variable, AllocStackInst, or
  /// theoretically an Instruction * if the value has been SSA-ed.
  /// TDZ values are unique in the sense that once they are set to true, or
  /// detected to be true, they can never go back to undefined. So, a check
  /// that is dominated by a another check of the same value or storing true
  /// to the same value, is always safe to eliminate.
  ScopedHTType availableValues_{};
};

inline StackNode::StackNode(TDZDedupContext *ctx, DominanceInfoNode *n)
    : DomTreeDFS::StackNode<TDZDedupContext>(ctx, n),
      scope_{ctx->availableValues_} {}

bool TDZDedupContext::run() {
  // First, collect all TDZ state variables.
  for (auto &BB : *F_) {
    for (auto &I : BB) {
      auto *TIU = dyn_cast<ThrowIfUndefinedInst>(&I);
      if (!TIU)
        continue;

      Value *checkedValue = TIU->getCheckedValue();
      Value *tdzStorage;

      if (auto *LFI = dyn_cast<LoadFrameInst>(checkedValue)) {
        tdzStorage = LFI->getSingleOperand();
      } else if (auto *LSI = dyn_cast<LoadStackInst>(checkedValue)) {
        tdzStorage = LSI->getSingleOperand();
      } else {
        continue;
      }

      tdzState_.insert(tdzStorage);
    }
  }

  return DFS();
}

bool TDZDedupContext::processNode(StackNode *SN) {
  BasicBlock *BB = SN->node()->getBlock();
  bool changed = false;

  // Keep a list of instructions that should be deleted when the basic block
  // is processed.
  IRBuilder::InstructionDestroyer destroyer;

  // See if any instructions in the block can be eliminated.  If so, do it.  If
  // not, add them to AvailableValues.
  for (auto &inst : *BB) {
    Value *checkedValue = nullptr;
    Value *tdzStorage = nullptr;
    if (auto *TIU = dyn_cast<ThrowIfUndefinedInst>(&inst)) {
      checkedValue = TIU->getCheckedValue();

      if (auto *LFI = dyn_cast<LoadFrameInst>(checkedValue)) {
        tdzStorage = LFI->getSingleOperand();
      } else if (auto *LSI = dyn_cast<LoadStackInst>(checkedValue)) {
        tdzStorage = LSI->getSingleOperand();
      } else {
        tdzStorage = checkedValue;
      }
    } else if (auto *SF = dyn_cast<StoreFrameInst>(&inst)) {
      // Check whether it is setting the target to non-undefined.
      if (SF->getValue()->getType().canBeUndefined())
        continue;
      tdzStorage = SF->getVariable();
      // Is the target a TDZ state variable?
      if (!tdzState_.count(tdzStorage))
        continue;
    } else if (auto *SS = dyn_cast<StoreStackInst>(&inst)) {
      // Check whether it is setting the target to non-undefined.
      if (SS->getValue()->getType().canBeUndefined())
        continue;
      tdzStorage = SS->getPtr();
      // Is the target a TDZ state variable?
      if (!tdzState_.count(tdzStorage))
        continue;
    } else {
      continue;
    }

    // If the tdz state is not already known to be set to true, add it to
    // the map.
    if (!availableValues_.count(tdzStorage)) {
      availableValues_.insert(tdzStorage, tdzStorage);
      continue;
    }

    // The TDZ state is known to be true, so we can eliminate the instruction,
    // which is either a check, or a store to set the state to true.
    destroyer.add(&inst);
    changed = true;
    ++NumTDZDedup;

    // Attempt to destroy the load too, to save work in other passes.
    if (checkedValue) {
      if (auto *LFI = dyn_cast<LoadFrameInst>(checkedValue)) {
        ++NumTDZFrameDedup;
        if (LFI->hasOneUser())
          destroyer.add(LFI);
      } else if (auto *LSI = dyn_cast<LoadStackInst>(checkedValue)) {
        ++NumTDZStackDedup;
        if (LSI->hasOneUser())
          destroyer.add(LSI);
      } else {
        ++NumTDZOtherDedup;
      }
    }
  }

  return changed;
}

} // end anonymous namespace

bool TDZDedup::runOnFunction(Function *F) {
  DominanceInfo DT{F};
  TDZDedupContext CCtx{F, DT};
  return CCtx.run();
}

Pass *createTDZDedup() {
  return new TDZDedup();
}

} // namespace hermes
