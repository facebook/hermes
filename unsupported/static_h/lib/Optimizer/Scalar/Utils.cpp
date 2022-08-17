/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "utils"

#include "hermes/Optimizer/Scalar/Utils.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

using namespace hermes;

Value *hermes::isStoreOnceVariable(Variable *V) {
  Value *res = nullptr;

  for (auto *U : V->getUsers()) {
    if (llvh::isa<LoadFrameInst>(U)) {
      continue;
    }
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U)) {
      auto *val = SF->getValue();

      // We found a stored value. Make sure that there is only one stored value.
      if (res && val != res)
        return nullptr;

      res = val;
      continue;
    }

    llvm_unreachable("invalid user!");
  }

  return res;
}

Value *hermes::isStoreOnceStackLocation(AllocStackInst *AS) {
  Value *res = nullptr;

  for (auto *U : AS->getUsers()) {
    if (llvh::isa<LoadStackInst>(U)) {
      continue;
    }
    if (auto *SS = llvh::dyn_cast<StoreStackInst>(U)) {
      // Theoretically someone might try storing a stack address in another
      // stack location. That is not allowed, but we might as well be correct
      // here.
      if (SS->getPtr() != AS)
        continue;

      auto *val = SS->getValue();

      // We found a stored value. Make sure that there is only one stored value.
      if (res && val != res)
        return nullptr;

      res = val;
      continue;
    }

    llvm_unreachable("invalid user!");
  }

  return res;
}

Function *hermes::getCallee(Value *callee) {
  // This is a direct call.
  if (auto *F = llvh::dyn_cast<Function>(callee)) {
    return F;
  }

  // This is a direct use of a closure.
  if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(callee)) {
    return CFI->getFunctionCode();
  }

  // If we load from a frame variable, check if this is a non-global store-only
  // variable.
  if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(callee)) {
    auto *V = LFI->getLoadVariable();

    if (Value *singleValue = isStoreOnceVariable(V))
      return getCallee(singleValue);
  }

  return nullptr;
}

bool hermes::isDirectCallee(Value *C, CallInst *CI) {
  if (CI->getCallee() != C)
    return false;

  for (int i = 0, e = CI->getNumArguments(); i < e; i++) {
    // Check if C is captured.
    if (C == CI->getArgument(i))
      return false;
  }

  return true;
}

bool hermes::getCallSites(
    Function *F,
    llvh::SmallVectorImpl<CallInst *> &callsites) {
  for (auto *CU : F->getUsers()) {
    auto *CFI = cast<CreateFunctionInst>(CU);

    // Collect direct calls.
    for (auto *U : CFI->getUsers()) {
      auto *CI = llvh::dyn_cast<CallInst>(U);
      if (CI && isDirectCallee(CFI, CI)) {
        callsites.push_back(CI);
        continue;
      }

      // Check if the variable is stored somewhere.
      auto *SFI = llvh::dyn_cast<StoreFrameInst>(U);
      if (!SFI)
        return false;

      // If the variable is analyzable then try to see where the closure
      // goes.
      Variable *V = SFI->getVariable();
      if (!isStoreOnceVariable(V))
        return false;

      for (auto *VU : V->getUsers()) {
        if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(VU)) {
          if (!LFI->hasOneUser())
            return false;

          Value *loadUser = LFI->getUsers()[0];
          if (auto *loadUserCI = llvh::dyn_cast<CallInst>(loadUser)) {
            if (loadUserCI && isDirectCallee(LFI, loadUserCI)) {
              callsites.push_back(loadUserCI);
              continue;
            }
          }

          // Unknown load used.
          return false;
        }
      }
    }
  }
  return true;
}

/// Delete all incoming arrows from \p incoming in PhiInsts in \p blockToModify.
bool hermes::deleteIncomingBlockFromPhis(
    BasicBlock *blockToModify,
    BasicBlock *incoming) {
  bool changed = false;
  for (auto &I : *blockToModify) {
    auto *phi = llvh::dyn_cast<PhiInst>(&I);
    if (!phi)
      break;

    for (signed i = (signed)phi->getNumEntries() - 1; i >= 0; i--) {
      auto entry = phi->getEntry(i);
      if (entry.second != incoming)
        continue;

      phi->removeEntry(i);
      changed = true;
    }
  }
  return changed;
}

void hermes::splitCriticalEdge(
    IRBuilder *builder,
    BasicBlock *from,
    BasicBlock *to) {
  // Special case: If the target block is Catch block, there's only one
  // possible arrow and we can't insert anything between them. Just
  // start writing after the Catch statement.
  if (auto *tryStart = llvh::dyn_cast<TryStartInst>(from->getTerminator())) {
    if (tryStart->getCatchTarget() == to) {
      builder->setInsertionPointAfter(&to->front());
      return;
    }
  }

  // General case: insert a new block and rewrite Phis
  auto *newBlock = builder->createBasicBlock(from->getParent());
  builder->setInsertionBlock(newBlock);
  auto *branch = builder->createBranchInst(to);
  int updates = 0;

  Instruction *terminator = from->getTerminator();
  for (int i = 0, e = terminator->getNumOperands(); i < e; i++) {
    if (terminator->getOperand(i) == to) {
      terminator->setOperand(newBlock, i);
      for (auto &I : *to) {
        auto *phi = llvh::dyn_cast<PhiInst>(&I);
        if (!phi)
          break;
        for (int j = 0, f = phi->getNumEntries(); j < f; j++) {
          auto entry = phi->getEntry(j);
          if (entry.second != from)
            continue;
          phi->updateEntry(j, entry.first, newBlock);
        }
      }
      updates++;
    }
  }

  if (!updates) {
    llvm_unreachable("There were no current transitions between blocks");
  }
  builder->setInsertionPoint(branch);
}

bool hermes::isSimpleSideEffectFreeInstruction(Instruction *I) {
  if (I->hasSideEffect()) {
    return false;
  }
  switch (I->getKind()) {
    case ValueKind::GetNewTargetInstKind:
    case ValueKind::UnaryOperatorInstKind:
    case ValueKind::BinaryOperatorInstKind:
    case ValueKind::HBCResolveEnvironmentKind:
    case ValueKind::HBCLoadConstInstKind:
    case ValueKind::HBCGetGlobalObjectInstKind:
      return true;
    default:
      return false;
  }
  llvm_unreachable("unreachable");
}

#undef DEBUG_TYPE
