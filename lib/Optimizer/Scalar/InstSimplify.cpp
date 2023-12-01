/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Perform simple peephole optimizations.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "instsimplify"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseSet.h"

using namespace hermes;

STATISTIC(NumSimp, "Number of instructions simplified");

namespace {

constexpr Type kNullOrUndef =
    Type::unionTy(Type::createUndefined(), Type::createNull());

/// \return true if the value is known to not be NaN. Note that it may still
///    be convertible to NaN.
bool notNaN(Value *value) {
  // Only numbers can be NaN.
  if (!value->getType().canBeNumber()) {
    return true;
  }

  // If it is a literal, we can simply check.
  if (auto *literalNumber = llvh::dyn_cast<LiteralNumber>(value)) {
    return !std::isnan(literalNumber->getValue());
  }

  // The value could be a number, so it could be NaN.
  return false;
}

class InstSimplifyImpl {
 public:
  InstSimplifyImpl(Function *F) : F_(F), builder_(F_) {}

  bool run() {
    bool changed = false;
    IRBuilder::InstructionDestroyer destroyer;

    // For all reachable blocks in the function, in RPO order.
    PostOrderAnalysis PO(F_);
    for (BasicBlock *BB : llvh::reverse(PO)) {
      // For all instructions:
      for (auto instIter = BB->begin(), e = BB->end(); instIter != e;) {
        Instruction *II = &*instIter;
        ++instIter;

        /// When optimizing instructions, the new instruction is inserted after
        /// the instruction to be optimized. This allows the main loop over the
        /// instructions to pick up the new instruction and potentially optimize
        /// it further.
        builder_.setInsertionPointAfter(II);

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

 private:
  template <typename T>
  Value *reduceAsNumberLike(T *asNumber) {
    auto *op = asNumber->getSingleOperand();

    // If operand is a literal, try to evaluate ToNumber(operand).
    if (auto *lit = llvh::dyn_cast<Literal>(op)) {
      if (auto *result = evalToNumber(builder_, lit)) {
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
    auto *op = asInt32->getSingleOperand();

    // If operand is a literal, try to evaluate ToNumber(operand).
    if (auto *lit = llvh::dyn_cast<Literal>(op)) {
      if (auto *result = evalToInt32(builder_, lit)) {
        return result;
      }
    }

    // Nothing can be done to simplify, return it as-is.
    return asInt32;
  }

  /// Attempt to simplify \p unary and return a typed instruction if possible.
  /// \return the new instruction, nullptr if it can't be simplified.
  Instruction *simplifyTypedUnaryExpression(UnaryOperatorInst *unary) {
    auto kind = unary->getKind();
    auto *op = unary->getSingleOperand();
    Type t = op->getType();

    if (t.isNumberType()) {
      switch (kind) {
        case ValueKind::UnaryMinusInstKind:
          return builder_.createFUnaryMathInst(ValueKind::FNegateKind, op);
        case ValueKind::UnaryIncInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FAddInstKind, op, builder_.getLiteralNumber(1));
        case ValueKind::UnaryDecInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FSubtractInstKind, op, builder_.getLiteralNumber(1));
        default:
          break;
      }
    }

    return nullptr;
  }

  Value *simplifyUnOp(UnaryOperatorInst *unary) {
    auto kind = unary->getKind();
    auto *op = unary->getSingleOperand();
    Type t = op->getType();

    // If the operand is a literal, try to evaluate the expression.
    if (auto *lit = llvh::dyn_cast<Literal>(op)) {
      if (auto result = evalUnaryOperator(kind, builder_, lit)) {
        return result;
      }
    }

    // Try to simplify based on type information.
    switch (kind) {
      case ValueKind::UnaryTypeofInstKind:
        if (t.isNullType()) {
          return builder_.getLiteralString("object");
        }
        if (t.isNumberType()) {
          return builder_.getLiteralString("number");
        }
        if (t.isUndefinedType()) {
          return builder_.getLiteralString("undefined");
        }
        if (t.isBooleanType()) {
          return builder_.getLiteralString("boolean");
        }
        if (t.isStringType()) {
          return builder_.getLiteralString("string");
        }
        if (t.isObjectType()) {
          // Object type also includes closures.
          // Can't know whether this is supposed to be "function" or "object".
          break;
        }
        break;

      case ValueKind::UnaryBangInstKind:
        if (op->getType().isSubsetOf(kNullOrUndef)) {
          return builder_.getLiteralBool(true);
        }
        break;

      default:
        break;
    }

    if (Instruction *typed = simplifyTypedUnaryExpression(unary))
      return typed;

    return nullptr;
  }

  /// Attempt to simplify \p binary and return a typed instruction if possible.
  /// \return the new instruction, nullptr if it can't be simplified.
  Instruction *simplifyTypedBinaryExpression(BinaryOperatorInst *binary) {
    Value *lhs = binary->getLeftHandSide();
    Value *rhs = binary->getRightHandSide();
    auto kind = binary->getKind();

    bool bothNumber =
        lhs->getType().isNumberType() && rhs->getType().isNumberType();

    if (bothNumber) {
      switch (kind) {
        // Double math.
        case ValueKind::BinaryAddInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FAddInstKind, lhs, rhs);
        case ValueKind::BinarySubtractInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FSubtractInstKind, lhs, rhs);
        case ValueKind::BinaryMultiplyInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FMultiplyInstKind, lhs, rhs);
        case ValueKind::BinaryDivideInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FDivideInstKind, lhs, rhs);
        case ValueKind::BinaryModuloInstKind:
          return builder_.createFBinaryMathInst(
              ValueKind::FModuloInstKind, lhs, rhs);

        // Double comparisons.
        case ValueKind::BinaryEqualInstKind:
        case ValueKind::BinaryStrictlyEqualInstKind:
          return builder_.createFCompareInst(
              ValueKind::FEqualInstKind, lhs, rhs);
        case ValueKind::BinaryNotEqualInstKind:
        case ValueKind::BinaryStrictlyNotEqualInstKind:
          return builder_.createFCompareInst(
              ValueKind::FNotEqualInstKind, lhs, rhs);
        case ValueKind::BinaryLessThanInstKind:
          return builder_.createFCompareInst(
              ValueKind::FLessThanInstKind, lhs, rhs);
        case ValueKind::BinaryLessThanOrEqualInstKind:
          return builder_.createFCompareInst(
              ValueKind::FLessThanOrEqualInstKind, lhs, rhs);
        case ValueKind::BinaryGreaterThanInstKind:
          return builder_.createFCompareInst(
              ValueKind::FGreaterThanInstKind, lhs, rhs);
        case ValueKind::BinaryGreaterThanOrEqualInstKind:
          return builder_.createFCompareInst(
              ValueKind::FGreaterThanOrEqualInstKind, lhs, rhs);

        default:
          break;
      }
    }

    return nullptr;
  }

  Value *simplifyBinOp(BinaryOperatorInst *binary) {
    Value *lhs = binary->getLeftHandSide();
    Value *rhs = binary->getRightHandSide();
    auto kind = binary->getKind();

    // Try simplifying without replacing the instruction.
    auto *litLhs = llvh::dyn_cast<Literal>(lhs);
    auto *litRhs = llvh::dyn_cast<Literal>(rhs);
    if (litLhs && litRhs) {
      if (auto result = evalBinaryOperator(kind, builder_, litLhs, litRhs)) {
        return result;
      }
    }

    // In some cases, the instruction can be replaced with a faster version
    // or type information can be used to simplify some non-constant
    // expressions.
    Type leftTy = lhs->getType();
    Type rightTy = rhs->getType();
    const bool primitiveTypes = leftTy.isPrimitive() && rightTy.isPrimitive();

    // This flag helps to simplify equality comparisons (==, ===, !=, !===).
    // Indicate that the operands are the same value which has a primitive type
    // and always compares equal to itself when the comparison does NOT perform
    // any conversions.
    // NaN is never equal to itself, and comparisons between non-primitive types
    // have side effects and can't be predicted.
    const bool identicalForEquality =
        primitiveTypes && lhs == rhs && notNaN(lhs);

    // This flag helps to simplify relational comparisons (<, <=, >, >=).
    // In additional to the equality checks, it also checks that the operands
    // will not be converted to NaN when the relational comparison invokes
    // toNumeric().
    // We know the identical operand is a primitive, and the only primitive
    // types that can be converted to NaN by toNumeric() are string and
    // undefined. However, if the operands are strings, the comparison is
    // performed before toNumeric(). So the only case that for NaN that remains
    // is "undefined".
    bool identicalForRelational =
        identicalForEquality && !leftTy.canBeUndefined();

    switch (kind) {
      case ValueKind::BinaryEqualInstKind: // ==
        // Identical operands must be equal.
        if (identicalForEquality) {
          return builder_.getLiteralBool(true);
        }

        // Promote equality to strict equality if we know that the types are
        // identical primitive types.
        if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
          return builder_.createBinaryOperatorInst(
              lhs, rhs, ValueKind::BinaryStrictlyEqualInstKind);
        }
        break;

      case ValueKind::BinaryNotEqualInstKind: // !=
        // Identical operands can't be non-equal.
        if (identicalForEquality) {
          return builder_.getLiteralBool(false);
        }

        // Promote inequality to strict inequality if we know that the types are
        // identical primitive types.
        if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
          return builder_.createBinaryOperatorInst(
              lhs, rhs, ValueKind::BinaryStrictlyNotEqualInstKind);
        }
        break;

      case ValueKind::BinaryStrictlyEqualInstKind: // ===
        // Identical operand must be strictly equal.
        if (identicalForEquality) {
          return builder_.getLiteralBool(true);
        }

        // Operands of different types can't be strictly equal.
        if (leftTy.isPrimitive() && rightTy.isPrimitive()) {
          if (Type::intersectTy(leftTy, rightTy).isNoType()) {
            return builder_.getLiteralBool(false);
          }
        }
        break;

      case ValueKind::BinaryStrictlyNotEqualInstKind: // !===
        // Identical operands can't be non-equal.
        if (identicalForEquality) {
          return builder_.getLiteralBool(false);
        }
        break;

      case ValueKind::BinaryLessThanInstKind: // <
        if (identicalForRelational) {
          return builder_.getLiteralBool(false);
        }
        break;

      case ValueKind::BinaryLessThanOrEqualInstKind: // <=
        // Handle comparison to self:
        if (identicalForRelational) {
          return builder_.getLiteralBool(true);
        }
        break;

      case ValueKind::BinaryGreaterThanInstKind: // >
        // Handle comparison to self:
        if (identicalForRelational) {
          return builder_.getLiteralBool(false);
        }
        break;

      case ValueKind::BinaryGreaterThanOrEqualInstKind: // >=
        // Handle comparison to self:
        if (identicalForRelational) {
          return builder_.getLiteralBool(true);
        }
        break;

      case ValueKind::BinaryAddInstKind:
        // Convert ("" + x) or (x + "") as AsString(x).
        if (llvh::isa<LiteralString>(lhs) &&
            cast<LiteralString>(lhs)->getValue().str() == "") {
          return builder_.createAddEmptyStringInst(rhs);
        } else if (
            llvh::isa<LiteralString>(rhs) &&
            cast<LiteralString>(rhs)->getValue().str() == "") {
          return builder_.createAddEmptyStringInst(lhs);
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
          return reduceAsInt32(builder_.createAsInt32Inst(nonzeroOp));
        }
        break;
      }

      default:
        break;
    }

    if (Instruction *typed = simplifyTypedBinaryExpression(binary))
      return typed;

    return nullptr;
  }

  Value *simplifyPhiInst(PhiInst *P) {
    // Optimize PHI nodes where all incoming values that are not self-edges are
    // the same, by replacing them with that single source value. Note that Phis
    // that have no inputs, or where all inputs are self-edges, must be dead,
    // and will be left untouched.
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
        return builder_.getLiteralUndefined();

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
        // potentially allowing for better codegen. This is particularly true
        // for super() calls, where the callee and new.target are not the same.
        CI->setNewTarget(builder_.getLiteralUndefined());
        changed = true;
      }
    }

    return changed ? CI : nullptr;
  }

  Value *simplifyGetConstructedObjectInst(GetConstructedObjectInst *GCOI) {
    // If we can statically determine whether the return value of the
    // constructor will be an object, we can eliminate this instruction.
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
        // Single-use ToNumeric that feeds Inc/Dec can be optimized out as
        // Inc/Dec will perform ToNumeric internally.
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
    auto *op = AES->getSingleOperand();

    // If operand is a literal, try to evaluate ToString(operand).
    if (auto *lit = llvh::dyn_cast<Literal>(op)) {
      if (auto *result = evalToString(builder_, lit)) {
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
      return builder_.getGlobalObject();
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
    // If the arg is a literal, try to evaluate the expression.
    if (auto *lit = llvh::dyn_cast<LiteralNumber>(inst->getArg())) {
      switch (inst->getKind()) {
        case ValueKind::FNegateKind:
          return builder_.getLiteralNumber(-lit->getValue());
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
    // If the arg is a literal, try to evaluate the expression.
    if (auto *l = llvh::dyn_cast<LiteralNumber>(inst->getLeft())) {
      if (auto *r = llvh::dyn_cast<LiteralNumber>(inst->getRight())) {
        switch (inst->getKind()) {
          case ValueKind::FAddInstKind:
            return builder_.getLiteralNumber(l->getValue() + r->getValue());
          case ValueKind::FSubtractInstKind:
            return builder_.getLiteralNumber(l->getValue() - r->getValue());
          case ValueKind::FMultiplyInstKind:
            return builder_.getLiteralNumber(l->getValue() * r->getValue());
          case ValueKind::FDivideInstKind:
            return builder_.getLiteralNumber(l->getValue() / r->getValue());
          case ValueKind::FModuloInstKind:
            return builder_.getLiteralNumber(
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
    // If the arg is a literal, try to evaluate the expression.
    if (auto *l = llvh::dyn_cast<LiteralNumber>(inst->getLeft())) {
      if (auto *r = llvh::dyn_cast<LiteralNumber>(inst->getRight())) {
        switch (inst->getKind()) {
          case ValueKind::FEqualInstKind:
            return builder_.getLiteralBool(l->getValue() == r->getValue());
          case ValueKind::FNotEqualInstKind:
            return builder_.getLiteralBool(l->getValue() != r->getValue());
          case ValueKind::FLessThanInstKind:
            return builder_.getLiteralBool(l->getValue() < r->getValue());
          case ValueKind::FLessThanOrEqualInstKind:
            return builder_.getLiteralBool(l->getValue() <= r->getValue());
          case ValueKind::FGreaterThanInstKind:
            return builder_.getLiteralBool(l->getValue() > r->getValue());
          case ValueKind::FGreaterThanOrEqualInstKind:
            return builder_.getLiteralBool(l->getValue() >= r->getValue());
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
  /// \returns one of:
  ///   - nullptr if the instruction cannot be simplified.
  ///   - a new value to replace the original one
  OptValue<Value *> simplifyCheckedTypeCast(CheckedTypeCastInst *ctc) {
    Type resType =
        Type::intersectTy(ctc->getType(), ctc->getSingleOperand()->getType());
    // This is a cast that always fails. Do nothing.
    if (resType.isNoType())
      return nullptr;

    // A widening checked cast is pointless.
    if (ctc->getSingleOperand()->getType().isSubsetOf(ctc->getType()))
      return ctc->getSingleOperand();

    // If the result type is wider than necessary, narrow it.
    if (ctc->getType() != resType)
      ctc->setType(resType);

    // Casting a cast is pointless. Use the first cast's operand as operand.
    if (auto *inputCast =
            llvh::dyn_cast<CheckedTypeCastInst>(ctc->getSingleOperand())) {
      ctc->setOperand(inputCast->getSingleOperand(), 0);
    }
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
      case ValueKind::CheckedTypeCastInstKind:
        return simplifyCheckedTypeCast(cast<CheckedTypeCastInst>(I));

      default:
        // TODO: handle other kinds of instructions.
        return nullptr;
    }
  }

 private:
  /// The function being optimized.
  Function *F_;

  /// Shared builder used for creating the optimized instructions.
  IRBuilder builder_;
};

} // namespace

Pass *hermes::createInstSimplify() {
  class InstSimplify : public FunctionPass {
   public:
    explicit InstSimplify() : FunctionPass("InstSimplify") {}
    ~InstSimplify() override = default;

    bool runOnFunction(Function *F) override {
      return InstSimplifyImpl(F).run();
    }
  };
  return new InstSimplify();
}

#undef DEBUG_TYPE
