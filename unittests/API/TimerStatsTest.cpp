/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/TimerStats.h"

#include "hermes/hermes.h"
#include "jsi/jsi.h"

#include <gtest/gtest.h>

namespace facebook {
namespace hermes {

static constexpr auto kJSITimerInternalName = "JSITimerInternal";
static constexpr auto kGetTimesName = "getTimes";
static constexpr auto kRuntimeDurationName = "jsi_runtimeDuration";
static constexpr auto kRuntimeCPUDurationName = "jsi_runtimeCPUDuration";

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
  //    return an object with two number properties: jsi_runtimeDuration and
  //    jsi_runtimeCPUDuration.
  auto times =
      getTimes.asObject(rt()).asFunction(rt()).call(rt()).asObject(rt());

  auto runtimeDuration = times.getProperty(rt(), kRuntimeDurationName);
  EXPECT_TRUE(runtimeDuration.isNumber());

  auto runtimeCPUDuration = times.getProperty(rt(), kRuntimeCPUDurationName);
  EXPECT_TRUE(runtimeDuration.isNumber());
}
} // namespace hermes
} // namespace facebook
