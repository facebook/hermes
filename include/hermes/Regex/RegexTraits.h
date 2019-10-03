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
  static char_type canonicalize(char_type c) {
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
    return hermes::canonicalize(c);
  }

  /// \return whether the character c is contained within the range [first,
  /// last]. If ICase is set, perform a canonicalizing membership test as
  /// specified in "CharacterSetMatcher" ES5.1 15.10.2.8.
  bool rangesContain(llvm::ArrayRef<BracketRange16> ranges, char16_t c) const {
    return anyRangeContainsChar(ranges, c);
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

  static char canonicalize(char c) {
    if ('a' <= c && c <= 'z')
      c &= ~('a' ^ 'A'); // toupper
    return c;
  }

  /// \return whether any of a list of ranges contains \p c.
  /// Note that our ranges contain char16_t, but we test chars for membership.
  bool rangesContain(llvm::ArrayRef<BracketRange16> ranges, char16_t c) const {
    return anyRangeContainsChar(ranges, c);
  }
};

} // end namespace regex
} // end namespace hermes

#endif
