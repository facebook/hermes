/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Base64.h"

namespace hermes {

// A lookup table that map (Base64-encoded) Ascii back to binary.
static constexpr unsigned char decMap[128] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64,
    64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64};

llvh::Optional<std::string> base64Decode(llvh::StringRef input) {
  size_t inputLen = input.size();

  // Approximate output length.
  // Every four Ascii are converted back to 3 bytes.
  size_t outputLen = inputLen / 4 * 3;

  // Adjust output length according to padding.
  if (inputLen % 4 == 0) {
    // Multiples of four are treated as padded so trailing '=' are excluded.
    if (input[inputLen - 1] == '=')
      outputLen--;
    if (input[inputLen - 2] == '=')
      outputLen--;
  } else {
    // Otherwise, treat as unpadded and calculate length via reminder.
    switch (inputLen % 4) {
      case 1:
        return llvh::None;
      case 2:
        outputLen++;
        break;
      case 3:
        outputLen += 2;
        break;
    }
  }

  // Pre-allocate fixed length string to be faster.
  std::string output(outputLen, '\0');

  /// Iterate over the \p input, decode every \c c into a sextet and store into
  /// a buffer \c buf of capacity 32 bits. \c bufSize is maintained to track
  /// how many bits are actually buffered.
  unsigned buf = 0;
  unsigned bufSize = 0;
  size_t i = 0;
  for (const unsigned char c : input) {
    if (LLVM_UNLIKELY(c > 127))
      return llvh::None;

    if (LLVM_UNLIKELY(c == '=')) {
      // Errors on '=' in the middle.
      if (i < outputLen) {
        return llvh::None;
      } else {
        break;
      }
    }

    unsigned char sextet = decMap[c];
    if (LLVM_UNLIKELY(sextet >= 64)) {
      return llvh::None;
    } else {
      // Making rooms for the new sextet.
      buf = (buf << 6) + sextet;
      bufSize += 6;

      // Once buffer is filled over a byte, evacuate a byte to the output.
      if (bufSize >= 8) {
        output[i++] = (char)(buf >> (bufSize - 8));
        bufSize -= 8;
      }
    }
  }

  return output;
}

llvh::Optional<std::string> parseJSONBase64DataURL(llvh::StringRef url) {
  // data:
  if (!url.consume_front("data:"))
    return llvh::None;

  // [<mediatype>]
  // This is limited to JSON at this moment.
  if (!url.consume_front("application/json"))
    return llvh::None;

  // [;base64],
  // This is required to be presented in our case.
  if (!url.consume_front(";base64,"))
    return llvh::None;

  // <data>
  return base64Decode(url);
}

} // namespace hermes
