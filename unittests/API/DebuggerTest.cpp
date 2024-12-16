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

#include <llvh/ADT/ScopeExit.h>

using namespace facebook::hermes;
using namespace facebook::hermes::debugger;

struct TestEventObserver : public debugger::EventObserver {
  std::vector<PauseReason> pauseReasons;
  std::vector<StackTrace> stackTraces;
  std::vector<LexicalInfo> lexicalInfos;
  std::vector<std::string> thrownValueStrings;
  bool useCaptureStackTrace = false;

  Command didPause(Debugger &debugger) override {
    auto &state = debugger.getProgramState();
    pauseReasons.push_back(state.getPauseReason());
    if (useCaptureStackTrace) {
      stackTraces.push_back(debugger.captureStackTrace());
    } else {
      stackTraces.push_back(state.getStackTrace());
    }

    auto &stackTrace = stackTraces.back();
    uint32_t frameCount = stackTrace.callFrameCount();
    for (uint32_t i = 0; i < frameCount; i++) {
      lexicalInfos.push_back(debugger.getProgramState().getLexicalInfo(i));
    }

    thrownValueStrings.push_back(
        debugger.getThrownValue().toString(*runtime_).utf8(*runtime_));

    return Command::continueExecution();
  }

  void setRuntime(std::shared_ptr<HermesRuntime> runtime) {
    runtime_ = runtime;
  }

 private:
  std::shared_ptr<HermesRuntime> runtime_;
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
    observer.setRuntime(rt);
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

TEST_F(DebuggerAPITest, ExceptionDetailsFromDidPauseTest) {
  using namespace facebook;

  rt->getDebugger().setPauseOnThrowMode(PauseOnThrowMode::All);

  try {
    eval(R"(
      try {
        throw new Error('test');
      } catch (e) {
        throw 123;
      }
    )");
  } catch (jsi::JSError &err) {
    EXPECT_EQ(123, err.value().getNumber());
  }

  ASSERT_EQ(
      std::vector<PauseReason>(
          {PauseReason::Exception, PauseReason::Exception}),
      observer.pauseReasons);

  ASSERT_EQ(
      std::vector<std::string>({"Error: test", "123"}),
      observer.thrownValueStrings);
}

TEST_F(DebuggerAPITest, GetEmptyThownValueTest) {
  using namespace facebook;

  // Before execution
  EXPECT_TRUE(rt->getDebugger().getThrownValue().isUndefined());

  eval(R"(
    debugger;

    try {
      throw new Error('test');
    } catch (e) {
      debugger;
    }

    debugger;
  )");

  // During execution: before throwing, while catching, and after try/catch
  ASSERT_EQ(
      std::vector<std::string>({"undefined", "undefined", "undefined"}),
      observer.thrownValueStrings);

  // After execution
  EXPECT_TRUE(rt->getDebugger().getThrownValue().isUndefined());
}

TEST_F(DebuggerAPITest, CaptureStackTraceTest) {
  auto setCaptureStackTraceFlag =
      llvh::make_scope_exit([this] { observer.useCaptureStackTrace = false; });
  using namespace facebook;

  // Make sure when we're not in the interpreter loop, captureStackTrace() works
  // and just return 0 call frames.
  StackTrace stackTrace = rt->getDebugger().captureStackTrace();
  ASSERT_EQ(stackTrace.callFrameCount(), 0);

  jsi::Function captureStackTrace = jsi::Function::createFromHostFunction(
      *rt,
      jsi::PropNameID::forAscii(*rt, "captureStackTrace"),
      0,
      [&stackTrace](
          jsi::Runtime &runtime, const jsi::Value &, const jsi::Value *, size_t)
          -> jsi::Value {
        stackTrace = static_cast<HermesRuntime &>(runtime)
                         .getDebugger()
                         .captureStackTrace();
        return jsi::Value::undefined();
      });
  rt->global().setProperty(*rt, "captureStackTrace", captureStackTrace);

  // Test for using captureStackTrace from a HostFunction
  eval(R"(
    function level4() {
      captureStackTrace();      // line 3
    }

    function level3() {
      level4(...arguments);     // line 7
    }

    function level2() {
      level3.call();            // line 11
    }

    function level1() {
      var lvl2 = level2.bind();
      lvl2();                   // line 16
    }

    level1();                   // line 19
  )");

  ASSERT_EQ(stackTrace.callFrameCount(), 9);

  uint32_t frameIndex = 0;

  // Even though the JavaScript looks like it should have the top frame being
  // the level3() function, the top call frame is actually a
  // FinalizedNativeFunction corresponding to the HostFunction. It'll be
  // displayed as "(native)".
  CallFrameInfo frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "(native)");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, kInvalidLocation);
  ASSERT_EQ(frame.location.line, kInvalidLocation);

  // Next 2 frames are for the `level4(...arguments)` call. This causes the
  // JSFunction to be called through hermesBuiltinApply, which creates an
  // additional native call frame.
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level4");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 2);
  ASSERT_EQ(frame.location.line, 3);
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "(native)");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, kInvalidLocation);
  ASSERT_EQ(frame.location.line, kInvalidLocation);

  // Next 2 frames are for the `level3.call()` call. This causes the JSFunction
  // to be called through functionPrototypeCall, which creates an additional
  // native call frame.
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level3");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 2);
  ASSERT_EQ(frame.location.line, 7);
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "(native)");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, kInvalidLocation);
  ASSERT_EQ(frame.location.line, kInvalidLocation);

  // This is for the `lvl2()` BoundFuntion call after creating the bound
  // function via function.prototype.bind. This doesn't create additional native
  // call frame, but it does have the characteristic that it has a null
  // savedCodeBlock. This is handled in getStackTrace() when getting savedIP and
  // savedCodeBlock to properly transition to the correct CodeBlock for the
  // "level1" frame.
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level2");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 2);
  ASSERT_EQ(frame.location.line, 11);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level1");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 2);
  ASSERT_EQ(frame.location.line, 16);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "global");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 2);
  ASSERT_EQ(frame.location.line, 19);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "(native)");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, kInvalidLocation);
  ASSERT_EQ(frame.location.line, kInvalidLocation);

  // Test for using captureStackTrace while in didPause()
  observer.useCaptureStackTrace = true;
  eval(R"(
    function level3() {
      debugger;            // line 3
    }

    function level2() {
      level3();            // line 7
    }

    function level1() {
      level2();            // line 11
    }

    level1();              // line 14
  )");

  stackTrace = observer.stackTraces[0];

  frameIndex = 0;

  // When called from didPause() due to the debugger; statement, there isn't an
  // extra indirection like the HostFunction case. The top frame is the level3()
  // call frame.
  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level3");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 3);
  ASSERT_EQ(frame.location.line, 3);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level2");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 3);
  ASSERT_EQ(frame.location.line, 7);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "level1");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 3);
  ASSERT_EQ(frame.location.line, 11);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "global");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, 3);
  ASSERT_EQ(frame.location.line, 14);

  frame = stackTrace.callFrameForIndex(frameIndex++);
  ASSERT_EQ(frame.functionName, "(native)");
  ASSERT_EQ(frame.location.fileName, "");
  ASSERT_EQ(frame.location.fileId, kInvalidLocation);
  ASSERT_EQ(frame.location.line, kInvalidLocation);
}

TEST_F(DebuggerAPITest, GetLoadedScriptsTest) {
  // Don't use the member runtime, as we don't want to
  // trigger the observer on script loads.
  std::unique_ptr<HermesRuntime> runtime = makeHermesRuntime(
      ((hermes::vm::RuntimeConfig::Builder())
           .withCompilationMode(
               hermes::vm::CompilationMode::ForceLazyCompilation)
           .build()));

  auto scripts = runtime->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 0);

  runtime->global()
      .getPropertyAsFunction(*runtime, "eval")
      .call(*runtime, "var x = 1;");
  scripts = runtime->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 1);
  EXPECT_EQ(scripts[0].line, 1);
  EXPECT_EQ(scripts[0].column, 1);
  EXPECT_EQ(scripts[0].fileName, "");

  bool foundJavaScript = false;
  bool foundTestJs = false;
  // Use a script containing a function (in combination with forced lazy
  // compilation in the test setup) to cause multiple runtime modules for this
  // single script, allowing this test to verify we don't get duplicate
  // results.
  runtime->debugJavaScript("(function(){var x = 2;})()", "Test.js", {});
  scripts = runtime->getDebugger().getLoadedScripts();
  EXPECT_EQ(scripts.size(), 2);
  for (auto script : scripts) {
    if (script.fileName == "") {
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

  bool foundGC = false;
  bool foundAfterGC = false;
  runtime->debugJavaScript("gc();", "gc.js", {});
  runtime->debugJavaScript("var y = 1;", "after-gc.js", {});
  scripts = runtime->getDebugger().getLoadedScripts();
  // The 2 scripts prior to gc(); got cleaned up
  EXPECT_EQ(scripts.size(), 2);
  for (auto script : scripts) {
    if (script.fileName == "gc.js") {
      foundGC = true;
    } else if (script.fileName == "after-gc.js") {
      foundAfterGC = true;
    }
  }
  EXPECT_TRUE(foundGC);
  EXPECT_TRUE(foundAfterGC);
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
