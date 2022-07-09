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
    double value,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::fromDoubleResultSize(value);

  auto u =
      BigIntPrimitive::createUninitializedWithNumDigits(numDigits, runtime);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = bigint::fromDouble(u->getMutableRef(runtime), value);
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(res, runtime);
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
    uint64_t n,
    Handle<BigIntPrimitive> src,
    Runtime &runtime) {
  if (n == 0) {
    return BigIntPrimitive::fromSigned(0, runtime);
  }

  const uint32_t numDigits =
      bigint::asIntNResultSize(n, src->getImmutableRef(runtime));
  return unaryOp(makeTruncAdapter<&bigint::asIntN>(n), src, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::asUintN(
    uint64_t n,
    Handle<BigIntPrimitive> src,
    Runtime &runtime) {
  if (n == 0) {
    return BigIntPrimitive::fromSigned(0, runtime);
  }

  const uint32_t numDigits =
      bigint::asUintNResultSize(n, src->getImmutableRef(runtime));
  return unaryOp(
      makeTruncAdapter<&bigint::asUintN>(n), src, numDigits, runtime);
}

template <typename UnaryOpT>
CallResult<HermesValue> BigIntPrimitive::unaryOp(
    UnaryOpT op,
    Handle<BigIntPrimitive> src,
    size_t numDigits,
    Runtime &runtime) {
  auto u =
      BigIntPrimitive::createUninitializedWithNumDigits(numDigits, runtime);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = (op)(u->getMutableRef(runtime), src->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(res, runtime);
  }

  return HermesValue::encodeBigIntValue(u->getBigIntPrimitive());
}

CallResult<HermesValue> BigIntPrimitive::unaryMinus(
    Handle<BigIntPrimitive> src,
    Runtime &runtime) {
  if (src->compare(0) == 0) {
    return HermesValue::encodeBigIntValue(*src);
  }

  const uint32_t numDigits =
      bigint::unaryMinusResultSize(src->getImmutableRef(runtime));
  return unaryOp(&bigint::unaryMinus, src, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::unaryNOT(
    Handle<BigIntPrimitive> src,
    Runtime &runtime) {
  const uint32_t numDigits =
      bigint::unaryNotResultSize(src->getImmutableRef(runtime));
  return unaryOp(&bigint::unaryNot, src, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::binaryOp(
    BinaryOp op,
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    uint32_t numDigitsResult,
    Runtime &runtime) {
  auto u = BigIntPrimitive::createUninitializedWithNumDigits(
      numDigitsResult, runtime);

  if (LLVM_UNLIKELY(u == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto res = (*op)(
      u->getMutableRef(runtime),
      lhs->getImmutableRef(runtime),
      rhs->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(res, runtime);
  }

  return HermesValue::encodeBigIntValue(u->getBigIntPrimitive());
}

CallResult<HermesValue> BigIntPrimitive::add(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const size_t numDigits = bigint::addResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::add, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::subtract(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const size_t numDigits = bigint::subtractResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::subtract, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::multiply(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::multiplyResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::multiply, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::divide(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::divideResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::divide, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::remainder(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::remainderResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::remainder, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::exponentiate(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  uint32_t tmpDstSize = bigint::BigIntMaxSizeInDigits;
  bigint::TmpStorage tmpDst(tmpDstSize);
  bigint::MutableBigIntRef dst{tmpDst.requestNumDigits(tmpDstSize), tmpDstSize};
  auto res = bigint::exponentiate(
      dst, lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  if (LLVM_UNLIKELY(res != bigint::OperationStatus::RETURNED)) {
    return raiseOnError(res, runtime);
  }

  auto ptr = reinterpret_cast<const uint8_t *>(dst.digits);
  uint32_t size = dst.numDigits * DigitSizeInBytes;
  return BigIntPrimitive::fromBytes(llvh::makeArrayRef(ptr, size), runtime);
}

CallResult<HermesValue> BigIntPrimitive::bitwiseAND(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::bitwiseANDResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::bitwiseAND, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::bitwiseOR(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::bitwiseORResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::bitwiseOR, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::bitwiseXOR(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::bitwiseXORResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::bitwiseXOR, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::leftShift(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::leftShiftResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::leftShift, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::signedRightShift(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  const uint32_t numDigits = bigint::signedRightShiftResultSize(
      lhs->getImmutableRef(runtime), rhs->getImmutableRef(runtime));
  return binaryOp(&bigint::signedRightShift, lhs, rhs, numDigits, runtime);
}

CallResult<HermesValue> BigIntPrimitive::unsignedRightShift(
    Handle<BigIntPrimitive> lhs,
    Handle<BigIntPrimitive> rhs,
    Runtime &runtime) {
  return runtime.raiseTypeError("BigInts have no unsigned shift");
}

CallResult<HermesValue> BigIntPrimitive::inc(
    Handle<BigIntPrimitive> src,
    Runtime &runtime) {
  auto incAdapter = [](bigint::MutableBigIntRef dst,
                       bigint::ImmutableBigIntRef lhs) {
    constexpr bigint::SignedBigIntDigitType one = 1ll;
    return bigint::addSigned(dst, lhs, one);
  };

  const size_t numDigits =
      bigint::addSignedResultSize(src->getImmutableRef(runtime), 1);
  return unaryOp(incAdapter, src, numDigits, runtime);
}

} // namespace vm
} // namespace hermes
