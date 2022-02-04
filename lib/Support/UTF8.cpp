/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/UTF8.h"

namespace hermes {

void encodeUTF8(char *&dst, uint32_t cp) {
  char *d = dst;
  if (cp <= 0x7F) {
    *d = (char)cp;
    ++d;
  } else if (cp <= 0x7FF) {
    d[1] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[0] = (cp & 0x1F) | 0xC0;
    d += 2;
  } else if (cp <= 0xFFFF) {
    d[2] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[1] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[0] = (cp & 0x0F) | 0xE0;
    d += 3;
  } else if (cp <= 0x1FFFFF) {
    d[3] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[2] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[1] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[0] = (cp & 0x07) | 0xF0;
    d += 4;
  } else if (cp <= 0x3FFFFFF) {
    d[4] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[3] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[2] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[1] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[0] = (cp & 0x03) | 0xF8;
    d += 5;
  } else {
    d[5] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[4] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[3] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[2] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[1] = (cp & 0x3F) | 0x80;
    cp >>= 6;
    d[0] = (cp & 0x01) | 0xFC;
    d += 6;
  }
  dst = d;
}

bool convertUTF16ToUTF8WithReplacements(
    std::string &out,
    llvh::ArrayRef<char16_t> input,
    size_t maxCharacters) {
  out.clear();
  out.reserve(input.size());
  // Stop early if we've reached currNumCharacters worth of UTF-8 characters.
  size_t currNumCharacters = 0;
  if (!maxCharacters) {
    // Condition checks are easier if this number is set to the max value.
    maxCharacters = std::numeric_limits<size_t>::max();
  }
  for (auto cur = input.begin(), end = input.end();
       cur < end && currNumCharacters < maxCharacters;
       ++cur, ++currNumCharacters) {
    char16_t c = cur[0];
    // ASCII fast-path.
    if (LLVM_LIKELY(c <= 0x7F)) {
      out.push_back(static_cast<char>(c));
      continue;
    }

    char32_t c32;
    if (isLowSurrogate(cur[0])) {
      // Unpaired low surrogate.
      c32 = UNICODE_REPLACEMENT_CHARACTER;
    } else if (isHighSurrogate(cur[0])) {
      // Leading high surrogate. See if the next character is a low surrogate.
      if (cur + 1 == end || !isLowSurrogate(cur[1])) {
        // Trailing or unpaired high surrogate.
        c32 = UNICODE_REPLACEMENT_CHARACTER;
      } else {
        // Decode surrogate pair and increment, because we consumed two chars.
        c32 = decodeSurrogatePair(cur[0], cur[1]);
        ++cur;
      }
    } else {
      // Not a surrogate.
      c32 = c;
    }

    char buff[UTF8CodepointMaxBytes];
    char *ptr = buff;
    encodeUTF8(ptr, c32);
    out.insert(out.end(), buff, ptr);
  }
  return currNumCharacters < maxCharacters;
}

void convertUTF16ToUTF8WithSingleSurrogates(
    std::string &dest,
    llvh::ArrayRef<char16_t> input) {
  dest.clear();
  dest.reserve(input.size());
  for (char16_t c : input) {
    // ASCII fast-path.
    if (LLVM_LIKELY(c <= 0x7F)) {
      dest.push_back(static_cast<char>(c));
      continue;
    }
    char32_t c32 = c;
    char buff[UTF8CodepointMaxBytes];
    char *ptr = buff;
    encodeUTF8(ptr, c32);
    dest.insert(dest.end(), buff, ptr);
  }
}

bool isAllASCII(const uint8_t *start, const uint8_t *end) {
  const uint8_t *cursor = start;
  size_t len = end - start;
  static_assert(
      sizeof(uint32_t) == 4 && alignof(uint32_t) <= 4,
      "uint32_t must be 4 bytes and cannot be more than 4 byte aligned");

  if (len >= 4) {
    // Step by 1s until aligned for uint32_t.
    uint8_t mask = 0;
    while ((uintptr_t)cursor % alignof(uint32_t)) {
      mask |= *cursor++;
      len -= 1;
    }
    if (mask & 0x80u) {
      return false;
    }

    // Now that we are aligned, step by 4s.
    while (len >= 4) {
      uint32_t val = *(const uint32_t *)cursor;
      if (val & 0x80808080u) {
        return false;
      }
      cursor += 4;
      len -= 4;
    }
  }
  assert(len < 4 && "Length should now be less than 4");
  uint8_t mask = 0;
  while (len--) {
    mask |= *cursor++;
  }
  if (mask & 0x80u)
    return false;
  return true;
}

} // namespace hermes
