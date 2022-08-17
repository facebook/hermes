/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _WINDOWS
#include "hermes/VM/instrumentation/ApproxIntegral.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <functional>

using namespace hermes::vm;

namespace {

/// Test that with more samples, the estimate of
///
///     \int_{0}^{hi} f(x) dx
///
/// Converges on \p expected.
void testErrorConverges(
    int64_t hi,
    int64_t expected,
    std::function<int64_t(int64_t)> f);

TEST(ApproxIntegralTest, Empty) {
  ApproxIntegral ai;
  EXPECT_EQ(0, ai.area());
}

TEST(ApproxIntegralTest, Square) {
  ApproxIntegral ai;

  // +
  // |          (10, 10)
  // +---------+
  // |XXXXXXXXX|
  // |XXXXXXXXX|
  // |XXXXXXXXX|
  // +---------+--+

  ai.push(0, 10);
  ai.push(10, 10);
  EXPECT_EQ(100, ai.area());
}

TEST(ApproxIntegralTest, Rect) {
  ApproxIntegral ai;

  // +
  // |          (6, 7)
  // +---------+
  // |XXXXXXXXX|
  // |XXXXXXXXX|
  // |XXXXXXXXX|
  // |XXXXXXXXX|
  // +---------+--+

  ai.push(0, 7);
  ai.push(6, 7);
  EXPECT_EQ(42, ai.area());
}

TEST(ApproxIntegralTest, Wedge) {
  ApproxIntegral ai;

  // +
  // |             (7, 6)
  // |            +
  // |        ..""|
  // |    ..""XXXX|
  // |..""XXXXXXXX|
  // +------------+--+

  ai.push(7, 6);
  EXPECT_EQ(21, ai.area());
}

/// The areas of order 2+ polynomials cannot be approximated exactly.  In these
/// cases, the approximation should converge on the true value, as the number of
/// samples passed to the accumulator increases.
TEST(ApproxIntegralTest, ErrorConvergence) {
  // A quadratic with a positive second differential (happy face).
  testErrorConverges(6, 72, [](int64_t x) { return x * x - 6 * x + 18; });

  // A quadratic with a negative second differential (sad face).
  testErrorConverges(6, 144, [](int64_t x) { return -x * x + 6 * x + 18; });
}

/// When the estimated area is not a whole number, its value is rounded down.
/// When the the area of the estimate's constituent trapezoids are not
/// necessarily whole, the contribution of integer rounding error to the overall
/// error term should not grow with the number of samples, but should remain
/// constant.
TEST(ApproxIntegralTest, NoRoundingError) {
  ApproxIntegral ai;

  // +
  // |      (x, x)
  // |     +
  // |   ."|
  // |."XXX|
  // +-----+--+
  //
  // x in {3, 6, 9, 12, 15}

  ai.push(3, 3);
  EXPECT_EQ(4, ai.area());

  ai.push(6, 6);
  EXPECT_EQ(18, ai.area());

  ai.push(9, 9);
  EXPECT_EQ(40, ai.area());

  ai.push(12, 12);
  EXPECT_EQ(72, ai.area());

  ai.push(15, 15);
  EXPECT_EQ(112, ai.area());
}

void testErrorConverges(
    int64_t hi,
    int64_t expected,
    std::function<int64_t(int64_t)> f) {
  static unsigned samples[] = {2, 3, 4, 7};
  std::vector<int64_t> absError;

  for (auto count : samples) {
    ApproxIntegral ai;
    int64_t step = hi / (count - 1);

    for (int64_t x = 0; x <= hi; x += step) {
      ai.push(x, f(x));
    }

    absError.push_back(llabs(ai.area() - expected));
  }

  EXPECT_TRUE(std::is_sorted(
      absError.begin(), absError.end(), [](int64_t l, int64_t r) {
        return l > r;
      }));
}

} // namespace
#endif // not _WINDOWS
