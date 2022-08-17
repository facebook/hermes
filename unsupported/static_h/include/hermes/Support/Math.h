/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MATH_H
#define HERMES_SUPPORT_MATH_H

#include <cmath>

namespace hermes {

/// ES9.0 12.6.4
/// Perform the exponentiation operation on doubles required by the JS spec.
inline double expOp(double x, double y) {
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();

  // Handle special cases that std::pow handles differently.
  if (std::isnan(y)) {
    return nan;
  } else if (y == 0) {
    return 1;
  } else if (std::abs(x) == 1 && std::isinf(y)) {
    return nan;
  }

  // std::pow handles the other edge cases as the ES9.0 spec requires.
  return std::pow(x, y);
}

} // namespace hermes

#endif
