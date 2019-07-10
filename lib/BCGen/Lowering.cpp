/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Utils/Dumper.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "lowering"

using namespace hermes;

bool SwitchLowering::runOnFunction(Function *F) {
  bool changed = false;
  llvm::SmallVector<SwitchInst *, 4> switches;
  // Collect all switch instructions.
  for (BasicBlock &BB : *F)
    for (auto &it : BB) {
      if (auto *S = dyn_cast<SwitchInst>(&it))
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
    auto *phi = dyn_cast<PhiInst>(&inst);
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
    auto *phi = dyn_cast<PhiInst>(&inst);
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

bool LowerAllocObject::runOnFunction(Function *F) {
  bool changed = false;
  llvm::SmallVector<AllocObjectInst *, 4> allocs;
  // Collect all AllocObject instructions.
  for (BasicBlock &BB : *F)
    for (auto &it : BB) {
      if (auto *A = dyn_cast<AllocObjectInst>(&it))
        if (isa<EmptySentinel>(A->getParentObject()))
          allocs.push_back(A);
    }

  for (auto *A : allocs) {
    changed |= lowerAlloc(A);
  }

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

uint32_t LowerAllocObject::estimateBestNumElemsToSerialize(
    AllocObjectInst *allocInst) {
  uint32_t elemCount = 0;
  // We want to track savingSoFar to avoid serializing too many place holders
  // which ends up causing a big size regression.
  // We set savingSoFar to be the delta of the size of two instructions to avoid
  // serializing a literal object with only one entry, which turns out to
  // significantly increase bytecode size.
  int32_t savingSoFar = static_cast<int32_t>(sizeof(inst::NewObjectInst)) -
      static_cast<int32_t>(sizeof(inst::NewObjectWithBufferInst));
  int32_t maxSaving = 0;
  uint32_t elemNumForMaxSaving = 0;
  uint32_t nonLiteralsSoFar = 0;
  for (Instruction *u : allocInst->getUsers()) {
    // Stop when we see a getter/setter.
    if (isa<StoreGetterSetterInst>(u)) {
      break;
    } else if (auto *put = dyn_cast<StoreOwnPropertyInst>(u)) {
      // Skip if the instruction is storing the object itself into another
      // object.
      if (put->getStoredValue() == dyn_cast<Value>(allocInst) &&
          put->getObject() != allocInst) {
        continue;
      }
      elemCount++;
      if (elemCount > maxSize_) {
        break;
      }
      auto *loadInst = dyn_cast<HBCLoadConstInst>(put->getStoredValue());
      // Not counting undefined as literal since the parser doesn't
      // support it.
      if (loadInst &&
          loadInst->getSingleOperand()->getKind() !=
              ValueKind::LiteralUndefinedKind) {
        savingSoFar += kLiteralSavedBytes;
        if (savingSoFar > maxSaving) {
          maxSaving = savingSoFar;
          elemNumForMaxSaving = elemCount;
        }
      } else {
        // If the key is a number, we can't overwrite with PutById, so stop.
        if (isa<LiteralNumber>(put->getProperty())) {
          break;
        }
        nonLiteralsSoFar++;
        if (nonLiteralsSoFar > kNonLiteralPlaceholderLimit) {
          // Stop when we serialize too many placeholders.
          break;
        }
        savingSoFar -= kNonLiteralCostBytes;
      }
    } else {
      // Stop when we reach an unsupported instruction.
      break;
    }
  }
  return elemNumForMaxSaving;
}

bool LowerAllocObject::lowerAlloc(AllocObjectInst *allocInst) {
  // First pass to compute best number of properties to serialize.
  uint32_t elemNumForMaxSaving = estimateBestNumElemsToSerialize(allocInst);
  if (elemNumForMaxSaving == 0) {
    return false;
  }

  IRBuilder builder(allocInst->getParent()->getParent());
  HBCAllocObjectFromBufferInst::ObjectPropertyMap prop_map;
  uint32_t elemCountSoFar = 0;
  // Since instructions are created sequentially, and since Users
  // are added when an instruction is created, getUsers() preserves
  // insertion order.
  std::vector<Instruction *> users(
      allocInst->getUsers().begin(), allocInst->getUsers().end());
  for (Instruction *u : users) {
    if (auto *put = dyn_cast<StoreOwnPropertyInst>(u)) {
      // Skip if the property name isn't a LiteralString or a valid array index.
      Literal *propLiteral = nullptr;
      if (auto *LN = dyn_cast<LiteralNumber>(put->getProperty())) {
        if (LN->convertToArrayIndex())
          propLiteral = LN;
      } else {
        propLiteral = dyn_cast<LiteralString>(put->getProperty());
      }
      if (!propLiteral)
        continue;

      // Skip if the instruction is storing the object itself into another
      // object.
      if (put->getStoredValue() == dyn_cast<Value>(allocInst) &&
          put->getObject() != allocInst) {
        continue;
      }
      elemCountSoFar++;
      if (elemCountSoFar > elemNumForMaxSaving) {
        break;
      }
      auto *loadInst = dyn_cast<HBCLoadConstInst>(put->getStoredValue());
      // Not counting undefined as literal since the parser doesn't
      // support it.
      if (loadInst &&
          loadInst->getSingleOperand()->getKind() !=
              ValueKind::LiteralUndefinedKind) {
        prop_map.push_back(
            std::pair<Literal *, Literal *>(propLiteral, loadInst->getConst()));
        put->eraseFromParent();
      } else {
        // In the case of non-literal, use null as placeholder, and
        // later a PutById instruction will overwrite it with correct value.
        prop_map.push_back(std::pair<Literal *, Literal *>(
            propLiteral, builder.getLiteralNull()));
        builder.setLocation(put->getLocation());
        builder.setInsertionPoint(put);
        auto *newPut = builder.createStorePropertyInst(
            put->getStoredValue(), put->getObject(), put->getProperty());
        put->replaceAllUsesWith(newPut);
        put->eraseFromParent();
      }
    } else {
      // Stop when we reach an unsupported instruction.
      break;
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

bool LowerStoreInstrs::runOnFunction(Function *F) {
  IRBuilder builder(F);
  IRBuilder::InstructionDestroyer destroyer;
  bool changed = false;

  PostOrderAnalysis PO(F);
  llvm::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  for (auto *bbit : order) {
    for (auto &it : bbit->getInstList()) {
      auto *SSI = dyn_cast<StoreStackInst>(&it);
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
  auto strLit = dyn_cast<LiteralString>(Inst.getOperand(operandIdx));
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
      // If StoreNewOwnPropertyInst's property name is a valid array index, we
      // must convert the instruction to StoreOwnPropertyInst.
      if (auto *SNOP = llvm::dyn_cast<StoreNewOwnPropertyInst>(&Inst)) {
        auto *strLit = SNOP->getPropertyName();

        // Check if the string looks exactly like an array index.
        if (auto num = toArrayIndex(strLit->getValue().str())) {
          builder.setInsertionPoint(&Inst);
          builder.setLocation(SNOP->getLocation());
          auto *inst = builder.createStoreOwnPropertyInst(
              SNOP->getStoredValue(),
              SNOP->getObject(),
              builder.getLiteralNumber(*num),
              SNOP->getIsEnumerable() ? IRBuilder::PropEnumerable::Yes
                                      : IRBuilder::PropEnumerable::No);

          Inst.replaceAllUsesWith(inst);
          destroyer.add(&Inst);
          changed = true;
          continue;
        }
      }

      if (llvm::isa<LoadPropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, LoadPropertyInst::PropertyIdx);
      } else if (llvm::isa<StorePropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StorePropertyInst::PropertyIdx);
      } else if (llvm::isa<StoreOwnPropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StoreOwnPropertyInst::PropertyIdx);
      } else if (llvm::isa<DeletePropertyInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, DeletePropertyInst::PropertyIdx);
      } else if (llvm::isa<StoreGetterSetterInst>(&Inst)) {
        changed |= stringToNumericProperty(
            builder, Inst, StoreGetterSetterInst::PropertyIdx);
      }
    }
  }
  return changed;
}

bool LimitAllocArray::runOnFunction(Function *F) {
  bool changed = false;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      int totalElems = 0;
      auto *inst = dyn_cast<AllocArrayInst>(&I);
      if (!inst || inst->getElementCount() == 0)
        continue;

      IRBuilder builder(F);
      builder.setInsertionPointAfter(inst);
      builder.setLocation(inst->getLocation());

      // Checks if any operand of an AllocArray is undefined.
      // If it finds one, the loop removes it along with every operand past it.
      {
        bool seenUndef = false;
        unsigned ind = -1;
        unsigned i = AllocArrayInst::ElementStartIdx;
        unsigned e = inst->getElementCount() + AllocArrayInst::ElementStartIdx;
        while (i < e) {
          ind++;
          seenUndef |=
              inst->getOperand(i)->getKind() == ValueKind::LiteralUndefinedKind;
          if (seenUndef) {
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
      totalElems += inst->getElementCount();
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
    llvm::DenseMap<CondBranchInst *, CompareBranchInst *> condToCompMap;

    for (auto &I : BB) {
      auto *cbInst = dyn_cast<CondBranchInst>(&I);
      // This also matches constructors.
      if (!cbInst)
        continue;

      Value *cond = cbInst->getCondition();

      // If the condition has more than one user, we can't lower it.
      if (!cond->hasOneUser())
        continue;

      // The condition must be a binary operator.
      auto binopInst = dyn_cast<BinaryOperatorInst>(cond);
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

    for (const auto cbiter : condToCompMap) {
      auto binopInst =
          dyn_cast<BinaryOperatorInst>(cbiter.first->getCondition());

      cbiter.first->replaceAllUsesWith(condToCompMap[cbiter.first]);
      cbiter.first->eraseFromParent();
      binopInst->eraseFromParent();
    }
  }
  return changed;
}
