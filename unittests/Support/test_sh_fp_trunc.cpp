/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/sh_tryfast_fp_cvt.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace {

TEST(test_sh_fp_trunc, TryFastF64ToI32) {
  int32_t result;

  // In range
  double x = 123.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i32(x, result));
  EXPECT_EQ(result, 123);

  // Out of range
  x = 1e10;
  EXPECT_FALSE(sh_tryfast_f64_to_i32(x, result));

  x = -123.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i32(x, result));
  EXPECT_EQ(result, -123);

  x = 0.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i32(x, result));
  EXPECT_EQ(result, 0);

  x = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_i32(x, result));

  x = -std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_i32(x, result));
}

TEST(test_sh_fp_trunc, TryFastF64ToU32) {
  uint32_t result;

  // In range
  double x = 123.0;
  EXPECT_TRUE(sh_tryfast_f64_to_u32(x, result));
  EXPECT_EQ(result, 123);

  // Out of range (negative value)
  x = -1.0;
  EXPECT_FALSE(sh_tryfast_f64_to_u32(x, result));

  // Large positive value that fits into uint32_t but not int32_t
  x = 0xFFFFFFFF;
  EXPECT_TRUE(sh_tryfast_f64_to_u32(x, result));
  EXPECT_EQ(result, std::numeric_limits<uint32_t>::max());

  // Large positive value that doesn't fit into uint32_t
  x = 1e10;
  EXPECT_FALSE(sh_tryfast_f64_to_u32(x, result));

  x = 0.0;
  EXPECT_TRUE(sh_tryfast_f64_to_u32(x, result));
  EXPECT_EQ(result, 0);

  x = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_u32(x, result));

  x = -std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_u32(x, result));
}

TEST(test_sh_fp_trunc, TryFastF64ToI64) {
  int64_t result;

  // In range
  double x = 1234567890123.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i64(x, result));
  EXPECT_EQ(result, 1234567890123);

  // Out of range
  x = 1e20; // A value that is out of range for int64_t
  EXPECT_FALSE(sh_tryfast_f64_to_i64(x, result));

  // Negative value
  x = -4611686141884235776.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i64(x, result));
  EXPECT_EQ(result, -4611686141884235776);

  x = 0.0;
  EXPECT_TRUE(sh_tryfast_f64_to_i64(x, result));
  EXPECT_EQ(result, 0);

  x = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_i64(x, result));

  x = -std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_i64(x, result));
}

TEST(test_sh_fp_trunc, TryFastF64ToU64) {
  uint64_t result;

  // In range
  double x = 1234567890123.0;
  EXPECT_TRUE(sh_tryfast_f64_to_u64(x, result));
  EXPECT_EQ(result, 1234567890123);

  // Out of range (negative value)
  x = -1.0;
  EXPECT_FALSE(sh_tryfast_f64_to_u64(x, result));

  // Large positive value.
  x = 0x7FFF000000000000ull;
  EXPECT_TRUE(sh_tryfast_f64_to_u64(x, result));
  EXPECT_EQ(result, 0x7FFF000000000000ull);

  // Large positive value that doesn't fit into uint64_t
  x = 1e20;
  EXPECT_FALSE(sh_tryfast_f64_to_u64(x, result));

  x = 0.0;
  EXPECT_TRUE(sh_tryfast_f64_to_u64(x, result));
  EXPECT_EQ(result, 0);

  x = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_u64(x, result));

  x = -std::numeric_limits<double>::infinity();
  EXPECT_FALSE(sh_tryfast_f64_to_u64(x, result));
}
} // namespace
