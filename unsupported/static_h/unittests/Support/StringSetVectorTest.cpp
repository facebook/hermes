/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/Support/StringSetVector.h"

namespace {

using namespace hermes;

TEST(StringSetVectorTest, Uniquing) {
  StringSetVector ssv;

  EXPECT_TRUE(ssv.empty());
  EXPECT_EQ(0, ssv.size());

  auto ix0 = ssv.insert("hello");
  EXPECT_FALSE(ssv.empty());
  EXPECT_EQ(1, ssv.size());

  auto ix1 = ssv.insert("world");
  EXPECT_EQ(2, ssv.size());

  auto ix2 = ssv.insert("hello");
  EXPECT_EQ(2, ssv.size());

  EXPECT_NE(ix0, ix1);
  EXPECT_EQ(ix0, ix2);
}

TEST(StringSetVector, Order) {
  StringSetVector ssv;

  ssv.insert("hello");
  ssv.insert("world");
  ssv.insert("hello");
  ssv.insert("foo");
  ssv.insert("bar");
  ssv.insert("bar");
  ssv.insert("baz");

  std::vector<std::string> expected{"hello", "world", "foo", "bar", "baz"};
  EXPECT_EQ(ssv.size(), expected.size());

  unsigned i = 0;
  for (auto &s : ssv) {
    EXPECT_EQ(s, expected[i]) << "At index " << i;
    i++;
  }
}

TEST(StringSetVector, Find) {
  StringSetVector ssv;

  ssv.insert("hello");
  ssv.insert("world");
  ssv.insert("hello");

  EXPECT_EQ("hello", *ssv.find("hello"));
  EXPECT_EQ("world", *ssv.find("world"));

  // Can't use EXPECT_EQ because it requires gtest to be able to print the
  // values, and in some builds it cannot print iterators.
  EXPECT_TRUE(ssv.end() == ssv.find("bar"));
}

TEST(StringSetVector, Indexing) {
  StringSetVector ssv;

  ssv.insert("hello");
  ssv.insert("world");
  ssv.insert("hello");

  ASSERT_EQ(2, ssv.size());
  EXPECT_EQ("hello", ssv[0]);
  EXPECT_EQ("world", ssv[1]);
}

} // namespace
