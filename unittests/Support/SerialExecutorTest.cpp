/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SerialExecutor.h"

#include "gtest/gtest.h"

namespace {

TEST(SerialExecutorTest, TestBasic) {
  hermes::SerialExecutor executor;
  std::atomic<int> counter{0};

  for (int i = 0; i < 100; ++i)
    executor.add([&counter] { ++counter; });

  while (counter < 100) {
  }
}

TEST(SerialExecutorTest, DestructorDrainsTasks) {
  int counter{0};
  {
    hermes::SerialExecutor executor;

    for (int i = 0; i < 100; ++i)
      executor.add([&counter] { ++counter; });
  }
  ASSERT_EQ(counter, 100);
}

} // end anonymous namespace
