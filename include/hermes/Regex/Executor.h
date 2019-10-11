/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_REGEX_EXECUTOR_H
#define HERMES_REGEX_EXECUTOR_H

#include "hermes/Regex/Compiler.h"

// This file contains the machinery for executing a regexp compiled to bytecode.

namespace hermes {
namespace regex {

/// The result of trying to find a match.
enum class MatchRuntimeResult {
  /// Match found.
  Match,

  /// No match found.
  NoMatch,

  /// Stack overflow during match attempt.
  StackOverflow,

};

template <class BidirectionalIterator>
class SubMatch {
 public:
  typedef typename iterator_traits<BidirectionalIterator>::value_type CharT;
  typedef basic_string<CharT> StringT;

  BidirectionalIterator first{};
  BidirectionalIterator second{};
  bool matched = false;

  size_t length() const {
    return matched ? distance(this->first, this->second) : 0;
  }
  StringT str() const {
    return matched ? StringT(this->first, this->second) : StringT();
  }
};

/// Given a string \p first with length \p length, look for regex matches
/// starting at offset \p start. We must have 0 <= start <= length.
/// Search using the compiled regex represented by \p bytecode with the flags \p
/// matchFlags. If the search succeeds, populate \p m with the capture
/// groups.
/// \return true if some portion of the string matched the regex represented by
/// the bytecode, false otherwise.
/// This is the char16_t overload.
MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char16_t *first,
    uint32_t start,
    uint32_t length,
    MatchResults<const char16_t *> &m,
    constants::MatchFlagType matchFlags);

/// This is the ASCII overload.
MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char *first,
    uint32_t start,
    uint32_t length,
    MatchResults<const char *> &m,
    constants::MatchFlagType matchFlags);

} // namespace regex
} // namespace hermes

#endif
