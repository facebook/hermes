/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Inst/Inst.h"

#define DEBUG_TYPE "lowering"

using namespace hermes;

bool SwitchLowering::runOnFunction(Function *F) {
  bool changed = false;
  llvh::SmallVector<SwitchInst *, 4> switches;
  // Collect all switch instructions.
  for (BasicBlock &BB : *F)
    for (auto &it : BB) {
      if (auto *S = llvh::dyn_cast<SwitchInst>(&it))
        switches.push_back(S);
    }

  for (auto *S : switches) {
    lowerSwitchIntoIfs(S);
    changed = true;
  }

  return changed;
}

void SwitchLowering::lowerSwitchIntoIfs(SwitchInst *switchInst) {
  IRBuilder builder(switchInst->getParent()->getParent());
  builder.setLocation(switchInst->getLocation());

  BasicBlock *defaultDest = switchInst->getDefaultDestination();
  BasicBlock *next = defaultDest;
  BasicBlock *currentBlock = switchInst->getParent();

  // In this loop we are generating a sequence of IFs in reverse. We start
  // with the last IF that points to the Default case, and go back until we
  // generate the first IF. Then we connect the first IF into the entry
  // block and delete the Switch instruction.
  for (unsigned i = 0, e = switchInst->getNumCasePair(); i < e; ++i) {
    // Create an IF statement that matches the i'th case.
    BasicBlock *ifBlock = builder.createBasicBlock(currentBlock->getParent());

    // We scan the basic blocks in reverse!
    unsigned idx = (e - i - 1);
    auto caseEntry = switchInst->getCasePair(idx);

    builder.setInsertionBlock(ifBlock);
    auto *pred = builder.createBinaryOperatorInst(
        caseEntry.first,
        switchInst->getInputValue(),
        BinaryOperatorInst::OpKind::StrictlyEqualKind);
    // Cond branch - if the predicate of the comparison is true then jump
    // into the destination block. Otherwise jump to the next comparison in
    // the chain.
    builder.createCondBranchInst(pred, caseEntry.second, next);

    // Update any phis in the destination block.
    copyPhiTarget(caseEntry.second, currentBlock, ifBlock);

    if (next == defaultDest) {
      // If this block is responsible for jumps to the default block,
      // update phi nodes there too (true on the first iteration).
      copyPhiTarget(next, currentBlock, ifBlock);
    }

    next = ifBlock;
  }

  // Erase the phi edges that previously came from this block.
  erasePhiTarget(defaultDest, currentBlock);
  for (unsigned i = 0, e = switchInst->getNumCasePair(); i < e; ++i) {
    erasePhiTarget(switchInst->getCasePair(i).second, currentBlock);
  }

  switchInst->eraseFromParent();
  builder.setInsertionBlock(currentBlock);
  builder.createBranchInst(next);
}

/// Copy all incoming phi edges from a block to a new one
void SwitchLowering::copyPhiTarget(
    BasicBlock *block,
    BasicBlock *previousBlock,
    BasicBlock *newBlock) {
  for (auto &inst : *block) {
    auto *phi = llvh::dyn_cast<PhiInst>(&inst);
    if (!phi)
      break; // Phi must be first, so we won't find any more.

    Value *currentValue = nullptr;
    for (int i = 0, e = phi->getNumEntries(); i < e; i++) {
      auto pair = phi->getEntry(i);
      if (pair.second != previousBlock)
        continue;
      currentValue = pair.first;
      break;
    }

    if (currentValue) {
      phi->addEntry(currentValue, newBlock);
    }
  }
}

void SwitchLowering::erasePhiTarget(BasicBlock *block, BasicBlock *toDelete) {
  for (auto &inst : *block) {
    auto *phi = llvh::dyn_cast<PhiInst>(&inst);
    if (!phi)
      break; // Phi must be first, so we won't find any more.

    for (signed i = (signed)phi->getNumEntries() - 1; i >= 0; i--) {
      auto pair = phi->getEntry(i);
      if (pair.second != toDelete)
        continue;
      phi->removeEntry(i);
      // Some codegen can add multiple identical entries, so keep looking.
    }
  }
}

/// LowerAllocObjectFuncContext allows us to walk down the dominance tree and
/// process each basic block in the order of dominance relationship. This is
/// required to achieve reliable lowering optimizations for object
/// creations.
class LowerAllocObjectFuncContext
    : public DomTreeDFS::Visitor<
          LowerAllocObjectFuncContext,
          DomTreeDFS::StackNode<LowerAllocObjectFuncContext>> {
 public:
  LowerAllocObjectFuncContext(
      const DominanceInfo &DI,
      AllocObjectInst *allocInst)
      : DomTreeDFS::Visitor<
            LowerAllocObjectFuncContext,
            DomTreeDFS::StackNode<LowerAllocObjectFuncContext>>(DI),
        allocInst_(allocInst) {
    // The following loop constructs userBasicBlockMap_, by storing
    // all users of allocInst keyed by Basic Block.
    for (Instruction *I : allocInst->getUsers()) {
      BasicBlock *BB = I->getParent();
      userBasicBlockMap_[BB].insert(I);
    }
  }

  llvh::SmallVector<StoreNewOwnPropertyInst *, 4> run() {
    // First of all, get a list of basic blocks that contain users of
    // allocInst_, sorted by dominance relationship.
    DFS(DT_.getNode(allocInst_->getParent()));
    // Extract instructions (users) from these basic blocks that meet
    // the requirement of lowering.
    return collectInstructions();
  }

 private:
  // friend Visitor such that DFS can call processNode.
  friend DomTreeDFS::Visitor<
      LowerAllocObjectFuncContext,
      DomTreeDFS::StackNode<LowerAllocObjectFuncContext>>;

  /// Called by DFS recursively to process each node.
  /// The outcome (sortedBasicBlocks_) we expect is the list of basic blocks
  /// that contain users of allocInst_, that's strictly ordered by dominance
  /// (i.e. every BB must dominate the next one on the list). One important
  /// extra constraint is that the last BB in the list must also dominate all
  /// other BBs that contains users of allocInst_ that's not in the basic block
  /// list. The return value indicates whether this basic block is added to the
  /// list. Note that the return value isn't actually used.
  bool processNode(DomTreeDFS::StackNode<LowerAllocObjectFuncContext> *SN);

  /// collectInstructions walks through sortedBasicBlocks_, extract instructions
  /// that are users of allocInst_, ordered by dominance relationship.
  /// We also look into the type of each user and decide when to stop the
  /// lowering process. Specifically, we only process StoreNewOwnPropertyInst.
  llvh::SmallVector<StoreNewOwnPropertyInst *, 4> collectInstructions() const;

  /// The instruction that allocates the object.
  AllocObjectInst *allocInst_;

  /// Constructed from all users of allocInst_. Map from basic block to the set
  /// of instructions in that basic block that are users of allocInst_. This
  /// data structure is needed to speed up all the lookup operations during the
  /// process.
  llvh::DenseMap<BasicBlock *, llvh::DenseSet<Instruction *>>
      userBasicBlockMap_{};

  /// When we encounter branching, i.e. for a given basic block, if multiple of
  /// the basic blocks dominated by that basic block all contain users of
  /// allocInst_, we can not append any of those basic blocks to
  /// sortedBasicBlocks_. Furthermore, we can not append any other basic blocks
  /// to sortedBasicBlocks_ because the branch already exists.
  bool stopAddingBasicBlock_{false};

  /// List of basic blocks that contain users of allocInst_, ordered by
  /// dominance relationship.
  llvh::SmallVector<BasicBlock *, 4> sortedBasicBlocks_{};
};

bool LowerAllocObjectFuncContext::processNode(
    DomTreeDFS::StackNode<LowerAllocObjectFuncContext> *SN) {
  assert(!SN->isDone() && "Visiting same basic block twice");
  SN->markDone();

  BasicBlock *BB = SN->node()->getBlock();
  if (!userBasicBlockMap_.count(BB)) {
    // BB does not contain any users of allocInst_.
    return false;
  }
  while (!sortedBasicBlocks_.empty() &&
         !DT_.properlyDominates(sortedBasicBlocks_.back(), BB)) {
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

llvh::SmallVector<StoreNewOwnPropertyInst *, 4>
LowerAllocObjectFuncContext::collectInstructions() const {
  llvh::SmallVector<StoreNewOwnPropertyInst *, 4> instrs;

  for (BasicBlock *BB : sortedBasicBlocks_) {
    bool terminate = false;
    for (Instruction &I : *BB) {
      if (!userBasicBlockMap_.find(BB)->second.count(&I)) {
        // I is not a user of allocInst_, ignore it.
        continue;
      }
      auto *SI = llvh::dyn_cast<StoreNewOwnPropertyInst>(&I);
      if (!SI || SI->getObject() != allocInst_) {
        // A user that's not a StoreNewOwnPropertyInst storing into the object
        // created by allocInst_. We have to stop processing here.
        terminate = true;
        break;
      }
      instrs.push_back(SI);
    }
    if (terminate) {
      break;
    }
  }
  return instrs;
}

bool LowerAllocObject::runOnFunction(Function *F) {
  bool changed = false;
  llvh::SmallVector<AllocObjectInst *, 4> allocs;
  // Collect all AllocObject instructions.
  for (BasicBlock &BB : *F)
    for (auto &it : BB) {
      if (auto *A = llvh::dyn_cast<AllocObjectInst>(&it))
        if (llvh::isa<EmptySentinel>(A->getParentObject()))
          allocs.push_back(A);
    }

  for (auto *A : allocs) {
    changed |= lowerAlloc(A);
  }

  return changed;
}

bool LowerAllocObject::lowerAlloc(AllocObjectInst *allocInst) {
  Function *F = allocInst->getParent()->getParent();
  DominanceInfo DI(F);
  LowerAllocObjectFuncContext ctx(DI, allocInst);
  llvh::SmallVector<StoreNewOwnPropertyInst *, 4> users = ctx.run();
  if (users.empty()) {
    return false;
  }

  bool changed = lowerAllocObjectBuffer(allocInst, users, UINT16_MAX);
  return changed;
}

// Number of bytes saved for serializing a literal into the buffer.
// Estimated with the example of an integer. Substract the cost of serializing
// the int and a 1-byte tag.
static constexpr int32_t kLiteralSavedBytes = static_cast<int32_t>(
    sizeof(inst::LoadConstIntInst) + sizeof(inst::PutNewOwnByIdInst) -
    sizeof(int32_t) - 1);
// Number of bytes cost for serializing a non-literal into the buffer.
// Cost includes a 1-byte tag and replacing with a longer put instruction.
static constexpr int32_t kNonLiteralCostBytes = static_cast<int32_t>(
    1 + sizeof(inst::PutByIdInst) - sizeof(inst::PutNewOwnByIdInst));
// Max number of non-literals we allow to serialize into the buffer.
// The number is chosen to be small and can allow most literals to be serialized
// for most cases.
static constexpr uint32_t kNonLiteralPlaceholderLimit = 3;

/// Whether the given value \v V can be serialized into the object literal
/// buffer.
static bool isSerializableLiteral(Value *V) {
  return V &&
      (llvh::isa<LiteralNull>(V) || llvh::isa<LiteralBool>(V) ||
       llvh::isa<LiteralNumber>(V) || llvh::isa<LiteralString>(V));
}

static bool canSerialize(Value *V) {
  if (auto *LCI = llvh::dyn_cast_or_null<HBCLoadConstInst>(V))
    return isSerializableLiteral(LCI->getConst());
  return false;
}

uint32_t LowerAllocObject::estimateBestNumElemsToSerialize(
    llvh::SmallVectorImpl<StoreNewOwnPropertyInst *> &users) {
  // We want to track curSaving to avoid serializing too many place holders
  // which ends up causing a big size regression.
  // We set curSaving to be the delta of the size of two instructions to avoid
  // serializing a literal object with only one entry, which turns out to
  // significantly increase bytecode size.
  int32_t curSaving = static_cast<int32_t>(sizeof(inst::NewObjectInst)) -
      static_cast<int32_t>(sizeof(inst::NewObjectWithBufferInst));
  int32_t maxSaving = 0;
  uint32_t optimumStopIndex = 0;
  uint32_t nonLiteralPlaceholderCount = 0;

  uint32_t curSize = 0;
  for (StoreNewOwnPropertyInst *I : users) {
    ++curSize;
    assert(
        (llvh::isa<LiteralString>(I->getProperty()) ||
         llvh::isa<LiteralNumber>(I->getProperty())) &&
        "StoreNewOwnPropertyInst property must be literal.");
    if (canSerialize(I->getStoredValue())) {
      // Property Value is a literal that's not undefined.
      curSaving += kLiteralSavedBytes;
      if (curSaving > maxSaving) {
        maxSaving = curSaving;
        optimumStopIndex = curSize;
      }
    } else {
      // Property Value is computed. we could try to store a null as
      // placeholder, and set the proper value latter.
      if (llvh::isa<LiteralNumber>(I->getProperty())) {
        // If the key is a number, we can't set it latter with PutById, so
        // have to skip it. We only need to check if it's an instance of
        // LiteralNumber because LowerNumericProperties must have lowered any
        // number-like property to LiteralNumber.
        // We don't need to stop the whole process because a numeric literal
        // property can be inserted in any order. So it's safe to skip it
        // in the lowering.
        continue;
      }
      if (nonLiteralPlaceholderCount == kNonLiteralPlaceholderLimit) {
        // We have reached the maximum number of place holders we can put.
        break;
      }
      nonLiteralPlaceholderCount++;
      curSaving -= kNonLiteralCostBytes;
    }
  }
  return optimumStopIndex;
}

bool LowerAllocObject::lowerAllocObjectBuffer(
    AllocObjectInst *allocInst,
    llvh::SmallVectorImpl<StoreNewOwnPropertyInst *> &users,
    uint32_t maxSize) {
  auto size = estimateBestNumElemsToSerialize(users);
  if (size == 0) {
    return false;
  }
  size = std::min(maxSize, size);

  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);
  HBCAllocObjectFromBufferInst::ObjectPropertyMap prop_map;
  for (uint32_t i = 0; i < size; ++i) {
    StoreNewOwnPropertyInst *I = users[i];
    Literal *propLiteral = nullptr;
    // Property name can be either a LiteralNumber or a LiteralString.
    if (auto *LN = llvh::dyn_cast<LiteralNumber>(I->getProperty())) {
      assert(
          LN->convertToArrayIndex() &&
          "LiteralNumber can be a property name only if it can be converted to array index.");
      propLiteral = LN;
    } else {
      propLiteral = cast<LiteralString>(I->getProperty());
    }

    auto *loadInst = llvh::dyn_cast<HBCLoadConstInst>(I->getStoredValue());
    // Not counting undefined as literal since the parser doesn't
    // support it.
    if (canSerialize(loadInst)) {
      prop_map.push_back(
          std::pair<Literal *, Literal *>(propLiteral, loadInst->getConst()));
      I->eraseFromParent();
    } else if (llvh::isa<LiteralString>(propLiteral)) {
      // If prop is a literal number, there is no need to put it into the
      // buffer or change the instruction.
      // Otherwise, use null as placeholder, and
      // later a PutById instruction will overwrite it with correct value.
      prop_map.push_back(std::pair<Literal *, Literal *>(
          propLiteral, builder.getLiteralNull()));

      // Since we will be defining this property twice, once in the buffer
      // once setting the correct value later, we can no longer use
      // StoreNewOwnPropertyInst. Replace this instruction with
      // StorePropertyInst.
      builder.setLocation(I->getLocation());
      builder.setInsertionPoint(I);
      auto *NI = builder.createStorePropertyInst(
          I->getStoredValue(), I->getObject(), I->getProperty());
      I->replaceAllUsesWith(NI);
      I->eraseFromParent();
    }
  }

  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *alloc = builder.createHBCAllocObjectFromBufferInst(
      prop_map, allocInst->getSize());
  allocInst->replaceAllUsesWith(alloc);
  allocInst->eraseFromParent();

  return true;
}

bool LowerAllocObjectLiteral::runOnFunction(Function *F) {
  bool changed = false;
  llvh::SmallVector<AllocObjectLiteralInst *, 4> allocs;
  for (BasicBlock &BB : *F) {
    // We need to increase the iterator before calling lowerAllocObjectBuffer.
    // Otherwise deleting the instruction will invalidate the iterator.
    for (auto it = BB.begin(), e = BB.end(); it != e;) {
      if (auto *A = llvh::dyn_cast<AllocObjectLiteralInst>(&*it++)) {
        changed |= lowerAllocObjectBuffer(A);
      }
    }
  }

  return changed;
}

bool LowerAllocObjectLiteral::lowerAlloc(AllocObjectLiteralInst *allocInst) {
  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);

  auto size = allocInst->getKeyValuePairCount();

  // Replace AllocObjectLiteral with a regular AllocObject
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *Obj = builder.createAllocObjectInst(size, nullptr);

  for (unsigned i = 0; i < allocInst->getKeyValuePairCount(); i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    builder.createStoreNewOwnPropertyInst(
        value, allocInst, key, IRBuilder::PropEnumerable::Yes);
  }
  allocInst->replaceAllUsesWith(Obj);
  allocInst->eraseFromParent();

  return true;
}

uint32_t LowerAllocObjectLiteral::estimateBestNumElemsToSerialize(
    AllocObjectLiteralInst *allocInst) {
  // Reuse calc logic from LowerAllocObject.
  int32_t curSaving = static_cast<int32_t>(sizeof(inst::NewObjectInst)) -
      static_cast<int32_t>(sizeof(inst::NewObjectWithBufferInst));
  int32_t maxSaving = 0;
  uint32_t optimumStopIndex = 0;
  uint32_t nonLiteralPlaceholderCount = 0;

  uint32_t curSize = 0;
  for (unsigned i = 0; i < allocInst->getKeyValuePairCount(); i++) {
    ++curSize;
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    if (isSerializableLiteral(value)) {
      curSaving += kLiteralSavedBytes;
      if (curSaving > maxSaving) {
        maxSaving = curSaving;
        optimumStopIndex = curSize;
      }
    } else {
      if (llvh::isa<LiteralNumber>(key)) {
        continue;
      }
      if (nonLiteralPlaceholderCount == kNonLiteralPlaceholderLimit) {
        // We have reached the maximum number of place holders we can put.
        break;
      }
      nonLiteralPlaceholderCount++;
      curSaving -= kNonLiteralCostBytes;
    }
  }
  return optimumStopIndex;
}

bool LowerAllocObjectLiteral::lowerAllocObjectBuffer(
    AllocObjectLiteralInst *allocInst) {
  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);

  auto maxSize = (unsigned)UINT16_MAX;
  auto size = estimateBestNumElemsToSerialize(allocInst);
  size = std::min(maxSize, size);

  // Should not create HBCAllocObjectFromBufferInst.
  if (size == 0) {
    return lowerAlloc(allocInst);
  }

  // Replace AllocObjectLiteral with HBCAllocObjectFromBufferInst
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPointAfter(allocInst);
  HBCAllocObjectFromBufferInst::ObjectPropertyMap propMap;

  unsigned i = 0;
  for (; i < size; i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    Literal *propLiteral = nullptr;
    // Property name can be either a LiteralNumber or a LiteralString.
    if (auto *LN = llvh::dyn_cast<LiteralNumber>(key)) {
      assert(
          LN->convertToArrayIndex() &&
          "LiteralNumber can be a property name only if it can be converted to array index.");
      propLiteral = LN;
    } else {
      propLiteral = cast<LiteralString>(key);
    }

    if (isSerializableLiteral(value)) {
      propMap.push_back(std::pair<Literal *, Literal *>(
          propLiteral, llvh::cast<Literal>(value)));
    } else if (llvh::isa<LiteralString>(propLiteral)) {
      // LiteralString key with undefined / non-constant value.
      propMap.push_back(std::pair<Literal *, Literal *>(
          propLiteral, builder.getLiteralNull()));
      builder.createStorePropertyInst(value, allocInst, key);
    } else {
      // LiteralNumber key with undefined / non-constant value.
      // No need to put Null in the buffer, as numeric properties can
      // be added in any order.
      builder.createStoreOwnPropertyInst(
          value, allocInst, key, IRBuilder::PropEnumerable::Yes);
    }
  }

  // Handle properties beyond best num of properties or that cannot fit in
  // maxSize.
  for (; i < allocInst->getKeyValuePairCount(); i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    builder.createStoreNewOwnPropertyInst(
        value, allocInst, key, IRBuilder::PropEnumerable::Yes);
  }

  // Emit HBCAllocObjectFromBufferInst.
  // First, we reset insertion location.
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *alloc = builder.createHBCAllocObjectFromBufferInst(
      propMap, allocInst->getKeyValuePairCount());
  allocInst->replaceAllUsesWith(alloc);
  allocInst->eraseFromParent();

  return true;
}

bool LowerStoreInstrs::runOnFunction(Function *F) {
  IRBuilder builder(F);
  IRBuilder::InstructionDestroyer destroyer;
  bool changed = false;

  PostOrderAnalysis PO(F);
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  for (auto *bbit : order) {
    for (auto &it : bbit->getInstList()) {
      auto *SSI = llvh::dyn_cast<StoreStackInst>(&it);
      if (!SSI)
        continue;

      Value *ptr = SSI->getPtr();
      Value *val = SSI->getValue();

      builder.setInsertionPoint(&it);
      auto dstReg = RA_.getRegister(ptr);
      auto *mov = builder.createMovInst(val);
      RA_.updateRegister(mov, dstReg);
      it.replaceAllUsesWith(mov);
      destroyer.add(&it);
      changed = true;
    }
  }
  return changed;
}

bool LowerNumericProperties::stringToNumericProperty(
    IRBuilder &builder,
    Instruction &Inst,
    unsigned operandIdx) {
  auto strLit = llvh::dyn_cast<LiteralString>(Inst.getOperand(operandIdx));
  if (!strLit)
    return false;

  // Check if the string looks exactly like an array index.
  auto num = toArrayIndex(strLit->getValue().str());
  if (num) {
    Inst.setOperand(builder.getLiteralNumber(*num), operandIdx);
    return true;
  }

  return false;
}

bool LowerNumericProperties::runOnFunction(Function *F) {
  IRBuilder builder(F);
  IRBuilder::InstructionDestroyer destroyer{};

  bool changed = false;
  for (BasicBlock &BB : *F) {
    for (Instruction &Inst : BB) {
      if (llvh::isa<LoadPropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, LoadPropertyInst::PropertyIdx);
      } else if (llvh::isa<StorePropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StorePropertyInst::PropertyIdx);
      } else if (llvh::isa<StoreOwnPropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StoreOwnPropertyInst::PropertyIdx);
      } else if (llvh::isa<DeletePropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, DeletePropertyInst::PropertyIdx);
      } else if (llvh::isa<StoreGetterSetterInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StoreGetterSetterInst::PropertyIdx);
      } else if (llvh::isa<AllocObjectLiteralInst>(&Inst)) {
        auto allocInst = cast<AllocObjectLiteralInst>(&Inst);
        for (unsigned i = 0; i < allocInst->getKeyValuePairCount(); i++) {
          changed |= stringToNumericProperty(builder, Inst, i * 2);
        }
      }
    }
  }
  return changed;
}

bool LimitAllocArray::runOnFunction(Function *F) {
  bool changed = false;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      auto *inst = llvh::dyn_cast<AllocArrayInst>(&I);
      if (!inst || inst->getElementCount() == 0)
        continue;

      IRBuilder builder(F);
      builder.setInsertionPointAfter(inst);
      builder.setLocation(inst->getLocation());

      // Checks if any operand of an AllocArray is unserializable.
      // If it finds one, the loop removes it along with every operand past it.
      {
        bool seenUnserializable = false;
        unsigned ind = -1;
        unsigned i = AllocArrayInst::ElementStartIdx;
        unsigned e = inst->getElementCount() + AllocArrayInst::ElementStartIdx;
        while (i < e) {
          ind++;
          seenUnserializable |=
              inst->getOperand(i)->getKind() == ValueKind::LiteralBigIntKind ||
              inst->getOperand(i)->getKind() == ValueKind::LiteralUndefinedKind;
          if (seenUnserializable) {
            e--;
            builder.createStoreOwnPropertyInst(
                inst->getOperand(i),
                inst,
                builder.getLiteralNumber(ind),
                IRBuilder::PropEnumerable::Yes);
            inst->removeOperand(i);
            changed = true;
            continue;
          }
          i++;
        }
      }

      if (inst->getElementCount() == 0)
        continue;

      // Since we remove elements from inst until it fits in maxSize_,
      // the final addition to totalElems will make it equal maxSize_. Any
      // AllocArray past that would have all its operands removed, and add
      // 0 to totalElems.
      for (unsigned i = inst->getElementCount() - 1; i >= maxSize_; i--) {
        int operandOffset = AllocArrayInst::ElementStartIdx + i;
        builder.createStoreOwnPropertyInst(
            inst->getOperand(operandOffset),
            inst,
            builder.getLiteralNumber(i),
            IRBuilder::PropEnumerable::Yes);
        inst->removeOperand(operandOffset);
      }
      changed = true;
    }
  }
  return changed;
}

bool LowerCondBranch::isOperatorSupported(BinaryOperatorInst::OpKind op) {
  using OpKind = BinaryOperatorInst::OpKind;
  switch (op) {
    case OpKind::LessThanKind: // <
    case OpKind::LessThanOrEqualKind: // <=
    case OpKind::GreaterThanKind: // >
    case OpKind::GreaterThanOrEqualKind: // >=
    case OpKind::StrictlyEqualKind:
    case OpKind::StrictlyNotEqualKind:
    case OpKind::NotEqualKind: // !=
    case OpKind::EqualKind: // ==
      return true;
    default:
      return false;
  }
}

bool LowerCondBranch::runOnFunction(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  for (auto &BB : *F) {
    llvh::DenseMap<CondBranchInst *, CompareBranchInst *> condToCompMap;

    for (auto &I : BB) {
      auto *cbInst = llvh::dyn_cast<CondBranchInst>(&I);
      // This also matches constructors.
      if (!cbInst)
        continue;

      Value *cond = cbInst->getCondition();

      // If the condition has more than one user, we can't lower it.
      if (!cond->hasOneUser())
        continue;

      // The condition must be a binary operator.
      auto binopInst = llvh::dyn_cast<BinaryOperatorInst>(cond);
      if (!binopInst)
        continue;

      auto *LHS = binopInst->getLeftHandSide();
      auto *RHS = binopInst->getRightHandSide();

      // The condition must either be side-effect free, or it must be the
      // previous instruction.
      if (binopInst->hasSideEffect())
        if (cbInst->getPrevNode() != binopInst)
          continue;

      // Only certain operators are supported.
      if (!isOperatorSupported(binopInst->getOperatorKind()))
        continue;

      builder.setInsertionPoint(cbInst);
      builder.setLocation(cbInst->getLocation());
      auto *cmpBranch = builder.createCompareBranchInst(
          LHS,
          RHS,
          binopInst->getOperatorKind(),
          cbInst->getTrueDest(),
          cbInst->getFalseDest());

      condToCompMap[cbInst] = cmpBranch;
      changed = true;
    }

    for (const auto &cbiter : condToCompMap) {
      auto binopInst =
          llvh::dyn_cast<BinaryOperatorInst>(cbiter.first->getCondition());

      cbiter.first->replaceAllUsesWith(condToCompMap[cbiter.first]);
      cbiter.first->eraseFromParent();
      binopInst->eraseFromParent();
    }
  }
  return changed;
}

bool LowerExponentiationOperator::runOnFunction(Function *F) {
  IRBuilder builder{F};
  llvh::DenseSet<Instruction *> toTransform{};
  bool changed = false;

  for (BasicBlock &bb : *F) {
    for (auto it = bb.begin(), e = bb.end(); it != e; /* empty */) {
      auto *inst = &*it;
      // Increment iterator before potentially erasing inst and invalidating
      // iteration.
      ++it;
      if (auto *binOp = llvh::dyn_cast<BinaryOperatorInst>(inst)) {
        if (binOp->getOperatorKind() ==
            BinaryOperatorInst::OpKind::ExponentiationKind) {
          changed |= lowerExponentiationOperator(builder, binOp);
        }
      }
    }
  }

  return changed;
}

bool LowerExponentiationOperator::lowerExponentiationOperator(
    IRBuilder &builder,
    BinaryOperatorInst *binOp) {
  assert(
      binOp->getOperatorKind() ==
          BinaryOperatorInst::OpKind::ExponentiationKind &&
      "lowerExponentiationOperator must take a ** operator");
  // Replace a ** b with HermesInternal.exponentiationOperator(a, b)
  builder.setInsertionPoint(binOp);
  auto *result = builder.createCallBuiltinInst(
      BuiltinMethod::HermesBuiltin_exponentiationOperator,
      {binOp->getLeftHandSide(), binOp->getRightHandSide()});
  binOp->replaceAllUsesWith(result);
  binOp->eraseFromParent();
  return true;
}

#undef DEBUG_TYPE
