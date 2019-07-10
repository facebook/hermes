/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// Regex traits appropriate for Hermes regex.
//===----------------------------------------------------------------------===//

#ifndef HERMES_REGEX_TRAITS_H
#define HERMES_REGEX_TRAITS_H

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/PlatformUnicode.h"
#include "hermes/Regex/Compiler.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"

namespace hermes {
namespace regex {

enum CaseCanonicalization { CaseLower, CaseUpper };

/// \return the character \p ch converted to lowercase or uppercase, subject
/// to the requirements of ES5.1 15.10.2.8. Specifically a non-ASCII character
/// may not canonicalize to an ASCII character, and eszett does not
/// canonicalize to Z.
inline char16_t canonicalizeToCase(
    char16_t ch,
    CaseCanonicalization whichCase) {
  // Fast ASCII path.
  if (ch <= 127) {
    if (whichCase == CaseLower && 'A' <= ch && ch <= 'Z') {
      return ch | 32;
    } else if (whichCase == CaseUpper && 'a' <= ch && ch <= 'z') {
      return ch & ~32;
    } else {
      return ch;
    }
  }

  // "Let u be ch converted to upper case as if by calling the standard
  // built-in method String.prototype.toUpperCase on the one-character String
  // ch"
  llvm::SmallVector<char16_t, 2> u = {ch};
  auto caseConv = whichCase == CaseLower
      ? platform_unicode::CaseConversion::ToLower
      : platform_unicode::CaseConversion::ToUpper;
  platform_unicode::convertToCase(u, caseConv, false /* useCurrentLocale */);

  // "If u does not consist of a single character, return ch."
  if (u.size() != 1)
    return ch;

  // "Let cu be u's character"
  char16_t cu = u[0];

  // "If ch's code unit value is greater than or equal to decimal 128 and
  // cu's code unit value is less than decimal 128, then return ch"
  // (Note we checked for ch <= 127 above.)
  assert(ch >= 128 && "ch should be >= 128");
  if (cu < 128)
    return ch;

  // "Return cu."
  return cu;
}

/// \return whether any range in \p ranges contains the character \p c,
/// inclusive of both ends.
inline bool anyRangeContainsChar(
    llvm::ArrayRef<BracketRange16> ranges,
    char16_t c) {
  for (const auto &r : ranges) {
    if (r.start <= c && c <= r.end) {
      return true;
    }
  }
  return false;
}

/// Implementation of regex::Traits for char16_t
struct U16RegexTraits {
 private:
  // A DenseMapInfo for our canonicalization cache.
  // Use 0 and 1 as empty and tombstone; we skip the cache for ASCII but wish to
  // support 0xFFFF and 0xFFFE as keys.
  struct CanonicalizationDenseMapInfo {
    static inline char16_t getEmptyKey() {
      return 0;
    }
    static inline char16_t getTombstoneKey() {
      return 1;
    }

    static unsigned getHashValue(char16_t c) {
      return llvm::DenseMapInfo<uint16_t>::getHashValue(c);
    }

    static bool isEqual(char16_t lhs, char16_t rhs) {
      return lhs == rhs;
    }
  };

  using CanonicalizeCache =
      llvm::SmallDenseMap<uint16_t, uint16_t, 16, CanonicalizationDenseMapInfo>;
  mutable CanonicalizeCache toLowerCache_;
  mutable CanonicalizeCache toUpperCache_;

  /// ES5.1 7.2
  static bool isWhiteSpaceChar(char16_t c) {
    return c == u'\u0009' || c == u'\u000B' || c == u'\u000C' ||
        c == u'\u0020' || c == u'\u00A0' || c == u'\uFEFF' || c == u'\u1680' ||
        (c >= u'\u2000' && c <= u'\u200A') || c == u'\u202F' ||
        c == u'\u205F' || c == u'\u3000';
  }

  /// ES5.1 7.3
  static bool isLineTerminatorChar(char16_t c) {
    return c == u'\u000A' || c == u'\u000D' || c == u'\u2028' || c == u'\u2029';
  }

  // Optimized variant of canonicalizeToCaseSlowPath. Convert \p ch to lowecase
  // or uppercase, storing cached values in \p cache.
  char16_t cachingCanonicalizeToCase(
      char16_t ch,
      CaseCanonicalization whichCase) const {
    // canonicalizeToCase has a fast ASCII path.
    // Note that we use 0 and 1 as sentinel values in the map so they must not
    // be placed in the cache.
    if (ch <= 127) {
      return canonicalizeToCase(ch, whichCase);
    }

    // Read from the cache by inserting a bogus value of zero. If the cache
    // probe is a miss, we'll populate it.
    CanonicalizeCache &cache =
        whichCase == CaseLower ? toLowerCache_ : toUpperCache_;
    auto emplaced = cache.try_emplace(ch, 0);
    auto &cachedChar = emplaced.first->second;
    if (emplaced.second) {
      cachedChar = canonicalizeToCase(ch, whichCase);
    }
    return cachedChar;
  }

  /// Given a list of inclusive ranges, a character \p c, and its
  /// canonicalized form \p cc, return whether any character that canonicalizes
  /// to cc is contained within the range. The caller will have already checked
  /// both c and cc (which may be the same character). This implements
  /// ES5.1 15.10.2.8 "CharacterSetMatcher".
  bool rangeContainsPrecanonicalizedForm(
      llvm::ArrayRef<BracketRange16> ranges,
      char16_t cc,
      char16_t c) const {
    // Check special cases. These are all the cases where simple case mapping is
    // not sufficient, that is, where the lowercase and uppercase variants are
    // not in 1-1 correspondence. Examples include dotted capital I, Greek
    // sigma, etc.
    if (const auto *pclist = hermes::getExceptionalPrecanonicalizations(cc)) {
      // 0 is used to indicate a missing entry.
      for (auto pc : *pclist) {
        if (pc != 0 && anyRangeContainsChar(ranges, pc)) {
          return true;
        }
      }
      return false;
    } else {
      // If we get here, then the only character which may match is the
      // lowercase form. The caller already checked c and cc, so we can skip the
      // check for those.
      char16_t lowc = cachingCanonicalizeToCase(cc, CaseLower);
      return lowc != c && lowc != cc && anyRangeContainsChar(ranges, lowc);
    }
  }

 public:
  using char_type = char16_t;
  using CharT = char_type;

  /// \return whether the character \p c has the character type \p type.
  bool characterHasType(char_type c, regex::CharacterClass::Type type) const {
    switch (type) {
      case regex::CharacterClass::Digits:
        return u'0' <= c && c <= u'9';
      case regex::CharacterClass::Spaces:
        return isWhiteSpaceChar(c) || isLineTerminatorChar(c);
      case regex::CharacterClass::Words:
        return (u'a' <= c && c <= u'z') || (u'A' <= c && c <= u'Z') ||
            (u'0' <= c && c <= u'9') || (c == u'_');
    }
    llvm_unreachable("Unknown character type");
  }

  /// \return the case-insensitive equivalence key for \p c.
  /// Our implementation follows ES5.1 15.10.2.8.
  char_type caseFold(char_type c) const {
    static_assert(
        std::numeric_limits<char_type>::min() == 0,
        "char_type must be unsigned");
    if (c <= 127) {
      // ASCII fast path. Uppercase by clearing bit 5.
      if ('a' <= c && c <= 'z') {
        c &= ~(1 << 5);
      }
      return c;
    }

    // Canonicalize (i.e. uppercase) the 1-character string.
    return cachingCanonicalizeToCase(c, CaseUpper);
  }

  /// \return whether the character c is contained within the range [first,
  /// last]. If ICase is set, perform a canonicalizing membership test as
  /// specified in "CharacterSetMatcher" ES5.1 15.10.2.8.
  template <bool ICase>
  bool rangesContain(llvm::ArrayRef<BracketRange16> ranges, char16_t c) const {
    if (anyRangeContainsChar(ranges, c)) {
      return true;
    }
    if (ICase) {
      // Canonicalize and check for membership in the range again, if the
      // character changed.
      char16_t cc = cachingCanonicalizeToCase(c, CaseUpper);
      if (cc != c && anyRangeContainsChar(ranges, cc))
        return true;
      // Check for other pre-canonicalizations of cc.
      return rangeContainsPrecanonicalizedForm(ranges, cc, c);
    }
    return false;
  }
};

/// Implementation of regex::Traits for 7-bit ASCII.
struct ASCIIRegexTraits {
  using char_type = char;

  bool characterHasType(char c, regex::CharacterClass::Type type) const {
    switch (type) {
      case regex::CharacterClass::Digits:
        return '0' <= c && c <= '9';
      case regex::CharacterClass::Spaces:
        switch (c) {
          case ' ':
          case '\t':
          case '\r':
          case '\n':
          case '\v':
          case '\f':
            return true;
          default:
            return false;
        }
      case regex::CharacterClass::Words:
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') || (c == '_');
    }
    llvm_unreachable("Unknown character type");
  }

  static char caseFold(char c) {
    if ('a' <= c && c <= 'z')
      c &= ~('a' ^ 'A'); // toupper
    return c;
  }

  /// \return whether any of a list of ranges contains \p c.
  /// Note that our ranges contain char16_t, but we test chars for membership.
  template <bool ICase>
  bool rangesContain(llvm::ArrayRef<BracketRange16> ranges, char16_t c) const {
    if (anyRangeContainsChar(ranges, c))
      return true;
    if (ICase) {
      // We need to do a case-insensitive comparison.
      // We need to check if toupper(c) is equal to toupper of any character in
      // our range [first_, last_]. This can't be done naively, for example,
      // calling toupper on the endpoints of the range [0-a] would result in the
      // range [_-A] which is invalid. Instead we simply check if c is a letter
      // and, if so, whether its other case is in the range.
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
        c ^= 0x20; // flip case!
        return anyRangeContainsChar(ranges, c);
      }
    }
    return false;
  }
};

} // end namespace regex
} // end namespace hermes

#endif
