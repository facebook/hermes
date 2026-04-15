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
#include <cstring>

namespace hermes {

double float16ToDouble(uint16_t bits) {
  uint32_t sign = (bits >> 15) & 1;
  uint32_t exp = (bits >> 10) & 0x1F;
  uint32_t mant = bits & 0x3FF;

  uint64_t dbits;

  if (exp == 0) {
    if (mant == 0) {
      // ±zero.
      dbits = (uint64_t)sign << 63;
    } else {
      // Subnormal: shift mantissa left until bit 10 is set to normalize.
      int shift = 0;
      while ((mant & 0x400) == 0) {
        mant <<= 1;
        shift++;
      }
      mant &= 0x3FF; // Remove the implicit leading 1.
      // Unbiased exponent is (-14 - shift), biased for double: 1009 - shift.
      uint64_t dexp = (uint64_t)(1009 - shift);
      dbits = ((uint64_t)sign << 63) | (dexp << 52) | ((uint64_t)mant << 42);
    }
  } else if (exp == 31) {
    // Infinity or NaN. Preserve sign and mantissa bits.
    dbits = ((uint64_t)sign << 63) | ((uint64_t)0x7FF << 52) |
        ((uint64_t)mant << 42);
  } else {
    // Normal: rebias exponent from float16 (bias 15) to double (bias 1023).
    uint64_t dexp = (uint64_t)(exp + 1008);
    dbits = ((uint64_t)sign << 63) | (dexp << 52) | ((uint64_t)mant << 42);
  }

  double result;
  memcpy(&result, &dbits, sizeof(result));
  return result;
}

uint16_t doubleToFloat16(double d) {
  uint64_t dbits;
  memcpy(&dbits, &d, sizeof(dbits));

  uint32_t sign = (uint32_t)((dbits >> 63) & 1);
  int32_t exp = (int32_t)((dbits >> 52) & 0x7FF);
  uint64_t mant = dbits & 0x000FFFFFFFFFFFFF;

  // Infinity or NaN.
  if (exp == 0x7FF) {
    if (mant == 0)
      return (uint16_t)((sign << 15) | 0x7C00);
    // Return a quiet NaN (set bit 9).
    return (uint16_t)((sign << 15) | 0x7E00);
  }

  // Zero or double subnormal (magnitude < 2^-1022, far below float16 range).
  if (exp == 0)
    return (uint16_t)(sign << 15);

  // Add the implicit leading 1 bit.
  mant |= (uint64_t)1 << 52;

  int32_t unbiased = exp - 1023;

  // Overflow to ±infinity.
  if (unbiased > 15)
    return (uint16_t)((sign << 15) | 0x7C00);

  int32_t f16Exp;
  int32_t shift; // Right-shift amount to extract 10-bit float16 mantissa.

  if (unbiased >= -14) {
    // Normal float16 range.
    f16Exp = unbiased + 15;
    shift = 42; // 52 - 10
  } else if (unbiased >= -25) {
    // Subnormal float16 range (or rounding boundary).
    f16Exp = 0;
    shift = -14 - unbiased + 42;
  } else {
    // Underflow to ±zero.
    return (uint16_t)(sign << 15);
  }

  uint32_t f16Mant;

  if (shift >= 53) {
    // shift == 53 (unbiased == -25): the implicit 1 is the guard bit.
    // Nearest representable values are 0 and the smallest subnormal (2^-24).
    f16Mant = 0;
    uint64_t remaining = mant & (((uint64_t)1 << 52) - 1);
    // Round up only if there are fractional bits beyond the halfway point.
    // At exact halfway (remaining == 0), round to even keeps 0.
    if (remaining)
      f16Mant = 1;
  } else {
    f16Mant = (uint32_t)(mant >> shift);
    if (f16Exp > 0)
      f16Mant &= 0x3FF; // Strip implicit 1 for normal representation.

    // Round to nearest even: examine guard bit and remaining bits.
    uint64_t guard = (mant >> (shift - 1)) & 1;
    uint64_t remaining =
        (shift >= 2) ? (mant & (((uint64_t)1 << (shift - 1)) - 1)) : 0;

    if (guard && (remaining || (f16Mant & 1))) {
      f16Mant++;
      if (f16Exp > 0 && f16Mant > 0x3FF) {
        // Mantissa overflow in normal range: carry into exponent.
        f16Mant = 0;
        f16Exp++;
        if (f16Exp > 30)
          return (uint16_t)((sign << 15) | 0x7C00);
      } else if (f16Exp == 0 && f16Mant >= 0x400) {
        // Subnormal promoted to normal by rounding.
        f16Mant &= 0x3FF;
        f16Exp = 1;
      }
    }
  }

  return (uint16_t)((sign << 15) | (f16Exp << 10) | f16Mant);
}

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

    char nDigitsBuf[NUMBER_TO_STRING_BUF_SIZE];
    char *nDigitsBegin = uintToStr(
        (unsigned int)std::abs(n - 1), llvh::MutableArrayRef<char>(nDigitsBuf));
    char *nDigitsEnd = nDigitsBuf + NUMBER_TO_STRING_BUF_SIZE - 1;
    int nLen = nDigitsEnd - nDigitsBegin + 1;

    // 11. If k = 1, then
    if (k == 1) {
      // a. Return the string-concatenation of:
      // the code unit of the single digit of s
      *destPtr++ = sDigitsBegin[0];
      // - the code unit 0x0065 (LATIN SMALL LETTER E)
      *destPtr++ = '\x65';
      // - exponentSign
      *destPtr++ = exponentSign;
      // - the code units of the decimal representation of abs(n - 1)
      for (int i = 0; i < nLen; ++i) {
        *destPtr++ = nDigitsBegin[i];
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
        *destPtr++ = nDigitsBegin[i];
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
