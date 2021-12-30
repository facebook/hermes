/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ADT/BitArray.h"

#include "gtest/gtest.h"

#include <deque>

namespace {

using namespace hermes;

template <typename SIZE>
struct BitArrayTest : public ::testing::Test {
 protected:
  std::vector<std::deque<size_t>> getTestIndices() {
    std::vector<std::deque<size_t>> testIndices;
    // Use a mix of word-aligned and non-word-aligned values. Set the first and
    // last bits to check that those edge cases are handled correctly.
    testIndices.push_back(
        {0,
         1,
         2,
         128,
         234,
         436,
         789,
         1099,
         SIZE::value - 65,
         SIZE::value - 2,
         SIZE::value - 1});

    // Test the case where there are no set bits at the start/end.
    testIndices.push_back({128, 234, 436, 789, 1099, SIZE::value - 100});

    // Test the case where set bits occur at word boundaries.
    testIndices.push_back({63, 127, 1023, 1024, SIZE::value - 1});

    return testIndices;
  }
};

// Use a mix of word-aligned and non-word-aligned sizes. Also have a mix of
// different numbers of words.
using TestSizes = testing::Types<
    std::integral_constant<size_t, 1571>,
    std::integral_constant<size_t, 2047>,
    std::integral_constant<size_t, 2048>>;
TYPED_TEST_CASE(BitArrayTest, TestSizes);

TYPED_TEST(BitArrayTest, NextSetBit) {
  constexpr size_t N = TypeParam::value;
  BitArray<N> ba;
  auto testIndices = this->getTestIndices();
  for (auto &indices : testIndices) {
    ba.reset();
    // Empty case: No marked bits
    EXPECT_EQ(N, ba.findNextSetBitFrom(0));
    for (size_t idx : indices) {
      ba.set(idx, true);
    }
    // Use the same style of loop we use elsewhere for scanning the array.
    for (size_t from = ba.findNextSetBitFrom(0); from < N;
         from = ba.findNextSetBitFrom(from + 1)) {
      EXPECT_EQ(indices.front(), from);
      indices.pop_front();
    }
    EXPECT_EQ(0, indices.size());
  }
}

TYPED_TEST(BitArrayTest, NextZeroBit) {
  constexpr size_t N = TypeParam::value;
  BitArray<N> ba;
  auto testIndices = this->getTestIndices();
  for (auto &indices : testIndices) {
    ba.set();
    // Full case: No unmarked bits
    EXPECT_EQ(N, ba.findNextZeroBitFrom(0));
    for (auto idx : indices) {
      ba.set(idx, false);
    }

    // Use the same style of loop we use elsewhere for scanning the array.
    for (size_t from = ba.findNextZeroBitFrom(0); from < N;
         from = ba.findNextZeroBitFrom(from + 1)) {
      EXPECT_EQ(indices.front(), from);
      indices.pop_front();
    }
    EXPECT_EQ(0, indices.size());
  }
}

TYPED_TEST(BitArrayTest, PrevSetBit) {
  constexpr size_t N = TypeParam::value;
  BitArray<N> ba;

  auto testIndices = this->getTestIndices();
  for (auto &indices : testIndices) {
    ba.reset();
    // Empty case: No marked bits
    EXPECT_EQ(N, ba.findPrevSetBitFrom(N));
    for (auto idx : indices) {
      ba.set(idx, true);
    }
    for (size_t from = ba.findPrevSetBitFrom(N); from != N;
         from = ba.findPrevSetBitFrom(from)) {
      EXPECT_EQ(indices.back(), from);
      indices.pop_back();
    }
    EXPECT_EQ(0, indices.size());
  }
}

TYPED_TEST(BitArrayTest, PrevZeroBit) {
  constexpr size_t N = TypeParam::value;
  BitArray<N> ba;
  auto testIndices = this->getTestIndices();
  for (auto &indices : testIndices) {
    ba.set();
    // Full case: No unmarked bits
    EXPECT_EQ(N, ba.findPrevZeroBitFrom(N));
    for (auto idx : indices) {
      ba.set(idx, false);
    }
    for (size_t from = ba.findPrevZeroBitFrom(N); from != N;
         from = ba.findPrevZeroBitFrom(from)) {
      EXPECT_EQ(indices.back(), from);
      indices.pop_back();
    }
    EXPECT_EQ(0, indices.size());
  }
}

} // namespace
