/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/Base64Util.h"

#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

namespace {

constexpr const std::array<char, 64> Base64Chars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

// A lookup table that map (Base64-encoded) ASCII back to binary.
constexpr const std::array<unsigned char, 128> decMap = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64,
    64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64};

template <typename T>
inline bool isWhitespace(T c) {
  return (
      c == '\x09' || c == '\x0A' || c == '\x0C' || c == '\x0D' || c == '\x20');
}

} // namespace

template <typename T>
bool base64Encode(llvh::ArrayRef<T> str, StringBuilder &builder) {
  uint64_t strLength = str.size();

  // An implementation of the algorithm at
  // https://www.rfc-editor.org/rfc/rfc4648#section-4
  // Adapted from folly's base64Encode implementation.
  uint32_t i = 0;
  while ((strLength - i) >= 3) {
    if (str[i] > 0xFF || str[i + 1] > 0xFF || str[i + 2] > 0xFF) {
      return false;
    }

    uint8_t aaab = str[i];
    uint8_t bbcc = str[i + 1];
    uint8_t cddd = str[i + 2];

    uint8_t aaa = aaab >> 2;
    uint8_t bbb = ((aaab << 4) | (bbcc >> 4)) & 0x3f;
    uint8_t ccc = ((bbcc << 2) | (cddd >> 6)) & 0x3f;
    uint8_t ddd = cddd & 0x3f;

    builder.appendCharacter(Base64Chars[aaa]);
    builder.appendCharacter(Base64Chars[bbb]);
    builder.appendCharacter(Base64Chars[ccc]);
    builder.appendCharacter(Base64Chars[ddd]);

    i += 3;
  }

  if (i == strLength) {
    return true;
  }

  if (str[i] > 0xFF) {
    return false;
  }
  uint8_t aaab = str[i];
  uint8_t aaa = aaab >> 2;
  builder.appendCharacter(Base64Chars[aaa]);

  // Duplicating some tail handling to try to do less jumps.
  if (strLength - i == 1) {
    uint8_t b00 = aaab << 4 & 0x3f;
    builder.appendCharacter(Base64Chars[b00]);
    builder.appendCharacter('=');
    builder.appendCharacter('=');
    return true;
  }

  // When there are 2 characters left.
  assert(strLength - i == 2);
  if (str[i + 1] > 0xFF) {
    return false;
  }
  uint8_t bbcc = str[i + 1];
  uint8_t bbb = ((aaab << 4) | (bbcc >> 4)) & 0x3f;
  uint8_t cc0 = (bbcc << 2) & 0x3f;
  builder.appendCharacter(Base64Chars[bbb]);
  builder.appendCharacter(Base64Chars[cc0]);
  builder.appendCharacter('=');
  return true;
}

template bool base64Encode(llvh::ArrayRef<char> str, StringBuilder &builder);
template bool base64Encode(
    llvh::ArrayRef<char16_t> str,
    StringBuilder &builder);

template <typename T>
OptValue<uint32_t> base64DecodeOutputLength(llvh::ArrayRef<T> str) {
  // Figure out the actual string length after ignoring all whitespaces.
  uint64_t strLength = 0;
  T lastChar = 0;
  T secondLastChar = 0;
  for (const auto c : str) {
    // Only increment length if character is not a whitespace
    if (!isWhitespace(c)) {
      strLength++;
      secondLastChar = lastChar;
      lastChar = c;
    }
  }

  uint32_t numPadding = 0;
  if (strLength % 4 == 0) {
    // Check to see if the last character or the last 2 characters are the
    // padding character.
    if (strLength > 0 && lastChar == '=') {
      numPadding++;
      if (strLength > 1 && secondLastChar == '=') {
        numPadding++;
      }
    }
  } else {
    // The input string should always be divisible by 4.
    return llvh::None;
  }

  // This shouldn't overflow because the value is guaranteed to be smaller.
  uint32_t expectedLength = (strLength / 4 * 3) - numPadding;
  if (strLength != 0 && expectedLength == 0) {
    return llvh::None;
  }
  return expectedLength;
}

template OptValue<uint32_t> base64DecodeOutputLength(llvh::ArrayRef<char> str);
template OptValue<uint32_t> base64DecodeOutputLength(
    llvh::ArrayRef<char16_t> str);

template <typename T>
bool base64Decode(llvh::ArrayRef<T> str, StringBuilder &builder) {
  // Iterate over the trimmed \p str, decode every \c c into a sextet and store
  // into a buffer \c buf of capacity 32 bits. \c bufSize is maintained to
  // track how many bits are actually buffered.
  uint32_t buf = 0;
  uint32_t bufSize = 0;
  for (const auto c : str) {
    if (isWhitespace(c)) {
      continue;
    }

    if (LLVM_UNLIKELY(c > 127) || LLVM_UNLIKELY(c < 0)) {
      return false;
    }

    if (c == '=') {
      break;
    }

    unsigned char sextet = decMap[c];
    if (LLVM_UNLIKELY(sextet >= 64)) {
      return false;
    }

    // Making room for the new sextet.
    buf = (buf << 6) + sextet;
    bufSize += 6;

    // Once buffer is filled over a byte, evacuate a byte to the output.
    if (bufSize >= 8) {
      char16_t decodedChar = (buf >> (bufSize - 8)) & 0xFF;
      builder.appendCharacter(decodedChar);
      bufSize -= 8;
    }
  }

  return builder.currentLength() == builder.maxLength();
}

template bool base64Decode(llvh::ArrayRef<char> str, StringBuilder &builder);
template bool base64Decode(
    llvh::ArrayRef<char16_t> str,
    StringBuilder &builder);

} // namespace vm
} // namespace hermes
