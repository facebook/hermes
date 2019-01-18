/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "llvm_extra/SuffixTree.h"

#include "gtest/gtest.h"

#include <iterator>
#include <set>
#include <tuple>
#include <vector>

using llvm::SuffixTree;
using llvm::SuffixTreeNode;

namespace {

TEST(SuffixTreeTest, EmptyTreeTest) {
  std::vector<unsigned> vec;
  SuffixTree tree(vec);

  EXPECT_TRUE(tree.Str.empty());
  EXPECT_TRUE(tree.MultiLeafParents.empty());
}

// Suffix tree for BANANA
// Nodes in MultiLeafParents marked (*)
// Dollar sign $ means end-of-string
//
//              (*)
//               |
//  +------------+------------+
// $|       A|     BANANA$  NA|
//  |        |        |       |
//  *        *        *      (*)
//           |                |
//       +---+---+        +---+---+
//      $|     NA|       $|    NA$|
//       |       |        |       |
//       *      (*)        *       *
//               |
//           +---+---+
//          $|    NA$|
//           |       |
//           *       *
TEST(SuffixTreeTest, BananaTest) {
  const char str[] = "BANANA";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  SuffixTree tree(vec);

  EXPECT_EQ(llvm::ArrayRef<unsigned>(vec), tree.Str);
  EXPECT_EQ(3, tree.MultiLeafParents.size());

  // StartIdx, EndIdx, ConcatLen, NumChildren.
  using NodeInfo = std::tuple<unsigned, unsigned, unsigned, unsigned>;

  std::set<NodeInfo> expectedNodes;
  const unsigned EmptyIdx = SuffixTreeNode::EmptyIdx;
  // The root node.
  expectedNodes.emplace(EmptyIdx, EmptyIdx, 0, 4);
  // The first "NA" internal node.
  expectedNodes.emplace(2, 3, 2, 2);
  // The second "NA" internal node.
  expectedNodes.emplace(2, 3, 3, 2);

  for (SuffixTreeNode *node : tree.MultiLeafParents) {
    const NodeInfo info(
        node->StartIdx, *node->EndIdx, node->ConcatLen, node->Children.size());
    const auto numRemoved = expectedNodes.erase(info);
    EXPECT_EQ(1, numRemoved);
  }
  EXPECT_TRUE(expectedNodes.empty());
}

TEST(SuffixTreeTest, MultiLeafParentsTest) {
  const char str[] = "peter piper picked a peck of pickled peppers";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  SuffixTree tree(vec);

  EXPECT_FALSE(tree.MultiLeafParents.empty());
  for (SuffixTreeNode *node : tree.MultiLeafParents) {
    int leaves = 0;
    for (auto entry : node->Children) {
      SuffixTreeNode *child = entry.second;
      if (child->isLeaf()) {
        ++leaves;
      }
    }
    EXPECT_GE(leaves, 2);
  }
}

} // namespace
