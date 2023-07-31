/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IREval.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Support/Math.h"

#include "llvh/ADT/SmallString.h"

using namespace hermes;
using llvh::SmallString;

namespace {

/// \returns true when the types \p A and \p B prove that the instances can't
/// be strictly equal.
bool disjointComparisonTypes(Type A, Type B) {
  if (!A.isPrimitive() || !B.isPrimitive())
    return false;

  // Check if types are disjoint.
  return Type::intersectTy(A, B).isNoType();
}

bool isNaN(Literal *lit) {
  if (auto *number = llvh::dyn_cast<LiteralNumber>(lit)) {
    return std::isnan(number->getValue());
  }
  return false;
}

enum class NumericOrder { LessThan, Equal, GreaterThan, Unordered };

/// \returns the numeric ordering of two values.
llvh::Optional<NumericOrder> getNumericOrder(
    LiteralNumber *L,
    LiteralNumber *R) {
  if (!L || !R)
    return llvh::None;

  double l = L->getValue();
  double r = R->getValue();

  if (l < r)
    return NumericOrder::LessThan;
  if (l > r)
    return NumericOrder::GreaterThan;
  if (std::isnan(l) || std::isnan(r))
    return NumericOrder::Unordered;
  return NumericOrder::Equal;
}

SmallString<256> buildString(
    const llvh::StringRef &a,
    const llvh::StringRef &b) {
  SmallString<256> result;
  result.append(a);
  result.append(b);
  return result;
}

} // namespace

Literal *hermes::evalUnaryOperator(
    UnaryOperatorInst::OpKind kind,
    IRBuilder &builder,
    Literal *operand) {
  switch (kind) {
    case UnaryOperatorInst::OpKind::PlusKind:
      return evalToNumber(builder, operand);
    case UnaryOperatorInst::OpKind::MinusKind:
      return negateNumber(builder, evalToNumber(builder, operand));
      break;
    case UnaryOperatorInst::OpKind::TypeofKind:
      switch (operand->getKind()) {
        case ValueKind::GlobalObjectKind:
        case ValueKind::LiteralNullKind:
          return builder.getLiteralString("object");
        case ValueKind::LiteralUndefinedKind:
          return builder.getLiteralString("undefined");
        case ValueKind::LiteralBoolKind:
          return builder.getLiteralString("boolean");
        case ValueKind::LiteralNumberKind:
          return builder.getLiteralString("number");
        case ValueKind::LiteralStringKind:
          return builder.getLiteralString("string");
        default:
          break;
      }
      break;

    case UnaryOperatorInst::OpKind::BangKind:
      if (evalIsTrue(builder, operand)) {
        return builder.getLiteralBool(false);
      }
      if (evalIsFalse(builder, operand)) {
        return builder.getLiteralBool(true);
      }
      break;

    case UnaryOperatorInst::OpKind::VoidKind:
      return builder.getLiteralUndefined();

    case UnaryOperatorInst::OpKind::IncKind:
      if (auto *V = evalToNumber(builder, operand)) {
        return builder.getLiteralNumber(V->getValue() + 1);
      }
      break;

    case UnaryOperatorInst::OpKind::DecKind:
      if (auto *V = evalToNumber(builder, operand)) {
        return builder.getLiteralNumber(V->getValue() - 1);
      }
      break;

    default:
      break;
  }

  return nullptr;
}

Literal *hermes::evalBinaryOperator(
    BinaryOperatorInst::OpKind kind,
    IRBuilder &builder,
    Literal *lhs,
    Literal *rhs) LLVM_NO_SANITIZE("float-divide-by-zero");

Literal *hermes::evalBinaryOperator(
    BinaryOperatorInst::OpKind kind,
    IRBuilder &builder,
    Literal *lhs,
    Literal *rhs) {
  Type leftTy = lhs->getType();
  Type rightTy = rhs->getType();

  auto *leftNull = llvh::dyn_cast<LiteralNull>(lhs);
  auto *rightNull = llvh::dyn_cast<LiteralNull>(rhs);

  auto *leftUndef = llvh::dyn_cast<LiteralUndefined>(lhs);
  auto *rightUndef = llvh::dyn_cast<LiteralUndefined>(rhs);

  auto *leftStr = llvh::dyn_cast<LiteralString>(lhs);
  auto *rightStr = llvh::dyn_cast<LiteralString>(rhs);

  auto leftNaN = isNaN(lhs);
  auto rightNaN = isNaN(rhs);

  auto leftIsNullOrUndef = leftNull || leftUndef;
  auto rightIsNullOrUndef = rightNull || rightUndef;

  auto &ctx = builder.getModule()->getContext();

  using OpKind = BinaryOperatorInst::OpKind;

  if (leftNaN || rightNaN) {
    switch (kind) {
      case OpKind::EqualKind:
      case OpKind::StrictlyEqualKind:
      case OpKind::LessThanKind:
      case OpKind::LessThanOrEqualKind:
      case OpKind::GreaterThanKind:
      case OpKind::GreaterThanOrEqualKind:
        // Equality and order comparisons with NaN always evaluate to false,
        // even in cases like 'NaN == NaN' or 'NaN <= NaN'
        return builder.getLiteralBool(false);
      case OpKind::NotEqualKind:
      case OpKind::StrictlyNotEqualKind:
        // Inequality comparisons with NaN always evaluate to true, even in
        // cases like 'NaN != NaN' or 'NaN !== NaN'
        return builder.getLiteralBool(true);
      default:
        // The other binary operators will handle NaN as part of their IEEE
        // spec compliance.
        break;
    }
  }

  // Evaluate both operands to literal numbers if they can be.
  auto *lNumOrCoercedPrimitive = evalToNumber(builder, lhs);
  auto *rNumOrCoercedPrimitive = evalToNumber(builder, rhs);

  auto numericOrder =
      getNumericOrder(lNumOrCoercedPrimitive, rNumOrCoercedPrimitive);

  switch (kind) {
    case OpKind::EqualKind: // ==
      // Identical literals must be equal.
      if (lhs == rhs) {
        return builder.getLiteralBool(true);
      }

      // `null` and `undefined` are only equal to themselves.
      if (leftIsNullOrUndef || rightIsNullOrUndef) {
        return builder.getLiteralBool(leftIsNullOrUndef && rightIsNullOrUndef);
      }

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Equal:
            return builder.getLiteralBool(true);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Unordered:
            break;
        }
      }

      // Handle string equality:
      if (leftStr && rightStr) {
        return builder.getLiteralBool(
            leftStr->getValue() == rightStr->getValue());
      }

      break;
    case OpKind::NotEqualKind: // !=
      // Identical operands can't be non-equal.
      if (lhs == rhs) {
        return builder.getLiteralBool(false);
      }

      // `null` and `undefined` are only equal to themselves.
      if (leftIsNullOrUndef || rightIsNullOrUndef) {
        return builder.getLiteralBool(
            !(leftIsNullOrUndef && rightIsNullOrUndef));
      }

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Equal:
            return builder.getLiteralBool(false);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Unordered:
            break;
        }
      }
      // Handle string equality:
      if (leftStr && rightStr) {
        return builder.getLiteralBool(
            leftStr->getValue() != rightStr->getValue());
      }

      break;
    case OpKind::StrictlyEqualKind: // ===
      // Identical literals must be equal.
      if (lhs == rhs) {
        return builder.getLiteralBool(true);
      }

      // Operands of different types can't be strictly equal.
      if (disjointComparisonTypes(leftTy, rightTy))
        return builder.getLiteralBool(false);

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Equal:
            return builder.getLiteralBool(true);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Unordered:
            break;
        }
      }

      // Handle string equality:
      if (leftStr && rightStr) {
        return builder.getLiteralBool(
            leftStr->getValue() == rightStr->getValue());
      }

      break;
    case OpKind::StrictlyNotEqualKind: // !===
      // Identical operands can't be non-equal.
      if (lhs == rhs) {
        return builder.getLiteralBool(false);
      }

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Equal:
            return builder.getLiteralBool(false);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Unordered:
            break;
        }
      }

      // Handle string equality:
      if (leftStr && rightStr) {
        return builder.getLiteralBool(
            leftStr->getValue() != rightStr->getValue());
      }

      break;
    case OpKind::LessThanKind: // <
      // Handle comparison to self:
      if (!leftTy.isUndefinedType() && lhs == rhs)
        return builder.getLiteralBool(false);

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Equal:
            return builder.getLiteralBool(false);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Unordered:
            break;
        }
      }
      break;
    case OpKind::LessThanOrEqualKind: // <=
      // Handle comparison to self:
      if (!leftTy.isUndefinedType() && lhs == rhs)
        return builder.getLiteralBool(true);

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Equal:
            return builder.getLiteralBool(true);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Unordered:
            break;
        }
      }

      break;
    case OpKind::GreaterThanKind: // >
      // Handle comparison to self:
      if (!leftTy.isUndefinedType() && lhs == rhs)
        return builder.getLiteralBool(false);

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Equal:
            return builder.getLiteralBool(false);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Unordered:
            break;
        }
      }

      break;
    case OpKind::GreaterThanOrEqualKind: // >=
      // Handle comparison to self:
      if (!leftTy.isUndefinedType() && lhs == rhs)
        return builder.getLiteralBool(true);

      // Handle numeric comparisons:
      if (numericOrder.hasValue()) {
        switch (numericOrder.getValue()) {
          case NumericOrder::LessThan:
            return builder.getLiteralBool(false);
          case NumericOrder::Equal:
            return builder.getLiteralBool(true);
          case NumericOrder::GreaterThan:
            return builder.getLiteralBool(true);
          case NumericOrder::Unordered:
            break;
        }
      }

      break;
    case OpKind::LeftShiftKind: // <<  (<<=)
    case OpKind::RightShiftKind: // >>  (>>=)
    case OpKind::UnsignedRightShiftKind: { // >>> (>>>=)
      if (!lNumOrCoercedPrimitive || !rNumOrCoercedPrimitive) {
        // Can't be converted to a literal number.
        break;
      }
      uint32_t shiftCount = rNumOrCoercedPrimitive->truncateToUInt32() & 0x1f;
      // Large enough to hold both int32_t and uint32_t values.
      int64_t result = 0;
      if (kind == OpKind::LeftShiftKind) {
        // Truncate to unsigned so that the shift doesn't happen on negative
        // values. Cast it to a 32-bit signed int to get the sign back, then
        // promote to 64 bits.
        result = static_cast<int32_t>(
            lNumOrCoercedPrimitive->truncateToUInt32() << shiftCount);
      } else if (kind == OpKind::RightShiftKind) {
        result = static_cast<int64_t>(
            lNumOrCoercedPrimitive->truncateToInt32() >> shiftCount);
      } else {
        result = static_cast<int64_t>(
            lNumOrCoercedPrimitive->truncateToUInt32() >> shiftCount);
      }
      return builder.getLiteralNumber(result);
    }
    case OpKind::AddKind: { // +   (+=)
      // If either literal is a string, we must coerce the
      // other literal to a string and concatenate.
      if (leftStr || rightStr) {
        auto leftEval = evalToString(builder, lhs);
        auto rightEval = evalToString(builder, rhs);
        if (leftEval && rightEval) {
          SmallString<256> result = buildString(
              ctx.toString(leftEval->getValue()),
              ctx.toString(rightEval->getValue()));
          return builder.getLiteralString(result.str());
        }
        break;
      }

      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->getValue() +
            rNumOrCoercedPrimitive->getValue());
      }

      break;
    }
    case OpKind::SubtractKind: // -   (-=)
      // Handle numeric constants:
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->getValue() -
            rNumOrCoercedPrimitive->getValue());
      }

      break;

    case OpKind::MultiplyKind: // *   (*=)
      // Handle numeric constants:
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->getValue() *
            rNumOrCoercedPrimitive->getValue());
      }

      break;
    case OpKind::DivideKind: // /   (/=)
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        // This relies on IEEE 754 double division. All modern compilers
        // implement this.
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->getValue() /
            rNumOrCoercedPrimitive->getValue());
      }
      break;
    case OpKind::ModuloKind: // %   (%=)
      // Note that fmod differs slightly from the ES spec with regards to how
      // numbers not representable by double are rounded. This difference can be
      // ignored in practice, so most JS VMs use fmod.
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(std::fmod(
            lNumOrCoercedPrimitive->getValue(),
            rNumOrCoercedPrimitive->getValue()));
      }

      break;
    case OpKind::ExponentiationKind: // ** (**=)
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(hermes::expOp(
            lNumOrCoercedPrimitive->getValue(),
            rNumOrCoercedPrimitive->getValue()));
      }
      break;
    case OpKind::OrKind: // |   (|=)
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->truncateToInt32() |
            rNumOrCoercedPrimitive->truncateToInt32());
      }

      break;
    case OpKind::XorKind: // ^   (^=)
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->truncateToInt32() ^
            rNumOrCoercedPrimitive->truncateToInt32());
      }

      break;
    case OpKind::AndKind: // &   (&=)
      if (lNumOrCoercedPrimitive && rNumOrCoercedPrimitive) {
        return builder.getLiteralNumber(
            lNumOrCoercedPrimitive->truncateToInt32() &
            rNumOrCoercedPrimitive->truncateToInt32());
      }

      break;
    default:
      break;
  }

  return nullptr;
}

LiteralBool *hermes::evalToBoolean(IRBuilder &builder, Literal *operand) {
  bool value;
  switch (operand->getKind()) {
    case ValueKind::LiteralNullKind:
    case ValueKind::LiteralUndefinedKind:
      value = false;
      break;
    case ValueKind::LiteralBoolKind:
      value = cast<LiteralBool>(operand)->getValue();
      break;
    case ValueKind::LiteralNumberKind: {
      const auto n = cast<LiteralNumber>(operand)->getValue();
      value = !std::isnan(n) && n != 0.0;
      break;
    }
    case ValueKind::LiteralStringKind:
      value = !cast<LiteralString>(operand)->getValue().str().empty();
      break;
    default:
      return nullptr;
  }

  return builder.getLiteralBool(value);
}

LiteralString *hermes::evalToString(IRBuilder &builder, Literal *operand) {
  if (auto *str = llvh::dyn_cast<LiteralString>(operand))
    return str;
  if (auto *literalUndefined = llvh::dyn_cast<LiteralUndefined>(operand))
    return builder.getLiteralString("undefined");
  if (auto *literalNull = llvh::dyn_cast<LiteralNull>(operand))
    return builder.getLiteralString("null");
  if (auto *literalBool = llvh::dyn_cast<LiteralBool>(operand))
    return builder.getLiteralString(literalBool->getValue() ? "true" : "false");
  if (auto *num = llvh::dyn_cast<LiteralNumber>(operand)) {
    char buf[NUMBER_TO_STRING_BUF_SIZE];
    auto len = numberToString(num->getValue(), buf, sizeof(buf));
    return builder.getLiteralString(llvh::StringRef(buf, len));
  }
  return nullptr;
}

LiteralNumber *hermes::negateNumber(
    IRBuilder &builder,
    LiteralNumber *operand) {
  if (!operand)
    return nullptr;
  if (auto *numLiteral = llvh::dyn_cast<LiteralNumber>(operand))
    return builder.getLiteralNumber(-numLiteral->getValue());
  return nullptr;
}

LiteralNumber *hermes::evalToNumber(IRBuilder &builder, Literal *operand) {
  if (!operand) {
    return nullptr;
  }
  if (auto *numLiteral = llvh::dyn_cast<LiteralNumber>(operand)) {
    return numLiteral;
  }
  if (auto *boolLiteral = llvh::dyn_cast<LiteralBool>(operand)) {
    return builder.getLiteralNumber(boolLiteral->getValue());
  }
  if (operand->getType().isUndefinedType()) {
    return builder.getLiteralNaN();
  }
  if (operand->getType().isNullType()) {
    return builder.getLiteralPositiveZero();
  }
  return nullptr;
}

LiteralNumber *hermes::evalToInt32(IRBuilder &builder, Literal *operand) {
  // Eval to a number first, then truncate to a 32-bit int.
  LiteralNumber *lit = evalToNumber(builder, operand);
  if (!lit) {
    return nullptr;
  }
  double val = lit->getValue();
  return builder.getLiteralNumber(truncateToInt32(val));
}

LiteralBool *hermes::evalToBoolean(IRBuilder &builder, Value *operand) {
  if (auto *L = llvh::dyn_cast<Literal>(operand)) {
    return evalToBoolean(builder, L);
  }

  Type OpTY = operand->getType();
  if (OpTY.isObjectType()) {
    return builder.getLiteralBool(true);
  }
  if (OpTY.isNullType() || OpTY.isUndefinedType()) {
    return builder.getLiteralBool(false);
  }
  return nullptr;
}

bool hermes::evalIsTrue(IRBuilder &builder, Literal *operand) {
  if (auto *lit = evalToBoolean(builder, operand))
    return lit->getValue();
  return false;
}

bool hermes::evalIsFalse(IRBuilder &builder, Literal *operand) {
  if (auto *lit = evalToBoolean(builder, operand))
    return !lit->getValue();
  return false;
}
