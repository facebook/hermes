/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/DebuggerAPI.h>
#include <hermes/Support/ErrorHandling.h>
#include <hermes/inspector/chrome/MessageTypesInlines.h>
#include <hermes/inspector/chrome/tests/TestHelpers.h>

#include "CDPJSONHelpers.h"

using namespace facebook::hermes::inspector_modern::chrome;
using namespace hermes::parser;

namespace m = ::facebook::hermes::inspector_modern::chrome::message;

namespace facebook {
namespace hermes {

void ensureErrorResponse(const std::string &message, int id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<m::ErrorResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
}

void ensureOkResponse(const std::string &message, int id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<m::OkResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
}

template <typename T>
std::unique_ptr<T> getValue(
    const JSONValue *value,
    std::vector<std::string> paths) {
  int numPaths = paths.size();
  for (int i = 0; i < numPaths; i++) {
    if (JSONObject::classof(value)) {
      value = static_cast<const JSONObject *>(value)->get(paths[i]);
    } else if (JSONArray::classof(value)) {
      value = static_cast<const JSONArray *>(value)->at(std::stoi(paths[i]));
    } else {
      EXPECT_TRUE(false);
    }

    if (i != numPaths - 1) {
      EXPECT_TRUE(value != nullptr);
    } else {
      std::unique_ptr<T> target = m::valueFromJson<T>(value);
      return std::move(target);
    }
  }
  ::hermes::hermes_fatal("Should never reach here");
}

template <typename T>
std::unique_ptr<T> getValue(
    const std::string &message,
    std::vector<std::string> paths) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);

  return getValue<T>(obj, paths);
}
template std::unique_ptr<std::string> getValue(
    const std::string &message,
    std::vector<std::string> paths);

void ensureNotification(
    const std::string &message,
    const std::string &expectedMethod) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);

  EXPECT_EQ(*getValue<std::string>(obj, {"method"}), expectedMethod);
}

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

void ensurePaused(
    const std::string &message,
    const std::string &reason,
    const std::vector<FrameInfo> &infos) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto notification = mustMake<m::debugger::PausedNotification>(
      mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(notification.reason, reason);
  expectCallFrames(notification.callFrames, infos);
}

void ensureEvalResponse(
    const std::string &message,
    int id,
    const char *expectedValue) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "string");
  // The resp.result.value is a JSON blob, so it contains the surrounding
  // quotes. We need to add these quotes to the expected string value.
  EXPECT_EQ(*resp.result.value, "\"" + std::string(expectedValue) + "\"");
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void ensureEvalResponse(
    const std::string &message,
    int id,
    bool expectedValue) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "boolean");
  EXPECT_EQ(((*resp.result.value) == "true" ? true : false), expectedValue);
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void ensureEvalResponse(const std::string &message, int id, int expectedValue) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "number");
  EXPECT_EQ(std::stoi(*resp.result.value), expectedValue);
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

void ensureEvalException(
    const std::string &message,
    int id,
    const std::string &exceptionText,
    const std::vector<FrameInfo> infos) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

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

void ensureSetBreakpointResponse(
    const std::string &message,
    int id,
    const std::string &scriptID,
    long long lineNumber,
    long long columnNumber) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::SetBreakpointResponse>(
      mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.actualLocation.scriptId, scriptID);
  EXPECT_EQ(resp.actualLocation.lineNumber, lineNumber);
  EXPECT_EQ(resp.actualLocation.columnNumber.value(), columnNumber);
}

struct JSONScope::Private {
  Private() : allocator(), factory(allocator) {}

  JSLexer::Allocator allocator;
  JSONFactory factory;
};

JSONScope::JSONScope() : private_(std::make_unique<Private>()) {}

JSONScope::~JSONScope() {}

JSONObject *JSONScope::parseObject(const std::string &json) {
  return mustParseStrAsJsonObj(json, private_->factory);
}

std::string JSONScope::getString(
    JSONObject *obj,
    std::vector<std::string> paths) {
  return *getValue<std::string>(obj, paths);
}

long long JSONScope::getNumber(
    JSONObject *obj,
    std::vector<std::string> paths) {
  return *getValue<long long>(obj, paths);
}

} // namespace hermes
} // namespace facebook
