/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "tdzdedup"
#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseSet.h"

STATISTIC(NumTDZDedup, "Number of TDZ instructions eliminated");

namespace hermes {

namespace {

class TDZDedupContext;

/// States in order from less initialized to more initialized. The order matters
/// and is used for comparisons. The initial state "Missing" means that we have
/// not seen any stores to the value yet.
/// The valid transitions can only increase the ValueState into a "better"
/// state:
/// - Missing -> Empty -> Uninit -> Initialized
/// - Missing -> Empty -> Initialized
/// - Missing -> Uninit -> Initialized
/// - Missing -> Initialized
enum class ValueState {
  /// We have not seen any stores to the value. This is always the starting
  /// state.
  Missing,
  /// The value could be empty or better.
  Empty,
  /// The value could be uninit or better.
  Uninit,
  /// The value has been initialized.
  Initialized,
};

// StackNode - contains all the needed information to create a stack for doing
// a depth first traversal of the tree. This includes scopes for values and
// loads as well as the generation. There is a child iterator so that the
// children do not need to be store spearately.
class StackNode : public DomTreeDFS::StackNode {
 public:
  inline StackNode(TDZDedupContext *ctx, const DominanceInfoNode *n);

 private:
  /// RAII to create and pop a scope when the stack node is created and
  /// destroyed.
  ScopedHashTableScope<Value *, ValueState> scope_;
};

/// TDZDedupContext - This pass does a simple depth-first walk of the dominator
/// tree, eliminating trivially redundant ThrowIf instructions.
class TDZDedupContext : public DomTreeDFS::Visitor<TDZDedupContext, StackNode> {
  friend StackNode;

  Function *const F_;
  IRBuilder builder_;

  /// All TDZ variables are collected here.
  llvh::DenseSet<Value *> tdzStorage_{};

  /// AvailableValues - This scoped hash table conceptually contains the
  /// TDZ-obeying values and their state. It maps from instances of Variable,
  /// AllocStackInst, or theoretically an Instruction * (if the value has been
  /// SSA-ed), to \c ValueState.
  ///
  /// Values can transition between these states as we walk the dominator tree.
  /// Due to the recursive nature of the dominator walk, we must be careful
  /// how we update them. If the value is in the current scope, we update its
  /// value in-place. Otherwise, we insert a new key/value in the current scope.
  ///
  /// We eliminate checks to values whose state makes the check redundant.
  ScopedHashTable<Value *, ValueState> availableValues_{};

 public:
  TDZDedupContext(Function *F, DominanceInfo &DT)
      : DomTreeDFS::Visitor<TDZDedupContext, StackNode>(DT),
        F_(F),
        builder_(F) {}

  bool run();
  bool processNode(StackNode *SN);

 private:
  /// Handle stores to frame or stack. If the target is not a TDZ variable, do
  /// nothing.
  void processStore(Value *tdzStorage, Value *storedValue);

  /// Handle ThrowIf instructions.
  bool processThrowIf(
      ThrowIfInst *TI,
      IRBuilder::InstructionDestroyer &destroyer);
  /// Remove a ThrowIf instruction that has been deemed unnecessary.
  bool eliminateThrowIf(
      ThrowIfInst *TI,
      IRBuilder::InstructionDestroyer &destroyer);
};

inline StackNode::StackNode(TDZDedupContext *ctx, const DominanceInfoNode *n)
    : DomTreeDFS::StackNode(n), scope_{ctx->availableValues_} {}

bool TDZDedupContext::run() {
  // First, collect all TDZ variables.
  for (auto &BB : *F_) {
    for (auto &I : BB) {
      auto *TIU = llvh::dyn_cast<ThrowIfInst>(&I);
      if (!TIU)
        continue;

      Value *checkedValue = TIU->getCheckedValue();
      Value *tdzStorage;

      if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(checkedValue)) {
        tdzStorage = LFI->getLoadVariable();
      } else if (auto *LSI = llvh::dyn_cast<LoadStackInst>(checkedValue)) {
        tdzStorage = LSI->getSingleOperand();
      } else {
        continue;
      }

      tdzStorage_.insert(tdzStorage);
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

  // Check for instructions that affect TDZ value states: ThrowIf and
  // StoreFrame/StoreStack.
  for (auto &inst : *BB) {
    if (auto *TI = llvh::dyn_cast<ThrowIfInst>(&inst))
      changed |= processThrowIf(TI, destroyer);
    else if (auto *SF = llvh::dyn_cast<StoreFrameInst>(&inst))
      processStore(SF->getVariable(), SF->getValue());
    else if (auto *SS = llvh::dyn_cast<StoreStackInst>(&inst))
      processStore(SS->getPtr(), SS->getValue());
  }

  return changed;
}

void TDZDedupContext::processStore(Value *tdzStorage, Value *storedValue) {
  // Is the target a TDZ state variable?
  if (!tdzStorage_.count(tdzStorage))
    return;

#ifndef NDEBUG
  ValueState oldState = availableValues_.lookup(tdzStorage);
#endif

  // Check whether it is setting the target to Empty or Uninit, and record the
  // state accordingly.
  ValueState newState;
  if (storedValue->getType().canBeEmpty()) {
    assert(
        oldState < ValueState::Empty &&
        "TDZ variable can't be set to Empty twice");
    newState = ValueState::Empty;
  } else if (storedValue->getType().canBeUninit()) {
    assert(
        oldState < ValueState::Uninit &&
        "TDZ variable can't be set to Uninit twice");
    newState = ValueState::Uninit;
  } else if (availableValues_.lookup(tdzStorage) != ValueState::Initialized) {
    newState = ValueState::Initialized;
  } else {
    return;
  }

  availableValues_.setInCurrentScope(tdzStorage, newState);
}

bool TDZDedupContext::processThrowIf(
    ThrowIfInst *TI,
    IRBuilder::InstructionDestroyer &destroyer) {
  // A ThrowIfInst always discards a type, and a value can never transition back
  // from Uninit to Empty.
  ValueState newState = TI->getType().canBeUninit() ? ValueState::Uninit
                                                    : ValueState::Initialized;

  // The storage containing the value that can potentially be empty.
  Value *tdzStorage;
  auto *checkedValue = TI->getCheckedValue();
  if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(checkedValue)) {
    tdzStorage = LFI->getLoadVariable();
  } else if (auto *LSI = llvh::dyn_cast<LoadStackInst>(checkedValue)) {
    tdzStorage = LSI->getSingleOperand();
  } else {
    tdzStorage = checkedValue;
  }

  ValueState oldState = availableValues_.lookup(tdzStorage);
  // If the check doesn't reveal any additional information (doesn't move the
  // value into a more initialized state), we can delete it.
  if (newState <= oldState)
    return eliminateThrowIf(TI, destroyer);

  availableValues_.setInCurrentScope(tdzStorage, newState);

  // This check moves the value into a more initialized state, so it can't be
  // deleted. But depending on the previous state, the check could possibly be
  // simplified. The only case is when moving from Uninit to Initialized, while
  // unnecessarily checking for Empty.
  if (oldState == ValueState::Uninit &&
      TI->getInvalidTypes()->getData().canBeEmpty()) {
    assert(
        newState == ValueState::Initialized &&
        "impossible: newState is known be > oldState");
    assert(
        TI->getInvalidTypes()->getData().canBeUninit() &&
        "impossible: the only possible transition from Uninit state involves removing Uninit type");

    // If the checked value still has Empty in its type, we must strip it away.
    // This can happen when loading from a variable - it's type includes Empty,
    // which needs to be discarded.
    if (checkedValue->getType().canBeEmpty()) {
      builder_.setInsertionPoint(TI);
      builder_.setLocation(TI->getLocation());
      auto *cast = builder_.createUnionNarrowTrustedInst(
          checkedValue,
          Type::subtractTy(checkedValue->getType(), Type::createEmpty()));
      TI->setCheckedValue(cast);
    }
    TI->setInvalidTypes(builder_.getLiteralIRType(Type::createUninit()));
    return true;
  }

  return false;
}

bool TDZDedupContext::eliminateThrowIf(
    ThrowIfInst *TI,
    IRBuilder::InstructionDestroyer &destroyer) {
  // The TDZ state is known to be true, so we can eliminate the check
  // instruction.
  destroyer.add(TI);
  ++NumTDZDedup;

  // If ThrowIf has no users, we will attempt to destroy the load too, to
  // save work in other passes.
  if (!TI->hasUsers()) {
    if (TI->getCheckedValue()->hasOneUser() &&
        (llvh::isa<LoadFrameInst>(TI->getCheckedValue()) ||
         llvh::isa<LoadStackInst>(TI->getCheckedValue()))) {
      destroyer.add(llvh::cast<Instruction>(TI->getCheckedValue()));
    }
  } else {
    builder_.setInsertionPoint(TI);
    builder_.setLocation(TI->getLocation());
    auto *cast = builder_.createUnionNarrowTrustedInst(
        TI->getCheckedValue(), TI->getType());
    TI->replaceAllUsesWith(cast);
  }

  // Return constant true to enable tail-recursion elimination in the caller,
  // which also returns a bool.
  return true;
}

} // end anonymous namespace

Pass *createTDZDedup() {
  class TDZDedup : public FunctionPass {
   public:
    explicit TDZDedup() : FunctionPass("TDZDedup") {}
    ~TDZDedup() override = default;

    bool runOnFunction(Function *F) override {
      DominanceInfo DT{F};
      TDZDedupContext CCtx{F, DT};
      return CCtx.run();
    }
  };
  return new TDZDedup();
}

} // namespace hermes

#undef DEBUG_TYPE
