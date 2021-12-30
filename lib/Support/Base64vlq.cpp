/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Base64vlq.h"

namespace hermes {

namespace base64vlq {

static constexpr uint32_t Base64Count = 64;
static constexpr const char Base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// Expect to have 64 characters + terminating null (unfortunately)
static_assert(
    sizeof(Base64Chars) == Base64Count + 1,
    "Base64Chars has unexpected length");

/// Decode a Base64 character.
/// \return the integer value, or None if not a Base64 character.
static OptValue<uint32_t> base64Decode(char c) {
  // This is not very optimal. A 127-byte lookup table would be faster.
  for (const char &bc : Base64Chars) {
    if (c == bc)
      return &bc - &Base64Chars[0];
  }
  return llvh::None;
}

// Each digit is stored in the low 5 bits, with bit 6 as a continuation flag.
enum {
  // Width in bits of each VLQ digit.
  DigitWidth = 5,

  // Mask to get at just the digit bits of a Base64 value.
  DigitMask = (1 << DigitWidth) - 1,

  // Flag indicating more digits follow.
  ContinuationFlag = 1 << DigitWidth,

  // The first digit reserves the LSB for the sign of the final value.
  SignBit = 1,
};

llvh::raw_ostream &encode(llvh::raw_ostream &OS, int32_t value) {
  // The first sextet reserves the LSB for the sign bit. Make space for it.
  // Widen to 64 bits to ensure we can multiply the value by 2.
  int64_t wideVal = value;
  wideVal *= 2;
  if (wideVal < 0)
    wideVal = -wideVal | SignBit;
  assert(wideVal >= 0 && "wideVal should not be negative any more");
  do {
    auto digit = wideVal & DigitMask;
    wideVal >>= DigitWidth;
    if (wideVal > 0)
      digit |= ContinuationFlag;
    assert(digit < Base64Count && "digit cannot exceed Base64 character count");
    OS << Base64Chars[digit];
  } while (wideVal > 0);
  return OS;
}

OptValue<int32_t> decode(const char *&begin, const char *end) {
  int64_t result = 0;
  for (const char *cursor = begin; cursor < end; cursor++) {
    OptValue<uint32_t> word = base64Decode(*cursor);
    int32_t shift = DigitWidth * (cursor - begin);

    // Fail if our shift has grown too large, or if we couldn't decode a Base64
    // character. This shift check is what ensures 'result' cannot overflow.
    if (!word || shift > 32)
      return llvh::None;

    // Digits are encoded little-endian (least-significant first).
    int64_t digit = *word & DigitMask;
    result |= (digit << shift);

    // Continue if we have a continuation flag.
    if (*word & ContinuationFlag)
      continue;

    // We're done. The sign bit is the LSB; fix up the sign.
    // Ensure we use a /2 (not shift) because we need round-towards-zero.
    if (result & SignBit) {
      result = -result;
    }
    result /= 2;

    // Check for overflow.
    if (result > INT32_MAX || result < INT32_MIN)
      return llvh::None;

    // Success. Update the begin pointer to say where we stopped.
    begin = cursor + 1;
    return int32_t(result);
  }
  // Exited the loop: we never found a character without a continuation bit.
  return llvh::None;
}

} // namespace base64vlq

} // namespace hermes
