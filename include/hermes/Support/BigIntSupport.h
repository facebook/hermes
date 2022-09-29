/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_BIGINT_H
#define HERMES_SUPPORT_BIGINT_H

#include "hermes/Support/Compiler.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MathExtras.h"

#include <deque>
#include <optional>
#include <string>
#include <vector>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace bigint {

enum class ParsedSign {
  Minus = -1,
  None = 0,
  Plus = 1,
};

/// https://tc39.es/ecma262/#sec-stringintegerliteral-grammar
/// Parse \p src as a StringIntegerLiteral.
///
/// \return an empty optional if \p src is not a valid StringIntegerLiteral, in
/// which case \p outError (if not null) will contain a description of the
/// error; or, if \p src is a valid StringIntegerLiteral, then a string with the
/// bigint digits is returned; \p radix is set to the literal's radix; \p sign
/// represents which sign was parsed, if any.
std::optional<std::string> getStringIntegerLiteralDigitsAndSign(
    llvh::ArrayRef<char> src,
    uint8_t &radix,
    ParsedSign &sign,
    std::string *outError = nullptr);
std::optional<std::string> getStringIntegerLiteralDigitsAndSign(
    llvh::ArrayRef<char16_t> src,
    uint8_t &radix,
    ParsedSign &sign,
    std::string *outError = nullptr);

/// https://tc39.es/ecma262/#sec-numericvalue
/// Parse \p src as a NumericValue.
///
/// \return an empty optional if \p src is not a valid NumericValue, in which
/// case \p outError (if not null) will contain a description of the error; or,
/// if \p src is a valid NumericValue, then a string with the bigint digits is
/// returned; \p radix is set to the literal's radix.
std::optional<std::string> getNumericValueDigits(
    llvh::StringRef src,
    uint8_t &radix,
    std::string *outError = nullptr);

using SignedBigIntDigitType = int64_t;
using BigIntDigitType = uint64_t;

inline constexpr uint32_t BigIntDigitSizeInBytes =
    static_cast<uint32_t>(sizeof(BigIntDigitType));
inline constexpr uint32_t BigIntDigitSizeInBits = BigIntDigitSizeInBytes * 8;

/// Arbitrary upper limit on number of Digits a bigint may have.
inline constexpr uint32_t BigIntMaxSizeInDigits =
    0x400; // 1k digits == 8k bytes

/// Arbitrary upper limit on number of bytes a bigint may have.
inline constexpr uint32_t BigIntMaxSizeInBytes =
    BigIntDigitSizeInBytes * BigIntMaxSizeInDigits;

/// Helper function that should be called before allocating a Digits array on
/// the stack.
inline constexpr bool tooManyDigits(uint32_t numDigits) {
  return BigIntMaxSizeInDigits < numDigits;
}

/// Helper function that returns if the given number of bytes exceeds the
/// maximum BigInt size in bytes.
inline constexpr bool tooManyBytes(uint32_t numBytes) {
  return BigIntMaxSizeInBytes < numBytes;
}

/// \return number of BigInt digits to represent \p v bits.
inline uint32_t numDigitsForSizeInBits(uint32_t v) {
  return static_cast<uint32_t>(llvh::alignTo(v, BigIntDigitSizeInBits)) /
      BigIntDigitSizeInBits;
}

/// \return number of BigInt digits to represent \p v bytes.
inline uint32_t numDigitsForSizeInBytes(uint32_t v) {
  return static_cast<uint32_t>(llvh::alignTo(v, BigIntDigitSizeInBytes)) /
      BigIntDigitSizeInBytes;
}

/// \return how many chars in base \p radix fit a BigIntDigitType.
inline uint32_t constexpr maxCharsPerDigitInRadix(uint8_t radix) {
  // To compute the lower bound of bits in a BigIntDigitType "covered" by a
  // char. For power of 2 radixes, it is known (exactly) that each character
  // covers log2(radix) bits. For non-power of 2 radixes, a lower bound is
  // log2(greatest power of 2 that is less than radix).
  uint32_t minNumBitsPerChar = radix < 4 ? 1
      : radix < 8                        ? 2
      : radix < 16                       ? 3
      : radix < 32                       ? 4
                                         : 5;

  // With minNumBitsPerChar being the lower bound estimate of how many bits each
  // char can represent, the upper bound of how many chars "fit" in a bigint
  // digit is ceil(sizeofInBits(bigint digit) / minNumBitsPerChar).
  uint32_t numCharsPerDigits = BigIntDigitSizeInBits / (1 << minNumBitsPerChar);

  return numCharsPerDigits;
}

/// Returns another view of \p src where high order bytes that are just used
/// for sign extension are dropped. Returns an empty ArrayRef if all bytes in \p
/// src are zero.
llvh::ArrayRef<uint8_t> dropExtraSignBits(llvh::ArrayRef<uint8_t> src);

/// \return the BigIntDigitType value that represents the sign extension of \p
/// byte. I.e., returns 0 if \p value is 0b0xxx....xxx, and ~0, if \p value is
/// 0x1xxx....xxx.
template <typename D, typename T, typename UT = std::make_unsigned_t<T>>
inline constexpr std::enable_if_t<std::is_integral_v<T>, D> getSignExtValue(
    const T &value) {
  uint32_t UnsignedTSizeInBits = sizeof(UT) * 8;
  UT unsignedValue = value;

  // We rely on the unsigned (i.e., "logical") shift right to convert the sign
  // bit to [0, 1], then do 0 - [0, 1] to get 0ull or ~0ull as the sign
  // extension value.
  uint64_t signExtValue = 0ull - (unsignedValue >> (UnsignedTSizeInBits - 1));

  // But still possibly truncate the value as requested by the caller.
  return static_cast<D>(signExtValue);
}

/// ImmutableBigIntRef is used to represent bigint payloads that are not mutated
/// by any of the API functions below.
struct ImmutableBigIntRef {
  const BigIntDigitType *digits;
  uint32_t numDigits;
};

/// MutableBigIntRef is used to represent bigint payloads that are mutated
/// by any of the API functions below. Note how numDigits is a reference to
/// numDigits - the API may modify that in order to canonicalize the payloads.
struct MutableBigIntRef {
  BigIntDigitType *digits;
  uint32_t &numDigits;
};

/// The "catch-all" enum type with the possible return type from the bigint
/// APIs. It contains all possible errors that any bigint API function could
/// return (e.g., division by zero), even for functions that never return them.
enum class OperationStatus : uint32_t {
  RETURNED,
  DEST_TOO_SMALL,
  TOO_MANY_DIGITS,
  DIVISION_BY_ZERO,
  NEGATIVE_EXPONENT,
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Initializes \p dst with \p data, sign-extending the bits as needed.
OperationStatus initWithBytes(
    MutableBigIntRef dst,
    llvh::ArrayRef<uint8_t> data);

/// \return true if \p src is a negative bigint, and false otherwise.
bool isNegative(ImmutableBigIntRef src);

/// \return number of digits needed to perform Z( R( \p src ) ).
uint32_t fromDoubleResultSize(double src);

/// \return \p dst = Z( R( \p src ) )
OperationStatus fromDouble(MutableBigIntRef dst, double src);

/// \return \p src converted to double.
double toDouble(ImmutableBigIntRef src);

/// Holds the bytes in a parsed BigInt value.
class ParsedBigInt {
  ParsedBigInt(const ParsedBigInt &) = delete;
  ParsedBigInt &operator=(const ParsedBigInt &) = delete;

 public:
  using Storage = std::vector<uint8_t>;

  ParsedBigInt(ParsedBigInt &&) = default;
  ParsedBigInt &operator=(ParsedBigInt &&) = default;

  static std::optional<ParsedBigInt> parsedBigIntFromStringIntegerLiteral(
      llvh::ArrayRef<char> input,
      std::string *outError = nullptr);

  static std::optional<ParsedBigInt> parsedBigIntFromStringIntegerLiteral(
      llvh::ArrayRef<char16_t> input,
      std::string *outError = nullptr);

  /// Tries to parse \p input assuming it is the text in a BigInt literal (see
  /// ES2022 12.8.3.2 SS: NumericValue for mor information). \p outError is set
  /// with a description of the first error encountered while parsing \p input.
  /// \return A ParsedBigInt if \p input can be parsed, or an empty optional
  /// otherwise.
  static std::optional<ParsedBigInt> parsedBigIntFromNumericValue(
      llvh::StringRef input,
      std::string *outError = nullptr);

  /// \return A compact representation of the BigInt. Compact means all most
  /// significant bytes in storage_ that can be inferred with a
  /// sign-extension are dropped.
  llvh::ArrayRef<uint8_t> getBytes() const {
    return dropExtraSignBits(storage_);
  }

  llvh::ArrayRef<uint8_t> getRawDataFull() const {
    return storage_;
  }

 private:
  ParsedBigInt(llvh::ArrayRef<uint8_t> bytes) : storage_(bytes) {}

  Storage storage_;
};

/// Helper class for allocating temporary storage needed for BigInt operations.
/// It uses inline storage for "small enough" temporary requirements, and heap
/// allocates
class TmpStorage {
  TmpStorage() = delete;
  TmpStorage(const TmpStorage &) = delete;
  TmpStorage(TmpStorage &&) = delete;

  TmpStorage &operator=(const TmpStorage &) = delete;
  TmpStorage &operator=(TmpStorage &&) = delete;

 public:
  explicit TmpStorage(uint32_t sizeInDigits)
      : storage_(sizeInDigits), data_(storage_.begin()) {}

  ~TmpStorage() = default;

  /// \return a pointer to \p size digits in the temporary storage.
  BigIntDigitType *requestNumDigits(uint32_t size) {
    BigIntDigitType *ret = data_;
    data_ += size;
    assert(data_ <= storage_.end() && "too many temporary digits requested.");
    return ret;
  }

 private:
  static constexpr uint32_t MaxStackTmpStorageInDigits = 4;
  llvh::SmallVector<BigIntDigitType, MaxStackTmpStorageInDigits> storage_;

  BigIntDigitType *data_;
};

/// \return \p src's representation in \p radix.
std::string toString(ImmutableBigIntRef src, uint8_t radix);

/// same as toString() above, but prints a byte payload.
OperationStatus
toString(std::string &out, llvh::ArrayRef<uint8_t> bytes, uint8_t radix);

/// Computes number of digits needed to perform asUintN(\p n, \p src), storing
/// the result in \p resultSize.
/// \returns OperationStatus::TOO_MANY_DIGITS if the result would be too large
/// to represent; and OperationStatus::RETURNED otherwise.
OperationStatus
asUintNResultSize(uint64_t n, ImmutableBigIntRef src, uint32_t &resultSize);

/// \return \p src % (2n ** \p n), zero extended.
OperationStatus
asUintN(MutableBigIntRef dst, uint64_t n, ImmutableBigIntRef src);

/// \return number of digits needed to perform asIntN(\p n, \p src).
uint32_t asIntNResultSize(uint64_t n, ImmutableBigIntRef src);

/// \return \p src % (2n ** \p n), sign extended; \p N-th bit is the sign bit.
OperationStatus
asIntN(MutableBigIntRef dst, uint64_t n, ImmutableBigIntRef src);

/// \return (\p lhs < \p rhs) ? negative value : (\p lhs > \p rhs) ? positive
/// value : zero.
int compare(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);
int compare(ImmutableBigIntRef lhs, SignedBigIntDigitType rhs);

/// \return Whether \p src can be losslessly truncated to a single
/// SignedBigIntDigitType (if signedTruncation == true) or BigIntDigitType
/// (signedTruncation == false) digit.
bool isSingleDigitTruncationLossless(
    ImmutableBigIntRef src,
    bool signedTruncation);

/// \return The first digit in \p src, or 0 if src.numDigits == 0.
inline BigIntDigitType truncateToSingleDigit(ImmutableBigIntRef src) {
  return src.numDigits == 0 ? 0 : src.digits[0];
}

/// \return number of digits needed to perform \p - src
uint32_t unaryMinusResultSize(ImmutableBigIntRef src);

/// \return \p dst = - \p src
OperationStatus unaryMinus(MutableBigIntRef dst, ImmutableBigIntRef src);

/// \return number of digits needed to perform \p ~ src
uint32_t unaryNotResultSize(ImmutableBigIntRef src);

/// \return \p dst = ~ \p src.
OperationStatus unaryNot(MutableBigIntRef dst, ImmutableBigIntRef src);

/// \return number of digits needed to perform \p lhs + \p rhs
uint32_t addResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs + \p rhs
OperationStatus
add(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs + \p sImm
uint32_t addSignedResultSize(
    ImmutableBigIntRef lhs,
    SignedBigIntDigitType sImm);

/// \return \p dst = \p lhs + \p sImm
OperationStatus addSigned(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    SignedBigIntDigitType sImm);

/// \return number of digits needed to perform \p lhs - \p rhs
uint32_t subtractResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs - \p rhs
OperationStatus
subtract(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs - \p sImm
uint32_t subtractSignedResultSize(
    ImmutableBigIntRef lhs,
    SignedBigIntDigitType sImm);

/// \return \p dst = \p lhs - \p sImm
OperationStatus subtractSigned(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    SignedBigIntDigitType sImm);

/// \return number of digits needed to perform \p lhs * \p rhs with full
/// precision
uint32_t multiplyResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs * \p rhs (full precision)
OperationStatus
multiply(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs / \p rhs
uint32_t divideResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs / \p rhs
OperationStatus
divide(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs % \p rhs
uint32_t remainderResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs % \p rhs
OperationStatus
remainder(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs ** \p rhs
/// N.B.: There is no easy way to compute a reasonable upper bound on how
/// many digits are needed to store the result of the exponentiate operation,
/// thus callers are expected to call with a "big enough" dst, and the operation
/// will try to compute the result in that buffer.
OperationStatus exponentiate(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs & \p rhs
uint32_t bitwiseANDResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return dst = lhs & rhs
OperationStatus bitwiseAND(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs | \p rhs
uint32_t bitwiseORResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs | \p rhs
OperationStatus
bitwiseOR(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs ^ \p rhs
uint32_t bitwiseXORResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs ^ \p rhs
OperationStatus bitwiseXOR(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs << \p rhs
uint32_t leftShiftResultSize(ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return \p dst = \p lhs << \p rhs
OperationStatus
leftShift(MutableBigIntRef dst, ImmutableBigIntRef lhs, ImmutableBigIntRef rhs);

/// \return number of digits needed to perform \p lhs >> \p rhs
uint32_t signedRightShiftResultSize(
    ImmutableBigIntRef lhs,
    ImmutableBigIntRef rhs);

/// \return dst = \p lhs >> \p rhs
OperationStatus signedRightShift(
    MutableBigIntRef dst,
    ImmutableBigIntRef lhs,
    ImmutableBigIntRef rhs);

/// A list of bytes representing all bigints known by UniquingBigIntTable.
using BigIntBytes = std::vector<uint8_t>;

/// A BigIntTableEntry is simply an (offset, length) pair for indexing the
/// bigint tables in the bytecode.
struct BigIntTableEntry {
  uint32_t offset;
  uint32_t length;
};

inline bool operator==(
    const BigIntTableEntry &lhs,
    const BigIntTableEntry &rhs) {
  return lhs.offset == rhs.offset && lhs.length == rhs.length;
}

/// A class responsible for assigning IDs to bigints.
class UniquingBigIntTable {
  /// List of created bigints. Use deque because it does not move elements
  /// when appending, which might invalidate the ArrayRefs.
  std::deque<ParsedBigInt> bigints_;

  using KeyType = llvh::ArrayRef<uint8_t>;

  /// Map from parsed bigint to its index. The
  /// StringRefs reference data owned by the bigints_ field.
  llvh::DenseMap<KeyType, uint32_t> keysToIndex_;

  /// A UniquingBigIntTable may not be copied.
  UniquingBigIntTable(const UniquingBigIntTable &) = delete;
  void operator=(const UniquingBigIntTable &) = delete;

 public:
  UniquingBigIntTable() = default;

  /// Adds a bigint to the table if not already present.
  /// \return the ID of the bigint.
  uint32_t addBigInt(ParsedBigInt bigint) {
    auto iter = keysToIndex_.find(keyFor(bigint));
    if (iter != keysToIndex_.end()) {
      return iter->second;
    }

    const uint32_t index = bigints_.size();
    bigints_.push_back(std::move(bigint));
    keysToIndex_[keyFor(bigints_.back())] = index;
    return index;
  }

  /// \return whether the table is empty.
  bool empty() const {
    return bigints_.empty();
  }

  /// Return the bigint entry list.
  std::vector<BigIntTableEntry> getEntryList() const;

  /// Return the combined bytecode buffer.
  BigIntBytes getDigitsBuffer() const;

 private:
  static KeyType keyFor(const ParsedBigInt &parsedBigInt) {
    return parsedBigInt.getBytes();
  }
};

} // namespace bigint
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_SUPPORT_BIGINT_H
