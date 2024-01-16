/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include <chrono>
#include <functional>
#include <future>

#include <gtest/gtest.h>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/tests/SerialExecutor.h>

using namespace facebook::hermes;
using namespace facebook::hermes::debugger;
using namespace facebook::hermes::inspector_modern::chrome;

constexpr auto kDefaultUrl = "url";

class AsyncDebuggerAPITest : public ::testing::Test {
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

  std::unique_ptr<SerialExecutor> runtimeThread_;
  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<AsyncDebuggerAPI> asyncDebuggerAPI_;
  DebuggerEventCallbackID eventCallbackID_;
};

void AsyncDebuggerAPITest::SetUp() {
  using namespace std::placeholders;

  // HermesRuntime and AsyncDebuggerAPI can be constructed from whatever thread
  // as long as they're done together and without any JavaScript execution in
  // between.
  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());
  asyncDebuggerAPI_ = AsyncDebuggerAPI::create(*runtime_);

  runtimeThread_ = std::make_unique<SerialExecutor>();

  eventCallbackID_ = kInvalidDebuggerEventCallbackID;
}

void AsyncDebuggerAPITest::TearDown() {
  if (asyncDebuggerAPI_) {
    asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID_);
  }

  // Make sure nothing is interacting with the runtime anymore
  runtimeThread_.reset();

  // AsyncDebuggerAPI must be destroyed before HermesRuntime
  asyncDebuggerAPI_.reset();

  runtime_.reset();
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

TEST_F(AsyncDebuggerAPITest, ConstructDestructResetOrderTest) {
  // An empty test to just exercise the expected creation and destruction
  // ordering.
}

TEST_F(AsyncDebuggerAPITest, InterruptWhileNotWaitingForCommandTest) {
  bool isWaitingForCommand = waitFor<bool>([this](auto promise) {
    // We call triggerInterrupt() and then just evaluate some JavaScript
    // that doesn't trigger any debugger events.
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [promise, this](HermesRuntime &runtime) {
          bool isWaitingForCommand = asyncDebuggerAPI_->isWaitingForCommand();
          promise->set_value(isWaitingForCommand);
        });

    scheduleScript("var x = 5;");
  });

  // Validate the interrupt callback got called and isWaitingForCommand()
  // returns value we expected.
  EXPECT_FALSE(isWaitingForCommand);
}

TEST_F(AsyncDebuggerAPITest, InterruptWhileWaitingForCommandTest) {
  bool gotDidPause = waitFor<bool>([this](auto promise) {
    eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          promise->set_value(event == DebuggerEventType::DebuggerStatement);
        });

    scheduleScript("debugger;");
  });

  EXPECT_TRUE(gotDidPause);

  bool ranInterrupt = waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [promise](HermesRuntime &runtime) { promise->set_value(true); });
  });

  // Expect interrupts to run even while didPause is holding onto the runtime
  // thread.
  EXPECT_TRUE(ranInterrupt);
}

TEST_F(AsyncDebuggerAPITest, TriggerInterruptDuringInterruptTest) {
  bool ranInterrupt = waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [promise, this](HermesRuntime &runtime) {
          // Make sure calling triggerInterrupt_TS while inside an interrupt
          // callback doesn't deadlock.
          asyncDebuggerAPI_->triggerInterrupt_TS([](HermesRuntime &runtime) {});
          promise->set_value(true);
        });

    scheduleScript("var x = 5;");
  });

  // Validate the interrupt callback got called.
  EXPECT_TRUE(ranInterrupt);
}

TEST_F(AsyncDebuggerAPITest, InterruptDuringDestructTest) {
  bool ranInterrupt = waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [promise](HermesRuntime &runtime) { promise->set_value(true); });

    runtimeThread_.reset();

    // Destroy AsyncDebuggerAPI and then verify the interrupt callback gets
    // called even though there was no JavaScript executing.
    asyncDebuggerAPI_.reset();
  });

  // Validate the interrupt callback got called.
  EXPECT_TRUE(ranInterrupt);
}

TEST_F(AsyncDebuggerAPITest, AddRemoveEventCallbackDuringEventCallbackTest) {
  bool gotDidPause = waitFor<bool>([this](auto promise) {
    eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise, this](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          // Make sure that while inside a DebuggerEventCallback, calling
          // addDebuggerEventCallback_TS() doesn't deadlock.
          auto id = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
              [](HermesRuntime &runtime,
                 AsyncDebuggerAPI &asyncDebugger,
                 DebuggerEventType event) {});
          // Make sure removing DebuggerEventCallback while inside one doesn't
          // deadlock.
          asyncDebuggerAPI_->removeDebuggerEventCallback_TS(id);

          promise->set_value(true);
        });

    scheduleScript("debugger;");
  });

  EXPECT_TRUE(gotDidPause);
}

TEST_F(AsyncDebuggerAPITest, RemoveCurrentEventCallbackDuringExecutionTest) {
  bool gotDidPause = waitFor<bool>([this](auto promise) {
    eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise, this](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          // Make sure removing the currently executing DebuggerEventCallback
          // doesn't deadlock and works fine. The executing callback is still
          // going to run to finish without getting interrupted,  but won't run
          // again.
          asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID_);
          eventCallbackID_ = kInvalidDebuggerEventCallbackID;

          promise->set_value(true);
        });

    scheduleScript("debugger;");
  });

  EXPECT_TRUE(gotDidPause);
}

TEST_F(AsyncDebuggerAPITest, DebuggerEventCallbackIsWaitingForCommandTest) {
  bool isWaitingForCommand = waitFor<bool>([this](auto promise) {
    eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          // While inside a DebuggerEventCallback, isWaitingForCommand() should
          // return true. This value is checked at the end.
          promise->set_value(asyncDebugger.isWaitingForCommand());
        });

    scheduleScript("debugger;");
  });

  EXPECT_TRUE(isWaitingForCommand);
}

TEST_F(AsyncDebuggerAPITest, SetNextCommandTest) {
  std::shared_ptr<std::promise<DebuggerEventType>> sharedPromise;
  eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
      [&sharedPromise](
          HermesRuntime &runtime,
          AsyncDebuggerAPI &asyncDebugger,
          DebuggerEventType event) { sharedPromise->set_value(event); });

  // Trigger a debugger statement and make sure DebuggerEventCallback gets the
  // expected event type.
  auto debuggerEvent =
      waitFor<DebuggerEventType>([&sharedPromise, this](auto promise) {
        sharedPromise = promise;
        scheduleScript("debugger;");
      });
  EXPECT_EQ(debuggerEvent, DebuggerEventType::DebuggerStatement);

  // After setting the next Command, we expect AsyncDebuggerAPI to exit
  // didPause(). Before it exits, it'll send a Resumed event to all
  // DebuggerEventCallbacks.
  auto finalEvent =
      waitFor<DebuggerEventType>([&sharedPromise, this](auto promise) {
        sharedPromise = promise;

        // Trigger an interrupt to get on the runtime thread to set the next
        // command
        asyncDebuggerAPI_->triggerInterrupt_TS([this](HermesRuntime &runtime) {
          asyncDebuggerAPI_->setNextCommand(Command::continueExecution());
        });
      });
  EXPECT_EQ(finalEvent, DebuggerEventType::Resumed);
}

TEST_F(AsyncDebuggerAPITest, NoDebuggerEventCallbackTest) {
  scheduleScript("debugger;");
  scheduleScript("debugger;");

  // Verify the previous debugger statements don't halt the runtime thread.
  // Verify by scheduling to run something on the runtime thread and check that
  // we're able to.
  EXPECT_TRUE(waitFor<bool>([this](auto promise) {
    runtimeThread_->add([promise]() { promise->set_value(true); });
  }));
}

#else // !HERMES_ENABLE_DEBUGGER

#include <gtest/gtest.h>

#include <hermes/AsyncDebuggerAPI.h>

TEST(AsyncDebuggerAPITest, StubImplementationTest) {
  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  std::unique_ptr<facebook::hermes::HermesRuntime> runtime =
      facebook::hermes::makeHermesRuntime(builder.build());
  std::unique_ptr<facebook::hermes::debugger::AsyncDebuggerAPI>
      asyncDebuggerAPI =
          facebook::hermes::debugger::AsyncDebuggerAPI::create(*runtime);
  EXPECT_TRUE(asyncDebuggerAPI == nullptr);
}

#endif // !HERMES_ENABLE_DEBUGGER
