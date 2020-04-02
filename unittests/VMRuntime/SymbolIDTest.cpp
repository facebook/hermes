/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SymbolID.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(SymbolIDTest, UniquedTest) {
  auto id1 = SymbolID::unsafeCreateNotUniqued(0x50000001);
  EXPECT_TRUE(id1.isNotUniqued());
  EXPECT_FALSE(id1.isUniqued());
  EXPECT_EQ(0xd0000001, id1.unsafeGetRaw());
  EXPECT_EQ(0x50000001, id1.unsafeGetIndex());
  auto id2 = SymbolID::unsafeCreate(0x40000007);
  EXPECT_TRUE(id2.isUniqued());
  EXPECT_FALSE(id2.isNotUniqued());
  EXPECT_EQ(0x40000007, id2.unsafeGetRaw());
  EXPECT_EQ(0x40000007, id2.unsafeGetIndex());
}

} // namespace
