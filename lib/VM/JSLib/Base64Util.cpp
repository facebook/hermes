/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/Base64Util.h"

#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

namespace {

constexpr const std::array<char, 64> Base64Chars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

constexpr const std::array<char, 64> Base64UrlChars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

// A lookup table that map (Base64-encoded) ASCII back to binary.
constexpr const std::array<unsigned char, 128> decMap = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64,
    64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64};

/// Returns true if c is an ASCII whitespace character per the Infra Standard.
/// Used by both WHATWG forgiving-base64 and TC39 FromBase64.
template <typename T>
inline bool isWhitespace(T c) {
  return (
      c == '\x09' || c == '\x0A' || c == '\x0C' || c == '\x0D' || c == '\x20');
}

/// Decode a single base64 character to its 6-bit value.
/// Returns -1 if the character is invalid for the given alphabet.
/// Used by the TC39 FromBase64 algorithm.
int decodeBase64Char(char16_t c, bool useBase64url) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (useBase64url) {
    if (c == '-')
      return 62;
    if (c == '_')
      return 63;
  } else {
    if (c == '+')
      return 62;
    if (c == '/')
      return 63;
  }
  return -1;
}

} // namespace

template <typename T>
bool base64Encode(
    llvh::ArrayRef<T> str,
    StringBuilder &builder,
    bool useBase64url,
    bool omitPadding) {
  uint64_t strLength = str.size();
  const auto &chars = useBase64url ? Base64UrlChars : Base64Chars;

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

    builder.appendCharacter(chars[aaa]);
    builder.appendCharacter(chars[bbb]);
    builder.appendCharacter(chars[ccc]);
    builder.appendCharacter(chars[ddd]);

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
  builder.appendCharacter(chars[aaa]);

  // Duplicating some tail handling to try to do less jumps.
  if (strLength - i == 1) {
    uint8_t b00 = aaab << 4 & 0x3f;
    builder.appendCharacter(chars[b00]);
    if (!omitPadding) {
      builder.appendCharacter('=');
      builder.appendCharacter('=');
    }
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
  builder.appendCharacter(chars[bbb]);
  builder.appendCharacter(chars[cc0]);
  if (!omitPadding) {
    builder.appendCharacter('=');
  }
  return true;
}

template bool base64Encode(
    llvh::ArrayRef<char> str,
    StringBuilder &builder,
    bool useBase64url,
    bool omitPadding);
template bool base64Encode(
    llvh::ArrayRef<char16_t> str,
    StringBuilder &builder,
    bool useBase64url,
    bool omitPadding);
template bool base64Encode(
    llvh::ArrayRef<uint8_t> str,
    StringBuilder &builder,
    bool useBase64url,
    bool omitPadding);

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
  } else if (strLength % 4 == 1) {
    // If strLength divides by 4 leaving a remainder of 1, then this is for sure
    // not a valid base64 encoded data because there cannot be 3 '=' padding
    // characters.
    return llvh::None;
  } else {
    // Otherwise, the data could be padded with '=' and still be valid. In this
    // case, the padding characters are not included, but we need to process as
    // if they are there.
    uint32_t simulatedPadding = 4 - (strLength % 4);
    strLength += simulatedPadding;
    numPadding += simulatedPadding;
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

    // Check for '=' in the middle
    if (LLVM_UNLIKELY(c == '=')) {
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

//===----------------------------------------------------------------------===//
// TC39 FromBase64 Algorithm
// https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
//===----------------------------------------------------------------------===//

Base64DecodeResult fromBase64(
    StringView input,
    bool useBase64url,
    LastChunkHandling lastChunkHandling,
    uint32_t maxLength) {
  Base64DecodeResult result;

  // Step 3: If maxLength is 0, return immediately.
  if (maxLength == 0) {
    return result;
  }

  uint32_t inputLen = input.length();

  // Accumulate characters in a chunk of 4.
  uint32_t chunkBuf[4];
  uint32_t chunkLen = 0;
  uint32_t read = 0;
  uint32_t index = 0;

  // Spec uses "Repeat" (infinite loop with explicit returns).
  for (;;) {
    // Step 10.1: Skip ASCII whitespace.
    while (index < inputLen && isWhitespace(input[index]))
      ++index;

    // Step 10.2: End of string — handle partial chunk.
    if (index == inputLen) {
      if (chunkLen == 0) {
        result.read = read;
        return result;
      }
      if (lastChunkHandling == LastChunkHandling::StopBeforePartial) {
        result.read = read;
        return result;
      }
      if (chunkLen == 1) {
        result.error = true;
        result.errorIndex = index;
        result.read = read;
        return result;
      }
      if (lastChunkHandling == LastChunkHandling::Strict) {
        result.error = true;
        result.errorIndex = index;
        result.read = read;
        return result;
      }
      // Loose mode: decode the partial chunk. Non-zero padding bits
      // are silently ignored per spec (throwOnExtraBits is false).
      if (chunkLen == 2) {
        uint32_t n = (chunkBuf[0] << 6) | chunkBuf[1];
        result.bytes.push_back(static_cast<uint8_t>(n >> 4));
      } else {
        // chunkLen == 3
        uint32_t n = (chunkBuf[0] << 12) | (chunkBuf[1] << 6) | chunkBuf[2];
        result.bytes.push_back(static_cast<uint8_t>(n >> 10));
        result.bytes.push_back(static_cast<uint8_t>((n >> 2) & 0xFF));
      }
      result.read = index;
      return result;
    }

    char16_t c = input[index];

    // Step 10.5: Handle padding character '='.
    if (c == '=') {
      if (chunkLen < 2) {
        // '=' at chunkLen 0 or 1 is always an error.
        result.error = true;
        result.errorIndex = index;
        result.read = read;
        return result;
      }
      // Skip whitespace after first '='.
      ++index;
      while (index < inputLen && isWhitespace(input[index]))
        ++index;
      if (chunkLen == 2) {
        // Need second '=' for chunkLen==2.
        if (index >= inputLen || input[index] != '=') {
          // Incomplete padding. In stop-before-partial mode,
          // return what we have without error.
          if (lastChunkHandling == LastChunkHandling::StopBeforePartial) {
            result.read = read;
            return result;
          }
          result.error = true;
          result.errorIndex = index;
          result.read = read;
          return result;
        }
        // Skip whitespace after second '='.
        ++index;
        while (index < inputLen && isWhitespace(input[index]))
          ++index;
      }
      // Must be at end of string now (trailing garbage is always an error).
      if (index < inputLen) {
        result.error = true;
        result.errorIndex = index;
        result.read = read;
        return result;
      }
      // Decode the padded chunk. Only reject non-zero padding bits
      // in strict mode (throwOnExtraBits per spec).
      bool throwOnExtraBits = lastChunkHandling == LastChunkHandling::Strict;
      if (chunkLen == 2) {
        if (throwOnExtraBits && (chunkBuf[1] & 0x0F)) {
          result.error = true;
          result.errorIndex = index;
          result.read = read;
          return result;
        }
        uint32_t n = (chunkBuf[0] << 6) | chunkBuf[1];
        result.bytes.push_back(static_cast<uint8_t>(n >> 4));
      } else {
        // chunkLen == 3
        if (throwOnExtraBits && (chunkBuf[2] & 0x03)) {
          result.error = true;
          result.errorIndex = index;
          result.read = read;
          return result;
        }
        uint32_t n = (chunkBuf[0] << 12) | (chunkBuf[1] << 6) | chunkBuf[2];
        result.bytes.push_back(static_cast<uint8_t>(n >> 10));
        result.bytes.push_back(static_cast<uint8_t>((n >> 2) & 0xFF));
      }
      result.read = index;
      return result;
    }

    // Step 10.6-10.7: Validate base64 character.
    int val = decodeBase64Char(c, useBase64url);
    if (val < 0) {
      result.error = true;
      result.errorIndex = index;
      result.read = read;
      return result;
    }

    // Step 10.8-10.9: Check remaining capacity BEFORE accumulating.
    uint32_t remaining = maxLength - result.bytes.size();
    if ((remaining == 1 && chunkLen == 2) ||
        (remaining == 2 && chunkLen == 3)) {
      result.read = read;
      return result;
    }

    // Step 10.10-10.11: Accumulate character into chunk.
    chunkBuf[chunkLen++] = static_cast<uint32_t>(val);
    ++index;

    // Step 10.12: Decode complete 4-character chunk.
    if (chunkLen == 4) {
      uint32_t n = (chunkBuf[0] << 18) | (chunkBuf[1] << 12) |
          (chunkBuf[2] << 6) | chunkBuf[3];
      result.bytes.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
      result.bytes.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
      result.bytes.push_back(static_cast<uint8_t>(n & 0xFF));
      chunkLen = 0;
      read = index;
      // Step 10.12.5: Exit if we've reached maxLength.
      if (result.bytes.size() == maxLength) {
        result.read = read;
        return result;
      }
    }
  }
}

} // namespace vm
} // namespace hermes
