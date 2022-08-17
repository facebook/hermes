/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_CONVERSIONS_H
#define HERMES_SUPPORT_CONVERSIONS_H

#include "hermes/Support/OptValue.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MathExtras.h"

#include <cstdint>

namespace hermes {

/// Convert a double to a 32-bit integer according to ES5.1 section 9.5.
/// It can also be used for converting to an unsigned integer, which has the
/// same bit pattern.
/// NaN and Infinity are always converted to 0. The rest of the numbers are
/// converted to a (conceptually) infinite-width integer and the low 32 bits of
/// the integer are then returned.
/// This is the out-of-line "slow path" which performs a relatively slow
/// conversion by manipulating bits in the representation of the double value.
int32_t truncateToInt32SlowPath(double d);

/// Convert a double to a 32-bit integer according to ES5.1 section 9.5.
/// It can also be used for converting to an unsigned integer, which has the
/// same bit pattern.
/// NaN and Infinity are always converted to 0. The rest of the numbers are
/// converted to a (conceptually) infinite-width integer and the low 32 bits of
/// the integer are then returned.
int32_t truncateToInt32(double d) LLVM_NO_SANITIZE("float-cast-overflow");
inline int32_t truncateToInt32(double d) {
  // Check of the value can be converted to integer without loss. We want to
  // use the widest available integer because this conversion will be much
  // faster than the bit-twiddling slow path.
  intmax_t fast = (intmax_t)d;
  if (LLVM_LIKELY(fast == d))
    return (int32_t)fast;
  return truncateToInt32SlowPath(d);
}

inline uint32_t truncateToUInt32(double d) {
  return (uint32_t)truncateToInt32(d);
}

/// Convert a string in the range defined by [first, last) to an array index
/// following ES5.1 15.4:
/// "A property name P (in the form of a String value) is an array index if and
/// only if ToString(ToUint32(P)) is equal to P and ToUint32(P) is not equal to
/// 2**32−1."
/// IT must meet the requirements of InputIterator and its 'value_type' must be
/// an integral type holding a superset of ASCII, typically char, char16_t,
/// char32_t or wchar_t.
/// \return an "empty" OptValue if the string isn't a valid index, or the
///   converted uint32_t value otherwise.
template <typename IT>
OptValue<uint32_t> toArrayIndex(IT first, IT last) {
  /// Empty string is invalid.
  if (first == last)
    return llvh::None;

  // Leading 0 is special.
  if (*first == '0') {
    ++first;
    // Just "0"?
    if (first == last)
      return 0;
    // Leading 0 is invalid otherwise.
    return llvh::None;
  }

  uint32_t res = 0;
  do {
    auto ch = *first;
    if (ch < '0' || ch > '9')
      return llvh::None;
    uint64_t tmp = (uint64_t)res * 10 + (ch - '0');
    // Check for overflow.
    if (tmp & ((uint64_t)0xFFFFFFFFu << 32))
      return llvh::None;
    res = (uint32_t)tmp;
  } while (++first != last);

  // 0xFFFFFFFF is not a valid array index.
  if (res == 0xFFFFFFFFu)
    return llvh::None;

  return res;
}

/// A convenient wrapper around 'toArrayIndex(first,last)'.
inline OptValue<uint32_t> toArrayIndex(llvh::StringRef str) {
  return toArrayIndex(str.begin(), str.end());
}

/// Attempt to convert a double to a valid JavaScript array number.
OptValue<uint32_t> doubleToArrayIndex(double d)
    LLVM_NO_SANITIZE("float-cast-overflow");
inline OptValue<uint32_t> doubleToArrayIndex(double d) {
  uint32_t index = (uint32_t)d;
  if (index == d && index != 0xFFFFFFFFu)
    return index;
  return llvh::None;
}

/// Size of buffer that must be passed to numberToString.
const size_t NUMBER_TO_STRING_BUF_SIZE = 32;

/// Convert a double number to string, following ES5.1 9.8.1.
/// \param m the number to convert
/// \param dest output buffer
/// \param destSize size of dest, at least NUMBER_TO_STRING_BUF_SIZE
/// \return the length of the generated string (excluding the terminating zero).
size_t numberToString(double m, char *dest, size_t destSize);

/// Takes a letter (a-z or A-Z) and makes it lowercase.
template <typename T>
inline T charLetterToLower(T ch) {
  return ch | 32;
}

/// Takes a non-empty string (without the leading "0x" if hex) and parses it
/// as radix \p radix, calling \p digitCallback with the value of each digit,
/// going from left to right.
/// \param AllowNumericSeparator when true, allow '_' as a separator and ignore
/// it when parsing.
/// \returns true if the string was successfully parsed, false otherwise.
template <bool AllowNumericSeparator, class Iterable, typename Callback>
bool parseIntWithRadixDigits(Iterable str, int radix, Callback digitCallback) {
  assert(
      radix >= 2 && radix <= 36 && "Invalid radix passed to parseIntWithRadix");
  assert(str.begin() != str.end() && "Empty string");
  for (auto it = str.begin(); it != str.end(); ++it) {
    auto c = *it;
    auto cLow = charLetterToLower(c);
    if ('0' <= c && c <= '9' && c < '0' + radix) {
      digitCallback(c - '0');
    } else if ('a' <= cLow && cLow < 'a' + radix - 10) {
      digitCallback(cLow - 'a' + 0xa);
    } else if (AllowNumericSeparator && LLVM_UNLIKELY(c == '_')) {
      // Ensure the '_' is in a valid location.
      // It can only be between two existing digits.
      if (it == str.begin() || it == str.end() - 1) {
        return false;
      }
      // Note that the previous character must not be '_' if the current
      // character is '_', because we would have returned None.
      // So just check if the next character is '_'.
      char next = *(it + 1);
      if (next == '_') {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

/// Takes a non-empty string (without the leading "0x" if hex) and parses it
/// as radix \p radix.
/// \param AllowNumericSeparator when true, allow '_' as a separator and ignore
///    it when parsing.
/// \returns the double that results on success, empty on error.
template <bool AllowNumericSeparator, class Iterable>
OptValue<double> parseIntWithRadix(Iterable str, int radix) {
  double result = 0;
  bool success = parseIntWithRadixDigits<AllowNumericSeparator>(
      str, radix, [&result, radix](uint8_t d) {
        result *= radix;
        result += d;
      });
  if (!success)
    return llvh::None;

  // The largest value that fits in the 53-bit mantissa (2**53).
  const double MAX_MANTISSA = 9007199254740992.0;
  if (result >= MAX_MANTISSA && llvh::isPowerOf2_32(radix)) {
    // If the result is too high, manually reconstruct the double if
    // the radix is 2, 4, 8, 16, 32.
    // Go through the digits bit by bit, and manually round when necessary.
    result = 0;

    // Keep track of how far along parsing is using this enum.
    enum Mode {
      LEADING_ZERO, // Haven't seen a set bit yet.
      MANTISSA, // Lower bits that allow exact representation.
      EXP_LOW_BIT, // Lowest bit of the exponent (determine rounding).
      EXP_LEADING_ZERO, // Zeros in the exponent.
      EXPONENT, // Seen a set bit in the exponent.
    };

    size_t remainingMantissa = 53;
    double expFactor = 0.0;
    size_t curDigit = 0;

    bool lastMantissaBit = false;
    bool lowestExponentBit = false;

    Mode curMode = Mode::LEADING_ZERO;
    auto itr = str.begin();
    auto e = str.end();
    for (size_t bitMask = 0;;) {
      if (bitMask == 0) {
        // Only need to do this check every log2(radix) iterations.
        if (itr == e) {
          break;
        }
        // We know it fits in 7 bits after the first pass.
        char c = (char)*itr;
        if (AllowNumericSeparator && LLVM_UNLIKELY(c == '_')) {
          ++itr;
          continue;
        }
        auto cLow = charLetterToLower(c);
        if ('0' <= c && c <= '9') {
          curDigit = c - '0';
        } else {
          // Must be valid, else we would have returned NaN on first pass.
          assert('a' <= cLow && cLow < 'a' + radix - 10);
          curDigit = cLow - 'a' + 0xa;
        }
        ++itr;
        // Reset bitmask to look at the first bit.
        bitMask = radix >> 1;
      }
      bool curBit = (curDigit & bitMask) != 0;
      bitMask >>= 1;

      switch (curMode) {
        case Mode::LEADING_ZERO:
          // Go through the string until we hit a nonzero bit.
          if (curBit) {
            --remainingMantissa;
            result = 1;
            // No more leading zeros.
            curMode = Mode::MANTISSA;
          }
          break;
        case Mode::MANTISSA:
          // Read into the lower bits of the mantissa (plain binary).
          result *= 2;
          result += curBit;
          --remainingMantissa;
          if (remainingMantissa == 0) {
            // Out of bits, set the last bit and go to the next curMode.
            lastMantissaBit = curBit;
            curMode = Mode::EXP_LOW_BIT;
          }
          break;
        case Mode::EXP_LOW_BIT:
          lowestExponentBit = curBit;
          expFactor = 2.0;
          curMode = Mode::EXP_LEADING_ZERO;
          break;
        case Mode::EXP_LEADING_ZERO:
          if (curBit) {
            curMode = Mode::EXPONENT;
          }
          expFactor *= 2.0;
          break;
        case Mode::EXPONENT:
          expFactor *= 2.0;
          break;
      }
    }
    switch (curMode) {
      case Mode::LEADING_ZERO:
      case Mode::MANTISSA:
      case Mode::EXP_LOW_BIT:
        // Nothing to do here, already read those in.
        break;
      case Mode::EXP_LEADING_ZERO:
        // Rounding up.
        result += lowestExponentBit && lastMantissaBit;
        result *= expFactor;
        break;
      case Mode::EXPONENT:
        // Rounding up.
        result += lowestExponentBit;
        result *= expFactor;
        break;
    }
  }
  return result;
}

} // namespace hermes

extern "C" {
size_t hermes_numberToString(double m, char *dest, size_t destSize);
}

#endif // HERMES_SUPPORT_CONVERSIONS_H
