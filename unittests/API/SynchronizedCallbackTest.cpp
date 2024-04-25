/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <hermes/cdp/DomainAgent.h>

using namespace facebook::hermes::cdp;

// In this test file, we're not verifying the thread safety. That is already
// verified at compile time using Thread Safety Analysis.

TEST(SynchronizedCallbackTest, ZeroArgsTest) {
  bool wasCalled = false;
  SynchronizedCallback<> callback([&wasCalled]() { wasCalled = true; });
  // Verify it can be called successfully
  callback();
  EXPECT_TRUE(wasCalled);
}

TEST(SynchronizedCallbackTest, OneArgsTest) {
  std::string result;
  SynchronizedCallback<const std::string &> callback(
      [&result](const std::string &message) {
        result = "message: " + message;
      });
  // Verify it can be called successfully
  callback("test");
  EXPECT_EQ(result, "message: test");
}

TEST(SynchronizedCallbackTest, TwoArgsTest) {
  int sum = -1;
  SynchronizedCallback<int, int> callback(
      [&sum](int a, int b) { sum = a + b; });
  // Verify it can be called successfully
  callback(1, 2);
  EXPECT_EQ(sum, 3);
}

TEST(SynchronizedCallbackTest, UseAfterInvalidate) {
  int sum = -1;
  SynchronizedCallback<int, int> callback(
      [&sum](int a, int b) { sum = a + b; });
  callback.invalidate();
  callback(1, 2);
  // Should be a no-op and doesn't change sum
  EXPECT_EQ(sum, -1);
}
