/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_UTILS_H
#define HERMES_OPTIMIZER_SCALAR_UTILS_H

#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
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

/// \return the callee Function for \p callee or nullptr, if none was found.
Function *getCallee(Value *callee);

/// \returns True if the value \p C is the caller's callee and is not captured
///   by any of the arguments.
bool isDirectCallee(Value *C, CallInst *CI);

/// Collect the call sites for function \p F in \p callsites.
/// \returns True if all call sites are known and \p callsites is valid.
bool getCallSites(Function *F, llvh::SmallVectorImpl<CallInst *> &callsites);

/// Delete all incoming arrows from \p incoming in PhiInsts in \p blockToModify.
bool deleteIncomingBlockFromPhis(
    BasicBlock *blockToModify,
    BasicBlock *incoming);

/// Position a builder on the arrow between \p from and \p to.
/// This can be used to insert instructions to be run on the transition from one
/// block to another. After this call, the blocks may no longer be neighbors
/// due to an inserted block (and can therefore fail when called twice).
void splitCriticalEdge(IRBuilder *builder, BasicBlock *from, BasicBlock *to);

/// \returns True if the instruction \p I has no side effects, can be combined
/// with identical instructions or duplicated without changing semantics, and
/// can be placed anywhere in the middle of a basic block.
bool isSimpleSideEffectFreeInstruction(Instruction *I);
} // namespace hermes
#endif // HERMES_OPTIMIZER_SCALAR_UTILS_H
