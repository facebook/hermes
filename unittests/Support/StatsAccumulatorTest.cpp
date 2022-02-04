/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/Support/StatsAccumulator.h"

namespace {

TEST(StatsAccumulatorTest, Test10EqualDiff) {
  ::hermes::StatsAccumulator<int32_t> stats;
  for (unsigned i = 0; i < 10; i++) {
    stats.record(90);
    stats.record(110);
  }
  EXPECT_EQ(20, stats.count());
  EXPECT_EQ(90.0, stats.min());
  EXPECT_EQ(110.0, stats.max());
  EXPECT_EQ(20 * 100, stats.sum());
  EXPECT_EQ(100.0, stats.average());
  EXPECT_EQ(((110.0 * 110.0) + (90.0 * 90.0)) * 10.0, stats.sumOfSquares());
  // Every sample is 10 from the mean, so that should be the standard deviation.
  EXPECT_EQ(10.0, stats.stddev());
}

} // namespace
