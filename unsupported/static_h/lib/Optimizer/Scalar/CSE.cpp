/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "cse"
#include "hermes/Optimizer/Scalar/CSE.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/STLExtras.h"
#include "llvh/ADT/ScopedHashTable.h"
#include "llvh/Support/RecyclingAllocator.h"

using namespace hermes;

STATISTIC(NumCSE, "Number of instructions CSE'd");

//===----------------------------------------------------------------------===//
//                                Simple Value
//===----------------------------------------------------------------------===//

namespace {
/// CSEValue - Instances of this struct represent available values in the
/// scoped hash table.
struct CSEValue {
  Instruction *inst_;

  CSEValue(Instruction *I) : inst_(I) {
    assert((isSentinel() || canHandle(I)) && "Inst can't be handled!");
  }

  bool isSentinel() const {
    return inst_ == llvh::DenseMapInfo<Instruction *>::getEmptyKey() ||
        inst_ == llvh::DenseMapInfo<Instruction *>::getTombstoneKey();
  }

  /// Return true if we know how to CSE this instruction.
  static bool canHandle(Instruction *Inst) {
    return isSimpleSideEffectFreeInstruction(Inst);
  }
};
} // end anonymous namespace

namespace llvh {
template <>
struct DenseMapInfo<CSEValue> {
  static inline CSEValue getEmptyKey() {
    return DenseMapInfo<hermes::Instruction *>::getEmptyKey();
  }
  static inline CSEValue getTombstoneKey() {
    return DenseMapInfo<hermes::Instruction *>::getTombstoneKey();
  }
  static unsigned getHashValue(CSEValue Val);
  static bool isEqual(CSEValue LHS, CSEValue RHS);
};
} // end namespace llvh

unsigned llvh::DenseMapInfo<CSEValue>::getHashValue(CSEValue Val) {
  return Val.inst_->getHashCode();
}

bool llvh::DenseMapInfo<CSEValue>::isEqual(CSEValue LHS, CSEValue RHS) {
  hermes::Instruction *LHSI = LHS.inst_, *RHSI = RHS.inst_;
  if (LHS.isSentinel() || RHS.isSentinel())
    return LHSI == RHSI;

  return LHSI->getKind() == RHSI->getKind() && LHSI->isIdenticalTo(RHSI);
}

//===----------------------------------------------------------------------===//
//                               CSE Interface
//===----------------------------------------------------------------------===//

namespace {

class CSEContext;

using CSEValueHTType = llvh::ScopedHashTableVal<CSEValue, Value *>;
using AllocatorTy =
    llvh::RecyclingAllocator<llvh::BumpPtrAllocator, CSEValueHTType>;
using ScopedHTType = llvh::ScopedHashTable<
    CSEValue,
    Value *,
    llvh::DenseMapInfo<CSEValue>,
    AllocatorTy>;

// StackNode - contains all the needed information to create a stack for doing
// a depth first traversal of the tree. This includes scopes for values and
// loads as well as the generation. There is a child iterator so that the
// children do not need to be store spearately.
class StackNode : public DomTreeDFS::StackNode<CSEContext> {
 public:
  inline StackNode(CSEContext *ctx, const DominanceInfoNode *n);

 private:
  /// RAII to create and pop a scope when the stack node is created and
  /// destroyed.
  ScopedHTType::ScopeTy scope_;
};

/// CSEContext - This pass does a simple depth-first walk of the dominator
/// tree, eliminating trivially redundant instructions.
class CSEContext : public DomTreeDFS::Visitor<CSEContext, StackNode> {
 public:
  CSEContext(const DominanceInfo &DT)
      : DomTreeDFS::Visitor<CSEContext, StackNode>(DT) {}

  bool run() {
    return DFS();
  }

  bool processNode(StackNode *SN);

 private:
  friend StackNode;

  /// AvailableValues - This scoped hash table contains the current values of
  /// all of our simple scalar expressions.  As we walk down the domtree, we
  /// look to see if instructions are in this. If so, we replace them with what
  /// we find, otherwise we insert them so that dominated values can succeed in
  /// their lookup.
  ScopedHTType availableValues_{};
};

inline StackNode::StackNode(CSEContext *ctx, const DominanceInfoNode *n)
    : DomTreeDFS::StackNode<CSEContext>(ctx, n),
      scope_{ctx->availableValues_} {}
} // end anonymous namespace

//===----------------------------------------------------------------------===//
//                             CSE Implementation
//===----------------------------------------------------------------------===//

bool CSEContext::processNode(StackNode *SN) {
  BasicBlock *BB = SN->node()->getBlock();
  bool changed = false;

  // Keep a list of instructions that should be deleted when the basic block
  // is processed.
  IRBuilder::InstructionDestroyer destroyer;

  // See if any instructions in the block can be eliminated.  If so, do it.  If
  // not, add them to AvailableValues.
  for (auto &Inst : *BB) {
    // If this is not a simple instruction that we can value number, skip it.
    if (!CSEValue::canHandle(&Inst))
      continue;

    // Now that we know we have an instruction we understand see if the
    // instruction has an available value.  If so, use it.
    if (Value *V = availableValues_.lookup(&Inst)) {
      Inst.replaceAllUsesWith(V);
      destroyer.add(&Inst);
      changed = true;
      ++NumCSE;
      continue;
    }

    // Otherwise, just remember that this value is available.
    availableValues_.insert(&Inst, &Inst);
  }

  return changed;
}

bool CSE::runOnFunction(Function *F) {
  DominanceInfo DT{F};
  CSEContext CCtx{DT};
  return CCtx.run();
}

Pass *hermes::createCSE() {
  return new CSE();
}

#undef DEBUG_TYPE
