/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/TransparentConservativeVector.h"

namespace hermes {

namespace {

TEST(TransparentConservativeVector, GrowTest) {
  struct Ints {
    int x;
    int y;
    Ints(int x, int y) : x(x), y(y) {}
  };

  TransparentConservativeVector<Ints> v{};
  for (int i = 0; i < 1000; ++i) {
    v.emplace_back(i, i);
  }

  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(v[i].x, i);
    EXPECT_EQ(v[i].y, i);
  }
}

} // namespace

} // namespace hermes
