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
#include <hermes/CompileJS.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/tests/SerialExecutor.h>

#include "CDPJSONHelpers.h"

using namespace facebook::hermes::cdp;
using namespace facebook::hermes::debugger;
using namespace facebook::hermes::inspector_modern::chrome;
using namespace facebook::hermes;
using namespace facebook;
using namespace std::placeholders;

constexpr auto kDefaultUrl = "url";

class CDPAgentTest : public ::testing::Test {
 public:
  void handleRuntimeTask(RuntimeTask task) {
    runtimeThread_->add([this, task]() { task(*runtime_); });
  }

  void handleResponse(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    messages_.push(message);
    hasMessage_.notify_one();
  }

 protected:
  void SetUp() override;
  void TearDown() override;

  void scheduleScript(
      const std::string &script,
      const std::string &url = kDefaultUrl,
      facebook::hermes::HermesRuntime::DebugFlags flags =
          facebook::hermes::HermesRuntime::DebugFlags{}) {
    runtimeThread_->add([this, script, url, flags]() {
      runtime_->debugJavaScript(script, url, flags);
    });
  }

  /// waits for the next message of either kind (response or notification)
  /// from the debugger. returns the message. throws on timeout.
  std::string waitForMessage(
      std::string context = "reply",
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  void expectNothing();

  void sendAndCheckResponse(const std::string &method, int id);

  jsi::Value shouldStop(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<AsyncDebuggerAPI> asyncDebuggerAPI_;
  std::unique_ptr<SerialExecutor> runtimeThread_;
  std::unique_ptr<CDPAgent> cdpAgent_;

  std::atomic<bool> stopFlag_{};

  std::mutex mutex_;
  std::condition_variable hasMessage_;
  std::queue<std::string> messages_;
};

void CDPAgentTest::SetUp() {
  runtimeThread_ = std::make_unique<SerialExecutor>();

  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());
  runtime_->global().setProperty(
      *runtime_,
      "shouldStop",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "shouldStop"),
          0,
          std::bind(&CDPAgentTest::shouldStop, this, _1, _2, _3, _4)));

  asyncDebuggerAPI_ = AsyncDebuggerAPI::create(*runtime_);

  cdpAgent_ = CDPAgent::create(
      *runtime_,
      *asyncDebuggerAPI_,
      std::bind(&CDPAgentTest::handleRuntimeTask, this, _1),
      std::bind(&CDPAgentTest::handleResponse, this, _1));
}

void CDPAgentTest::TearDown() {
  // CDPAgent can be cleaned up from any thread and at any time without
  // synchronization with the runtime thread.
  cdpAgent_.reset();

  // Clean up AsyncDebuggerAPI and HermesRuntime on the runtime thread to avoid
  // tripping TSAN. This ensures all DomainAgents code will run on the runtime
  // thread if they're being executed by ~AsyncDebuggerAPI(). Technically you
  // could destroy things on non-runtime thread _IF_ you know for sure that's
  // ok, but TSAN doesn't know that.
  runtimeThread_->add([this]() {
    asyncDebuggerAPI_.reset();
    runtime_.reset();
  });

  runtimeThread_.reset();
}

std::string CDPAgentTest::waitForMessage(
    std::string context,
    std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);

  bool success = hasMessage_.wait_for(
      lock, timeout, [this]() -> bool { return !messages_.empty(); });

  if (!success) {
    throw std::runtime_error("timed out waiting for " + context);
  }

  std::string message = std::move(messages_.front());
  messages_.pop();
  return message;
}

void CDPAgentTest::expectNothing() {
  std::string message;
  try {
    message = waitForMessage();
  } catch (...) {
    // if no values are received it times out with an exception
    // so we can say that we've succeeded at seeing nothing
    return;
  }

  throw std::runtime_error(
      "received a notification but didn't expect one: " + message);
}

jsi::Value CDPAgentTest::shouldStop(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  return stopFlag_.load() ? jsi::Value(true) : jsi::Value(false);
}

void CDPAgentTest::sendAndCheckResponse(const std::string &method, int id) {
  std::string json =
      "{\"id\": " + std::to_string(id) + ", \"method\": \"" + method + "\"}";
  cdpAgent_->handleCommand(json);
  ensureOkResponse(waitForMessage(method), id);
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
          ensureErrorResponse(message, commandID);
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
          ensureErrorResponse(message, commandID);
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

TEST_F(CDPAgentTest, TestScriptsOnEnable) {
  int msgId = 1;

  // Add a script being run in the VM prior to Debugger.enable
  scheduleScript("true");

  // Verify that upon enable, we get notification of existing scripts
  sendAndCheckResponse("Debugger.enable", msgId++);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  sendAndCheckResponse("Debugger.disable", msgId++);

  sendAndCheckResponse("Debugger.enable", msgId++);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
}

TEST_F(CDPAgentTest, TestEnableWhenAlreadyPaused) {
  int msgId = 1;

  scheduleScript("true");

  // Before Debugger.enable, register another debug client and trigger a pause
  DebuggerEventCallbackID eventCallbackID;
  waitFor<bool>([this, &eventCallbackID](auto promise) {
    eventCallbackID = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) { promise->set_value(true); });
    runtime_->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
  });

  // At this point, the runtime thread is paused due to Explicit AsyncBreak

  sendAndCheckResponse("Debugger.enable", msgId++);
  ensureNotification(
      waitForMessage("Debugger.scriptParsed"), "Debugger.scriptParsed");

  // Verify that after Debugger.enable is processed, we'll automatically get a
  // Debugger.paused notification
  ensurePaused(
      waitForMessage("paused"),
      "other",
      {FrameInfo("global", 0, 1).setLineNumberMax(9)});

  asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID);

  asyncDebuggerAPI_->triggerInterrupt_TS([this](HermesRuntime &runtime) {
    asyncDebuggerAPI_->setNextCommand(Command::continueExecution());
  });

  ensureNotification(waitForMessage("Debugger.resumed"), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestScriptsOrdering) {
  int msgId = 1;
  std::vector<std::string> notifications;

  const int kNumScriptParsed = 10;

  sendAndCheckResponse("Debugger.enable", msgId++);

  // Trigger a bunch of scriptParsed notifications to later verify that they get
  // re-sent in the same order
  for (int i = 0; i < kNumScriptParsed; i++) {
    scheduleScript("true");
    std::string notification = waitForMessage();
    ensureNotification(notification, "Debugger.scriptParsed");
    notifications.push_back(notification);
  }

  // Make sure the same ordering is retained after a disable request
  sendAndCheckResponse("Debugger.disable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  for (int i = 0; i < kNumScriptParsed; i++) {
    std::string notification = waitForMessage();
    ensureNotification(notification, "Debugger.scriptParsed");
    EXPECT_EQ(notifications[i], notification);
  }
}

TEST_F(CDPAgentTest, TestBytecodeScript) {
  int msgId = 1;
  sendAndCheckResponse("Debugger.enable", msgId++);

  // Compile code without debug info so that the SourceLocation would be
  // invalid.
  std::string bytecode;
  EXPECT_TRUE(::hermes::compileJS(
      R"(
    true
  )",
      bytecode));

  runtimeThread_->add([this, bytecode]() {
    runtime_->evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(bytecode)),
        "url");
  });

  // Verify that invalid SourceLocations simply don't trigger scriptParsed
  // notifications
  expectNothing();
}

TEST_F(CDPAgentTest, TestAsyncPauseWhileRunning) {
  int msgId = 1;

  scheduleScript(R"(
    var accum = 10;

    while (!shouldStop()) {
      var a = 1;
      var b = 2;
      var c = a + b;

      accum += c;
    }                        // (line 9)

    var d = -accum;
  )");

  sendAndCheckResponse("Debugger.enable", msgId++);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // send some number of async pauses, make sure that we always stop before
  // the end of the loop on line 9
  for (int i = 0; i < 10; i++) {
    sendAndCheckResponse("Debugger.pause", msgId++);
    ensurePaused(
        waitForMessage(),
        "other",
        {FrameInfo("global", 0, 1).setLineNumberMax(9)});

    sendAndCheckResponse("Debugger.resume", msgId++);
    ensureNotification(waitForMessage(), "Debugger.resumed");
  }

  // break out of loop
  stopFlag_.store(true);
}

#endif // HERMES_ENABLE_DEBUGGER
