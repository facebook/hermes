/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Conversions.h"

#include "dtoa/dtoa.h"

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

/// ES5.1 9.8.1
size_t numberToString(double m, char *dest, size_t destSize) {
  assert(destSize >= NUMBER_TO_STRING_BUF_SIZE);
  (void)destSize;
  DtoaAllocator<> dalloc{};

  if (std::isnan(m)) {
    strcpy(dest, "NaN");
    return 3;
  }

  if (m == 0) {
    strcpy(dest, "0");
    return 1;
  }

  if (m == std::numeric_limits<double>::infinity()) {
    strcpy(dest, "Infinity");
    return 8;
  }
  if (m == -std::numeric_limits<double>::infinity()) {
    strcpy(dest, "-Infinity");
    return 9;
  }

  // After special cases, run dtoa to convert.
  // We do this manually because we need all of the output of dtoa.
  // Note that n, k, s are defined per ES5.1 9.8.1

  // Iterator for easier population.
  char *destPtr = dest;

  // Decimal point index.
  int n;

  // 1 if negative, 0 else.
  int sign;

  // Points to the end of the string s after it's populated.
  char *sEnd;

  char *s = ::g_dtoa(dalloc, m, 0, 0, &n, &sign, &sEnd);

  if (sign)
    *destPtr++ = '-';

  // Length of decimal representation of s.
  int k = sEnd - s;

  if (k <= n && n <= 21) {
    // Step 6 of 9.8.1.
    for (int i = 0; i < k; ++i) {
      *destPtr++ = s[i];
    }
    for (int i = 0; i < n - k; ++i) {
      *destPtr++ = '0';
    }
  } else if (0 < n && n <= 21) {
    // Step 7 of 9.8.1.
    for (int i = 0; i < n; ++i) {
      *destPtr++ = s[i];
    }
    *destPtr++ = '.';
    for (int i = n; i < k; ++i) {
      *destPtr++ = s[i];
    }
  } else if (-6 < n && n <= 0) {
    // Step 8 of 9.8.1.
    *destPtr++ = '0';
    *destPtr++ = '.';
    for (int i = 0; i < -n; ++i) {
      *destPtr++ = '0';
    }
    for (int i = 0; i < k; ++i) {
      *destPtr++ = s[i];
    }
  } else if (k == 1) {
    // Step 9 of 9.8.1.
    char nBuf[NUMBER_TO_STRING_BUF_SIZE];
    int nLen = ::snprintf(nBuf, sizeof(nBuf), "%d", ::abs(n - 1));

    *destPtr++ = s[0];
    *destPtr++ = 'e';
    *destPtr++ = n - 1 < 0 ? '-' : '+';
    for (int i = 0; i < nLen; ++i) {
      *destPtr++ = nBuf[i];
    }
  } else {
    // Step 10 of 9.8.1.
    char nBuf[NUMBER_TO_STRING_BUF_SIZE];
    int nLen = ::snprintf(nBuf, sizeof(nBuf), "%d", ::abs(n - 1));

    *destPtr++ = s[0];
    *destPtr++ = '.';
    for (int i = 1; i < k; ++i) {
      *destPtr++ = s[i];
    }
    *destPtr++ = 'e';
    *destPtr++ = n - 1 < 0 ? '-' : '+';
    for (int i = 0; i < nLen; ++i) {
      *destPtr++ = nBuf[i];
    }
  }

  // Null-terminate
  *destPtr++ = '\0';
  assert(static_cast<size_t>(destPtr - dest) < NUMBER_TO_STRING_BUF_SIZE);

  g_freedtoa(dalloc, s);
  return destPtr - dest - 1;
}
} // namespace hermes

extern "C" {
size_t hermes_numberToString(double m, char *dest, size_t destSize) {
  return hermes::numberToString(m, dest, destSize);
}
}
