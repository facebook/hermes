/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/BitField.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(BitField, Basics) {
  using B1 = BitField<uint32_t, uint8_t, 4, 4>;
  uint32_t storage = 0;
  storage = B1::update(storage, 13);
  ASSERT_EQ(13, B1::extract(storage));
  ASSERT_EQ(13 << 4, storage);
  storage = B1::update(storage, 1);
  ASSERT_EQ(1, B1::extract(storage));
  ASSERT_EQ(1 << 4, storage);
}

TEST(BitField, Overflow) {
  using B1 = BitField<uint32_t, uint8_t, 4, 4>;
  ASSERT_FALSE(B1::isValid(16));
  // -8..7
  using B2 = BitField<uint32_t, int8_t, 4, 4>;
  ASSERT_FALSE(B2::isValid(-9));
  ASSERT_TRUE(B2::isValid(-8));
  ASSERT_TRUE(B2::isValid(7));
  ASSERT_FALSE(B2::isValid(8));
}

TEST(BitField, Whole) {
  using B1 = BitField<uint8_t, uint8_t, 0, 8>;
  uint8_t storage = 0;
  storage = B1::update(storage, 13);
  ASSERT_EQ(13, B1::extract(storage));
  ASSERT_EQ(13, storage);
  storage = B1::update(storage, 1);
  ASSERT_EQ(1, B1::extract(storage));
  ASSERT_EQ(1, storage);
}

TEST(BitField, Bit) {
  using B1 = BitField<uint8_t, uint8_t, 1, 1>;
  using B2 = BitField<uint8_t, uint8_t, 2, 1>;
  uint8_t storage = 0;

  storage = B2::update(storage, 1);
  ASSERT_EQ(1, B2::extract(storage));
  ASSERT_EQ(1 << 2, storage);
  storage = B2::update(storage, 0);
  ASSERT_EQ(0, B2::extract(storage));
  ASSERT_EQ(0, storage);
  storage = B2::update(storage, 1);

  ASSERT_EQ(0, B1::extract(storage));
  storage = B1::update(storage, 1);
  ASSERT_EQ(1, B1::extract(storage));
  ASSERT_EQ(3 << 1, storage);
  storage = B1::update(storage, 0);
  ASSERT_EQ(1 << 2, storage);
}

} // end anonymous namespace
