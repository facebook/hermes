/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace hermes {

/// The result of converting a double to its decimal components.
struct DoubleDecimalComponents {
  bool negative;
  unsigned long long significand;
  int exponent;
};

/// Convert a double into its decimal significand and exponent components, and
/// sign information.
/// \param m double to convert. It must be a finite nonzero number.
DoubleDecimalComponents fastDoubleToDecimal(double m);

} // namespace hermes
