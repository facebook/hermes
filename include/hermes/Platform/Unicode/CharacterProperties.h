/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_PLATFORMUNICODE_CHARACTERPROPERTIES_H
#define HERMES_PLATFORMUNICODE_CHARACTERPROPERTIES_H

#include <cassert>
#include <cstdint>

namespace hermes {

const uint32_t UNICODE_MAX_VALUE = 0x10FFFF;
/// The start of the surrogate range.
const uint32_t UNICODE_SURROGATE_FIRST = 0xD800;
/// The last character of the surrogate range (inclusive).
const uint32_t UNICODE_SURROGATE_LAST = 0xDFFF;
const uint32_t UTF16_HIGH_SURROGATE = 0xD800;
const uint32_t UTF16_LOW_SURROGATE = 0xDC00;
const uint32_t UNICODE_REPLACEMENT_CHARACTER = 0xFFFD;
/// The last member of the BMP.
const uint32_t UNICODE_LAST_BMP = 0xFFFF;

const uint32_t UNICODE_LINE_SEPARATOR = 0x2028;
const uint32_t UNICODE_PARAGRAPH_SEPARATOR = 0x2029;

const uint32_t UNICODE_ZWNJ = 0x200C;
const uint32_t UNICODE_ZWJ = 0x200D;

/// The maximum number of precanonicalizations of any character.
/// Precanonicalization is not a term from the Unicode spec; rather it refers to
/// the RegExp Canonicalize function given in ES5.1 15.10.2.8. Most characters
/// are either canonicalized to by themselves or their lowercase variant;
/// there's a handful of exceptions which are tracked here.
const uint32_t UNICODE_MAX_PRECANONICALIZATIONS = 3;

inline bool isValidCodePoint(uint32_t cp) {
  return !(
      (cp >= UNICODE_SURROGATE_FIRST && cp <= UNICODE_SURROGATE_LAST) ||
      cp > UNICODE_MAX_VALUE);
}

/// \return whether \p cp is part of the Basic Multilingual Plane.
/// Surrogate characters are considered part of the BMP.
inline bool isMemberOfBMP(uint32_t cp) {
  return cp <= UNICODE_LAST_BMP;
}

/// \return whether cp is a high surrogate.
inline bool isHighSurrogate(uint32_t cp) {
  return UNICODE_SURROGATE_FIRST <= cp && cp < UTF16_LOW_SURROGATE;
}

/// \return whether cp is a low surrogate.
inline bool isLowSurrogate(uint32_t cp) {
  return UTF16_LOW_SURROGATE <= cp && cp <= UNICODE_SURROGATE_LAST;
}

/// Decode a surrogate pair [\p hi, \p lo] into a code point.
inline uint32_t decodeSurrogatePair(uint32_t hi, uint32_t lo) {
  assert(isHighSurrogate(hi) && isLowSurrogate(lo) && "Not a surrogate pair");
  return ((hi - UTF16_HIGH_SURROGATE) << 10) + (lo - UTF16_LOW_SURROGATE) +
      0x10000;
}

/// \return true if the codepoint is not ASCII and is a Unicode letter.
bool isUnicodeOnlyLetter(uint32_t cp);
/// \return true if the codepoint is not ASCII and is a Unicode space.
bool isUnicodeOnlySpace(uint32_t cp);
/// \return true if the codepoint is in the Non-Spacing Mark or
/// Combining-Spacing Mark categories.
bool isUnicodeCombiningMark(uint32_t cp);
/// \return true if the codepoint is in the Decimal Number category.
bool isUnicodeDigit(uint32_t cp);
/// \return true if the codepoint is in the Connector Punctuation category.
bool isUnicodeConnectorPunctuation(uint32_t cp);

/// PrecanonicalizationList is a pointer to an array of precanonicalization
/// forms. A 0 value indicates no entry. We use uint16 instead of uint32 because
/// RegExp is cast in terms of 16-bit UCS-2, and because there are no
/// exceptional precanonicalizations outside of the BMP (that is, all astral
/// character canonicalizations use simple case mapping).
using PrecanonicalizationList = uint16_t[UNICODE_MAX_PRECANONICALIZATIONS];

/// \return a pointer to the exceptional precanonicalization list for a given
/// character, or nullptr if the character's precanonicalization is given by its
/// case mapping.
const PrecanonicalizationList *getExceptionalPrecanonicalizations(uint16_t cp);

}; // namespace hermes

#endif // HERMES_PLATFORMUNICODE_CHARACTERPROPERTIES_H
