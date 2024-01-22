/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include <future>

#include <gtest/gtest.h>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/tests/SerialExecutor.h>
#include <hermes/inspector/chrome/tests/TestHelpers.h>

using namespace facebook::hermes;
using namespace facebook::hermes::cdp;
using namespace facebook::hermes::debugger;
using namespace facebook::hermes::inspector_modern::chrome;
using namespace hermes;
using namespace hermes::parser;

class CDPAgentTest : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<AsyncDebuggerAPI> asyncDebuggerAPI_;
  std::unique_ptr<SerialExecutor> runtimeThread_;
};

void CDPAgentTest::SetUp() {
  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());
  asyncDebuggerAPI_ = AsyncDebuggerAPI::create(*runtime_);
  runtimeThread_ = std::make_unique<SerialExecutor>();
}

void CDPAgentTest::TearDown() {
  runtimeThread_.reset();
  asyncDebuggerAPI_.reset();
  runtime_.reset();
}

void ensureErrorResponse(int id, const std::string &json) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<message::ErrorResponse>(mustParseStrAsJsonObj(json, factory));
  EXPECT_EQ(response.id, id);
}

template <typename T>
T waitFor(std::function<void(std::shared_ptr<std::promise<T>>)> callback) {
  auto promise = std::make_shared<std::promise<T>>();
  auto future = promise->get_future();

  callback(promise);

  auto status = future.wait_for(std::chrono::milliseconds(2500));
  if (status != std::future_status::ready) {
    throw std::runtime_error("triggerInterrupt didn't get executed");
  }
  return future.get();
}

TEST_F(CDPAgentTest, IssuesStartupTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  // Trigger the startup task
  auto cdpAgent = CDPAgent::create(
      *runtime_, *asyncDebuggerAPI_, handleTask, handleMessage);

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, IssuesShutdownTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  auto cdpAgent = CDPAgent::create(
      *runtime_, *asyncDebuggerAPI_, handleTask, handleMessage);

  // Ignore the startup task
  gotTask = false;

  // Trigger the shutdown task
  cdpAgent.reset();

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, IssuesCommandHandlingTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  auto cdpAgent = CDPAgent::create(
      *runtime_, *asyncDebuggerAPI_, handleTask, handleMessage);

  // Ignore the startup task
  gotTask = false;

  // Trigger the command-handling task
  cdpAgent->handleCommand(R"({"id": 1, "method": "Runtime.enable"})");

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, RejectsMalformedMethods) {
  int commandID = 1;
  std::unique_ptr<CDPAgent> cdpAgent;

  waitFor<bool>([this, &cdpAgent, commandID](auto promise) {
    OutboundMessageFunc handleMessage =
        [promise, commandID](const std::string &message) {
          ensureErrorResponse(commandID, message);
          promise->set_value(true);
        };

    EnqueueRuntimeTaskFunc handleTask = [this](RuntimeTask task) {
      runtimeThread_->add([this, task]() { task(*runtime_); });
    };
    cdpAgent = CDPAgent::create(
        *runtime_, *asyncDebuggerAPI_, handleTask, handleMessage);

    // Send a command with no domain delimiter in the method. Just format the
    // JSON manually, as there is no Request object for this fake method.
    cdpAgent->handleCommand(
        R"({"id": )" + std::to_string(commandID) +
        R"(, "method": "MethodWithoutADomainDelimiter"})");
  });
}

TEST_F(CDPAgentTest, RejectsUnknownDomains) {
  int commandID = 1;
  std::unique_ptr<CDPAgent> cdpAgent;

  waitFor<bool>([this, &cdpAgent, commandID](auto promise) {
    OutboundMessageFunc handleMessage =
        [promise, commandID](const std::string &message) {
          ensureErrorResponse(commandID, message);
          promise->set_value(true);
        };

    EnqueueRuntimeTaskFunc handleTask = [this](RuntimeTask task) {
      runtimeThread_->add([this, task]() { task(*runtime_); });
    };
    cdpAgent = CDPAgent::create(
        *runtime_, *asyncDebuggerAPI_, handleTask, handleMessage);

    // Send a command with a properly-formatted domain, but unrecognized by the
    // CDP Agent. Just format the JSON manually, as there is no Request object
    // for this fake domain.
    cdpAgent->handleCommand(
        R"({"id": )" + std::to_string(commandID) +
        R"(, "method": "FakeDomain.enable"})");
  });
}

#endif // HERMES_ENABLE_DEBUGGER
