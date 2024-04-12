/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_UTILS_H
#define HERMES_OPTIMIZER_SCALAR_UTILS_H

#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/IR/Instrs.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {

/// If this is a variable that only has stores of a single value then
/// return the value that initializes the variable.
///
/// Notice that all variable are always explicitly initialized by stores and the
/// initialization dominates any other uses. So if we find a single value
/// stored, that means that the optimizer was able to figure out that the whole
/// lifetime of the variable is constant.
Value *isStoreOnceVariable(Variable *V);

/// If this is a stack location that only has stores of a single value then
/// return the value that initializes the variable.
///
/// Notice that all stack locations are always explicitly initialized by stores
/// and the initialization dominates any other uses. So if we find a single
/// value stored, that means that the optimizer was able to figure out that the
/// whole lifetime of the variable is constant.
Value *isStoreOnceStackLocation(AllocStackInst *AS);

/// \return True if \p V is an instruction that may be used in a constructor
/// invocation of the \p closure. In the absence of other instructions that
/// manipulate the closure, these instructions cannot leak the closure.
bool isConstructionSetup(Value *V, Value *closure);

/// \return a list of known callsites of \p F based on its users.
/// It is possible that \p F has additional unknown callsites, call
/// \c F->allCallsitesKnown() to check that.
llvh::SmallVector<BaseCallInst *, 2> getKnownCallsites(Function *F);

/// Delete all incoming arrows from \p incoming in PhiInsts in \p blockToModify.
bool deleteIncomingBlockFromPhis(
    BasicBlock *blockToModify,
    BasicBlock *incoming);

/// If all of the incoming values to the Phi that are not self-edges are the
/// same, return that value. Otherwise return nullptr.
Value *getSinglePhiValue(PhiInst *P);

/// Position a builder on the arrow between \p from and \p to.
/// This can be used to insert instructions to be run on the transition from one
/// block to another. After this call, the blocks may no longer be neighbors
/// due to an inserted block (and can therefore fail when called twice).
void splitCriticalEdge(IRBuilder *builder, BasicBlock *from, BasicBlock *to);

/// Split a basic block into two at the specified instruction.
/// The instructions before \p it are retained in \p BB, while those after and
/// including \p it are moved into a newly-created successor basic block. Phi
/// instructions in successors of \p BB will be updated to refer to the new
/// BasicBlock. \p makeTerminator is called with the new basic block and returns
/// a TerminatorInst used to terminate the original block. \return the newly
/// created basic block.
template <typename CB>
BasicBlock *splitBasicBlock(
    BasicBlock *BB,
    BasicBlock::InstListType::iterator it,
    CB makeTerminator) {
  Function *F = BB->getParent();

  IRBuilder builder(F);
  auto *newBB = builder.createBasicBlock(F);

  for (auto *succ : successors(BB))
    updateIncomingPhiValues(succ, BB, newBB);

  // Move the instructions after the split point into the new BB.
  newBB->getInstList().splice(
      newBB->end(), BB->getInstList(), it, BB->getInstList().end());

  // setParent is not called by splice, so add it ourselves.
  for (auto &movedInst : *newBB)
    movedInst.setParent(newBB);

  makeTerminator(newBB);

  return newBB;
}

/// Delete all variables that have no remaining uses.
/// \return true if anything was deleted, false otherwise.
bool deleteUnusedVariables(Module *M);

/// Delete all unused functions, and then delete any variables that have no
/// remaining uses.
/// \return true if anything was deleted, false otherwise.
bool deleteUnusedFunctionsAndVariables(Module *M);
} // namespace hermes
#endif // HERMES_OPTIMIZER_SCALAR_UTILS_H
