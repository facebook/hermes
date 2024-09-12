/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SerialExecutor.h"
#include "llvh/ADT/ScopeExit.h"

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

TEST(SerialExecutorTest, TestTimeout) {
  // Set up an executor with a short timeout.
  constexpr std::chrono::milliseconds timeout{10};
  hermes::SerialExecutor executor{0, timeout};
  std::atomic<int> counter{0};

  for (int i = 0; i < 5; ++i) {
    auto t0 = std::chrono::steady_clock::now();

    // Add a task that sets up a thread-local destructor that will increment the
    // counter. This allows us to observe when the thread is destroyed by the
    // executor.
    executor.add([&counter] {
      thread_local auto se = llvh::make_scope_exit([&counter] { ++counter; });
    });

    // Wait for the counter to be incremented when the thread is joined.
    while (counter == i) {
    }

    auto t1 = std::chrono::steady_clock::now();
    ASSERT_GE(t1 - t0, timeout);
  }
}

} // end anonymous namespace
