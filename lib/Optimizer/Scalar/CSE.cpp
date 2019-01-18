/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "cse"
#include "hermes/Optimizer/Scalar/CSE.h"
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

using namespace hermes;
using llvm::dbgs;
using llvm::isa;

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
    return inst_ == llvm::DenseMapInfo<Instruction *>::getEmptyKey() ||
        inst_ == llvm::DenseMapInfo<Instruction *>::getTombstoneKey();
  }

  /// Return true if we know how to CSE this instruction.
  static bool canHandle(Instruction *Inst) {
    return isSimpleSideEffectFreeInstruction(Inst);
  }
};
} // end anonymous namespace

namespace llvm {
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
} // end namespace llvm

unsigned llvm::DenseMapInfo<CSEValue>::getHashValue(CSEValue Val) {
  return Val.inst_->getHashCode();
}

bool llvm::DenseMapInfo<CSEValue>::isEqual(CSEValue LHS, CSEValue RHS) {
  hermes::Instruction *LHSI = LHS.inst_, *RHSI = RHS.inst_;
  if (LHS.isSentinel() || RHS.isSentinel())
    return LHSI == RHSI;

  return LHSI->getKind() == RHSI->getKind() && LHSI->isIdenticalTo(RHSI);
}

//===----------------------------------------------------------------------===//
//                               CSE Interface
//===----------------------------------------------------------------------===//

namespace {

/// CSEContext - This pass does a simple depth-first walk over the dominator
/// tree, eliminating trivially redundant instructions.
class CSEContext {
 public:
  CSEContext(Function &F) : DT_(&F) {}

  bool run();

 private:
  using CSEValueHTType = llvm::ScopedHashTableVal<CSEValue, Value *>;
  using AllocatorTy =
      llvm::RecyclingAllocator<llvm::BumpPtrAllocator, CSEValueHTType>;
  using ScopedHTType = llvm::ScopedHashTable<
      CSEValue,
      Value *,
      llvm::DenseMapInfo<CSEValue>,
      AllocatorTy>;

  /// Dominator tree of the function being processed.
  DominanceInfo DT_;

  /// AvailableValues - This scoped hash table contains the current values of
  /// all of our simple scalar expressions.  As we walk down the domtree, we
  /// look to see if instructions are in this. If so, we replace them with what
  /// we find, otherwise we insert them so that dominated values can succeed in
  /// their lookup.
  ScopedHTType availableValues_{};

  // StackNode - contains all the needed information to create a stack for doing
  // a depth first traversal of the tree. This includes scopes for values and
  // loads as well as the generation. There is a child iterator so that the
  // children do not need to be store spearately.
  class StackNode {
   public:
    StackNode(ScopedHTType &availableValues, DominanceInfoNode *n)
        : node_(n),
          childIter_(n->begin()),
          endIter_(n->end()),
          scope_(availableValues),
          processed_(false) {}

    // Accessors.
    DominanceInfoNode *node() {
      return node_;
    }
    // Return nullptr when the end is reached, or the next child otherwise.
    DominanceInfoNode *nextChild() {
      return childIter_ == endIter_ ? nullptr : *childIter_++;
    }
    bool isProcessed() {
      return processed_;
    }
    void process() {
      processed_ = true;
    }

   private:
    StackNode(const StackNode &) = delete;
    void operator=(const StackNode &) = delete;

    // Members.
    DominanceInfoNode *node_;
    DominanceInfoNode::iterator childIter_;
    DominanceInfoNode::iterator endIter_;
    /// RAII to create and pop a scope when the stack node is created and
    /// destroyed.
    ScopedHTType::ScopeTy scope_;
    bool processed_;
  };

  bool processNode(DominanceInfoNode *Node);
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
//                             CSE Implementation
//===----------------------------------------------------------------------===//

bool CSEContext::run() {
  llvm::SmallVector<StackNode *, 4> nodesToProcess;

  bool changed = false;

  // Process the root node.
  nodesToProcess.push_back(new StackNode(availableValues_, DT_.getRootNode()));

  // Process the stack.
  while (!nodesToProcess.empty()) {
    // Grab the first item off the stack. Set the current generation, remove
    // the node from the stack, and process it.
    StackNode *toProcess = nodesToProcess.back();

    // Check if the node needs to be processed.
    if (!toProcess->isProcessed()) {
      // Process the node.
      changed |= processNode(toProcess->node());
      // This node has been processed.
      toProcess->process();
    } else if (auto *child = toProcess->nextChild()) {
      // Push the next child onto the stack.
      nodesToProcess.push_back(new StackNode(availableValues_, child));
    } else {
      // It has been processed, and there are no more children to process,
      // so delete it and pop it off the stack and its scope in the
      // AvailableValues
      // will be popped.
      delete toProcess;
      nodesToProcess.pop_back();
    }
  }

  return changed;
}

bool CSEContext::processNode(DominanceInfoNode *Node) {
  BasicBlock *BB = Node->getBlock();
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
  CSEContext CCtx{*F};
  return CCtx.run();
}

Pass *hermes::createCSE() {
  return new CSE();
}
