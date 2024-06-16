/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPJSONHELPERS_H
#define HERMES_UNITTESTS_API_CDPJSONHELPERS_H

#include <hermes/Parser/JSONParser.h>
#include <hermes/cdp/MessageTypes.h>

namespace facebook {
namespace hermes {

using namespace ::hermes::parser;
namespace m = ::facebook::hermes::cdp::message;

struct JSONScope {
  JSONScope();
  ~JSONScope();

  JSONValue *parse(const std::string &str);
  JSONObject *parseObject(const std::string &str);
  std::optional<JSONObject *> tryParseObject(const std::string &json);
  std::string getString(JSONObject *obj, std::vector<std::string> paths);
  long long getNumber(JSONObject *obj, std::vector<std::string> paths);
  bool getBoolean(JSONObject *obj, std::vector<std::string> paths);
  const JSONObject *getObject(JSONObject *obj, std::vector<std::string> paths);
  const JSONArray *getArray(JSONObject *obj, std::vector<std::string> paths);

  struct Private;
  std::unique_ptr<Private> private_;
};

struct FrameInfo {
  FrameInfo(
      const std::string &functionName,
      uint32_t lineNumber,
      uint32_t scopeCount)
      : functionName(functionName),
        lineNumberMin(lineNumber),
        lineNumberMax(lineNumber),
        scopeCount(scopeCount),
        columnNumber(debugger::kInvalidLocation) {}

  FrameInfo(
      const std::string &callFrameId,
      const std::string &functionName,
      uint32_t lineNumber,
      uint32_t scopeCount)
      : callFrameId(callFrameId),
        functionName(functionName),
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

  FrameInfo &setThisType(const std::string &type) {
    thisType = type;
    return *this;
  }

  std::optional<std::string> callFrameId;
  std::string functionName;
  uint32_t lineNumberMin;
  uint32_t lineNumberMax;
  uint32_t scopeCount;
  uint32_t columnNumber;
  std::string scriptId;
  // If set, we optionally verify the type of the 'this' object. The 'this'
  // object should always exist in Debugger.CallFrame, but it's not necessary to
  // verify it with every test.
  std::optional<std::string> thisType;
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

  explicit PropInfo() : type("<ignored>"), accessor(true) {}

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

  /// \note ignored for internal property descriptors.
  PropInfo &setConfigurable(bool configurableParam) {
    configurable = configurableParam;
    return *this;
  }

  /// \note ignored for internal property descriptors.
  PropInfo &setEnumerable(bool enumerableParam) {
    enumerable = enumerableParam;
    return *this;
  }

  /// \note ignored for internal property descriptors.
  PropInfo &setWritable(bool writableParam) {
    writable = writableParam;
    return *this;
  }

  PropInfo &setAccessor(bool accessorParam) {
    accessor = accessorParam;
    return *this;
  }

  std::string type;
  std::optional<std::string> subtype;
  std::optional<m::JSONBlob> value;
  std::optional<std::string> unserializableValue;
  bool configurable{true};
  bool enumerable{true};
  bool writable{true};
  bool accessor{false};
};

/// Ensure that \p message is a an error response with the given \p id,
/// and return the error description.
std::string ensureErrorResponse(const std::string &message, long long id);
void ensureOkResponse(const std::string &message, long long id);

void ensureNotification(const std::string &message, const std::string &method);

m::debugger::PausedNotification ensurePaused(
    const std::string &message,
    const std::string &reason,
    const std::vector<FrameInfo> &infos);

void ensureEvalResponse(
    const std::string &message,
    long long id,
    const char *expectedValue);
void ensureEvalResponse(
    const std::string &message,
    long long id,
    bool expectedValue);
void ensureEvalResponse(
    const std::string &message,
    long long id,
    int expectedValue);

std::string ensureObjectEvalResponse(const std::string &message, int id);

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

m::runtime::GetPropertiesResponse ensureProps(
    const std::string &message,
    const std::unordered_map<std::string, PropInfo> &infos,
    const std::unordered_map<std::string, PropInfo> &internalInfos);

std::string serializeRuntimeCallFunctionOnRequest(
    const m::runtime::CallFunctionOnRequest &req);
m::runtime::GetPropertiesResponse parseRuntimeGetPropertiesResponse(
    const std::string &json);

std::unordered_map<std::string, m::runtime::PropertyDescriptor> indexProps(
    const std::vector<m::runtime::PropertyDescriptor> &props);

} // namespace hermes
} // namespace facebook

#endif // HERMES_UNITTESTS_API_CDPJSONHELPERS_H
