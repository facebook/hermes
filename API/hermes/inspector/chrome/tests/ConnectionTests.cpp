/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <optional>
#include <thread>

#include <gtest/gtest.h>

#include <hermes/CompileJS.h>
#include <hermes/DebuggerAPI.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/JSONValueInterfaces.h>
#include <hermes/inspector/chrome/MessageTypes.h>
#include <hermes/inspector/chrome/MessageTypesInlines.h>
#include <jsi/instrumentation.h>

#include "SyncConnection.h"
#include "TestHelpers.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

namespace m = ::facebook::hermes::inspector_modern::chrome::message;

using namespace std::chrono_literals;

namespace {

struct ConnectionTests : public ::testing::Test {
  ConnectionTests(bool waitForDebugger = false)
      : asyncRuntime(waitForDebugger),
        conn(asyncRuntime, waitForDebugger),
        jsonAlloc(),
        factory(jsonAlloc) {}

  void TearDown() override {
    // We need to make sure the runtime is finished executing scripts before we
    // destruct the debugger connection. Otherwise, if we destruct the
    // connection while scripts are still executing, the script could perform an
    // action (like hitting a breakpoint) that sends a message to the
    // already-deallocated connection.
    asyncRuntime.wait();
  }

  AsyncHermesRuntime asyncRuntime;
  SyncConnection conn;
  JSLexer::Allocator jsonAlloc;
  JSONFactory factory;
};

template <typename ResponseType>
ResponseType expectResponse(SyncConnection &conn, int id) {
  const std::string message = conn.waitForMessage();

  JSLexer::Allocator jsonAlloc;
  JSONFactory factory(jsonAlloc);
  ResponseType resp =
      mustMake<ResponseType>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(resp.id, id);

  return resp;
}

template <typename NotificationType>
NotificationType expectNotification(SyncConnection &conn) {
  const std::string reply = conn.waitForMessage();

  JSLexer::Allocator jsonAlloc;
  JSONFactory factory(jsonAlloc);
  return mustMake<NotificationType>(mustParseStrAsJsonObj(reply, factory));
}

class UnexpectedNotificationException : public std::runtime_error {
 public:
  UnexpectedNotificationException()
      : std::runtime_error("unexpected notification") {}
};

void expectNothing(SyncConnection &conn) {
  try {
    conn.waitForMessage();
  } catch (...) {
    // if no values are received it times out with an exception
    // so we can say that we've succeeded at seeing nothing
    return;
  }

  throw std::runtime_error("received a notification but didn't expect one");
}

struct FrameInfo {
  FrameInfo(const std::string &functionName, int lineNumber, int scopeCount)
      : functionName(functionName),
        lineNumberMin(lineNumber),
        lineNumberMax(lineNumber),
        scopeCount(scopeCount),
        columnNumber(debugger::kInvalidLocation) {}

  FrameInfo &setLineNumberMax(int lineNumberMaxParam) {
    lineNumberMax = lineNumberMaxParam;
    return *this;
  }

  FrameInfo &setScriptId(const std::string &scriptIdParam) {
    scriptId = scriptIdParam;
    return *this;
  }

  FrameInfo &setColumnNumber(int columnNumberParam) {
    columnNumber = columnNumberParam;
    return *this;
  }

  std::string functionName;
  uint32_t lineNumberMin;
  uint32_t lineNumberMax;
  uint32_t scopeCount;
  uint32_t columnNumber;
  std::string scriptId;
};

void expectCallFrames(
    const std::vector<m::debugger::CallFrame> &frames,
    const std::vector<FrameInfo> &infos) {
  EXPECT_EQ(frames.size(), infos.size());

  int i = 0;
  for (const FrameInfo &info : infos) {
    const m::debugger::CallFrame &frame = frames[i];

    EXPECT_EQ(frame.callFrameId, std::to_string(i));
    EXPECT_EQ(frame.functionName, info.functionName);
    EXPECT_GE(frame.location.lineNumber, info.lineNumberMin);
    EXPECT_LE(frame.location.lineNumber, info.lineNumberMax);

    if (info.columnNumber != debugger::kInvalidLocation) {
      EXPECT_EQ(frame.location.columnNumber, info.columnNumber);
    }

    if (info.scriptId.size() > 0) {
      EXPECT_EQ(frame.location.scriptId, info.scriptId);
    }

    // TODO: make expectation more specific once Hermes gives us something other
    // than kInvalidBreakpoint for the file id
    EXPECT_FALSE(frame.location.scriptId.empty());

    if (info.scopeCount > 0) {
      EXPECT_EQ(frame.scopeChain.size(), info.scopeCount);

      for (uint32_t j = 0; j < info.scopeCount; j++) {
        EXPECT_TRUE(frame.scopeChain[j].object.objectId.has_value());
      }
    }

    i++;
  }
}

// Helper to send a request with no params and wait for a response (defaults
// to empty) containing the req id.
template <typename RequestType, typename ResponseType = m::OkResponse>
ResponseType send(SyncConnection &conn, int id) {
  RequestType req;
  req.id = id;
  conn.send(req.toJsonStr());

  return expectResponse<ResponseType>(conn, id);
}

template <typename RequestType, typename ResponseType = m::OkResponse>
ResponseType send(SyncConnection &conn, RequestType req) {
  conn.send(req.toJsonStr());
  return expectResponse<ResponseType>(conn, req.id);
}

void sendRuntimeEvalRequest(
    SyncConnection &conn,
    int id,
    const std::string &expression) {
  m::runtime::EvaluateRequest req;
  req.id = id;
  req.expression = expression;
  req.generatePreview = true;
  conn.send(req.toJsonStr());
}

void sendEvalRequest(
    SyncConnection &conn,
    int id,
    int callFrameId,
    const std::string &expression) {
  m::debugger::EvaluateOnCallFrameRequest req;
  req.id = id;
  req.callFrameId = std::to_string(callFrameId);
  req.expression = expression;
  req.generatePreview = true;
  conn.send(req.toJsonStr());
}

m::runtime::ExecutionContextCreatedNotification expectExecutionContextCreated(
    SyncConnection &conn) {
  auto note =
      expectNotification<m::runtime::ExecutionContextCreatedNotification>(conn);

  EXPECT_EQ(note.context.id, 1);
  EXPECT_EQ(note.context.origin, "");
  EXPECT_EQ(note.context.name, "hermes");

  return note;
}

m::debugger::ScriptParsedNotification expectScriptParsed(
    SyncConnection &conn,
    const std::string &url,
    const std::string &sourceMapURL) {
  auto note = expectNotification<m::debugger::ScriptParsedNotification>(conn);

  EXPECT_EQ(note.url, url);
  EXPECT_GT(note.scriptId.size(), 0);

  if (sourceMapURL.empty()) {
    EXPECT_FALSE(note.sourceMapURL.has_value());
  } else {
    EXPECT_EQ(note.sourceMapURL.value(), sourceMapURL);
  }

  return note;
}

m::debugger::PausedNotification expectPaused(
    SyncConnection &conn,
    const std::string &reason,
    const std::vector<FrameInfo> &infos) {
  auto note = expectNotification<m::debugger::PausedNotification>(conn);

  EXPECT_EQ(note.reason, reason);
  expectCallFrames(note.callFrames, infos);
  // TODO: check breakpoint location for pause once hermes gives that to us

  return note;
}

m::debugger::BreakpointId expectBreakpointResponse(
    SyncConnection &conn,
    int id,
    int line,
    int resolvedLine) {
  auto resp = expectResponse<m::debugger::SetBreakpointByUrlResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_FALSE(resp.breakpointId.empty());
  EXPECT_NE(
      resp.breakpointId,
      std::to_string(facebook::hermes::debugger::kInvalidBreakpoint));

  if (line == -1) {
    EXPECT_EQ(resp.locations.size(), 0);
  } else {
    EXPECT_EQ(resp.locations.size(), 1);
    EXPECT_EQ(resp.locations[0].lineNumber, resolvedLine);
  }

  return resp.breakpointId;
}

void expectEvalResponse(
    SyncConnection &conn,
    int id,
    const char *expectedValue) {
  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "string");
  // The resp.result.value is a JSON blob, so it contains the surrounding
  // quotes. We need to add these quotes to the expected string value.
  EXPECT_EQ(*resp.result.value, "\"" + std::string(expectedValue) + "\"");
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void expectEvalResponse(SyncConnection &conn, int id, bool expectedValue) {
  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "boolean");
  EXPECT_EQ(((*resp.result.value) == "true" ? true : false), expectedValue);
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void expectEvalResponse(SyncConnection &conn, int id, int expectedValue) {
  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "number");
  EXPECT_EQ(std::stoi(*resp.result.value), expectedValue);
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void expectEvalException(
    SyncConnection &conn,
    int id,
    const std::string &exceptionText,
    const std::vector<FrameInfo> infos) {
  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_TRUE(resp.exceptionDetails.has_value());

  m::runtime::ExceptionDetails &details = resp.exceptionDetails.value();
  EXPECT_EQ(details.text, exceptionText);

  // TODO: Hermes doesn't seem to populate the line number for the exception?
  EXPECT_EQ(details.lineNumber, 0);

  EXPECT_TRUE(details.stackTrace.has_value());

  m::runtime::StackTrace &stackTrace = details.stackTrace.value();
  EXPECT_EQ(stackTrace.callFrames.size(), infos.size());

  int i = 0;
  for (const FrameInfo &info : infos) {
    const m::runtime::CallFrame &callFrame = stackTrace.callFrames[i];

    EXPECT_GE(callFrame.lineNumber, info.lineNumberMin);
    EXPECT_LE(callFrame.lineNumber, info.lineNumberMax);
    EXPECT_EQ(callFrame.functionName, info.functionName);

    i++;
  }
}

// Expect a sequence of messages conveying a heap snapshot:
// 1 or more notifications containing chunks of the snapshot JSON object
// followed by an OK response to the snapshot request.
// conn specifies the connection from which to receive. id specifies the
// id of the snapshot request.
void expectHeapSnapshot(SyncConnection &conn, int id) {
  JSLexer::Allocator jsonAlloc;
  JSONFactory factory(jsonAlloc);

  // Expect chunk notifications until the snapshot object is complete. Fail if
  // the object is invalid (e.g. truncated data, excess data, malformed JSON).
  // There is no indication of how many segments there will be, so just receive
  // until the object is complete, then expect no more.
  std::stringstream snapshot;
  do {
    auto note =
        expectNotification<m::heapProfiler::AddHeapSnapshotChunkNotification>(
            conn);
    snapshot << note.chunk;
  } while (!parseStrAsJsonObj(snapshot.str(), factory).has_value());

  // Expect the snapshot response after all chunks have been received.
  expectResponse<m::OkResponse>(conn, id);
}

struct PropInfo {
  PropInfo(const std::string &type) : type(type) {}

  PropInfo &setSubtype(const std::string &subtypeParam) {
    subtype = subtypeParam;
    return *this;
  }

  PropInfo &setValue(const std::string &v) {
    value = v;
    return *this;
  }

  PropInfo &setUnserializableValue(
      const std::string &unserializableValueParam) {
    unserializableValue = unserializableValueParam;
    return *this;
  }

  std::string type;
  std::optional<std::string> subtype;
  std::optional<message::JSONBlob> value;
  std::optional<std::string> unserializableValue;
};

std::unordered_map<std::string, std::string> expectProps(
    SyncConnection &conn,
    int msgId,
    const std::string &objectId,
    const std::unordered_map<std::string, PropInfo> &infos,
    bool ownProperties = true) {
  m::runtime::GetPropertiesRequest req;
  req.id = msgId;
  req.objectId = objectId;
  req.ownProperties = ownProperties;
  conn.send(req.toJsonStr());

  std::unordered_map<std::string, std::string> objectIds;
  auto resp = expectResponse<m::runtime::GetPropertiesResponse>(conn, msgId);

  EXPECT_EQ(resp.result.size(), infos.size());

  for (size_t i = 0; i < resp.result.size(); i++) {
    m::runtime::PropertyDescriptor &desc = resp.result[i];

    auto infoIt = infos.find(desc.name);
    EXPECT_FALSE(infoIt == infos.end()) << desc.name;

    if (infoIt != infos.end()) {
      const PropInfo &info = infoIt->second;

      EXPECT_TRUE(desc.value.has_value());

      m::runtime::RemoteObject &remoteObj = desc.value.value();
      EXPECT_EQ(remoteObj.type, info.type);

      if (info.subtype.has_value()) {
        EXPECT_TRUE(remoteObj.subtype.has_value());
        EXPECT_EQ(remoteObj.subtype.value(), info.subtype.value());
      }

      if (info.value.has_value()) {
        EXPECT_TRUE(remoteObj.value.has_value());
        JSLexer::Allocator jsonAlloc;
        JSONFactory factory(jsonAlloc);
        EXPECT_TRUE(jsonValsEQ(
            mustParseStr(remoteObj.value.value(), factory),
            mustParseStr(info.value.value(), factory)));
      }

      if (info.unserializableValue.has_value()) {
        EXPECT_TRUE(remoteObj.unserializableValue.has_value());
        EXPECT_EQ(
            remoteObj.unserializableValue.value(),
            info.unserializableValue.value());
      }

      if ((info.type == "object" && info.subtype != "null") ||
          info.type == "function") {
        EXPECT_TRUE(remoteObj.objectId.has_value());
        objectIds[desc.name] = remoteObj.objectId.value();
      }
    }
  }

  return objectIds;
}

void expectEvalResponse(
    SyncConnection &conn,
    int id,
    const std::unordered_map<std::string, PropInfo> &infos) {
  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, id);

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "object");
  EXPECT_FALSE(resp.exceptionDetails.has_value());

  EXPECT_TRUE(resp.result.objectId.has_value());
  EXPECT_TRUE(resp.result.preview.has_value());
  EXPECT_EQ(resp.result.preview->type, "object");

  expectProps(conn, id + 1, resp.result.objectId.value(), infos);
}

m::runtime::CallArgument makeValueCallArgument(std::string val) {
  m::runtime::CallArgument ret;
  ret.value = val;
  return ret;
}

m::runtime::CallArgument makeUnserializableCallArgument(std::string val) {
  m::runtime::CallArgument ret;
  ret.unserializableValue = std::move(val);
  return ret;
}

m::runtime::CallArgument makeObjectIdCallArgument(
    m::runtime::RemoteObjectId objectId) {
  m::runtime::CallArgument ret;
  ret.objectId = std::move(objectId);
  return ret;
}

} // namespace

// Test that destructing a CDPHandler does not leave the runtime in any bad
// state.
TEST(CleanUpTests, testDestructor) {
  int msgId = 0;
  AsyncHermesRuntime asyncRuntime;
  asyncRuntime.executeScriptSync(R"(
    var a = 0;
    var console = {
      log: function(...args){
        a += 1;
      }
    };
  )");
  {
    SyncConnection conn{asyncRuntime};
    send<m::runtime::EnableRequest>(conn, msgId++);
    expectExecutionContextCreated(conn);
    asyncRuntime.executeScriptAsync(R"(console.log("hi");)");
    JSLexer::Allocator jsonAlloc;
    JSONFactory factory{jsonAlloc};
    // Verify that console.log has been intercepted by the handler and emits the
    // correct CDP event.
    const std::string message = conn.waitForMessage();
    auto parsedNote = mustParseStrAsJsonObj(message, factory);
    message::JSONValue *methodRes = parsedNote->get("method");
    EXPECT_TRUE(methodRes != nullptr);
    std::unique_ptr<std::string> method =
        message::valueFromJson<std::string>(methodRes);
    EXPECT_TRUE(method != nullptr);
    EXPECT_EQ(*method, "Runtime.consoleAPICalled");
  }
  // The destructor has been called. Calling any console methods should not
  // break, and the original console method is still called.
  asyncRuntime.executeScriptSync(R"(console.log("bye");)");
  std::shared_ptr<HermesRuntime> rt = asyncRuntime.runtime();
  jsi::Value a = rt->global().getProperty(*rt, "a");
  EXPECT_TRUE(a.isNumber());
  // a should have been incremented to two, since there have been two console
  // log statements executed.
  EXPECT_EQ(a.getNumber(), 2);
}

// Not applicable for CDPAgent
TEST_F(ConnectionTests, testUnregisteringCallback) {
  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    var b = a / 2;
  )");

  send<m::debugger::EnableRequest>(conn, 1);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  EXPECT_TRUE(conn.unregisterCallbacks());
  EXPECT_TRUE(conn.onUnregisterWasCalled());
  EXPECT_FALSE(conn.unregisterCallbacks());

  conn.send(R"({"id": 2, "method": "Debugger.foo"})");
  expectNothing(conn);

  EXPECT_TRUE(conn.registerCallbacks());
}

// Also implemented as CDPAgentTest::DebuggerScriptsOnEnable
TEST_F(ConnectionTests, testScriptsOnEnable) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    true
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  send<m::debugger::DisableRequest>(conn, msgId++);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerScriptsOrdering
TEST_F(ConnectionTests, testScriptsOrdering) {
  int msgId = 1;
  std::vector<m::debugger::ScriptParsedNotification> notifications;

  const int kNumScriptParsed = 10;

  send<m::debugger::EnableRequest>(conn, msgId++);

  // Trigger a bunch of scriptParsed notifications to later verify that they get
  // re-sent in the same order
  for (int i = 0; i < kNumScriptParsed; i++) {
    asyncRuntime.executeScriptAsync(R"(
      true
    )");
    auto notification =
        expectNotification<m::debugger::ScriptParsedNotification>(conn);
    notifications.push_back(notification);
  }

  // Make sure a new Debugger.enable will see the same ordering of scriptParsed
  send<m::debugger::EnableRequest>(conn, msgId++);
  for (int i = 0; i < kNumScriptParsed; i++) {
    auto notification =
        expectNotification<m::debugger::ScriptParsedNotification>(conn);
    EXPECT_EQ(notifications[i].scriptId, notification.scriptId);
  }

  // Make sure the same ordering is retained after a disable request
  send<m::debugger::DisableRequest>(conn, msgId++);
  send<m::debugger::EnableRequest>(conn, msgId++);
  for (int i = 0; i < kNumScriptParsed; i++) {
    auto notification =
        expectNotification<m::debugger::ScriptParsedNotification>(conn);
    EXPECT_EQ(notifications[i].scriptId, notification.scriptId);
  }
}

// Also implemented as CDPAgentTest::DebuggerBytecodeScript
TEST_F(ConnectionTests, testBytecodeScript) {
  int msgId = 1;
  send<m::debugger::EnableRequest>(conn, msgId++);

  // Compile code without debug info so that the SourceLocation would be
  // invalid.
  std::string bytecode;
  EXPECT_TRUE(::hermes::compileJS(
      R"(
    true
  )",
      bytecode));

  asyncRuntime.evaluateBytecodeAsync(bytecode);

  // Verify that invalid SourceLocations simply don't trigger scriptParsed
  // notifications
  expectNothing(conn);
}

// Also implemented as CDPAgentTest::CDPAgentRejectsMalformedMethods
TEST_F(ConnectionTests, testRespondsErrorToUnknownRequests) {
  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    var b = a / 2;
  )");

  send<m::debugger::EnableRequest>(conn, 1);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  conn.send(R"({"id": 2, "method": "Debugger.foo"})");
  conn.send(R"({"id": 3, "method": "Debugger.bar", "params": {"a": "b"}})");

  auto resp1 = expectResponse<m::ErrorResponse>(conn, 2);
  EXPECT_EQ(resp1.code, -32601);
  auto resp2 = expectResponse<m::ErrorResponse>(conn, 3);
  EXPECT_EQ(resp2.code, -32601);
}

// Also implemented as CDPAgentTest::DebuggerTestDebuggerStatement
TEST_F(ConnectionTests, testDebuggerStatement) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;       // [1] (line 2) hit debugger statement, resume
    var b = a / 2;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement, resume
  expectPaused(conn, "other", {{"global", 2, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testDebuggerStatementFromPausedWaitEnable) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;       // [1] (line 2) hit debugger statement, resume
    var b = a / 2;
  )");

  // TODO: hack that gives JS the chance to run so that we end up in the
  // PausedWaitEnable state. Will move the entire test to InspectorTests once
  // I get around to refactoring InspectorTests.
  std::this_thread::sleep_for(250ms);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement, resume
  expectPaused(conn, "other", {{"global", 2, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testIsDebuggerAttached) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;       // [1] (line 2) hit debugger statement
                    // [2] evaluate DebuggerInternal.isDebuggerAttached to true
    var b = a / 2;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement
  expectPaused(conn, "other", {{"global", 2, 1}});

  // [2] evaluate DebuggerInternal.IsDebuggerAttached to true
  sendEvalRequest(conn, 0, 0, R"("-> " + DebuggerInternal.isDebuggerAttached)");
  expectEvalResponse(conn, 0, "-> true");

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerStepOver
TEST_F(ConnectionTests, testStepOver) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;       // [1] (line 2) hit debugger statement, step over
    var b = a / 2;  // [2] (line 3) step over
    var c = a + b;  // [3] (line 4) resume
    var d = b - c;
    var e = c * d;
    var f = 10;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2): hit debugger statement, step over
  expectPaused(conn, "other", {{"global", 2, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 3): step over
  expectPaused(conn, "other", {{"global", 3, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 4): resume
  expectPaused(conn, "other", {{"global", 4, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerStepIn
TEST_F(ConnectionTests, testStepIn) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    function addOne(val) {
      return val + 1;   // [3]: resume
    }

    var a = 1 + 2;
    debugger;           // [1] (line 6) hit debugger statement, step over
    var b = addOne(a);  // [2] (line 7) step in
    var c = a + b;
    var d = b - c;
    var e = c * d;
    var f = 10;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 6): hit debugger statement, step over
  expectPaused(conn, "other", {{"global", 6, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 7): step in
  expectPaused(conn, "other", {{"global", 7, 1}});
  send<m::debugger::StepIntoRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 2): resume
  expectPaused(conn, "other", {{"addOne", 2, 2}, {"global", 7, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerStepOut
TEST_F(ConnectionTests, testStepOut) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    function addSquares(a, b) {
      var a2 = a * a;
      debugger;        // [1] (line 3) hit debugger statement, step over
      var b2 = b * b;  // [2] (line 4) step out
      return a2 + b2;
    }

    var c = addSquares(1, 2); // [3] (line 8) resume
    var d = c * c;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 3) hit debugger statement, step over
  expectPaused(conn, "other", {{"addSquares", 3, 2}, {"global", 8, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 4) step out
  expectPaused(conn, "other", {{"addSquares", 4, 2}, {"global", 8, 1}});
  send<m::debugger::StepOutRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 8): resume
  expectPaused(conn, "other", {{"global", 8, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerSetBreakpointByUrl
TEST_F(ConnectionTests, testSetBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;      // [1] (line 2) hit debugger statement,
                   //     set breakpoint on line 5
    var b = a / 2;
    var c = a + b; // [2] (line 5) hit breakpoint, step over
    var d = b - c; // [3] (line 6) resume
    var e = c * d;
    var f = 10;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement, set breakpoint on line 6
  expectPaused(conn, "other", {{"global", 2, 1}});

  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 5;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  expectBreakpointResponse(conn, req.id, 5, 5);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 5) hit breakpoint, step over
  expectPaused(conn, "other", {{"global", 5, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 6) resume
  expectPaused(conn, "other", {{"global", 6, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerSetMultiLocationBreakpoint
TEST_F(ConnectionTests, testSetMultiLocationBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(
      R"(
        function one() {
          var a = 1;
          var b = 2;
          var c = a + b;
        }
  )",
      "someFile");

  asyncRuntime.executeScriptAsync(
      R"(
        function two() {
          var d = 3;
          var e = 4;
          var f = d + e;
        }
  )",
      "someFile");

  asyncRuntime.executeScriptAsync(
      R"(
    debugger; // First break, line 1
    one();
    two();
    debugger; // Last break, line 4
  )",
      "doesntMatch");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // First break, the first "debugger;"
  expectPaused(conn, "other", {{"global", 1, 1}});

  // Set a breakpoint that applies to two Hermes locations
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = "someFile";
  req.id = msgId++;
  req.lineNumber = 2;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  auto resp =
      expectResponse<m::debugger::SetBreakpointByUrlResponse>(conn, req.id);

  EXPECT_FALSE(resp.breakpointId.empty());
  EXPECT_NE(
      resp.breakpointId,
      std::to_string(facebook::hermes::debugger::kInvalidBreakpoint));
  EXPECT_EQ(resp.locations.size(), 2);
  for (auto &location : resp.locations) {
    EXPECT_EQ(location.lineNumber, 2);
  }

  // Continue so we can hit the breakpoints...
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Second break, breakpoint in function "one"
  expectPaused(conn, "other", {{"one", 2, 2}, {"global", 2, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Third break, breakpoint in function "two"
  expectPaused(conn, "other", {{"two", 2, 2}, {"global", 3, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Last break, the second "debugger;""
  expectPaused(conn, "other", {{"global", 4, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerDeleteMultiLocationBreakpoint
TEST_F(ConnectionTests, testDeleteMultiLocationBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(
      R"(
        function one() {
          var a = 1;
          var b = 2;
          var c = a + b;
        }
  )",
      "someFile");

  asyncRuntime.executeScriptAsync(
      R"(
        function two() {
          var d = 3;
          var e = 4;
          var f = d + e;
        }
  )",
      "someFile");

  asyncRuntime.executeScriptAsync(
      R"(
    debugger; // First break, line 1
    one();
    two();
    debugger; // Last break, line 4
  )",
      "doesntMatch");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // First break, the first "debugger;"
  expectPaused(conn, "other", {{"global", 1, 1}});

  // Set a breakpoint that applies to two Hermes locations
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = "someFile";
  req.id = msgId++;
  req.lineNumber = 2;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  auto resp =
      expectResponse<m::debugger::SetBreakpointByUrlResponse>(conn, req.id);
  EXPECT_EQ(resp.locations.size(), 2);

  m::debugger::RemoveBreakpointRequest req2;
  req2.id = msgId++;
  req2.breakpointId = resp.breakpointId;
  conn.send(req2.toJsonStr());
  expectResponse<m::OkResponse>(conn, req2.id);

  // Continue so we can hit the breakpoints...
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Breakpoints that were set and deleted are not hit

  // Last break, the second "debugger;""
  expectPaused(conn, "other", {{"global", 4, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerApplyBreakpointsToNewLoadedScripts
TEST_F(ConnectionTests, testApplyBreakpointsToLoadedScripts) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  // Set a breakpoint that applies to zero Hermes locations
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = "someFile";
  req.id = msgId++;
  req.lineNumber = 2;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  auto resp =
      expectResponse<m::debugger::SetBreakpointByUrlResponse>(conn, req.id);

  // Ensure the breakpoint was successfully created
  EXPECT_FALSE(resp.breakpointId.empty());
  EXPECT_NE(
      resp.breakpointId,
      std::to_string(facebook::hermes::debugger::kInvalidBreakpoint));
  EXPECT_EQ(resp.locations.size(), 0);

  // Load a script that matches the breakpoint description
  asyncRuntime.executeScriptAsync(
      R"(
        var a = 1;
        var b = 2;
        var c = a + b;
  )",
      "someFile");
  auto parsed = expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Ensure the breakpoint was applied
  auto resolution =
      expectNotification<m::debugger::BreakpointResolvedNotification>(conn);
  EXPECT_EQ(resolution.location.scriptId, parsed.scriptId);
  EXPECT_EQ(resolution.location.lineNumber, 2);

  // Ensure the breakpoint is hit
  expectPaused(conn, "other", {{"global", 2, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Load a script that doesn't match the breakpoint description
  asyncRuntime.executeScriptAsync(
      R"(
        var a = 1;
        var b = 2;
        var c = a + b;
  )",
      "anotherFile");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Ensure the breakpoint wasn't applied (i.e. no breakpoint resolved
  // notification and no breakpoints hit).
  expectNothing(conn);
}

// Also implemented as CDPAgentTest::DebuggerSetBreakpointById
TEST_F(ConnectionTests, testSetBreakpointById) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger;      // line 1
    Math.random(); //      2
  )");

  send<m::debugger::EnableRequest>(conn, ++msgId);
  auto script = expectNotification<m::debugger::ScriptParsedNotification>(conn);

  expectPaused(conn, "other", {{"global", 1, 1}});

  m::debugger::SetBreakpointRequest req;
  req.id = ++msgId;
  req.location.scriptId = script.scriptId;
  req.location.lineNumber = 2;

  conn.send(req.toJsonStr());
  auto resp = expectResponse<m::debugger::SetBreakpointResponse>(conn, req.id);
  EXPECT_EQ(resp.actualLocation.scriptId, script.scriptId);
  EXPECT_EQ(resp.actualLocation.lineNumber, 2);
  EXPECT_EQ(resp.actualLocation.columnNumber.value(), 4);

  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);

  expectPaused(conn, "other", {{"global", 2, 1}});

  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerActivateBreakpoints
TEST_F(ConnectionTests, testActivateBreakpoints) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger;      // line 1
    x=100          //      2
    debugger;      //      3
    x=101;         //      4
  )");

  send<m::debugger::EnableRequest>(conn, ++msgId);
  auto script = expectNotification<m::debugger::ScriptParsedNotification>(conn);

  expectPaused(conn, "other", {{"global", 1, 1}});

  // Set breakpoint #1
  m::debugger::SetBreakpointRequest req;
  req.id = ++msgId;
  req.location.scriptId = script.scriptId;
  req.location.lineNumber = 2;
  conn.send(req.toJsonStr());
  expectResponse<m::debugger::SetBreakpointResponse>(conn, req.id);

  // Set breakpoint #2
  req.id = ++msgId;
  req.location.scriptId = script.scriptId;
  req.location.lineNumber = 4;
  conn.send(req.toJsonStr());
  expectResponse<m::debugger::SetBreakpointResponse>(conn, req.id);

  // Disable breakpoints
  m::debugger::SetBreakpointsActiveRequest activeReq;
  activeReq.id = ++msgId;
  activeReq.active = false;
  conn.send(activeReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, activeReq.id);

  // Resume
  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Expect first breakpoint to be skipped, now hitting line #3
  expectPaused(conn, "other", {{"global", 3, 1}});

  // Re-enable breakpoints
  activeReq.id = ++msgId;
  activeReq.active = true;
  conn.send(activeReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, activeReq.id);

  // Resume and expect breakpoints to trigger again
  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectPaused(conn, "other", {{"global", 4, 1}});

  // Continue and exit
  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetBreakpointByIdWithColumnInIndenting) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger;      // line 1
    Math.random(); //      2
  )");

  send<m::debugger::EnableRequest>(conn, ++msgId);
  auto script = expectNotification<m::debugger::ScriptParsedNotification>(conn);

  expectPaused(conn, "other", {{"global", 1, 1}});

  m::debugger::SetBreakpointRequest req;
  req.id = ++msgId;
  req.location.scriptId = script.scriptId;
  req.location.lineNumber = 2;
  // Specify a column location *before* rather than *on* the actual location
  req.location.columnNumber = 0;

  conn.send(req.toJsonStr());
  auto resp = expectResponse<m::debugger::SetBreakpointResponse>(conn, req.id);
  EXPECT_EQ(resp.actualLocation.scriptId, script.scriptId);
  EXPECT_EQ(resp.actualLocation.lineNumber, 2);
  // Check that we resolved the column to the first available location
  EXPECT_EQ(resp.actualLocation.columnNumber.value(), 4);

  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);

  expectPaused(conn, "other", {{"global", 2, 1}});

  send<m::debugger::ResumeRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetLazyBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(
      R"(
    var a = 1 + 2;
    debugger;      // [1] (line 2) hit debugger statement,
                   //     set breakpoint on line 5

    function foo() {
      var b = a / 2;
      var c = a + b; // [2] (line 7) hit breakpoint, step over
      var d = b - c; // [3] (line 8) resume
      var e = c * d;
      var f = 10;
    }

    foo();
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement, set breakpoint on line 6
  expectPaused(conn, "other", {{"global", 2, 1}});

  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 7;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  auto breakpointId = expectBreakpointResponse(conn, req.id, 7, 7);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 7) hit breakpoint, step over
  expectPaused(conn, "other", {{"foo", 7, 2}, {"global", 13, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 8) resume
  expectPaused(conn, "other", {{"foo", 8, 2}, {"global", 13, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetBreakpointWhileRunning) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    while (!shouldStop()) {
      var a = 1;
      var b = 2;
      var c = a + b; // [1] (line 4) first time: step over
                     // [3]          second time: set stop flag, resume
      var d = 10;    // [2] (line 6) resume
    }
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // set breakpoint on line 4: "var c = ..."
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 4;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());

  expectBreakpointResponse(conn, req.id, 4, 4);

  // [1] (line 4) hit breakpoint, step over
  expectPaused(conn, "other", {{"global", 4, 1}});
  send<m::debugger::StepOverRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 6) resume
  expectPaused(conn, "other", {{"global", 6, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] (line 4) hit breakpoint again, set stop flag, resume
  expectPaused(conn, "other", {{"global", 4, 1}});
  asyncRuntime.stop();
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetBreakpointConditional) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 3;
    debugger;      // [1] (line 2) hit debugger statement,
                   //  set conditional breakpoint on lines 4, 5 and 6
    var b = a + 5; // [2] (line 4) skip breakpoint, condition throws
    var c = b - a; // [3] (line 5) skip breakpoint, condition false
    var d = b - c; // [4] (line 6) hit breakpoint, condition true
    var e = c * d;
    var f = 10;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement,
  // set conditional breakpoint on lines 4, 5 and 6
  expectPaused(conn, "other", {{"global", 2, 1}});

  m::debugger::SetBreakpointByUrlRequest req0;
  req0.url = kDefaultUrl;
  req0.id = msgId++;
  req0.lineNumber = 4;
  req0.condition = std::optional<std::string>("throw Error('Boom!')");
  conn.send(req0.toJsonStr());

  expectBreakpointResponse(conn, req0.id, 4, 4);

  m::debugger::SetBreakpointByUrlRequest req1;
  req1.url = kDefaultUrl;
  req1.id = msgId++;
  req1.lineNumber = 5;
  req1.condition = std::optional<std::string>("b === a");
  conn.send(req1.toJsonStr());

  expectBreakpointResponse(conn, req1.id, 5, 5);

  m::debugger::SetBreakpointByUrlRequest req2;
  req2.url = kDefaultUrl;
  req2.id = msgId++;
  req2.lineNumber = 6;
  req2.condition = std::optional<std::string>("c === 5");
  conn.send(req2.toJsonStr());

  expectBreakpointResponse(conn, req2.id, 6, 6);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] (line 4) skip breakpoint, condition throws

  // [3] (line 5) skip breakpoint, condition false

  // [4] (line 6) hit breakpoint, condition true
  expectPaused(conn, "other", {{"global", 6, 1}});

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::DebuggerRemoveBreakpoint
TEST_F(ConnectionTests, testRemoveBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    // [1] (line 2) hit debugger statement, set breakpoint on line 7
    debugger;
    var a = 1;

    for (var i = 1; i <= 2; i++) {
      // [1] (line 7) hit breakpoint and then remove it
      a += i;
    }

    storeValue(a);
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement, set breakpoint on line 7
  expectPaused(conn, "other", {{"global", 2, 1}});

  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 7;
  conn.send(req.toJsonStr());

  auto breakpointId = expectBreakpointResponse(conn, req.id, 7, 7);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [1] (line 7) hit breakpoint and then remove it
  expectPaused(conn, "other", {{"global", 7, 1}});

  m::debugger::RemoveBreakpointRequest removeReq;
  removeReq.id = msgId++;
  removeReq.breakpointId = breakpointId;
  conn.send(removeReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, removeReq.id);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // check final value
  jsi::Value finalValue = asyncRuntime.awaitStoredValue();
  EXPECT_EQ(finalValue.asNumber(), 4);
}

// Also implemented as CDPAgentTest::DebuggerAsyncPauseWhileRunning
TEST_F(ConnectionTests, testAsyncPauseWhileRunning) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var accum = 10;

    while (!shouldStop()) {
      var a = 1;
      var b = 2;
      var c = a + b;

      accum += c;
    }                        // (line 9)

    var d = -accum;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // send some number of async pauses, make sure that we always stop before
  // the end of the loop on line 9
  for (int i = 0; i < 10; i++) {
    send<m::debugger::PauseRequest>(conn, msgId++);
    expectPaused(
        conn, "other", {FrameInfo("global", 0, 1).setLineNumberMax(9)});

    send<m::debugger::ResumeRequest>(conn, msgId++);
    expectNotification<m::debugger::ResumedNotification>(conn);
  }

  // break out of loop
  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::DebuggerEvalOnCallFrame
TEST_F(ConnectionTests, testEvalOnCallFrame) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var globalVar = "omega";
    var booleanVar = true;
    var numberVar = 42;
    var objectVar = {number: 1, bool: false, str: "string"};

    function func1(closure, f1param) { // frame 4
        var f1v1 = "alpha";
        var f1v2 = "beta";
        function func1b() {            // frame 3
            var f1bv1 = "gamma";
            function func1c() {        // frame 2
                var f1cv1 = 19;
                closure();
            }
            func1c();
        }
        func1b();
    }

    function func2() {                 // frame 1
        var f2v1 = "baker";
        var f2v2 = "charlie";
        function func2b() {            // frame 0
            var f2bv1 = "dog";
            debugger;                  // [1] (line 25) hit debugger statement
                                       // [2]           run evals
                                       // [3]           resume
            print(globalVar);
            print(f2bv1);
        }
        func2b();
    }

    func1(func2, "tau");
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 25) hit debugger statement
  expectPaused(
      conn,
      "other",
      {{"func2b", 25, 3},
       {"func2", 31, 2},
       {"func1c", 13, 4},
       {"func1b", 15, 3},
       {"func1", 17, 2},
       {"global", 34, 1}});

  // [2] run eval statements
  int frame = 0;
  sendEvalRequest(conn, msgId + 0, frame, R"("0: " + globalVar)");
  sendEvalRequest(conn, msgId + 1, frame, R"("1: " + f2bv1)");
  sendEvalRequest(conn, msgId + 2, frame, R"("2: " + f2v2)");
  sendEvalRequest(conn, msgId + 3, frame, R"("3: " + f2bv1 + " && " + f2v2)");
  expectEvalResponse(conn, msgId + 0, "0: omega");
  expectEvalResponse(conn, msgId + 1, "1: dog");
  expectEvalResponse(conn, msgId + 2, "2: charlie");
  expectEvalResponse(conn, msgId + 3, "3: dog && charlie");
  msgId += 4;

  frame = 1;
  sendEvalRequest(conn, msgId + 0, frame, R"("4: " + f2v1)");
  sendEvalRequest(conn, msgId + 1, frame, R"("5: " + f2v2)");
  sendEvalRequest(conn, msgId + 2, frame, R"(globalVar = "mod by debugger")");
  expectEvalResponse(conn, msgId + 0, "4: baker");
  expectEvalResponse(conn, msgId + 1, "5: charlie");
  expectEvalResponse(conn, msgId + 2, "mod by debugger");
  msgId += 3;

  frame = 2;
  sendEvalRequest(conn, msgId + 0, frame, R"("6: " + f1cv1 + f1bv1 + f1v1)");
  sendEvalRequest(conn, msgId + 1, frame, R"("7: " + globalVar)");
  expectEvalResponse(conn, msgId + 0, "6: 19gammaalpha");
  expectEvalResponse(conn, msgId + 1, "7: mod by debugger");
  msgId += 2;

  // [2.1] run eval statements that return non-string primitive values
  frame = 0;
  sendEvalRequest(conn, msgId + 0, frame, "booleanVar");
  sendEvalRequest(conn, msgId + 1, frame, "numberVar");
  expectEvalResponse(conn, msgId + 0, true);
  expectEvalResponse(conn, msgId + 1, 42);
  msgId += 2;

  // [2.2] run eval statement that returns object
  frame = 0;
  sendEvalRequest(conn, msgId + 0, frame, "objectVar");
  expectEvalResponse(
      conn,
      msgId + 0,
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"str", PropInfo("string").setValue("\"string\"")},
       {"__proto__", PropInfo("object")}});

  // msgId is increased by 2 because expectEvalResponse will make additional
  // request with expectProps.
  msgId += 2;

  // [3] resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeEnableDisable
TEST_F(ConnectionTests, testRuntimeEnable) {
  int msgId = 1;

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);
}

// Also implemented as CDPAgentTest::RuntimeEvaluate
TEST_F(ConnectionTests, testRuntimeEvaluate) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var globalVar = "omega";
    var booleanVar = true;
    var numberVar = 42;
    var objectVar = {number: 1, bool: false, str: "string"};

    while(!shouldStop()) {  // [1] (line 6) hit infinite loop
      var a = 1;            // [2] run evals
      a++;                  // [3] exit run loop
    }
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 6) hit infinite loop

  // [2] run eval statements
  sendRuntimeEvalRequest(conn, msgId + 0, R"("0: " + globalVar)");
  expectEvalResponse(conn, msgId + 0, "0: omega");

  // [2.1] run eval statements that return non-string primitive values
  sendRuntimeEvalRequest(conn, msgId + 1, "booleanVar");
  sendRuntimeEvalRequest(conn, msgId + 2, "numberVar");
  expectEvalResponse(conn, msgId + 1, true);
  expectEvalResponse(conn, msgId + 2, 42);

  // [2.2] run eval statement that returns object
  sendRuntimeEvalRequest(conn, msgId + 3, "objectVar");
  expectEvalResponse(
      conn,
      msgId + 3,
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"str", PropInfo("string").setValue("\"string\"")},
       {"__proto__", PropInfo("object")}});

  // [3] exit run loop
  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::RuntimeEvaluateReturnByValue
TEST_F(ConnectionTests, testRuntimeEvaluateReturnByValue) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync("while(!shouldStop());");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // We expect this JSON object to be evaluated and return by value, so
  // that JSON encoding the result will give the same string.
  auto object = "{\"key\":[1,\"two\"]}";
  auto preview =
      "{\"description\":\"Object\",\"overflow\":false,\"properties\":[{\"name\":\"key\",\"subtype\":\"array\",\"type\":\"object\",\"value\":\"Array(2)\"}],\"type\":\"object\"}";

  m::runtime::EvaluateRequest req;
  req.id = msgId;
  req.expression = std::string("(") + object + ")";
  req.returnByValue = true;
  req.generatePreview = true;
  conn.send(req.toJsonStr());

  auto resp =
      expectResponse<m::debugger::EvaluateOnCallFrameResponse>(conn, msgId);
  EXPECT_EQ(resp.result.type, "object");
  ASSERT_TRUE(resp.result.value.has_value());
  ASSERT_TRUE(resp.result.preview.has_value());
  EXPECT_EQ(resp.result.preview->type, "object");
  ASSERT_TRUE(jsonValsEQ(
      (resp.result.preview->toJsonVal(factory)),
      mustParseStrAsJsonObj(preview, factory)));
  ASSERT_TRUE(jsonValsEQ(
      mustParseStrAsJsonObj(resp.result.value.value(), factory),
      mustParseStrAsJsonObj(object, factory)));

  // [3] exit run loop
  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::RuntimeCompileScript
TEST_F(ConnectionTests, testRuntimeCompileScript) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync("while(!shouldStop());");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  m::runtime::CompileScriptRequest req;
  req.id = msgId;
  req.persistScript = true;
  req.sourceURL = "none";
  req.expression = "1+1";
  conn.send(req.toJsonStr());

  auto resp = expectResponse<m::runtime::CompileScriptResponse>(conn, 3);
  EXPECT_EQ(resp.scriptId, "userScript0");

  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::RuntimeCompileScriptParseError
TEST_F(ConnectionTests, testRuntimeCompileScriptParseError) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync("while(!shouldStop());");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  m::runtime::CompileScriptRequest req;
  req.id = msgId;
  req.persistScript = true;
  req.sourceURL = "none";
  req.expression = "/oops";
  conn.send(req.toJsonStr());

  auto resp = expectResponse<m::runtime::CompileScriptResponse>(conn, 3);
  EXPECT_TRUE(resp.exceptionDetails.has_value());

  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::DebuggerEvalOnCallFrameException
TEST_F(ConnectionTests, testEvalOnCallFrameException) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var count = 0;

    function eventuallyThrows(x) {
      if (x <= 0)
        throw new Error("I frew up.");
      count++;
      eventuallyThrows(x-1);
    }

    function callme() {
      print("Hello");
      debugger;          // [1] (line 12) hit debugger statement
                         // [2]           run evals
                         // [3]           resume
      print("Goodbye");
    }

    callme();
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 12) hit debugger statement
  expectPaused(conn, "other", {{"callme", 12, 2}, {"global", 18, 1}});

  // [2] run evals
  int frame = 0;
  sendEvalRequest(conn, msgId + 0, frame, "this is not valid javascript");
  sendEvalRequest(conn, msgId + 1, frame, "eventuallyThrows(5)");
  sendEvalRequest(conn, msgId + 2, frame, "count");

  expectEvalException(conn, msgId + 0, "SyntaxError: 1:6:';' expected", {});
  expectEvalException(
      conn,
      msgId + 1,
      "Error: I frew up.",
      {{"eventuallyThrows", 5, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},
       {"eventuallyThrows", 7, 0},

       // TODO: unsure why these frames are here, but they're in hdb tests
       // too. Ask Hermes about if they really should be there.
       FrameInfo("eval", 0, 0).setLineNumberMax(19),
       FrameInfo("callme", 12, 2),
       FrameInfo("global", 0, 0).setLineNumberMax(19)});
  expectEvalResponse(conn, msgId + 2, 5);
  msgId += 3;

  // [3] resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testLoadMultipleScripts) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(
      R"(
    function foo(x) {
      debugger;
      print(x);
    }

    var a = 1 + 1;

    //# sourceMappingURL=/foo/bar/url1.js.map
  )",
      "url1");

  send<m::debugger::EnableRequest>(conn, msgId++);

  m::debugger::ScriptParsedNotification script1 =
      expectScriptParsed(conn, "url1", "/foo/bar/url1.js.map");

  asyncRuntime.executeScriptAsync(
      R"(
    var b = a + 2;
    var c = b - 1;
    foo(c);

    //# sourceMappingURL=/foo/bar/url2.js.map
  )",
      "url2");

  m::debugger::ScriptParsedNotification script2 =
      expectScriptParsed(conn, "url2", "/foo/bar/url2.js.map");

  // [1] (line 2) hit debugger statement, resume
  expectPaused(
      conn,
      "other",
      {FrameInfo("foo", 2, 2).setScriptId(script1.scriptId),
       FrameInfo("global", 3, 1).setScriptId(script2.scriptId)});

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeGetProperties
TEST_F(ConnectionTests, testGetProperties) {
  int msgId = 1;
  std::vector<std::string> objIds;

  asyncRuntime.executeScriptAsync(R"(
    function foo() {
      var num = 123;
      var obj = {
        "depth": 0,
        "value": {
          "a": -1/0,
          "b": 1/0,
          "c": Math.sqrt(-2),
          "d": -0,
          "e": "e_string"
        }
      };
      var arr = [1, 2, 3];
      function bar() {
        var num = 456;
        var obj = {"depth": 1, "value": {"c": 5, "d": "d_string"}};
        debugger;
      };
      bar();
      debugger;
    }

    foo();
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  auto pausedNote = expectPaused(
      conn, "other", {{"bar", 17, 3}, {"foo", 19, 2}, {"global", 23, 1}});

  auto &scopeObj = pausedNote.callFrames.at(1).scopeChain.at(0).object;
  EXPECT_TRUE(scopeObj.objectId.has_value());
  std::string scopeObjId = scopeObj.objectId.value();
  objIds.push_back(scopeObjId);

  auto scopeChildren = expectProps(
      conn,
      msgId++,
      scopeObjId,
      {{"this", PropInfo("undefined")},
       {"num", PropInfo("number").setValue("123")},
       {"obj", PropInfo("object")},
       {"arr", PropInfo("object").setSubtype("array")},
       {"bar", PropInfo("function")}});
  EXPECT_EQ(scopeChildren.size(), 3);

  EXPECT_EQ(scopeChildren.count("obj"), 1);
  std::string objId = scopeChildren.at("obj");
  objIds.push_back(objId);

  auto objChildren = expectProps(
      conn,
      msgId++,
      objId,
      {{"depth", PropInfo("number").setValue("0")},
       {"value", PropInfo("object")},
       {"__proto__", PropInfo("object")}});
  EXPECT_EQ(objChildren.size(), 2);

  EXPECT_EQ(objChildren.count("value"), 1);
  std::string valueId = objChildren.at("value");
  objIds.push_back(valueId);

  auto valueChildren = expectProps(
      conn,
      msgId++,
      valueId,
      {{"a", PropInfo("number").setUnserializableValue("-Infinity")},
       {"b", PropInfo("number").setUnserializableValue("Infinity")},
       {"c", PropInfo("number").setUnserializableValue("NaN")},
       {"d", PropInfo("number").setUnserializableValue("-0")},
       {"e", PropInfo("string").setValue("\"e_string\"")},
       {"__proto__", PropInfo("object")}});
  EXPECT_EQ(valueChildren.size(), 1);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  expectPaused(conn, "other", {{"foo", 20, 2}, {"global", 23, 1}});

  // all old object ids should be invalid after resuming
  for (std::string oldObjId : objIds) {
    expectProps(
        conn, msgId++, oldObjId, std::unordered_map<std::string, PropInfo>{});
  }

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeGetPropertiesOnlyOwn
TEST_F(ConnectionTests, testGetPropertiesOnlyOwnProperties) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    function foo() {
      var protoObject = {
        "protoNum": 77
      };
      var obj = Object.create(protoObject);
      obj.num = 42;
      debugger;
    }
    foo();
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // wait for a pause on debugger statement and get object ID from the local
  // scope.
  auto pausedNote =
      expectPaused(conn, "other", {{"foo", 7, 2}, {"global", 9, 1}});
  const auto &scopeObject = pausedNote.callFrames.at(0).scopeChain.at(0).object;
  auto scopeChildren = expectProps(
      conn,
      msgId++,
      scopeObject.objectId.value(),
      {{"this", PropInfo("undefined")},
       {"obj", PropInfo("object")},
       {"protoObject", PropInfo("object")}});
  EXPECT_EQ(scopeChildren.count("obj"), 1);
  std::string objId = scopeChildren.at("obj");

  // Check that GetProperties request for obj object only have own properties
  // when onlyOwnProperties = true.
  expectProps(
      conn,
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")},
       {"__proto__", PropInfo("object")}},
      true);

  // Check that GetProperties request for obj object only have all properties
  // when onlyOwnProperties = false.
  // __proto__ is not returned here because all properties from proto chain
  // are already included in the result.
  expectProps(
      conn,
      msgId++,
      objId,
      {{"num", PropInfo("number").setValue("42")},
       {"protoNum", PropInfo("number").setValue("77")}},
      false);

  // resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testDisable) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    while (!shouldStop()) {
      var a = 1;
      var b = 2;
      var c = a + b; // [1] (line 4) disable to remove breakpoints and resume
                     // [2] (line 4) the breakpoint should not hit anymore
      var d = 10;
    }
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // set breakpoint on line 4: "var c = ..."
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 4;
  conn.send(req.toJsonStr());

  expectBreakpointResponse(conn, req.id, 4, 4);

  // [1] (line 4) disable to remove breakpoints and resume
  expectPaused(conn, "other", {{"global", 4, 1}});

  send<m::debugger::DisableRequest>(conn, msgId++);

  // [2] (line 4) the breakpoint should not hit anymore
  expectNothing(conn);
  asyncRuntime.stop();
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testDisableWhileRunning) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] initial pause to set the breakpoint on line 6
    while (!shouldStop()) { // [2] loop running until we receive a detach request
      var a = 1;
    }
    while (shouldStop()) {
      var c = a + 1; // [3] (line 6) the breakpoint should not hit after detach
    }
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] initial pause to set the breakpoint on line 6
  expectPaused(conn, "other", {{"global", 1, 1}});

  // set breakpoint on line 6: "var c = ..."
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 6;
  conn.send(req.toJsonStr());

  expectBreakpointResponse(conn, req.id, 6, 6);

  // [2] loop running until we receive a detach request
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  send<m::debugger::DisableRequest>(conn, msgId++);
  asyncRuntime.stop();

  // [3] (line 6) the breakpoint should not hit after detach
  expectNothing(conn);
  asyncRuntime.start();
}

// Also implemented as CDPAgentTest::DebuggerSetPauseOnExceptionsAll
TEST_F(ConnectionTests, testSetPauseOnExceptionsAll) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] (line 1) initial pause, set throw on exceptions to 'All'

    try {
      var a = 123;
      throw new Error('Caught error'); // [2] line 5, pause on exception
    } catch (err) {
      // Do nothing.
    }

    throw new Error('Uncaught exception'); // [3] line 10, pause on exception
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 1) initial pause, set throw on exceptions to 'All'
  expectPaused(conn, "other", {{"global", 1, 1}});
  m::debugger::SetPauseOnExceptionsRequest allExceptionsReq;
  allExceptionsReq.id = msgId++;
  allExceptionsReq.state = "all";
  conn.send(allExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, allExceptionsReq.id);

  // Resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [2] line 5, pause on exception
  expectPaused(conn, "exception", {{"global", 5, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] line 10, pause on exception
  expectPaused(conn, "exception", {{"global", 10, 1}});

  // Send resume event and check that Hermes has thrown an exception.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  asyncRuntime.wait();
  EXPECT_EQ(asyncRuntime.getNumberOfExceptions(), 1);
  EXPECT_EQ(asyncRuntime.getLastThrownExceptionMessage(), "Uncaught exception");
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetPauseOnExceptionsNone) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] (line 1) initial pause, set throw on exceptions to 'None'

    try {
      var a = 123;
      throw new Error('Caught error');
    } catch (err) {
      // Do nothing.
    }

    throw new Error('Uncaught exception');
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 1) initial pause, set throw on exceptions to 'None'
  expectPaused(conn, "other", {{"global", 1, 1}});
  m::debugger::SetPauseOnExceptionsRequest allExceptionsReq;
  allExceptionsReq.id = msgId++;
  allExceptionsReq.state = "none";
  conn.send(allExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, allExceptionsReq.id);

  // Resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Check that Hermes has thrown an exception (without reporting it).
  expectNothing(conn);
  asyncRuntime.wait();
  EXPECT_EQ(asyncRuntime.getNumberOfExceptions(), 1);
  EXPECT_EQ(asyncRuntime.getLastThrownExceptionMessage(), "Uncaught exception");
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetPauseOnExceptionsUncaught) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] (line 1) initial pause, set throw on exceptions to 'Uncaught'

    try {
      var a = 123;
      throw new Error('Caught error');
    } catch (err) {
      // Do nothing.
    }

    throw new Error('Uncaught exception'); // [3] line 10, pause on exception
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 1) initial pause, set throw on exceptions to 'Uncaught'
  expectPaused(conn, "other", {{"global", 1, 1}});
  m::debugger::SetPauseOnExceptionsRequest allExceptionsReq;
  allExceptionsReq.id = msgId++;
  allExceptionsReq.state = "uncaught";
  conn.send(allExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, allExceptionsReq.id);

  // Resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // [3] line 10, pause on exception
  expectPaused(conn, "exception", {{"global", 10, 1}});

  // Send resume event and check that Hermes has thrown an exception.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  asyncRuntime.wait();
  EXPECT_EQ(asyncRuntime.getNumberOfExceptions(), 1);
  EXPECT_EQ(asyncRuntime.getLastThrownExceptionMessage(), "Uncaught exception");
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, invalidPauseModeGivesError) {
  m::debugger::SetPauseOnExceptionsRequest req;
  req.id = 1;
  req.state = "badgers";
  conn.send(req.toJsonStr());
  expectResponse<m::ErrorResponse>(conn, req.id);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testShouldPauseOnThrow) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] (line 1) initial pause, check shouldPauseOnThrow is false
              // [2] set throw to 'All', check shouldPauseOnThrow is true
              // [3] set throw to 'None', check shouldPauseOnThrow is false
              // [4] set throw to 'Uncaught', check shouldPauseOnThrow is true
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  auto shouldPauseOnThrowEvalMsg =
      R"("-> " + DebuggerInternal.shouldPauseOnThrow)";
  auto responseTrue = "-> true";
  auto responseFalse = "-> false";

  // [1] (line 1) initial pause, check shouldPauseOnThrow is false
  expectPaused(conn, "other", {{"global", 1, 1}});
  sendEvalRequest(conn, msgId + 1, 0, shouldPauseOnThrowEvalMsg);
  expectEvalResponse(conn, msgId + 1, responseFalse);

  // [2] set throw to 'All', check shouldPauseOnThrow is true
  m::debugger::SetPauseOnExceptionsRequest allExceptionsReq;
  allExceptionsReq.id = msgId++;
  allExceptionsReq.state = "all";
  conn.send(allExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, allExceptionsReq.id);

  sendEvalRequest(conn, msgId + 1, 0, shouldPauseOnThrowEvalMsg);
  expectEvalResponse(conn, msgId + 1, responseTrue);

  // [3] set throw to 'None', check shouldPauseOnThrow is false
  m::debugger::SetPauseOnExceptionsRequest noExceptionsReq;
  noExceptionsReq.id = msgId++;
  noExceptionsReq.state = "none";
  conn.send(noExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, noExceptionsReq.id);

  sendEvalRequest(conn, msgId + 1, 0, shouldPauseOnThrowEvalMsg);
  expectEvalResponse(conn, msgId + 1, responseFalse);

  // [4] set throw to 'Uncaught', check shouldPauseOnThrow is true
  m::debugger::SetPauseOnExceptionsRequest uncaughtExceptionsReq;
  uncaughtExceptionsReq.id = msgId++;
  uncaughtExceptionsReq.state = "uncaught";
  conn.send(uncaughtExceptionsReq.toJsonStr());
  expectResponse<m::OkResponse>(conn, uncaughtExceptionsReq.id);

  sendEvalRequest(conn, msgId + 1, 0, shouldPauseOnThrowEvalMsg);
  expectEvalResponse(conn, msgId + 1, responseTrue);

  // Resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testScopeVariables) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var globalString = "global-string";
    var globalObject = {number: 1, bool: false};

    function func() {
      var localString = "local-string";
      var localObject = {number: 2, bool: true};
      debugger; // [1] (line 7) hit debugger statement
                // two local vars - localString and localObject
                // two global vars - globalString and globalObject
    }

    func(); // line 12
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 7) hit debugger statement
  auto pausedNote =
      expectPaused(conn, "other", {{"func", 7, 2}, {"global", 12, 1}});
  const auto &scopeChain = pausedNote.callFrames.at(0).scopeChain;
  EXPECT_EQ(scopeChain.size(), 2);

  // [2] inspect local scope
  EXPECT_EQ(scopeChain.at(0).type, "local");
  const auto &localScopeObject = scopeChain.at(0).object;
  auto localScopeObjectChildren = expectProps(
      conn,
      msgId++,
      localScopeObject.objectId.value(),
      {{"this", PropInfo("undefined")},
       {"localString", PropInfo("string").setValue("\"local-string\"")},
       {"localObject", PropInfo("object")}});
  auto localObjectId = localScopeObjectChildren.at("localObject");
  expectProps(
      conn,
      msgId++,
      localObjectId,
      {{"number", PropInfo("number").setValue("2")},
       {"bool", PropInfo("boolean").setValue("true")},
       {"__proto__", PropInfo("object")}});

  // [3] inspect global scope
  // Global scope can contain more properties than we have defined
  // in our test code and we can't use expectProps() method here.
  // As a workaround we create a Map of properties and check that
  // those global properties that we have defined are in the map.
  EXPECT_EQ(scopeChain.at(1).type, "global");
  const auto &globalScopeObject = scopeChain.at(1).object;
  m::runtime::GetPropertiesRequest req;
  req.id = msgId++;
  req.objectId = globalScopeObject.objectId.value();
  conn.send(req.toJsonStr());
  auto resp = expectResponse<m::runtime::GetPropertiesResponse>(conn, req.id);
  std::unordered_map<std::string, std::optional<m::runtime::RemoteObject>>
      globalProperties;
  for (auto &propertyDescriptor : resp.result) {
    globalProperties[propertyDescriptor.name] =
        std::move(propertyDescriptor.value);
  }
  EXPECT_GE(globalProperties.size(), 3);

  // globalString should be of type "string" and have value "global-string".
  EXPECT_EQ(globalProperties.count("globalString"), 1);
  EXPECT_TRUE(globalProperties["globalString"].has_value());
  EXPECT_EQ(globalProperties["globalString"].value().type, "string");
  EXPECT_EQ(
      globalProperties["globalString"].value().value.value(),
      "\"global-string\"");

  // func should be of type "function".
  EXPECT_EQ(globalProperties.count("func"), 1);
  EXPECT_TRUE(globalProperties["func"].has_value());
  EXPECT_EQ(globalProperties["func"].value().type, "function");

  // globalObject should be of type "object" with "number" and "bool"
  // properties.
  EXPECT_EQ(globalProperties.count("globalObject"), 1);
  EXPECT_TRUE(globalProperties["globalObject"].has_value());
  EXPECT_EQ(globalProperties["globalObject"].value().type, "object");
  expectProps(
      conn,
      msgId++,
      globalProperties["globalObject"].value().objectId.value(),
      {{"number", PropInfo("number").setValue("1")},
       {"bool", PropInfo("boolean").setValue("false")},
       {"__proto__", PropInfo("object")}});

  // [4] resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeCallFunctionOnObject
TEST_F(ConnectionTests, testRuntimeCallFunctionOnObject) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
      debugger;
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"global", 1, 1}});

  // create a new Object() that will be used as "this" below.
  m::runtime::RemoteObjectId thisId;
  {
    sendRuntimeEvalRequest(conn, msgId, "new Object()");
    auto resp = expectResponse<m::runtime::EvaluateResponse>(conn, msgId++);
    ASSERT_TRUE(resp.result.objectId) << resp.toJsonVal(factory);
    thisId = *resp.result.objectId;
  }

  // expectedPropInfos are properties that are expected to exist in thisId. It
  // is modified by addMember (below).
  std::unordered_map<std::string, PropInfo> expectedPropInfos;

  // Add __proto__ as it always exists.
  expectedPropInfos.emplace("__proto__", PropInfo("object"));

  /// addMember sends Runtime.callFunctionOn() requests with a function
  /// declaration that simply adds a new property called \p propName with type
  /// \p type to the remote object \p id. \p ca is the property's value.
  /// The new property must not exist in \p id unless \p allowRedefinition is
  /// true.
  auto addMember = [&](const m::runtime::RemoteObjectId id,
                       const char *type,
                       const char *propName,
                       m::runtime::CallArgument ca,
                       bool allowRedefinition = false) {
    auto it = expectedPropInfos.emplace(propName, PropInfo(type));

    EXPECT_TRUE(allowRedefinition || it.second)
        << "property \"" << propName << "\" redefined.";

    if (ca.value) {
      it.first->second.setValue(*ca.value);
    }

    if (ca.unserializableValue) {
      it.first->second.setUnserializableValue(*ca.unserializableValue);
    }

    m::runtime::CallFunctionOnRequest req;
    req.id = msgId++;
    req.functionDeclaration =
        std::string("function(e){const r=\"") + propName + "\"; this[r]=e,r}";
    req.arguments = std::vector<m::runtime::CallArgument>{};
    req.arguments->push_back(std::move(ca));
    req.objectId = thisId;
    conn.send(req.toJsonStr());
    expectResponse<m::runtime::CallFunctionOnResponse>(conn, req.id);
  };

  addMember(thisId, "boolean", "b", makeValueCallArgument("true"));
  addMember(thisId, "number", "num", makeValueCallArgument("12"));
  addMember(thisId, "string", "str", makeValueCallArgument("\"string value\""));
  addMember(thisId, "object", "self_ref", makeObjectIdCallArgument(thisId));
  addMember(
      thisId, "number", "inf", makeUnserializableCallArgument("Infinity"));
  addMember(
      thisId, "number", "ni", makeUnserializableCallArgument("-Infinity"));
  addMember(thisId, "number", "nan", makeUnserializableCallArgument("NaN"));

  /// ensures that \p objId has all of the expected properties; Returns the
  /// runtime::RemoteObjectId for the "self_ref" property (which must exist).
  auto verifyObjShape = [&](const m::runtime::RemoteObjectId &objId)
      -> std::optional<std::string> {
    auto objProps = expectProps(conn, msgId++, objId, expectedPropInfos);
    EXPECT_TRUE(objProps.count("__proto__"));
    auto objPropIt = objProps.find("self_ref");
    if (objPropIt == objProps.end()) {
      EXPECT_TRUE(false) << "missing \"self_ref\" property.";
      return {};
    }
    return objPropIt->second;
  };

  // Verify that thisId has the correct shape.
  auto selfRefId = verifyObjShape(thisId);
  ASSERT_TRUE(selfRefId);
  // Then verify that the self reference has the correct shape. If thisId does
  // not have the "self_ref" property the call to verifyObjShape will return an
  // empty Optional, as well as report an error.
  selfRefId = verifyObjShape(*selfRefId);
  ASSERT_TRUE(selfRefId);

  // Now we modify the self reference, which should cause thisId to change
  // as well.
  const bool kAllowRedefinition = true;

  addMember(
      *selfRefId,
      "number",
      "num",
      makeValueCallArgument("42"),
      kAllowRedefinition);

  addMember(
      *selfRefId, "number", "neg_zero", makeUnserializableCallArgument("-0"));

  verifyObjShape(thisId);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeCallFunctionOnExecutionContext
TEST_F(ConnectionTests, testRuntimeCallFunctionOnExecutionContext) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
      debugger;
  )");

  /// helper that returns a map with all of \p objId 's members.
  auto getProps = [this, &msgId](const m::runtime::RemoteObjectId &objId) {
    m::runtime::GetPropertiesRequest req;
    req.id = msgId++;
    req.objectId = objId;
    conn.send(req.toJsonStr());
    auto resp = expectResponse<m::runtime::GetPropertiesResponse>(conn, req.id);
    std::unordered_map<std::string, std::optional<m::runtime::RemoteObject>>
        properties;
    for (auto &propertyDescriptor : resp.result) {
      properties[propertyDescriptor.name] = std::move(propertyDescriptor.value);
    }
    return properties;
  };

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"global", 1, 1}});

  // globalThisId is the inspector's object Id for globalThis.
  m::runtime::RemoteObjectId globalThisId;
  {
    sendRuntimeEvalRequest(conn, msgId, "globalThis");
    auto resp = expectResponse<m::runtime::EvaluateResponse>(conn, msgId++);
    ASSERT_TRUE(resp.result.objectId) << resp.toJsonVal(factory);
    globalThisId = *resp.result.objectId;
  }

  // This test table has all of the new fields we want to add to globalThis,
  // plus the Runtime.CallArgument to be sent to the inspector.
  struct {
    const char *propName;
    m::runtime::CallArgument callArg;
  } tests[] = {
      {"callFunctionOnTestMember1", makeValueCallArgument("10")},
      {"callFunctionOnTestMember2", makeValueCallArgument("\"string\"")},
      {"callFunctionOnTestMember3", makeUnserializableCallArgument("NaN")},
      {"callFunctionOnTestMember4", makeUnserializableCallArgument("-0")},
  };

  // sanity-check that our test fields don't exist in global this.
  {
    auto currProps = getProps(globalThisId);
    for (const auto &test : tests) {
      EXPECT_EQ(currProps.count(test.propName), 0) << test.propName;
    }
  }

  auto addMember = [this, &msgId](
                       const char *propName, m::runtime::CallArgument &ca) {
    m::runtime::CallFunctionOnRequest req;
    req.id = msgId++;
    req.functionDeclaration =
        std::string("function(e){const r=\"") + propName + "\"; this[r]=e,r}";
    // Don't have an easy way to copy these, so...
    req.arguments = std::vector<m::runtime::CallArgument>{};
    req.arguments->push_back(std::move(ca));
    req.executionContextId = 1;
    conn.send(req.toJsonStr());
    expectResponse<m::runtime::CallFunctionOnResponse>(conn, req.id);
    // n.b. we're only borrowing the CallArgument, so give it back...
    ca = std::move(req.arguments->at(0));
  };

  for (auto &test : tests) {
    addMember(test.propName, test.callArg);
  }

  {
    auto currProps = getProps(globalThisId);
    for (const auto &test : tests) {
      auto it = currProps.find(test.propName);

      // there should be a property named test.propName in globalThis.
      ASSERT_TRUE(it != currProps.end()) << test.propName;

      // and it should have a value.
      ASSERT_TRUE(it->second) << test.propName;

      if (it->second->value.has_value()) {
        // the property has a value, so make sure that's what's being expected.
        auto actual = it->second->value;
        auto expected = test.callArg.value;
        ASSERT_TRUE(expected.has_value()) << test.propName;
        ASSERT_TRUE(jsonValsEQ(
            mustParseStr(*actual, factory), mustParseStr(*expected, factory)))
            << test.propName;
      } else if (it->second->unserializableValue.has_value()) {
        // the property has an unserializable value, so make sure that's what's
        // being expected.
        auto actual = it->second->unserializableValue;
        auto expected = test.callArg.unserializableValue;
        ASSERT_TRUE(expected.has_value()) << test.propName;
        EXPECT_EQ(*actual, *expected) << test.propName;
      } else {
        FAIL() << "No value or unserializable value in " << test.propName;
      }
    }
  }

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeConsoleLog
TEST_F(ConnectionTests, testConsoleLog) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger; [1]
    var object1 = {number1: 1, bool1: false};
    var object2 = {number2: 2, bool2: true};
    console.warn('string value', object1, object2, '{"number3": 3}');

    debugger; // Hit debugger statement so that we receive console
              // api notification before VM gets destroyed.
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  // [1] (line 1) hit debugger statement
  expectPaused(conn, "other", {{"global", 1, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  auto warningNote =
      expectNotification<m::runtime::ConsoleAPICalledNotification>(conn);
  EXPECT_EQ(warningNote.type, "warning");
  EXPECT_EQ(warningNote.args.size(), 4);

  EXPECT_EQ(warningNote.args[0].type, "string");
  EXPECT_EQ(*warningNote.args[0].value, "\"string value\"");

  EXPECT_EQ(warningNote.args[1].type, "object");

  EXPECT_EQ(warningNote.args[2].type, "object");

  EXPECT_EQ(warningNote.args[3].type, "string");
  // This assertion is equivalent to CDPAgentTest::RuntimeConsoleLogJSON
  EXPECT_EQ(*warningNote.args[3].value, R"("{\"number3\": 3}")");

  expectPaused(conn, "other", {{"global", 6, 1}});

  // Requesting object properties sends requests and expects response messages
  // with the result, so this must be done after the paused message above has
  // already been removed from the queue of incoming messages.
  expectProps(
      conn,
      msgId++,
      warningNote.args[1].objectId.value(),
      {{"number1", PropInfo("number").setValue("1")},
       {"bool1", PropInfo("boolean").setValue("false")},
       {"__proto__", PropInfo("object")}});
  expectProps(
      conn,
      msgId++,
      warningNote.args[2].objectId.value(),
      {{"number2", PropInfo("number").setValue("2")},
       {"bool2", PropInfo("boolean").setValue("true")},
       {"__proto__", PropInfo("object")}});

  // Resume and expect no further notifications
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectNothing(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testConsoleGroup) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    debugger;

    var object1 = {number1: 1, bool1: false};
    var object2 = {number2: 2, bool2: true};

    console.group('grouping', object1, object2);
    console.groupEnd();

    debugger; // Hit debugger statement so that we receive console
              // api notification before VM gets destroyed.
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"global", 1, 1}});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  constexpr long long kNewYears2023 = 1672549200000;
  constexpr long long kNewYears3023 = 33229458000000;

  auto groupNote =
      expectNotification<m::runtime::ConsoleAPICalledNotification>(conn);
  EXPECT_EQ(groupNote.type, "startGroup");

  EXPECT_GT(groupNote.timestamp, kNewYears2023);
  EXPECT_LT(groupNote.timestamp, kNewYears3023);
  EXPECT_EQ(groupNote.args.size(), 3);

  EXPECT_EQ(groupNote.args[0].type, "string");
  EXPECT_EQ(groupNote.args[0].value.value(), "\"grouping\"");

  EXPECT_EQ(groupNote.args[1].type, "object");

  EXPECT_EQ(groupNote.args[2].type, "object");

  auto endNote =
      expectNotification<m::runtime::ConsoleAPICalledNotification>(conn);
  EXPECT_EQ(endNote.type, "endGroup");

  EXPECT_GT(endNote.timestamp, kNewYears2023);
  EXPECT_LT(endNote.timestamp, kNewYears3023);
  EXPECT_EQ(endNote.args.size(), 0);

  expectPaused(conn, "other", {{"global", 9, 1}});

  // Requesting object properties sends requests and expects response messages
  // with the result, so this must be done after the groupEnd and paused
  // messages above have already been removed from the queue of incoming
  // messages.
  expectProps(
      conn,
      msgId++,
      groupNote.args[1].objectId.value(),
      {{"number1", PropInfo("number").setValue("1")},
       {"bool1", PropInfo("boolean").setValue("false")},
       {"__proto__", PropInfo("object")}});
  expectProps(
      conn,
      msgId++,
      groupNote.args[2].objectId.value(),
      {{"number2", PropInfo("number").setValue("2")},
       {"bool2", PropInfo("boolean").setValue("true")},
       {"__proto__", PropInfo("object")}});

  // Resume and expect no further notifications
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectNothing(conn);
}

// Also implemented as CDPAgentTest::RuntimeConsoleBuffer
TEST_F(ConnectionTests, testConsoleBuffer) {
  int msgId = 1;

  const int kExpectedMaxBufferSize = 1000;
  const int kNumLogsToTest = kExpectedMaxBufferSize * 2;

  std::ostringstream oss;
  oss << "for (let i = 0; i < " << kNumLogsToTest << "; i++) {" << std::endl
      << "  console.log(`${i}`);" << std::endl
      << "}";

  asyncRuntime.executeScriptAsync(oss.str());
  asyncRuntime.wait(std::chrono::milliseconds(3500));

  bool receivedWarning = false;
  std::array<bool, kExpectedMaxBufferSize> received;

  // Test for repeated connection by sending Runtime.enable multiple times. It's
  // expected that the message cache is always kept around and provided to the
  // frontend each time.
  for (int numConnect = 0; numConnect < 2; numConnect++) {
    receivedWarning = false;
    received.fill(false);

    send<m::runtime::EnableRequest>(conn, msgId++);
    expectExecutionContextCreated(conn);

    // Loop for 1 iteration more than kExpectedMaxBufferSize because there is a
    // warning message given when buffer is exceeded
    for (size_t i = 0; i < kExpectedMaxBufferSize + 1; i++) {
      auto note =
          expectNotification<m::runtime::ConsoleAPICalledNotification>(conn);
      EXPECT_EQ(note.args[0].type, "string");

      try {
        // Verify that the latest kExpectedMaxBufferSize number of logs are
        // emitted
        int nthLog = std::stoi(
            note.args[0].value->substr(1, note.args[0].value->length() - 2));
        EXPECT_GT(nthLog, kExpectedMaxBufferSize - 1);
        EXPECT_LT(nthLog, kNumLogsToTest);
        EXPECT_EQ(note.type, "log");
        EXPECT_EQ(note.args.size(), 1);
        received[nthLog % kExpectedMaxBufferSize] = true;
      } catch (const std::exception &) {
        EXPECT_EQ(note.type, "warning");
        EXPECT_EQ(note.args.size(), 1);
        EXPECT_NE((*note.args[0].value).find("discarded"), std::string::npos);
        receivedWarning = true;
      }
    }

    expectNothing(conn);

    for (size_t i = 0; i < kExpectedMaxBufferSize; i++) {
      EXPECT_TRUE(received[i]);
    }
    EXPECT_TRUE(receivedWarning);
  }
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testThisObject) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var globalString = "global-string";

    var object = {
      someVar: "object var",
      foo: function() {
        var localString = "local-string";
        debugger; // [1] (line 7) hit debugger statement.
      }
    }

    object.foo(); // (line 11)
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 1) hit debugger statement
  auto pausedNote =
      expectPaused(conn, "other", {{"foo", 7, 2}, {"global", 11, 1}});

  // [2] inspect first call frame (foo)
  const auto &localThisObj = pausedNote.callFrames.at(0).thisObj;
  expectProps(
      conn,
      msgId++,
      localThisObj.objectId.value(),
      {{"someVar", PropInfo("string").setValue("\"object var\"")},
       {"foo", PropInfo("function")},
       {"__proto__", PropInfo("object")}});

  // [3] inspect second call frame (global)
  // Global scope can contain more properties than we have defined
  // in our test code and we can't use expectProps() method here.
  // As a workaround we create a Map of properties and check that
  // those global properties that we have defined are in the map.
  const auto &globalThisObj = pausedNote.callFrames.at(1).thisObj;
  m::runtime::GetPropertiesRequest req;
  req.id = msgId++;
  req.objectId = globalThisObj.objectId.value();
  conn.send(req.toJsonStr());
  auto resp = expectResponse<m::runtime::GetPropertiesResponse>(conn, req.id);
  std::unordered_map<std::string, std::optional<m::runtime::RemoteObject>>
      properties;
  for (auto &propertyDescriptor : resp.result) {
    properties[propertyDescriptor.name] = std::move(propertyDescriptor.value);
  }

  // globalString should be of type "string" and have value "global-string".
  EXPECT_EQ(properties.count("globalString"), 1);
  EXPECT_TRUE(properties["globalString"].has_value());
  EXPECT_EQ(properties["globalString"].value().type, "string");
  EXPECT_EQ(
      properties["globalString"].value().value.value(), "\"global-string\"");

  // [4] resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetBreakpointsMultipleScripts) {
  int msgId = 1;

  std::string url1 = "first-url";
  asyncRuntime.executeScriptAsync(
      R"(
    function foo1() {
      var somevar1 = 111; // (line 2) hit breakpoint
      var somevar2 = 222;
    }
  )",
      url1);
  send<m::debugger::EnableRequest>(conn, msgId++);
  auto scriptParsed1 =
      expectNotification<m::debugger::ScriptParsedNotification>(conn);

  std::string url2 = "second-url";
  asyncRuntime.executeScriptAsync(
      R"(
    function foo2() {
      var somevar3 = 333;
      var somevar4 = 444; // (line 3) hit breakpoint
    }
  )",
      url2);
  auto scriptParsed2 =
      expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // In the third script we will set breakpoint (on debugger statement)
  // and call both functions (script url doesn't matter).
  asyncRuntime.executeScriptAsync(R"(
    debugger; // [1] (line 1) set breakpoints in both files
    foo1();
    foo2();
  )");
  auto scriptParsed3 =
      expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] hit debugger statement
  expectPaused(conn, "other", {{"global", 1, 1}});

  // Set breakpoint on line 2 in the first script and line 3 in the second
  // script.
  m::debugger::SetBreakpointByUrlRequest req1;
  req1.url = kDefaultUrl;
  req1.id = msgId++;
  req1.lineNumber = 2;
  req1.url = url1;
  conn.send(req1.toJsonStr());

  expectBreakpointResponse(conn, req1.id, 2, 2);

  m::debugger::SetBreakpointByUrlRequest req2;
  req2.url = kDefaultUrl;
  req2.id = msgId++;
  req2.lineNumber = 3;
  req2.url = url2;
  conn.send(req2.toJsonStr());

  expectBreakpointResponse(conn, req2.id, 3, 3);

  // Resume and check that we hit correct breakpoints.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // First we should stop on line 2 of the first script.
  expectPaused(
      conn,
      "other",
      {FrameInfo("foo1", 2, 2).setScriptId(scriptParsed1.scriptId),
       FrameInfo("global", 2, 1).setScriptId(scriptParsed3.scriptId)});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // Next we should stop on line 3 of the second script.
  expectPaused(
      conn,
      "other",
      {FrameInfo("foo2", 3, 2).setScriptId(scriptParsed2.scriptId),
       FrameInfo("global", 3, 1).setScriptId(scriptParsed3.scriptId)});
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testSetBreakpointByUrlRegex) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
    var a = 1 + 2;
    debugger;      // [1] (line 2) hit debugger statement,
    var b = a / 2;
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // [1] (line 2) hit debugger statement
  expectPaused(conn, "other", {{"global", 2, 1}});

  // Unsuccessfully attempt a regex breakpoint
  m::debugger::SetBreakpointByUrlRequest req;
  req.urlRegex = ".*";
  req.id = msgId++;
  req.lineNumber = 3;
  req.columnNumber = 0;
  conn.send(req.toJsonStr());
  expectResponse<m::ErrorResponse>(conn, req.id);

  // Resume
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testColumnBreakpoint) {
  int msgId = 1;

  asyncRuntime.executeScriptAsync(
      R"(
function foo(){x=1}debugger;foo();
)");
  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Hit debugger statement.
  expectPaused(conn, "other", {{"global", 1, 1}});

  // Set breakpoint on position 1:16 (x=1).
  m::debugger::SetBreakpointByUrlRequest req;
  req.url = kDefaultUrl;
  req.id = msgId++;
  req.lineNumber = 1;
  req.columnNumber = 16;
  req.url = "url";
  conn.send(req.toJsonStr());

  expectBreakpointResponse(conn, req.id, 1, 1);

  // Resume and except to pause on a breakpoint.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectPaused(
      conn,
      "other",
      {FrameInfo("foo", 1, 2).setColumnNumber(16), FrameInfo("global", 1, 1)});

  // Resume execution
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, canBreakOnScriptsWithSourceMap) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  m::debugger::SetInstrumentationBreakpointRequest req;
  req.id = msgId++;
  req.instrumentation = "beforeScriptWithSourceMapExecution";

  conn.send(req.toJsonStr());
  auto bpId = expectResponse<m::debugger::SetInstrumentationBreakpointResponse>(
                  conn, req.id)
                  .breakpointId;

  asyncRuntime.executeScriptAsync(R"(
      storeValue(42); debugger;
      //# sourceURL=http://example.com/source.js
      //# sourceMappingURL=http://example.com/source.map
    )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // We should get a pause before the first statement
  auto note = expectNotification<m::debugger::PausedNotification>(conn);
  ASSERT_FALSE(asyncRuntime.hasStoredValue());
  EXPECT_EQ(note.reason, "other");
  ASSERT_TRUE(note.hitBreakpoints.has_value());
  ASSERT_EQ(note.hitBreakpoints->size(), 1);
  EXPECT_EQ(note.hitBreakpoints->at(0), bpId);

  // Continue and verify that the JS code has now executed
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectPaused(conn, "other", {{"global", 1, 1}});
  EXPECT_EQ(asyncRuntime.awaitStoredValue().asNumber(), 42);

  // Resume and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, wontStopOnFilesWithoutSourceMaps) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  m::debugger::SetInstrumentationBreakpointRequest req;
  req.id = msgId++;
  req.instrumentation = "beforeScriptWithSourceMapExecution";

  conn.send(req.toJsonStr());
  expectResponse<m::debugger::SetInstrumentationBreakpointResponse>(
      conn, req.id);

  // This script has no source map, so it should not trigger a break
  asyncRuntime.executeScriptAsync(R"(
      storeValue(42); debugger;
      //# sourceURL=http://example.com/source.js
    )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Continue and verify that the JS code has now executed without first
  // pausing on the script load.
  expectPaused(conn, "other", {{"global", 1, 1}});
  EXPECT_EQ(asyncRuntime.awaitStoredValue().asNumber(), 42);

  // Resume and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

struct WaitForDebuggerTests : public ConnectionTests {
  WaitForDebuggerTests() : ConnectionTests(true) {}
};

// TODO: Missing test in CDPAgentTest
TEST_F(WaitForDebuggerTests, runIfWaitingForDebugger) {
  int msgId = 0;

  asyncRuntime.executeScriptAsync(R"(
     storeValue(1);  debugger;
    )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, ++msgId);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"global", 1, 1}});

  // We should now be paused on load. Verify that we didn't run code.
  ASSERT_FALSE(asyncRuntime.hasStoredValue());

  // RunIfWaitingForDebugger should cause us to resume
  send<m::runtime::RunIfWaitingForDebuggerRequest>(conn, ++msgId);
  expectNotification<m::debugger::ResumedNotification>(conn);

  // We should immediately hit the 'debugger;' statement
  expectPaused(conn, "other", {{"global", 1, 1}});
  EXPECT_EQ(1, asyncRuntime.awaitStoredValue().asNumber());

  // RunIfWaitingForDebuggerResponse should be accepted but have no effect
  send<m::runtime::RunIfWaitingForDebuggerRequest>(conn, ++msgId);

  // Do a dummy call so we can expect something other than a ResumeRequest
  sendRuntimeEvalRequest(conn, ++msgId, "true");
  expectEvalResponse(conn, msgId, true);

  // Finally explicitly continue and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::HeapProfilerSampling
TEST_F(ConnectionTests, heapProfilerSampling) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(
      debugger;
      function allocator() {
        // Do some allocation.
        return new Object;
      }
      (function main() {
        var a = [];
        for (var i = 0; i < 100; i++) {
          a[i] = allocator();
        }
      })();
      debugger;
    )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // We should get a pause before the first statement.
  expectPaused(conn, "other", {{"global", 1, 1}});

  {
    m::heapProfiler::StartSamplingRequest req;
    req.id = msgId++;
    // Sample every 256 bytes to ensure there are some samples. The default is
    // 32768, which is too high for a small example. Note that sampling is a
    // random process, so there's no guarantee there will be any samples in
    // any finite number of allocations. In practice the likelihood is so high
    // that there shouldn't be any issues.
    req.samplingInterval = 256;
    send(conn, req);
  }
  // Resume, run the allocations, and once it's paused again, stop them.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectPaused(conn, "other", {{"global", 12, 1}});
  // Send the stop sampling request, expect the value coming back to be JSON.
  auto resp = send<
      m::heapProfiler::StopSamplingRequest,
      m::heapProfiler::StopSamplingResponse>(conn, msgId++);
  // Make sure there were some samples.
  EXPECT_NE(resp.profile.samples.size(), 0);
  // Don't test the content of the JSON, that is tested via the
  // SamplingHeapProfilerTest.

  // Resume and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::RuntimeGetHeapUsage
TEST_F(ConnectionTests, getHeapUsage) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(
    (function main() {
      var a = [];
      for (var i = 0; i < 100; i++) {
        a[i] = new Object;
      }
      debugger;
      print(a); // Keep allocations alive until after the debugger statement.
    })();
  )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"main", 6, 2}, {"global", 8, 1}});

  m::runtime::GetHeapUsageResponse resp =
      send<m::runtime::GetHeapUsageRequest, m::runtime::GetHeapUsageResponse>(
          conn, msgId++);
  EXPECT_GT(resp.usedSize, 0);

  // Resume and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::HeapProfilerCollectGarbage
TEST_F(ConnectionTests, collectGarbage) {
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(
    (function main() {
      var a = [];
      for (var i = 0; i < 100; i++) {
        a[i] = new Object;
      }
      debugger;
      print(a); // Keep allocations alive until after the debugger statement.
    })();
    debugger;
  )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"main", 6, 2}, {"global", 8, 1}});

  double before =
      send<m::runtime::GetHeapUsageRequest, m::runtime::GetHeapUsageResponse>(
          conn, msgId++)
          .usedSize;

  send<m::heapProfiler::CollectGarbageRequest>(conn, msgId++);

  // Move to the next debugger statement
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
  expectPaused(conn, "other", {{"global", 9, 1}});

  double after =
      send<m::runtime::GetHeapUsageRequest, m::runtime::GetHeapUsageResponse>(
          conn, msgId++)
          .usedSize;

  EXPECT_LT(after, before);

  // Resume and exit
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::HeapProfilerSnapshotRemoteObject
TEST_F(ConnectionTests, heapSnapshotRemoteObject) {
  std::shared_ptr<HermesRuntime> runtime = asyncRuntime.runtime();
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(
    storeValue([1, 2, 3]);
    debugger;
  )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // We should get a pause before the first statement.
  expectPaused(conn, "other", {{"global", 2, 1}});

  {
    // Take a heap snapshot first to assign IDs.
    m::heapProfiler::TakeHeapSnapshotRequest req;
    req.id = msgId++;
    req.reportProgress = false;
    // We don't need the response because we can directly query for object IDs
    // from the runtime.
    conn.send(req.toJsonStr());
    expectHeapSnapshot(conn, req.id);
  }

  const uint64_t globalObjID = runtime->getUniqueID(runtime->global());
  jsi::Value storedValue = asyncRuntime.awaitStoredValue();
  const uint64_t storedObjID =
      runtime->getUniqueID(storedValue.asObject(*runtime));

  auto testObject = [this, &msgId](
                        uint64_t objID,
                        const char *type,
                        const char *className,
                        const char *description,
                        const char *subtype) {
    // Get the object by its snapshot ID.
    m::heapProfiler::GetObjectByHeapObjectIdRequest req;
    req.id = msgId++;
    req.objectId = std::to_string(objID);
    auto resp = send<
        m::heapProfiler::GetObjectByHeapObjectIdRequest,
        m::heapProfiler::GetObjectByHeapObjectIdResponse>(conn, req);
    EXPECT_EQ(resp.result.type, type);
    EXPECT_EQ(resp.result.className, className);
    EXPECT_EQ(resp.result.description, description);
    if (subtype) {
      EXPECT_EQ(resp.result.subtype, subtype);
    }

    // Check that fetching the object by heap snapshot ID works.
    m::heapProfiler::GetHeapObjectIdRequest idReq;
    idReq.id = msgId++;
    idReq.objectId = resp.result.objectId.value();
    auto idResp = send<
        m::heapProfiler::GetHeapObjectIdRequest,
        m::heapProfiler::GetHeapObjectIdResponse>(conn, idReq);
    EXPECT_EQ(atoi(idResp.heapSnapshotObjectId.c_str()), objID);
  };

  // Test once before a collection.
  testObject(globalObjID, "object", "Object", "Object", nullptr);
  testObject(storedObjID, "object", "Array", "Array(3)", "array");
  // Force a collection to move the heap.
  runtime->instrumentation().collectGarbage("test");
  // A collection should not disturb the unique ID lookup, and it should be
  // the same object as before. Note that it won't have the same remote ID,
  // because Hermes doesn't do uniquing.
  testObject(globalObjID, "object", "Object", "Object", nullptr);
  testObject(storedObjID, "object", "Array", "Array(3)", "array");

  // Resume and exit.
  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::ProfilerBasicOperation
TEST_F(ConnectionTests, DISABLED_testBasicProfilerOperation) {
  SamplingProfilerRAII spRegistration(asyncRuntime);
  int msgId = 1;

  asyncRuntime.executeScriptAsync(R"(
      while(!shouldStop());
  )");

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Start the sampling profiler. At this point it is not safe to manipulate the
  // VM, so...
  send<m::profiler::StartRequest>(conn, msgId++);

  // Disable the debugger.
  send<m::debugger::DisableRequest>(conn, msgId++);

  // Keep the profiler running for a small amount of time to allow for some
  // samples to be collected.
  std::this_thread::sleep_for(500ms);

  // Finally, re-enable the debugger in order to stop profiling.
  send<m::debugger::EnableRequest>(conn, msgId++);

  // Being re-attached to the VM, send the stop sampling profile request.
  {
    auto resp = send<m::profiler::StopRequest, m::profiler::StopResponse>(
        conn, msgId++);

    const m::profiler::Profile &profile = resp.profile;
    EXPECT_GT(profile.nodes.size(), 0);
    EXPECT_LT(profile.startTime, profile.endTime);
    ASSERT_TRUE(profile.samples);
    EXPECT_FALSE(profile.samples->empty());
    ASSERT_TRUE(profile.timeDeltas);
    EXPECT_EQ(profile.samples->size(), profile.timeDeltas->size());
  }

  asyncRuntime.stop();
}

// Also implemented as CDPAgentTest::RuntimeGlobalLexicalScopeNames
TEST_F(ConnectionTests, testGlobalLexicalScopeNames) {
  int msgId = 1;
  asyncRuntime.executeScriptAsync(R"(
    let globalLet = "let";
    const globalConst = "const";
    var globalVar = "var";

    let func1 = () => {
      let local1 = 111;
      func2();
    }

    function func2() {
      let func3 = () => {
        let local3 = 333;
        debugger;
      }

      let local2 = 222;
      func3();
    }

    func1();
  )");

  send<m::runtime::EnableRequest>(conn, msgId++);
  expectExecutionContextCreated(conn);

  send<m::debugger::EnableRequest>(conn, msgId++);
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(
      conn,
      "other",
      {{"func3", 13, 3}, {"func2", 17, 2}, {"func1", 7, 2}, {"global", 20, 1}});

  m::runtime::GlobalLexicalScopeNamesRequest req;
  req.id = msgId;
  req.executionContextId = 1;
  conn.send(req.toJsonStr());

  auto resp =
      expectResponse<m::runtime::GlobalLexicalScopeNamesResponse>(conn, msgId);
  EXPECT_EQ(resp.id, msgId++);
  std::sort(resp.names.begin(), resp.names.end());
  std::vector<std::string> expectedNames{"func1", "globalConst", "globalLet"};
  EXPECT_EQ(resp.names, expectedNames);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// TODO: Missing test in CDPAgentTest
TEST_F(ConnectionTests, testInvalidExecutionContext) {
  int msgId = 1;
  send<m::runtime::EnableRequest>(conn, msgId++);
  auto executionContextNotification = expectExecutionContextCreated(conn);
  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(debugger;)");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);
  expectPaused(conn, "other", {{"global", 0, 1}});

  m::runtime::GlobalLexicalScopeNamesRequest req;
  req.id = msgId;
  // Chose an execution context ID other than what was returned as the current
  // execution context ID.
  req.executionContextId = ~executionContextNotification.context.id;
  conn.send(req.toJsonStr());
  expectResponse<m::ErrorResponse>(conn, msgId++);

  send<m::debugger::ResumeRequest>(conn, msgId++);
  expectNotification<m::debugger::ResumedNotification>(conn);
}

// Also implemented as CDPAgentTest::HeapProfilerSnapshot
TEST_F(ConnectionTests, heapSnapshot) {
  std::shared_ptr<HermesRuntime> runtime = asyncRuntime.runtime();
  int msgId = 1;

  send<m::debugger::EnableRequest>(conn, msgId++);

  asyncRuntime.executeScriptAsync(R"(
      while(!shouldStop());
  )");
  expectNotification<m::debugger::ScriptParsedNotification>(conn);

  // Request a heap snapshot.
  m::heapProfiler::TakeHeapSnapshotRequest req;
  req.id = msgId;
  req.reportProgress = false;
  conn.send(req.toJsonStr());

  // Expect the heap snapshot chunks and confirmation, in order.
  expectHeapSnapshot(conn, req.id);

  // Expect no more chunks are pending.
  expectNothing(conn);

  asyncRuntime.stop();
}

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
