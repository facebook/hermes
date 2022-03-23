/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Algorithms.h"

#include "gtest/gtest.h"

#include <array>
#include <type_traits>

using namespace hermes;

namespace {
struct NontrivialType {
  static int kConstructorCalls;
  static int kAssignmentCalls;
  int x;
  NontrivialType() {
    x = 17;
    kConstructorCalls++;
  }
  NontrivialType(const NontrivialType &) {
    x = 18;
    kConstructorCalls++;
  }
  NontrivialType &operator=(const NontrivialType &) {
    x = 19;
    kAssignmentCalls++;
    return *this;
  }
};
int NontrivialType::kConstructorCalls = 0;
int NontrivialType::kAssignmentCalls = 0;
} // namespace

TEST(Algorithms, Copy) {
  NontrivialType::kConstructorCalls = 0;
  NontrivialType::kAssignmentCalls = 0;
  EXPECT_FALSE(std::is_trivial<NontrivialType>::value);

  std::array<NontrivialType, 1> nts;
  EXPECT_EQ(1, NontrivialType::kConstructorCalls);
  EXPECT_EQ(0, NontrivialType::kAssignmentCalls);

  // Use malloc() to get uninitialized memory.
  NontrivialType *ptr = static_cast<NontrivialType *>(malloc(sizeof *ptr));

  // uninitializedCopy should not invoke assignment.
  hermes::uninitializedCopy(nts.begin(), nts.end(), ptr);
  EXPECT_EQ(2, NontrivialType::kConstructorCalls);
  EXPECT_EQ(0, NontrivialType::kAssignmentCalls);
  EXPECT_EQ(ptr->x, 18);

  // uninitializedCopyN should not invoke assignment.
  nts[0].x = 100;
  hermes::uninitializedCopyN(nts.begin(), 1, ptr);
  EXPECT_EQ(3, NontrivialType::kConstructorCalls);
  EXPECT_EQ(0, NontrivialType::kAssignmentCalls);
  EXPECT_EQ(ptr->x, 18);
  free(ptr);
}
