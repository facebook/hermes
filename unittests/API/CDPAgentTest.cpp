/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/CompileJS.h>
#include <hermes/Support/JSONEmitter.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/CDPDebugAPI.h>
#include <hermes/cdp/JSONValueInterfaces.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/tests/SerialExecutor.h>

#include <llvh/ADT/ScopeExit.h>

#include "CDPJSONHelpers.h"

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <sys/resource.h>
#endif

using namespace facebook::hermes::cdp;
using namespace facebook::hermes::debugger;
using namespace facebook::hermes;
using namespace facebook;
using namespace std::chrono_literals;
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

m::runtime::CallArgument makeValueCallArgument(std::string val) {
  m::runtime::CallArgument ret;
  ret.value = val;
  return ret;
}

m::runtime::CallArgument makeUnserializableCallArgument(std::string val) {
  m::runtime::CallArgument ret;
  ret.unserializableValue = std::move(val);
  return ret;
}

m::runtime::CallArgument makeObjectIdCallArgument(
    m::runtime::RemoteObjectId objectId) {
  m::runtime::CallArgument ret;
  ret.objectId = std::move(objectId);
  return ret;
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
  static constexpr int32_t kTestExecutionContextId_ = 1;

  void SetUp() override;
  void TearDown() override;

  void setupRuntimeTestInfra();

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
  /// Wait for a message, validate that it is an error with the specified
  /// \p messageID, and assert that the error description contains the
  /// specified \p substring.
  void expectErrorMessageContaining(
      const std::string &substring,
      long long messageID);

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

  std::unordered_map<std::string, std::string> getAndEnsureProps(
      int msgId,
      const std::string &objectId,
      const std::unordered_map<std::string, PropInfo> &infos,
      bool ownProperties = true);

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<CDPDebugAPI> cdpDebugAPI_;
  std::unique_ptr<facebook::hermes::inspector_modern::chrome::SerialExecutor>
      runtimeThread_;
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
  setupRuntimeTestInfra();

  cdpAgent_ = CDPAgent::create(
      kTestExecutionContextId_,
      *cdpDebugAPI_,
      std::bind(&CDPAgentTest::handleRuntimeTask, this, _1),
      std::bind(&CDPAgentTest::handleResponse, this, _1));
}

void CDPAgentTest::setupRuntimeTestInfra() {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  // Give the runtime thread the same stack size as the main thread. The runtime
  // thread is the main thread of the HermesRuntime.
  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  runtimeThread_ = std::make_unique<
      facebook::hermes::inspector_modern::chrome::SerialExecutor>(
      limit.rlim_cur);
#else
  runtimeThread_ = std::make_unique<
      facebook::hermes::inspector_modern::chrome::SerialExecutor>();
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

  cdpDebugAPI_ = CDPDebugAPI::create(*runtime_);
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
    cdpDebugAPI_.reset();
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

void CDPAgentTest::expectErrorMessageContaining(
    const std::string &substring,
    long long messageID) {
  std::string errorMessage = ensureErrorResponse(waitForMessage(), messageID);
  ASSERT_NE(errorMessage.find(substring), std::string::npos);
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

std::unordered_map<std::string, std::string> CDPAgentTest::getAndEnsureProps(
    int msgId,
    const std::string &objectId,
    const std::unordered_map<std::string, PropInfo> &infos,
    bool ownProperties) {
  sendRequest(
      "Runtime.getProperties",
      msgId,
      [objectId, ownProperties](::hermes::JSONEmitter &json) {
        json.emitKeyValue("objectId", objectId);
        json.emitKeyValue("ownProperties", ownProperties);
      });
  return ensureProps(waitForMessage(), infos);
}

TEST_F(CDPAgentTest, CDPAgentIssuesStartupTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  // Trigger the startup task
  auto cdpAgent = CDPAgent::create(
      kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, CDPAgentIssuesShutdownTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  auto cdpAgent = CDPAgent::create(
      kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

  // Ignore the startup task
  gotTask = false;

  // Trigger the shutdown task
  cdpAgent.reset();

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, CDPAgentIssuesCommandHandlingTask) {
  bool gotTask = false;
  EnqueueRuntimeTaskFunc handleTask = [&gotTask](RuntimeTask task) {
    gotTask = true;
  };

  OutboundMessageFunc handleMessage = [](const std::string &message) {};

  auto cdpAgent = CDPAgent::create(
      kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

  // Ignore the startup task
  gotTask = false;

  // Trigger the command-handling task
  cdpAgent->handleCommand(R"({"id": 1, "method": "Runtime.enable"})");

  ASSERT_TRUE(gotTask);
}

TEST_F(CDPAgentTest, CDPAgentRejectsMalformedJson) {
  std::unique_ptr<CDPAgent> cdpAgent;

  waitFor<bool>([this, &cdpAgent](auto promise) {
    OutboundMessageFunc handleMessage = [this,
                                         promise](const std::string &message) {
      // Ensure the invalid JSON is reported
      JSONObject *resp = jsonScope_.parseObject(message);
      EXPECT_EQ(
          jsonScope_.getString(resp, {"error", "message"}), "Malformed JSON");
      promise->set_value(true);
    };

    EnqueueRuntimeTaskFunc handleTask = [this](RuntimeTask task) {
      runtimeThread_->add([this, task]() { task(*runtime_); });
    };
    cdpAgent = CDPAgent::create(
        kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

    // Send a command that's not valid JSON
    cdpAgent->handleCommand("_");
  });
}

TEST_F(CDPAgentTest, CDPAgentRejectsMalformedMethods) {
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
        kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

    // Send a command with no domain delimiter in the method. Just format the
    // JSON manually, as there is no Request object for this fake method.
    cdpAgent->handleCommand(
        R"({"id": )" + std::to_string(commandID) +
        R"(, "method": "MethodWithoutADomainDelimiter"})");
  });
}

TEST_F(CDPAgentTest, CDPAgentRejectsUnknownDomains) {
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
        kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

    // Send a command with a properly-formatted domain, but unrecognized by the
    // CDP Agent. Just format the JSON manually, as there is no Request object
    // for this fake domain.
    cdpAgent->handleCommand(
        R"({"id": )" + std::to_string(commandID) +
        R"(, "method": "FakeDomain.enable"})");
  });
}

TEST_F(CDPAgentTest, CDPAgentCanReenter) {
  std::unique_ptr<CDPAgent> cdpAgent;

  waitFor<bool>([this, &cdpAgent](auto promise) {
    int commandID = 1;

    OutboundMessageFunc handleMessage = [promise,
                                         &cdpAgent](const std::string &) {
      // Re-enter the CDP Agent
      cdpAgent->getState();
      promise->set_value(true);
    };

    EnqueueRuntimeTaskFunc handleTask = [this](RuntimeTask task) {
      runtimeThread_->add([this, task]() { task(*runtime_); });
    };
    cdpAgent = CDPAgent::create(
        kTestExecutionContextId_, *cdpDebugAPI_, handleTask, handleMessage);

    // Send a command that's not handled, triggering a callback with the error
    // response.
    cdpAgent->handleCommand(
        R"({"id": )" + std::to_string(commandID) +
        R"(, "method": "Unsupported.Message"})");
  });
}

TEST_F(CDPAgentTest, DebuggerAllowDoubleEnable) {
  int msgId = 1;
  sendAndCheckResponse("Debugger.enable", msgId++);

  // Verify enabling a second time succeeds
  sendAndCheckResponse("Debugger.enable", msgId++);
}

TEST_F(CDPAgentTest, DebuggerAllowDoubleDisable) {
  int msgId = 1;
  sendAndCheckResponse("Debugger.enable", msgId++);

  sendAndCheckResponse("Debugger.disable", msgId++);

  // Verify disabling a second time succeeds
  sendAndCheckResponse("Debugger.disable", msgId++);
}

TEST_F(CDPAgentTest, DebuggerDestroyWhileEnabled) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger;      // line 1
    Math.random(); //      2
  )");

  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  sendRequest(
      "Debugger.setBreakpoint", msgId, [scriptID](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptID);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId++, {scriptID, 2, 4});

  // Finally, destroy CDPAgent without first disable the Debugger domain. Then
  // verify things are cleaned up from the HermesRuntime and AsyncDebuggerAPI
  // properly.
  cdpAgent_.reset();

  // Queue a job on the runtime queue. The runtime queue was busy paused on the
  // debugger statement on line 1, but the destruction of CDPAgent should
  // removeDebuggerEventCallback_TS() and thus free up the runtime queue.
  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      // Verify that breakpoints are cleaned up from HermesRuntime
      auto breakpoints = runtime_->getDebugger().getBreakpoints();
      EXPECT_EQ(breakpoints.size(), 0);

      // Verify pause on ScriptLoad state is reset as well
      EXPECT_FALSE(runtime_->getDebugger().getShouldPauseOnScriptLoad());

      promise->set_value(true);
    });
  });
}

TEST_F(CDPAgentTest, DebuggerScriptsOnEnable) {
  int msgId = 1;

  // Wait for a script to be run in the VM prior to Debugger.enable
  scheduleScript("signalTest();");
  waitForTestSignal();

  // Verify that upon enable, we get notification of existing scripts
  sendParameterlessRequest("Debugger.enable", msgId);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(waitForMessage(), msgId++);

  sendAndCheckResponse("Debugger.disable", msgId++);

  sendParameterlessRequest("Debugger.enable", msgId);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, DebuggerEnableWhenAlreadyPaused) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  // This needs to be a while-loop because Explicit AsyncBreak will only happen
  // while there is JS to run
  scheduleScript(R"(
    signalTest();
    while (!shouldStop()) {
    }
  )");
  // Wait for the script to start.
  waitForTestSignal();

  // Before Debugger.enable, register another debug client and trigger a pause
  DebuggerEventCallbackID eventCallbackID;
  AsyncDebuggerAPI &asyncDebuggerAPI = cdpDebugAPI_->asyncDebuggerAPI();
  waitFor<bool>(
      [this, &asyncDebuggerAPI, &eventCallbackID](auto promise) {
        eventCallbackID = asyncDebuggerAPI.addDebuggerEventCallback_TS(
            [promise](
                HermesRuntime &runtime,
                AsyncDebuggerAPI &asyncDebugger,
                DebuggerEventType event) {
              if (event == DebuggerEventType::ExplicitPause) {
                promise->set_value(true);
              }
            });
        runtime_->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
      },
      "wait on explicit pause");

  // At this point, the runtime thread is paused due to Explicit AsyncBreak. Now
  // we'll test if we can perform Debugger.enable while the runtime is in that
  // state.

  sendParameterlessRequest("Debugger.enable", msgId);
  ensureNotification(
      waitForMessage("Debugger.scriptParsed"), "Debugger.scriptParsed");

  // Verify that after Debugger.enable is processed, we'll automatically get a
  // Debugger.paused notification
  ensurePaused(
      waitForMessage("paused"),
      "other",
      {FrameInfo("global", 0, 1).setLineNumberMax(9)});

  ensureOkResponse(waitForMessage(), msgId++);

  // After removing this callback, AsyncDebuggerAPI will still have another
  // callback registered by CDPAgent. Therefore, JS will not continue by itself.
  asyncDebuggerAPI.removeDebuggerEventCallback_TS(eventCallbackID);
  // Have to manually resume it:
  waitFor<bool>([&asyncDebuggerAPI](auto promise) {
    asyncDebuggerAPI.triggerInterrupt_TS(
        [&asyncDebuggerAPI, promise](HermesRuntime &runtime) {
          asyncDebuggerAPI.resumeFromPaused(AsyncDebugCommand::Continue);
          promise->set_value(true);
        });
  });

  ensureNotification(waitForMessage("Debugger.resumed"), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerScriptsOrdering) {
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
  sendParameterlessRequest("Debugger.enable", msgId);
  for (int i = 0; i < kNumScriptParsed; i++) {
    std::string notification = waitForMessage();
    ensureNotification(notification, "Debugger.scriptParsed");
    EXPECT_EQ(notifications[i], notification);
  }
  ensureOkResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, DebuggerBytecodeScript) {
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

TEST_F(CDPAgentTest, DebuggerAsyncPauseWhileRunning) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  scheduleScript(R"(
    signalTest();
    var accum = 10;

    while (!shouldStop()) {
      var a = 1;
      var b = 2;
      var c = a + b;

      accum += c;
    }                        // (line 9)

    var d = -accum;
  )");
  // Wait for the script to start.
  waitForTestSignal();

  sendParameterlessRequest("Debugger.enable", msgId);
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(waitForMessage(), msgId++);

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
}

TEST_F(CDPAgentTest, DebuggerTestDebuggerStatement) {
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

TEST_F(CDPAgentTest, DebuggerStepOver) {
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
  ensurePaused(waitForMessage(), "step", {{"global", 7, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 8): resume
  ensurePaused(waitForMessage(), "step", {{"global", 8, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerStepOverThrow) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    try {
      var a = 1 + 2;
      debugger;             // [1] (line 3) hit debugger statement, step over
      throw new Error(a);   // [2] (line 4) step over
    } catch (e) {
      var b = a + a;        // [3] (line 6) resume
    }
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 3): hit debugger statement, step over
  ensurePaused(waitForMessage(), "other", {{"global", 3, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] (line 4): step over
  ensurePaused(waitForMessage(), "step", {{"global", 4, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 6): resume
  ensurePaused(waitForMessage(), "step", {{"global", 6, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerStepIn) {
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
  ensurePaused(waitForMessage(), "step", {{"global", 7, 1}});
  sendAndCheckResponse("Debugger.stepInto", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 2): resume
  ensurePaused(waitForMessage(), "step", {{"addOne", 2, 2}, {"global", 7, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerStepOut) {
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
      waitForMessage(), "step", {{"addSquares", 4, 2}, {"global", 8, 1}});
  sendAndCheckResponse("Debugger.stepOut", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 8): resume
  ensurePaused(waitForMessage(), "step", {{"global", 8, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerSetPauseOnExceptionsAll) {
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

TEST_F(CDPAgentTest, DebuggerEvalOnCallFrame) {
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

  // [2.2] run eval statement that returns object
  frame = 0;
  sendEvalRequest(msgId, frame, "objectVar");
  auto objectId = ensureObjectEvalResponse(waitForMessage(), msgId++);

  getAndEnsureProps(
      msgId++,
      objectId,
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"str", PropInfo("string").setValue("\"string\"")},
       {"__proto__", PropInfo("object")}});

  // [3] resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerEvalOnCallFrameException) {
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

TEST_F(CDPAgentTest, DebuggerSetBreakpointById) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger;      // line 1
    Math.random(); //      2
  )");

  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  sendRequest(
      "Debugger.setBreakpoint", msgId, [scriptID](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptID);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId++, {scriptID, 2, 4});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerSetBreakpointByUrl) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    var a = 1 + 2;
    debugger;      // [1] (line 2) hit debugger statement,
                   //     set breakpoint on line 5
    var b = a / 2;
    var c = a + b; // [2] (line 5) hit breakpoint, step over
    var d = b - c; // [3] (line 6) resume
    var e = c * d;
    var f = 10;
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 2) hit debugger statement, set breakpoint on line 6
  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});

  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", kDefaultUrl);
        json.emitKeyValue("lineNumber", 5);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {{5}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] (line 5) hit breakpoint, step over
  ensurePaused(waitForMessage(), "other", {{"global", 5, 1}});
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [3] (line 6) resume
  ensurePaused(waitForMessage(), "step", {{"global", 6, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

// This test specifically exercises the case when a breakpoint is placed on the
// return statement. In this case, we replace the Ret OpCode with a Debugger
// OpCode. There used to be a bug in this situation where it becomes impossible
// to move past the Ret OpCode.
TEST_F(CDPAgentTest, DebuggerBreakpointOnReturn) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger; // [1] (line 1) hit debugger statement, set breakpoint on line 3
    function demo() {
      return; // [2] (line 3) hit breakpoint
    }
    demo();
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 1) hit debugger statement, set breakpoint on line 3
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", kDefaultUrl);
        json.emitKeyValue("lineNumber", 3);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {{3}});

  // Resume and have the script call demo()
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Hit the breakpoint installed on the Ret OpCode
  ensurePaused(waitForMessage(), "other", {{"demo", 3, 2}, {"global", 5, 1}});

  // Expect doing a step over will get execution back out to the global scope
  sendAndCheckResponse("Debugger.stepOver", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensurePaused(waitForMessage(), "step", {{"global", 5, 1}});
}

TEST_F(CDPAgentTest, DebuggerSetMultiLocationBreakpoint) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(
      R"(
        function one() {
          var a = 1;
          var b = 2;
          var c = a + b;
        }
  )",
      "someFile");

  scheduleScript(
      R"(
        function two() {
          var d = 3;
          var e = 4;
          var f = d + e;
        }
  )",
      "someFile");

  scheduleScript(
      R"(
    debugger; // First break, line 1
    one();
    two();
    debugger; // Last break, line 4
  )",
      "doesntMatch");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // First break, the first "debugger;"
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  // Set a breakpoint that applies to two Hermes locations
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "someFile");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {{2}, {2}});

  // Continue so we can hit the breakpoints...
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Second break, breakpoint in function "one"
  ensurePaused(waitForMessage(), "other", {{"one", 2, 2}, {"global", 2, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Third break, breakpoint in function "two"
  ensurePaused(waitForMessage(), "other", {{"two", 2, 2}, {"global", 3, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Last break, the second "debugger;""
  ensurePaused(waitForMessage(), "other", {{"global", 4, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerDeleteMultiLocationBreakpoint) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(
      R"(
        function one() {
          var a = 1;
          var b = 2;
          var c = a + b;
        }
  )",
      "someFile");

  scheduleScript(
      R"(
        function two() {
          var d = 3;
          var e = 4;
          var f = d + e;
        }
  )",
      "someFile");

  scheduleScript(
      R"(
    debugger; // First break, line 1
    one();
    two();
    debugger; // Last break, line 4
  )",
      "doesntMatch");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // First break, the first "debugger;"
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  // Set a breakpoint that applies to two Hermes locations
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "someFile");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  auto breakpointId =
      ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {{2}, {2}});

  sendRequest(
      "Debugger.removeBreakpoint",
      msgId,
      [breakpointId](::hermes::JSONEmitter &json) {
        json.emitKeyValue("breakpointId", breakpointId);
      });
  ensureOkResponse(waitForMessage(), msgId++);

  // Continue so we can hit the breakpoints...
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Breakpoints that were set and deleted are not hit

  // Last break, the second "debugger;""
  ensurePaused(waitForMessage(), "other", {{"global", 4, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, DebuggerApplyBreakpointsToNewLoadedScripts) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  // Set a breakpoint that applies to zero Hermes locations
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "someFile");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  // Ensure the breakpoint was successfully created
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {});

  // Load a script that matches the breakpoint description
  scheduleScript(
      R"(
        var a = 1;
        var b = 2;
        var c = a + b;
  )",
      "someFile");
  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  // Ensure the breakpoint was applied
  auto resolution = expectNotification("Debugger.breakpointResolved");
  auto resolvedScriptID =
      jsonScope_.getString(resolution, {"params", "location", "scriptId"});
  auto resolvedLineNumber =
      jsonScope_.getNumber(resolution, {"params", "location", "lineNumber"});
  EXPECT_EQ(resolvedScriptID, scriptID);
  EXPECT_EQ(resolvedLineNumber, 2);

  // Ensure the breakpoint is hit
  ensurePaused(waitForMessage(), "other", {{"global", 2, 1}});
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Load a script that doesn't match the breakpoint description
  scheduleScript(
      R"(
        var a = 1;
        var b = 2;
        var c = a + b;
  )",
      "anotherFile");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // Ensure the breakpoint wasn't applied (i.e. no breakpoint resolved
  // notification and no breakpoints hit).
  expectNothing();
}

TEST_F(CDPAgentTest, DebuggerRemoveBreakpoint) {
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

TEST_F(CDPAgentTest, DebuggerRestoreState) {
  int msgId = 1;

  // First, create a breakpoint that will be persisted.
  sendAndCheckResponse("Debugger.enable", msgId++);
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", kDefaultUrl);
        json.emitKeyValue("lineNumber", 3);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {});

  for (int i = 0; i < 2; i++) {
    State state;
    if (i == 0) {
      // Save CDPAgent state on non-runtime thread and shut everything down.
      state = cdpAgent_->getState();
      cdpAgent_.reset();
      waitFor<bool>([this](auto promise) {
        runtimeThread_->add([this, promise]() {
          cdpDebugAPI_.reset();
          promise->set_value(true);
        });
      });
    } else {
      // Save CDPAgent state on the runtime thread and shut everything down.
      waitFor<bool>([this, &state](auto promise) {
        runtimeThread_->add([this, &state, promise]() {
          state = cdpAgent_->getState();
          cdpAgent_.reset();
          cdpDebugAPI_.reset();
          promise->set_value(true);
        });
      });
    }
    // Can't destroy runtime_ in the runtimeThread_ due to handleRuntimeTask()
    // still uses runtime_.
    runtimeThread_.reset();
    runtime_.reset();

    // Set everything up again, but with the persisted state this time for
    // CDPAgent.
    setupRuntimeTestInfra();
    cdpAgent_ = CDPAgent::create(
        kTestExecutionContextId_,
        *cdpDebugAPI_,
        std::bind(&CDPAgentTest::handleRuntimeTask, this, _1),
        std::bind(&CDPAgentTest::handleResponse, this, _1),
        std::move(state));

    sendAndCheckResponse("Debugger.enable", msgId++);
    scheduleScript(R"(
      var a = 1 + 2;
      var b = a / 2;
      var c = a + b; // (line 3) hit breakpoint
      var d = b - c;
      var e = c * d;
      var f = 10;
    )");
    ensureNotification(waitForMessage(), "Debugger.scriptParsed");

    // Check that CDPAgent was restored with previously set breakpoint and
    // pauses on new script
    auto resolution = expectNotification("Debugger.breakpointResolved");
    auto resolvedLineNumber =
        jsonScope_.getNumber(resolution, {"params", "location", "lineNumber"});
    EXPECT_EQ(resolvedLineNumber, 3);
    ensurePaused(waitForMessage(), "other", {{"global", 3, 1}});

    sendAndCheckResponse("Debugger.resume", msgId++);
    ensureNotification(waitForMessage(), "Debugger.resumed");
  }
}

TEST_F(CDPAgentTest, DebuggerDeactivateBreakpointsWhileDisabled) {
  int msgId = 1;
  sendRequest(
      "Debugger.setBreakpointsActive", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("active", false);
      });
  ensureOkResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, DebuggerActivateBreakpoints) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger;      // line 1
    x=100          //      2
    debugger;      //      3
    x=101;         //      4
  )");
  auto note = expectNotification("Debugger.scriptParsed");
  auto scriptID = jsonScope_.getString(note, {"params", "scriptId"});

  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  // Set breakpoint #1
  sendRequest(
      "Debugger.setBreakpoint", msgId, [scriptID](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptID);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureOkResponse(waitForMessage(), msgId++);

  // Set breakpoint #2
  sendRequest(
      "Debugger.setBreakpoint", msgId, [scriptID](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptID);
        json.emitKeyValue("lineNumber", 4);
        json.closeDict();
      });
  ensureOkResponse(waitForMessage(), msgId++);

  // Disable breakpoints
  sendRequest(
      "Debugger.setBreakpointsActive",
      msgId,
      [scriptID](::hermes::JSONEmitter &json) {
        json.emitKeyValue("active", false);
      });
  ensureOkResponse(waitForMessage(), msgId++);

  // Resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Expect first breakpoint to be skipped, now hitting line #3
  ensurePaused(waitForMessage(), "other", {{"global", 3, 1}});

  // Re-enable breakpoints
  sendRequest(
      "Debugger.setBreakpointsActive",
      msgId,
      [scriptID](::hermes::JSONEmitter &json) {
        json.emitKeyValue("active", true);
      });
  ensureOkResponse(waitForMessage(), msgId++);

  // Resume and expect breakpoints to trigger again
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensurePaused(waitForMessage(), "other", {{"global", 4, 1}});

  // Continue and exit
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, RuntimeEnableDisable) {
  int msgId = 1;

  // Verify enable gets an "OK" response
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Verify disable gets an "OK" response
  sendAndCheckResponse("Runtime.disable", msgId++);
}

TEST_F(CDPAgentTest, RuntimeAllowDoubleEnable) {
  int msgId = 1;
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Verify enabling a second time succeeds
  sendAndCheckResponse("Runtime.enable", msgId++);
}

TEST_F(CDPAgentTest, RuntimeRefuseOperationsWithoutEnable) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.disable", msgId++);

  sendRequest("Runtime.compileScript", msgId, [](::hermes::JSONEmitter &json) {
    json.emitKeyValue("persistScript", true);
    json.emitKeyValue("sourceURL", "none");
    json.emitKeyValue("expression", "1+1");
  });
  ensureErrorResponse(waitForMessage(), msgId++);
}

TEST_F(CDPAgentTest, RuntimeGetHeapUsage) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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
}

TEST_F(CDPAgentTest, RuntimeGlobalLexicalScopeNames) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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
        json.emitKeyValue("executionContextId", kTestExecutionContextId_);
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
}

TEST_F(CDPAgentTest, RuntimeGlobalLexicalScopeNamesOnEmptyStack) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  sendRequest(
      "Runtime.globalLexicalScopeNames",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("executionContextId", kTestExecutionContextId_);
      });

  // Can't get lexical scopes on an empty stack.
  ensureErrorResponse(waitForMessage(), msgId);
}

TEST_F(CDPAgentTest, RuntimeCompileScript) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  // Compile a valid script
  sendRequest("Runtime.compileScript", msgId, [](::hermes::JSONEmitter &json) {
    json.emitKeyValue("persistScript", true);
    json.emitKeyValue("sourceURL", "none");
    json.emitKeyValue("expression", "1+1");
  });

  // Expect success, and a unique identifier for the script
  auto resp = expectResponse(std::nullopt, msgId++);
  EXPECT_EQ("userScript0", jsonScope_.getString(resp, {"result", "scriptId"}));
}

TEST_F(CDPAgentTest, RuntimeCompileScriptParseError) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  // Compile an invalid script
  sendRequest("Runtime.compileScript", msgId, [](::hermes::JSONEmitter &json) {
    json.emitKeyValue("persistScript", true);
    json.emitKeyValue("sourceURL", "none");
    json.emitKeyValue("expression", "/oops");
  });

  // Expect it to be rejected with details about the compilation failure
  auto resp = expectResponse(std::nullopt, msgId++);
  EXPECT_GT(
      jsonScope_.getString(resp, {"result", "exceptionDetails", "text"}).size(),
      0);
}

TEST_F(CDPAgentTest, RuntimeGetProperties) {
  int msgId = 1;
  std::vector<std::string> objIds;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(
    function foo() {
      var num = 123;
      var obj = {
        "depth": 0,
        "value": {
          "a": -1/0,
          "b": 1/0,
          "c": Math.sqrt(-2),
          "d": -0,
          "e": "e_string"
        }
      };
      var arr = [1, 2, 3];
      function bar() {
        var num = 456;
        var obj = {"depth": 1, "value": {"c": 5, "d": "d_string"}};
        debugger; // First break location
      };
      bar();
      debugger; // Second break location
    }

    foo();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // Wait for first break location
  auto pausedNote = ensurePaused(
      waitForMessage(),
      "other",
      {{"bar", 17, 3}, {"foo", 19, 2}, {"global", 23, 1}});

  auto &scopeObj = pausedNote.callFrames.at(1).scopeChain.at(0).object;
  EXPECT_TRUE(scopeObj.objectId.has_value());
  std::string scopeObjId = scopeObj.objectId.value();
  objIds.push_back(scopeObjId);

  auto scopeChildren = getAndEnsureProps(
      msgId++,
      scopeObjId,
      {{"this", PropInfo("undefined")},
       {"num", PropInfo("number").setValue("123")},
       {"obj", PropInfo("object")},
       {"arr", PropInfo("object").setSubtype("array")},
       {"bar", PropInfo("function")}});
  EXPECT_EQ(scopeChildren.size(), 3);

  EXPECT_EQ(scopeChildren.count("obj"), 1);
  std::string objId = scopeChildren.at("obj");
  objIds.push_back(objId);

  auto objChildren = getAndEnsureProps(
      msgId++,
      objId,
      {{"depth", PropInfo("number").setValue("0")},
       {"value", PropInfo("object")},
       {"__proto__", PropInfo("object")}});
  EXPECT_EQ(objChildren.size(), 2);

  EXPECT_EQ(objChildren.count("value"), 1);
  std::string valueId = objChildren.at("value");
  objIds.push_back(valueId);

  auto valueChildren = getAndEnsureProps(
      msgId++,
      valueId,
      {{"a", PropInfo("number").setUnserializableValue("-Infinity")},
       {"b", PropInfo("number").setUnserializableValue("Infinity")},
       {"c", PropInfo("number").setUnserializableValue("NaN")},
       {"d", PropInfo("number").setUnserializableValue("-0")},
       {"e", PropInfo("string").setValue("\"e_string\"")},
       {"__proto__", PropInfo("object")}});
  EXPECT_EQ(valueChildren.size(), 1);

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  ensurePaused(waitForMessage(), "other", {{"foo", 20, 2}, {"global", 23, 1}});

  // all old object ids should be invalid after resuming
  for (std::string oldObjId : objIds) {
    getAndEnsureProps(
        msgId++, oldObjId, std::unordered_map<std::string, PropInfo>{});
  }

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, RuntimeGetPropertiesOnlyOwn) {
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(
    function foo() {
      var protoObject = {
        "protoNum": 77
      };
      var obj = Object.create(protoObject);
      obj.num = 42;
      debugger;
    }
    foo();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // wait for a pause on debugger statement and get object ID from the local
  // scope.
  auto pausedNote = ensurePaused(
      waitForMessage(), "other", {{"foo", 7, 2}, {"global", 9, 1}});
  const auto &scopeObject = pausedNote.callFrames.at(0).scopeChain.at(0).object;
  auto scopeChildren = getAndEnsureProps(
      msgId++,
      scopeObject.objectId.value(),
      {{"this", PropInfo("undefined")},
       {"obj", PropInfo("object")},
       {"protoObject", PropInfo("object")}});
  EXPECT_EQ(scopeChildren.count("obj"), 1);
  std::string objId = scopeChildren.at("obj");

  // Check that GetProperties request for obj object only have own properties
  // when onlyOwnProperties = true.
  getAndEnsureProps(
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")},
       {"__proto__", PropInfo("object")}},
      true);

  // Check that GetProperties request for obj object only have all properties
  // when onlyOwnProperties = false.
  // __proto__ is not returned here because all properties from proto chain
  // are already included in the result.
  getAndEnsureProps(
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")},
       {"protoNum", PropInfo("number").setValue("77")}},
      false);

  // resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, RuntimeEvaluate) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  scheduleScript(R"(
    var globalVar = "omega";
    var booleanVar = true;
    var numberVar = 42;
    var objectVar = {number: 1, bool: false, str: "string"};

    while(!shouldStop()) {  // [1] (line 6) hit infinite loop
      var a = 1;            // [2] run evals
      a++;                  // [3] exit run loop
    }
  )");

  // [1] (line 6) hit infinite loop

  // [2] run eval statements
  sendRequest("Runtime.evaluate", msgId + 0, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"("0: " + globalVar)");
  });
  auto resp0 = expectResponse(std::nullopt, msgId + 0);
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "type"}), "string");
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "value"}), "0: omega");

  // [2.1] run eval statements that return non-string primitive values
  sendRequest("Runtime.evaluate", msgId + 1, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "booleanVar");
  });
  sendRequest("Runtime.evaluate", msgId + 2, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "numberVar");
  });
  auto resp1 = expectResponse(std::nullopt, msgId + 1);
  EXPECT_EQ(
      jsonScope_.getString(resp1, {"result", "result", "type"}), "boolean");
  EXPECT_EQ(jsonScope_.getBoolean(resp1, {"result", "result", "value"}), true);
  auto resp2 = expectResponse(std::nullopt, msgId + 2);
  EXPECT_EQ(
      jsonScope_.getString(resp2, {"result", "result", "type"}), "number");
  EXPECT_EQ(jsonScope_.getNumber(resp2, {"result", "result", "value"}), 42);

  // [2.2] run eval statement that returns object
  sendRequest("Runtime.evaluate", msgId + 3, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "objectVar");
  });
  auto resp3 = expectResponse(std::nullopt, msgId + 3);
  EXPECT_EQ(
      jsonScope_.getString(resp3, {"result", "result", "type"}), "object");
  EXPECT_EQ(
      jsonScope_.getString(resp3, {"result", "result", "className"}), "Object");
  std::string objId =
      jsonScope_.getString(resp3, {"result", "result", "objectId"});
  getAndEnsureProps(
      msgId + 3,
      objId,
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"str", PropInfo("string").setValue("\"string\"")},
       {"__proto__", PropInfo("object")}},
      true);
}

TEST_F(CDPAgentTest, RuntimeEvaluateWhilePaused) {
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(
    var inGlobalScope = 123;
    (function func() {
      var inFunctionScope = 456;
      debugger;
    })();
  )");
  expectNotification("Debugger.scriptParsed");

  auto pausedNote = ensurePaused(
      waitForMessage(), "other", {{"func", 4, 2}, {"global", 5, 1}});

  // Evaluate the global variable; it should be visible to the runtime
  // evaluation.
  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "inGlobalScope");
  });
  auto resp0 = expectResponse(std::nullopt, msgId++);
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "type"}), "number");
  EXPECT_EQ(jsonScope_.getNumber(resp0, {"result", "result", "value"}), 123);

  // Evaluate the local variable; it should not be visible to the runtime
  // evaluation.
  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "inFunctionScope");
  });
  auto resp1 = expectResponse(std::nullopt, msgId++);
  EXPECT_GT(
      jsonScope_.getString(resp1, {"result", "exceptionDetails", "text"})
          .size(),
      0);

  // Let the script terminate
  sendAndCheckResponse("Debugger.resume", msgId++);
}

TEST_F(CDPAgentTest, RuntimeEvaluateReturnByValue) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  scheduleScript(R"(while(!shouldStop());)");

  // We expect this JSON object to be evaluated and return by value, so
  // that JSON encoding the result will give the same string.
  auto object = "{\"key\":[1,\"two\"]}";
  auto preview =
      "{\"description\":\"Object\",\"overflow\":false,\"properties\":[{\"name\":\"key\",\"subtype\":\"array\",\"type\":\"object\",\"value\":\"Array(2)\"}],\"type\":\"object\"}";

  sendRequest(
      "Runtime.evaluate", msgId, [object](::hermes::JSONEmitter &params) {
        params.emitKeyValue("expression", std::string("(") + object + ")");
        params.emitKeyValue("returnByValue", true);
        params.emitKeyValue("generatePreview", true);
      });

  auto resp = expectResponse(std::nullopt, msgId + 0);
  EXPECT_EQ(jsonScope_.getString(resp, {"result", "result", "type"}), "object");
  EXPECT_EQ(
      jsonScope_.getString(resp, {"result", "result", "preview", "type"}),
      "object");
  EXPECT_TRUE(jsonValsEQ(
      jsonScope_.getObject(resp, {"result", "result", "preview"}),
      jsonScope_.parseObject(preview)));
  EXPECT_TRUE(jsonValsEQ(
      jsonScope_.getObject(resp, {"result", "result", "value"}),
      jsonScope_.parseObject(object)));
}

TEST_F(CDPAgentTest, RuntimeEvaluateException) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  scheduleScript(R"(while(!shouldStop()) {})");

  // Evaluate something that throws
  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"((undefined)();)");
  });
  auto resp = expectResponse(std::nullopt, msgId++);

  // Ensure the exception (remote) object and text were delivered
  EXPECT_GT(
      jsonScope_
          .getString(
              resp, {"result", "exceptionDetails", "exception", "objectId"})
          .size(),
      0);
  EXPECT_GT(
      jsonScope_.getString(resp, {"result", "exceptionDetails", "text"}).size(),
      0);

  // Evaluate something that isn't valid JavaScript syntax
  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"(*ptr));)");
  });
  resp = expectResponse(std::nullopt, msgId++);

  // Ensure that we catch parse exception as well
  EXPECT_NE(
      jsonScope_.getString(resp, {"result", "exceptionDetails", "text"})
          .find("Compiling JS failed"),
      std::string::npos);
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnObject) {
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(debugger;)");
  expectNotification("Debugger.scriptParsed");

  auto pausedNote = ensurePaused(waitForMessage(), "other", {{"global", 0, 1}});

  // create a new Object() that will be used as "this" below.
  m::runtime::RemoteObjectId thisId;
  {
    sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
      params.emitKeyValue("expression", "new Object()");
      params.emitKeyValue("generatePreview", true);
    });

    auto resp = expectResponse(std::nullopt, msgId++);
    thisId = jsonScope_.getString(resp, {"result", "result", "objectId"});
  }

  // expectedPropInfos are properties that are expected to exist in thisId.
  // It is modified by addMember (below).
  std::unordered_map<std::string, PropInfo> expectedPropInfos;

  // Add __proto__ as it always exists.
  expectedPropInfos.emplace("__proto__", PropInfo("object"));

  /// addMember sends Runtime.callFunctionOn() requests with a function
  /// declaration that simply adds a new property called \p propName with
  /// type \p type to the remote object \p id. \p ca is the property's value.
  /// The new property must not exist in \p id unless \p allowRedefinition is
  /// true.
  auto addMember = [&](const m::runtime::RemoteObjectId id,
                       const char *type,
                       const char *propName,
                       m::runtime::CallArgument ca,
                       bool allowRedefinition = false) {
    auto it = expectedPropInfos.emplace(propName, PropInfo(type));

    EXPECT_TRUE(allowRedefinition || it.second)
        << "property \"" << propName << "\" redefined.";

    if (ca.value) {
      it.first->second.setValue(*ca.value);
    }

    if (ca.unserializableValue) {
      it.first->second.setUnserializableValue(*ca.unserializableValue);
    }

    m::runtime::CallFunctionOnRequest req;
    req.id = msgId;
    req.functionDeclaration =
        std::string("function(e){const r=\"") + propName + "\"; this[r]=e,r}";
    req.arguments = std::vector<m::runtime::CallArgument>{};
    req.arguments->push_back(std::move(ca));
    req.objectId = thisId;

    cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));
    expectResponse(std::nullopt, msgId++);
  };

  addMember(thisId, "boolean", "b", makeValueCallArgument("true"));
  addMember(thisId, "number", "num", makeValueCallArgument("12"));
  addMember(thisId, "string", "str", makeValueCallArgument("\"string value\""));
  addMember(thisId, "object", "self_ref", makeObjectIdCallArgument(thisId));
  addMember(
      thisId, "number", "inf", makeUnserializableCallArgument("Infinity"));
  addMember(
      thisId, "number", "ni", makeUnserializableCallArgument("-Infinity"));
  addMember(thisId, "number", "nan", makeUnserializableCallArgument("NaN"));

  /// ensures that \p objId has all of the expected properties; Returns the
  /// runtime::RemoteObjectId for the "self_ref" property (which must exist).
  auto verifyObjShape = [&](const m::runtime::RemoteObjectId &objId)
      -> std::optional<std::string> {
    auto objProps = getAndEnsureProps(msgId++, objId, expectedPropInfos);
    EXPECT_TRUE(objProps.count("__proto__"));
    auto objPropIt = objProps.find("self_ref");
    if (objPropIt == objProps.end()) {
      EXPECT_TRUE(false) << "missing \"self_ref\" property.";
      return {};
    }
    return objPropIt->second;
  };

  // Verify that thisId has the correct shape.
  auto selfRefId = verifyObjShape(thisId);
  ASSERT_TRUE(selfRefId);
  // Then verify that the self reference has the correct shape. If thisId does
  // not have the "self_ref" property the call to verifyObjShape will return an
  // empty Optional, as well as report an error.
  selfRefId = verifyObjShape(*selfRefId);
  ASSERT_TRUE(selfRefId);

  // Now we modify the self reference, which should cause thisId to change
  // as well.
  const bool kAllowRedefinition = true;

  addMember(
      *selfRefId,
      "number",
      "num",
      makeValueCallArgument("42"),
      kAllowRedefinition);

  addMember(
      *selfRefId, "number", "neg_zero", makeUnserializableCallArgument("-0"));

  verifyObjShape(thisId);

  // Let the script terminate
  sendAndCheckResponse("Debugger.resume", msgId++);
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnExecutionContext) {
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(debugger;)");
  expectNotification("Debugger.scriptParsed");
  auto pausedNote = ensurePaused(waitForMessage(), "other", {{"global", 0, 1}});

  /// helper that returns a map with all of \p objId 's members.
  auto getProps = [this, &msgId](const m::runtime::RemoteObjectId &objId) {
    sendRequest(
        "Runtime.getProperties", msgId++, [objId](::hermes::JSONEmitter &json) {
          json.emitKeyValue("objectId", objId);
        });
    auto resp = parseRuntimeGetPropertiesResponse(waitForMessage());

    std::unordered_map<std::string, std::optional<m::runtime::RemoteObject>>
        properties;
    for (auto &propertyDescriptor : resp.result) {
      properties[propertyDescriptor.name] = std::move(propertyDescriptor.value);
    }
    return properties;
  };

  // globalThisId is the inspector's object Id for globalThis.
  m::runtime::RemoteObjectId globalThisId;
  {
    sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
      params.emitKeyValue("expression", "globalThis");
      params.emitKeyValue("generatePreview", true);
    });

    auto resp = expectResponse(std::nullopt, msgId++);
    globalThisId = jsonScope_.getString(resp, {"result", "result", "objectId"});
  }

  // This test table has all of the new fields we want to add to globalThis,
  // plus the Runtime.CallArgument to be sent to the inspector.
  struct {
    const char *propName;
    m::runtime::CallArgument callArg;
  } tests[] = {
      {"callFunctionOnTestMember1", makeValueCallArgument("10")},
      {"callFunctionOnTestMember2", makeValueCallArgument("\"string\"")},
      {"callFunctionOnTestMember3", makeUnserializableCallArgument("NaN")},
      {"callFunctionOnTestMember4", makeUnserializableCallArgument("-0")},
  };

  // sanity-check that our test fields don't exist in global this.
  {
    auto currProps = getProps(globalThisId);
    for (const auto &test : tests) {
      EXPECT_EQ(currProps.count(test.propName), 0) << test.propName;
    }
  }

  auto addMember = [this, &msgId](
                       const char *propName, m::runtime::CallArgument &ca) {
    m::runtime::CallFunctionOnRequest req;
    req.id = msgId;
    req.functionDeclaration =
        std::string("function(e){const r=\"") + propName + "\"; this[r]=e,r}";
    // Don't have an easy way to copy these, so...
    req.arguments = std::vector<m::runtime::CallArgument>{};
    req.arguments->push_back(std::move(ca));
    req.executionContextId = kTestExecutionContextId_;

    cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));
    expectResponse(std::nullopt, msgId++);

    // n.b. we're only borrowing the CallArgument, so give it back...
    ca = std::move(req.arguments->at(0));
  };

  for (auto &test : tests) {
    addMember(test.propName, test.callArg);
  }

  {
    auto currProps = getProps(globalThisId);
    for (const auto &test : tests) {
      auto it = currProps.find(test.propName);

      // there should be a property named test.propName in globalThis.
      ASSERT_TRUE(it != currProps.end()) << test.propName;

      // and it should have a value.
      ASSERT_TRUE(it->second) << test.propName;

      if (it->second->value.has_value()) {
        // the property has a value, so make sure that's what's being expected.
        auto actual = it->second->value;
        auto expected = test.callArg.value;
        ASSERT_TRUE(expected.has_value()) << test.propName;
        ASSERT_TRUE(
            jsonValsEQ(jsonScope_.parse(*actual), jsonScope_.parse(*expected)))
            << test.propName;
      } else if (it->second->unserializableValue.has_value()) {
        // the property has an unserializable value, so make sure that's what's
        // being expected.
        auto actual = it->second->unserializableValue;
        auto expected = test.callArg.unserializableValue;
        ASSERT_TRUE(expected.has_value()) << test.propName;
        EXPECT_EQ(*actual, *expected) << test.propName;
      } else {
        FAIL() << "No value or unserializable value in " << test.propName;
      }
    }
  }

  // Let the script terminate
  sendAndCheckResponse("Debugger.resume", msgId++);
}

TEST_F(CDPAgentTest, RuntimeConsoleLog) {
  int msgId = 1;
  constexpr double kTimestamp = 123.0;
  const std::string kStringValue = "string value";

  // Startup
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Generate message
  waitFor<bool>([this, timestamp = kTimestamp, kStringValue](auto promise) {
    runtimeThread_->add([this, timestamp, kStringValue, promise]() {
      jsi::String arg0 = jsi::String::createFromAscii(*runtime_, kStringValue);

      jsi::Object arg1 = jsi::Object(*runtime_);
      arg1.setProperty(*runtime_, "number1", 1);
      arg1.setProperty(*runtime_, "bool1", false);

      jsi::Object arg2 = jsi::Object(*runtime_);
      arg2.setProperty(*runtime_, "number2", 2);
      arg2.setProperty(*runtime_, "bool2", true);

      ConsoleMessage message(
          timestamp, ConsoleAPIType::kWarning, std::vector<jsi::Value>());
      message.args.reserve(3);
      message.args.push_back(std::move(arg0));
      message.args.push_back(std::move(arg1));
      message.args.push_back(std::move(arg2));
      cdpDebugAPI_->addConsoleMessage(std::move(message));

      promise->set_value(true);
    });
  });

  // Validate notification
  auto note = expectNotification("Runtime.consoleAPICalled");

  EXPECT_EQ(jsonScope_.getNumber(note, {"params", "timestamp"}), kTimestamp);
  EXPECT_EQ(
      jsonScope_.getNumber(note, {"params", "executionContextId"}),
      kTestExecutionContextId_);
  EXPECT_EQ(jsonScope_.getString(note, {"params", "type"}), "warning");

  EXPECT_EQ(jsonScope_.getArray(note, {"params", "args"})->size(), 3);

  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "0", "type"}), "string");
  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "0", "value"}),
      kStringValue);

  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "1", "type"}), "object");
  std::string object1ID =
      jsonScope_.getString(note, {"params", "args", "1", "objectId"});
  getAndEnsureProps(
      msgId++,
      object1ID,
      {{"number1", PropInfo("number").setValue("1")},
       {"bool1", PropInfo("boolean").setValue("false")},
       {"__proto__", PropInfo("object")}});

  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "2", "type"}), "object");
  std::string object2ID =
      jsonScope_.getString(note, {"params", "args", "2", "objectId"});
  getAndEnsureProps(
      msgId++,
      object2ID,
      {{"number2", PropInfo("number").setValue("2")},
       {"bool2", PropInfo("boolean").setValue("true")},
       {"__proto__", PropInfo("object")}});
}

TEST_F(CDPAgentTest, RuntimeConsoleBuffer) {
  int msgId = 1;

  constexpr int kExpectedMaxBufferSize = 1000;
  constexpr int kNumLogsToTest = kExpectedMaxBufferSize * 2;

  // Generate console messages on the runtime thread
  waitFor<bool>([this, numLogs = kNumLogsToTest](auto promise) {
    runtimeThread_->add([this, promise, numLogs]() {
      for (int i = 0; i < numLogs; i++) {
        jsi::Value value =
            jsi::String::createFromUtf8(*runtime_, std::to_string(i));
        std::vector<jsi::Value> args;
        args.push_back(std::move(value));
        cdpDebugAPI_->addConsoleMessage(
            ConsoleMessage{0.0, ConsoleAPIType::kLog, std::move(args)});
      }

      promise->set_value(true);
    });
  });

  bool receivedWarning = false;
  std::array<bool, kExpectedMaxBufferSize> received;

  // Test for repeated connection by sending Runtime.enable multiple times. It's
  // expected that the message cache is always kept around and provided to the
  // frontend each time.
  for (int numConnect = 0; numConnect < 2; numConnect++) {
    receivedWarning = false;
    received.fill(false);

    sendParameterlessRequest("Runtime.enable", msgId);

    // Loop for 1 iteration more than kExpectedMaxBufferSize because there is a
    // warning message given when buffer is exceeded
    for (size_t i = 0; i < kExpectedMaxBufferSize + 1; i++) {
      auto note = expectNotification("Runtime.consoleAPICalled");
      EXPECT_EQ(
          jsonScope_.getString(note, {"params", "args", "0", "type"}),
          "string");

      size_t argCount = jsonScope_.getArray(note, {"params", "args"})->size();
      EXPECT_EQ(argCount, 1);

      std::string type = jsonScope_.getString(note, {"params", "type"});
      std::string value =
          jsonScope_.getString(note, {"params", "args", "0", "value"});
      try {
        // Verify that the latest kExpectedMaxBufferSize number of logs are
        // emitted
        int nthLog = std::stoi(value);
        EXPECT_GT(nthLog, kExpectedMaxBufferSize - 1);
        EXPECT_LT(nthLog, kNumLogsToTest);
        EXPECT_EQ(type, "log");
        received[nthLog % kExpectedMaxBufferSize] = true;
      } catch (const std::exception &e) {
        EXPECT_EQ(type, "warning");
        EXPECT_NE(value.find("discarded"), std::string::npos);
        receivedWarning = true;
      }
    }

    ensureOkResponse(waitForMessage(), msgId++);

    // Make sure no more log messages arrive
    expectNothing();

    // Ensure everything was expected
    for (size_t i = 0; i < kExpectedMaxBufferSize; i++) {
      EXPECT_TRUE(received[i]);
    }
    EXPECT_TRUE(receivedWarning);

    // Disable the runtime so it can be enabled again in the next iteration of
    // the loop
    sendAndCheckResponse("Runtime.disable", msgId++);
  }
}

TEST_F(CDPAgentTest, ProfilerBasicOperation) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });
  int msgId = 1;

  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      runtime_->registerForProfiling();
      promise->set_value(true);
    });
  });

  scheduleScript(R"(
      while(!shouldStop());
  )");

  // Start the sampling profiler. At this point it is not safe to manipulate the
  // VM, so...
  sendAndCheckResponse("Profiler.start", msgId++);

  // Keep the profiler running for a small amount of time to allow for some
  // samples to be collected.
  std::this_thread::sleep_for(500ms);

  // Being re-attached to the VM, send the stop sampling profile request.
  sendParameterlessRequest("Profiler.stop", msgId);
  auto resp = expectResponse(std::nullopt, msgId++);
  auto nodes = jsonScope_.getArray(resp, {"result", "profile", "nodes"});
  EXPECT_GT(nodes->size(), 0);
  EXPECT_LE(
      jsonScope_.getNumber(resp, {"result", "profile", "startTime"}),
      jsonScope_.getNumber(resp, {"result", "profile", "endTime"}));
}

TEST_F(CDPAgentTest, RuntimeValidatesExecutionContextId) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });

  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  scheduleScript(R"(while(!shouldStop());)");

  constexpr auto kExecutionContextSubstring = "execution context id";

  sendRequest(
      "Runtime.globalLexicalScopeNames",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("executionContextId", kTestExecutionContextId_ + 1);
      });
  expectErrorMessageContaining(kExecutionContextSubstring, msgId++);

  sendRequest("Runtime.compileScript", msgId, [](::hermes::JSONEmitter &json) {
    json.emitKeyValue("persistScript", true);
    json.emitKeyValue("sourceURL", "none");
    json.emitKeyValue("expression", "1+1");
    json.emitKeyValue("executionContextId", kTestExecutionContextId_ + 1);
  });
  expectErrorMessageContaining(kExecutionContextSubstring, msgId++);

  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"("0: " + globalVar)");
    params.emitKeyValue("contextId", kTestExecutionContextId_ + 1);
  });
  expectErrorMessageContaining(kExecutionContextSubstring, msgId++);

  m::runtime::CallFunctionOnRequest req;
  req.id = msgId;
  req.functionDeclaration = std::string("function(){}");
  req.executionContextId = kTestExecutionContextId_ + 1;
  cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));
  expectErrorMessageContaining(kExecutionContextSubstring, msgId++);
}

#endif // HERMES_ENABLE_DEBUGGER
