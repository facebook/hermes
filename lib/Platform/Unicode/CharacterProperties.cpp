/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Platform/Unicode/CharacterProperties.h"

#include <algorithm>
#include <climits>
#include <iterator>
#include <utility>

namespace hermes {

#include "UnicodeData.inc"

namespace {
struct UnicodeRangeComp {
  bool operator()(UnicodeRange p, uint32_t s) const {
    return p.second < s;
  }
  bool operator()(uint32_t s, UnicodeRange p) const {
    return s < p.first;
  }
};
} // namespace

template <typename UnicodeRangeTable>
inline bool lookup(const UnicodeRangeTable &table, const uint32_t cp) {
  return std::binary_search(
      std::begin(table), std::end(table), cp, UnicodeRangeComp());
}

bool isUnicodeOnlyLetter(uint32_t cp) {
  // "any character in the Unicode categories “Uppercase letter (Lu)”,
  // “Lowercase letter (Ll)”, “Titlecase letter (Lt)”, “Modifier letter (Lm)”,
  // “Other letter (Lo)”, or “Letter number (Nl)”".
  // ASCII characters are not "UnicodeOnly" and so we return false.
  if (cp <= 0x7F)
    return false;

  return lookup(UNICODE_LETTERS, cp);
}

// Special cased due to small number of separate values.
bool isUnicodeOnlySpace(uint32_t cp) {
  // "Other category “Zs”: Any other Unicode “space separator”"
  // Exclude ASCII.
  if (cp <= 0x7F)
    return false;

  switch (cp) {
    case 0xa0:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200a:
    case 0x202f:
    case 0x205f:
    case 0x3000:
      return true;
    default:
      return false;
  }
}

bool isUnicodeCombiningMark(uint32_t cp) {
  // "any character in the Unicode categories “Non-spacing mark (Mn)” or
  // “Combining spacing mark (Mc)”"
  return lookup(UNICODE_COMBINING_MARK, cp);
}

bool isUnicodeDigit(uint32_t cp) {
  // "any character in the Unicode category “Decimal number (Nd)”"
  // 0-9 is the common case.
  return (cp >= '0' && cp <= '9') || lookup(UNICODE_DIGIT, cp);
};

bool isUnicodeConnectorPunctuation(uint32_t cp) {
  // "any character in the Unicode category “Connector punctuation (Pc)"
  // _ is the common case.
  return cp == '_' || lookup(UNICODE_CONNECTOR_PUNCTUATION, cp);
}

// Predicate used to enable binary search on mappings.
static bool operator<(const UnicodePrecanonicalizationMapping &m, uint16_t cp) {
  return m.canonicalized < cp;
}

const PrecanonicalizationList *getExceptionalPrecanonicalizations(uint16_t cp) {
  auto iter = std::lower_bound(
      std::begin(UNICODE_PRECANONS), std::end(UNICODE_PRECANONS), cp);
  if (iter != std::end(UNICODE_PRECANONS) && iter->canonicalized == cp)
    return &iter->forms;
  return nullptr;
}

}; // namespace hermes
