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
  std::vector<LexicalInfo> lexicalInfos;

  Command didPause(Debugger &debugger) override {
    auto &state = debugger.getProgramState();
    pauseReasons.push_back(state.getPauseReason());
    stackTraces.push_back(state.getStackTrace());

    auto &stackTrace = stackTraces.back();
    uint32_t frameCount = stackTrace.callFrameCount();
    for (uint32_t i = 0; i < frameCount; i++) {
      lexicalInfos.push_back(debugger.getProgramState().getLexicalInfo(i));
    }

    return Command::continueExecution();
  }
};

struct DebuggerAPITest : public ::testing::Test {
  void eval(const std::string &code) {
    rt->global().getPropertyAsFunction(*rt, "eval").call(*rt, code);
  }

  std::shared_ptr<HermesRuntime> rt;
  TestEventObserver observer;

  DebuggerAPITest()
      : rt(makeHermesRuntime(((hermes::vm::RuntimeConfig::Builder())
                                  .withEnableBlockScoping(true)
                                  .build()))) {
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

TEST_F(DebuggerAPITest, GetLoadedScriptsTest) {
  auto scripts = rt->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 0);

  eval("var x = 1;");
  scripts = rt->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 1);
  EXPECT_EQ(scripts[0].line, 1);
  EXPECT_EQ(scripts[0].column, 1);
  EXPECT_EQ(scripts[0].fileName, "JavaScript");

  bool foundJavaScript = false;
  bool foundTestJs = false;
  rt->debugJavaScript("var x = 2;", "Test.js", {});
  scripts = rt->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 2);
  for (auto script : scripts) {
    if (script.fileName == "JavaScript") {
      EXPECT_EQ(script.line, 1);
      EXPECT_EQ(script.column, 1);
      foundJavaScript = true;
    } else if (script.fileName == "Test.js") {
      EXPECT_EQ(script.line, 1);
      EXPECT_EQ(script.column, 1);
      foundTestJs = true;
    }
  }
  EXPECT_TRUE(foundJavaScript);
  EXPECT_TRUE(foundTestJs);
}

TEST_F(DebuggerAPITest, ImplicitAsyncPauseTest) {
  rt->getDebugger().triggerAsyncPause(AsyncPauseKind::Implicit);
  eval("var x = 5;");
  EXPECT_EQ(
      std::vector<PauseReason>({PauseReason::AsyncTriggerImplicit}),
      observer.pauseReasons);
}

TEST_F(DebuggerAPITest, ExplicitAsyncPauseTest) {
  rt->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
  eval("var x = 5;");
  EXPECT_EQ(
      std::vector<PauseReason>({PauseReason::AsyncTriggerExplicit}),
      observer.pauseReasons);
}

TEST_F(DebuggerAPITest, ImplicitAndExplicitAsyncPauseTest) {
  rt->getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
  rt->getDebugger().triggerAsyncPause(AsyncPauseKind::Implicit);
  eval("var x = 5;");
  // Current implementation uses flags to track if Implicit or Explicit has been
  // requested, so there is no ordering information. This test just demonstrates
  // that the order the PauseReason gets process could be in different order.
  EXPECT_EQ(
      std::vector<PauseReason>(
          {PauseReason::AsyncTriggerImplicit,
           PauseReason::AsyncTriggerExplicit}),
      observer.pauseReasons);
}

TEST_F(DebuggerAPITest, GetScopes) {
  // Pause with 4 variables in scope.
  eval(R"(
    (function testFunction(one) {
      let two = 2;
      if (one > 0) {
        let three = 3;
        try {
          let four = 4;
          debugger;
        } catch (err) {}
      }
    })(1);
  )");

  EXPECT_EQ(
      std::vector<PauseReason>({PauseReason::DebuggerStatement}),
      observer.pauseReasons);

  EXPECT_GT(observer.lexicalInfos.size(), 0);
  auto &lexicalInfo = observer.lexicalInfos[0];
  EXPECT_GT(lexicalInfo.getScopesCount(), 0);
  EXPECT_EQ(lexicalInfo.getVariablesCountInScope(0), 4);
}

#endif
