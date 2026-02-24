/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/PropertyCache.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(PropertyCacheTest, WritePropertyCacheTest) {
  WritePropertyCacheEntry entry{};
  entry.setSlot(0xab);
  EXPECT_EQ(0xab, entry.getSlot());
  EXPECT_EQ(0, entry.getAddCacheIndex());
  entry.setSlot(0xab);
  EXPECT_EQ(0xab, entry.getSlot());
  EXPECT_EQ(0x0, entry.getAddCacheIndex());
  entry.setAddCacheIndex(0x5abacc);
  EXPECT_EQ(0xab, entry.getSlot());
  EXPECT_EQ(0x5abacc, entry.getAddCacheIndex());
  entry.setSlot(0x12);
  EXPECT_EQ(0x12, entry.getSlot());
  EXPECT_EQ(0x5abacc, entry.getAddCacheIndex());
}

} // namespace
