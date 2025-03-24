/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "codemotion"
#include "hermes/Optimizer/Scalar/CodeMotion.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"

#include <algorithm>

using namespace hermes;

STATISTIC(NumCM, "Number of instructions moved");
STATISTIC(NumHoistedCond, "Number of instructions hoisted from conditionals");
STATISTIC(NumHoistedLoop, "Number of instructions hoisted from loops");
STATISTIC(NumSunk, "Number of instructions sunk");

/// Search the \p searchBudget instructions following \p copy in search of
/// instructions that are identical to \p I and can be hoisted. Stop the search
/// on any optimization barrier.
/// \returns the identical instruction or null if none was found.
static Instruction *findIdenticalInWindow(
    Instruction *I,
    Instruction *copy,
    unsigned searchBudget) {
  // Don't hoist terminators.
  if (llvh::isa<TerminatorInst>(I) || llvh::isa<TerminatorInst>(copy))
    return nullptr;

  while (searchBudget) {
    if (I->isIdenticalTo(copy))
      return copy;

    // Stop the search on instructions with side effects, if the instruction
    // that we will be hoisting has side effects.
    if (I->getSideEffect().mayReadOrWorse() &&
        copy->getSideEffect().mayReadOrWorse())
      return nullptr;

    searchBudget--;
    copy = copy->getNextNode();

    if (llvh::isa<TerminatorInst>(copy))
      return nullptr;
  }

  return nullptr;
}

/// Check whether \p inst can be hoisted out of its basic block.
/// \returns true if \p inst is safe to hoist.
static inline bool canHoistFromCondBranch(Instruction *inst) {
  // Terminators and instructions that need to be first should not be hoisted.
  return !(
      llvh::isa<TerminatorInst>(inst) ||
      inst->getSideEffect().getFirstInBlock());
}

/// Try to hoist instructions from both sides of the branch.
/// \returns true if some instructions were hoisted.
static bool hoistCBI(CondBranchInst *CBI) {
  // Don't hoist instructions across conditional branches that can throw.
  if (CBI->getSideEffect().mayReadOrWorse())
    return false;

  BasicBlock *BB0 = CBI->getTrueDest();
  BasicBlock *BB1 = CBI->getFalseDest();

  // Don't hoist instructions if there are multiple predecessors.
  if (BB0 == BB1 || pred_count(BB0) != 1 || pred_count(BB1) != 1)
    return false;

  bool changed = false;

  while (true) {
    Instruction *I0 = &*BB0->begin();
    Instruction *I1 = &*BB1->begin();

    if (!(canHoistFromCondBranch(I0) && canHoistFromCondBranch(I1)))
      return changed;

    Instruction *LHS;
    Instruction *RHS;

    /// Search for instructions that are identical on the left and right sides
    /// of the branch.
    if (Instruction *copy = findIdenticalInWindow(I0, I1, 5)) {
      LHS = I0;
      RHS = copy;
    } else if (Instruction *copy2 = findIdenticalInWindow(I1, I0, 5)) {
      LHS = I1;
      RHS = copy2;
    } else {
      // Could not find identical instructions. exit.
      return changed;
    }

    changed = true;
    ++NumCM;
    ++NumHoistedCond;
    LHS->moveBefore(CBI);
    RHS->replaceAllUsesWith(LHS);
    RHS->eraseFromParent();
  }
}

/// Check whether \p inst, an instruction in a loop, can be hoisted to just
/// before \p branchInst, the last instruction in the preheader of the loop.
/// Only certain types of instructions can be hoisted, and their dependencies
/// must dominate \p branchInst.
/// \param dominance the dominance tree for the function
/// \returns true if \p inst is safe to hoist.
static bool canHoistFromLoop(
    Instruction *inst,
    Instruction *branchInst,
    const DominanceInfo &dominance) {
  // Check whether the instruction is pure, and whether it has restrictions on
  // where it can be placed within a block.
  if (llvh::isa<TerminatorInst>(inst) || !inst->getSideEffect().isPure() ||
      inst->getSideEffect().getFirstInBlock()) {
    return false;
  }
  for (int i = 0, e = inst->getNumOperands(); i < e; ++i) {
    auto *operand = llvh::dyn_cast<Instruction>(inst->getOperand(i));
    if (operand && !dominance.properlyDominates(operand, branchInst)) {
      return false;
    }
  }
  return true;
}

/// Try to hoist instructions from a loop block to the preheader.
/// \returns true if some instructions were hoisted.
static bool hoistInstructionsFromLoop(
    BasicBlock *BB,
    const DominanceInfo &dominance,
    const LoopAnalysis &loops) {
  bool changed = false;
  BasicBlock *preheader = loops.getLoopPreheader(BB);
  if (!preheader) {
    // TODO: If there is a header but no preheader, we can just create one. This
    // is tricky while traversing the CFG so we could create one for every loop
    // beforehand and let SimplifyCFG remove unnecessary ones. The need for this
    // isn't all that uncommon (example: when an if-statement precedes a loop).
    return changed;
  }
  Instruction *branchInst = &preheader->back();

  for (auto it = BB->begin(), e = BB->end(); it != e;) {
    // Save the advanced iterator here since calling inst->moveBefore below
    // invalidates the iterator.
    auto nextIt = std::next(it);
    Instruction *inst = &*it;
    if (canHoistFromLoop(inst, branchInst, dominance)) {
      inst->moveBefore(branchInst);
      changed = true;
      ++NumCM;
      ++NumHoistedLoop;
    }
    it = nextIt;
  }
  return changed;
}

/// Try to sink instructions in the block \p BB to the point where they are
/// used.
static bool sinkInstructionsInBlock(
    BasicBlock *BB,
    const DominanceInfo &dominance,
    const LoopAnalysis &loops) {
  bool changed = false;
  const BasicBlock *header = loops.getLoopHeader(BB);

  for (auto it = BB->rbegin(); it != BB->rend();) {
    auto *I = &*(it++);
    auto se = I->getSideEffect();

    // If the instruction cannot be moved, or if it can observe or modify memory
    // locations, skip it.
    if (se.getFirstInBlock() || se.mayReadOrWorse() ||
        llvh::isa<CreateArgumentsInst>(I) || llvh::isa<TerminatorInst>(I))
      continue;

    // If the instruction only has one user, we can move it right before that
    // user, as long as that user is not FirstInBlock.
    if (I->hasOneUser() &&
        !I->getUsers()[0]->getSideEffect().getFirstInBlock()) {
      auto *user = I->getUsers()[0];
      // If the user is already the next instruction, nothing to do.
      if (I->getNextNode() == user)
        continue;

      auto *userBB = user->getParent();

      // We cannot sink into a different loop, because it may be suboptimal, and
      // the instruction may not be idempotent.
      if (userBB != BB && loops.isBlockInLoop(userBB)) {
        auto *ipHeader = loops.getLoopHeader(userBB);
        // Note that if the header is null, we cannot prove that it is the same
        // loop.
        if (!ipHeader || ipHeader != header)
          continue;
      }

      I->moveBefore(user);
      changed = true;
      ++NumCM;
      ++NumSunk;
    }

    // If the instruction has multiple users, we will sink it to the first
    // common dominator of all of them, tracked by this variable.
    BasicBlock *newBlock = nullptr;

    /// Update newBlock to account for a user in \p useBB.
    auto merge = [&newBlock, &dominance](BasicBlock *useBB) {
      newBlock = newBlock
          ? dominance.findNearestCommonDominator(newBlock, useBB)
          : useBB;
    };
    for (auto *U : I->getUsers()) {
      if (auto *phi = llvh::dyn_cast<PhiInst>(U)) {
        // For Phi, ensure that the new block dominates the blocks from which
        // this instruction is the incoming value.
        for (unsigned i = 0, e = phi->getNumEntries(); i < e; ++i) {
          auto [val, pred] = phi->getEntry(i);
          if (val == I)
            merge(pred);
        }
        continue;
      }

      // If the user is some other FirstInBlock instruction, give up.
      if (U->getSideEffect().getFirstInBlock()) {
        newBlock = nullptr;
        break;
      }

      // Ensure that the new block dominates this user.
      merge(U->getParent());
    }

    // If no new block was determined, move on.
    if (!newBlock || newBlock == BB)
      continue;

    // If we cannot prove that the new location is in the same loop as the
    // current location, bail.
    if (loops.isBlockInLoop(newBlock)) {
      auto *nbHeader = loops.getLoopHeader(newBlock);
      if (!nbHeader || nbHeader != header)
        continue;
    }

    // Move the instruction to the first available location in the new block.
    for (auto &newLoc : *newBlock) {
      if (!newLoc.getSideEffect().getFirstInBlock()) {
        I->moveBefore(&newLoc);
        changed = true;
        ++NumCM;
        ++NumSunk;
        break;
      }
    }
  }
  return changed;
}

bool CodeMotion::runOnFunction(Function *F) {
  bool changed = false;
  auto PO = postOrderAnalysis(F);

  for (auto &BB : PO) {
    auto *term = BB->getTerminator();

    if (auto *CBI = llvh::dyn_cast<CondBranchInst>(term)) {
      changed |= hoistCBI(CBI);
    }
  }

  DominanceInfo dominance(F);
  LoopAnalysis loops(F, dominance);

  // Scan the function in post order (from end to start) and:
  //
  //   1. First, sink instructions to where we need them. This shortens their
  //      lifetime and reduces register pressure.
  //   2. Once instructions in all blocks have been sunk, hoist instructions out
  //      of loops to avoid repeated evaluation.
  //
  // Do it in this order so that (hopefully) operands are sunk and then both
  // they and the instruction that uses them can all be hoisted at once.
  // Otherwise, that instruction couldn't be hoisted because its operands are
  // still in (an earlier block of) the loop.
  for (auto *BB : PO)
    changed |= sinkInstructionsInBlock(BB, dominance, loops);

  // Note: ideally we would visit the blocks from inner loops first and outer
  // loops last, but post-order doesn't guarantee that.
  for (auto *BB : PO)
    changed |= hoistInstructionsFromLoop(BB, dominance, loops);

  return changed;
}

Pass *hermes::createCodeMotion() {
  return new CodeMotion();
}

#undef DEBUG_TYPE
