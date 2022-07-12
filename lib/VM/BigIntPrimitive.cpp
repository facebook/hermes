/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BigIntPrimitive.h"

#include "hermes/VM/StringPrimitive.h"

#include "llvh/ADT/APInt.h"
#include "llvh/Support/MathExtras.h"

#include <cstdlib>

namespace hermes {
namespace vm {

static_assert(
    std::is_same<BigIntPrimitive::DigitType, llvh::APInt::WordType>::value,
    "BigIntPrimitive digit must match APInt::WordType");

const VTable BigIntPrimitive::vt{CellKind::BigIntPrimitiveKind, 0};

void BigIntPrimitiveBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&BigIntPrimitive::vt);
}

uint32_t BigIntPrimitive::calcCellSizeInBytes(uint32_t numDigits) {
  return sizeof(BigIntPrimitive) + numDigits * DigitSizeInBytes;
}

BigIntPrimitive::BigIntPrimitive(uint32_t numDigits) : numDigits(numDigits) {
  assert(
      (calcCellSizeInBytes(numDigits) - numTrailingBytes() ==
       sizeof(BigIntPrimitive)) &&
      "cell must fit BigIntPrimitive + Digits exactly");
}

CallResult<HermesValue> BigIntPrimitive::fromDouble(
    Runtime &runtime,
    double value) {
  const uint32_t numDigits = bigint::fromDoubleResultSize(value);

  auto u =
      BigIntPrimitive::createUninitializedWithNumDigits(runtime, numDigits);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = bigint::fromDouble(u->getMutableRef(runtime), value);
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(runtime, res);
  }

  return HermesValue::encodeBigIntValue(u->getBigIntPrimitive());
}

CallResult<HermesValue> BigIntPrimitive::toString(
    Runtime &runtime,
    uint8_t radix) const {
  std::string result = bigint::toString(this->getImmutableRef(runtime), radix);
  return StringPrimitive::createEfficient(
      runtime, createASCIIRef(result.c_str()));
}

template <auto Op>
static auto makeTruncAdapter(uint64_t n) {
  return [n](bigint::MutableBigIntRef dst, bigint::ImmutableBigIntRef src) {
    return (*Op)(dst, n, src);
  };
}

CallResult<HermesValue> BigIntPrimitive::asIntN(
    Runtime &runtime,
    uint64_t n,
    Handle<BigIntPrimitive> src) {
  if (n == 0) {
    return BigIntPrimitive::fromSigned(runtime, 0);
  }

  const uint32_t numDigits =
      bigint::asIntNResultSize(n, src->getImmutableRef(runtime));
  return unaryOp(runtime, makeTruncAdapter<&bigint::asIntN>(n), src, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::asUintN(
    Runtime &runtime,
    uint64_t n,
    Handle<BigIntPrimitive> src) {
  if (n == 0) {
    return BigIntPrimitive::fromSigned(runtime, 0);
  }

  const uint32_t numDigits =
      bigint::asUintNResultSize(n, src->getImmutableRef(runtime));
  return unaryOp(
      runtime, makeTruncAdapter<&bigint::asUintN>(n), src, numDigits);
}

template <typename UnaryOpT>
CallResult<HermesValue> BigIntPrimitive::unaryOp(
    Runtime &runtime,
    UnaryOpT op,
    Handle<BigIntPrimitive> src,
    size_t numDigits) {
  auto u =
      BigIntPrimitive::createUninitializedWithNumDigits(runtime, numDigits);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = (op)(u->getMutableRef(runtime), src->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(runtime, res);
  }

  return HermesValue::encodeBigIntValue(u->getBigIntPrimitive());
}

CallResult<HermesValue> BigIntPrimitive::unaryMinus(
    Runtime &runtime,
    Handle<BigIntPrimitive> src) {
  if (src->compare(0) == 0) {
    return HermesValue::encodeBigIntValue(*src);
  }

  const uint32_t numDigits =
      bigint::unaryMinusResultSize(src->getImmutableRef(runtime));
  return unaryOp(runtime, &bigint::unaryMinus, src, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::unaryNOT(
    Runtime &runtime,
    Handle<BigIntPrimitive> src) {
  const uint32_t numDigits =
      bigint::unaryNotResultSize(src->getImmutableRef(runtime));
  return unaryOp(runtime, &bigint::unaryNot, src, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::binaryOp(
    Runtime &runtime,
    BinaryOp op,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    uint32_t numDigitsResult) {
  auto u = BigIntPrimitive::createUninitializedWithNumDigits(
      runtime, numDigitsResult);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = (*op)(
      u->getMutableRef(runtime),
      lhs->getImmutableRef(runtime),
      rhs->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(runtime, res);
  }

  return HermesValue::encodeBigIntValue(u->getBigIntPrimitive());
}

CallResult<HermesValue> BigIntPrimitive::add(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const size_t numDigits = bigint::addResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::add, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::subtract(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const size_t numDigits = bigint::subtractResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::subtract, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::multiply(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::multiplyResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::multiply, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::divide(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::divideResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::divide, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::remainder(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::remainderResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::remainder, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::exponentiate(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  uint32_t tmpDstSize = bigint::BigIntMaxSizeInDigits;
  bigint::TmpStorage tmpDst(tmpDstSize);
  bigint::MutableBigIntRef dst{tmpDst.requestNumDigits(tmpDstSize), tmpDstSize};
  auto res = bigint::exponentiate(
      dst, lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(runtime, res);
  }

  auto ptr = reinterpret_cast<const uint8_t *>(dst.digits);
  uint32_t size = dst.numDigits * DigitSizeInBytes;
  return BigIntPrimitive::fromBytes(runtime, llvh::makeArrayRef(ptr, size));
}

CallResult<HermesValue> BigIntPrimitive::bitwiseAND(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::bitwiseANDResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::bitwiseAND, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::bitwiseOR(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::bitwiseORResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::bitwiseOR, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::bitwiseXOR(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::bitwiseXORResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::bitwiseXOR, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::leftShift(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::leftShiftResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::leftShift, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::signedRightShift(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  const uint32_t numDigits = bigint::signedRightShiftResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(runtime, &bigint::signedRightShift, lhs, rhs, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::unsignedRightShift(
    Runtime &runtime,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs) {
  return runtime.raiseTypeError("BigInts have no unsigned shift");
}

CallResult<HermesValue> BigIntPrimitive::inc(
    Runtime &runtime,
    Handle<BigIntPrimitive> src) {
  auto incAdapter = [](bigint::MutableBigIntRef dst,
                       bigint::ImmutableBigIntRef lhs) {
    constexpr bigint::SignedBigIntDigitType one = 1ll;
    return bigint::addSigned(dst, lhs, one);
  };

  const size_t numDigits =
      bigint::addSignedResultSize(src->getImmutableRef(runtime), 1);
  return unaryOp(runtime, incAdapter, src, numDigits);
}

CallResult<HermesValue> BigIntPrimitive::dec(
    Runtime &runtime,
    Handle<BigIntPrimitive> src) {
  auto decAdapter = [](bigint::MutableBigIntRef dst,
                       bigint::ImmutableBigIntRef lhs) {
    constexpr bigint::SignedBigIntDigitType one = 1ll;
    return bigint::subtractSigned(dst, lhs, one);
  };

  const size_t numDigits =
      bigint::subtractSignedResultSize(src->getImmutableRef(runtime), 1);
  return unaryOp(runtime, decAdapter, src, numDigits);
}

} // namespace vm
} // namespace hermes
