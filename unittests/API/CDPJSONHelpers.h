/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPJSONHELPERS_H
#define HERMES_UNITTESTS_API_CDPJSONHELPERS_H

#include <hermes/Parser/JSONParser.h>

namespace facebook {
namespace hermes {

using namespace ::hermes::parser;
namespace m = ::facebook::hermes::inspector_modern::chrome::message;

struct JSONScope {
  JSONScope();
  ~JSONScope();

  JSONObject *parseObject(const std::string &str);
  std::string getString(JSONObject *obj, std::vector<std::string> paths);
  long long getNumber(JSONObject *obj, std::vector<std::string> paths);
  bool getBoolean(JSONObject *obj, std::vector<std::string> paths);
  const JSONObject *getObject(JSONObject *obj, std::vector<std::string> paths);
  const JSONArray *getArray(JSONObject *obj, std::vector<std::string> paths);

  struct Private;
  std::unique_ptr<Private> private_;
};

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

struct BreakpointLocation {
  BreakpointLocation(long long lineNumber)
      : scriptId(std::nullopt),
        lineNumber(lineNumber),
        columnNumber(std::nullopt) {}

  BreakpointLocation(long long lineNumber, long long columnNumber)
      : scriptId(std::nullopt),
        lineNumber(lineNumber),
        columnNumber(columnNumber) {}

  BreakpointLocation(
      std::string scriptId,
      long long lineNumber,
      long long columnNumber)
      : scriptId(scriptId),
        lineNumber(lineNumber),
        columnNumber(columnNumber) {}

  std::optional<std::string> scriptId;
  long long lineNumber;
  std::optional<long long> columnNumber;
};

template <typename T>
std::unique_ptr<T> getValue(
    const std::string &message,
    std::vector<std::string> paths);

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
  std::optional<m::JSONBlob> value;
  std::optional<std::string> unserializableValue;
};

void ensureErrorResponse(const std::string &message, int id);
void ensureOkResponse(const std::string &message, int id);

void ensureNotification(const std::string &message, const std::string &method);

m::debugger::PausedNotification ensurePaused(
    const std::string &message,
    const std::string &reason,
    const std::vector<FrameInfo> &infos);

void ensureEvalResponse(
    const std::string &message,
    int id,
    const char *expectedValue);
void ensureEvalResponse(const std::string &message, int id, bool expectedValue);
void ensureEvalResponse(const std::string &message, int id, int expectedValue);

void ensureEvalException(
    const std::string &message,
    int id,
    const std::string &exceptionText,
    const std::vector<FrameInfo> infos);

m::debugger::BreakpointId ensureSetBreakpointResponse(
    const std::string &message,
    int id,
    BreakpointLocation location);

m::debugger::BreakpointId ensureSetBreakpointByUrlResponse(
    const std::string &message,
    int id,
    std::vector<BreakpointLocation> locations);

std::unordered_map<std::string, std::string> ensureProps(
    const std::string &message,
    const std::unordered_map<std::string, PropInfo> &infos);
} // namespace hermes
} // namespace facebook

#endif // HERMES_UNITTESTS_API_CDPJSONHELPERS_H
