/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/FastArraySearch.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

using namespace hermes;

//===----------------------------------------------------------------------===//
// searchU32
//===----------------------------------------------------------------------===//

TEST(SIMD, ForwardU32Empty) {
  uint32_t arr[] = {0};
  EXPECT_EQ(searchU32(arr, 0, 0, 42), -1);
}

TEST(SIMD, ForwardU32SingleFound) {
  uint32_t arr[] = {42};
  EXPECT_EQ(searchU32(arr, 0, 1, 42), 0);
}

TEST(SIMD, ForwardU32SingleNotFound) {
  uint32_t arr[] = {99};
  EXPECT_EQ(searchU32(arr, 0, 1, 42), -1);
}

TEST(SIMD, ForwardU32First) {
  uint32_t arr[] = {42, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(searchU32(arr, 0, 8, 42), 0);
}

TEST(SIMD, ForwardU32Last) {
  uint32_t arr[] = {1, 2, 3, 4, 5, 6, 7, 42};
  EXPECT_EQ(searchU32(arr, 0, 8, 42), 7);
}

TEST(SIMD, ForwardU32Middle) {
  uint32_t arr[] = {1, 2, 42, 3, 4, 5, 6, 7};
  EXPECT_EQ(searchU32(arr, 0, 8, 42), 2);
}

TEST(SIMD, ForwardU32MultipleMatches) {
  uint32_t arr[] = {1, 42, 2, 42, 3, 42, 4, 42};
  // Should return first match.
  EXPECT_EQ(searchU32(arr, 0, 8, 42), 1);
}

TEST(SIMD, ForwardU32ExactSimdWidth) {
  // Exactly 4 elements (one SIMD lane on NEON/SSE2).
  uint32_t arr[] = {1, 2, 42, 4};
  EXPECT_EQ(searchU32(arr, 0, 4, 42), 2);
}

TEST(SIMD, ForwardU32SimdPlusRemainder) {
  // 5 elements: 4 SIMD + 1 scalar tail.
  uint32_t arr[] = {1, 2, 3, 4, 42};
  EXPECT_EQ(searchU32(arr, 0, 5, 42), 4);
}

TEST(SIMD, ForwardU32WithStartOffset) {
  uint32_t arr[] = {42, 1, 2, 3, 42, 5, 6, 7};
  // Start at index 1, should skip the first 42.
  EXPECT_EQ(searchU32(arr, 1, 8, 42), 4);
}

TEST(SIMD, ForwardU32Large) {
  std::vector<uint32_t> arr(200, 0);
  arr[150] = 42;
  EXPECT_EQ(searchU32(arr, 0, arr.size(), 42), 150);
}

TEST(SIMD, ForwardU32LargeNotFound) {
  std::vector<uint32_t> arr(200, 0);
  EXPECT_EQ(searchU32(arr, 0, arr.size(), 42), -1);
}

//===----------------------------------------------------------------------===//
// searchReverseU32
//===----------------------------------------------------------------------===//

TEST(SIMD, ReverseU32Empty) {
  uint32_t arr[] = {0};
  EXPECT_EQ(searchReverseU32(arr, 0, 0, 42), -1);
}

TEST(SIMD, ReverseU32SingleFound) {
  uint32_t arr[] = {42};
  EXPECT_EQ(searchReverseU32(arr, 0, 1, 42), 0);
}

TEST(SIMD, ReverseU32SingleNotFound) {
  uint32_t arr[] = {99};
  EXPECT_EQ(searchReverseU32(arr, 0, 1, 42), -1);
}

TEST(SIMD, ReverseU32First) {
  uint32_t arr[] = {42, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(searchReverseU32(arr, 0, 8, 42), 0);
}

TEST(SIMD, ReverseU32Last) {
  uint32_t arr[] = {1, 2, 3, 4, 5, 6, 7, 42};
  EXPECT_EQ(searchReverseU32(arr, 0, 8, 42), 7);
}

TEST(SIMD, ReverseU32MultipleMatches) {
  uint32_t arr[] = {1, 42, 2, 42, 3, 42, 4, 42};
  // Should return last match.
  EXPECT_EQ(searchReverseU32(arr, 0, 8, 42), 7);
}

TEST(SIMD, ReverseU32ExactSimdWidth) {
  uint32_t arr[] = {1, 42, 3, 4};
  EXPECT_EQ(searchReverseU32(arr, 0, 4, 42), 1);
}

TEST(SIMD, ReverseU32SimdPlusRemainder) {
  uint32_t arr[] = {42, 1, 2, 3, 4};
  // 5 elements: the scalar remainder at the front contains the match.
  EXPECT_EQ(searchReverseU32(arr, 0, 5, 42), 0);
}

TEST(SIMD, ReverseU32WithStartOffset) {
  uint32_t arr[] = {42, 1, 2, 3, 42, 5, 6, 42};
  // Search [1, 7): should find index 4, not 0 or 7.
  EXPECT_EQ(searchReverseU32(arr, 1, 7, 42), 4);
}

TEST(SIMD, ReverseU32Large) {
  std::vector<uint32_t> arr(200, 0);
  arr[50] = 42;
  arr[150] = 42;
  // Should return last match.
  EXPECT_EQ(searchReverseU32(arr, 0, arr.size(), 42), 150);
}

//===----------------------------------------------------------------------===//
// searchU64
//===----------------------------------------------------------------------===//

TEST(SIMD, ForwardU64Empty) {
  uint64_t arr[] = {0};
  EXPECT_EQ(searchU64(arr, 0, 0, 42), -1);
}

TEST(SIMD, ForwardU64SingleFound) {
  uint64_t arr[] = {42};
  EXPECT_EQ(searchU64(arr, 0, 1, 42), 0);
}

TEST(SIMD, ForwardU64SingleNotFound) {
  uint64_t arr[] = {99};
  EXPECT_EQ(searchU64(arr, 0, 1, 42), -1);
}

TEST(SIMD, ForwardU64First) {
  uint64_t arr[] = {42, 1, 2, 3};
  EXPECT_EQ(searchU64(arr, 0, 4, 42), 0);
}

TEST(SIMD, ForwardU64Last) {
  uint64_t arr[] = {1, 2, 3, 42};
  EXPECT_EQ(searchU64(arr, 0, 4, 42), 3);
}

TEST(SIMD, ForwardU64MultipleMatches) {
  uint64_t arr[] = {1, 42, 2, 42, 3, 42};
  EXPECT_EQ(searchU64(arr, 0, 6, 42), 1);
}

TEST(SIMD, ForwardU64ExactSimdWidth) {
  // 2 elements (one SIMD lane on NEON).
  uint64_t arr[] = {1, 42};
  EXPECT_EQ(searchU64(arr, 0, 2, 42), 1);
}

TEST(SIMD, ForwardU64SimdPlusRemainder) {
  // 3 elements: 2 SIMD + 1 scalar tail.
  uint64_t arr[] = {1, 2, 42};
  EXPECT_EQ(searchU64(arr, 0, 3, 42), 2);
}

TEST(SIMD, ForwardU64WithStartOffset) {
  uint64_t arr[] = {42, 1, 2, 42, 4};
  EXPECT_EQ(searchU64(arr, 1, 5, 42), 3);
}

TEST(SIMD, ForwardU64Large) {
  std::vector<uint64_t> arr(200, 0);
  arr[150] = 42;
  EXPECT_EQ(searchU64(arr, 0, arr.size(), 42), 150);
}

TEST(SIMD, ForwardU64LargeValue) {
  // Test with a value that uses upper 32 bits.
  uint64_t target = 0xDEADBEEF12345678ULL;
  std::vector<uint64_t> arr(20, 0);
  arr[13] = target;
  EXPECT_EQ(searchU64(arr, 0, arr.size(), target), 13);
}

//===----------------------------------------------------------------------===//
// searchReverseU64
//===----------------------------------------------------------------------===//

TEST(SIMD, ReverseU64Empty) {
  uint64_t arr[] = {0};
  EXPECT_EQ(searchReverseU64(arr, 0, 0, 42), -1);
}

TEST(SIMD, ReverseU64SingleFound) {
  uint64_t arr[] = {42};
  EXPECT_EQ(searchReverseU64(arr, 0, 1, 42), 0);
}

TEST(SIMD, ReverseU64SingleNotFound) {
  uint64_t arr[] = {99};
  EXPECT_EQ(searchReverseU64(arr, 0, 1, 42), -1);
}

TEST(SIMD, ReverseU64First) {
  uint64_t arr[] = {42, 1, 2, 3};
  EXPECT_EQ(searchReverseU64(arr, 0, 4, 42), 0);
}

TEST(SIMD, ReverseU64Last) {
  uint64_t arr[] = {1, 2, 3, 42};
  EXPECT_EQ(searchReverseU64(arr, 0, 4, 42), 3);
}

TEST(SIMD, ReverseU64MultipleMatches) {
  uint64_t arr[] = {1, 42, 2, 42, 3, 42};
  EXPECT_EQ(searchReverseU64(arr, 0, 6, 42), 5);
}

TEST(SIMD, ReverseU64ExactSimdWidth) {
  uint64_t arr[] = {42, 1};
  EXPECT_EQ(searchReverseU64(arr, 0, 2, 42), 0);
}

TEST(SIMD, ReverseU64SimdPlusRemainder) {
  uint64_t arr[] = {42, 1, 2};
  EXPECT_EQ(searchReverseU64(arr, 0, 3, 42), 0);
}

TEST(SIMD, ReverseU64WithStartOffset) {
  uint64_t arr[] = {42, 1, 42, 3, 42};
  // Search [1, 4): should find index 2.
  EXPECT_EQ(searchReverseU64(arr, 1, 4, 42), 2);
}

TEST(SIMD, ReverseU64Large) {
  std::vector<uint64_t> arr(200, 0);
  arr[50] = 42;
  arr[150] = 42;
  EXPECT_EQ(searchReverseU64(arr, 0, arr.size(), 42), 150);
}

//===----------------------------------------------------------------------===//
// Max-value target tests
//===----------------------------------------------------------------------===//

TEST(SIMD, ForwardU32MaxValue) {
  // UINT32_MAX (0xFFFFFFFF) is the same bit pattern as SIMD comparison match
  // results, so verify the search does not confuse data with comparison masks.
  uint32_t target = std::numeric_limits<uint32_t>::max();
  std::vector<uint32_t> arr(20, 0);
  arr[11] = target;
  EXPECT_EQ(searchU32(arr, 0, arr.size(), target), 11);
}

TEST(SIMD, ReverseU32MaxValue) {
  uint32_t target = std::numeric_limits<uint32_t>::max();
  std::vector<uint32_t> arr(20, 0);
  arr[3] = target;
  arr[17] = target;
  EXPECT_EQ(searchReverseU32(arr, 0, arr.size(), target), 17);
}

TEST(SIMD, ForwardU64MaxValue) {
  uint64_t target = std::numeric_limits<uint64_t>::max();
  std::vector<uint64_t> arr(20, 0);
  arr[13] = target;
  EXPECT_EQ(searchU64(arr, 0, arr.size(), target), 13);
}

TEST(SIMD, ReverseU64MaxValue) {
  uint64_t target = std::numeric_limits<uint64_t>::max();
  std::vector<uint64_t> arr(20, 0);
  arr[4] = target;
  arr[16] = target;
  EXPECT_EQ(searchReverseU64(arr, 0, arr.size(), target), 16);
}

//===----------------------------------------------------------------------===//
// Alignment stress tests
//===----------------------------------------------------------------------===//

TEST(SIMD, ForwardU32VariousOffsets) {
  // Test with different start offsets to exercise alignment variations.
  std::vector<uint32_t> arr(20, 0);
  arr[15] = 42;
  for (size_t start = 0; start <= 15; ++start) {
    EXPECT_EQ(searchU32(arr, start, 20, 42), 15) << "start=" << start;
  }
  // Starting past the target should not find it.
  EXPECT_EQ(searchU32(arr, 16, 20, 42), -1);
}

TEST(SIMD, ReverseU32VariousOffsets) {
  std::vector<uint32_t> arr(20, 0);
  arr[5] = 42;
  for (size_t end = 6; end <= 20; ++end) {
    EXPECT_EQ(searchReverseU32(arr, 0, end, 42), 5) << "end=" << end;
  }
  // Ending before the target should not find it.
  EXPECT_EQ(searchReverseU32(arr, 0, 5, 42), -1);
}

TEST(SIMD, ForwardU64VariousOffsets) {
  std::vector<uint64_t> arr(20, 0);
  arr[15] = 42;
  for (size_t start = 0; start <= 15; ++start) {
    EXPECT_EQ(searchU64(arr, start, 20, 42), 15) << "start=" << start;
  }
  EXPECT_EQ(searchU64(arr, 16, 20, 42), -1);
}

TEST(SIMD, ReverseU64VariousOffsets) {
  std::vector<uint64_t> arr(20, 0);
  arr[5] = 42;
  for (size_t end = 6; end <= 20; ++end) {
    EXPECT_EQ(searchReverseU64(arr, 0, end, 42), 5) << "end=" << end;
  }
  EXPECT_EQ(searchReverseU64(arr, 0, 5, 42), -1);
}

} // namespace
