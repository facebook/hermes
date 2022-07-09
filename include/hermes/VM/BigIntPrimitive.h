/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_BIGINTPRIMITIVE_H
#define HERMES_VM_BIGINTPRIMITIVE_H

#include "hermes/VM/Runtime.h"

#include "hermes/Support/BigIntSupport.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Handle.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/MathExtras.h"
#include "llvh/Support/TrailingObjects.h"

namespace hermes {
namespace vm {
/// A variable size GCCell used to store the words that compose the bigint.
class BigIntPrimitive final
    : public VariableSizeRuntimeCell,
      private llvh::TrailingObjects<BigIntPrimitive, bigint::BigIntDigitType> {
 public:
  using DigitType = bigint::BigIntDigitType;
  static constexpr uint32_t DigitSizeInBytes = bigint::BigIntDigitSizeInBytes;
  static_assert(
      llvh::isPowerOf2_64(DigitSizeInBytes),
      "DigitSizeInBytes is not power of 2.");

 private:
  friend class llvh::TrailingObjects<BigIntPrimitive, DigitType>;

  friend void BigIntPrimitiveBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  /// Number of BigInt digits in the trailing storage area.
  uint32_t numDigits;

  /// Number of bytes in the trailing storage area. Always a multiple of
  /// DigitSizeInBytes.
  uint32_t numTrailingBytes() const {
    return numDigits * DigitSizeInBytes;
  }

  static const VTable vt;

  /// \return The exact GCCell size to hold a BigIntPrimitive plus the Digits in
  /// \p bytes sign-extended so that \p bytes' size is a multiple of
  /// DigitSizeInBytes.
  static uint32_t calcCellSizeInBytes(uint32_t numDigits);

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BigIntPrimitiveKind;
  }

  static constexpr CellKind getCellKind() {
    return CellKind::BigIntPrimitiveKind;
  }

  /// \return a newly allocated BigIntPrimitive representing \p value (a signed
  /// integer). Brings down the VM on OOM.
  template <typename T>
  static std::enable_if_t<std::is_signed<T>::value, Handle<BigIntPrimitive>>
  fromSignedNoThrow(T value, Runtime &runtime) {
    return runtime.makeHandle<BigIntPrimitive>(
        runtime.ignoreAllocationFailure(fromSigned(value, runtime)));
  }

  /// \return a newly allocated BigIntPrimitive representing \p value (a signed
  /// integer).
  template <typename T>
  static std::enable_if_t<std::is_signed<T>::value, CallResult<HermesValue>>
  fromSigned(T value, Runtime &runtime) {
    const auto *ptr = reinterpret_cast<const uint8_t *>(&value);
    const uint32_t size = sizeof(T);

    return fromBytes(llvh::makeArrayRef(ptr, size), runtime);
  }

  /// \return a newly allocated BigIntPrimitive representing Z( R( \p value ) ).
  static CallResult<HermesValue> fromDouble(double value, Runtime &runtime);

  /// \return a newly allocated BigIntPrimitive with digits filled from \p
  /// bytes, possibly sign-extending to a multiple of DigitType. Brings down the
  /// VM on OOM.
  static Handle<BigIntPrimitive> fromBytesNoThrow(
      llvh::ArrayRef<uint8_t> bytes,
      Runtime &runtime) {
    return runtime.makeHandle<BigIntPrimitive>(
        runtime.ignoreAllocationFailure(fromBytes(bytes, runtime)));
  }

  /// \return a newly allocated BigIntPrimitive with digits filled from \p
  /// bytes.
  static CallResult<HermesValue> fromBytes(
      llvh::ArrayRef<uint8_t> bytes,
      Runtime &runtime) {
    const uint32_t numDigits = bigint::numDigitsForSizeInBytes(bytes.size());

    auto ret = createUninitializedWithNumDigits(numDigits, runtime);
    if (LLVM_UNLIKELY(ret == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    auto res = raiseOnError(
        bigint::initWithBytes(ret->getMutableRef(runtime), bytes), runtime);

    if (LLVM_UNLIKELY(res != ExecutionStatus::RETURNED)) {
      return res;
    }

    return HermesValue::encodeBigIntValue(ret->getBigIntPrimitive());
  }

  /// \return A view of all the digits in this object's trailing storage.
  llvh::ArrayRef<DigitType> getDigits() const {
    return llvh::makeArrayRef(getTrailingObjects<DigitType>(), numDigits);
  }

  /// \return A view of the raw the bytes in this object's trailing storage.
  /// Always returns an array ref whose size is a multiple of DigitSizeInBytes.
  llvh::ArrayRef<uint8_t> getRawDataFull() const {
    auto digits = getDigits();
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(digits.data());
    const uint32_t size = digits.size() * DigitSizeInBytes;
    return llvh::makeArrayRef(ptr, size);
  }

  /// \return A compact view of the bytes in this object's trailing storage,
  /// i.e., the returned view drops any bytes which merely represent a
  /// sign-extension of the previous byte.
  llvh::ArrayRef<uint8_t> getRawDataCompact() const {
    return bigint::dropExtraSignBits(getRawDataFull());
  }

  /// \return This bigint converted to a string in \p radix.
  CallResult<HermesValue> toString(Runtime &runtime, uint8_t radix) const;

  /// Compares this with \p other. Logically similar to *this - *other.
  /// \return < 0 if this is less than other; > 0, if other is less than this;
  /// and 0 if this and other represent the same bigint.
  int32_t compare(const BigIntPrimitive *other) const {
    return bigint::compare(
        this->getImmutableRefUnsafe(), other->getImmutableRefUnsafe());
  }

  /// Same as compare(const BigIntPrimitive *) above, but specialized for the
  /// case when comparing with a integral element.
  template <typename T>
  std::enable_if_t<std::is_signed<T>::value, int32_t> compare(T value) const {
    return bigint::compare(this->getImmutableRefUnsafe(), value);
  }

  // Supported Math Operations

  /// \return - \p src
  static CallResult<HermesValue> unaryMinus(
      Handle<BigIntPrimitive> src,
      Runtime &runtime);

  /// \return ~ \p src
  static CallResult<HermesValue> unaryNOT(
      Handle<BigIntPrimitive> src,
      Runtime &runtime);

  /// \return \p lhs - \p rhs
  static CallResult<HermesValue> subtract(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// \return \p lhs + \p rhs
  static CallResult<HermesValue> add(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// \return \p lhs * \p rhs
  static CallResult<HermesValue> multiply(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// \return \p lhs / \p rhs
  static CallResult<HermesValue> divide(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// \return \p lhs % \p rhs
  static CallResult<HermesValue> remainder(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// \return \p lhs & \p rhs
  static CallResult<HermesValue> bitwiseAND(
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      Runtime &runtime);

  /// N.B.: public so we can create using runtime.makeAVariable. Do not call.
  explicit BigIntPrimitive(uint32_t numDigits);

  /// \return true if this bigint is negative, and false if it is zero or
  /// positive.
  bool sign() const {
    return bigint::isNegative(getImmutableRefUnsafe());
  }

  /// \return R(Z(*this))
  double toDouble() const {
    return bigint::toDouble(this->getImmutableRefUnsafe());
  }

 private:
  static ExecutionStatus raiseOnError(
      bigint::OperationStatus status,
      Runtime &runtime) {
    switch (status) {
      case bigint::OperationStatus::RETURNED:
        return ExecutionStatus::RETURNED;
      case bigint::OperationStatus::DEST_TOO_SMALL:
        return runtime.raiseRangeError(
            "BigInt is too small for the operation result");
      case bigint::OperationStatus::TOO_MANY_DIGITS:
        return runtime.raiseRangeError("Maximum BigInt size exceeded");
      case bigint::OperationStatus::DIVISION_BY_ZERO:
        return runtime.raiseRangeError("Division by zero");
    }
  }

  template <typename BigIntRefTy>
  struct SafeBigIntRef : public BigIntRefTy {
    SafeBigIntRef() = delete;
    SafeBigIntRef(const SafeBigIntRef &) = delete;
    SafeBigIntRef(SafeBigIntRef &&) = delete;
    SafeBigIntRef &operator=(const SafeBigIntRef &) = delete;
    SafeBigIntRef &operator=(SafeBigIntRef &&) = delete;

    template <typename... Args>
    explicit SafeBigIntRef(Runtime &rt, Args &&...args)
#ifndef HERMES_SLOW_DEBUG
        : BigIntRefTy{std::forward<Args>(args)...} {
    }
#else // HERMES_SLOW_DEBUG
        : BigIntRefTy{std::forward<Args>(args)...}, noAlloc(rt) {
    }

    NoAllocScope noAlloc;
#endif // HERMES_SLOW_DEBUG
  };

  using SafeImmutableBigIntRef = SafeBigIntRef<bigint::ImmutableBigIntRef>;
  using SafeMutableBigIntRef = SafeBigIntRef<bigint::MutableBigIntRef>;

  /// Helper method that converts this SafeBigIntRef to a
  /// BigIntPrimitive::ImmutableBigIntRef, which is just a
  /// bigint::ImmutableBigIntRef with a NoAllocScope (in non-optimized builds)
  /// to iron out bugs that could occur due to memory allocation happening while
  /// ImmutableBigIntRefs are alive.
  SafeImmutableBigIntRef getImmutableRef(Runtime &rt) const {
    return SafeImmutableBigIntRef(
        rt, getTrailingObjects<DigitType>(), numDigits);
  }

  /// Returns a bigint::ImmutableBigIntRef representing this BigIntPrimitive.
  /// Note that this API will not "attach" a NoAllocScope to the returned
  /// reference, and thus should only be invoked in contexts where allocation is
  /// impossible (i.e., there's no Runtime & available, thus no memory can be
  /// allocated).
  bigint::ImmutableBigIntRef getImmutableRefUnsafe() const {
    return bigint::ImmutableBigIntRef{
        getTrailingObjects<DigitType>(), numDigits};
  }

  /// Helper class that holds BigIntPrimitives that have been created but are
  /// still uninitialized. In this state, BigIntPrimitives should only be used
  /// as output arguments to the APIs in bigint::*. This abstraction prevents
  /// uninitialized BigIntPrimitives to escape the confines of this class -- and
  /// also leads to a runtime assert() in HERMES_SLOW_DEBUG builds.
  class UninitializedBigIntPrimitive {
   public:
    UninitializedBigIntPrimitive() = delete;
    UninitializedBigIntPrimitive(const UninitializedBigIntPrimitive &) =
        default;
    UninitializedBigIntPrimitive &operator=(
        const UninitializedBigIntPrimitive &) = default;

    explicit UninitializedBigIntPrimitive(BigIntPrimitive *u)
        : uninitialized(u) {}

    SafeMutableBigIntRef getMutableRef(Runtime &rt) {
#ifdef HERMES_SLOW_DEBUG
      possiblyInitialized = true;
#endif // HERMES_SLOW_DEBUG
      return SafeMutableBigIntRef{
          rt,
          uninitialized->getTrailingObjects<DigitType>(),
          uninitialized->numDigits};
    }

    BigIntPrimitive *getBigIntPrimitive() const {
#ifdef HERMES_SLOW_DEBUG
      assert(
          possiblyInitialized &&
          "UninitializedBigIntPrimitive was never possibly-initialized.");
#endif // HERMES_SLOW_DEBUG
      return uninitialized;
    }

   private:
    BigIntPrimitive *uninitialized = nullptr;
#ifdef HERMES_SLOW_DEBUG
    // possiblyInitialized is a proxy for determining whether uninitialized
    // has been passed to an API in bigint::* or not. The assumption here is
    // that UninitializedBigIntPrimitive must be converted to a
    // bigint::MutableBigIntRef before being initialized (i.e., before being the
    // result of a bigint::* operation).
    bool possiblyInitialized = false;
#endif // HERMES_SLOW_DEBUG
  };

  /// Create and returns an uninitialized BigIntPrimitive with \p numDigits
  /// digits.
  static CallResult<UninitializedBigIntPrimitive>
  createUninitializedWithNumDigits(uint32_t numDigits, Runtime &runtime) {
    if (bigint::tooManyDigits(numDigits)) {
      return raiseOnError(bigint::OperationStatus::TOO_MANY_DIGITS, runtime);
    }

    const uint32_t cellSizeInBytes = calcCellSizeInBytes(numDigits);

    UninitializedBigIntPrimitive ret{
        runtime.makeAVariable<BigIntPrimitive>(cellSizeInBytes, numDigits)};
    return ret;
  }

  using UnaryOp = bigint::OperationStatus (*)(
      bigint::MutableBigIntRef dst,
      bigint::ImmutableBigIntRef src);
  static CallResult<HermesValue> unaryOp(
      UnaryOp op,
      Handle<BigIntPrimitive> src,
      size_t numDigits,
      Runtime &runtime);

  using BinaryOp = bigint::OperationStatus (*)(
      bigint::MutableBigIntRef dst,
      bigint::ImmutableBigIntRef lhs,
      bigint::ImmutableBigIntRef src);
  static CallResult<HermesValue> binaryOp(
      BinaryOp op,
      Handle<BigIntPrimitive> lhs,
      Handle<BigIntPrimitive> rhs,
      uint32_t numDigitsResult,
      Runtime &runtime);
};
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_BIGINTPRIMITIVE_H
