/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

  Command didPause(Debugger &debugger) override {
    pauseReasons.push_back(debugger.getProgramState().getPauseReason());
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

#endif
