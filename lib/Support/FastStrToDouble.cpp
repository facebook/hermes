/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/FastStrToDouble.h"
#include "fast_float/fast_float.h"

namespace hermes {

OptValue<double> fastStrToDouble(llvh::ArrayRef<char> numView) {
  double result;
  fast_float::from_chars_result_t<char> res = fast_float::from_chars(
      numView.data(), numView.data() + numView.size(), result);
  // If the error code matches the default value of std::errc(), then that means
  // no error occurred.
  if (res.ec == std::errc()) {
    return result;
  }
  return llvh::None;
}

OptValue<double> fastStrToDouble(llvh::ArrayRef<char16_t> numView) {
  double result;
  fast_float::from_chars_result_t<char16_t> res = fast_float::from_chars(
      numView.data(), numView.data() + numView.size(), result);
  // If the error code matches the default value of std::errc(), then that means
  // no error occurred.
  if (res.ec == std::errc()) {
    return result;
  }
  return llvh::None;
}

} // namespace hermes
