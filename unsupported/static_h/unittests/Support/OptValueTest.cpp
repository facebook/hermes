/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OptValue.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

template <typename T>
void run1BasicTest() {
  T val1{};
  T val2 = !val1;
  OptValue<T> empty1{};
  OptValue<T> empty2(llvh::None);
  OptValue<T> filled1(val1);
  OptValue<T> filled2(val2);
  ASSERT_EQ(empty1, empty2);
  ASSERT_NE(empty1, filled1);
  ASSERT_EQ(filled1, filled1);
  ASSERT_NE(filled1, filled2);
  ASSERT_EQ(*filled1, val1);
  ASSERT_EQ(filled2.getValue(), val2);
}

TEST(OptValue, Basics) {
  run1BasicTest<int>();
  run1BasicTest<double>();
  run1BasicTest<bool>();
}

} // end anonymous namespace
