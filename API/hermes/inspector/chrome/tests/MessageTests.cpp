/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>
#include <variant>

#include <gtest/gtest.h>

#include <hermes/inspector/chrome/JSONValueInterfaces.h>
#include <hermes/inspector/chrome/MessageTypes.h>
#include <hermes/inspector/chrome/MessageTypesInlines.h>

#include <llvh/ADT/SmallVector.h>

#include "TestHelpers.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {
namespace message {

struct MessageTests : public ::testing::Test {
  MessageTests() : jsonAlloc(), factory(jsonAlloc) {}
  JSLexer::Allocator jsonAlloc;
  JSONFactory factory;
};

TEST_F(MessageTests, testSerializeSomeFieldsInRequest) {
  debugger::SetBreakpointByUrlRequest req;
  // req.id should default to 0
  req.lineNumber = 2;
  req.url = "http://example.com/example.js";

  JSONObject *expected = mustParseStrAsJsonObj(
      R"({
    "id": 0,
    "method": "Debugger.setBreakpointByUrl",
    "params": {
      "lineNumber": 2,
      "url": "http://example.com/example.js"
    }
  })",
      factory);
  EXPECT_EQ(req.toJsonStr(), jsonValToStr(expected));
}

TEST_F(MessageTests, testDeserializeSomeFieldsInRequest) {
  JSONObject *message = mustParseStrAsJsonObj(
      R"({
      "id": 10,
      "method": "Debugger.setBreakpointByUrl",
      "params": {
        "lineNumber": 42,
        "url": "http://example.com"
      }
    }
  )",
      factory);
  auto req = mustMake<debugger::SetBreakpointByUrlRequest>(message);

  EXPECT_EQ(req.toJsonStr(), jsonValToStr(message));
  EXPECT_EQ(req.id, 10);
  EXPECT_EQ(req.method, "Debugger.setBreakpointByUrl");
  EXPECT_EQ(req.lineNumber, 42);
  EXPECT_FALSE(req.columnNumber.has_value());
  EXPECT_FALSE(req.condition.has_value());
  EXPECT_EQ(req.url, "http://example.com");
  EXPECT_FALSE(req.urlRegex.has_value());
}

TEST_F(MessageTests, testSerializeAllFieldsInRequest) {
  debugger::SetBreakpointByUrlRequest req;
  req.id = 1;
  req.lineNumber = 2;
  req.columnNumber = 3;
  req.condition = "foo == 42";
  req.url = "http://example.com/example.js";
  req.urlRegex = "http://example.com/.*";

  JSONObject *expected = mustParseStrAsJsonObj(
      R"({
    "id": 1,
    "method": "Debugger.setBreakpointByUrl",
    "params": {
      "lineNumber": 2,
      "columnNumber": 3,
      "condition": "foo == 42",
      "url": "http://example.com/example.js",
      "urlRegex": "http://example.com/.*"
    }
  })",
      factory);
  EXPECT_EQ(req.toJsonStr(), jsonValToStr(expected));
}

TEST_F(MessageTests, testDeserializeAllFieldsInRequest) {
  JSONObject *message = mustParseStrAsJsonObj(
      R"({
    "id": 1,
    "method": "Debugger.setBreakpointByUrl",
    "params": {
      "lineNumber": 2,
      "columnNumber": 3,
      "condition": "foo == 42",
      "url": "http://example.com/example.js",
      "urlRegex": "http://example.com/.*"
    }
  })",
      factory);
  auto req = mustMake<debugger::SetBreakpointByUrlRequest>(message);

  EXPECT_EQ(req.id, 1);
  EXPECT_EQ(req.method, "Debugger.setBreakpointByUrl");
  EXPECT_EQ(req.lineNumber, 2);
  EXPECT_EQ(req.columnNumber, 3);
  EXPECT_EQ(req.condition, "foo == 42");
  EXPECT_EQ(req.url, "http://example.com/example.js");
  EXPECT_EQ(req.urlRegex, "http://example.com/.*");
}

TEST_F(MessageTests, testSerializeResponse) {
  debugger::Location location;
  location.scriptId = "myScriptId";
  location.lineNumber = 2;
  location.columnNumber = 3;

  debugger::SetBreakpointByUrlResponse resp;
  resp.id = 1;
  resp.breakpointId = "myBreakpointId";
  resp.locations.push_back(std::move(location));

  JSONObject *expected = mustParseStrAsJsonObj(
      R"({
    "id": 1,
    "result": {
      "breakpointId": "myBreakpointId",
      "locations": [
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
      ]
    }
  })",
      factory);
  EXPECT_EQ(resp.toJsonStr(), jsonValToStr(expected));
}

TEST_F(MessageTests, testDeserializeResponse) {
  JSONObject *message = mustParseStrAsJsonObj(
      R"({
    "id": 1,
    "result": {
      "breakpointId": "myBreakpointId",
      "locations": [
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
      ]
    }
  })",
      factory);
  auto resp = mustMake<debugger::SetBreakpointByUrlResponse>(message);
  EXPECT_EQ(resp.toJsonStr(), jsonValToStr(message));
  EXPECT_EQ(resp.id, 1);
  EXPECT_EQ(resp.breakpointId, "myBreakpointId");
  EXPECT_EQ(resp.locations.size(), 1);
  EXPECT_EQ(resp.locations[0].lineNumber, 2);
  EXPECT_EQ(resp.locations[0].columnNumber, 3);
  EXPECT_EQ(resp.locations[0].scriptId, "myScriptId");
}

TEST_F(MessageTests, testSerializeNotification) {
  debugger::Location startLocation;
  startLocation.lineNumber = 1;
  startLocation.scriptId = "script1";

  debugger::Location endLocation;
  endLocation.lineNumber = 2;
  endLocation.scriptId = "script2";

  debugger::Scope scope;
  scope.type = "closure";
  scope.object.type = "object";
  scope.object.subtype = "regexp";
  scope.object.className = "RegExp";
  scope.object.value = "{ \"foo\" : \"bar\" }";
  scope.object.unserializableValue = "nope";
  scope.object.description = "myDesc";
  scope.object.objectId = "id1";
  scope.name = "myScope";
  scope.startLocation = std::move(startLocation);
  scope.endLocation = std::move(endLocation);

  debugger::CallFrame frame;
  frame.callFrameId = "callFrame1";
  frame.functionName = "foo1";
  frame.location.scriptId = "script1";
  frame.location.lineNumber = 3;
  frame.location.columnNumber = 4;
  frame.url = "foo.js";
  frame.scopeChain.push_back(std::move(scope));
  frame.thisObj.type = "function";

  debugger::PausedNotification note;
  note.callFrames.push_back(std::move(frame));
  note.reason = "debugCommand";
  note.data = "{ \"foo\" : \"bar\" }";
  note.hitBreakpoints = std::vector<std::string>{"a", "b"};

  JSONObject *expected = mustParseStrAsJsonObj(
      R"({
    "method": "Debugger.paused",
    "params": {
      "callFrames": [
        {
          "callFrameId": "callFrame1",
          "functionName": "foo1",
          "location": {
            "scriptId": "script1",
            "lineNumber": 3,
            "columnNumber": 4
          },
          "url": "foo.js",
          "scopeChain": [
            {
              "type": "closure",
              "object": {
                "type": "object",
                "subtype": "regexp",
                "className": "RegExp",
                "value": { "foo": "bar" },
                "unserializableValue": "nope",
                "description": "myDesc",
                "objectId": "id1"
              },
              "name": "myScope",
              "startLocation": {
                "lineNumber": 1,
                "scriptId": "script1"
              },
              "endLocation": {
                "lineNumber": 2,
                "scriptId": "script2"
              }
            }
          ],
          "this": { "type": "function" }
        }
      ],
      "reason": "debugCommand",
      "data": {
        "foo": "bar"
      },
      "hitBreakpoints": [ "a", "b" ]
    }
  })",
      factory);
  EXPECT_EQ(note.toJsonStr(), jsonValToStr(expected));
}

TEST_F(MessageTests, testDeserializeNotification) {
  JSONObject *message = mustParseStrAsJsonObj(
      R"({
    "method": "Debugger.paused",
    "params": {
      "callFrames": [
        {
          "callFrameId": "callFrame1",
          "functionName": "foo1",
          "location": {
            "scriptId": "script1",
            "lineNumber": 3,
            "columnNumber": 4
          },
          "url": "foo.js",
          "scopeChain": [
            {
              "type": "closure",
              "object": {
                "type": "object",
                "subtype": "regexp",
                "className": "RegExp",
                "value": { "foo": "bar" },
                "unserializableValue": "nope",
                "description": "myDesc",
                "objectId": "id1"
              },
              "name": "myScope",
              "startLocation": {
                "lineNumber": 1,
                "scriptId": "script1"
              },
              "endLocation": {
                "lineNumber": 2,
                "scriptId": "script2"
              }
            }
          ],
          "this": { "type": "function" }
        }
      ],
      "reason": "debugCommand",
      "data": {
        "foo": "bar"
      },
      "hitBreakpoints": [ "a", "b" ]
    }
  })",
      factory);
  auto note = mustMake<debugger::PausedNotification>(message);

  EXPECT_EQ(note.method, "Debugger.paused");
  EXPECT_EQ(note.callFrames.size(), 1);
  EXPECT_EQ(note.reason, "debugCommand");
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  props.push_back({factory.getString("foo"), factory.getString("bar")});
  ASSERT_TRUE(jsonValsEQ(
      mustParseStrAsJsonObj(note.data.value(), factory),
      factory.newObject(props.begin(), props.end())));
  auto expectedHitBreakpoints = std::vector<std::string>{"a", "b"};
  EXPECT_EQ(note.hitBreakpoints, expectedHitBreakpoints);

  debugger::CallFrame &callFrame = note.callFrames[0];
  EXPECT_EQ(callFrame.callFrameId, "callFrame1");
  EXPECT_EQ(callFrame.functionName, "foo1");
  EXPECT_EQ(callFrame.location.scriptId, "script1");
  EXPECT_EQ(callFrame.location.lineNumber, 3);
  EXPECT_EQ(callFrame.location.columnNumber, 4);
  EXPECT_EQ(callFrame.url, "foo.js");
  EXPECT_EQ(callFrame.scopeChain.size(), 1);
  EXPECT_EQ(callFrame.thisObj.type, "function");

  debugger::Scope &scope = callFrame.scopeChain[0];
  EXPECT_EQ(scope.type, "closure");
  EXPECT_EQ(scope.object.type, "object");
  EXPECT_EQ(scope.object.subtype, "regexp");
  EXPECT_EQ(scope.object.className, "RegExp");
  ASSERT_TRUE(jsonValsEQ(
      mustParseStrAsJsonObj(scope.object.value.value(), factory),
      factory.newObject(props.begin(), props.end())));
  EXPECT_EQ(scope.object.unserializableValue, "nope");
  EXPECT_EQ(scope.object.description, "myDesc");
  EXPECT_EQ(scope.object.objectId, "id1");
  EXPECT_EQ(scope.name, "myScope");

  debugger::Location &startLocation = scope.startLocation.value();
  EXPECT_EQ(startLocation.lineNumber, 1);
  EXPECT_FALSE(startLocation.columnNumber.has_value());
  EXPECT_EQ(startLocation.scriptId, "script1");

  debugger::Location &endLocation = scope.endLocation.value();
  EXPECT_EQ(endLocation.lineNumber, 2);
  EXPECT_FALSE(endLocation.columnNumber.has_value());
  EXPECT_EQ(endLocation.scriptId, "script2");
}

TEST_F(MessageTests, TestSerializeAsyncStackTrace) {
  runtime::StackTrace stack;
  stack.description = "childStack";
  stack.parent = std::make_unique<runtime::StackTrace>();
  stack.parent->description = "parentStack";

  JSONObject *expected = mustParseStrAsJsonObj(
      R"({
    "description": "childStack",
    "callFrames": [],
    "parent": {
      "description": "parentStack",
      "callFrames": []
    }
  })",
      factory);
  EXPECT_EQ(stack.toJsonStr(), jsonValToStr(expected));
}

TEST_F(MessageTests, TestDeserializeAsyncStackTrace) {
  JSONObject *message = mustParseStrAsJsonObj(
      R"({
    "description": "childStack",
    "callFrames": [],
    "parent": {
      "description": "parentStack",
      "callFrames": []
    }
  })",
      factory);
  auto stack = mustMake<runtime::StackTrace>(message);

  EXPECT_EQ(stack.description, "childStack");
  EXPECT_EQ(stack.callFrames.size(), 0);
  EXPECT_EQ(stack.parent->description, "parentStack");
  EXPECT_EQ(stack.parent->callFrames.size(), 0);
}

TEST_F(MessageTests, TestRequestFromJson) {
  std::unique_ptr<Request> baseReq1 = mustGetRequestFromJson(R"({
    "id": 1,
    "method": "Debugger.enable"
  })");
  auto req1 = static_cast<debugger::EnableRequest *>(baseReq1.get());
  EXPECT_EQ(req1->id, 1);
  EXPECT_EQ(req1->method, "Debugger.enable");

  std::unique_ptr<Request> baseReq2 = mustGetRequestFromJson(R"({
    "id": 2,
    "method": "Debugger.removeBreakpoint",
    "params": {
      "breakpointId": "foobar"
    }
  })");
  auto req2 = static_cast<debugger::RemoveBreakpointRequest *>(baseReq2.get());
  EXPECT_EQ(req2->id, 2);
  EXPECT_EQ(req2->method, "Debugger.removeBreakpoint");
  EXPECT_EQ(req2->breakpointId, "foobar");

  std::unique_ptr<Request> invalidReq = Request::fromJson("invalid");
  EXPECT_TRUE(invalidReq == nullptr);
}

TEST_F(MessageTests, TestBreakpointRequestFromJSON) {
  std::unique_ptr<Request> baseReq = mustGetRequestFromJson(R"({
    "id": 1,
    "method": "Debugger.setBreakpoint",
    "params": {
      "location": {
        "scriptId": "23",
        "lineNumber": 45,
        "columnNumber": 67
      }
    }
  })");
  auto req = static_cast<debugger::SetBreakpointRequest *>(baseReq.get());
  EXPECT_EQ(req->location.scriptId, "23");
  EXPECT_EQ(req->location.lineNumber, 45);
  EXPECT_EQ(req->location.columnNumber.value(), 67);
}

struct MyHandler : public NoopRequestHandler {
  void handle(const debugger::EnableRequest &req) override {
    enableReq = req;
  }

  void handle(const debugger::RemoveBreakpointRequest &req) override {
    removeReq = req;
  }

  debugger::EnableRequest enableReq;
  debugger::RemoveBreakpointRequest removeReq;
};

TEST_F(MessageTests, TestRequestHandler) {
  MyHandler handler;

  std::unique_ptr<Request> enableReq = mustGetRequestFromJson(R"({
    "id": 1,
    "method": "Debugger.enable"
  })");
  enableReq->accept(handler);

  EXPECT_EQ(handler.enableReq.id, 1);
  EXPECT_EQ(handler.enableReq.method, "Debugger.enable");

  std::unique_ptr<Request> removeReq = mustGetRequestFromJson(R"({
    "id": 2,
    "method": "Debugger.removeBreakpoint",
    "params": {
      "breakpointId": "foobar"
    }
  })");
  removeReq->accept(handler);

  EXPECT_EQ(handler.removeReq.id, 2);
  EXPECT_EQ(handler.removeReq.method, "Debugger.removeBreakpoint");
  EXPECT_EQ(handler.removeReq.breakpointId, "foobar");
}

TEST_F(MessageTests, testEnableRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.enable"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::EnableRequest *resolvedReq =
      dynamic_cast<debugger::EnableRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::EnableRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.enable");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testDisableRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.disable"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::DisableRequest *resolvedReq =
      dynamic_cast<debugger::DisableRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::DisableRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.disable");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testEvaluateOnCallFrameRequestMinimal) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.evaluateOnCallFrame",
      "params":{
        "callFrameId" : "42",
        "expression": "Foo Bar"
    }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::EvaluateOnCallFrameRequest *resolvedReq =
      dynamic_cast<debugger::EvaluateOnCallFrameRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::EvaluateOnCallFrameRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.evaluateOnCallFrame");
  EXPECT_EQ(resolvedReq->callFrameId, "42");
  EXPECT_EQ(resolvedReq->expression, "Foo Bar");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->callFrameId, deserializedReq.callFrameId);
  EXPECT_EQ(resolvedReq->expression, deserializedReq.expression);

  EXPECT_FALSE(resolvedReq->generatePreview.has_value());
  EXPECT_FALSE(resolvedReq->objectGroup.has_value());
  EXPECT_FALSE(resolvedReq->includeCommandLineAPI.has_value());
  EXPECT_FALSE(resolvedReq->silent.has_value());
  EXPECT_FALSE(resolvedReq->returnByValue.has_value());
  EXPECT_FALSE(resolvedReq->throwOnSideEffect.has_value());
}

TEST_F(MessageTests, testEvaluateOnCallFrameRequestFull) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.evaluateOnCallFrame",
      "params":{
        "callFrameId" : "42",
        "expression": "Foo Bar",
        "objectGroup" : "FooBarGroup",
        "includeCommandLineAPI" : false,
        "silent" : true,
        "returnByValue" : false,
        "throwOnSideEffect" : true,
        "generatePreview": true
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::EvaluateOnCallFrameRequest *resolvedReq =
      dynamic_cast<debugger::EvaluateOnCallFrameRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::EvaluateOnCallFrameRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics, resolvedReq is correct
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.evaluateOnCallFrame");
  EXPECT_EQ(resolvedReq->callFrameId, "42");
  EXPECT_EQ(resolvedReq->expression, "Foo Bar");

  EXPECT_TRUE(resolvedReq->objectGroup.has_value());
  EXPECT_TRUE(resolvedReq->includeCommandLineAPI.has_value());
  EXPECT_TRUE(resolvedReq->silent.has_value());
  EXPECT_TRUE(resolvedReq->returnByValue.has_value());
  EXPECT_TRUE(resolvedReq->throwOnSideEffect.has_value());
  EXPECT_TRUE(resolvedReq->generatePreview.has_value());

  EXPECT_TRUE(resolvedReq->objectGroup.value() == "FooBarGroup");
  EXPECT_TRUE(resolvedReq->includeCommandLineAPI.value() == false);
  EXPECT_TRUE(resolvedReq->silent.value() == true);
  EXPECT_TRUE(resolvedReq->returnByValue.value() == false);
  EXPECT_TRUE(resolvedReq->throwOnSideEffect.value() == true);
  EXPECT_TRUE(resolvedReq->generatePreview.value() == true);

  // Specifics, resolvedReq and deserialized match

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->callFrameId, deserializedReq.callFrameId);
  EXPECT_EQ(resolvedReq->expression, deserializedReq.expression);
  EXPECT_EQ(
      resolvedReq->objectGroup.value(), deserializedReq.objectGroup.value());
  EXPECT_EQ(
      resolvedReq->includeCommandLineAPI.value(),
      deserializedReq.includeCommandLineAPI.value());
  EXPECT_EQ(resolvedReq->silent.value(), deserializedReq.silent.value());
  EXPECT_EQ(
      resolvedReq->returnByValue.value(),
      deserializedReq.returnByValue.value());
  EXPECT_EQ(
      resolvedReq->throwOnSideEffect.value(),
      deserializedReq.throwOnSideEffect.value());
}

TEST_F(MessageTests, testPauseRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.pause"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::PauseRequest *resolvedReq =
      dynamic_cast<debugger::PauseRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::PauseRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.pause");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testRemoveBreakpointRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.removeBreakpoint",
      "params":{
        "breakpointId" : "42"
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::RemoveBreakpointRequest *resolvedReq =
      dynamic_cast<debugger::RemoveBreakpointRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::RemoveBreakpointRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.removeBreakpoint");
  EXPECT_TRUE(resolvedReq->breakpointId == "42");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->breakpointId, deserializedReq.breakpointId);
}

TEST_F(MessageTests, testResumeRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.resume",
      "params": {
        "terminateOnResume": false
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::ResumeRequest *resolvedReq =
      dynamic_cast<debugger::ResumeRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::ResumeRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.resume");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testSetBreakpointRequestMinimal) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.setBreakpoint",
      "params":{
        "location" :
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetBreakpointRequest *resolvedReq =
      dynamic_cast<debugger::SetBreakpointRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::SetBreakpointRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  debugger::Location location;
  location.scriptId = "myScriptId";
  location.lineNumber = 2;
  location.columnNumber = 3;

  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.setBreakpoint");
  EXPECT_EQ(resolvedReq->location.scriptId, "myScriptId");
  EXPECT_EQ(resolvedReq->location.lineNumber, 2);
  EXPECT_EQ(resolvedReq->location.columnNumber, 3);

  EXPECT_FALSE(resolvedReq->condition.has_value());

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->location.scriptId, deserializedReq.location.scriptId);
  EXPECT_EQ(
      resolvedReq->location.lineNumber, deserializedReq.location.lineNumber);
  EXPECT_EQ(
      resolvedReq->location.columnNumber,
      deserializedReq.location.columnNumber);
}

TEST_F(MessageTests, testSetBreakpointRequestFull) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.setBreakpoint",
      "params":{
        "location" :
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        },
        "condition": "FooBarCondition"
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetBreakpointRequest *resolvedReq =
      dynamic_cast<debugger::SetBreakpointRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::SetBreakpointRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.setBreakpoint");
  EXPECT_EQ(resolvedReq->location.scriptId, "myScriptId");
  EXPECT_EQ(resolvedReq->location.lineNumber, 2);
  EXPECT_EQ(resolvedReq->location.columnNumber, 3);

  EXPECT_TRUE(resolvedReq->condition.has_value());
  EXPECT_EQ(resolvedReq->condition.value(), "FooBarCondition");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->location.scriptId, deserializedReq.location.scriptId);
  EXPECT_EQ(
      resolvedReq->location.lineNumber, deserializedReq.location.lineNumber);
  EXPECT_EQ(
      resolvedReq->location.columnNumber,
      deserializedReq.location.columnNumber);
  EXPECT_EQ(resolvedReq->condition.value(), deserializedReq.condition.value());
}

TEST_F(MessageTests, testSetBreakpointByUrlRequestMinimal) {
  std::string message = R"(
    {
      "id": 1,
      "method": "Debugger.setBreakpointByUrl",
      "params": {
        "lineNumber": 2
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetBreakpointByUrlRequest *resolvedReq =
      dynamic_cast<debugger::SetBreakpointByUrlRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetBreakpointByUrlRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 1);
  EXPECT_EQ(resolvedReq->method, "Debugger.setBreakpointByUrl");
  EXPECT_EQ(resolvedReq->lineNumber, 2);

  EXPECT_FALSE(resolvedReq->condition.has_value());
  EXPECT_FALSE(resolvedReq->columnNumber.has_value());
  EXPECT_FALSE(resolvedReq->url.has_value());
  EXPECT_FALSE(resolvedReq->urlRegex.has_value());

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->lineNumber, deserializedReq.lineNumber);
}

TEST_F(MessageTests, testSetBreakpointByUrlRequestFull) {
  std::string message = R"(
    {
      "id": 1,
      "method": "Debugger.setBreakpointByUrl",
      "params": {
        "lineNumber": 2,
        "columnNumber": 3,
        "condition": "foo == 42",
        "url": "http://example.com/example.js",
        "urlRegex": "http://example.com/.*"
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetBreakpointByUrlRequest *resolvedReq =
      dynamic_cast<debugger::SetBreakpointByUrlRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetBreakpointByUrlRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 1);
  EXPECT_EQ(resolvedReq->method, "Debugger.setBreakpointByUrl");
  EXPECT_EQ(resolvedReq->lineNumber, 2);

  EXPECT_TRUE(resolvedReq->condition.has_value());
  EXPECT_EQ(resolvedReq->condition.value(), "foo == 42");
  EXPECT_TRUE(resolvedReq->columnNumber.has_value());
  EXPECT_EQ(resolvedReq->columnNumber.value(), 3);
  EXPECT_TRUE(resolvedReq->url.has_value());
  EXPECT_EQ(resolvedReq->url.value(), "http://example.com/example.js");
  EXPECT_TRUE(resolvedReq->urlRegex.has_value());
  EXPECT_EQ(resolvedReq->urlRegex.value(), "http://example.com/.*");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->lineNumber, deserializedReq.lineNumber);
  EXPECT_EQ(resolvedReq->condition.value(), deserializedReq.condition.value());
  EXPECT_EQ(
      resolvedReq->columnNumber.value(), deserializedReq.columnNumber.value());
  EXPECT_EQ(resolvedReq->url.value(), deserializedReq.url.value());
  EXPECT_EQ(resolvedReq->urlRegex.value(), deserializedReq.urlRegex.value());
}

TEST_F(MessageTests, testSetBreakpointsActiveRequest) {
  std::string message = R"(
    {
      "id": 1,
      "method": "Debugger.setBreakpointsActive",
      "params": {
        "active": true
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetBreakpointsActiveRequest *resolvedReq =
      dynamic_cast<debugger::SetBreakpointsActiveRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetBreakpointsActiveRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 1);
  EXPECT_EQ(resolvedReq->method, "Debugger.setBreakpointsActive");
  EXPECT_EQ(resolvedReq->active, true);

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->active, deserializedReq.active);
}

TEST_F(MessageTests, testSetInstrumentationBreakpointRequest) {
  std::string message = R"(
    {
      "id": 1,
      "method": "Debugger.setInstrumentationBreakpoint",
      "params": {
        "instrumentation": "TODO: THIS SHOUD NOT BE ACCEPTED BY ENUM"
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetInstrumentationBreakpointRequest *resolvedReq =
      dynamic_cast<debugger::SetInstrumentationBreakpointRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetInstrumentationBreakpointRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 1);
  EXPECT_EQ(resolvedReq->method, "Debugger.setInstrumentationBreakpoint");
  EXPECT_EQ(
      resolvedReq->instrumentation, "TODO: THIS SHOUD NOT BE ACCEPTED BY ENUM");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->instrumentation, deserializedReq.instrumentation);
}

TEST_F(MessageTests, testSetPauseOnExceptionsRequest) {
  std::string message = R"(
    {
      "id": 1,
      "method": "Debugger.setPauseOnExceptions",
      "params": {
        "state": "TODO: THIS SHOUD NOT BE ACCEPTED BY ENUM"
      }
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::SetPauseOnExceptionsRequest *resolvedReq =
      dynamic_cast<debugger::SetPauseOnExceptionsRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetPauseOnExceptionsRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 1);
  EXPECT_EQ(resolvedReq->method, "Debugger.setPauseOnExceptions");
  EXPECT_EQ(resolvedReq->state, "TODO: THIS SHOUD NOT BE ACCEPTED BY ENUM");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
  EXPECT_EQ(resolvedReq->state, deserializedReq.state);
}

TEST_F(MessageTests, testStepIntoRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.stepInto"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::StepIntoRequest *resolvedReq =
      dynamic_cast<debugger::StepIntoRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::StepIntoRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.stepInto");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testStepOutRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.stepOut"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::StepOutRequest *resolvedReq =
      dynamic_cast<debugger::StepOutRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::StepOutRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.stepOut");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testStepOverRequest) {
  std::string message = R"(
    {
      "id": 10,
      "method": "Debugger.stepOver"
    }
  )";

  // Builder does not throw
  auto req = mustGetRequestFromJson(message);
  debugger::StepOverRequest *resolvedReq =
      dynamic_cast<debugger::StepOverRequest *>(req.get());

  // Builder returns correct type
  EXPECT_FALSE(resolvedReq == nullptr);

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::StepOverRequest>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(resolvedReq->id, 10);
  EXPECT_EQ(resolvedReq->method, "Debugger.stepOver");

  EXPECT_EQ(resolvedReq->id, deserializedReq.id);
  EXPECT_EQ(resolvedReq->method, deserializedReq.method);
}

TEST_F(MessageTests, testEvaluateOnCallFrameResponseMinimal) {
  std::string message = R"(
    {
      "result":
        {
          "result":{
            "type": "string"
          }
        },
      "id":2
    }
  )";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::EvaluateOnCallFrameResponse>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  EXPECT_FALSE(deserializedReq.result.subtype.has_value());
  EXPECT_FALSE(deserializedReq.result.value.has_value());
  EXPECT_FALSE(deserializedReq.result.unserializableValue.has_value());
  EXPECT_FALSE(deserializedReq.result.description.has_value());
  EXPECT_FALSE(deserializedReq.result.objectId.has_value());
  EXPECT_FALSE(deserializedReq.result.preview.has_value());

  // Specifics
  EXPECT_EQ(deserializedReq.id, 2);
  EXPECT_EQ(deserializedReq.result.type, "string");
}

TEST_F(MessageTests, testEvaluateOnCallFrameResponseFull) {
  std::string message = R"(
    {
      "result":
        {
          "result":{
            "type": "string",
            "subtype": "SuperString",
            "value": {"foobarkey": "foobarval"},
            "unserializableValue": "unserializableValueVal",
            "description": "A Wonderful desc",
            "objectId": "AnObjectID",
            "preview": {
              "type": "object",
              "overflow": false,
              "properties": []
            }
          }
        },
      "id":2
    }
  )";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::EvaluateOnCallFrameResponse>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  EXPECT_TRUE(deserializedReq.result.subtype.has_value());
  EXPECT_TRUE(deserializedReq.result.value.has_value());
  EXPECT_TRUE(deserializedReq.result.unserializableValue.has_value());
  EXPECT_TRUE(deserializedReq.result.description.has_value());
  EXPECT_TRUE(deserializedReq.result.objectId.has_value());
  EXPECT_TRUE(deserializedReq.result.preview.has_value());

  EXPECT_EQ(deserializedReq.result.subtype.value(), "SuperString");
  ASSERT_TRUE(jsonValsEQ(
      mustParseStrAsJsonObj(deserializedReq.result.value.value(), factory),
      mustParseStrAsJsonObj(R"({"foobarkey": "foobarval"})", factory)));
  EXPECT_EQ(
      deserializedReq.result.unserializableValue.value(),
      "unserializableValueVal");
  EXPECT_EQ(deserializedReq.result.description.value(), "A Wonderful desc");
  EXPECT_EQ(deserializedReq.result.objectId.value(), "AnObjectID");

  // Specifics
  EXPECT_EQ(deserializedReq.id, 2);
  EXPECT_EQ(deserializedReq.result.type, "string");
  EXPECT_EQ(deserializedReq.result.preview->type, "object");
  EXPECT_EQ(deserializedReq.result.preview->overflow, false);
  EXPECT_EQ(deserializedReq.result.preview->properties.size(), 0);
}

TEST_F(MessageTests, testSetBreakpointByUrlResponse) {
  std::string message = R"({
    "id": 1,
    "result":{
      "breakpointId": "myBreakpointId",
      "locations": [
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
      ]
    }
  })";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetBreakpointByUrlResponse>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(deserializedReq.id, 1);
  EXPECT_EQ(deserializedReq.breakpointId, "myBreakpointId");
  EXPECT_EQ(deserializedReq.locations.size(), 1);
  EXPECT_EQ(deserializedReq.locations[0].lineNumber, 2);
  EXPECT_EQ(deserializedReq.locations[0].columnNumber, 3);
  EXPECT_EQ(deserializedReq.locations[0].scriptId, "myScriptId");
}

TEST_F(MessageTests, testSetBreakpointResponse) {
  std::string message = R"({
    "id": 1,
    "result":{
      "breakpointId": "myBreakpointId",
      "actualLocation":
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
    }
  })";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::SetBreakpointResponse>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(deserializedReq.breakpointId, "myBreakpointId");
  EXPECT_EQ(deserializedReq.actualLocation.lineNumber, 2);
  EXPECT_EQ(deserializedReq.actualLocation.columnNumber, 3);
  EXPECT_EQ(deserializedReq.actualLocation.scriptId, "myScriptId");
  EXPECT_EQ(deserializedReq.id, 1);
}

TEST_F(MessageTests, testSetInstrumentationBreakpointResponse) {
  std::string message = R"({
    "id": 1,
    "result":{
      "breakpointId": "myBreakpointId"
    }
  })";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::SetInstrumentationBreakpointResponse>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(deserializedReq.breakpointId, "myBreakpointId");
  EXPECT_EQ(deserializedReq.id, 1);
}

TEST_F(MessageTests, testBreakpointResolvedNotification) {
  std::string message = R"(
    {
      "method": "Debugger.breakpointResolved",
      "params":{
        "breakpointId" : "42",
        "location":
        {
          "lineNumber": 2,
          "columnNumber": 3,
          "scriptId": "myScriptId"
        }
      }
    }
  )";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq =
      mustMake<debugger::BreakpointResolvedNotification>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(deserializedReq.method, "Debugger.breakpointResolved");
  EXPECT_EQ(deserializedReq.breakpointId, "42");
  EXPECT_EQ(deserializedReq.location.lineNumber, 2);
  EXPECT_EQ(deserializedReq.location.columnNumber, 3);
  EXPECT_EQ(deserializedReq.location.scriptId, "myScriptId");
}

TEST_F(MessageTests, testPauseNotificationMinimal) {
  std::string message = R"(
    {
      "method": "Debugger.paused",
      "params":{
        "reason": "Some Valid Reason",
        "callFrames":[
          {
            "callFrameId": "aCallFrameId",
            "functionName": "aFunctionName",
            "location":{
              "lineNumber": 2,
              "columnNumber": 3,
              "scriptId": "myScriptId"
            },
            "url": "aURL",
            "scopeChain": [
              {
                "type": "aType",
                "object": {
                  "type": "aRemoteObjectType"
                }
              }
            ],
            "this": {
              "type": "aType"
            }
          }
        ]
      }
    }
  )";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::PausedNotification>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  EXPECT_FALSE(deserializedReq.callFrames[0].functionLocation.has_value());
  EXPECT_FALSE(deserializedReq.callFrames[0].returnValue.has_value());
  EXPECT_FALSE(deserializedReq.asyncStackTrace.has_value());
  EXPECT_FALSE(deserializedReq.hitBreakpoints.has_value());
  EXPECT_FALSE(deserializedReq.data.has_value());

  // Specifics
  EXPECT_EQ(deserializedReq.method, "Debugger.paused");
  EXPECT_EQ(deserializedReq.reason, "Some Valid Reason");
  EXPECT_EQ(deserializedReq.callFrames[0].functionName, "aFunctionName");
  EXPECT_EQ(deserializedReq.callFrames[0].callFrameId, "aCallFrameId");
  EXPECT_EQ(deserializedReq.callFrames[0].url, "aURL");
  EXPECT_EQ(deserializedReq.callFrames[0].location.lineNumber, 2);
  EXPECT_EQ(deserializedReq.callFrames[0].location.columnNumber, 3);
  EXPECT_EQ(deserializedReq.callFrames[0].location.scriptId, "myScriptId");
  EXPECT_EQ(deserializedReq.callFrames[0].scopeChain[0].type, "aType");
  EXPECT_EQ(
      deserializedReq.callFrames[0].scopeChain[0].object.type,
      "aRemoteObjectType");
  EXPECT_EQ(deserializedReq.callFrames[0].thisObj.type, "aType");
}

TEST_F(MessageTests, testPauseNotificationFull) {
  std::string message = R"(
    {
      "method": "Debugger.paused",
      "params":{
        "reason": "Some Valid Reason",
        "callFrames":[
          {
            "functionLocation": {
              "lineNumber": 2,
              "columnNumber": 3,
              "scriptId": "myScriptId"
            },
            "returnValue" : {
              "type": "aRemoteObjectType",
              "subtype": "subtype",
              "className":"className",
              "value": "value",
              "unserializableValue": "unserializableValue",
              "description": "description",
              "objectId": "objectId"
            },
            "callFrameId": "aCallFrameId",
            "functionName": "aFunctionName",
            "location":{
              "lineNumber": 2,
              "columnNumber": 3,
              "scriptId": "myScriptId"
            },
            "url": "aURL",
            "scopeChain": [
              {
                "type": "aType",
                "object": {
                  "type": "aRemoteObjectType"
                }
              }
            ],
            "this": {
              "type": "aType"
            }
          }
        ],
        "data": {"dataKey": "dataVal"},
        "hitBreakpoints": [
          "foo","bar"
        ],
        "asyncStackTrace":{
          "description": "an asyncStackTrace Desc",
          "callFrames":[
          {
            "functionName": "aFunctionName",
            "lineNumber": 2,
            "columnNumber": 3,
            "scriptId": "myScriptId",
            "url": "aURL"
          }
        ]
      }
      }
    }
  )";

  std::optional<debugger::Location> functionLocation;
  std::optional<runtime::RemoteObject> returnValue;
  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::PausedNotification>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));
  ASSERT_TRUE(jsonValsEQ(deserializedReq.toJsonVal(factory), messageJSON));

  // Check optionnals
  // ----------------
  EXPECT_TRUE(deserializedReq.callFrames[0].functionLocation.has_value());
  EXPECT_TRUE(deserializedReq.callFrames[0].returnValue.has_value());
  EXPECT_TRUE(deserializedReq.asyncStackTrace.has_value());
  EXPECT_TRUE(deserializedReq.hitBreakpoints.has_value());
  EXPECT_TRUE(deserializedReq.data.has_value());

  EXPECT_TRUE(
      deserializedReq.callFrames[0].returnValue.value().subtype.has_value());
  EXPECT_TRUE(
      deserializedReq.callFrames[0].returnValue.value().className.has_value());
  EXPECT_TRUE(deserializedReq.callFrames[0]
                  .returnValue.value()
                  .unserializableValue.has_value());
  EXPECT_TRUE(
      deserializedReq.callFrames[0].returnValue.value().value.has_value());
  EXPECT_TRUE(deserializedReq.callFrames[0]
                  .returnValue.value()
                  .description.has_value());
  EXPECT_TRUE(
      deserializedReq.callFrames[0].returnValue.value().objectId.has_value());

  // Check optionnals Values
  // -----------------------
  EXPECT_EQ(
      deserializedReq.callFrames[0].functionLocation.value().lineNumber, 2);
  EXPECT_EQ(
      deserializedReq.callFrames[0].functionLocation.value().columnNumber, 3);
  EXPECT_EQ(
      deserializedReq.callFrames[0].functionLocation.value().scriptId,
      "myScriptId");

  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().type,
      "aRemoteObjectType");
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().subtype.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().subtype.value(),
      "subtype");
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().className.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().className.value(),
      "className");
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().value.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().value.value(),
      "\"value\"");
  EXPECT_EQ(
      deserializedReq.callFrames[0]
          .returnValue.value()
          .unserializableValue.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0]
          .returnValue.value()
          .unserializableValue.value(),
      "unserializableValue");
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().description.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().description.value(),
      "description");
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().objectId.has_value(),
      true);
  EXPECT_EQ(
      deserializedReq.callFrames[0].returnValue.value().objectId.value(),
      "objectId");

  EXPECT_EQ(deserializedReq.hitBreakpoints.value()[0], "foo");
  EXPECT_EQ(deserializedReq.hitBreakpoints.value()[1], "bar");

  ASSERT_TRUE(jsonValsEQ(
      mustParseStrAsJsonObj(deserializedReq.data.value(), factory),
      mustParseStrAsJsonObj(R"({"dataKey": "dataVal"})", factory)));

  // Check Compulsory
  // ----------------
  EXPECT_EQ(deserializedReq.method, "Debugger.paused");
  EXPECT_EQ(deserializedReq.reason, "Some Valid Reason");
  EXPECT_EQ(deserializedReq.callFrames[0].functionName, "aFunctionName");
  EXPECT_EQ(deserializedReq.callFrames[0].callFrameId, "aCallFrameId");
  EXPECT_EQ(deserializedReq.callFrames[0].url, "aURL");
  EXPECT_EQ(deserializedReq.callFrames[0].location.lineNumber, 2);
  EXPECT_EQ(deserializedReq.callFrames[0].location.columnNumber, 3);
  EXPECT_EQ(deserializedReq.callFrames[0].location.scriptId, "myScriptId");
  EXPECT_EQ(deserializedReq.callFrames[0].scopeChain[0].type, "aType");
  EXPECT_EQ(
      deserializedReq.callFrames[0].scopeChain[0].object.type,
      "aRemoteObjectType");
  EXPECT_EQ(deserializedReq.callFrames[0].thisObj.type, "aType");
}

TEST_F(MessageTests, testResumedNotification) {
  std::string message = R"(
    {
      "method": "Debugger.resumed"
    }
  )";

  // Serialize and Deserialize are inverse functions
  JSONObject *messageJSON = mustParseStrAsJsonObj(message, factory);
  auto deserializedReq = mustMake<debugger::ResumedNotification>(messageJSON);
  EXPECT_EQ(deserializedReq.toJsonStr(), jsonValToStr(messageJSON));

  // Specifics
  EXPECT_EQ(deserializedReq.method, "Debugger.resumed");
}

} // namespace message
} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
