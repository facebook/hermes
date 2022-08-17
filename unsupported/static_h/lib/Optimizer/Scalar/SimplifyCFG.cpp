/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "simplifycfg"

#include "hermes/Optimizer/Scalar/SimplifyCFG.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/ADT/SmallVector.h"

using namespace hermes;

STATISTIC(NumUnreachableBlock, "Number of unreachable blocks eliminated");
STATISTIC(NumSB, "Number of static branches simplified");

/// \returns true if the control-flow edge between \p src to \p dest crosses
/// a catch region.
static bool isCrossCatchRegionBranch(BasicBlock *src, BasicBlock *dest) {
  auto kind = dest->front().getKind();
  if (kind == ValueKind::TryStartInstKind ||
      kind == ValueKind::TryEndInstKind || kind == ValueKind::CatchInstKind)
    return true;
  return false;
}

/// \returns true if the block \b BB is an input to a PHI node.
static bool isUsedInPhiNode(BasicBlock *BB) {
  for (auto use : BB->getUsers())
    if (llvh::isa<PhiInst>(use))
      return true;

  return false;
}

static void removeEntryFromPhi(BasicBlock *BB, BasicBlock *edge) {
  // For all PHI nodes in block:
  for (auto &it : *BB) {
    auto *P = llvh::dyn_cast<PhiInst>(&it);
    if (!P)
      continue;
    // For each Phi entry:
    for (int i = 0, e = P->getNumEntries(); i < e; i++) {
      auto E = P->getEntry(i);
      // Remove the incoming edge.
      if (E.second == edge) {
        P->removeEntry(i);
        break;
      }
    }
  }
}

/// Delete the conditional branch and create a new direct branch to the
/// destination block \p dest.
static void replaceCondBranchWithDirectBranch(
    CondBranchInst *CB,
    BasicBlock *dest) {
  BasicBlock *currentBlock = CB->getParent();
  auto *trueDest = CB->getTrueDest();
  auto *falseDest = CB->getFalseDest();

  if (trueDest != dest)
    removeEntryFromPhi(trueDest, currentBlock);
  if (falseDest != dest)
    removeEntryFromPhi(falseDest, currentBlock);

  IRBuilder builder(currentBlock->getParent());
  builder.setInsertionBlock(currentBlock);
  builder.createBranchInst(dest);
  CB->eraseFromParent();
}

/// Try to remove a branch used by phi nodes.
static bool attemptBranchRemovalFromPhiNodes(BasicBlock *BB) {
  // Only handle blocks that are a single, unconditional branch.
  if (BB->getTerminator() != &*(BB->begin()) ||
      BB->getTerminator()->getKind() != ValueKind::BranchInstKind) {
    return false;
  }

  // Find our parents and also ensure that there aren't
  // any instructions we can't handle.
  llvh::SmallPtrSet<BasicBlock *, 8> blockParents;
  // Keep unique parents by the original order, which is deterministic.
  llvh::SmallVector<BasicBlock *, 8> orderedParents;
  for (const auto *user : BB->getUsers()) {
    switch (user->getKind()) {
      case ValueKind::BranchInstKind:
      case ValueKind::CondBranchInstKind:
      case ValueKind::SwitchInstKind:
      case ValueKind::GetPNamesInstKind:
      case ValueKind::GetNextPNameInstKind:
        // This is an instruction where the branch argument is a simple
        // jump target that can be substituted for any other branch.
        if (blockParents.count(user->getParent()) == 0) {
          orderedParents.push_back(user->getParent());
        }
        blockParents.insert(user->getParent());
        break;
      case ValueKind::PhiInstKind:
        // The branch argument is not a jump target, but we know how
        // to rewrite them.
        break;
      default:
        // This is some other instruction where we don't know whether we can
        // unconditionally substitute another branch. Bail for safety.
        return false;
    }
  }

  if (blockParents.empty()) {
    return false;
  }

  BasicBlock *phiBlock = nullptr;

  // Verify that we'll be able to rewrite all relevant Phi nodes.
  for (auto *user : BB->getUsers()) {
    if (auto *phi = llvh::dyn_cast<PhiInst>(user)) {
      if (phiBlock && phi->getParent() != phiBlock) {
        // We have PhiInsts in multiple blocks referencing BB, but BB is a
        // single static branch. This is invalid, but the bug is elsewhere.
        llvm_unreachable(
            "Different BBs use Phi for BB with single static jump");
      }
      phiBlock = phi->getParent();

      Value *ourValue = nullptr;
      for (unsigned int i = 0; i < phi->getNumEntries(); i++) {
        auto entry = phi->getEntry(i);
        if (entry.second == BB) {
          if (ourValue) {
            // The incoming phi node is invalid. The problem is not here.
            llvm_unreachable("Phi node has multiple entries for a branch.");
          }
          ourValue = entry.first;
        }
      }

      if (!ourValue) {
        llvm_unreachable("getUsers returned a non-user PhiInst");
      }

      for (unsigned int i = 0; i < phi->getNumEntries(); i++) {
        auto entry = phi->getEntry(i);
        if (blockParents.count(entry.second)) {
          // We have a PhiInst referencing our block BB and its parent, e.g.
          // %BB0:
          // CondBranchInst %1, %BB1, %BB2
          // %BB1:
          // BranchInst %BB2
          // %BB2:
          // PhiInst ??, %BB0, ??, %BB1
          if (entry.first == ourValue) {
            // Fortunately, the two values are equal, so we can rewrite to:
            // PhiInst ??, %BB0
          } else {
            // Unfortunately, the value is different in each case,
            // which naively would have led to an invalid rewrite like:
            // PhiInst %1, %BB0, %2, %BB0
            return false;
          }
        }
      }
    }
  }
  if (!phiBlock) {
    llvm_unreachable("We shouldn't be in this function if there are no Phis.");
  }

  // This branch is removable. Start by rewriting the Phi nodes.
  for (auto *user : BB->getUsers()) {
    if (auto *phi = llvh::dyn_cast<PhiInst>(user)) {
      Value *ourValue = nullptr;

      const unsigned int numEntries = phi->getNumEntries();
      for (unsigned int i = 0; i < numEntries; i++) {
        auto entry = phi->getEntry(i);
        if (entry.second == BB) {
          ourValue = entry.first;
        }
      }
      assert(ourValue && "getUsers returned a non-user PhiInst");

      for (int i = phi->getNumEntries() - 1; i >= 0; i--) {
        auto pair = phi->getEntry(i);
        if (pair.second == BB || blockParents.count(pair.second)) {
          phi->removeEntry(i);
        }
      }

      // Add parents back in sorted order to avoid any non-determinism
      for (BasicBlock *parent : orderedParents) {
        phi->addEntry(ourValue, parent);
      }
    }
  }
  // We verified earlier that all uses are branches and phis, so now that
  // we've rewritten the phis, we can have all branches jump there directly.
  BB->replaceAllUsesWith(phiBlock);
  BB->eraseFromParent();
  return true;
}

/// Get rid of trampolines and merge basic blocks that are split by static
/// non-conditional branches.
static bool optimizeStaticBranches(Function *F) {
  bool changed = false;
  IRBuilder builder(F);

  // Remove conditional branches with a constant condition.
  for (auto &it : *F) {
    BasicBlock *BB = &it;
    auto *cbr = llvh::dyn_cast<CondBranchInst>(BB->getTerminator());
    if (!cbr)
      continue;

    BasicBlock *trueDest = cbr->getTrueDest();
    BasicBlock *falseDest = cbr->getFalseDest();

    // If both sides of the branch point to the same block then just jump to it
    // directly.
    if (trueDest == falseDest) {
      replaceCondBranchWithDirectBranch(cbr, trueDest);
      changed = true;
      ++NumSB;
      continue;
    }

    // If the condition is optimized to a literal bool then replace the branch
    // with a non-conditional branch.
    auto *cond = cbr->getCondition();
    BasicBlock *dest = nullptr;

    if (LiteralBool *B = evalToBoolean(builder, cond)) {
      if (B->getValue()) {
        dest = trueDest;
      } else {
        dest = falseDest;
      }
    }

    if (dest != nullptr) {
      replaceCondBranchWithDirectBranch(cbr, dest);
      ++NumSB;
      changed = true;
      continue;
    }
  } // for all blocks.

  // Check if a basic block is a simple trampoline (empty non-conditional branch
  // to another basic block) and get rid of it. Replace all uses of the current
  // block with the destination of this block.
  for (auto &it : *F) {
    BasicBlock *BB = &it;
    auto *br = llvh::dyn_cast<BranchInst>(BB->getTerminator());
    if (!br)
      continue;

    BasicBlock *dest = br->getBranchDest();

    // Don't try to optimize infinite loops or unreachable blocks.
    if (dest == BB)
      continue;

    // Don't handle edges that go across any catch region.
    if (isCrossCatchRegionBranch(BB, dest))
      continue;

    // Handle branches used in phi nodes specially.
    if (isUsedInPhiNode(BB)) {
      if (attemptBranchRemovalFromPhiNodes(BB)) {
        ++NumSB;
        changed = true;
        break; // Iterator invalidated.
      }
      continue;
    }

    // Check if the terminator is the only instruction in the block.
    bool isSingleInstr = (&*BB->begin() == br);

    // If the first and only instruction is a static branch, and it does not
    // cross a catch boundary then redirect all predecessors to the destination.
    if (isSingleInstr && !pred_empty(BB)) {
      BB->replaceAllUsesWith(dest);
      ++NumSB;
      changed = true;
      assert(pred_count(BB) == 0);
      continue;
    }

    // If the source block is not empty then try to slurp the destination block
    // and eliminate it altogether.
    if (pred_count(dest) == 1 && dest != BB) {
      // Slurp the instructions from the destination block one by one.
      while (dest->begin() != dest->end()) {
        dest->begin()->moveBefore(br);
      }

      // Now that we moved all of the instructions from the destination block we
      // can delete the original terminator and delete the destination block.
      dest->replaceAllUsesWith(BB);
      br->eraseFromParent();
      dest->eraseFromParent();
      ++NumSB;
      changed = true;

      // We are invalidating the iterator here. Stop the scan and continue
      // afresh in the next iteration.
      break;
    }
  } // for all blocks.

  return changed;
}

/// Process the deletion of the basic block, and erase it.
static void deleteBasicBlock(BasicBlock *B) {
  // Remove all uses of this basic block.

  // Copy the uses of the block aside because removing the users invalidates the
  // iterator.
  Value::UseListTy users(B->getUsers().begin(), B->getUsers().end());

  // Remove the block from all Phi instructions referring to it. Note that
  // reachable blocks could end up with Phi instructions referring to
  // unreachable blocks.
  for (auto *I : users) {
    if (auto *Phi = llvh::dyn_cast<PhiInst>(I)) {
      Phi->removeEntry(B);
      continue;
    }
  }

  // There may still be uses of the block from other unreachable blocks.
  B->replaceAllUsesWith(nullptr);
  // Erase this basic block.
  B->eraseFromParent();
}

/// Remove all the unreachable basic blocks.
static bool removeUnreachedBasicBlocks(Function *F) {
  bool changed = false;

  // Visit all reachable blocks.
  llvh::SmallPtrSet<BasicBlock *, 16> visited;
  llvh::SmallVector<BasicBlock *, 32> workList;

  workList.push_back(&*F->begin());
  while (!workList.empty()) {
    auto *BB = workList.pop_back_val();
    // Already visited?
    if (!visited.insert(BB).second)
      continue;

    for (auto *succ : successors(BB))
      workList.push_back(succ);
  }

  // Delete all blocks that weren't visited.
  for (auto it = F->begin(), e = F->end(); it != e;) {
    auto *BB = &*it++;
    if (!visited.count(BB)) {
      ++NumUnreachableBlock;
      deleteBasicBlock(BB);
      changed = true;
    }
  }

  return changed;
}

bool SimplifyCFG::runOnFunction(hermes::Function *F) {
  bool changed = false;

  bool iterChanged = false;
  // Keep iterating over deleting unreachable code and removing trampolines as
  // long as we are making progress.
  do {
    iterChanged = optimizeStaticBranches(F) || removeUnreachedBasicBlocks(F);
    changed |= iterChanged;
  } while (iterChanged);

  return changed;
}

Pass *hermes::createSimplifyCFG() {
  return new SimplifyCFG();
}

#undef DEBUG_TYPE
