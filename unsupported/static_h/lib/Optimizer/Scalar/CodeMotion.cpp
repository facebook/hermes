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
    if (I->hasSideEffect() && copy->hasSideEffect())
      return nullptr;

    searchBudget--;
    copy = copy->getNextNode();

    if (llvh::isa<TerminatorInst>(copy))
      return nullptr;
  }

  return nullptr;
}

/// Try to hoist instructions from both sides of the branch.
/// \returns true if some instructions were hoisted.
static bool hoistCBI(CondBranchInst *CBI) {
  // Don't hoist instructions across conditional branches that can throw.
  if (CBI->hasSideEffect())
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

    // Don't hoist terminators.
    if (llvh::isa<TerminatorInst>(I0) || llvh::isa<TerminatorInst>(I1))
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
  if (!isSimpleSideEffectFreeInstruction(inst)) {
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

/// Try to sink operands of instructions in the basic block \p BB.
static bool sinkInstructionsInBlock(
    BasicBlock *BB,
    const DominanceInfo &dominance,
    const LoopAnalysis &loops) {
  bool changed = false;
  const bool inLoop = loops.isBlockInLoop(BB);
  const BasicBlock *header = loops.getLoopHeader(BB);
  for (auto it = BB->rbegin(), e = BB->rend(); it != e; ++it) {
    Instruction *inst = &*it;

    if (llvh::isa<PhiInst>(inst))
      continue;

    for (int i = 0, numOperands = inst->getNumOperands(); i < numOperands;
         i++) {
      auto *I = llvh::dyn_cast<Instruction>(inst->getOperand(i));
      // Don't touch non-instructions, special instructions, instructions that
      // have multiple uses or instructions with side effects.
      if (!I || !I->hasOneUser() || I->hasSideEffect() ||
          llvh::isa<PhiInst>(I) || llvh::isa<TerminatorInst>(I) ||
          llvh::isa<CreateArgumentsInst>(I))
        continue;

      // If block is in a loop, only sink instructions from the same loop.
      BasicBlock *parent = I->getParent();
      if (inLoop && parent != BB && (loops.getLoopHeader(parent) != header)) {
        continue;
      }

      I->moveBefore(inst);
      changed = true;
      ++NumCM;
      ++NumSunk;
    }
  }
  return changed;
}

bool CodeMotion::runOnFunction(Function *F) {
  bool changed = false;
  PostOrderAnalysis PO(F);

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
  //   1. Sink instruction operands to where we need them. This shortens the
  //      lifetime of instructions and reduces register pressure.
  //   2. Hoist instructions out of loops to avoid repeated evaluation.
  //
  // Do it in this order so that (hopefully) operands are sunk and then both
  // they and the instruction that uses them can all be hoisted at once.
  // Otherwise, that instruction couldn't be hoisted because its operands are
  // still in (an earlier block of) the loop.
  //
  // Note: ideally we would visit the blocks from inner loops first and outer
  // loops last, but post-order doesn't guarantee that.
  for (auto *BB : PO) {
    changed |= sinkInstructionsInBlock(BB, dominance, loops);
    changed |= hoistInstructionsFromLoop(BB, dominance, loops);
  }

  return changed;
}

Pass *hermes::createCodeMotion() {
  return new CodeMotion();
}

#undef DEBUG_TYPE
