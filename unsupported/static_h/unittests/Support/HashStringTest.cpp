/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/HashString.h"

#include <limits>

#include "gtest/gtest.h"

using namespace hermes;

namespace {

static llvh::ArrayRef<char> makeArrayRef(const char *s) {
  return {s, strlen(s)};
}

TEST(HashStringTest, Constexpr) {
  EXPECT_EQ(hashString(makeArrayRef("abc")), constexprHashString("abc"));
  EXPECT_EQ(hashString(makeArrayRef("")), constexprHashString(""));
  EXPECT_EQ(
      hashString(makeArrayRef("1234567")), constexprHashString("1234567"));
}

} // end anonymous namespace
