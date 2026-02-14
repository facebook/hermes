/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Conversions.h"
#include "hermes/Support/FastDoubleToDecimal.h"

#include "llvh/ADT/ArrayRef.h"

#include <cmath>

namespace hermes {

/// Convert a double to a 32-bit integer according to ES5.1 section 9.5.
/// It can also be used for converting to an unsigned integer, which has the
/// same bit pattern.
/// NaN and Infinity are always converted to 0. The rest of the numbers are
/// converted to a (conceptually) infinite-width integer and the low 32 bits of
/// the integer are then returned.
int32_t truncateToInt32SlowPath(double d) {
  double tmp = d; // Allow d to stay in a register.
  uint64_t bits = llvh::DoubleToBits(tmp);
  int exp = (int)(bits >> 52) & 0x7FF;
  // A negative sign is turned into 2, a positive into 0. Subtracting from 1
  // gives us what we need.
  int sign = 1 - ((int)((int64_t)bits >> 62) & 2);
  uint64_t m = bits & 0xFFFFFFFFFFFFFul;

  // Check for a denormalized exponent. We can bail early in that case.
  if (!exp)
    return 0;

  // Subtract the IEEE bias (1023). Additionally, move the decimal point to
  // the right of the mantissa by further decreasing the exponent by 52.
  exp -= 1023 + 52;
  // Add the implied leading 1 bit.
  m |= 1ull << 52;

  // The sign of the exponent tells us which way to shift.
  if (exp >= 0) {
    // Check if the shift would push all bits out. Additionally this catches
    // Infinity and NaN.
    // Cast to int64 here to avoid UB for the case where sign is negative one
    // and m << exp is exactly INT32_MIN, since a 32-bit signed int cannot hold
    // the resulting INT32_MAX + 1. When it is returned, it will be correctly
    // set to INT32_MIN.
    return exp <= 31 ? sign * (int64_t)(m << exp) : 0;
  } else {
    // Check if the shift would push out the entire mantissa.
    // We need to use int64_t here in case we are multiplying
    // -1 and 2147483648.
    return exp > -53 ? sign * (int64_t)(m >> -exp) : 0;
  }
}

/// Convert an unsigned integer \p value to its decimal string representation,
/// not null terminated. Least significant digit will be written to the last
/// position in \p buf.
/// \param value is the value to be converted.
/// \param buf is a pre-allocated buffer to hold the string contents. Must be
///   at least 20 characters to hold the maximum uint64_t value.
/// \return pointer to the beginning of the string within \p buf.
template <typename T>
static char *uintToStr(T value, llvh::MutableArrayRef<char> buf) {
  static_assert(std::is_integral<T>::value, "T must be integral");
  static_assert(std::is_unsigned<T>::value, "T must be unsigned");
  // uint64_t max number is 20 digits.
  assert(buf.size() >= 20 && "buffer must hold at least 20 characters");
  char *bufEnd = buf.end() - 1;
  if (value == 0) {
    *bufEnd = '0';
    return bufEnd;
  }
  // Extract digits least-significant first.
  for (; value > 0; value /= 10, --bufEnd) {
    T digit = value % 10;
    *bufEnd = (char)('0' + digit);
  }
  return bufEnd + 1;
}

/// ES2025 6.1.6.1.20 Number::toString ( x, radix ). This only implements
/// base 10.
size_t numberToString(double m, char *dest, size_t destSize) {
  assert(destSize >= NUMBER_TO_STRING_BUF_SIZE);
  (void)destSize;
  // 1. If x is NaN, return "NaN".
  if (std::isnan(m)) {
    strcpy(dest, "NaN");
    return 3;
  }
  // 2. If x is either +0 or -0, return "0".
  if (m == 0) {
    strcpy(dest, "0");
    return 1;
  }

  // 3. If x < -0, return the string-concatenation of "-" and
  // Number::toString(-x, radix). We don't actually implement this with
  // recursion, we simply add the negative sign in-place.

  // 4. If x is +Infinity, return "Infinity".
  if (m == std::numeric_limits<double>::infinity()) {
    strcpy(dest, "Infinity");
    return 8;
  }
  if (m == -std::numeric_limits<double>::infinity()) {
    strcpy(dest, "-Infinity");
    return 9;
  }
  char *destPtr = dest;

  // 5. Let n, k, and s be integers such that:
  // k >= 1,
  // radix**(k - 1) <= s < radix**k,
  // s * radix**(n - k) is x,
  // and k is as small as possible.
  // Note that k is the number of digits in the representation of s using radix
  // radix, that s is not divisible by radix, and that the least significant
  // digit of s is not necessarily uniquely determined by these criteria.

  // Informally, another way of understanding these variables are:
  // s is an integer containing the significant digits.
  // k is how many digits s has.
  // n says where the decimal (or radix) point goes.
  // 123.45 as example: s = 12345 k = 5, n = 3.
  DoubleDecimalComponents conv = fastDoubleToDecimal(m);
  if (conv.negative) {
    *destPtr++ = '-';
  }
  // Convert significand to a temporary digit buffer.
  char sDigitsBuf[NUMBER_TO_STRING_BUF_SIZE];
  char *sDigitsBegin =
      uintToStr(conv.significand, llvh::MutableArrayRef<char>(sDigitsBuf));
  char *sDigitsEnd = sDigitsBuf + NUMBER_TO_STRING_BUF_SIZE - 1;
  int k = sDigitsEnd - sDigitsBegin + 1;
  int n = conv.exponent + k;

  // 6. If radix != 10 or n is in the inclusive interval from -5 to 21, then
  if (n >= -5 && n <= 21) {
    // a. If n >= k, then
    if (n >= k) {
      // i. Return the string-concatenation of:
      // - the code units of the k digits of the representation of s using radix
      //   radix
      for (int i = 0; i < k; ++i) {
        *destPtr++ = sDigitsBegin[i];
      }
      // - n - k occurrences of the code unit 0x0030 (DIGIT ZERO)
      for (int i = 0; i < n - k; ++i) {
        *destPtr++ = '\x30';
      }
    } else if (n > 0) {
      // b. Else if n > 0, then
      // i. Return the string-concatenation of:
      // - the code units of the most significant n digits of the representation
      // of s using radix radix.
      for (int i = 0; i < n; ++i) {
        *destPtr++ = sDigitsBegin[i];
      }
      // the code unit 0x002E (FULL STOP)
      *destPtr++ = '\x2e';
      // the code units of the remaining k - n digits of the representation of s
      // using radix radix
      for (int i = n; i < k; ++i) {
        *destPtr++ = sDigitsBegin[i];
      }
    } else {
      // c. Else,
      // i. Assert: n <= 0.
      assert(n <= 0 && "n is not less than or equal to zero");
      // Return the string-concatenation of:
      // - the code unit 0x0030 (DIGIT ZERO)
      *destPtr++ = '\x30';
      // - the code unit 0x002E (FULL STOP)
      *destPtr++ = '\x2e';
      // - -n occurrences of the code unit 0x0030 (DIGIT ZERO)
      for (int i = 0; i < -n; ++i) {
        *destPtr++ = '\x30';
      }
      // - the code units of the k digits of the representation of s using radix
      // radix
      for (int i = 0; i < k; ++i) {
        *destPtr++ = sDigitsBegin[i];
      }
    }
  } else {
    // 7. NOTE: In this case, the input will be represented using scientific E
    // notation, such as 1.2e+3.
    // 8. Assert: radix is 10 (this is always true for this function.)
    // 9. If n < 0, then
    // a. Let exponentSign be the code unit 0x002D (HYPHEN-MINUS).
    // 10. Else,
    // a. Let exponentSign be the code unit 0x002B (PLUS SIGN).
    char exponentSign = n < 0 ? '\x2d' : '\x2b';

    char nBuf[NUMBER_TO_STRING_BUF_SIZE];
    int expVal = n - 1;
    int nLen = ::snprintf(nBuf, sizeof(nBuf), "%d", ::abs(expVal));

    // 11. If k = 1, then
    if (k == 1) {
      // a. Return the string-concatenation of:
      // the code unit of the single digit of s
      *destPtr++ = '0' + conv.significand;
      // - the code unit 0x0065 (LATIN SMALL LETTER E)
      *destPtr++ = '\x65';
      // - exponentSign
      *destPtr++ = exponentSign;
      // - the code units of the decimal representation of abs(n - 1)
      for (int i = 0; i < nLen; ++i) {
        *destPtr++ = nBuf[i];
      }
    } else {
      // 12. Return the string-concatenation of:
      // - the code unit of the most significant digit of the decimal
      // representation of s
      *destPtr++ = sDigitsBegin[0];
      // - the code unit 0x002E (FULL STOP)
      *destPtr++ = '\x2e';
      // - the code units of the remaining k - 1 digits of the decimal
      // representation of s
      for (int i = 1; i < k; ++i) {
        *destPtr++ = sDigitsBegin[i];
      }
      // - the code unit 0x0065 (LATIN SMALL LETTER E)
      *destPtr++ = '\x65';
      // - exponentSign
      *destPtr++ = exponentSign;
      // - the code units of the decimal representation of abs(n - 1)
      for (int i = 0; i < nLen; ++i) {
        *destPtr++ = nBuf[i];
      }
    }
  }

  // Null-terminate
  *destPtr++ = '\0';
  assert(static_cast<size_t>(destPtr - dest) < NUMBER_TO_STRING_BUF_SIZE);
  return static_cast<size_t>(destPtr - dest - 1);
}
} // namespace hermes

extern "C" {
size_t hermes_numberToString(double m, char *dest, size_t destSize) {
  return hermes::numberToString(m, dest, destSize);
}
}
