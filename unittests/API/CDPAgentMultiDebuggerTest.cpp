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

/// Cross-session breakpoint collision: When two sessions try to set breakpoints
/// at the same location, the second one fails with an error.
TEST_F(CDPAgentMultiDebuggerTest, SecondSessionRejectsIdenticalBreakpoint) {
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

  // Agent 1 sets breakpoint successfully
  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId - 1, {2, 12});

  // Agent 2 connects and enables
  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  // Agent 2 tries to set breakpoint at same location - should fail
  sendRequest(
      "Debugger.setBreakpoint",
      msgId,
      [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      },
      secondAgent.get());
  ensureErrorResponse(secondMessages->waitForMessage(), msgId++);

  // Agent 1's breakpoint should still work
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

TEST_F(CDPAgentMultiDebuggerTest, SecondSessionRejectsOverlappingBreakpoint) {
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

  // Agent 1 sets breakpoint successfully at the exact line+column
  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 12);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId - 1, {2, 12});

  // Agent 2 connects and enables
  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  // Agent 2 tries to set breakpoint at the same line (Hermes resolves the
  // column)
  sendRequest(
      "Debugger.setBreakpoint",
      msgId,
      [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      },
      secondAgent.get());
  ensureErrorResponse(secondMessages->waitForMessage(), msgId++);

  // Agent 1's breakpoint should still work
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

/// Cross-session independent breakpoints: Two sessions can set breakpoints at
/// different locations without conflict.
TEST_F(CDPAgentMultiDebuggerTest, CrossSessionIndependentBreakpoints) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(
      R"(
    function funcA() {  // line 1
      var x = 1;        // line 2 - agent 1's breakpoint
      return x;         // line 3
    }
    function funcB() {  // line 5
      var y = 2;        // line 6 - agent 2's breakpoint
      return y;         // line 7
    }
  )",
      "breakpoint_test.js");
  auto *scriptParsed = expectNotification("Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  std::string scriptId =
      jsonScope_.getString(scriptParsed, {"params", "scriptId"});

  // Agent 1 sets breakpoint in funcA
  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId - 1, {2, 12});

  // Agent 2 sets breakpoint in funcB (different location - should succeed)
  sendRequest(
      "Debugger.setBreakpoint",
      msgId,
      [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 6);
        json.closeDict();
      },
      secondAgent.get());
  ensureSetBreakpointResponse(
      secondMessages->waitForMessage(), msgId++, {6, 12});

  // Call funcA - agent 1's breakpoint fires
  scheduleScript("funcA();", "call_test.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(waitForMessage(), "other", {{"funcA", 2, 2}, {"global", 0, 1}});
  ensurePaused(
      secondMessages->waitForMessage(),
      "other",
      {{"funcA", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

/// Disabling one agent preserves the other agent's breakpoints.
TEST_F(
    CDPAgentMultiDebuggerTest,
    DisablingAgentPreservesOtherAgentBreakpoints) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  scheduleScript(
      R"(
    function funcA() {  // line 1
      var x = 1;        // line 2 - agent 1's breakpoint
      return x;         // line 3
    }
    function funcB() {  // line 5
      var y = 2;        // line 6 - agent 2's breakpoint
      return y;         // line 7
    }
  )",
      "breakpoint_test.js");
  auto *scriptParsed = expectNotification("Debugger.scriptParsed");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  std::string scriptId =
      jsonScope_.getString(scriptParsed, {"params", "scriptId"});

  // Agent 1 sets breakpoint in funcA
  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId - 1, {2, 12});

  // Agent 2 sets breakpoint in funcB
  sendRequest(
      "Debugger.setBreakpoint",
      msgId,
      [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 6);
        json.closeDict();
      },
      secondAgent.get());
  ensureSetBreakpointResponse(
      secondMessages->waitForMessage(), msgId++, {6, 12});

  // Disable agent 2 - its breakpoints should be deleted
  sendAndCheckResponse(
      "Debugger.disable", msgId++, secondAgent.get(), secondMessages.get());

  // Call funcA - agent 1's breakpoint should still fire
  scheduleScript("funcA();", "call_test.js");
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(waitForMessage(), "other", {{"funcA", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse("Debugger.resume", msgId++);
  ensureNotification(waitForMessage(), "Debugger.resumed");

  // Disabled agent should NOT receive any notifications
  expectNothing(secondMessages.get());
}

/// After an agent disables, another agent can set a breakpoint at the
/// previously occupied location.
TEST_F(CDPAgentMultiDebuggerTest, DisablingAgentFreesLocation) {
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

  // Agent 1 sets breakpoint
  sendRequest(
      "Debugger.setBreakpoint", msgId++, [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      });
  ensureSetBreakpointResponse(waitForMessage(), msgId - 1, {2, 12});

  // Agent 2 connects
  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendRequest("Debugger.enable", msgId, {}, secondAgent.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");
  ensureOkResponse(secondMessages->waitForMessage(), msgId++);

  // Agent 1 disables (freeing the location)
  sendAndCheckResponse("Debugger.disable", msgId++);

  // Agent 2 can now set breakpoint at the same location
  sendRequest(
      "Debugger.setBreakpoint",
      msgId,
      [&](::hermes::JSONEmitter &json) {
        json.emitKey("location");
        json.openDict();
        json.emitKeyValue("scriptId", scriptId);
        json.emitKeyValue("lineNumber", 2);
        json.closeDict();
      },
      secondAgent.get());
  ensureSetBreakpointResponse(
      secondMessages->waitForMessage(), msgId++, {2, 12});

  // Agent 2's breakpoint should work
  scheduleScript("breakpointTarget();", "call_test.js");
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  ensurePaused(
      secondMessages->waitForMessage(),
      "other",
      {{"breakpointTarget", 2, 2}, {"global", 0, 1}});

  sendAndCheckResponse(
      "Debugger.resume", msgId++, secondAgent.get(), secondMessages.get());
  ensureNotification(secondMessages->waitForMessage(), "Debugger.resumed");
}

/// Cross-session deferred breakpoint collision: Two sessions set identical
/// setBreakpointByUrl requests before the script is loaded. When the script
/// loads, the first agent's breakpoint resolves but the second fails
/// due to Hermes's single-breakpoint-per-location limitation.
/// Known deviation from V8: The first-enabled agent's breakpoint wins,
/// regardless of the order in which breakpoints were set.
TEST_F(CDPAgentMultiDebuggerTest, DeferredBreakpointCollisionIdentical) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  // Both agents set identical deferred breakpoints (script not loaded yet)
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {});

  sendRequest(
      "Debugger.setBreakpointByUrl",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      },
      secondAgent.get());
  ensureSetBreakpointByUrlResponse(
      secondMessages->waitForMessage(), msgId++, {});

  // Now load the script - first agent's breakpoint resolves, second fails
  scheduleScript(
      R"(
    function breakpointTarget() {  // line 1
      var x = 1;                   // line 2 - breakpoint here
      return x;                    // line 3
    }
  )",
      "deferred_test.js");

  // First agent: scriptParsed + breakpointResolved
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.breakpointResolved");

  // Second agent: scriptParsed only (breakpoint fails to resolve due to
  // collision)
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  // First agent's breakpoint should work
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

/// Cross-session deferred breakpoint collision: Two sessions set different
/// setBreakpointByUrl requests that resolve to the same bytecode location.
/// When the script loads, the first agent's breakpoint resolves but the
/// second fails.
/// Known deviation from V8: The first-enabled agent's breakpoint wins,
/// regardless of the order in which breakpoints were set.
TEST_F(
    CDPAgentMultiDebuggerTest,
    DeferredBreakpointCollisionDifferentRequests) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  // Agent 1 sets deferred breakpoint at column 0
  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {});

  // Agent 2 sets deferred breakpoint at a different column on the same line
  // (Hermes will resolve both to the same bytecode location)
  sendRequest(
      "Debugger.setBreakpointByUrl",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 6);
      },
      secondAgent.get());
  ensureSetBreakpointByUrlResponse(
      secondMessages->waitForMessage(), msgId++, {});

  // Now load the script - first agent's breakpoint resolves, second fails
  scheduleScript(
      R"(
    function breakpointTarget() {  // line 1
      var x = 1;                   // line 2 - both breakpoints resolve here
      return x;                    // line 3
    }
  )",
      "deferred_test.js");

  // First agent: scriptParsed + breakpointResolved
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.breakpointResolved");

  // Second agent: scriptParsed only (breakpoint fails to resolve due to
  // collision)
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  // First agent's breakpoint should work
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

/// Cross-session deferred breakpoint collision: The second agent sets its
/// breakpoint first, but the first agent's breakpoint still wins when the
/// script loads. This demonstrates that agent order, not breakpoint
/// setting order, determines which breakpoint resolves.
/// Known deviation from V8: The first-enabled agent's breakpoint wins,
/// regardless of the order in which breakpoints were set.
TEST_F(CDPAgentMultiDebuggerTest, DeferredBreakpointCollisionFirstAgentWins) {
  int msgId = 1;

  sendAndCheckResponse("Debugger.enable", msgId++);

  auto [secondAgent, secondMessages] = createCDPAgentAndQueue();
  sendAndCheckResponse(
      "Debugger.enable", msgId++, secondAgent.get(), secondMessages.get());

  // Set breakpoints: Second agent, THEN first.
  sendRequest(
      "Debugger.setBreakpointByUrl",
      msgId,
      [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      },
      secondAgent.get());
  ensureSetBreakpointByUrlResponse(
      secondMessages->waitForMessage(), msgId++, {});

  sendRequest(
      "Debugger.setBreakpointByUrl", msgId, [](::hermes::JSONEmitter &json) {
        json.emitKeyValue("url", "deferred_test.js");
        json.emitKeyValue("lineNumber", 2);
        json.emitKeyValue("columnNumber", 0);
      });
  ensureSetBreakpointByUrlResponse(waitForMessage(), msgId++, {});

  // Now load the script - first agent's breakpoint wins, even though
  // second agent set its breakpoint first chronologically
  scheduleScript(
      R"(
    function breakpointTarget() {  // line 1
      var x = 1;                   // line 2 - breakpoint here
      return x;                    // line 3
    }
  )",
      "deferred_test.js");

  // First agent: scriptParsed + breakpointResolved (wins!)
  ensureNotification(waitForMessage(), "Debugger.scriptParsed");
  ensureNotification(waitForMessage(), "Debugger.breakpointResolved");

  // Second agent: scriptParsed only (breakpoint fails to resolve, even
  // though it was set first)
  ensureNotification(secondMessages->waitForMessage(), "Debugger.scriptParsed");

  // First agent's breakpoint should work
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
