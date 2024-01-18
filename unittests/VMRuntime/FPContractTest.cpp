/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

namespace {

/// Test that Hermes is compiled such that floating point multiplication and
/// addition in separate expressions cannot be combined into a single FMA
/// operation with higher intermediate precision. For instance, GCC uses
/// -ffp-contract=fast by default, which allows separate expressions to be
/// contracted. This is important because it is sometimes required by the JS
/// spec, for instance when computing dates.
TEST(FPContractTest, SeparateExpressionsFMA) {
  double d0 = 213503982335.0 * 86400000.0;
  double d1 = d0 - 18446744073709552000.0;
  EXPECT_EQ(d1, 34447360.0);
}

} // namespace
