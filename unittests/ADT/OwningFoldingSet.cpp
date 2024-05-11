/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/OwningFoldingSet.h"

namespace {

class TestNode : public llvh::FoldingSetNode {
 public:
  int data_;

  explicit TestNode(int data) : data_(data) {}

  static void Profile(llvh::FoldingSetNodeID &ID, int data) {
    ID.AddInteger(data);
  }
  void Profile(llvh::FoldingSetNodeID &ID) const {
    Profile(ID, data_);
  }
};

TEST(OwningFoldingSet, SmokeTest) {
  hermes::OwningFoldingSet<TestNode> set{};
  auto [node, ins] = set.getOrEmplace(42);
  EXPECT_TRUE(ins);
  EXPECT_EQ(42, node->data_);
  auto res = set.getOrEmplace(42);
  EXPECT_FALSE(res.second);
  EXPECT_EQ(node, res.first);
  res = set.getOrEmplace(43);
  EXPECT_TRUE(res.second);
  EXPECT_NE(node, res.first);
}

} // anonymous namespace
