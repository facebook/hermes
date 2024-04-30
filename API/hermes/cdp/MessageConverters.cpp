/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MessageConverters.h"

#include <cmath>
#include <limits>

#include <hermes/cdp/JSONValueInterfaces.h>

namespace facebook {
namespace hermes {
namespace cdp {

namespace h = ::facebook::hermes;
namespace m = ::facebook::hermes::cdp::message;

m::ErrorResponse
m::makeErrorResponse(int id, m::ErrorCode code, const std::string &message) {
  m::ErrorResponse resp;
  resp.id = id;
  resp.code = static_cast<int>(code);
  resp.message = message;
  return resp;
}

m::OkResponse m::makeOkResponse(int id) {
  m::OkResponse resp;
  resp.id = id;
  return resp;
}

/*
 * debugger message conversion helpers
 */

m::debugger::Location m::debugger::makeLocation(
    const h::debugger::SourceLocation &loc) {
  m::debugger::Location result;

  result.scriptId = std::to_string(loc.fileId);
  m::setChromeLocation(result, loc);

  return result;
}

/*
 * runtime message conversion helpers
 */

m::runtime::CallFrame m::runtime::makeCallFrame(
    const h::debugger::CallFrameInfo &info) {
  m::runtime::CallFrame result;

  result.functionName = info.functionName;
  result.scriptId = std::to_string(info.location.fileId);
  result.url = info.location.fileName;
  m::setChromeLocation(result, info.location);

  return result;
}

std::vector<m::runtime::CallFrame> m::runtime::makeCallFrames(
    const facebook::hermes::debugger::StackTrace &stackTrace) {
  std::vector<m::runtime::CallFrame> result;
  result.reserve(stackTrace.callFrameCount());

  for (size_t i = 0; i < stackTrace.callFrameCount(); i++) {
    h::debugger::CallFrameInfo info = stackTrace.callFrameForIndex(i);

    // @cdp This is not explicitly documented for Runtime.CallFrame in the
    // protocol spec, but there are different behaviors that Chrome expects in
    // different situations.
    //
    // For the Profiling scenarios, Chrome itself uses Runtime.CallFrame and
    // just use the functionName field as a display string. So for Profiling,
    // native frames and internal details are included. However, for any other
    // situation, Chrome DevTools expects there to be no native frames. This is
    // filtered out by V8 in StackFrameBuilder as it visits each frame:
    // https://source.chromium.org/chromium/chromium/src/+/main:v8/src/execution/isolate.cc;l=1439;drc=6a1467666bf8f85bb49fe3d37fce5eba67763061
    //
    // This function makeCallFrames() is not used by the Profiling code to
    // generate Runtime.CallFrame, so we should filter out all non-user
    // JavaScript frames to match Chrome's expectation.
    if (info.location.fileId ==
        ::facebook::hermes::debugger::kInvalidLocation) {
      continue;
    }

    result.emplace_back(makeCallFrame(info));
  }

  return result;
}

std::unique_ptr<m::heapProfiler::SamplingHeapProfile>
m::heapProfiler::makeSamplingHeapProfile(const std::string &value) {
  // We are fine with this JSONObject becoming invalid after this function
  // exits, so we declare a local factory.
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  std::optional<JSONObject *> json = parseStrAsJsonObj(value, factory);
  if (!json) {
    return nullptr;
  }
  return m::heapProfiler::SamplingHeapProfile::tryMake(*json);
}

std::unique_ptr<m::profiler::Profile> m::profiler::makeProfile(
    const std::string &value) {
  // parseJson throws on errors, so make sure we don't crash the app
  // if somehow the sampling profiler output is borked.
  // We are fine with resp.profile becoming invalid after this function
  // exits, so we declare a local factory.
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  std::optional<JSONObject *> json = parseStrAsJsonObj(value, factory);
  if (!json) {
    return nullptr;
  }
  return m::profiler::Profile::tryMake(*json);
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
