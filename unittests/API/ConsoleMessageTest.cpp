/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <hermes/cdp/ConsoleMessage.h>
#include <hermes/hermes.h>

using namespace facebook::hermes::cdp;
using namespace facebook::hermes;
using namespace facebook;

class ConsoleMessageTest : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  jsi::Value eval(const char *code) {
    return runtime_->global()
        .getPropertyAsFunction(*runtime_, "eval")
        .call(*runtime_, code);
  }

  std::unique_ptr<HermesRuntime> runtime_;
};

void ConsoleMessageTest::SetUp() {
  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());
}

void ConsoleMessageTest::TearDown() {
  runtime_.reset();
}

TEST_F(ConsoleMessageTest, TestBasicOperations) {
  ConsoleMessageStorage storage;

  // Cache a couple messages
  jsi::Value val1 = eval("1");
  jsi::Value val2 = eval("2");
  jsi::Value val3 = eval("'3'");
  std::vector<jsi::Value> args1;
  args1.push_back(std::move(val1));
  std::vector<jsi::Value> args2;
  args2.push_back(std::move(val2));
  args2.push_back(std::move(val3));
  storage.addMessage(ConsoleMessage{1, ConsoleAPIType::kLog, std::move(args1)});
  storage.addMessage(
      ConsoleMessage{2, ConsoleAPIType::kDebug, std::move(args2)});

  // Get them back out to verify they're stored properly
  auto &messages = storage.messages();
  EXPECT_EQ(messages.size(), 2);
  EXPECT_EQ(messages[0].timestamp, 1);
  EXPECT_EQ(messages[0].type, ConsoleAPIType::kLog);
  EXPECT_EQ(messages[0].args[0].getNumber(), 1);
  EXPECT_EQ(messages[1].timestamp, 2);
  EXPECT_EQ(messages[1].type, ConsoleAPIType::kDebug);
  EXPECT_EQ(messages[1].args[0].getNumber(), 2);
  EXPECT_EQ(messages[1].args[1].getString(*runtime_).utf8(*runtime_), "3");
  EXPECT_EQ(storage.discarded(), 0);

  // Test that clearing works properly
  storage.clear();
  auto &messagesAfter = storage.messages();
  EXPECT_EQ(messagesAfter.size(), 0);
  EXPECT_EQ(storage.discarded(), 0);
}

TEST_F(ConsoleMessageTest, TestOverflow) {
  // Create a cache that only allows 2 messages
  ConsoleMessageStorage storage(2);

  // Add 3 messages to overflow the cache
  for (int i = 0; i < 3; i++) {
    jsi::Value value = eval(std::to_string(i).c_str());
    std::vector<jsi::Value> args;
    args.push_back(std::move(value));
    storage.addMessage(
        ConsoleMessage{1, ConsoleAPIType::kLog, std::move(args)});
  }

  // Verify only the most recent messages remain
  auto &messages = storage.messages();
  EXPECT_EQ(storage.discarded(), 1);
  EXPECT_EQ(messages.size(), 2);
  EXPECT_EQ(messages[0].timestamp, 1);
  EXPECT_EQ(messages[0].type, ConsoleAPIType::kLog);
  EXPECT_EQ(messages[0].args[0].getNumber(), 1);
  EXPECT_EQ(messages[1].timestamp, 1);
  EXPECT_EQ(messages[1].type, ConsoleAPIType::kLog);
  EXPECT_EQ(messages[1].args[0].getNumber(), 2);

  // Test that clearing works properly
  storage.clear();
  auto &messagesAfter = storage.messages();
  EXPECT_EQ(messagesAfter.size(), 0);
  EXPECT_EQ(storage.discarded(), 0);
}
