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
#include <hermes/Support/SerialExecutor.h>
#include <hermes/hermes.h>
#include <jsi/jsi.h>

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <sys/resource.h>
#endif

using namespace facebook;
using namespace facebook::hermes;
using namespace facebook::hermes::debugger;

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

  bool evaluateBool(const std::string &script) {
    jsi::Value result = runtime_->evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(script)),
        "url");
    return result.getBool();
  }

  jsi::Value shouldStop(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  std::atomic<bool> stopFlag_{};

  std::unique_ptr<::hermes::SerialExecutor> runtimeThread_;
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
  runtime_->global().setProperty(
      *runtime_,
      "shouldStop",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "shouldStop"),
          0,
          std::bind(&AsyncDebuggerAPITest::shouldStop, this, _1, _2, _3, _4)));
  asyncDebuggerAPI_ = AsyncDebuggerAPI::create(*runtime_);

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  // Give the runtime thread the same stack size as the main thread. The runtime
  // thread is the main thread of the HermesRuntime.
  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  runtimeThread_ = std::make_unique<::hermes::SerialExecutor>(limit.rlim_cur);
#else
  runtimeThread_ = std::make_unique<::hermes::SerialExecutor>();
#endif

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

jsi::Value AsyncDebuggerAPITest::shouldStop(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  return stopFlag_.load() ? jsi::Value(true) : jsi::Value(false);
}

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

TEST_F(AsyncDebuggerAPITest, ResumeFromPausedTest) {
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
          asyncDebuggerAPI_->resumeFromPaused(AsyncDebugCommand::Continue);
        });
      });
  EXPECT_EQ(finalEvent, DebuggerEventType::Resumed);
}

TEST_F(AsyncDebuggerAPITest, RemoveLastDebuggerEventCallbackTest) {
  // This needs to be a while-loop because Explicit AsyncBreak will only happen
  // while there is JS to run
  scheduleScript(R"(
    while (!shouldStop()) {
    }
  )");

  // Multiple callbacks are registered to make sure AsyncDebuggerAPI only breaks
  // out of processInterruptWhilePaused() when it reaches 0
  // DebuggerEventCallbacks.
  DebuggerEventCallbackID callbackID =
      asyncDebuggerAPI_->addDebuggerEventCallback_TS(
          [](HermesRuntime &runtime,
             AsyncDebuggerAPI &asyncDebugger,
             DebuggerEventType event) {});

  EXPECT_TRUE(waitFor<bool>(
      [this](auto promise) {
        eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
            [promise](
                HermesRuntime &runtime,
                AsyncDebuggerAPI &asyncDebugger,
                DebuggerEventType event) {
              EXPECT_EQ(event, DebuggerEventType::ExplicitPause);
              promise->set_value(true);
            });
        runtime_->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
      },
      "wait on explicit pause"));

  // Trigger an interrupt to make sure that we're inside
  // processInterruptWhilePaused()
  EXPECT_TRUE(waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [this, promise](HermesRuntime &runtime) {
          // stop the script
          stopFlag_.store(true);

          promise->set_value(asyncDebuggerAPI_->isWaitingForCommand());
        });
  }));

  // Remove one of the DebuggerEventCallbacks. Since this is not the last one,
  // it's expected that we don't exit out of processInterruptWhilePaused().
  asyncDebuggerAPI_->removeDebuggerEventCallback_TS(callbackID);

  // Trigger an interrupt to make sure that we're still waiting for command in
  // processInterruptWhilePaused().
  EXPECT_TRUE(waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [this, promise](HermesRuntime &runtime) {
          promise->set_value(asyncDebuggerAPI_->isWaitingForCommand());
        });
  }));

  EXPECT_TRUE(waitFor<bool>(
      [this](auto promise) {
        // Schedule something on the runtime thread to confirm that we broke out
        // of processInterruptWhilePaused() after removing the last
        // DebuggerEventCallback
        runtimeThread_->add([promise]() { promise->set_value(true); });
        // Removing the last DebuggerEventCallback will signal and cause another
        // iteration of processInterruptWhilePaused()
        asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID_);
      },
      "wait on runtime thread freed up"));
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

TEST_F(AsyncDebuggerAPITest, IsDebuggerAttachedTest) {
  // isDebuggerAttached should be false when no DebuggerEventCallback is
  // registered
  EXPECT_FALSE(waitFor<bool>([this](auto promise) {
    runtimeThread_->add([this, promise]() {
      promise->set_value(evaluateBool("DebuggerInternal.isDebuggerAttached;"));
    });
  }));

  // isDebuggerAttached should be true if any DebuggerEventCallback is
  // registered
  EXPECT_TRUE(waitFor<bool>([this](auto promise) {
    eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [promise](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {});

    runtimeThread_->add([this, promise]() {
      promise->set_value(evaluateBool("DebuggerInternal.isDebuggerAttached;"));
    });
  }));

  // After removing the last DebuggerEventCallback, isDebuggerAttached should
  // become false again.
  EXPECT_FALSE(waitFor<bool>([this](auto promise) {
    asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID_);
    eventCallbackID_ = kInvalidDebuggerEventCallbackID;

    runtimeThread_->add([this, promise]() {
      promise->set_value(evaluateBool("DebuggerInternal.isDebuggerAttached;"));
    });
  }));
}

TEST_F(AsyncDebuggerAPITest, StopInterruptsAfterNewCommand) {
  eventCallbackID_ = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
      [](HermesRuntime &runtime,
         AsyncDebuggerAPI &asyncDebugger,
         DebuggerEventType event) {});

  DebuggerEventCallbackID tmpCallbackID = kInvalidDebuggerEventCallbackID;

  // First run some code to get the Runtime to a paused state
  auto debuggerEvent =
      waitFor<DebuggerEventType>([this, &tmpCallbackID](auto promise) {
        tmpCallbackID = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
            [this, promise, &tmpCallbackID](
                HermesRuntime &runtime,
                AsyncDebuggerAPI &asyncDebugger,
                DebuggerEventType event) {
              asyncDebuggerAPI_->removeDebuggerEventCallback_TS(tmpCallbackID);
              promise->set_value(event);
            });

        scheduleScript(R"(
          debugger;
          while (!shouldStop()) {
          }
        )");
      });
  EXPECT_EQ(debuggerEvent, DebuggerEventType::DebuggerStatement);

  // Once the Runtime is in a paused state, then queue up a bunch of interrupts.
  int interruptRunCount = 0;
  int runCountSnapshot = -1;
  debuggerEvent = waitFor<DebuggerEventType>([this,
                                              &interruptRunCount,
                                              &runCountSnapshot,
                                              &tmpCallbackID](auto promise) {
    // Add a DebuggerEventCallback to take a snapshot of the interruptRunCount.
    tmpCallbackID = asyncDebuggerAPI_->addDebuggerEventCallback_TS(
        [this, promise, &interruptRunCount, &runCountSnapshot, &tmpCallbackID](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          runCountSnapshot = interruptRunCount;
          asyncDebuggerAPI_->removeDebuggerEventCallback_TS(tmpCallbackID);
          promise->set_value(event);
        });

    // Queue up the interrupts inside another interrupt to ensure the queued
    // interrupts aren't executing right away.
    asyncDebuggerAPI_->triggerInterrupt_TS([this, &interruptRunCount](
                                               HermesRuntime &runtime) {
      asyncDebuggerAPI_->triggerInterrupt_TS(
          [&interruptRunCount](HermesRuntime &runtime) {
            interruptRunCount++;
          });
      asyncDebuggerAPI_->triggerInterrupt_TS(
          [&interruptRunCount](HermesRuntime &runtime) {
            interruptRunCount++;
          });
      asyncDebuggerAPI_->triggerInterrupt_TS([this](HermesRuntime &runtime) {
        asyncDebuggerAPI_->resumeFromPaused(AsyncDebugCommand::Continue);
      });
      asyncDebuggerAPI_->triggerInterrupt_TS(
          [&interruptRunCount](HermesRuntime &runtime) {
            interruptRunCount++;
          });
    });
  });
  EXPECT_EQ(debuggerEvent, DebuggerEventType::Resumed);

  // Expect to have run count = 2 because there are 2 interrupts queued before
  // the one that sets next command to continue.
  EXPECT_EQ(runCountSnapshot, 2);

  asyncDebuggerAPI_->removeDebuggerEventCallback_TS(eventCallbackID_);

  // break out of loop
  stopFlag_.store(true);

  EXPECT_TRUE(waitFor<bool>([this](auto promise) {
    runtimeThread_->add([promise]() { promise->set_value(true); });
  }));
}

TEST_F(AsyncDebuggerAPITest, SetEvalCommandTest) {
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

        scheduleScript(R"(
          const a = 123;
          debugger;
        )");
      });
  EXPECT_EQ(debuggerEvent, DebuggerEventType::DebuggerStatement);

  // Verify evalWhilePaused() gets executed properly.
  double value = waitFor<double>([this](auto promise) {
    asyncDebuggerAPI_->triggerInterrupt_TS(
        [this, promise](HermesRuntime &runtime) {
          asyncDebuggerAPI_->evalWhilePaused(
              "a",
              0,
              [promise](
                  HermesRuntime &runtime, const debugger::EvalResult &result) {
                EXPECT_TRUE(result.value.isNumber());
                EXPECT_FALSE(result.isException);
                promise->set_value(result.value.getNumber());
              });
        });
  });
  EXPECT_EQ(value, 123);
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
