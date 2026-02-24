/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/FastDoubleToDecimal.h"

#include "dragonbox/dragonbox.h"

#include <cmath>

namespace hermes {

DoubleDecimalComponents fastDoubleToDecimal(double m) {
  assert(std::isfinite(m) && "non finite number not allowed");
  assert(m != 0 && "0 is not a finite nonzero number");
  auto dec = jkj::dragonbox::to_decimal(m);
  return {dec.is_negative, dec.significand, dec.exponent};
}

} // namespace hermes
