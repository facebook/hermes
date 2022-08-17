/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Regex traits appropriate for Hermes regex.
//===----------------------------------------------------------------------===//

#ifndef HERMES_REGEX_TRAITS_H
#define HERMES_REGEX_TRAITS_H

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/PlatformUnicode.h"
#include "hermes/Regex/RegexBytecode.h"
#include "hermes/Regex/RegexTypes.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/Optional.h"

namespace hermes {
namespace regex {

/// \return whether any range in \p ranges contains the character \p c,
/// inclusive of both ends.
inline bool anyRangeContainsChar(
    llvh::ArrayRef<BracketRange32> ranges,
    uint32_t c) {
  for (const auto &r : ranges) {
    if (r.start <= c && c <= r.end) {
      return true;
    }
  }
  return false;
}

/// Implementation of regex::Traits for UTF-16.
struct UTF16RegexTraits {
  /// A CodePoint is a 24-bit Unicode code point.
  using CodePoint = uint32_t;

  /// A CodeUnit is either a CodePoint or half of a UTF-16 surrogate pair.
  using CodeUnit = char16_t;

 private:
  using CanonicalizeCache = llvh::SmallDenseMap<CodePoint, CodePoint, 16>;
  mutable CanonicalizeCache toUpperCache_;

  /// ES9 11.2
  static bool isWhiteSpaceChar(CodePoint c) {
    return c == u'\u0009' || c == u'\u000B' || c == u'\u000C' ||
        c == u'\u0020' || c == u'\u00A0' || c == u'\uFEFF' || c == u'\u1680' ||
        (c >= u'\u2000' && c <= u'\u200A') || c == u'\u202F' ||
        c == u'\u205F' || c == u'\u3000';
  }

  /// ES9 11.3
  static bool isLineTerminatorChar(CodePoint c) {
    return c == u'\u000A' || c == u'\u000D' || c == u'\u2028' || c == u'\u2029';
  }

 public:
  /// \return whether the character \p c has the character type \p type.
  bool characterHasType(CodePoint c, regex::CharacterClass::Type type) const {
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

  /// ES9 21.2.2.8.2
  /// \return the canonicalized form of \p c, following either the unicode or
  /// non-unicode algorithm according to \p unicode.
  static CodePoint canonicalize(CodePoint c, bool unicode) {
    static_assert(
        std::numeric_limits<CodePoint>::min() == 0,
        "CodePoint must be unsigned");
    // If we are unicode, we want to case-fold, which is effectively lowercase
    // form. If we are non-Unicode, we need the uppercase form.
    if (LLVM_LIKELY(c <= 127)) {
      // ASCII fast path.
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
        // It's a letter. Make it lowercase (uppercase) if Unicode is set
        // (unset).
        c &= ~(1u << 5); // uppercase
        c |= ((uint32_t)unicode << 5); // lowercase if Unicode is set.
      }
      return c;
    }
    return hermes::canonicalize(c, unicode);
  }

  /// \return whether the character c is contained within the range [first,
  /// last]. If ICase is set, perform a canonicalizing membership test as
  /// specified in "CharacterSetMatcher" ES5.1 15.10.2.8.
  bool rangesContain(llvh::ArrayRef<BracketRange32> ranges, CodePoint c) const {
    return anyRangeContainsChar(ranges, c);
  }
};

/// Implementation of regex::Traits for 7-bit ASCII.
struct ASCIIRegexTraits {
  /// CodePoint and CodeUnits are both 7-bit ASCII values.
  using CodePoint = uint8_t;
  using CodeUnit = char;

  bool characterHasType(CodePoint c, regex::CharacterClass::Type type) const {
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

  /// ES6 21.2.2.8.2
  /// The Unicode path is lowercase; the non-Unicode path is uppercase.
  static CodePoint canonicalize(CodePoint c, bool unicode) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      // It's a letter. Make it lowercase (uppercase) if Unicode is set (unset).
      c &= ~(1u << 5); // uppercase
      c |= ((uint32_t)unicode << 5); // lowercase if Unicode is set.
    }
    return c;
  }

  /// \return whether any of a list of ranges contains \p c.
  /// Note that our ranges contain uint32_t, but we test chars for membership.
  bool rangesContain(llvh::ArrayRef<BracketRange32> ranges, char16_t c) const {
    return anyRangeContainsChar(ranges, c);
  }
};

} // end namespace regex
} // end namespace hermes

#endif
