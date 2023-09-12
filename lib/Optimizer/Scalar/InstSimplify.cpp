/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "instsimplify"

#include "hermes/Optimizer/Scalar/InstSimplify.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseSet.h"

using namespace hermes;

STATISTIC(NumSimp, "Number of instructions simplified");

namespace {

constexpr Type kNullOrUndef =
    Type::unionTy(Type::createUndefined(), Type::createNull());

bool canBeNaN(Value *value) {
  if (!value->getType().canBeNumber()) {
    return false;
  }

  if (auto *literalNumber = llvh::dyn_cast<LiteralNumber>(value)) {
    return std::isnan(literalNumber->getValue());
  }

  return true;
}

template <typename T>
Value *reduceAsNumberLike(T *asNumber) {
  IRBuilder builder(asNumber->getParent()->getParent());
  auto *op = asNumber->getSingleOperand();

  // If operand is a literal, try to evaluate ToNumber(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToNumber(builder, lit)) {
      return result;
    }
  }

  // No need to convert if the value is a number already.
  if (op->getType().isNumberType()) {
    return op;
  }

  // If it can't be converted, return the original instruction.
  return asNumber;
}

Value *reduceAsNumber(AsNumberInst *asNumber) {
  return reduceAsNumberLike(asNumber);
}

Value *reduceAsNumeric(AsNumericInst *asNumeric) {
  auto *op = asNumeric->getSingleOperand();
  if (op->getType().isBigIntType()) {
    return op;
  }

  return reduceAsNumberLike(asNumeric);
}

Value *reduceAsInt32(AsInt32Inst *asInt32) {
  IRBuilder builder(asInt32->getParent()->getParent());
  auto *op = asInt32->getSingleOperand();

  // If operand is a literal, try to evaluate ToNumber(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToInt32(builder, lit)) {
      return result;
    }
  }

#ifdef HERMES_RUN_WASM
  if (op->getType().isInt32Type()) {
    return op;
  }
#endif

  // Nothing can be done to simplify, return it as-is.
  return asInt32;
}

/// Attempt to simplify \p unary and return a typed instruction if possible.
/// \return the new instruction, nullptr if it can't be simplified.
Instruction *simplifyTypedUnaryExpression(
    IRBuilder &builder,
    UnaryOperatorInst *unary) {
  auto kind = unary->getKind();
  auto *op = unary->getSingleOperand();
  Type t = op->getType();

  if (t.isNumberType()) {
    builder.setInsertionPoint(unary);
    switch (kind) {
      case ValueKind::UnaryMinusInstKind:
        return builder.createFUnaryMathInst(ValueKind::FNegateKind, op);
      case ValueKind::UnaryIncInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FAddInstKind, op, builder.getLiteralNumber(1));
      case ValueKind::UnaryDecInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FSubtractInstKind, op, builder.getLiteralNumber(1));
      default:
        break;
    }
  }

  return nullptr;
}

Value *simplifyUnOp(UnaryOperatorInst *unary) {
  IRBuilder builder(unary->getParent()->getParent());

  auto kind = unary->getKind();
  auto *op = unary->getSingleOperand();
  Type t = op->getType();

  // If the operand is a literal, try to evaluate the expression.
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto result = evalUnaryOperator(kind, builder, lit)) {
      return result;
    }
  }

  // Try to simplify based on type information.
  switch (kind) {
    case ValueKind::UnaryTypeofInstKind:
      if (t.isNullType()) {
        return builder.getLiteralString("object");
      }
      if (t.isNumberType()) {
        return builder.getLiteralString("number");
      }
      if (t.isUndefinedType()) {
        return builder.getLiteralString("undefined");
      }
      if (t.isBooleanType()) {
        return builder.getLiteralString("boolean");
      }
      if (t.isStringType()) {
        return builder.getLiteralString("string");
      }
      if (t.isObjectType()) {
        // Object type also includes closures.
        // Can't know whether this is supposed to be "function" or "object".
        break;
      }
      break;

    case ValueKind::UnaryBangInstKind:
      if (op->getType().isSubsetOf(kNullOrUndef)) {
        return builder.getLiteralBool(true);
      }
      break;

    default:
      break;
  }

  if (Instruction *typed = simplifyTypedUnaryExpression(builder, unary))
    return typed;

  return nullptr;
}

/// Attempt to simplify \p binary and return a typed instruction if possible.
/// \return the new instruction, nullptr if it can't be simplified.
Instruction *simplifyTypedBinaryExpression(
    IRBuilder &builder,
    BinaryOperatorInst *binary) {
  Value *lhs = binary->getLeftHandSide();
  Value *rhs = binary->getRightHandSide();
  auto kind = binary->getKind();

  bool bothNumber =
      lhs->getType().isNumberType() && rhs->getType().isNumberType();

  if (bothNumber) {
    builder.setInsertionPoint(binary);
    switch (kind) {
      // Double math.
      case ValueKind::BinaryAddInstKind:
        return builder.createFBinaryMathInst(ValueKind::FAddInstKind, lhs, rhs);
      case ValueKind::BinarySubtractInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FSubtractInstKind, lhs, rhs);
      case ValueKind::BinaryMultiplyInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FMultiplyInstKind, lhs, rhs);
      case ValueKind::BinaryDivideInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FDivideInstKind, lhs, rhs);
      case ValueKind::BinaryModuloInstKind:
        return builder.createFBinaryMathInst(
            ValueKind::FModuloInstKind, lhs, rhs);

      // Double comparisons.
      case ValueKind::BinaryEqualInstKind:
      case ValueKind::BinaryStrictlyEqualInstKind:
        return builder.createFCompareInst(ValueKind::FEqualInstKind, lhs, rhs);
      case ValueKind::BinaryNotEqualInstKind:
      case ValueKind::BinaryStrictlyNotEqualInstKind:
        return builder.createFCompareInst(
            ValueKind::FNotEqualInstKind, lhs, rhs);
      case ValueKind::BinaryLessThanInstKind:
        return builder.createFCompareInst(
            ValueKind::FLessThanInstKind, lhs, rhs);
      case ValueKind::BinaryLessThanOrEqualInstKind:
        return builder.createFCompareInst(
            ValueKind::FLessThanOrEqualInstKind, lhs, rhs);
      case ValueKind::BinaryGreaterThanInstKind:
        return builder.createFCompareInst(
            ValueKind::FGreaterThanInstKind, lhs, rhs);
      case ValueKind::BinaryGreaterThanOrEqualInstKind:
        return builder.createFCompareInst(
            ValueKind::FGreaterThanOrEqualInstKind, lhs, rhs);

      default:
        break;
    }
  }

  return nullptr;
}

Value *simplifyBinOp(BinaryOperatorInst *binary) {
  IRBuilder builder(binary->getParent()->getParent());

  Value *lhs = binary->getLeftHandSide();
  Value *rhs = binary->getRightHandSide();
  auto kind = binary->getKind();

  // Try simplifying without replacing the instruction.
  auto *litLhs = llvh::dyn_cast<Literal>(lhs);
  auto *litRhs = llvh::dyn_cast<Literal>(rhs);
  if (litLhs && litRhs) {
    if (auto result = evalBinaryOperator(kind, builder, litLhs, litRhs)) {
      return result;
    }
  }

  // In some cases, the instruction can be replaced with a faster version
  // or type information can be used to simplify some non-constant expressions.
  Type leftTy = lhs->getType();
  Type rightTy = rhs->getType();
  bool safeTypes = isSideEffectFree(leftTy) && isSideEffectFree(rightTy);

  // Verify that the operands are identical and can be replaced. NaN is never
  // identical to itself, and comparisons with side effects can't be replaced.
  bool identicalOperands = safeTypes && lhs == rhs && !canBeNaN(lhs);

  switch (kind) {
    case ValueKind::BinaryEqualInstKind: // ==
      // Identical operands must be equal.
      if (identicalOperands) {
        return builder.getLiteralBool(true);
      }

      // Promote equality to strict equality if we know that the types are
      // identical primitive types.
      if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
        builder.setInsertionPoint(binary);
        return builder.createBinaryOperatorInst(
            lhs, rhs, ValueKind::BinaryStrictlyEqualInstKind);
      }
      break;

    case ValueKind::BinaryNotEqualInstKind: // !=
      // Identical operands can't be non-equal.
      if (identicalOperands) {
        return builder.getLiteralBool(false);
      }

      // Promote inequality to strict inequality if we know that the types are
      // identical primitive types.
      if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
        builder.setInsertionPoint(binary);
        return builder.createBinaryOperatorInst(
            lhs, rhs, ValueKind::BinaryStrictlyNotEqualInstKind);
      }
      break;

    case ValueKind::BinaryStrictlyEqualInstKind: // ===
      // Identical operand must be strictly equal.
      if (identicalOperands) {
        return builder.getLiteralBool(true);
      }

      // Operands of different types can't be strictly equal.
      if (leftTy.isPrimitive() && rightTy.isPrimitive()) {
        if (Type::intersectTy(leftTy, rightTy).isNoType()) {
          return builder.getLiteralBool(false);
        }
      }
      break;

    case ValueKind::BinaryStrictlyNotEqualInstKind: // !===
      // Identical operands can't be non-equal.
      if (identicalOperands) {
        return builder.getLiteralBool(false);
      }
      break;

    case ValueKind::BinaryLessThanInstKind: // <
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(false);
      }
      break;

    case ValueKind::BinaryLessThanOrEqualInstKind: // <=
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(true);
      }
      break;

    case ValueKind::BinaryGreaterThanInstKind: // >
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(false);
      }
      break;

    case ValueKind::BinaryGreaterThanOrEqualInstKind: // >=
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(true);
      }
      break;

    case ValueKind::BinaryAddInstKind:
      // Convert ("" + x) or (x + "") as AsString(x).
      if (llvh::isa<LiteralString>(lhs) &&
          cast<LiteralString>(lhs)->getValue().str() == "") {
        builder.setInsertionPoint(binary);
        return builder.createAddEmptyStringInst(rhs);
      } else if (
          llvh::isa<LiteralString>(rhs) &&
          cast<LiteralString>(rhs)->getValue().str() == "") {
        builder.setInsertionPoint(binary);
        return builder.createAddEmptyStringInst(lhs);
      }
      break;

    case ValueKind::BinaryOrInstKind: { // |
      // Convert (x | 0) of (0 | x) to AsInt32.
      Value *nonzeroOp = nullptr;
      if (llvh::isa<LiteralNumber>(lhs) &&
          cast<LiteralNumber>(lhs)->getValue() == 0) {
        nonzeroOp = rhs;
      } else if (
          llvh::isa<LiteralNumber>(rhs) &&
          cast<LiteralNumber>(rhs)->getValue() == 0) {
        nonzeroOp = lhs;
      }
      if (nonzeroOp) {
        builder.setInsertionPoint(binary);
        return reduceAsInt32(builder.createAsInt32Inst(nonzeroOp));
      }
      break;
    }

    default:
      break;
  }

  if (Instruction *typed = simplifyTypedBinaryExpression(builder, binary))
    return typed;

  return nullptr;
}

Value *simplifyPhiInst(PhiInst *P) {
  // Optimize PHI nodes where all incoming values that are not self-edges are
  // the same, by replacing them with that single source value. Note that Phis
  // that have no inputs, or where all inputs are self-edges, must be dead, and
  // will be left untouched.
  Value *incoming = nullptr;
  for (int i = 0, e = P->getNumEntries(); i < e; i++) {
    auto E = P->getEntry(i);
    // Ignore self edges.
    if (E.first == P)
      continue;

    // Record the first valid input.
    if (!incoming) {
      incoming = E.first;
      continue;
    }

    // Found another unique value. Bail out.
    if (incoming != E.first)
      return nullptr;
  }

  // The PHI has a single incoming value. Replace all uses of the PHI with
  // the incoming value.
  if (incoming) {
    P->replaceAllUsesWith(incoming);
    P->eraseFromParent();
  }

  return nullptr;
}

Value *simplifyCondBranchInst(CondBranchInst *CBI) {
  auto *cond = CBI->getCondition();

  // Replace branches where the condition is negation of some expr with the
  // expr itself. Example:
  //
  // %1 = NOT %0
  // COND_BR %1, %BB1, %BB2
  //
  // into:
  //
  // %1 = NOT %0
  // COND_BR %0, %BB2, %BB1
  //
  if (auto *U = llvh::dyn_cast<UnaryOperatorInst>(cond)) {
    if (U->getSideEffect().mayReadOrWorse() &&
        U->getKind() == ValueKind::UnaryBangInstKind) {
      // Strip the negation.
      CBI->setOperand(U->getSingleOperand(), 0);
      // Swap the destination blocks:
      Value *BB1 = CBI->getOperand(1);
      Value *BB2 = CBI->getOperand(2);
      CBI->setOperand(BB1, 2);
      CBI->setOperand(BB2, 1);
      return CBI;
    }
  }
  return nullptr;
}

/// \return the one possible return value for the function \p F
/// for callsite \p callSite, or nullptr if none found.
Value *getKnownReturnValue(Function *F, CallInst *callSite) {
  IRBuilder builder(F);
  llvh::DenseSet<Value *> returnValues;

  for (auto &BB : F->getBasicBlockList()) {
    auto *term = BB.getTerminator();
    auto *ret = llvh::dyn_cast<ReturnInst>(term);
    if (!ret)
      continue;

    returnValues.insert(ret->getValue());
  }

  // Found too many return values, or not enough. Notice that
  // in some cases the function may not return anything and just
  // throw and stay in an infinite loop, and this code is legal.
  if (returnValues.size() != 1)
    return nullptr;

  Value *v = *returnValues.begin();

  // The function always returns a single literal.
  if (llvh::isa<Literal>(v))
    return v;

  // The function returns a parameter, let's inspect the call
  // site and find the value.
  if (auto *LPI = llvh::dyn_cast<LoadParamInst>(v)) {
    uint32_t idx = LPI->getParam()->getIndexInParamList();

    // Returning unpassed parameter results in undef.
    if (idx >= callSite->getNumArguments())
      return builder.getLiteralUndefined();

    // return the argument that the user passed in (ignore the
    // 'this' argument).
    return callSite->getArgument(idx);
  }

  // Returning some locally computed value.
  return nullptr;
}

Value *simplifyCallInst(CallInst *CI) {
  bool changed = false;
  if (Function *F = llvh::dyn_cast<Function>(CI->getTarget())) {
    if (CI->hasUsers()) {
      if (Value *V = getKnownReturnValue(F, CI)) {
        CI->replaceAllUsesWith(V);
        changed = true;
      }
    }
    if (!F->getNewTargetParam()->hasUsers() &&
        !llvh::isa<LiteralUndefined>(CI->getNewTarget())) {
      // The function does not use the supplied new.target param, replace it
      // with undefined. This has two advantages:
      // 1. It removes a usage of the closure, making it easier to analyze and
      // potentially eliminate in the future.
      // 2. It turns constructor calls into the same form as ordinary calls,
      // potentially allowing for better codegen. This is particularly true for
      // super() calls, where the callee and new.target are not the same.
      IRBuilder builder(CI->getParent()->getParent());
      CI->setNewTarget(builder.getLiteralUndefined());
      changed = true;
    }
  }

  return changed ? CI : nullptr;
}

Value *simplifyGetConstructedObjectInst(GetConstructedObjectInst *GCOI) {
  // If we can statically determine whether the return value of the constructor
  // will be an object, we can eliminate this instruction.
  auto opTy = GCOI->getConstructorReturnValue()->getType();
  if (opTy.isObjectType())
    return GCOI->getConstructorReturnValue();
  if (!opTy.canBeObject())
    return GCOI->getThisValue();
  return nullptr;
}

Value *simplifyAsNumber(AsNumberInst *asNumber) {
  Value *reduced = reduceAsNumber(asNumber);
  return reduced == asNumber ? nullptr : reduced;
}

bool isUnaryIncOrDec(Value *value) {
  if (auto *unOp = llvh::dyn_cast<UnaryOperatorInst>(value)) {
    switch (unOp->getKind()) {
      default:
        break;
      case ValueKind::UnaryIncInstKind:
      case ValueKind::UnaryDecInstKind:
        return true;
    }
  }

  return false;
}

Value *simplifyAsNumeric(AsNumericInst *asNumeric) {
  // reduced == asNumeric means that it wasn't possible to optimize asNumeric.
  Value *reduced = asNumeric;

  if (asNumeric->hasOneUser()) {
    if (isUnaryIncOrDec(*asNumeric->users_begin())) {
      // Single-use ToNumeric that feeds Inc/Dec can be optimized out as Inc/Dec
      // will perform ToNumeric internally.
      reduced = asNumeric->getSingleOperand();
    }
  }

  if (reduced == asNumeric) {
    // asNumeric can be optimized out if its (single) argument is provably
    // numeric.
    reduced = reduceAsNumeric(asNumeric);
  }

  return reduced == asNumeric ? nullptr : reduced;
}

Value *simplifyAsInt32(AsInt32Inst *asInt32) {
  Value *reduced = reduceAsInt32(asInt32);
  return (reduced == asInt32) ? nullptr : reduced;
}

Value *simplifyAddEmptyString(AddEmptyStringInst *AES) {
  IRBuilder builder(AES->getParent()->getParent());
  auto *op = AES->getSingleOperand();

  // If operand is a literal, try to evaluate ToString(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToString(builder, lit)) {
      return result;
    }
  }

  // No need to convert if the value is a string already.
  if (op->getType().isStringType()) {
    return op;
  }

  return nullptr;
}

Value *simplifyCoerceThisNS(CoerceThisNSInst *coerce) {
  auto *operand = coerce->getSingleOperand();

  // null or undefined produce global scope.
  if (operand->getType().isSubsetOf(kNullOrUndef)) {
    IRBuilder builder(coerce->getParent()->getParent());
    builder.setInsertionPoint(coerce);
    return builder.getGlobalObject();
  }

  // Objects are unchanged.
  if (operand->getType().isObjectType())
    return operand;

  return nullptr;
}

/// Try to simplify ThrowIfEmptyInst
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyThrowIfEmpty(ThrowIfEmptyInst *TIE) {
  // If the operand does not contain the "poison" type, it can be safely
  // eliminated.
  if (!TIE->getCheckedValue()->getType().canBeEmpty())
    return TIE->getCheckedValue();
  return nullptr;
}

/// Try to simplify FUnaryMath
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyFUnaryMath(FUnaryMathInst *inst) {
  IRBuilder builder(inst->getFunction());

  // If the arg is a literal, try to evaluate the expression.
  if (auto *lit = llvh::dyn_cast<LiteralNumber>(inst->getArg())) {
    switch (inst->getKind()) {
      case ValueKind::FNegateKind:
        return builder.getLiteralNumber(-lit->getValue());
      default:
        break;
    }
  }

  return nullptr;
}
/// Try to simplify FBinaryMath
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyFBinaryMath(FBinaryMathInst *inst) {
  IRBuilder builder(inst->getFunction());

  // If the arg is a literal, try to evaluate the expression.
  if (auto *l = llvh::dyn_cast<LiteralNumber>(inst->getLeft())) {
    if (auto *r = llvh::dyn_cast<LiteralNumber>(inst->getRight())) {
      switch (inst->getKind()) {
        case ValueKind::FAddInstKind:
          return builder.getLiteralNumber(l->getValue() + r->getValue());
        case ValueKind::FSubtractInstKind:
          return builder.getLiteralNumber(l->getValue() - r->getValue());
        case ValueKind::FMultiplyInstKind:
          return builder.getLiteralNumber(l->getValue() * r->getValue());
        case ValueKind::FDivideInstKind:
          return builder.getLiteralNumber(l->getValue() / r->getValue());
        case ValueKind::FModuloInstKind:
          return builder.getLiteralNumber(
              std::fmod(l->getValue(), r->getValue()));
        default:
          break;
      }
    }
  }

  return nullptr;
}

/// Try to simplify FCompare
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyFCompare(FCompareInst *inst) {
  IRBuilder builder(inst->getFunction());

  // If the arg is a literal, try to evaluate the expression.
  if (auto *l = llvh::dyn_cast<LiteralNumber>(inst->getLeft())) {
    if (auto *r = llvh::dyn_cast<LiteralNumber>(inst->getRight())) {
      switch (inst->getKind()) {
        case ValueKind::FEqualInstKind:
          return builder.getLiteralBool(l->getValue() == r->getValue());
        case ValueKind::FNotEqualInstKind:
          return builder.getLiteralBool(l->getValue() != r->getValue());
        case ValueKind::FLessThanInstKind:
          return builder.getLiteralBool(l->getValue() < r->getValue());
        case ValueKind::FLessThanOrEqualInstKind:
          return builder.getLiteralBool(l->getValue() <= r->getValue());
        case ValueKind::FGreaterThanInstKind:
          return builder.getLiteralBool(l->getValue() > r->getValue());
        case ValueKind::FGreaterThanOrEqualInstKind:
          return builder.getLiteralBool(l->getValue() >= r->getValue());
        default:
          break;
      }
    }
  }

  return nullptr;
}

/// Try to simplify UnionNarrowTrustedInst
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - a new value to replace the original one
OptValue<Value *> simplifyUnionNarrowTrusted(UnionNarrowTrustedInst *UNT) {
  if (UNT->getSingleOperand()->getType().isSubsetOf(UNT->getType()))
    return UNT->getSingleOperand();
  return nullptr;
}

/// Try to simplify the instruction \p I.
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyInstruction(Instruction *I) {
  // Dispatch the different simplification kinds:
  if (llvh::isa<UnaryOperatorInst>(I))
    return simplifyUnOp(llvh::cast<UnaryOperatorInst>(I));
  if (llvh::isa<BinaryOperatorInst>(I))
    return simplifyBinOp(llvh::cast<BinaryOperatorInst>(I));
  if (llvh::isa<FUnaryMathInst>(I))
    return simplifyFUnaryMath(llvh::cast<FUnaryMathInst>(I));
  if (llvh::isa<FBinaryMathInst>(I))
    return simplifyFBinaryMath(llvh::cast<FBinaryMathInst>(I));
  if (llvh::isa<FCompareInst>(I))
    return simplifyFCompare(llvh::cast<FCompareInst>(I));
  switch (I->getKind()) {
    case ValueKind::AsNumberInstKind:
      return simplifyAsNumber(cast<AsNumberInst>(I));
    case ValueKind::AsNumericInstKind:
      return simplifyAsNumeric(cast<AsNumericInst>(I));
    case ValueKind::AsInt32InstKind:
      return simplifyAsInt32(cast<AsInt32Inst>(I));
    case ValueKind::AddEmptyStringInstKind:
      return simplifyAddEmptyString(cast<AddEmptyStringInst>(I));
    case ValueKind::PhiInstKind:
      return simplifyPhiInst(cast<PhiInst>(I));
    case ValueKind::CondBranchInstKind:
      return simplifyCondBranchInst(cast<CondBranchInst>(I));
    case ValueKind::CallInstKind:
      return simplifyCallInst(cast<CallInst>(I));
    case ValueKind::GetConstructedObjectInstKind:
      return simplifyGetConstructedObjectInst(
          cast<GetConstructedObjectInst>(I));
    case ValueKind::CoerceThisNSInstKind:
      return simplifyCoerceThisNS(cast<CoerceThisNSInst>(I));
    case ValueKind::ThrowIfEmptyInstKind:
      return simplifyThrowIfEmpty(cast<ThrowIfEmptyInst>(I));
    case ValueKind::UnionNarrowTrustedInstKind:
      return simplifyUnionNarrowTrusted(cast<UnionNarrowTrustedInst>(I));

    default:
      // TODO: handle other kinds of instructions.
      return nullptr;
  }
}

} // namespace

bool InstSimplify::runOnFunction(Function *F) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;

  // For all reachable blocks in the function, in RPO order.
  PostOrderAnalysis PO(F);
  for (BasicBlock *BB : llvh::reverse(PO)) {
    // For all instructions:
    for (auto instIter = BB->begin(), e = BB->end(); instIter != e;) {
      Instruction *II = &*instIter;
      ++instIter;

      auto optNewVal = simplifyInstruction(II);
      if (optNewVal.hasValue()) {
        auto newVal = optNewVal.getValue();
        if (!newVal)
          continue;

        changed = true;
        NumSimp++;

        // Instruction changed inplace.
        if (II == newVal)
          continue;

        // We have a better and simpler instruction. Replace the original
        // instruction and mark it for deletion.
        II->replaceAllUsesWith(newVal);
      } else {
        changed = true;
        NumSimp++;
      }
      destroyer.add(II);
    }
  }

  return changed;
}

Pass *hermes::createInstSimplify() {
  return new InstSimplify();
}

#undef DEBUG_TYPE
