/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/ADT/CompactArray.h"

namespace {

using namespace hermes;

static constexpr CompactArray::Scale scales[] = {
    CompactArray::UINT8,
    CompactArray::UINT16,
    CompactArray::UINT32};

TEST(CompactArrayTest, Construct) {
  for (auto scale : scales) {
    {
      CompactArray t(0, scale);
      EXPECT_EQ(0, t.size());
      EXPECT_EQ(scale, t.getCurrentScale());
    }
    {
      CompactArray t(1234, scale);
      EXPECT_EQ(1234, t.size());
      EXPECT_EQ(scale, t.getCurrentScale());
      EXPECT_EQ(0, t.get(1001));
    }
  }
}

TEST(CompactArrayTest, ScaleUp) {
  CompactArray t(1234, CompactArray::UINT8);

  t.set(1001, 1 << 8);
  EXPECT_EQ(CompactArray::UINT16, t.getCurrentScale());
  EXPECT_EQ(1234, t.size());
  EXPECT_EQ(1 << 8, t.get(1001));

  t.set(1002, 1 << 16);
  EXPECT_EQ(CompactArray::UINT32, t.getCurrentScale());
  EXPECT_EQ(1234, t.size());
  EXPECT_EQ(1 << 16, t.get(1002));
}

TEST(CompactArrayTest, Values) {
  for (uint32_t v = 0; v <= 66000; ++v) {
    CompactArray t(1, CompactArray::UINT8);
    t.set(0, v);
    EXPECT_EQ(v, t.get(0));
  }
}

TEST(CompactTableTest, Basic) {
  CompactTable t(1);
  EXPECT_TRUE(t.isEmpty(0));

  t.set(0, 123456);
  EXPECT_TRUE(t.isValid(0));
  EXPECT_EQ(123456, t.get(0));

  t.markAsDeleted(0);
  EXPECT_TRUE(t.isDeleted(0));

  t.set(0, 654321);
  EXPECT_TRUE(t.isValid(0));
  EXPECT_EQ(654321, t.get(0));
}

} // namespace
