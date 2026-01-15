/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/FastStrToDouble.h"
#include "fast_float/fast_float.h"

#include <limits>

namespace hermes {
namespace {

template <typename CharT>
StrToDoubleParseResult<CharT> fastStrToDoubleImpl(
    llvh::ArrayRef<CharT> numView) {
  double result = std::numeric_limits<double>::quiet_NaN();
  auto *first = numView.data();
  auto *last = first + numView.size();
  fast_float::parse_options_t<CharT> options;
  options.format = fast_float::chars_format::general |
      fast_float::chars_format::allow_leading_plus;
  // When res.ec is result_out_of_range we simply ignore it and keep `result` as
  // whatever fast_float wrote (+/-inf or 0).
  auto res = fast_float::from_chars_advanced(first, last, result, options);
  bool invalidArgument = res.ec == std::errc::invalid_argument;
  return {res.ptr, invalidArgument, result};
}

} // namespace

Char8StrToDoubleParseResult fastStrToDouble(llvh::ArrayRef<char> numView) {
  return fastStrToDoubleImpl(numView);
}

Char16StrToDoubleParseResult fastStrToDouble(llvh::ArrayRef<char16_t> numView) {
  return fastStrToDoubleImpl(numView);
}

} // namespace hermes
