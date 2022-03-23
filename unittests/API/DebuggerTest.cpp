/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include <gtest/gtest.h>
#include <hermes/DebuggerAPI.h>
#include <hermes/hermes.h>
#include <memory>

using namespace facebook::hermes;
using namespace facebook::hermes::debugger;

struct TestEventObserver : public debugger::EventObserver {
  std::vector<PauseReason> pauseReasons;
  std::vector<StackTrace> stackTraces;

  Command didPause(Debugger &debugger) override {
    pauseReasons.push_back(debugger.getProgramState().getPauseReason());
    stackTraces.push_back(debugger.getProgramState().getStackTrace());
    return Command::continueExecution();
  }
};

struct DebuggerAPITest : public ::testing::Test {
  void eval(const std::string &code) {
    rt->global().getPropertyAsFunction(*rt, "eval").call(*rt, code);
  }

  std::shared_ptr<HermesRuntime> rt;
  TestEventObserver observer;

  DebuggerAPITest() : rt(makeHermesRuntime()) {
    rt->getDebugger().setEventObserver(&observer);
  }
};

TEST_F(DebuggerAPITest, SetAgentTest) {
  // A basic test that exercises setting the debugger event observer by virtue
  // of running the DebuggerAPITest constructor.
}

TEST_F(DebuggerAPITest, BasicPauseTest) {
  // Test that a basic Pause scenario works.
  eval("var x = 5; debugger; x=8; debugger;");
  EXPECT_EQ(
      std::vector<PauseReason>(
          {PauseReason::DebuggerStatement, PauseReason::DebuggerStatement}),
      observer.pauseReasons);
}

TEST_F(DebuggerAPITest, SingleFrameStackTraceTest) {
  using namespace facebook;

  rt->getDebugger().setPauseOnThrowMode(PauseOnThrowMode::All);

  jsi::Function nativeThrower = jsi::Function::createFromHostFunction(
      *rt,
      jsi::PropNameID::forAscii(*rt, "nativeThrower"),
      0,
      [](jsi::Runtime &rt,
         const jsi::Value &thisVal,
         const jsi::Value *args,
         size_t count) -> jsi::Value {
        throw jsi::JSINativeException("NativeException");
      });
  rt->global().setProperty(*rt, "nativeThrower", nativeThrower);

  jsi::Function caller = jsi::Function::createFromHostFunction(
      *rt,
      jsi::PropNameID::forAscii(*rt, "caller"),
      0,
      [](jsi::Runtime &rt,
         const jsi::Value &thisVal,
         const jsi::Value *args,
         size_t count) -> jsi::Value {
        jsi::Function jsThrower =
            rt.global().getPropertyAsFunction(rt, "jsThrower");
        return jsThrower.call(rt);
      });
  rt->global().setProperty(*rt, "caller", caller);

  eval(
      "globalThis.jsThrower = function jsThrower() { throw 1; };\n"
      "globalThis.tester = function tester() {"
      "  try { throw 2; }"
      "  catch (e) { caller(); } };"
      "");

  jsi::Function tester =
      rt->global().getProperty(*rt, "tester").asObject(*rt).asFunction(*rt);
  try {
    tester.call(*rt);
  } catch (jsi::JSIException &) {
    try {
      tester.call(*rt);
    } catch (jsi::JSError &err) {
      EXPECT_EQ(1, err.value().getNumber());
    }
  }

  ASSERT_EQ(
      std::vector<PauseReason>(
          {PauseReason::Exception,
           PauseReason::Exception,
           PauseReason::Exception}),
      observer.pauseReasons);

  ASSERT_EQ(3, observer.stackTraces[2].callFrameCount());
  ASSERT_EQ(
      "jsThrower", observer.stackTraces[2].callFrameForIndex(0).functionName);
  ASSERT_EQ(
      "(native)", observer.stackTraces[2].callFrameForIndex(1).functionName);
  ASSERT_EQ(
      "tester", observer.stackTraces[2].callFrameForIndex(2).functionName);
}

#endif
