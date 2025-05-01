/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PUBLIC_SAMPLINGPROFILER_H
#define HERMES_PUBLIC_SAMPLINGPROFILER_H

#include <hermes/Public/HermesExport.h>

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace facebook {
namespace hermes {
namespace sampling_profiler {

/// JavaScript function frame. Guaranteed to have function name, potentially
/// an empty string, if function is anonymous or if function names were filtered
/// out during bytecode compilation. Could have scriptId, url, line and column
/// numbers, if debug source location is available.
class HERMES_EXPORT ProfileSampleCallStackJSFunctionFrame {
 public:
  explicit ProfileSampleCallStackJSFunctionFrame(
      const std::string &functionName,
      const std::optional<uint32_t> &scriptId = std::nullopt,
      const std::optional<std::string> &url = std::nullopt,
      const std::optional<uint32_t> &lineNumber = std::nullopt,
      const std::optional<uint32_t> &columnNumber = std::nullopt)
      : functionName_(functionName),
        scriptId_(scriptId),
        url_(url),
        lineNumber_(lineNumber),
        columnNumber_(columnNumber) {}

  /// \return name of the function that represents call frame.
  const std::string &getFunctionName() const {
    return functionName_;
  }

  bool hasScriptId() const {
    return scriptId_.has_value();
  }

  /// \return id of the corresponding script in the VM.
  uint32_t getScriptId() const {
    return scriptId_.value();
  }

  bool hasUrl() const {
    return url_.has_value();
  }

  /// \return source url of the corresponding script in the VM.
  const std::string &getUrl() const {
    return url_.value();
  }

  bool hasLineNumber() const {
    return lineNumber_.has_value();
  }

  /// \return 1-based line number of the corresponding call frame.
  uint32_t getLineNumber() const {
    return lineNumber_.value();
  }

  bool hasColumnNumber() const {
    return columnNumber_.has_value();
  }

  /// \return 1-based column number of the corresponding call frame.
  uint32_t getColumnNumber() const {
    return columnNumber_.value();
  }

 private:
  std::string functionName_;
  std::optional<uint32_t> scriptId_;
  std::optional<std::string> url_;
  std::optional<uint32_t> lineNumber_;
  std::optional<uint32_t> columnNumber_;
};

/// Native (Hermes) function frame. Example: implementation of a built-in
/// Array.prototype.map.
class HERMES_EXPORT ProfileSampleCallStackNativeFunctionFrame {
 public:
  explicit ProfileSampleCallStackNativeFunctionFrame(
      const std::string &functionName)
      : functionName_(functionName) {}

  /// \return name of the function that represents call frame.
  const std::string &getFunctionName() const {
    return functionName_;
  }

 private:
  std::string functionName_;
};

/// Host function frame. Native functions defined by the integrator. Example:
/// for React Native, this could be performance.measure or console.log.
class HERMES_EXPORT ProfileSampleCallStackHostFunctionFrame {
 public:
  explicit ProfileSampleCallStackHostFunctionFrame(
      const std::string &functionName)
      : functionName_(functionName) {}

  /// \return name of the function that represents call frame.
  const std::string &getFunctionName() const {
    return functionName_;
  }

 private:
  std::string functionName_;
};

/// Frame that suspends the execution of the VM: could be GC, Debugger or
/// combination of them.
class HERMES_EXPORT ProfileSampleCallStackSuspendFrame {
 public:
  /// Subtype of the Suspend frame.
  enum class SuspendFrameKind {
    GC, /// Frame that suspends the execution of the VM due to GC.
    Debugger, /// Frame that suspends the execution of the VM due to debugger.
    Multiple, /// Multiple suspensions have occurred.
  };

  explicit ProfileSampleCallStackSuspendFrame(
      const SuspendFrameKind suspendFrameKind)
      : suspendFrameKind_(suspendFrameKind) {}

  /// \return subtype of the suspend frame.
  SuspendFrameKind getSuspendFrameKind() const {
    return suspendFrameKind_;
  }

 private:
  SuspendFrameKind suspendFrameKind_;
};

/// Variant of all possible call stack frames options.
using ProfileSampleCallStackFrame = std::variant<
    ProfileSampleCallStackSuspendFrame,
    ProfileSampleCallStackNativeFunctionFrame,
    ProfileSampleCallStackHostFunctionFrame,
    ProfileSampleCallStackJSFunctionFrame>;

/// A pair of a timestamp and a snapshot of the call stack at this point in
/// time.
class HERMES_EXPORT ProfileSample {
 public:
  ProfileSample(
      uint64_t timestamp,
      uint64_t threadId,
      std::vector<ProfileSampleCallStackFrame> callStack)
      : timestamp_(timestamp),
        threadId_(threadId),
        callStack_(std::move(callStack)) {}

  /// \return serialized unix timestamp in microseconds granularity. The
  /// moment when this sample was recorded.
  uint64_t getTimestamp() const {
    return timestamp_;
  }

  /// \return thread id where sample was recorded.
  uint64_t getThreadId() const {
    return threadId_;
  }

  /// \return a snapshot of the call stack. The first element of the vector is
  /// the lowest frame in the stack.
  const std::vector<ProfileSampleCallStackFrame> &getCallStack() const {
    return callStack_;
  }

 private:
  /// When the call stack snapshot was taken (μs).
  uint64_t timestamp_;
  /// Thread id where sample was recorded.
  uint64_t threadId_;
  /// Snapshot of the call stack. The first element of the vector is
  /// the lowest frame in the stack.
  std::vector<ProfileSampleCallStackFrame> callStack_;
};

/// Contains relevant information about the sampled trace from start to finish.
class HERMES_EXPORT Profile {
 public:
  explicit Profile(std::vector<ProfileSample> samples)
      : samples_(std::move(samples)) {}

  /// \return list of recorded samples, should be chronologically sorted.
  const std::vector<ProfileSample> &getSamples() const {
    return samples_;
  }

 private:
  /// List of recorded samples, should be chronologically sorted.
  std::vector<ProfileSample> samples_;
};

} // namespace sampling_profiler
} // namespace hermes
} // namespace facebook

#endif
