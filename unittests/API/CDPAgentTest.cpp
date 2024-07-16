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
#include <hermes/Support/SerialExecutor.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/CDPDebugAPI.h>
#include <hermes/cdp/JSONValueInterfaces.h>
#include <hermes/hermes.h>
#include <jsi/instrumentation.h>

#include <llvh/ADT/ScopeExit.h>

#include "CDPJSONHelpers.h"
#include "CDPTestHelpers.h"

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

// A function passed to Runtime.callFunctionOn in order to invoke a getter. See
// https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/58eff7d6a4ed165a3350c8817c1ec5724eab5cb7/front_end/ui/legacy/components/object_ui/ObjectPropertiesSection.ts#L1125-L1132
constexpr auto kInvokeGetterFunction = R"(
  function invokeGetter(getter) {
    return Function.prototype.apply.call(getter, this, []);
  }
)";

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

  /// check to see if a response or notification is immediately available.
  /// returns the message, or nullopt if no message is available.
  std::optional<std::string> tryGetMessage();

  void expectNothing();
  JSONObject *expectNotification(const std::string &method);
  JSONObject *expectResponse(const std::optional<std::string> &method, int id);
  /// Wait for a message, validate that it is an error with the specified
  /// \p messageID, and assert that the error description contains the
  /// specified \p substring.
  void expectErrorMessageContaining(
      const std::string &substring,
      long long messageID);
  /// Expect a sequence of messages conveying a heap snapshot:
  /// 1 or more notifications containing chunks of the snapshot JSON object
  /// followed by an OK response to the snapshot request.
  /// \p messageID specifies the id of the snapshot request.
  /// \p ignoreTrackingNotifications indicates whether lastSeenObjectId and
  /// heapStatsUpdate notifications are tolerated before the snapshot arrives.
  void expectHeapSnapshot(
      int messageID,
      bool ignoreTrackingNotifications = false);

  void sendRequest(
      const std::string &method,
      int id,
      const std::function<void(::hermes::JSONEmitter &)> &setParameters = {},
      CDPAgent *altAgent = nullptr);
  void sendParameterlessRequest(const std::string &method, int id);
  void sendAndCheckResponse(const std::string &method, int id);
  void sendEvalRequest(
      int id,
      int callFrameId,
      const std::string &expression,
      CDPAgent *altAgent = nullptr);

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

  /// Store a value provided by a test script so it can be later used by a
  /// test.
  jsi::Value storeValue(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  /// Move the previously stored value out of the storage location and
  /// return it.
  jsi::Value takeStoredValue();

  m::runtime::GetPropertiesResponse getAndEnsureProps(
      int msgId,
      const std::string &objectId,
      const std::unordered_map<std::string, PropInfo> &infos,
      const std::unordered_map<std::string, PropInfo> &internalInfos = {},
      bool ownProperties = true,
      bool accessorPropertiesOnly = false);

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<CDPDebugAPI> cdpDebugAPI_;
  std::unique_ptr<::hermes::SerialExecutor> runtimeThread_;
  std::unique_ptr<CDPAgent> cdpAgent_;

  std::atomic<bool> stopFlag_{};

  std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl_ =
      std::make_shared<std::atomic_bool>(false);

  std::mutex testSignalMutex_;
  std::condition_variable testSignalCondition_;
  bool testSignalled_ = false;

  /// Mutex to protect storedValue_, as it is typically written on the runtime
  /// thread (via the "storeValue" host function), and read from the test
  /// thread (via "takeStoredValue").
  std::mutex storedValueMutex_;
  jsi::Value storedValue_;

  std::mutex messageMutex_;
  std::condition_variable hasMessage_;
  std::queue<std::string> messages_;

  std::vector<std::string> thrownExceptions_;
  JSONScope jsonScope_;
};

void CDPAgentTest::SetUp() {
  setupRuntimeTestInfra();

  cdpAgent_ = std::unique_ptr<CDPAgent>(new CDPAgent(
      kTestExecutionContextId_,
      *cdpDebugAPI_,
      std::bind(&CDPAgentTest::handleRuntimeTask, this, _1),
      std::bind(&CDPAgentTest::handleResponse, this, _1),
      {},
      destroyedDomainAgentsImpl_));
}

void CDPAgentTest::setupRuntimeTestInfra() {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  // Give the runtime thread the same stack size as the main thread. The runtime
  // thread is the main thread of the HermesRuntime.
  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  runtimeThread_ = std::make_unique<::hermes::SerialExecutor>(limit.rlim_cur);
#else
  runtimeThread_ = std::make_unique<::hermes::SerialExecutor>();
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
  runtime_->global().setProperty(
      *runtime_,
      "storeValue",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "storeValue"),
          0,
          std::bind(&CDPAgentTest::storeValue, this, _1, _2, _3, _4)));

  cdpDebugAPI_ = CDPDebugAPI::create(*runtime_);
}

void CDPAgentTest::TearDown() {
  // Drop reference to value inside the runtime
  storedValue_ = jsi::Value::undefined();

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

std::optional<std::string> CDPAgentTest::tryGetMessage() {
  // Take a message, if present, while holding the mutex protecting the message
  // collection. This doesn't clear the "hasMessage_" notification, so it could
  // leave "hasMessage_" signaled when there is no message waiting. This is
  // okay, because waiting on "hasMessage_" elsewhere tolerates spurious
  // wake-ups by also checking "messages_.empty()".
  std::unique_lock<std::mutex> lock(messageMutex_);
  if (messages_.empty()) {
    return std::nullopt;
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

void CDPAgentTest::expectHeapSnapshot(
    int messageID,
    bool ignoreTrackingNotifications) {
  // Expect chunk notifications until the snapshot object is complete. Fail if
  // the object is invalid (e.g. truncated data, excess data, malformed JSON).
  // There is no indication of how many segments there will be, so just receive
  // until the object is complete, then expect no more.
  std::stringstream snapshot;
  do {
    JSONObject *note = jsonScope_.parseObject(waitForMessage());
    std::string method = jsonScope_.getString(note, {"method"});
    if (ignoreTrackingNotifications &&
        (method == "HeapProfiler.lastSeenObjectId" ||
         method == "HeapProfiler.heapStatsUpdate")) {
      continue;
    }

    ASSERT_EQ(method, "HeapProfiler.addHeapSnapshotChunk");
    snapshot << jsonScope_.getString(note, {"params", "chunk"});
  } while (!jsonScope_.tryParseObject(snapshot.str()).has_value());

  // Expect the snapshot response after all chunks have been received.
  ensureOkResponse(waitForMessage(), messageID);
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
    const std::function<void(::hermes::JSONEmitter &)> &setParameters,
    CDPAgent *altAgent) {
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
  if (altAgent != nullptr) {
    altAgent->handleCommand(command);
  } else {
    cdpAgent_->handleCommand(command);
  }
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

jsi::Value CDPAgentTest::storeValue(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  if (count > 0) {
    std::unique_lock<std::mutex> lock(storedValueMutex_);
    storedValue_ = jsi::Value(runtime, args[0]);
  }
  return jsi::Value::undefined();
}

jsi::Value CDPAgentTest::takeStoredValue() {
  std::unique_lock<std::mutex> lock(storedValueMutex_);
  return std::move(storedValue_);
}

void CDPAgentTest::sendEvalRequest(
    int id,
    int callFrameId,
    const std::string &expression,
    CDPAgent *altAgent) {
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
  if (altAgent != nullptr) {
    altAgent->handleCommand(command);
  } else {
    cdpAgent_->handleCommand(command);
  }
}

m::runtime::GetPropertiesResponse CDPAgentTest::getAndEnsureProps(
    int msgId,
    const std::string &objectId,
    const std::unordered_map<std::string, PropInfo> &infos,
    const std::unordered_map<std::string, PropInfo> &internalInfos,
    bool ownProperties,
    bool accessorPropertiesOnly) {
  sendRequest("Runtime.getProperties", msgId, [&](::hermes::JSONEmitter &json) {
    json.emitKeyValue("objectId", objectId);
    json.emitKeyValue("ownProperties", ownProperties);
    json.emitKeyValue("accessorPropertiesOnly", accessorPropertiesOnly);
  });
  return ensureProps(waitForMessage(), infos, internalInfos);
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

TEST_F(CDPAgentTest, CDPAgentTaskRunsAfterCleanUp) {
  int msgId = 1;
  auto setStopFlag = llvh::make_scope_exit([this] { stopFlag_.store(true); });

  sendAndCheckResponse("Runtime.enable", msgId++);

  // Start a long-running expression, which simulates uninterruptable JavaScript
  // work
  scheduleScript(R"(
    signalTest();

    while (!shouldStop()) {}
  )");
  // Wait for the script to start
  waitForTestSignal();

  // Schedule Runtime.evaluate. This will be queued, but not executed yet due to
  // the previous long-running code.
  sendRequest("Runtime.evaluate", msgId + 0, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"(while(!shouldStop()); 1+1)");
  });

  // Clean up CDPAgent. We allow this to be done without synchronization with
  // the runtime thread.
  cdpAgent_.reset();

  // We expect destroying CDPAgent will queue cleanup logic with the interrupt
  // queue. Schedule a task on the interrupt queue so we know when that cleanup
  // logic finishes running.
  AsyncDebuggerAPI &asyncDebuggerAPI = cdpDebugAPI_->asyncDebuggerAPI();
  waitFor<bool>([&asyncDebuggerAPI](auto promise) {
    asyncDebuggerAPI.triggerInterrupt_TS(
        [promise](HermesRuntime &) { promise->set_value(true); });
  });

  // The interrupt queue will be able to run before the queued Runtime.evaluate
  // task, so we expect DomainAgents to be cleaned up by this time.
  EXPECT_TRUE(*destroyedDomainAgentsImpl_);

  // Allow the JavaScript code from scheduleScript to finish, so the queued
  // Runtime.evaluate task can start.
  stopFlag_.store(true);

  // Expect the queued Runtime.evaluate to do nothing.
}

TEST_F(CDPAgentTest, CDPAgentCleanUpWhileTaskRunning) {
  int msgId = 1;
  auto setStopFlag = llvh::make_scope_exit([this] { stopFlag_.store(true); });

  sendAndCheckResponse("Runtime.enable", msgId++);

  sendRequest("Runtime.evaluate", msgId + 0, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"(
      signalTest();

      while (!shouldStop()) {}
      )");
  });
  // Wait for Runtime.evaluate to start
  waitForTestSignal();

  // At this point Runtime.evaluate is currently running via the integrator
  // queue. We now destroy CDPAgent, which will do some cleanup logic via the
  // interrupt queue, thus interrupting Runtime.evaluate while it's running.
  cdpAgent_.reset();

  // We expect destroying CDPAgent will queue cleanup logic with the interrupt
  // queue. Schedule a task on the interrupt queue so we know when that cleanup
  // logic finishes running.
  AsyncDebuggerAPI &asyncDebuggerAPI = cdpDebugAPI_->asyncDebuggerAPI();
  waitFor<bool>([&asyncDebuggerAPI](auto promise) {
    asyncDebuggerAPI.triggerInterrupt_TS(
        [promise](HermesRuntime &) { promise->set_value(true); });
  });

  // Since Runtime.evaluate is still running, it will still hold onto a strong
  // reference of DomainAgentsImpl. That's why DomainAgentsImpl won't actually
  // have been destroyed here.
  EXPECT_FALSE(*destroyedDomainAgentsImpl_);

  // Allow the JavaScript code for Runtime.evaluate to run to finish.
  stopFlag_.store(true);

  runtimeThread_->add([this]() {
    // Expect the DomainAgentsImpl to finally be cleaned up after
    // Runtime.evaluate finishes.
    EXPECT_TRUE(*destroyedDomainAgentsImpl_);
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

TEST_F(CDPAgentTest, DebuggerCallFrameThisType) {
  int msgId = 1;
  sendAndCheckResponse("Debugger.enable", msgId++);

  // Trigger a debugger pause where one of the call frames has an undefined
  // 'this'
  scheduleScript(R"(
    function test() {
      debugger;           // line 2
    }
    test.call(undefined); // line 4 - Call test() with an undefined 'this'
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // Verify that the 'this' RemoteObject is populated correctedly.
  ensurePaused(
      waitForMessage(),
      "other",
      {FrameInfo("0", "test", 2, 2).setThisType("undefined"),
       FrameInfo("2", "global", 4, 1).setThisType("object")});
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

TEST_F(CDPAgentTest, DebuggerFiltersNativeFrames) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  // `debugger;` statement won't work unless Debugger domain is enabled, so
  // call `scheduleScript()` after sending `Debugger.enable`.
  scheduleScript(R"(
    function level4() {
      debugger;             // line 2
    }
    function level3() {
      // This inserts a native frame due to calling via hermesBuiltinApply
      level4(...arguments); // line 6
    }
    function level2() {
      // This inserts a native frame due to calling via functionPrototypeApply
      level3.apply();       // line 10
    }
    function level1() {
      // This inserts a native frame due to calling via functionPrototypeCall
      level2.call();        // line 14
    }
    level1();               // line 16
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // Check that no native frames are emitted. The Debugger.CallFrameId won't be
  // sequential.
  ensurePaused(
      waitForMessage(),
      "other",
      {{"0", "level4", 2, 2},
       {"2", "level3", 6, 2},
       {"4", "level2", 10, 2},
       {"6", "level1", 14, 2},
       {"7", "global", 16, 1}});
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

TEST_F(CDPAgentTest, DebuggerReportsException) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    debugger; // [1] (line 1) initial pause, set throw on exceptions to 'All'
    throw new Error('Catch me if you can'); // [2] (line 2) pause on exception
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // [1] (line 1) initial pause. set throw on exceptions to 'All' then resume.
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});
  sendRequest(
      "Debugger.setPauseOnExceptions", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("state", "all");
      });
  ensureOkResponse(waitForMessage(), msgId++);
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // [2] line 2, pause on exception, ensure exception details arrived.
  m::debugger::PausedNotification note =
      ensurePaused(waitForMessage(), "exception", {{"global", 2, 1}});
  EXPECT_TRUE(note.data.has_value());
  JSONObject *data = jsonScope_.parseObject(*note.data);
  EXPECT_EQ(
      jsonScope_.getString(data, {"description"}),
      "Error: Catch me if you can");

  // Let the script finish
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  waitForScheduledScripts();
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
       {"str", PropInfo("string").setValue("\"string\"")}},
      {{"[[Prototype]]", PropInfo("object")}});

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

TEST_F(CDPAgentTest, DebuggerMultipleCDPAgents) {
  // Make a second CDPAgent
  auto secondCDPAgent = CDPAgent::create(
      kTestExecutionContextId_,
      *cdpDebugAPI_,
      std::bind(&CDPAgentTest::handleRuntimeTask, this, _1),
      std::bind(&CDPAgentTest::handleResponse, this, _1));

  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  std::array<std::string, 4> secondAgentMethods = {
      "Debugger.resume",
      "Debugger.stepOver",
      "Debugger.stepInto",
      "Debugger.stepOut",
  };

  for (const auto &method : secondAgentMethods) {
    scheduleScript(R"(
      debugger;      // line 1
    )");
    ensureNotification(waitForMessage(), "Debugger.scriptParsed");
    ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

    // Send a command from the second CDPAgent even though we never enabled the
    // Debugger domain on the second one.
    sendRequest(method, msgId, {}, secondCDPAgent.get());

    // Check that the command gets processed successfully
    ensureOkResponse(waitForMessage(), msgId++);

    // And the debugger resumed
    ensureNotification(waitForMessage(), "Debugger.resumed");
  }

  scheduleScript(R"(
    function level1() {
      var foo = "bar";
      debugger;        // line 3
    }
    level1();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensurePaused(waitForMessage(), "other", {{"level1", 3, 2}, {"global", 5, 1}});

  // Send a command from the second CDPAgent for evaluateOnCallFrame even though
  // we never enabled the Debugger domain on the second one.
  sendEvalRequest(msgId, 0, R"("foo" + foo)", secondCDPAgent.get());

  ensureEvalResponse(waitForMessage(), msgId++, "foobar");
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

  // Some GC configurations report zero memory usage (e.g. mallocgc).
  // Expect a successful response with non-negative memory usage,
  // accepting zero memory usage rather than baking the specifics
  // of each GC into this test of CDP.
  EXPECT_GE(jsonScope_.getNumber(resp, {"result", "usedSize"}), 0);
  EXPECT_GE(jsonScope_.getNumber(resp, {"result", "totalSize"}), 0);
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

  auto scopeChildrenResp = getAndEnsureProps(
      msgId++,
      scopeObjId,
      {{"num", PropInfo("number").setValue("123")},
       {"obj", PropInfo("object")},
       {"arr", PropInfo("object").setSubtype("array")},
       {"bar", PropInfo("function")}});
  auto scopeChildren = indexProps(scopeChildrenResp.result);
  EXPECT_EQ(scopeChildren.size(), 4);

  ASSERT_EQ(scopeChildren.count("obj"), 1);
  const auto &obj = scopeChildren.at("obj");
  std::string objId = obj.value.value().objectId.value();
  objIds.push_back(objId);

  auto objChildrenResp = getAndEnsureProps(
      msgId++,
      objId,
      {{"depth", PropInfo("number").setValue("0")},
       {"value", PropInfo("object")}},
      {{"[[Prototype]]", PropInfo("object")}});
  auto objChildren = indexProps(objChildrenResp.result);
  EXPECT_EQ(objChildren.size(), 2);

  ASSERT_EQ(objChildren.count("value"), 1);
  const auto &value = objChildren.at("value");
  std::string valueId = value.value.value().objectId.value();
  objIds.push_back(valueId);

  auto valueChildren = getAndEnsureProps(
      msgId++,
      valueId,
      {{"a", PropInfo("number").setUnserializableValue("-Infinity")},
       {"b", PropInfo("number").setUnserializableValue("Infinity")},
       {"c", PropInfo("number").setUnserializableValue("NaN")},
       {"d", PropInfo("number").setUnserializableValue("-0")},
       {"e", PropInfo("string").setValue("\"e_string\"")}},
      {{"[[Prototype]]", PropInfo("object")}});
  EXPECT_EQ(valueChildren.result.size(), 5);

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
      var protoObject = Object.create(null);
      protoObject.protoNum = 77;

      var obj = Object.create(protoObject);
      obj.num = 42;
      protoObject.num = 1234 /* shadowed */;
      debugger;
    }
    foo();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  // wait for a pause on debugger statement and get object ID from the local
  // scope.
  auto pausedNote = ensurePaused(
      waitForMessage(), "other", {{"foo", 8, 2}, {"global", 10, 1}});
  const auto &scopeObject = pausedNote.callFrames.at(0).scopeChain.at(0).object;
  auto scopeChildrenResp = getAndEnsureProps(
      msgId++,
      scopeObject.objectId.value(),
      {{"obj", PropInfo("object")}, {"protoObject", PropInfo("object")}});
  auto scopeChildren = indexProps(scopeChildrenResp.result);
  EXPECT_EQ(scopeChildren.count("obj"), 1);
  const auto &obj = scopeChildren.at("obj");
  std::string objId = obj.value.value().objectId.value();

  // Check that GetProperties request for obj object only have own properties
  // when onlyOwnProperties = true.
  getAndEnsureProps(
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")}},
      {{"[[Prototype]]", PropInfo("object")}},
      true);

  // Check that GetProperties request for obj object only have all properties
  // when onlyOwnProperties = false.
  getAndEnsureProps(
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")},
       {"protoNum", PropInfo("number").setValue("77")}},
      {{"[[Prototype]]", PropInfo("object")}},
      false);

  ASSERT_EQ(scopeChildren.count("protoObject"), 1);
  std::string protoObjectId =
      scopeChildren.at("protoObject").value.value().objectId.value();
  getAndEnsureProps(
      msgId++,
      protoObjectId,
      {{"num", PropInfo("number").setValue("1234")},
       {"protoNum", PropInfo("number").setValue("77")}},
      // No [[Prototype]] when the prototype is null
      {});

  // resume
  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentTest, RuntimeGetPropertiesExtendedDescriptors) {
  int msgId = 1;

  // Start a script
  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);
  scheduleScript(R"(
    function foo() {
      var obj = Object.create(null);
      Object.defineProperties(
        obj,
        {
          data: {
            value: 42,
            configurable: false,
            enumerable: false,
            writable: false,
          },
          accessor: {
            get() { return 1234; },
            set(v) {},
            configurable: true,
            enumerable: false,
          },
          throwingAccessor: {
            get() { throw new Error("Throwing accessor"); },
          }
        },
      );
      debugger;
    }
    foo();
  )");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  auto pausedNote = ensurePaused(
      waitForMessage(), "other", {{"foo", 23, 2}, {"global", 25, 1}});

  const auto &scopeObject = pausedNote.callFrames.at(0).scopeChain.at(0).object;
  EXPECT_TRUE(scopeObject.objectId.has_value());
  std::string scopeObjectId = scopeObject.objectId.value();

  auto scopeChildrenResp =
      getAndEnsureProps(msgId++, scopeObjectId, {{"obj", PropInfo("object")}});
  auto scopeChildren = indexProps(scopeChildrenResp.result);
  ASSERT_EQ(scopeChildren.count("obj"), 1);
  const auto &obj = scopeChildren.at("obj");
  std::string objId = obj.value.value().objectId.value();

  getAndEnsureProps(
      msgId++,
      objId,
      {{"accessor", PropInfo().setConfigurable(true).setEnumerable(false)},
       {"throwingAccessor",
        PropInfo().setConfigurable(false).setEnumerable(false)}},
      {},
      /* ownProperties */ true,
      /* accessorPropertiesOnly */ true);

  auto objPropsResp = getAndEnsureProps(
      msgId++,
      objId,
      {{"data",
        PropInfo("number")
            .setValue("42")
            .setConfigurable(false)
            .setEnumerable(false)
            .setWritable(false)},
       {"accessor", PropInfo().setConfigurable(true).setEnumerable(false)},
       {"throwingAccessor",
        PropInfo().setConfigurable(false).setEnumerable(false)}},
      {},
      /* ownProperties */ true,
      /* accessorPropertiesOnly */ false);

  /// Helper that invokes a getter function on the specified object.
  auto invokeGetter = [&](std::string objId,
                          const m::runtime::PropertyDescriptor &prop)
      -> m::runtime::CallFunctionOnResponse {
    m::runtime::CallFunctionOnRequest req;
    req.id = msgId++;
    req.functionDeclaration = kInvokeGetterFunction;
    req.objectId = objId;
    req.arguments = std::vector<m::runtime::CallArgument>{};
    auto ca = m::runtime::CallArgument();
    ca.objectId = prop.get.value().objectId.value();
    req.arguments->push_back(std::move(ca));
    cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));
    auto message = expectResponse(std::nullopt, req.id);
    return mustMake<m::runtime::CallFunctionOnResponse>(message);
  };

  auto objProps = indexProps(objPropsResp.result);
  EXPECT_EQ(
      invokeGetter(objId, objProps.at("accessor")).result.value.value(),
      "1234");
  EXPECT_TRUE(invokeGetter(objId, objProps.at("throwingAccessor"))
                  .exceptionDetails.has_value());

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
  )");
  // Wait for the script to execute
  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([promise]() { promise->set_value(true); });
  });

  // run eval statements
  sendRequest("Runtime.evaluate", msgId + 0, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"("0: " + globalVar)");
  });
  auto resp0 = expectResponse(std::nullopt, msgId + 0);
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "type"}), "string");
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "value"}), "0: omega");

  // run eval statements that return non-string primitive values
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

  // run eval statement that returns object
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
       {"str", PropInfo("string").setValue("\"string\"")}},
      {{"[[Prototype]]", PropInfo("object")}},
      true);
}

TEST_F(CDPAgentTest, RuntimeEvaluateWhilePaused) {
  int msgId = 1;

  // Start a script that halts on a debugger statement
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

  // Runtime evaluate should not happen while paused.
  int evaluateMsgId = msgId++;
  sendRequest(
      "Runtime.evaluate", evaluateMsgId, [](::hermes::JSONEmitter &params) {
        params.emitKeyValue("expression", "inGlobalScope");
      });
  expectNothing();

  // Let the script terminate
  sendAndCheckResponse("Debugger.resume", msgId);
  ensureNotification(waitForMessage("Debugger.resumed"), "Debugger.resumed");

  // The eval should then complete, starting with a scriptParsed notification.
  // Other tests of Runtime.evaluate don't receive the scriptParsed notification
  // because they don't enable the debugger domain. This test is explicitly
  // checking the behavior of Runtime.evaluate while paused, so the debugger
  // domain is enabled.
  expectNotification("Debugger.scriptParsed");
  auto resp = expectResponse(std::nullopt, evaluateMsgId);
  EXPECT_EQ(jsonScope_.getString(resp, {"result", "result", "type"}), "number");
  EXPECT_EQ(jsonScope_.getNumber(resp, {"result", "result", "value"}), 123);
}

TEST_F(CDPAgentTest, RuntimeEvaluateReturnByValue) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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

  // Ensure that the result object is populated with the exception as well
  EXPECT_GT(
      jsonScope_.getString(resp, {"result", "result", "objectId"}).size(), 0);

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

TEST_F(CDPAgentTest, RuntimeEvaluateNested) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  // Start a long-running expression
  sendRequest("Runtime.evaluate", msgId + 0, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"(while(!shouldStop()); 1+1)");
  });
  // Expect it to keep running
  expectNothing();

  // Try to start another expression
  sendRequest("Runtime.evaluate", msgId + 1, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", "2+2");
  });
  // Expect it to not be evaluated
  expectNothing();

  // Let the evaluations terminate
  stopFlag_.store(true);

  // Expect the first evaluation to complete first
  auto resp0 = expectResponse(std::nullopt, msgId + 0);
  EXPECT_EQ(
      jsonScope_.getString(resp0, {"result", "result", "type"}), "number");
  EXPECT_EQ(jsonScope_.getNumber(resp0, {"result", "result", "value"}), 2);

  // Expect the second evaluation to complete second
  auto resp1 = expectResponse(std::nullopt, msgId + 1);
  EXPECT_EQ(
      jsonScope_.getString(resp1, {"result", "result", "type"}), "number");
  EXPECT_EQ(jsonScope_.getNumber(resp1, {"result", "result", "value"}), 4);
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnObject) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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
    auto objPropsResponse = getAndEnsureProps(
        msgId++,
        objId,
        expectedPropInfos,
        {{"[[Prototype]]", PropInfo("object")}});
    auto objProps = indexProps(objPropsResponse.result);
    auto objPropIt = objProps.find("self_ref");
    if (objPropIt == objProps.end()) {
      EXPECT_TRUE(false) << "missing \"self_ref\" property.";
      return {};
    }
    return objPropIt->second.value.value().objectId.value();
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
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnScope) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);
  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(R"(
    function test() {
      debugger;  // line 2
    }
    test();      // line 4
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  m::debugger::PausedNotification note = ensurePaused(
      waitForMessage(), "other", {{"test", 2, 2}, {"global", 4, 1}});
  EXPECT_EQ(note.callFrames[0].scopeChain.size(), 2);
  EXPECT_EQ(note.callFrames[0].scopeChain[0].object.objectId.value(), "-1");

  m::runtime::CallFunctionOnRequest req;
  req.id = msgId;
  req.functionDeclaration = std::string("function(){}");
  req.objectId = "-1";

  cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));
  expectResponse(std::nullopt, msgId++);
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnExecutionContext) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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
}

TEST_F(CDPAgentTest, RuntimeCallFunctionOnExecutionContextThrowingError) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  m::runtime::CallFunctionOnRequest req;
  req.id = msgId;
  // This function will throw an error when called, so we expect the inspector
  // to return a Runtime.ExceptionThrown notification with this message.
  req.functionDeclaration = "() => {throw new Error('test')}";
  req.executionContextId = kTestExecutionContextId_;
  cdpAgent_->handleCommand(serializeRuntimeCallFunctionOnRequest(req));

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

  // Ensure that the result object is populated with the exception as well
  EXPECT_GT(
      jsonScope_.getString(resp, {"result", "result", "objectId"}).size(), 0);
}

TEST_F(CDPAgentTest, RuntimeConsoleLog) {
  int msgId = 1;
  constexpr double kTimestamp = 123.0;
  const std::string kStringValue = "string value";

  waitFor<bool>([this, timestamp = kTimestamp, kStringValue](auto promise) {
    runtimeThread_->add([this, timestamp, kStringValue, promise]() {
      runtime_->global().setProperty(
          *runtime_,
          "consoleLog",
          jsi::Function::createFromHostFunction(
              *runtime_,
              jsi::PropNameID::forAscii(*runtime_, "consoleLog"),
              0,
              [this, timestamp, kStringValue](
                  jsi::Runtime &,
                  const jsi::Value &,
                  const jsi::Value *,
                  size_t) {
                jsi::String arg0 =
                    jsi::String::createFromAscii(*runtime_, kStringValue);

                jsi::Object arg1 = jsi::Object(*runtime_);
                arg1.setProperty(*runtime_, "number1", 1);
                arg1.setProperty(*runtime_, "bool1", false);

                jsi::Object arg2 = jsi::Object(*runtime_);
                arg2.setProperty(*runtime_, "number2", 2);
                arg2.setProperty(*runtime_, "bool2", true);

                ConsoleMessage message(
                    timestamp,
                    ConsoleAPIType::kWarning,
                    std::vector<jsi::Value>());
                message.args.reserve(3);
                message.args.push_back(std::move(arg0));
                message.args.push_back(std::move(arg1));
                message.args.push_back(std::move(arg2));
                message.stackTrace =
                    runtime_->getDebugger().captureStackTrace();
                cdpDebugAPI_->addConsoleMessage(std::move(message));

                return jsi::Value::undefined();
              }));
      promise->set_value(true);
    });
  });

  // Startup
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Generate message
  sendRequest("Runtime.evaluate", msgId, [](::hermes::JSONEmitter &params) {
    params.emitKeyValue("expression", R"(
      function level2() {
        consoleLog();
      }
      function level1() {
        // This creates a native frame due to executing via hermesBuiltinApply
        level2(...arguments);
      }
      level1();
    )");
  });

  // Validate notification
  auto note = expectNotification("Runtime.consoleAPICalled");

  // Runtime.evaluate's response comes after the consoleAPICalled notification
  expectResponse(std::nullopt, msgId++);

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

  auto callFrames =
      jsonScope_.getArray(note, {"params", "stackTrace", "callFrames"});
  EXPECT_EQ(callFrames->size(), 3);
  EXPECT_EQ(
      jsonScope_.getString(
          note, {"params", "stackTrace", "callFrames", "0", "functionName"}),
      "level2");
  EXPECT_EQ(
      jsonScope_.getString(
          note, {"params", "stackTrace", "callFrames", "1", "functionName"}),
      "level1");
  EXPECT_EQ(
      jsonScope_.getString(
          note, {"params", "stackTrace", "callFrames", "2", "functionName"}),
      "global");

  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "1", "type"}), "object");
  std::string object1ID =
      jsonScope_.getString(note, {"params", "args", "1", "objectId"});
  getAndEnsureProps(
      msgId++,
      object1ID,
      {{"number1", PropInfo("number").setValue("1")},
       {"bool1", PropInfo("boolean").setValue("false")}},
      {{"[[Prototype]]", PropInfo("object")}});

  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "2", "type"}), "object");
  std::string object2ID =
      jsonScope_.getString(note, {"params", "args", "2", "objectId"});
  getAndEnsureProps(
      msgId++,
      object2ID,
      {{"number2", PropInfo("number").setValue("2")},
       {"bool2", PropInfo("boolean").setValue("true")}},
      {{"[[Prototype]]", PropInfo("object")}});
}

TEST_F(CDPAgentTest, RuntimeConsoleLogJSON) {
  int msgId = 1;
  const std::string kStringValue = "{\"number\": 1}";

  // Startup
  sendAndCheckResponse("Runtime.enable", msgId++);

  // Generate ConsoleAPICalled notification containing a JSON string argument
  waitFor<bool>([this, kStringValue](auto promise) {
    runtimeThread_->add([this, promise, kStringValue]() {
      constexpr double kTimestamp = 123.0;
      jsi::String arg = jsi::String::createFromAscii(*runtime_, kStringValue);
      ConsoleMessage message(
          kTimestamp, ConsoleAPIType::kWarning, std::vector<jsi::Value>());
      message.args.push_back(std::move(arg));
      cdpDebugAPI_->addConsoleMessage(std::move(message));
      promise->set_value(true);
    });
  });
  auto note = expectNotification("Runtime.consoleAPICalled");

  // Ensure the JSON arrived intact
  EXPECT_EQ(jsonScope_.getArray(note, {"params", "args"})->size(), 1);
  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "0", "type"}), "string");
  EXPECT_EQ(
      jsonScope_.getString(note, {"params", "args", "0", "value"}),
      kStringValue);
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

TEST_F(CDPAgentTest, RuntimeDiscardConsoleEntries) {
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      cdpDebugAPI_->addConsoleMessage(
          ConsoleMessage{0.0, ConsoleAPIType::kLog, std::vector<jsi::Value>()});
      promise->set_value(true);
    });
  });

  expectNotification("Runtime.consoleAPICalled");

  sendAndCheckResponse("Runtime.discardConsoleEntries", msgId++);
  sendAndCheckResponse("Runtime.disable", msgId++);
  sendAndCheckResponse("Runtime.enable", msgId++);

  expectNothing();
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
  int msgId = 1;

  sendAndCheckResponse("Runtime.enable", msgId++);

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

TEST_F(CDPAgentTest, HeapProfilerSnapshot) {
  auto setStopFlag = llvh::make_scope_exit([this] {
    // break out of loop
    stopFlag_.store(true);
  });

  int msgId = 1;

  scheduleScript(R"(
      while(!shouldStop());
  )");

  // Request a heap snapshot and expect it to arrive.
  sendRequest(
      "HeapProfiler.takeHeapSnapshot", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("reportProgress", false);
      });
  expectHeapSnapshot(msgId);

  // Expect no more chunks are pending.
  expectNothing();
}

TEST_F(CDPAgentTest, HeapProfilerSnapshotRemoteObject) {
  int msgId = 1;
  scheduleScript(R"(
    storeValue([1, 2, 3]);
    signalTest();
  )");
  waitForTestSignal();

  {
    // Take a heap snapshot first to assign IDs.
    sendRequest(
        "HeapProfiler.takeHeapSnapshot",
        msgId,
        [](::hermes::JSONEmitter &json) {
          json.emitKeyValue("reportProgress", false);
        });
    // We don't need to keep the response because we can directly query for
    // object IDs from the runtime.
    expectHeapSnapshot(msgId++);
  }

  const uint64_t globalObjID = runtime_->getUniqueID(runtime_->global());
  jsi::Value storedValue = takeStoredValue();
  const uint64_t storedObjID =
      runtime_->getUniqueID(storedValue.asObject(*runtime_));

  auto testObject = [this, &msgId](
                        uint64_t objID,
                        const char *type,
                        const char *className,
                        const char *description,
                        const char *subtype) {
    // Get the object by its snapshot ID.
    sendRequest(
        "HeapProfiler.getObjectByHeapObjectId",
        msgId,
        [objID](::hermes::JSONEmitter &json) {
          json.emitKeyValue("objectId", std::to_string(objID));
        });
    auto resp0 = expectResponse(std::nullopt, msgId++);
    EXPECT_EQ(jsonScope_.getString(resp0, {"result", "result", "type"}), type);
    EXPECT_EQ(
        jsonScope_.getString(resp0, {"result", "result", "className"}),
        className);
    EXPECT_EQ(
        jsonScope_.getString(resp0, {"result", "result", "description"}),
        description);
    if (subtype) {
      EXPECT_EQ(
          jsonScope_.getString(resp0, {"result", "result", "subtype"}),
          subtype);
    }

    // Check that fetching the object by heap snapshot ID works.
    std::string responseObjectID =
        jsonScope_.getString(resp0, {"result", "result", "objectId"});
    sendRequest(
        "HeapProfiler.getHeapObjectId",
        msgId,
        [responseObjectID](::hermes::JSONEmitter &json) {
          json.emitKeyValue("objectId", responseObjectID);
        });
    auto resp1 = expectResponse(std::nullopt, msgId++);
    EXPECT_EQ(
        atoi(jsonScope_.getString(resp1, {"result", "heapSnapshotObjectId"})
                 .c_str()),
        objID);
  };

  // Test once before a collection.
  testObject(globalObjID, "object", "Object", "Object", nullptr);
  testObject(storedObjID, "object", "Array", "Array(3)", "array");

  // Force a collection to move the heap.
  waitFor<bool>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      runtime_->instrumentation().collectGarbage("test");
      promise->set_value(true);
    });
  });

  // A collection should not disturb the unique ID lookup, and it should be
  // the same object as before. Note that it won't have the same remote ID,
  // because Hermes doesn't do uniquing.
  testObject(globalObjID, "object", "Object", "Object", nullptr);
  testObject(storedObjID, "object", "Array", "Array(3)", "array");
}

TEST_F(CDPAgentTest, HeapProfilerCollectGarbage) {
  int msgId = 1;

  // Allocate some objects
  scheduleScript(R"(
    a = [];
    for (var i = 0; i < 1000; i++) {
      a[i] = new Object;
    }
  )");

  // Get the heap usage with objects allocated
  double before = waitFor<double>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      promise->set_value(runtime_->instrumentation().getHeapInfo(
          false)["hermes_allocatedBytes"]);
    });
  });

  // Abandon the objects
  scheduleScript("a = null;");

  // Collect garbage
  sendAndCheckResponse("HeapProfiler.collectGarbage", msgId);

  // Get the heap usage after collection
  double after = waitFor<double>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      promise->set_value(runtime_->instrumentation().getHeapInfo(
          false)["hermes_allocatedBytes"]);
    });
  });

  // Expect objects to have been freed
  EXPECT_LT(after, before);
}

TEST_F(CDPAgentTest, HeapProfilerTrackHeapObjects) {
  int msgId = 1;

  sendAndCheckResponse("HeapProfiler.startTrackingHeapObjects", msgId++);

  // Allocate until we get a notification, or timeout
  auto start = std::chrono::high_resolution_clock::now();
  constexpr float timeout = 5.0f;
  while (true) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = now - start;
    if (duration.count() >= timeout) {
      // Timeout
      break;
    }

    // Allocate some more
    scheduleScript(R"(
      a = a || []; // Ensure the array exists
      for (var i = 0; i < 1000; i++) {
        a.push(new Object);
      }
    )");
    waitForScheduledScripts();

    // Check to see if there is a report yet
    auto objectIdNote = tryGetMessage();
    if (objectIdNote) {
      ensureNotification(*objectIdNote, "HeapProfiler.lastSeenObjectId");

      auto statsNote = expectNotification("HeapProfiler.heapStatsUpdate");
      double objectsCount =
          jsonScope_.getNumber(statsNote, {"params", "statsUpdate", "1"});
      double objectSize =
          jsonScope_.getNumber(statsNote, {"params", "statsUpdate", "2"});
      // Assuming zero-sized objects aren't present, the size of objects should
      // be at least as large as the number of objects.
      EXPECT_GE(objectSize, objectsCount);

      // Verified that notifications are arriving; stop waiting.
      break;
    }
  }

  // Stop tracking, and expect a snapshot in summary (possibly with a few
  // lingering tracking notifications first).
  sendRequest("HeapProfiler.stopTrackingHeapObjects", msgId);
  expectHeapSnapshot(msgId++, true /* ignore tracking notifications */);

  // Expect no further responses or notifications.
  expectNothing();
}

TEST_F(CDPAgentTest, HeapProfilerSampling) {
  int msgId = 1;

  // Start sampling.
  {
    sendRequest(
        "HeapProfiler.startSampling", msgId, [](::hermes::JSONEmitter &json) {
          // Sample every 256 bytes to ensure there are some samples. The
          // default is 32768, which is too high for a small example. Note that
          // sampling is a random process, so there's no guarantee there will be
          // any samples in any finite number of allocations. In practice the
          // likelihood is so high that there shouldn't be any issues.
          json.emitKeyValue("samplingInterval", 256);
        });
    ensureOkResponse(waitForMessage(), msgId++);
  }

  // Run a script that allocates some objects.
  scheduleScript(R"(
      function allocator() {
        // Do some allocation.
        return new Object;
      }
      (function main() {
        var a = [];
        for (var i = 0; i < 1000; i++) {
          a[i] = allocator();
        }
      })();
    )");
  waitForScheduledScripts();

  // Stop sampling
  sendRequest("HeapProfiler.stopSampling", msgId);
  auto resp = expectResponse(std::nullopt, msgId++);
  // Ensure the JSON parsed and some samples were produced.
  EXPECT_NE(
      jsonScope_.getArray(resp, {"result", "profile", "samples"})->size(), 0);
  // Don't test the content of the JSON, that is tested via the
  // SamplingHeapProfilerTest.
}

#endif // HERMES_ENABLE_DEBUGGER
