/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_REGEX_EXECUTOR_H
#define HERMES_REGEX_EXECUTOR_H

#include "hermes/Regex/RegexBytecode.h"
#include "hermes/Regex/RegexTypes.h"

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

/// A constant used inside a capture group to indicate that the capture group
/// did not match.
constexpr uint32_t kNotMatched = UINT32_MAX;

/// The maximum number of times we will backtrack.
constexpr uint32_t kBacktrackLimit = 1u << 30;

/// A CapturedRange represents a range of the input string captured by a capture
/// group. A CaptureGroup may also not have matched, in which case its start is
/// set to kNotMatched. Note that an unmatched capture group is different than a
/// capture group that matched an empty string.
struct CapturedRange {
  /// Index of the first captured character, or kNotMatched if not matched.
  uint32_t start;

  /// One past the index of the last captured character.
  uint32_t end;

  /// \return whether this range was a successful match.
  bool matched() const {
    return start != kNotMatched;
  }
};

/// Given a string \p first with length \p length, look for regex matches
/// starting at offset \p start. We must have 0 <= start <= length.
/// Search using the compiled regex represented by \p bytecode with the flags \p
/// matchFlags. If the search succeeds, populate \p captures with the capture
/// groups.
/// \return true if some portion of the string matched the regex represented by
/// the bytecode, false otherwise.
/// This is the char16_t overload.
MatchRuntimeResult searchWithBytecode(
    llvh::ArrayRef<uint8_t> bytecode,
    const char16_t *first,
    uint32_t start,
    uint32_t length,
    std::vector<CapturedRange> *captures,
    constants::MatchFlagType matchFlags);

/// This is the ASCII overload.
MatchRuntimeResult searchWithBytecode(
    llvh::ArrayRef<uint8_t> bytecode,
    const char *first,
    uint32_t start,
    uint32_t length,
    std::vector<CapturedRange> *captures,
    constants::MatchFlagType matchFlags);

} // namespace regex
} // namespace hermes

#endif
