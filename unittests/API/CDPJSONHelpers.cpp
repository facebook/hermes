/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/DebuggerAPI.h>
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
std::unique_ptr<T> getValue(JSONValue *value, std::vector<std::string> paths) {
  int numPaths = paths.size();
  for (int i = 0; i < numPaths; i++) {
    EXPECT_TRUE(JSONObject::classof(value));
    value = static_cast<JSONObject *>(value)->get(paths[i]);

    if (i != numPaths - 1) {
      EXPECT_TRUE(value != nullptr);
    } else {
      std::unique_ptr<T> target = m::valueFromJson<T>(value);
      return std::move(target);
    }
  }
  // Should never reach here
  EXPECT_TRUE(false);
  return nullptr;
}

void ensureResponse(
    const std::string &message,
    const std::string &expectedMethod,
    int expectedID) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);

  EXPECT_EQ(*getValue<std::string>(obj, {"method"}), expectedMethod);
  EXPECT_EQ(*getValue<double>(obj, {"id"}), expectedID);
}

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
