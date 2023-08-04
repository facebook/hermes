/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "instnormalize"

#include "hermes/Optimizer/Scalar/InstCanonicalize.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

using namespace hermes;

STATISTIC(NumCanonicalized, "Number of instructions canonicalized");

namespace {

Value *canonicalizeUnOp(UnaryOperatorInst *unary) {
  IRBuilder builder(unary->getParent()->getParent());

  auto kind = unary->getOperatorKind();
  auto *op = unary->getSingleOperand();

  // If the operand is a literal, try to evaluate the expression.
  // We do this in canonicalization phase as minifiers often use
  // these unaries where literals would otherwise be used, e.g.
  // `void 0` => `undefined`; `!0` => `true`; and `!1` to `false`.
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto result = evalUnaryOperator(kind, builder, lit)) {
      return result;
    }
  }

  return nullptr;
}

const Type kNonStringPrimitive = Type::unionTy(
    Type::createNumeric(),
    Type::unionTy(
        Type::createBoolean(),
        Type::unionTy(Type::createNull(), Type::createUndefined())));

llvh::Optional<BinaryOperatorInst::OpKind> staticSafeReverseOperator(
    BinaryOperatorInst *binary) {
  auto *lhs = binary->getLeftHandSide();
  auto *rhs = binary->getRightHandSide();

  // If both operands have a side-effect, then changing the order can change
  // code semantics. But if just one has a side-effect, order shouldn't matter.
  if (!isSideEffectFree(lhs->getType()) && !isSideEffectFree(lhs->getType()))
    return llvh::None;

  auto kind = binary->getOperatorKind();

  switch (kind) {
    // We have to special case the '+' operator as when it acts
    // on runtime objects and strings, it concatenates which is
    // is not a reversible operation.
    case BinaryOperatorInst::OpKind::AddKind:
      for (const auto op : {lhs, rhs}) {
        const auto *strOp = llvh::dyn_cast<LiteralString>(op);
        // Special case empty string as the only reversible concat.
        if (strOp && strOp->getValue().str().empty())
          break;
        // Otherwise all other operands must be non-string primitives.
        if (!op->getType().isSubsetOf(kNonStringPrimitive))
          return llvh::None;
      }
      // If we pass all the checks, we can safely commute '+'.
      return kind;
    default:
      break;
  }

  return BinaryOperatorInst::tryGetReverseOperator(kind);
}

Value *canonicalizeBinOp(BinaryOperatorInst *binary) {
  using OpKind = BinaryOperatorInst::OpKind;

  IRBuilder builder(binary->getParent()->getParent());

  auto *lhs = binary->getLeftHandSide();
  auto *rhs = binary->getRightHandSide();
  auto kind = binary->getOperatorKind();

  auto *litLhs = llvh::dyn_cast<Literal>(lhs);
  auto *litRhs = llvh::dyn_cast<Literal>(rhs);

  // We evaluate binary operations on literals to make sure
  // string concatentation and other const exprs canonicalize.
  if (litLhs && litRhs) {
    if (auto result = evalBinaryOperator(kind, builder, litLhs, litRhs)) {
      return result;
    }
  }

  // We organize literals to appear on the right hand side which
  // simplifies pattern matching and aids CSE.
  if (litLhs && !litRhs) {
    if (auto reverseKind = staticSafeReverseOperator(binary)) {
      std::swap(lhs, rhs);
      std::swap(litLhs, litRhs);
      std::swap(kind, *reverseKind);
    }
  }

  switch (kind) {
    case OpKind::EqualKind: // ==
    case OpKind::NotEqualKind: // !=
      // Comparisons to null and undefined are loosely equivalent. While
      // minifiers often prefer `null`s over `void 0`, leveraging undefined
      // consistently presents more opportunity for optimization.
      for (const auto [op, litOp] :
           {std::pair{&lhs, &litLhs}, std::pair{&rhs, &litRhs}}) {
        if ((*op)->getKind() == ValueKind::LiteralNullKind)
          *op = (*litOp = builder.getLiteralUndefined());
      }
      break;

    default:
      break;
  }

  if (lhs != binary->getLeftHandSide() || rhs != binary->getRightHandSide() ||
      kind != binary->getOperatorKind()) {
    builder.setInsertionPoint(binary);
    return builder.createBinaryOperatorInst(lhs, rhs, kind);
  }

  return nullptr;
}

/// Try to normalize the instruction \p I.
/// \returns one of:
///   - nullptr if the instruction cannot be normalized.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> canonicalizeInstruction(Instruction *I) {
  switch (I->getKind()) {
    case ValueKind::UnaryOperatorInstKind:
      return canonicalizeUnOp(cast<UnaryOperatorInst>(I));
    case ValueKind::BinaryOperatorInstKind:
      return canonicalizeBinOp(cast<BinaryOperatorInst>(I));
    default:
      // TODO: handle other kinds of instructions.
      return nullptr;
  }
}

} // namespace

bool InstCanonicalize::runOnFunction(Function *F) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;

  // For all reachable blocks in the function, in RPO order.
  PostOrderAnalysis PO(F);
  for (BasicBlock *BB : llvh::reverse(PO)) {
    // For all instructions:
    for (auto instIter = BB->begin(), e = BB->end(); instIter != e;) {
      Instruction *II = &*instIter;
      ++instIter;

      auto optNewVal = canonicalizeInstruction(II);
      if (optNewVal.hasValue()) {
        auto newVal = optNewVal.getValue();
        if (!newVal)
          continue;

        changed = true;
        NumCanonicalized++;

        // Instruction changed inplace.
        if (II == newVal)
          continue;

        // We have a more canonicalized instruction. Replace the original
        // instruction and mark it for deletion.
        II->replaceAllUsesWith(newVal);
      } else {
        changed = true;
        NumCanonicalized++;
      }
      destroyer.add(II);
    }
  }

  return changed;
}

std::unique_ptr<Pass> hermes::createInstCanonicalize() {
  return std::make_unique<InstCanonicalize>();
}

#undef DEBUG_TYPE
