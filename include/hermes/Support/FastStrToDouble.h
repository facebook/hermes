/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_FAST_STR_TO_DOUBLE
#define HERMES_SUPPORT_FAST_STR_TO_DOUBLE

#include "llvh/ADT/ArrayRef.h"

namespace hermes {

template <typename CharT>
struct StrToDoubleParseResult {
  /// Position where parsing stopped.
  /// On success, points one past the last character of given input.
  /// On error, points to the first character that caused parsing to stop.
  CharT const *ptr;

  /// True if the input was invalid (no valid number prefix detected).
  /// Note that we do not treat overflow or rounding to 0 as errors.
  /// For example, a string input "1e-400" will output a value of 0, and
  /// invalidArgument as false. Similarly, 1e400 is yielded as 'infinity' with
  /// invalidArgument as false.
  bool invalidArgument;

  /// The parsed double value.
  double value;

  /// \return true if the parse operation was a success.
  constexpr explicit operator bool() const noexcept {
    return !invalidArgument;
  }
};

using Char8StrToDoubleParseResult = StrToDoubleParseResult<char>;
using Char16StrToDoubleParseResult = StrToDoubleParseResult<char16_t>;

/// Parse a string into a double. This parses until the first non-number
/// character is reached. If there was no number prefix detected at all, then
/// invalidArgument is set to true. For the string "123.456.789", the function
/// would return:
/// { ptr = addr of second period, invalidArgument = false, value = 123.456 }
///
/// \param numView the string contents to parse.
/// \return struct describing the result of the parse operation.
Char8StrToDoubleParseResult fastStrToDouble(llvh::ArrayRef<char> numView);

/// Overload for char16_t
Char16StrToDoubleParseResult fastStrToDouble(llvh::ArrayRef<char16_t> numView);

} // namespace hermes

#endif // HERMES_SUPPORT_FAST_STR_TO_DOUBLE
