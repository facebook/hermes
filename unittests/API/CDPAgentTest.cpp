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
  static constexpr int32_t kTestExecutionContextId = 1;

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

  void sendRequest(
      const std::string &method,
      int id,
      const std::function<void(::hermes::JSONEmitter &)> &setParameters = {});
  void sendParameterlessRequest(const std::string &method, int id);
  void sendAndCheckResponse(const std::string &method, int id);
  void sendEvalRequest(int id, int callFrameId, const std::string &expression);

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
  bool testSignalled_ = false;

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
      kTestExecutionContextId,
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

void CDPAgentTest::sendRequest(
    const std::string &method,
    int id,
    const std::function<void(::hermes::JSONEmitter &)> &setParameters) {
  std::string command;
  llvh::raw_string_ostream commandStream{command};
  ::hermes::JSONEmitter json{commandStream};
  json.openDict();
  json.emitKeyValue("method", method);
  json.emitKeyValue("id", id);
  if (setParameters) {
    json.emitKey("params");
    json.openDict();
    setParameters(json);
    json.closeDict();
  }
  json.closeDict();
  commandStream.flush();
  cdpAgent_->handleCommand(command);
}

void CDPAgentTest::sendParameterlessRequest(const std::string &method, int id) {
  sendRequest(method, id);
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

void CDPAgentTest::sendEvalRequest(
    int id,
    int callFrameId,
    const std::string &expression) {
  std::string command;
  llvh::raw_string_ostream commandStream{command};
  ::hermes::JSONEmitter json{commandStream};
  json.openDict();
  json.emitKeyValue("method", "Debugger.evaluateOnCallFrame");
  json.emitKeyValue("id", id);
  json.emitKey("params");
  json.openDict();
  json.emitKeyValue("callFrameId", std::to_string(callFrameId));
  json.emitKeyValue("expression", expression);
  json.emitKeyValue("generatePreview", true);
  json.closeDict();
  json.closeDict();
  commandStream.flush();
  cdpAgent_->handleCommand(command);
}

TEST_F(CDPAgentTest, IssuesStartupTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  // Trigger the startup task
  auto cdpAgent = CDPAgent::create(
      kTestExecutionContextId,
      *runtime_,
      *asyncDebuggerAPI_,
      handleTask,
      handleMessage);

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, IssuesShutdownTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  auto cdpAgent = CDPAgent::create(
      kTestExecutionContextId,
      *runtime_,
      *asyncDebuggerAPI_,
      handleTask,
      handleMessage);

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
      kTestExecutionContextId,
      *runtime_,
      *asyncDebuggerAPI_,
      handleTask,
      handleMessage);

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
        kTestExecutionContextId,
        *runtime_,
        *asyncDebuggerAPI_,
        handleTask,
        handleMessage);

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
        kTestExecutionContextId,
        *runtime_,
        *asyncDebuggerAPI_,
        handleTask,
        handleMessage);

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
  // call resumeFromPaused() directly.
  asyncDebuggerAPI_->resumeFromPaused(AsyncDebugCommand::Continue);

  // After removing this callback, AsyncDebuggerAPI will still have another
  // callback registered by CDPAgent. Therefore, JS will not continue by itself.
  // But because of the resumeFromPaused() in the previous line, removing the
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

TEST_F(CDPAgentTest, TestEvalOnCallFrame) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    var globalVar = "omega";
    var booleanVar = true;
    var numberVar = 42;
    var objectVar = {number: 1, bool: false, str: "string"};

    function func1(closure, f1param) { // frame 4
        var f1v1 = "alpha";
        var f1v2 = "beta";
        function func1b() {            // frame 3
            var f1bv1 = "gamma";
            function func1c() {        // frame 2
                var f1cv1 = 19;
                closure();
            }
            func1c();
        }
        func1b();
    }

    function func2() {                 // frame 1
        var f2v1 = "baker";
        var f2v2 = "charlie";
        function func2b() {            // frame 0
            var f2bv1 = "dog";
            debugger;                  // [1] (line 25) hit debugger statement
                                       // [2]           run evals
                                       // [3]           resume
            print(globalVar);
            print(f2bv1);
        }
        func2b();
    }

    func1(func2, "tau");
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 25) hit debugger statement
  ensurePaused(
      waitForMessage(),
      "other",
      {{"func2b", 25, 3},
       {"func2", 31, 2},
       {"func1c", 13, 4},
       {"func1b", 15, 3},
       {"func1", 17, 2},
       {"global", 34, 1}});

  // [2] run eval statements
  int frame = 0;
  sendEvalRequest(msgId + 0, frame, R"("0: " + globalVar)");
  sendEvalRequest(msgId + 1, frame, R"("1: " + f2bv1)");
  sendEvalRequest(msgId + 2, frame, R"("2: " + f2v2)");
  sendEvalRequest(msgId + 3, frame, R"("3: " + f2bv1 + " && " + f2v2)");
  ensureEvalResponse(waitForMessage(), msgId + 0, "0: omega");
  ensureEvalResponse(waitForMessage(), msgId + 1, "1: dog");
  ensureEvalResponse(waitForMessage(), msgId + 2, "2: charlie");
  ensureEvalResponse(waitForMessage(), msgId + 3, "3: dog && charlie");
  msgId += 4;

  frame = 1;
  sendEvalRequest(msgId + 0, frame, R"("4: " + f2v1)");
  sendEvalRequest(msgId + 1, frame, R"("5: " + f2v2)");
  sendEvalRequest(msgId + 2, frame, R"(globalVar = "mod by debugger")");
  ensureEvalResponse(waitForMessage(), msgId + 0, "4: baker");
  ensureEvalResponse(waitForMessage(), msgId + 1, "5: charlie");
  ensureEvalResponse(waitForMessage(), msgId + 2, "mod by debugger");
  msgId += 3;

  frame = 2;
  sendEvalRequest(msgId + 0, frame, R"("6: " + f1cv1 + f1bv1 + f1v1)");
  sendEvalRequest(msgId + 1, frame, R"("7: " + globalVar)");
  ensureEvalResponse(waitForMessage(), msgId + 0, "6: 19gammaalpha");
  ensureEvalResponse(waitForMessage(), msgId + 1, "7: mod by debugger");
  msgId += 2;

  // [2.1] run eval statements that return non-string primitive values
  frame = 0;
  sendEvalRequest(msgId + 0, frame, "booleanVar");
  sendEvalRequest(msgId + 1, frame, "numberVar");
  ensureEvalResponse(waitForMessage(), msgId + 0, true);
  ensureEvalResponse(waitForMessage(), msgId + 1, 42);
  msgId += 2;

  /* TODO: This needs Runtime domain capability
  // [2.2] run eval statement that returns object
  frame = 0;
  sendEvalRequest(msgId + 0, frame, "objectVar");
  ensureEvalResponse(
      waitForMessage(),
      msgId + 0,
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"str", PropInfo("string").setValue("\"string\"")},
       {"__proto__", PropInfo("object")}});

  // msgId is increased by 2 because expectEvalResponse will make additional
  // request with expectProps.
  msgId += 2;
  */

  // [3] resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestEvalOnCallFrameException) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    var count = 0;

    function eventuallyThrows(x) {
      if (x <= 0)
        throw new Error("I frew up.");
      count++;
      eventuallyThrows(x-1);
    }

    function callme() {
      print("Hello");
      debugger;          // [1] (line 12) hit debugger statement
                         // [2]           run evals
                         // [3]           resume
      print("Goodbye");
    }

    callme();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 12) hit debugger statement
  ensurePaused(
      waitForMessage(), "other", {{"callme", 12, 2}, {"global", 18, 1}});

  // [2] run evals
  int frame = 0;
  sendEvalRequest(msgId + 0, frame, "this is not valid javascript");
  sendEvalRequest(msgId + 1, frame, "eventuallyThrows(5)");
  sendEvalRequest(msgId + 2, frame, "count");

  ensureEvalException(
      waitForMessage(), msgId + 0, "SyntaxError: 1:6:';' expected", {});
  ensureEvalException(
      waitForMessage(),
      msgId + 1,
      "Error: I frew up.",
      {{"eventuallyThrows", 5, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},

       // TODO: unsure why these frames are here, but they're in hdb tests
       // too. Ask Hermes about if they really should be there.
       FrameInfo("eval", 0, 0).setLineNumberMax(19),
       FrameInfo("callme", 12, 2),
       FrameInfo("global", 0, 0).setLineNumberMax(19)});
  ensureEvalResponse(waitForMessage(), msgId + 2, 5);
  msgId += 3;

  // [3] resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestSetBreakpointById) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger;      // line 1
    Math.random(); //      2
  )");

  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  std::string command;
  llvh::raw_string_ostream commandStream{command};
  ::hermes::JSONEmitter json{commandStream};
  json.openDict();
  json.emitKeyValue("method", "Debugger.setBreakpoint");
  json.emitKeyValue("id", msgId);
  json.emitKey("params");
  json.openDict();
  json.emitKey("location");
  json.openDict();
  json.emitKeyValue("scriptId", scriptID);
  json.emitKeyValue("lineNumber", 2);
  json.closeDict();
  json.closeDict();
  json.closeDict();
  commandStream.flush();
  cdpAgent_->handleCommand(command);

  ensureSetBreakpointResponse(waitForMessage(), msgId++, scriptID, 2, 4);

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, TestRemoveBreakpoint) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    // [1] (line 2) hit debugger statement, set breakpoint on line 7
    debugger;
    var a = 1;

    for (var i = 1; i <= 2; i++) {
      // [1] (line 7) hit breakpoint and then remove it
      a += i;
    }
  )");

  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  // [1] (line 2) hit debugger statement, set breakpoint on line 7
  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});

  {
    std::string command;
    llvh::raw_string_ostream commandStream{command};
    ::hermes::JSONEmitter json{commandStream};
    json.openDict();
    json.emitKeyValue("method", "Debugger.setBreakpoint");
    json.emitKeyValue("id", msgId);
    json.emitKey("params");
    json.openDict();
    json.emitKey("location");
    json.openDict();
    json.emitKeyValue("scriptId", scriptID);
    json.emitKeyValue("lineNumber", 7);
    json.closeDict();
    json.closeDict();
    json.closeDict();
    commandStream.flush();
    cdpAgent_->handleCommand(command);
  }
  auto resp = expectResponse(std::nullopt, msgId++);
  EXPECT_EQ(
      jsonScope_.getString(resp, {"result", "actualLocation", "scriptId"}),
      scriptID);
  EXPECT_EQ(
      jsonScope_.getNumber(resp, {"result", "actualLocation", "lineNumber"}),
      7);
  EXPECT_EQ(
      jsonScope_.getNumber(resp, {"result", "actualLocation", "columnNumber"}),
      6);
  auto breakpointId = jsonScope_.getString(resp, {"result", "breakpointId"});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [1] (line 7) hit breakpoint and then remove it
  ensurePaused(waitForMessage(), "other", {{"global", 7, 1}});

  {
    std::string command;
    llvh::raw_string_ostream commandStream{command};
    ::hermes::JSONEmitter json{commandStream};
    json.openDict();
    json.emitKeyValue("method", "Debugger.removeBreakpoint");
    json.emitKeyValue("id", msgId);
    json.emitKey("params");
    json.openDict();
    json.emitKeyValue("breakpointId", breakpointId);
    json.closeDict();
    json.closeDict();
    commandStream.flush();
    cdpAgent_->handleCommand(command);
  }
  ensureOkResponse(waitForMessage(), msgId++);

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Make sure the script runs to finish after this, which indicates it didn't
  // get stopped by breakpoint in the second iteration of the loop.
  waitForScheduledScripts();
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

  // Disable
  sendParameterlessRequest("Runtime.disable", msgId);
  ensureErrorResponse(waitForMessage(), msgId++);

  // GetHeapUsage
  sendParameterlessRequest("Runtime.getHeapUsage", msgId);
  ensureErrorResponse(waitForMessage(), msgId++);

  // GlobalLexicalScopeNames
  sendParameterlessRequest("Runtime.globalLexicalScopeNames", msgId);
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

TEST_F(CDPAgentTest, RuntimeGlobalLexicalScopeNames) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);
  ensureNotification(waitForMessage(), "Runtime.executionContextCreated");

  scheduleScript(R"(
    // Declare some globals to get the names of
    let globalLet = "let";
    const globalConst = "const";
    var globalVar = "var";

    // Call some functions with locals that should not show up in the list of
    // globals
    let func1 = () => {
      let local1 = 111;
      func2();
    }

    function func2() {
      let func3 = () => {
        let local3 = 333;
        // Tell the test we're here
        signalTest();
        // Wait for the test to inspect the state
        while (!shouldStop()) {}
      }

      let local2 = 222;
      func3();
    }

    func1();
  )");

  waitForTestSignal();

  sendRequest(
      "Runtime.globalLexicalScopeNames",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("executionContextId", kHermesExecutionContextId);
      });

  auto resp = expectResponse(std::nullopt, msgId++);

  // Check for the "let" and "const" variables, excluding the "var" variable.
  std::vector<std::string> expectedNames{"globalLet", "globalConst", "func1"};
  uint32_t index = 0;
  for (const std::string &expectedName : expectedNames) {
    std::string name = jsonScope_.getString(
        resp, {"result", "names", std::to_string(index++)});
    EXPECT_EQ(name, expectedName);
  }

  // Let the script terminate
  stopFlag_.store(true);
}

#endif // HERMES_ENABLE_DEBUGGER
