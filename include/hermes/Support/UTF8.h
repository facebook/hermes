/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_UTF8_H
#define HERMES_SUPPORT_UTF8_H

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/Twine.h"
#include "llvh/Support/Compiler.h"

#include <cstddef>

namespace hermes {

/// Maximum number of bytes in a valid UTF-8 codepoint
constexpr size_t UTF8CodepointMaxBytes = 6;

/// Encode a unicode code point as a UTF-8 sequence of bytes.
void encodeUTF8(char *&dst, uint32_t cp);

/// Check whether a byte is a regular ASCII or a UTF8 starting byte.
/// \return true if it is UTF8 starting byte.
inline bool isUTF8Start(char ch) {
  return (ch & 0x80) != 0;
}

/// \return true if this is a UTF-8 leading byte.
inline bool isUTF8LeadingByte(char ch) {
  return (ch & 0xC0) == 0xC0;
}

/// \return true if this is a UTF-8 continuation byte, or in other words, this
/// is a byte in the "middle" of a UTF-8 codepoint.
inline static bool isUTF8ContinuationByte(char ch) {
  return (ch & 0xC0) == 0x80;
}

/// \return true if this is a pure ASCII char sequence.
template <typename Iter>
inline bool isAllASCII(Iter begin, Iter end) {
  while (begin < end) {
    if (*begin < 0 || *begin > 127)
      return false;
    ++begin;
  }
  return true;
}

/// Overload for char* and uint8_t*.
bool isAllASCII(const uint8_t *start, const uint8_t *end);

inline bool isAllASCII(const char *start, const char *end) {
  return isAllASCII((const uint8_t *)start, (const uint8_t *)end);
}

/// Decode a sequence of UTF8 encoded bytes when it is known that the first byte
/// is a start of an UTF8 sequence.
/// \tparam allowSurrogates when false, values in the surrogate range are
///     reported as errors
template <bool allowSurrogates, typename F>
uint32_t _decodeUTF8SlowPath(const char *&from, F error) {
  uint32_t ch = (uint32_t)from[0];
  uint32_t result;

  assert(isUTF8Start(ch));

  if (LLVM_LIKELY((ch & 0xE0) == 0xC0)) {
    uint32_t ch1 = (uint32_t)from[1];
    if (LLVM_UNLIKELY((ch1 & 0xC0) != 0x80)) {
      from += 1;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }

    from += 2;
    result = ((ch & 0x1F) << 6) | (ch1 & 0x3F);
    if (LLVM_UNLIKELY(result <= 0x7F)) {
      error("Non-canonical UTF-8 encoding");
      return UNICODE_REPLACEMENT_CHARACTER;
    }

  } else if (LLVM_LIKELY((ch & 0xF0) == 0xE0)) {
    uint32_t ch1 = (uint32_t)from[1];
    if (LLVM_UNLIKELY((ch1 & 0x40) != 0 || (ch1 & 0x80) == 0)) {
      from += 1;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    uint32_t ch2 = (uint32_t)from[2];
    if (LLVM_UNLIKELY((ch2 & 0x40) != 0 || (ch2 & 0x80) == 0)) {
      from += 2;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    from += 3;
    result = ((ch & 0x0F) << 12) | ((ch1 & 0x3F) << 6) | (ch2 & 0x3F);
    if (LLVM_UNLIKELY(result <= 0x7FF)) {
      error("Non-canonical UTF-8 encoding");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    if (LLVM_UNLIKELY(
            result >= UNICODE_SURROGATE_FIRST &&
            result <= UNICODE_SURROGATE_LAST && !allowSurrogates)) {
      error("Invalid UTF-8 code point 0x" + llvh::Twine::utohexstr(result));
      return UNICODE_REPLACEMENT_CHARACTER;
    }

  } else if ((ch & 0xF8) == 0xF0) {
    uint32_t ch1 = (uint32_t)from[1];
    if (LLVM_UNLIKELY((ch1 & 0x40) != 0 || (ch1 & 0x80) == 0)) {
      from += 1;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    uint32_t ch2 = (uint32_t)from[2];
    if (LLVM_UNLIKELY((ch2 & 0x40) != 0 || (ch2 & 0x80) == 0)) {
      from += 2;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    uint32_t ch3 = (uint32_t)from[3];
    if (LLVM_UNLIKELY((ch3 & 0x40) != 0 || (ch3 & 0x80) == 0)) {
      from += 3;
      error("Invalid UTF-8 continuation byte");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    from += 4;
    result = ((ch & 0x07) << 18) | ((ch1 & 0x3F) << 12) | ((ch2 & 0x3F) << 6) |
        (ch3 & 0x3F);
    if (LLVM_UNLIKELY(result <= 0xFFFF)) {
      error("Non-canonical UTF-8 encoding");
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    if (LLVM_UNLIKELY(result > UNICODE_MAX_VALUE)) {
      error("Invalid UTF-8 code point 0x" + llvh::Twine::utohexstr(result));
      return UNICODE_REPLACEMENT_CHARACTER;
    }

  } else {
    from += 1;
    error("Invalid UTF-8 lead byte 0x" + llvh::Twine::utohexstr((uint8_t)ch));
    return UNICODE_REPLACEMENT_CHARACTER;
  }

  return result;
}

/// Scans back from \p ptr until the start of the previous UTF-8 codepoint.
/// Logically, this is equivalent to `--ptr` in the codepoint space.
/// It could be a regular ASCII character, or a multi-byte encoded character.
/// This function assumes that the input is valid!
inline const char *previousUTF8Start(const char *ptr) {
  --ptr;
  // If the previous codepoint is ASCII, we are done.
  if (!(*ptr & 0x80))
    return ptr;
  // Scan backwards until we find a leading byte (11xxxxxx)
  while ((*ptr & 0xC0) != 0xC0)
    --ptr;
  return ptr;
}

/// Decode a sequence of UTF8 encoded bytes into a Unicode codepoint.
/// In case of decoding errors, the provided callback is invoked with an
/// apropriate messsage and UNICODE_REPLACEMENT_CHARACTER is returned.
///
/// \tparam allowSurrogates when false, values in the surrogate range are
///     reported as errors
/// \param error callback invoked with an error message
/// \return the codepoint
template <bool allowSurrogates, typename F>
inline uint32_t decodeUTF8(const char *&from, F error) {
  if (LLVM_LIKELY((*from & 0x80) == 0)) // Ordinary ASCII?
    return *from++;

  return _decodeUTF8SlowPath<allowSurrogates>(from, error);
}

/// Encode a 32-bit value, into UTF16. If the value is a part of a surrogate
/// pair, it is encoded without any conversion.
template <typename OutIt>
inline void encodeUTF16(OutIt &dest, uint32_t cp) {
  if (LLVM_LIKELY(cp < 0x10000)) {
    *dest = (uint16_t)cp;
    ++dest; // Use pre-increment in case this is an iterator.
  } else {
    assert(cp <= UNICODE_MAX_VALUE && "invalid Unicode value");
    cp -= 0x10000;
    *dest = UTF16_HIGH_SURROGATE + ((cp >> 10) & 0x3FF);
    ++dest;
    *dest = UTF16_LOW_SURROGATE + (cp & 0x3FF);
    ++dest;
  }
}

/// Decode a UTF-8 sequence, which is assumed to be valid, but may possibly
/// contain explicitly encoded surrogate pairs, into a UTF-16 sequence.
/// \return the updated destination iterator
template <typename OutIt>
inline OutIt convertUTF8WithSurrogatesToUTF16(
    OutIt dest,
    const char *begin8,
    const char *end8) {
  while (begin8 < end8)
    encodeUTF16(dest, decodeUTF8<true>(begin8, [](const llvh::Twine &) {
                  llvm_unreachable("invalid UTF-8");
                }));
  return dest;
}

/// Convert a UTF-16 encoded string \p input to UTF-8 stored in \p dest,
/// encoding each surrogate halves individually into UTF-8.
/// This is the inverse function of convertUTF8WithSurrogatesToUTF16.
/// Note the result is not valid utf-8 if it contains surrogate values.
/// Only use it to get the internal representation of utf-8 strings in hermes
/// compiler.
void convertUTF16ToUTF8WithSingleSurrogates(
    std::string &dest,
    llvh::ArrayRef<char16_t> input);

/// Convert a UTF-16 encoded string \p input to UTF-8 stored in \p dest,
/// replacing unpaired surrogates halves with the Unicode replacement character.
/// \param maxCharacters If non-zero, the maximum number of characters to
///   convert.
/// \return false if the string was truncated, true if the whole string was
///   written out successfully.
bool convertUTF16ToUTF8WithReplacements(
    std::string &dest,
    llvh::ArrayRef<char16_t> input,
    size_t maxCharacters = 0);

} // namespace hermes

#endif // HERMES_SUPPORT_UTF8_H
