/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/HermesSafeMath.h"

#include <limits>
#include "gtest/gtest.h"

namespace {

TEST(HermesSafeMathTest, sizeCheckSuccess) {
  uint64_t maxU32 = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
  hermes::sizeCheck<uint32_t>(maxU32, "overflow");
  int64_t maxI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::max());
  hermes::sizeCheck<int32_t>(maxI32, "overflow");
  int64_t minI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::min());
  hermes::sizeCheck<int32_t>(minI32, "overflow");
}

TEST(HermesSafeMathTest, narrowCastSuccess) {
  uint64_t maxU32 = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(
      std::numeric_limits<uint32_t>::max(),
      hermes::safePossiblyNarrowingCast<uint32_t>(maxU32, "overflow"));
  int64_t maxI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::max());
  EXPECT_EQ(
      std::numeric_limits<int32_t>::max(),
      hermes::safePossiblyNarrowingCast<int32_t>(maxI32, "overflow"));
  int64_t minI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::min());
  EXPECT_EQ(
      std::numeric_limits<int32_t>::min(),
      hermes::safePossiblyNarrowingCast<int32_t>(minI32, "overflow"));
}

// Only do death tests if they're supported.
#if defined(ASSERT_DEATH)

TEST(HermesSafeMathDeathTest, sizeCheckFailureUInt32) {
  uint64_t maxU32Plus1 =
      static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;
  ASSERT_DEATH(
      hermes::sizeCheck<uint32_t>(maxU32Plus1, "overflowxxx"), "overflowxxx");
}

TEST(HermesSafeMathDeathTest, sizeCheckFailureInt32Pos) {
  int64_t maxI32Plus1 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;
  ASSERT_DEATH(
      hermes::sizeCheck<int32_t>(maxI32Plus1, "overflowxxx"), "overflowxxx");
}

TEST(HermesSafeMathDeathTest, sizeCheckFailureInt32Neg) {
  int64_t minI32Minus1 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1;
  ASSERT_DEATH(
      hermes::sizeCheck<int32_t>(minI32Minus1, "overflowxxx"), "overflowxxx");
}

TEST(HermesSafeMathDeathTest, narrowCastFailureUInt32) {
  uint64_t maxU32Plus1 =
      static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;
  ASSERT_DEATH(
      hermes::safePossiblyNarrowingCast<uint32_t>(maxU32Plus1, "overflowxxx"),
      "overflowxxx");
}

TEST(HermesSafeMathDeathTest, narrowCastFailureInt32Pos) {
  int64_t maxI32Plus1 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;
  ASSERT_DEATH(
      hermes::safePossiblyNarrowingCast<int32_t>(maxI32Plus1, "overflowxxx"),
      "overflowxxx");
}

TEST(HermesSafeMathDeathTest, narrowCastFailureInt32Neg) {
  int64_t minI32Minus1 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1;
  ASSERT_DEATH(
      hermes::safePossiblyNarrowingCast<int32_t>(minI32Minus1, "overflowxxx"),
      "overflowxxx");
}

#endif // !defined(NDEBUG) && defined(ASSERT_DEATH)

} // namespace
