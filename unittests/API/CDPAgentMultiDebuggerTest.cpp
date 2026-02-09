/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CDPAgentTest.h"

#ifdef HERMES_ENABLE_DEBUGGER

using namespace facebook::hermes;
using namespace facebook::hermes::cdp;

using CDPAgentMultiDebuggerTest = CDPAgentTest;

TEST_F(CDPAgentMultiDebuggerTest, MultiDebuggerSmokeTest) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript("var x = 1;");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
}

TEST_F(CDPAgentMultiDebuggerTest, DebuggerEventsAfterOneAgentDisables) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());
  sendAndCheckResponse(
      "Debugger.disable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(R"(
    debugger;  // line 1
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});

  // Disabled agent should NOT receive notifications
  expectNothing(secondMessages.get());

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentMultiDebuggerTest, BothAgentsReceiveScriptParsed) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript("function foo() { return 42; }");

  auto *notification1 = expectNotification("Debugger.scriptParsed");
  auto *notification2 =
      expectNotification("Debugger.scriptParsed", secondMessages.get());

  EXPECT_EQ(
      jsonScope_.getString(notification1, {"params", "url"}),
      jsonScope_.getString(notification2, {"params", "url"}));
}

TEST_F(CDPAgentMultiDebuggerTest, BothAgentsReceivePauseNotification) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(R"(
    debugger;  // line 1
  )");

  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(waitForMessage(), "other", {{"global", 1, 1}});
  ensurePaused(secondMessages->waitForMessage(), "other", {{"global", 1, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);

  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

TEST_F(CDPAgentMultiDebuggerTest, LateEnablingAgentReceivesPreviousScripts) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript("var firstScript = 1;");
  auto *firstScriptNotification = expectNotification("Debugger.scriptParsed");
  std::string firstScriptId =
      jsonScope_.getString(firstScriptNotification, {"params", "scriptId"});

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());

  // Second agent receives scriptParsed for the previously loaded script
  auto *secondScriptNotification =
      expectNotification("Debugger.scriptParsed", secondMessages.get());
  std::string secondScriptId =
      jsonScope_.getString(secondScriptNotification, {"params", "scriptId"});

  EXPECT_EQ(firstScriptId, secondScriptId);

  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  // First agent should NOT receive a duplicate scriptParsed
  EXPECT_EQ(tryGetMessage(), std::nullopt);
}

TEST_F(CDPAgentMultiDebuggerTest, LastSetBreakpointsActiveWins) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(
      R"(
    function breakpointTarget() {  // line 1
      var x = 1;                   // line 2 - breakpoint here
      return x;                    // line 3
    }
  )",
      "breakpoint_test.js");
  auto *bpScriptParsed = expectNotification("Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  std::string scriptId =
      jsonScope_.getString(bpScriptParsed, {"params", "scriptId"});

  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureOkResponse(waitForMessage(), msgId - 1);

  sendRequest(
      "Debugger.setBreakpointsActive",
      msgId++,
      [](::hermes::JSONEmitter &json) { json.emitKeyValue("active", false); });
  ensureOkResponse(waitForMessage(), msgId - 1);

  // Agent 2 sets breakpoints to active - this should win
  sendRequest(
      "Debugger.setBreakpointsActive",
      msgId,
      [](::hermes::JSONEmitter &json) { json.emitKeyValue("active", true); },
      secondAgent.get());
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  scheduleScript("breakpointTarget();", "call_test.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(
      waitForMessage(),
      "other",
      {{"breakpointTarget", 2, 2}, {"global", 0, 1}});
  ensurePaused(
      secondMessages->waitForMessage(),
      "other",
      {{"breakpointTarget", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

/// Enabling Debugger in a new agent implicitly sets breakpoints to active,
/// overriding any previous setBreakpointsActive(false) from other agents.
TEST_F(CDPAgentMultiDebuggerTest, EnablingDebuggerActivatesBreakpoints) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  scheduleScript(
      R"(
    function breakpointTarget() {  // line 1
      var x = 1;                   // line 2 - breakpoint here
      return x;                    // line 3
    }
  )",
      "breakpoint_test.js");
  auto *scriptParsed = expectNotification("Debugger.scriptParsed");
  std::string scriptId =
      jsonScope_.getString(scriptParsed, {"params", "scriptId"});

  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureOkResponse(waitForMessage(), msgId - 1);

  sendRequest(
      "Debugger.setBreakpointsActive",
      msgId++,
      [](::hermes::JSONEmitter &json) { json.emitKeyValue("active", false); });
  ensureOkResponse(waitForMessage(), msgId - 1);

  // Enabling a second agent should implicitly set breakpoints active
  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  scheduleScript("breakpointTarget();", "call_test.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(
      waitForMessage(),
      "other",
      {{"breakpointTarget", 2, 2}, {"global", 0, 1}});
  ensurePaused(
      secondMessages->waitForMessage(),
      "other",
      {{"breakpointTarget", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

/// Blackboxing requires consensus: if one agent blackboxes a location but
/// another doesn't, the location is NOT treated as blackboxed.
TEST_F(CDPAgentMultiDebuggerTest, BlackboxingLostWhenSecondAgentConnects) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);
  sendSetBlackboxPatternsAndCheckResponse(msgId++, {"blackboxed\\.js"});

  // With only one agent, the debugger statement is skipped (blackboxed)
  scheduleScript("debugger;", "blackboxed.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  waitForScheduledScripts();

  // Connect a second agent WITHOUT setting blackbox patterns
  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  // Now the same script pauses because there's no consensus on blackboxing
  scheduleScript("debugger;", "blackboxed.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(waitForMessage(), "other", {{"global", 0, 1}});
  ensurePaused(secondMessages->waitForMessage(), "other", {{"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

/// When both agents blackbox the same location, it IS treated as blackboxed.
TEST_F(CDPAgentMultiDebuggerTest, BlackboxingActiveWhenBothAgentsAgree) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);
  sendSetBlackboxPatternsAndCheckResponse(msgId++, {"blackboxed\\.js"});

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());
  sendSetBlackboxPatternsAndCheckResponse(
      msgId++,
      {"blackboxed\\.js"},
      true,
      secondAgent.get(),
      secondMessages.get());

  // Both agents blackbox it, so the debugger statement is skipped
  scheduleScript("debugger;", "blackboxed.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  waitForScheduledScripts();
}

/// A manual breakpoint overrides blackboxing.
TEST_F(CDPAgentMultiDebuggerTest, BreakpointOverridesBlackboxing) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(
      R"(
    function blackboxedFn() {  // line 1
      var x = 1;               // line 2 - breakpoint here
      return x;                // line 3
    }
  )",
      "blackboxed.js");
  auto *parsed = expectNotification("Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  std::string scriptId = jsonScope_.getString(parsed, {"params", "scriptId"});

  sendSetBlackboxPatternsAndCheckResponse(msgId++, {"blackboxed\\.js"});
  sendSetBlackboxPatternsAndCheckResponse(
      msgId++,
      {"blackboxed\\.js"},
      true,
      secondAgent.get(),
      secondMessages.get());

  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureOkResponse(waitForMessage(), msgId - 1);

  scheduleScript("blackboxedFn();", "caller.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  // Breakpoint is hit despite blackboxing
  ensurePaused(
      waitForMessage(), "other", {{"blackboxedFn", 2, 2}, {"global", 0, 1}});
  ensurePaused(
      secondMessages->waitForMessage(),
      "other",
      {{"blackboxedFn", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

#endif // HERMES_ENABLE_DEBUGGER
