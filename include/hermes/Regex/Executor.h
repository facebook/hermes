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

/// Entry point for searching a string via regex compiled bytecode.
/// Given the bytecode \p bytecode, search the range starting at \p first up to
/// (not including) \p last with the flags \p matchFlags. If the search
/// succeeds, poopulate MatchResults with the capture groups. \return true if
/// some portion of the string matched the regex represented by the bytecode,
/// false otherwise.
/// char16_t overload.
MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char16_t *first,
    const char16_t *last,
    MatchResults<const char16_t *> &m,
    constants::MatchFlagType matchFlags);

/// ASCII overload.
MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char *first,
    const char *last,
    MatchResults<const char *> &m,
    constants::MatchFlagType matchFlags);

} // namespace regex
} // namespace hermes

#endif
