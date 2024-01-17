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

} // namespace vm
} // namespace hermes
