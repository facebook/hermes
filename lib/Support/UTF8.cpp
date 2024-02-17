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

/// The following logic is a combination of ES14 11.1.4 CodePointAt() and
/// what https://infra.spec.whatwg.org/#strings says about what to do with
/// singular surrogates: "To convert a string into a scalar value string,
/// replace any surrogates with U+FFFD." Therefore, if we encounter any lone
/// surrogate, replace the value with UNICODE_REPLACEMENT_CHARACTER (U+FFFD).
/// The result of this process is that the enclosing for-loop processes only
/// scalar values (aka a code point that is not a surrogate).
/// \param cur Iterator pointing to the current character
/// \param end Iterator pointing to the end of the string
/// \return std::pair with first element being the Unicode code point, and the
///         second being how many code point units were consumed
static std::pair<char32_t, size_t> convertToCodePointAt(
    llvh::ArrayRef<char16_t>::iterator cur,
    llvh::ArrayRef<char16_t>::iterator end) {
  char16_t c = cur[0];
  if (isLowSurrogate(c)) {
    // Unpaired low surrogate.
    return {UNICODE_REPLACEMENT_CHARACTER, 1};
  } else if (isHighSurrogate(c)) {
    // Leading high surrogate. See if the next character is a low surrogate.
    if (cur + 1 == end || !isLowSurrogate(cur[1])) {
      // Trailing or unpaired high surrogate.
      return {UNICODE_REPLACEMENT_CHARACTER, 1};
    } else {
      // Decode surrogate pair and increment, because we consumed two chars.
      return {utf16SurrogatePairToCodePoint(c, cur[1]), 2};
    }
  } else {
    // Not a surrogate.
    return {c, 1};
  }
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
  auto cur = input.begin(), end = input.end();
  for (; cur < end && currNumCharacters < maxCharacters;
       ++cur, ++currNumCharacters) {
    char16_t c = cur[0];
    // ASCII fast-path.
    if (LLVM_LIKELY(c <= 0x7F)) {
      out.push_back(static_cast<char>(c));
      continue;
    }

    auto [c32, inputConsumed] = convertToCodePointAt(cur, end);
    cur += (inputConsumed - 1);

    // The code point to be encoded here is guaranteed to be a valid unicode
    // code point and not a surrogate. Because of the convertToCodePointAt()
    // process.
    std::array<char, UTF8CodepointMaxBytes> buff;
    char *ptr = buff.data();
    encodeUTF8(ptr, c32);
    out.insert(out.end(), buff.data(), ptr);
  }
  return cur == end;
}

std::pair<uint32_t, uint32_t> convertUTF16ToUTF8BufferWithReplacements(
    llvh::MutableArrayRef<uint8_t> outBuffer,
    llvh::ArrayRef<char16_t> input) {
  uint32_t numRead = 0;
  uint32_t numWritten = 0;
  uint8_t *writtenPtr = outBuffer.begin();
  auto end = input.end();
  for (auto cur = input.begin(); cur < end; ++cur) {
    char16_t c = cur[0];
    // ASCII fast-path.
    if (LLVM_LIKELY(c <= 0x7F)) {
      if (numWritten + 1 > outBuffer.size()) {
        break;
      }
      *writtenPtr = static_cast<char>(c);
      writtenPtr++;
      numWritten++;
      numRead++;
      continue;
    }

    auto [c32, inputConsumed] = convertToCodePointAt(cur, end);
    cur += (inputConsumed - 1);

    // The code point to be encoded here is guaranteed to be a valid unicode
    // code point and not a surrogate. Because of the convertToCodePointAt()
    // process.
    std::array<char, UTF8CodepointMaxBytes> buff;
    char *ptr = buff.data();
    encodeUTF8(ptr, c32);

    size_t convertedLength = ptr - buff.data();
    if (numWritten + convertedLength > outBuffer.size()) {
      break;
    }
    std::memcpy(writtenPtr, buff.data(), convertedLength);
    writtenPtr += convertedLength;
    numWritten += convertedLength;
    numRead += inputConsumed;
  }

  return {numRead, numWritten};
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

void convertUTF8WithSurrogatesToUTF8WithReplacements(
    std::string &output,
    llvh::StringRef input) {
  // Temporary buffer to convert input to UTF16.
  llvh::SmallVector<char16_t, 8> ustr;
  // The temporary string cannot possibly have more characters than input. Using
  // reserve avoids default-initializing the elements.
  ustr.reserve(input.size());
  // ustr is pessimistically allocated, so pass a raw pointer to its contents to
  // convertUTF8WithSurrogatesToUTF16. The alternative is to use a
  // std::back_inserter that would resize the buffer if needed. It turns out
  // that the resulting code using std::back_inserter is considerably larger.
  char16_t *ustrEnd =
      convertUTF8WithSurrogatesToUTF16(ustr.data(), input.begin(), input.end());
  // Sanity-check for buffer overflow.
  assert(
      static_cast<uintptr_t>(ustrEnd - ustr.data()) <= ustr.capacity() &&
      "buffer overflow while converting UTF8 surrogates");
  // Now convert the UTF16 string to UTF8 without surrogates.
  convertUTF16ToUTF8WithReplacements(
      output, llvh::makeArrayRef(ustr.data(), ustrEnd));
}

} // namespace hermes
