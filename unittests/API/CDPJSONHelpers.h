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

struct JSONScope {
  JSONScope();
  ~JSONScope();

  JSONObject *parseObject(const std::string &str);
  std::string getString(JSONObject *obj, std::vector<std::string> paths);
  long long getNumber(JSONObject *obj, std::vector<std::string> paths);

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

template <typename T>
std::unique_ptr<T> getValue(
    const std::string &message,
    std::vector<std::string> paths);

void ensureErrorResponse(const std::string &message, int id);
void ensureOkResponse(const std::string &message, int id);

void ensureNotification(const std::string &message, const std::string &method);

void ensurePaused(
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

void ensureSetBreakpointResponse(
    const std::string &message,
    int id,
    const std::string &scriptID,
    long long lineNumber,
    long long columnNumber);

} // namespace hermes
} // namespace facebook

#endif // HERMES_UNITTESTS_API_CDPJSONHELPERS_H
