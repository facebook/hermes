/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

/// \return the canonicalized value of \p cp, following ES9 21.2.2.8.2.
uint32_t canonicalize(uint32_t cp, bool unicode);

class CodePointSet;
/// \return a set containing all characters which are canonically equivalent to
/// any character in \p set, following ES9 21.2.2.8.2.
CodePointSet makeCanonicallyEquivalent(const CodePointSet &set, bool unicode);

} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_CHARACTERPROPERTIES_H
