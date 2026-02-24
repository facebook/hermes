/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPAGENTTEST_H
#define HERMES_UNITTESTS_API_CDPAGENTTEST_H

#ifdef HERMES_ENABLE_DEBUGGER

#include <chrono>
#include <future>
#include <queue>

#include <gtest/gtest.h>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/CompileJS.h>
#include <hermes/Support/JSONEmitter.h>
#include <hermes/Support/SerialExecutor.h>
#include <hermes/VM/HeapSnapshot.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/CDPDebugAPI.h>
#include <hermes/cdp/JSONValueInterfaces.h>
#include <hermes/hermes.h>
#include <jsi/instrumentation.h>

#include <llvh/ADT/ScopeExit.h>

#include "CDPJSONHelpers.h"

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <sys/resource.h>
#endif

namespace facebook {
namespace hermes {

constexpr auto kDefaultUrl = "url";
#ifndef HERMES_MEMORY_INSTRUMENTATION
constexpr auto kMemoryInstrumentationSubstring = "memory instrumentation";
#endif

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
  future.wait();
  return future.get();
}

m::runtime::CallArgument makeValueCallArgument(std::string val);
m::runtime::CallArgument makeUnserializableCallArgument(std::string val);
m::runtime::CallArgument makeObjectIdCallArgument(
    m::runtime::RemoteObjectId objectId);

} // namespace hermes
} // namespace facebook

// CDPAgentTest class is at global scope for friend access from CDPAgent
class CDPAgentTest : public ::testing::Test {
 public:
  /// Thread-safe message queue for receiving CDP messages.
  class MessageQueue {
   public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    // Non-movable to ensure references captured in lambdas remain valid.
    MessageQueue(MessageQueue &&) = delete;
    MessageQueue &operator=(MessageQueue &&) = delete;

    // Non-copyable.
    MessageQueue(const MessageQueue &) = delete;
    MessageQueue &operator=(const MessageQueue &) = delete;

    void push(std::string message);

    std::string waitForMessage(
        std::string context = "reply",
        std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    std::optional<std::string> tryGetMessage();

   private:
    std::mutex mutex_;
    std::condition_variable hasMessage_;
    std::queue<std::string> messages_;
  };

  using CDPAgentAndQueue = std::pair<
      std::unique_ptr<facebook::hermes::cdp::CDPAgent>,
      std::shared_ptr<MessageQueue>>;

  /// Creates a secondary CDP session for multi-session tests.
  CDPAgentAndQueue createCDPAgentAndQueue();

  void handleRuntimeTask(facebook::hermes::debugger::RuntimeTask task);

 protected:
  static constexpr int32_t kTestExecutionContextId_ = 1;

  void SetUp() override;
  void TearDown() override;

  void setupRuntimeTestInfra();
  void preserveStateAndResetTestEnv(bool saveAgentInRuntimeThread);

  void scheduleScript(
      const std::string &script,
      const std::string &url = facebook::hermes::kDefaultUrl,
      facebook::hermes::HermesRuntime::DebugFlags flags =
          facebook::hermes::HermesRuntime::DebugFlags{});

  void waitForScheduledScripts();

  /// waits for the next message of either kind (response or notification)
  /// from the debugger. returns the message. throws on timeout.
  std::string waitForMessage(
      std::string context = "reply",
      std::optional<std::chrono::milliseconds> timeout = std::nullopt,
      MessageQueue *queue = nullptr);

  /// check to see if a response or notification is immediately available.
  /// returns the message, or nullopt if no message is available.
  std::optional<std::string> tryGetMessage(MessageQueue *queue = nullptr);

  void expectNothing(MessageQueue *queue = nullptr);
  facebook::hermes::cdp::JSONObject *expectNotification(
      const std::string &method,
      MessageQueue *queue = nullptr);
  facebook::hermes::cdp::JSONObject *expectResponse(
      const std::optional<std::string> &method,
      int id,
      MessageQueue *queue = nullptr);

  /// Wait for a message, validate that it is an error with the specified
  /// \p messageID, and assert that the error description contains the
  /// specified \p substring.
  void expectErrorMessageContaining(
      const std::string &substring,
      long long messageID,
      MessageQueue *queue = nullptr);

  /// Expect a sequence of messages conveying a heap snapshot:
  /// 1 or more notifications containing chunks of the snapshot JSON object
  /// followed by an OK response to the snapshot request.
  /// \p messageID specifies the id of the snapshot request.
  /// \p ignoreTrackingNotifications indicates whether lastSeenObjectId and
  /// heapStatsUpdate notifications are tolerated before the snapshot arrives.
  /// \return the completed heap snapshot JSON object
  facebook::hermes::cdp::JSONObject *expectHeapSnapshot(
      int messageID,
      bool ignoreTrackingNotifications = false,
      MessageQueue *queue = nullptr);

  void sendRequest(
      const std::string &method,
      int id,
      const std::function<void(::hermes::JSONEmitter &)> &setParameters = {},
      facebook::hermes::cdp::CDPAgent *agent = nullptr);
  void sendSetBlackboxedRangesAndCheckResponse(
      int msgId,
      std::string scriptID,
      std::vector<std::pair<int, int>> positions);
  void sendSetBlackboxPatternsAndCheckResponse(
      int msgId,
      std::vector<std::string> patterns,
      bool expectOK = true,
      facebook::hermes::cdp::CDPAgent *agent = nullptr,
      MessageQueue *queue = nullptr);
  void sendParameterlessRequest(
      const std::string &method,
      int id,
      facebook::hermes::cdp::CDPAgent *agent = nullptr);
  void sendAndCheckResponse(
      const std::string &method,
      int id,
      facebook::hermes::cdp::CDPAgent *agent = nullptr,
      MessageQueue *queue = nullptr);
  void sendEvalRequest(
      int id,
      int callFrameId,
      const std::string &expression,
      facebook::hermes::cdp::CDPAgent *agent = nullptr);

  facebook::jsi::Value shouldStop(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::Value &thisVal,
      const facebook::jsi::Value *args,
      size_t count);

  facebook::jsi::Value signalTest(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::Value &thisVal,
      const facebook::jsi::Value *args,
      size_t count);

  void waitForTestSignal(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  /// Store a value provided by a test script so it can be later used by a
  /// test.
  facebook::jsi::Value storeValue(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::Value &thisVal,
      const facebook::jsi::Value *args,
      size_t count);

  /// Move the previously stored value out of the storage location and
  /// return it.
  facebook::jsi::Value takeStoredValue();

  facebook::hermes::cdp::message::runtime::GetPropertiesResponse
  getAndEnsureProps(
      int msgId,
      const std::string &objectId,
      const std::unordered_map<std::string, facebook::hermes::PropInfo> &infos,
      const std::unordered_map<std::string, facebook::hermes::PropInfo>
          &internalInfos = {},
      bool ownProperties = true,
      bool accessorPropertiesOnly = false);

  std::unique_ptr<facebook::hermes::HermesRuntime> runtime_;
  std::unique_ptr<facebook::hermes::cdp::CDPDebugAPI> cdpDebugAPI_;
  std::unique_ptr<::hermes::SerialExecutor> runtimeThread_;
  std::unique_ptr<facebook::hermes::cdp::CDPAgent> cdpAgent_;
  MessageQueue defaultQueue_;

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
  facebook::jsi::Value storedValue_;

  std::vector<std::string> thrownExceptions_;
  facebook::hermes::JSONScope jsonScope_;
};

#endif // HERMES_ENABLE_DEBUGGER
#endif // HERMES_UNITTESTS_API_CDPAGENTTEST_H
