/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// -*- C++ -*-
//===--------------------------- regex ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef HERMES_REGEX_TYPES_H
#define HERMES_REGEX_TYPES_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallString.h"

namespace hermes {
namespace regex {
namespace constants {

// MatchFlagType

enum MatchFlagType {
  /// Default match options.
  matchDefault = 0,

  /// ^ anchors should not treat the input start as a line start.
  matchNotBeginningOfLine = 1 << 0,

  /// $ anchors should not treat the input end as a line end.
  matchNotEndOfLine = 1 << 1,

  /// Hint that the input is composed entirely of ASCII characters.
  matchInputAllAscii = 1 << 2,

  /// Do not search for a match past the search start location.
  matchOnlyAtStart = 1 << 3,
};

inline constexpr MatchFlagType operator~(MatchFlagType x) {
  return MatchFlagType(~int(x) & 0x0FFF);
}

inline constexpr MatchFlagType operator&(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) & int(y));
}

inline constexpr MatchFlagType operator|(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) | int(y));
}

inline constexpr MatchFlagType operator^(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) ^ int(y));
}

inline MatchFlagType &operator&=(MatchFlagType &x, MatchFlagType y) {
  x = x & y;
  return x;
}

inline MatchFlagType &operator|=(MatchFlagType &x, MatchFlagType y) {
  x = x | y;
  return x;
}

inline MatchFlagType &operator^=(MatchFlagType &x, MatchFlagType y) {
  x = x ^ y;
  return x;
}

enum class ErrorType {
  /// No error occurred.
  None,

  /// An escaped value would overflow: /\xFFFFFFFFFFF/
  EscapeOverflow,

  /// incomplete escape: new RegExp("\\")
  EscapeIncomplete,

  /// Invalid escape: new RegExp("\\123", "u")
  EscapeInvalid,

  /// Mismatched [ and ].
  UnbalancedBracket,

  /// Mismatched ( and ).
  UnbalancedParenthesis,

  /// Braces have valid syntax, but the range is invalid, such as {5,3}.
  BraceRange,

  /// Invalid character range, such as [b-a].
  CharacterRange,

  /// A lone { or } was found.
  InvalidQuantifierBracket,

  /// One of *?+{ was not preceded by a valid regular expression.
  InvalidRepeat,

  /// The pattern exceeded internal limits, such as capture group or loop count.
  PatternExceedsParseLimits,

  /// The flags supplied were either invalid or contained repetition.
  InvalidFlags,
};

/// \return an error message for the given \p error.
inline const char *messageForError(ErrorType error) {
  switch (error) {
    case ErrorType::EscapeOverflow:
      return "Escaped value too large";
    case ErrorType::EscapeIncomplete:
      return "Incomplete escape";
    case ErrorType::EscapeInvalid:
      return "Invalid escape";
    case ErrorType::UnbalancedBracket:
      return "Character class not closed";
    case ErrorType::UnbalancedParenthesis:
      return "Parenthesized expression not closed";
    case ErrorType::BraceRange:
      return "Quantifier range out of order";
    case ErrorType::CharacterRange:
      return "Character class range out of order";
    case ErrorType::InvalidQuantifierBracket:
      return "Invalid quantifier bracket";
    case ErrorType::InvalidRepeat:
      return "Quantifier has nothing to repeat";
    case ErrorType::PatternExceedsParseLimits:
      return "Pattern exceeds parse limits";
    case ErrorType::InvalidFlags:
      return "Invalid flags";
    case ErrorType::None:
      return "No error";
  }
  llvm_unreachable("Unknown error");
  return nullptr;
}

/// Maximum number of supported capture groups.
/// This is therefore also the maximum valid backreference.
constexpr uint16_t kMaxCaptureGroupCount = 65535;

/// Maximum number of supported loops.
constexpr uint16_t kMaxLoopCount = 65535;

} // namespace constants

/// After compiling a regex, there are certain properties we can test for that
/// enable us to quickly rule out matches. We refer to these as
/// MatchConstraints: they constrain the strings that may match the regex.
enum MatchConstraintFlags : uint8_t {
  /// If set, ASCII strings can never match because we require at least one
  /// non-ASCII character.
  MatchConstraintNonASCII = 1 << 0,

  /// If set, the regex can only match at the beginning of the input string, due
  /// to ^ anchors.
  MatchConstraintAnchoredAtStart = 1 << 1,

  /// If set, the regex cannot possibly match an empty string, e.g. /a/
  MatchConstraintNonEmpty = 1 << 2,
};

/// \return whether a code point \p cp is ASCII.
inline bool isASCII(uint32_t c) {
  return c <= 127;
}

// Type wrapping up a character class, like \d or \S.
struct CharacterClass {
  enum Type : uint8_t {
    Digits = 1 << 0, // \d \D
    Spaces = 1 << 1, // \s \S
    Words = 1 << 2, // \w \W
  } type_;

  // Whether the class is inverted (\D instead of \d).
  bool inverted_;

  CharacterClass(Type type, bool invert) : type_(type), inverted_(invert) {}
};

// Struct representing flags which may be used when constructing the RegExp
class SyntaxFlags {
 private:
  // Note these are encoded into bytecode files, so changing their values is a
  // breaking change.
  enum : uint8_t {
    ICASE = 1 << 0,
    GLOBAL = 1 << 1,
    MULTILINE = 1 << 2,
    UCODE = 1 << 3,
    DOTALL = 1 << 4,
    STICKY = 1 << 5
  };

 public:
  // Note: Preserving the order here and ensuring it lines up with the order of
  // the offsets above makes the generated assembly more efficient.
  // Specifically, it makes the conversion to/from a byte almost free.
  uint8_t ignoreCase : 1;
  uint8_t global : 1;
  uint8_t multiline : 1;
  uint8_t unicode : 1;
  uint8_t dotAll : 1;
  uint8_t sticky : 1;

  /// \return a byte representing the flags. Bits are set based on the offsets
  /// specified above. This is used for serialising the flags to bytecode.
  uint8_t toByte() const {
    uint8_t ret = 0;
    if (global)
      ret |= GLOBAL;
    if (ignoreCase)
      ret |= ICASE;
    if (multiline)
      ret |= MULTILINE;
    if (unicode)
      ret |= UCODE;
    if (sticky)
      ret |= STICKY;
    if (dotAll)
      ret |= DOTALL;
    return ret;
  }

  /// \return a SyntaxFlags struct generated from the given byte. Bit offsets
  /// are specified above. This is used for deserialising the flags from
  /// bytecode.
  static SyntaxFlags fromByte(uint8_t byte) {
    SyntaxFlags ret = {};
    if (byte & GLOBAL)
      ret.global = 1;
    if (byte & ICASE)
      ret.ignoreCase = 1;
    if (byte & MULTILINE)
      ret.multiline = 1;
    if (byte & UCODE)
      ret.unicode = 1;
    if (byte & STICKY)
      ret.sticky = 1;
    if (byte & DOTALL)
      ret.dotAll = 1;
    return ret;
  }

  /// \return a string representing the flags
  /// The characters are returned in the order given in ES 6 21.2.5.3
  /// (specifically global, ignoreCase, multiline, unicode, sticky)
  /// Note this may differ in order from the string passed in construction
  llvh::SmallString<6> toString() const {
    llvh::SmallString<6> result;
    if (global)
      result.push_back('g');
    if (ignoreCase)
      result.push_back('i');
    if (multiline)
      result.push_back('m');
    if (unicode)
      result.push_back('u');
    if (sticky)
      result.push_back('y');
    if (dotAll)
      result.push_back('s');
    return result;
  }

  /// Given a flags string \p str, generate the corresponding SyntaxFlags
  /// \return the flags if the string is valid, an empty optional otherwise
  /// See ES 5.1 15.10.4.1 for description of the validation
  static llvh::Optional<SyntaxFlags> fromString(
      const llvh::ArrayRef<char16_t> flags) {
    // A flags string may contain i,m,g, in any order, but at most once each
    auto error = llvh::NoneType::None;
    SyntaxFlags ret = {};
    for (auto c : flags) {
      switch (c) {
        case u'i':
          if (ret.ignoreCase)
            return error;
          ret.ignoreCase = 1;
          break;
        case u'm':
          if (ret.multiline)
            return error;
          ret.multiline = 1;
          break;
        case u'g':
          if (ret.global)
            return error;
          ret.global = 1;
          break;
        case u'u':
          if (ret.unicode)
            return error;
          ret.unicode = 1;
          break;
        case u'y':
          if (ret.sticky)
            return error;
          ret.sticky = 1;
          break;
        case u's':
          if (ret.dotAll)
            return error;
          ret.dotAll = 1;
          break;
        default:
          return error;
      }
    }
    return ret;
  }
};
} // namespace regex
} // namespace hermes
#endif // HERMES_REGEX_TYPES_H
