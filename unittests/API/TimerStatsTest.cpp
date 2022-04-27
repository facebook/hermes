/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/TimerStats.h"

#include "hermes/hermes.h"
#include "jsi/jsi.h"

#include <gtest/gtest.h>

#include <thread>

namespace facebook {
namespace hermes {

static constexpr auto kJSITimerInternalName = "JSITimerInternal";
static constexpr auto kGetTimesName = "getTimes";
static constexpr auto kRuntimeDurationName = "jsi_runtimeDuration";
static constexpr auto kRuntimeCPUDurationName = "jsi_runtimeCPUDuration";
static constexpr auto kTotalDurationName = "jsi_totalDuration";
static constexpr auto kTotalCPUDurationName = "jsi_totalCPUDuration";

class TimerStatsTest : public ::testing::Test {
 public:
  TimerStatsTest() : rt_(makeHermesRuntime()) {}

  void createDecoratedRuntime() {
    rt_ = makeTimedRuntime(std::move(rt_));
  }

  jsi::Runtime &rt() {
    return *rt_;
  }

 private:
  std::unique_ptr<jsi::Runtime> rt_;
};

TEST_F(TimerStatsTest, UndecoratedRuntime) {
  // An undecorated runtime (i.e., a plain HermesRuntime) should not have the
  // JSITimerInternal object.
  auto ret = rt().global().getProperty(rt(), kJSITimerInternalName);
  EXPECT_TRUE(ret.isUndefined());
}

TEST_F(TimerStatsTest, DecoratedRuntime) {
  createDecoratedRuntime();

  // Create a host function and sleep in it for kSleepTime.
  static constexpr std::chrono::duration<double> kSleepTime{0.05};
  jsi::Function::createFromHostFunction(
      rt(),
      jsi::PropNameID::forAscii(rt(), "sleep"),
      0,
      [](jsi::Runtime &, const jsi::Value &, const jsi::Value *, size_t) {
        std::this_thread::sleep_for(kSleepTime);
        return jsi::Value();
      })
      .call(rt());

  // Here we perform some basic validations on the instrumented runtime.

  // 1. There must be a global named JSITimerInternal
  auto jsiTimerInternal =
      rt().global().getProperty(rt(), kJSITimerInternalName);
  ASSERT_TRUE(jsiTimerInternal.isObject());

  // 2. JSITimerInternal must have a function named getTimes.
  auto getTimes =
      jsiTimerInternal.asObject(rt()).getProperty(rt(), kGetTimesName);
  ASSERT_TRUE(getTimes.isObject());
  ASSERT_TRUE(getTimes.asObject(rt()).isFunction(rt()));

  // 3. JSITimerInternal.getTimes should be invokable without args, and
  //    return an object with four number properties (tested below).
  auto times =
      getTimes.asObject(rt()).asFunction(rt()).call(rt()).asObject(rt());

  // 4. The total duration must be at least kSleepTime, and the runtime duration
  //    must be at least kSleepTime shorter than the total duration.
  auto totalDuration = times.getProperty(rt(), kTotalDurationName).asNumber();
  EXPECT_GE(totalDuration, kSleepTime.count());
  auto totalCPUDuration =
      times.getProperty(rt(), kTotalCPUDurationName).asNumber();
  EXPECT_GE(totalCPUDuration, 0);

  auto runtimeDuration =
      times.getProperty(rt(), kRuntimeDurationName).asNumber();
  EXPECT_LE(runtimeDuration, totalDuration - kSleepTime.count());
  auto runtimeCPUDuration =
      times.getProperty(rt(), kRuntimeCPUDurationName).asNumber();
  EXPECT_GE(runtimeCPUDuration, 0);
}
} // namespace hermes
} // namespace facebook
