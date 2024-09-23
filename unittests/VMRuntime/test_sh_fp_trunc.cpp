/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/sh_fp_trunc.h"

#include <cstdint>

#include "gtest/gtest.h"

namespace {

TEST(test_sh_fp_trunc, SmokeTest) {
  ASSERT_EQ(100, _sh_trunc_f64_to_i32(100.0));
  ASSERT_EQ(100, _sh_trunc_f64_to_u32(100.0));
  ASSERT_EQ(UINT32_MAX, _sh_trunc_f64_to_u32((double)UINT32_MAX));
}

} // namespace
