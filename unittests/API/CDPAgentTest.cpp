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
#include <hermes/Support/JSONEmitter.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/DomainAgent.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/tests/SerialExecutor.h>

#include "CDPJSONHelpers.h"

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <sys/resource.h>
#endif

using namespace facebook::hermes::cdp;
using namespace facebook::hermes::debugger;
using namespace facebook::hermes::inspector_modern::chrome;
using namespace facebook::hermes;
using namespace facebook;
using namespace std::placeholders;

constexpr auto kDefaultUrl = "url";

template <typename T>
T waitFor(
    std::function<void(std::shared_ptr<std::promise<T>>)> callback,
    const std::string &reason = "unknown") {
  auto promise = std::make_shared<std::promise<T>>();
  auto future = promise->get_future();

  callback(promise);

  auto status = future.wait_for(std::chrono::milliseconds(2500));
  if (status != std::future_status::ready) {
    throw std::runtime_error("triggerInterrupt didn't get executed: " + reason);
  }
  return future.get();
}

class CDPAgentTest : public ::testing::Test {
 public:
  void handleRuntimeTask(RuntimeTask task) {
    runtimeThread_->add([this, task]() { task(*runtime_); });
  }

  void handleResponse(const std::string &message) {
    std::lock_guard<std::mutex> lock(messageMutex_);
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
      try {
        runtime_->debugJavaScript(script, url, flags);
      } catch (jsi::JSError &error) {
        thrownExceptions_.push_back(error.getMessage());
      }
    });
  }

  void waitForScheduledScripts();

  /// waits for the next message of either kind (response or notification)
  /// from the debugger. returns the message. throws on timeout.
  std::string waitForMessage(
      std::string context = "reply",
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  void expectNothing();
  JSONObject *expectNotification(const std::string &method);
  JSONObject *expectResponse(const std::optional<std::string> &method, int id);

  void sendParameterlessRequest(const std::string &method, int id);
  void sendAndCheckResponse(const std::string &method, int id);

  jsi::Value shouldStop(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  jsi::Value signalTest(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  void waitForTestSignal(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<AsyncDebuggerAPI> asyncDebuggerAPI_;
  std::unique_ptr<SerialExecutor> runtimeThread_;
  std::unique_ptr<CDPAgent> cdpAgent_;

  std::atomic<bool> stopFlag_{};

  std::mutex testSignalMutex_;
  std::condition_variable testSignalCondition_;
  bool testSignalled_;

  std::mutex messageMutex_;
  std::condition_variable hasMessage_;
  std::queue<std::string> messages_;

  std::vector<std::string> thrownExceptions_;
  JSONScope jsonScope_;
};

void CDPAgentTest::SetUp() {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  // Give the runtime thread the same stack size as the main thread. The runtime
  // thread is the main thread of the HermesRuntime.
  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  runtimeThread_ = std::make_unique<SerialExecutor>(limit.rlim_cur);
#else
  runtimeThread_ = std::make_unique<SerialExecutor>();
#endif

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
  runtime_->global().setProperty(
      *runtime_,
      "signalTest",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "signalTest"),
          0,
          std::bind(&CDPAgentTest::signalTest, this, _1, _2, _3, _4)));

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

void CDPAgentTest::waitForScheduledScripts() {
  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([promise]() { promise->set_value(true); });
  });
}

std::string CDPAgentTest::waitForMessage(
    std::string context,
    std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(messageMutex_);

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

JSONObject *CDPAgentTest::expectNotification(const std::string &method) {
  std::string message = waitForMessage();
  JSONObject *notification = jsonScope_.parseObject(message);
  EXPECT_EQ(jsonScope_.getString(notification, {"method"}), method);
  return notification;
}

JSONObject *CDPAgentTest::expectResponse(
    const std::optional<std::string> &method,
    int id) {
  std::string message = waitForMessage();
  JSONObject *response = jsonScope_.parseObject(message);
  if (method) {
    EXPECT_EQ(jsonScope_.getString(response, {"method"}), *method);
  }
  EXPECT_EQ(jsonScope_.getNumber(response, {"id"}), id);
  return response;
}

jsi::Value CDPAgentTest::shouldStop(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  return stopFlag_.load() ? jsi::Value(true) : jsi::Value(false);
}

void CDPAgentTest::sendParameterlessRequest(const std::string &method, int id) {
  std::string command;
  llvh::raw_string_ostream commandStream{command};
  ::hermes::JSONEmitter json{commandStream};
  json.openDict();
  json.emitKeyValue("method", method);
  json.emitKeyValue("id", id);
  json.closeDict();
  commandStream.flush();
  cdpAgent_->handleCommand(command);
}

void CDPAgentTest::sendAndCheckResponse(const std::string &method, int id) {
  sendParameterlessRequest(method, id);
  ensureOkResponse(waitForMessage(), id);
}

jsi::Value CDPAgentTest::signalTest(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  {
    std::lock_guard<std::mutex> lock(testSignalMutex_);
    testSignalled_ = true;
  }
  testSignalCondition_.notify_one();
  return jsi::Value::undefined();
}

void CDPAgentTest::waitForTestSignal(std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(testSignalMutex_);
  testSignalCondition_.wait_for(
      lock, timeout, [this] { return testSignalled_; });
  EXPECT_TRUE(testSignalled_);
  testSignalled_ = false;
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

  // This needs to be a while-loop because Explicit AsyncBreak will only happen
  // while there is JS to run
  scheduleScript(R"(
    while (!shouldStop()) {
    }
  )");

  // Before Debugger.enable, register another debug client and trigger a pause
  DebuggerEventCallbackID eventCallbackID;
  waitFor<bool>(
      [this, &eventCallbackID](auto promise) {
        eventCallbackID = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
            [promise](
                HermesRuntime &runtime,
                AsyncDebuggerAPI &asyncDebugger,
                DebuggerEventType event) { promise->set_value(true); });
        runtime_->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
      },
      "wait on explicit pause");

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

  // At this point we know the runtime thread is paused, so it should be safe to
  // call setNextCommand() directly.
  asyncDebuggerAPI_->setNextCommand(Command::continueExecution());

  // After removing this callback, AsyncDebuggerAPI will still have another
  // callback registered by CDPAgent. Therefore, JS will not continue by itself.
  // But because of the setNextCommand() in the previous line, removing the
  // callback should wake up the runtime thread and check that a next command
  // has been set.
  asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID);

  ensureNotification(waitForMessage("Debugger.resumed"), "Debugger.resumed");

  // break out of loop
  stopFlag_.store(true);
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

TEST_F(CDPAgentTest, TestDebuggerStatement) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  // debugger; statement won't work unless Debugger domain is enabled
  scheduleScript(R"(
    var a = 1 + 2;
    debugger;       // [1] (line 2) hit debugger statement, resume
    var b = a / 2;
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 2) hit debugger statement, resume
  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestStepOver) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    function divideBy2(val) {
      return val / 2;
    }

    var a = 1 + 2;
    debugger;             // [1] (line 6) hit debugger statement, step over
    var b = divideBy2(a); // [2] (line 7) step over
    var c = a + b;        // [3] (line 8) resume
    var d = b - c;
    var e = c * d;
    var f = 10;
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 6): hit debugger statement, step over
  ensurePaused(waitForMessage(), "other", {{"global", 6, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] (line 7): step over
  ensurePaused(waitForMessage(), "other", {{"global", 7, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 8): resume
  ensurePaused(waitForMessage(), "other", {{"global", 8, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestStepIn) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    function addOne(val) {
      return val + 1;   // [3]: resume
    }

    var a = 1 + 2;
    debugger;           // [1] (line 6) hit debugger statement, step over
    var b = addOne(a);  // [2] (line 7) step in
    var c = a + b;
    var d = b - c;
    var e = c * d;
    var f = 10;
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 6): hit debugger statement, step over
  ensurePaused(waitForMessage(), "other", {{"global", 6, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] (line 7): step in
  ensurePaused(waitForMessage(), "other", {{"global", 7, 1}});
  sendAndCheckResponse("Debugger.stepInto", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 2): resume
  ensurePaused(waitForMessage(), "other", {{"addOne", 2, 2}, {"global", 7, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestStepOut) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    function addSquares(a, b) {
      var a2 = a * a;
      debugger;        // [1] (line 3) hit debugger statement, step over
      var b2 = b * b;  // [2] (line 4) step out
      return a2 + b2;
    }

    var c = addSquares(1, 2); // [3] (line 8) resume
    var d = c * c;
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 3) hit debugger statement, step over
  ensurePaused(
      waitForMessage(), "other", {{"addSquares", 3, 2}, {"global", 8, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] (line 4) step out
  ensurePaused(
      waitForMessage(), "other", {{"addSquares", 4, 2}, {"global", 8, 1}});
  sendAndCheckResponse("Debugger.stepOut", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 8): resume
  ensurePaused(waitForMessage(), "other", {{"global", 8, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestSetPauseOnExceptionsAll) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger; // [1] (line 1) initial pause, set throw on exceptions to 'All'

    try {
      var a = 123;
      throw new Error('Caught error'); // [2] line 5, pause on exception
    } catch (err) {
      // Do nothing.
    }

    throw new Error('Uncaught exception'); // [3] line 10, pause on exception
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 1) initial pause, set throw on exceptions to 'All'
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});
  std::string command;
  llvh::raw_string_ostream commandStream{command};
  ::hermes::JSONEmitter json{commandStream};
  json.openDict();
  json.emitKeyValue("method", "Debugger.setPauseOnExceptions");
  json.emitKeyValue("id", msgId);
  json.emitKey("params");
  json.openDict();
  json.emitKeyValue("state", "all");
  json.closeDict();
  json.closeDict();
  commandStream.flush();
  cdpAgent_->handleCommand(command);
  ensureOkResponse(waitForMessage(), msgId++);

  // Resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] line 5, pause on exception
  ensurePaused(waitForMessage(), "exception", {{"global", 5, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] line 10, pause on exception
  ensurePaused(waitForMessage(), "exception", {{"global", 10, 1}});

  // Send resume event and check that Hermes has thrown an exception.
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  waitForScheduledScripts();
  EXPECT_EQ(thrownExceptions_.size(), 1);
  EXPECT_EQ(thrownExceptions_.back(), "Uncaught exception");
}

TEST_F(CDPAgentTest, TestRuntimeEnable) {
  int msgId = 1;

  // Verify enable gets an "OK" response
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Verify the hard-coded execution context is announced.
  auto note = expectNotification("Runtime.executionContextCreated");
  EXPECT_EQ(
      jsonScope_.getNumber(note, {"params", "context", "id"}),
      kHermesExecutionContextId);
  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "context", "name"}), "hermes");

  // Verify disable gets an "OK" response
  sendAndCheckResponse("Runtime.disable", msgId++);
}

TEST_F(CDPAgentTest, RefuseDoubleRuntimeEnable) {
  int msgId = 1;
  sendAndCheckResponse("Runtime.enable", msgId++);
  ensureNotification(waitForMessage(), "Runtime.executionContextCreated");

  // Verify enabling a second time fails
  sendParameterlessRequest("Runtime.enable", msgId);
  ensureErrorResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, RefuseRuntimeOperationsWithoutEnable) {
  int msgId = 1;

  // Verify disabling fails before enabling
  sendParameterlessRequest("Runtime.disable", msgId);
  ensureErrorResponse(waitForMessage(), msgId++);

  // GetHeapUsage
  sendParameterlessRequest("Runtime.getHeapUsage", msgId);
  ensureErrorResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, GetHeapUsage) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);
  ensureNotification(waitForMessage(), "Runtime.executionContextCreated");

  scheduleScript(R"(
    // Allocate some objects
    var a = [];
    for (var i = 0; i < 100; i++) {
      a[i] = new Object;
    }

    // Tell the test the allocation is complete
    signalTest();
    // Wait for the test to get heap usage
    while (!shouldStop()) {}

    // Reference 'a' to keep allocations alive until the test has requested
    // the heap usage.
    print(a);
  )");

  // Wait to reach the sampling point
  waitForTestSignal();

  // Get heap usage while the script is running
  sendParameterlessRequest("Runtime.getHeapUsage", msgId);

  // getHeapUsage response does not include the method name
  auto resp = expectResponse(std::nullopt, msgId++);

  // Some memory should be in use. We don't know how much, but it should be
  // more than 0.
  EXPECT_GT(jsonScope_.getNumber(resp, {"result", "usedSize"}), 0);
  EXPECT_GT(jsonScope_.getNumber(resp, {"result", "totalSize"}), 0);

  // Let the script terminate
  stopFlag_.store(true);
}

#endif // HERMES_ENABLE_DEBUGGER
