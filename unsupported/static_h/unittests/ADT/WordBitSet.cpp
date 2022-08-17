/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/WordBitSet.h"

using namespace hermes;

namespace {

TEST(WordBitSet, SmokeTest) {
  WordBitSet<uint32_t> s{};

  ASSERT_TRUE(s.empty());
  ASSERT_TRUE(s.begin() == s.end());

  s.set(0);
  s.set(1);
  s.set(20);
  s.set(31);

  for (unsigned i = 0; i < 32; ++i) {
    bool b1 = s.at(i);
    bool b2 = s[i];
    ASSERT_EQ(b1, b2);
    ASSERT_EQ(i == 0 || i == 1 || i == 20 || i == 31, b2);
  }

  ASSERT_EQ(0, s.findFirst());
  ASSERT_EQ(0, s.findFirst());
  ASSERT_EQ(1, s.findNext(0));
  ASSERT_EQ(1, s.findNext(0));
  ASSERT_EQ(20, s.findNext(1));
  ASSERT_EQ(20, s.findNext(1));
  ASSERT_EQ(31, s.findNext(20));
  ASSERT_EQ(31, s.findNext(20));
  ASSERT_EQ(-1, s.findNext(31));
  ASSERT_EQ(-1, s.findNext(31));

  ASSERT_EQ(20, s.findNext(5));

  auto it = s.begin();
  auto end = s.end();

  ASSERT_TRUE(end == s.end());
  ASSERT_TRUE(it != end);

  ASSERT_EQ(0, *it);
  ASSERT_EQ(0, *it);

  ++it;
  ASSERT_TRUE(it != end);
  ASSERT_EQ(1, *it);
  ASSERT_EQ(1, *it);

  ++it;
  ASSERT_TRUE(it != end);
  ASSERT_EQ(20, *it);
  ASSERT_EQ(20, *it);

  ++it;
  ASSERT_TRUE(it != end);
  ASSERT_EQ(31, *it);
  ASSERT_EQ(31, *it);

  ++it;
  ASSERT_TRUE(it == end);
}

} // anonymous namespace
