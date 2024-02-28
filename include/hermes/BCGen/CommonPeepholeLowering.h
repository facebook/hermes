/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_PEEPHOLE_H
#define HERMES_BCGEN_PEEPHOLE_H

#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

namespace hermes {

/// Lower CoerceThisNSInst into LIRGetThisNSInst if possible.
/// \return the lowered instruction, or nullptr if it was not lowered.
/// The caller is expected to make the call to replaceAllUsesWith on \p CTI.
inline Value *lowerCoerceThisNSInst(
    CoerceThisNSInst *CTI,
    IRBuilder &builder,
    IRBuilder::InstructionDestroyer &destroyer) {
  // If the operand is LoadParamInst with one user, loading "this",
  // we can replace them with Load
  if (auto *LPI = llvh::dyn_cast<LoadParamInst>(CTI->getSingleOperand())) {
    auto *F = CTI->getParent()->getParent();
    if (LPI->hasOneUser() && LPI->getParam() == F->getJSDynamicParam(0)) {
      destroyer.add(CTI);
      destroyer.add(LPI);
      builder.setInsertionPointAfter(CTI);
      return builder.createLIRGetThisNSInst();
    }
  }
  return nullptr;
}

/// \return the lowered instruction.
/// The caller is expected to make the call to replaceAllUsesWith on \p BO.
inline Value *lowerBinaryExponentiationInst(
    BinaryOperatorInst *BO,
    IRBuilder &builder,
    IRBuilder::InstructionDestroyer &destroyer) {
  destroyer.add(BO);
  builder.setInsertionPoint(BO);
  auto *CBI = builder.createCallBuiltinInst(
      BuiltinMethod::HermesBuiltin_exponentiationOperator,
      {BO->getLeftHandSide(), BO->getRightHandSide()});
  CBI->setType(BO->getType());
  return CBI;
}

} // namespace hermes

#endif
