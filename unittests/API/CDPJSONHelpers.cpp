/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include <hermes/DebuggerAPI.h>
#include <hermes/Support/ErrorHandling.h>
#include <hermes/cdp/MessageTypesInlines.h>

#include "CDPJSONHelpers.h"
#include "CDPTestHelpers.h"

using namespace facebook::hermes::cdp;
using namespace hermes::parser;

namespace facebook {
namespace hermes {

std::string ensureErrorResponse(const std::string &message, long long id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<m::ErrorResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
  return response.message;
}

void ensureOkResponse(const std::string &message, long long id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<m::OkResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
}

static const JSONValue *getJSONValue(
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

    EXPECT_TRUE(value != nullptr);
  }

  return value;
}

template <typename T>
std::unique_ptr<T> getValue(
    const JSONValue *value,
    std::vector<std::string> paths) {
  value = getJSONValue(value, paths);
  std::unique_ptr<T> target = m::valueFromJson<T>(value);
  return target;
}

static const JSONObject *getJSONObject(
    const JSONValue *value,
    std::vector<std::string> paths) {
  value = getJSONValue(value, paths);
  EXPECT_TRUE(JSONObject::classof(value));
  return static_cast<const JSONObject *>(value);
}

static const JSONArray *getJSONArray(
    const JSONValue *value,
    std::vector<std::string> paths) {
  value = getJSONValue(value, paths);
  EXPECT_TRUE(JSONArray::classof(value));
  return static_cast<const JSONArray *>(value);
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

static void expectCallFrames(
    const std::vector<m::debugger::CallFrame> &frames,
    const std::vector<FrameInfo> &infos) {
  EXPECT_EQ(frames.size(), infos.size());

  int i = 0;
  for (const FrameInfo &info : infos) {
    const m::debugger::CallFrame &frame = frames[i];

    if (info.callFrameId.has_value()) {
      EXPECT_EQ(frame.callFrameId, info.callFrameId.value());
    } else {
      EXPECT_EQ(frame.callFrameId, std::to_string(i));
    }
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

    if (info.thisType.has_value()) {
      EXPECT_EQ(frame.thisObj.type, info.thisType.value());
    }

    i++;
  }
}

m::debugger::PausedNotification ensurePaused(
    const std::string &message,
    const std::string &reason,
    const std::vector<FrameInfo> &infos) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto notification = mustMake<m::debugger::PausedNotification>(
      mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(notification.reason, reason);
  expectCallFrames(notification.callFrames, infos);
  return notification;
}

m::runtime::GetPropertiesResponse ensureProps(
    const std::string &message,
    const std::unordered_map<std::string, PropInfo> &infos,
    const std::unordered_map<std::string, PropInfo> &internalInfos) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::runtime::GetPropertiesResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.result.size(), infos.size());

  if (internalInfos.size() > 0) {
    EXPECT_TRUE(resp.internalProperties.has_value());
    EXPECT_EQ(resp.internalProperties.value().size(), internalInfos.size());
  } else {
    EXPECT_TRUE(
        !resp.internalProperties.has_value() ||
        resp.internalProperties.value().size() == 0);
  }

  for (size_t i = 0; i < resp.result.size(); i++) {
    m::runtime::PropertyDescriptor &desc = resp.result[i];

    auto infoIt = infos.find(desc.name);
    EXPECT_FALSE(infoIt == infos.end()) << desc.name;

    if (infoIt != infos.end()) {
      const PropInfo &info = infoIt->second;

      if (!info.accessor) {
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
        }
      }

      EXPECT_EQ(desc.configurable, info.configurable);
      EXPECT_EQ(desc.enumerable, info.enumerable);
      if (info.accessor) {
        EXPECT_EQ(desc.writable, std::nullopt);
      } else {
        EXPECT_EQ(desc.writable, info.writable);
      }
    }
  }

  if (resp.internalProperties.has_value()) {
    for (size_t i = 0; i < resp.internalProperties->size(); i++) {
      m::runtime::InternalPropertyDescriptor &desc =
          resp.internalProperties.value()[i];

      auto infoIt = internalInfos.find(desc.name);
      EXPECT_FALSE(infoIt == internalInfos.end()) << desc.name;

      if (infoIt != internalInfos.end()) {
        const PropInfo &info = infoIt->second;

        EXPECT_FALSE(info.accessor)
            << "internal properties can't have accessor descriptors";

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
      }
    }
  }

  return resp;
}

std::unordered_map<std::string, m::runtime::PropertyDescriptor> indexProps(
    const std::vector<m::runtime::PropertyDescriptor> &props) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  std::unordered_map<std::string, m::runtime::PropertyDescriptor> indexedProps;
  for (const auto &prop : props) {
    EXPECT_FALSE(indexedProps.count(prop.name))
        << "Duplicate property name: " << prop.name;
    indexedProps[prop.name] = clone(prop, factory);
  }
  return indexedProps;
}

void ensureEvalResponse(
    const std::string &message,
    long long id,
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
    long long id,
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

void ensureEvalResponse(
    const std::string &message,
    long long id,
    int expectedValue) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "number");
  EXPECT_EQ(std::stoi(*resp.result.value), expectedValue);
  EXPECT_FALSE(resp.exceptionDetails.has_value());
}

std::string ensureObjectEvalResponse(const std::string &message, int id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::EvaluateOnCallFrameResponse>(
      mustParseStrAsJsonObj(message, factory));

  EXPECT_EQ(resp.id, id);
  EXPECT_EQ(resp.result.type, "object");
  EXPECT_FALSE(resp.exceptionDetails.has_value());

  EXPECT_TRUE(resp.result.objectId.has_value());
  EXPECT_TRUE(resp.result.preview.has_value());
  EXPECT_EQ(resp.result.preview->type, "object");

  return resp.result.objectId.value();
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
  ASSERT_TRUE(resp.exceptionDetails.has_value());

  m::runtime::ExceptionDetails &details = resp.exceptionDetails.value();
  EXPECT_EQ(details.text, exceptionText);

  ASSERT_TRUE(details.exception.has_value());
  EXPECT_TRUE(details.exception->objectId.has_value());

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

  EXPECT_TRUE(resp.result.objectId.has_value());
}

m::debugger::BreakpointId ensureSetBreakpointResponse(
    const std::string &message,
    int id,
    BreakpointLocation location) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::SetBreakpointResponse>(
      mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(resp.id, id);
  EXPECT_FALSE(resp.breakpointId.empty());
  EXPECT_NE(
      resp.breakpointId,
      std::to_string(facebook::hermes::debugger::kInvalidBreakpoint));
  if (location.scriptId) {
    EXPECT_EQ(resp.actualLocation.scriptId, location.scriptId);
  }
  EXPECT_EQ(resp.actualLocation.lineNumber, location.lineNumber);
  EXPECT_EQ(resp.actualLocation.columnNumber.value(), location.columnNumber);
  return resp.breakpointId;
}

m::debugger::BreakpointId ensureSetBreakpointByUrlResponse(
    const std::string &message,
    int id,
    std::vector<BreakpointLocation> locations) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto resp = mustMake<m::debugger::SetBreakpointByUrlResponse>(
      mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(resp.id, id);
  EXPECT_FALSE(resp.breakpointId.empty());
  EXPECT_NE(
      resp.breakpointId,
      std::to_string(facebook::hermes::debugger::kInvalidBreakpoint));
  EXPECT_EQ(resp.locations.size(), locations.size());
  for (size_t i = 0; i != locations.size(); i++) {
    if (locations[i].scriptId) {
      EXPECT_EQ(resp.locations[i].scriptId, locations[i].scriptId);
    }
    EXPECT_EQ(resp.locations[i].lineNumber, locations[i].lineNumber);
    if (locations[i].columnNumber) {
      EXPECT_EQ(
          resp.locations[i].columnNumber.value(), locations[i].columnNumber);
    }
  }
  return resp.breakpointId;
}

std::string serializeRuntimeCallFunctionOnRequest(
    const m::runtime::CallFunctionOnRequest &req) {
  return req.toJsonStr();
}

m::runtime::GetPropertiesResponse parseRuntimeGetPropertiesResponse(
    const std::string &json) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  return mustMake<m::runtime::GetPropertiesResponse>(
      mustParseStrAsJsonObj(json, factory));
}

struct JSONScope::Private {
  Private() : allocator(), factory(allocator) {}

  JSLexer::Allocator allocator;
  JSONFactory factory;
};

JSONScope::JSONScope() : private_(std::make_unique<Private>()) {}

JSONScope::~JSONScope() {}

JSONValue *JSONScope::parse(const std::string &json) {
  return mustParseStr(json, private_->factory);
}

JSONObject *JSONScope::parseObject(const std::string &json) {
  return mustParseStrAsJsonObj(json, private_->factory);
}

std::optional<JSONObject *> JSONScope::tryParseObject(const std::string &json) {
  return parseStrAsJsonObj(json, private_->factory);
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

bool JSONScope::getBoolean(JSONObject *obj, std::vector<std::string> paths) {
  return *getValue<bool>(obj, paths);
}

const JSONObject *JSONScope::getObject(
    JSONObject *obj,
    std::vector<std::string> paths) {
  return getJSONObject(obj, paths);
}

const JSONArray *JSONScope::getArray(
    JSONObject *obj,
    std::vector<std::string> paths) {
  return getJSONArray(obj, paths);
}

} // namespace hermes
} // namespace facebook

#endif // HERMES_ENABLE_DEBUGGER
