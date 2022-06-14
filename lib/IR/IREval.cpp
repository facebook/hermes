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
llvh::Optional<NumericOrder> getNumericOrder(Literal *LHS, Literal *RHS) {
  auto *L = llvh::dyn_cast<LiteralNumber>(LHS);
  auto *R = llvh::dyn_cast<LiteralNumber>(RHS);

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

SmallString<256> buildString(const StringRef &a, const StringRef &b) {
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
    case UnaryOperatorInst::OpKind::MinusKind:
      // Negate constant integers.
      switch (operand->getKind()) {
        case ValueKind::LiteralNumberKind:
          if (auto *literalNum = llvh::dyn_cast<LiteralNumber>(operand)) {
            auto V = -literalNum->getValue();
            return builder.getLiteralNumber(V);
          }
          break;
        case ValueKind::LiteralUndefinedKind:
          return builder.getLiteralNaN();
        case ValueKind::LiteralBoolKind:
          if (evalIsTrue(builder, operand)) {
            return builder.getLiteralNumber(-1);
          } else { // evalIsFalse(operand)
            return builder.getLiteralNegativeZero();
          }
        case ValueKind::LiteralNullKind:
          return builder.getLiteralNegativeZero();
        default:
          break;
      }
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

  auto *leftLiteralNum = llvh::dyn_cast<LiteralNumber>(lhs);
  auto *rightLiteralNum = llvh::dyn_cast<LiteralNumber>(rhs);

  auto *leftNull = llvh::dyn_cast<LiteralNull>(lhs);
  auto *rightNull = llvh::dyn_cast<LiteralNull>(rhs);

  auto *leftUndef = llvh::dyn_cast<LiteralUndefined>(lhs);
  auto *rightUndef = llvh::dyn_cast<LiteralUndefined>(rhs);

  auto *leftStr = llvh::dyn_cast<LiteralString>(lhs);
  auto *rightStr = llvh::dyn_cast<LiteralString>(rhs);

  auto leftNaN = isNaN(lhs);
  auto rightNaN = isNaN(rhs);

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
      case OpKind::LeftShiftKind:
      case OpKind::RightShiftKind:
      case OpKind::UnsignedRightShiftKind:
        // The code for bitwise shift operators below properly handles
        // NaN, so we break out of the switch and go through to there
        break;
      case OpKind::AddKind:
        // If we're trying to add NaN with a string, then we need to perform
        // string concatenation with "NaN" and the string operand
        if (leftStr) {
          SmallString<256> result =
              buildString(ctx.toString(leftStr->getValue()), "NaN");
          return builder.getLiteralString(result.str());
        }

        if (rightStr) {
          SmallString<256> result =
              buildString("NaN", ctx.toString(rightStr->getValue()));
          return builder.getLiteralString(result.str());
        }

        // None of the operands are strings, so the expression evaluates
        // to NaN
        return builder.getLiteralNaN();
      case OpKind::SubtractKind:
      case OpKind::MultiplyKind:
      case OpKind::DivideKind:
      case OpKind::ModuloKind:
        // Binary arithmetic operations involving NaN evaluate to  NaN
        return builder.getLiteralNaN();
      case OpKind::OrKind:
      case OpKind::XorKind:
      case OpKind::AndKind:
        // The code for bitwise logical operators below properly handles
        // NaN, so we break out of the switch and go through to there
        break;
      default:
        // All handling of NaN is done is the current block. The rest of this
        // code assumes the literals are not NaN (except for bitwise shifts and
        // logical ops), so we break out of the function here, giving up on
        // finding a compile-time literal representation of the result.
        return nullptr;
    }
  }

  auto numericOrder = getNumericOrder(lhs, rhs);

  switch (kind) {
    case OpKind::EqualKind: // ==
      // Identical literals must be equal.
      if (lhs == rhs) {
        return builder.getLiteralBool(true);
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
      // Convert both operands to literal numbers if they can be.
      auto *lnum = evalToNumber(builder, lhs);
      auto *rnum = evalToNumber(builder, rhs);
      if (!lnum || !rnum) {
        // Can't be converted to a literal number.
        break;
      }
      uint32_t shiftCount = rnum->truncateToUInt32() & 0x1f;
      // Large enough to hold both int32_t and uint32_t values.
      int64_t result = 0;
      if (kind == OpKind::LeftShiftKind) {
        // Truncate to unsigned so that the shift doesn't happen on negative
        // values. Cast it to a 32-bit signed int to get the sign back, then
        // promote to 64 bits.
        result = static_cast<int32_t>(lnum->truncateToUInt32() << shiftCount);
      } else if (kind == OpKind::RightShiftKind) {
        result = static_cast<int64_t>(lnum->truncateToInt32() >> shiftCount);
      } else {
        result = static_cast<int64_t>(lnum->truncateToUInt32() >> shiftCount);
      }
      return builder.getLiteralNumber(result);
    }
    case OpKind::AddKind: { // +   (+=)
      // Handle numeric constants:
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->getValue() + rightLiteralNum->getValue());
      }

      // Handle string concat:
      if (leftStr && rightStr) {
        SmallString<256> result = buildString(
            ctx.toString(leftStr->getValue()),
            ctx.toString(rightStr->getValue()));
        return builder.getLiteralString(result.str());
      }

      if (leftNull && rightNull) {
        return builder.getLiteralPositiveZero();
      }

      if (leftUndef && rightUndef) {
        return builder.getLiteralNaN();
      }

      if (leftNull) {
        if (rightLiteralNum) {
          return rhs;
        } else if (rightStr) {
          SmallString<256> result =
              buildString("null", ctx.toString(rightStr->getValue()));
          return builder.getLiteralString(result.str());
        }
      }

      if (rightNull) {
        if (leftLiteralNum) {
          return lhs;
        } else if (leftStr) {
          SmallString<256> result =
              buildString(ctx.toString(leftStr->getValue()), "null");
          return builder.getLiteralString(result.str());
        }
      }

      if (leftUndef) {
        if (rightLiteralNum) {
          return builder.getLiteralNaN();
        } else if (rightStr) {
          SmallString<256> result =
              buildString("undefined", ctx.toString(rightStr->getValue()));
          return builder.getLiteralString(result.str());
        }
      }

      if (rightUndef) {
        if (leftLiteralNum) {
          return builder.getLiteralNaN();
        } else if (leftStr) {
          SmallString<256> result =
              buildString(ctx.toString(leftStr->getValue()), "undefined");
          return builder.getLiteralString(result.str());
        }
      }

      break;
    }
    case OpKind::SubtractKind: // -   (-=)
      // Handle numeric constants:
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->getValue() - rightLiteralNum->getValue());
      }

      break;

    case OpKind::MultiplyKind: // *   (*=)
      // Handle numeric constants:
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->getValue() * rightLiteralNum->getValue());
      }

      if ((leftNull && rightLiteralNum) || (rightNull && leftLiteralNum) ||
          (leftNull && rightNull)) {
        if ((leftLiteralNum && std::signbit(leftLiteralNum->getValue())) ||
            (rightLiteralNum && std::signbit(rightLiteralNum->getValue()))) {
          return builder.getLiteralNegativeZero();
        }
        return builder.getLiteralPositiveZero();
      }

      break;
    case OpKind::DivideKind: // /   (/=)
      if (leftLiteralNum && rightLiteralNum) {
        // This relies on IEEE 754 double division. All modern compilers
        // implement this.
        return builder.getLiteralNumber(
            leftLiteralNum->getValue() / rightLiteralNum->getValue());
      }
      break;
    case OpKind::ModuloKind: // %   (%=)
      // Note that fmod differs slightly from the ES spec with regards to how
      // numbers not representable by double are rounded. This difference can be
      // ignored in practice, so most JS VMs use fmod.
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            std::fmod(leftLiteralNum->getValue(), rightLiteralNum->getValue()));
      }

      break;
    case OpKind::ExponentiationKind: // ** (**=)
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(hermes::expOp(
            leftLiteralNum->getValue(), rightLiteralNum->getValue()));
      }
      break;
    case OpKind::OrKind: // |   (|=)
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->truncateToInt32() |
            rightLiteralNum->truncateToInt32());
      }

      break;
    case OpKind::XorKind: // ^   (^=)
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->truncateToInt32() ^
            rightLiteralNum->truncateToInt32());
      }

      break;
    case OpKind::AndKind: // &   (&=)
      if (leftLiteralNum && rightLiteralNum) {
        return builder.getLiteralNumber(
            leftLiteralNum->truncateToInt32() &
            rightLiteralNum->truncateToInt32());
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
  if (auto *num = llvh::dyn_cast<LiteralNumber>(operand)) {
    char buf[NUMBER_TO_STRING_BUF_SIZE];
    auto len = numberToString(num->getValue(), buf, sizeof(buf));
    return builder.getLiteralString(StringRef(buf, len));
  }
  return nullptr;
}

LiteralNumber *hermes::evalToNumber(IRBuilder &builder, Literal *operand) {
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
