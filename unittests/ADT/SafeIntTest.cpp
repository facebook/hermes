/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/ADT/SafeInt.h"

namespace {

using namespace hermes;

// This test takes a long time if compiled without optimizations.
#ifdef NDEBUG
TEST(SafeIntTest, Overflow) {
  SafeUInt32 safe;
  uint64_t a = (1ull << 32) - 1;
  uint64_t b = (1ull << 32) + 2;
  // The product will overflow into a value that fits in 32 bits.
  ASSERT_LT(a * b, 1ull << 32);
  while (b--) {
    safe.add(static_cast<uint32_t>(a));
  }
  EXPECT_TRUE(safe.isOverflowed());
}
#endif // NDEBUG

} // namespace
