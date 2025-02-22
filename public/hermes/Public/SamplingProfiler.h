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
#include <vector>

namespace facebook {
namespace hermes {
namespace sampling_profiler {

/// Represents a single frame inside the captured sample stack.
/// Base struct for different kinds of frames.
struct HERMES_EXPORT ProfileSampleCallStackFrame {
  /// Represents type of frame inside of recorded call stack.
  enum class Kind {
    JSFunction, /// JavaScript function frame.
    NativeFunction, /// Native built-in functions, like arrayPrototypeMap.
    HostFunction, /// Native functions, defined by Host, a.k.a. Host functions.
    Suspend, /// Frame that suspends the execution of the VM: GC or Debugger.
  };

 public:
  explicit ProfileSampleCallStackFrame(const Kind kind) : kind_(kind) {}

  /// \return type of the call stack frame.
  Kind getKind() const {
    return kind_;
  }

 private:
  Kind kind_;
};

/// Extends ProfileSampleCallStackFrame with an information about JavaScript
/// function frame: function name, and possibly scriptId, url, line and column
/// numbers.
struct HERMES_EXPORT ProfileSampleCallStackJSFunctionFrame
    : public ProfileSampleCallStackFrame {
  explicit ProfileSampleCallStackJSFunctionFrame(
      const std::string &functionName,
      const std::optional<uint32_t> &scriptId = std::nullopt,
      const std::optional<std::string> &url = std::nullopt,
      const std::optional<uint32_t> &lineNumber = std::nullopt,
      const std::optional<uint32_t> &columnNumber = std::nullopt)
      : ProfileSampleCallStackFrame(
            ProfileSampleCallStackFrame::Kind::JSFunction),
        functionName_(functionName),
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

/// Extends ProfileSampleCallStackFrame with a function name.
struct HERMES_EXPORT ProfileSampleCallStackNativeFunctionFrame
    : public ProfileSampleCallStackFrame {
 public:
  explicit ProfileSampleCallStackNativeFunctionFrame(
      const std::string &functionName)
      : ProfileSampleCallStackFrame(
            ProfileSampleCallStackFrame::Kind::NativeFunction),
        functionName_(functionName) {}

  /// \return name of the function that represents call frame.
  const std::string &getFunctionName() const {
    return functionName_;
  }

 private:
  std::string functionName_;
};

/// Extends ProfileSampleCallStackFrame with a function name.
struct HERMES_EXPORT ProfileSampleCallStackHostFunctionFrame
    : public ProfileSampleCallStackFrame {
 public:
  explicit ProfileSampleCallStackHostFunctionFrame(
      const std::string &functionName)
      : ProfileSampleCallStackFrame(
            ProfileSampleCallStackFrame::Kind::HostFunction),
        functionName_(functionName) {}

  /// \return name of the function that represents call frame.
  const std::string &getFunctionName() const {
    return functionName_;
  }

 private:
  std::string functionName_;
};

/// Extends ProfileSampleCallStackFrame with a suspend frame information.
struct HERMES_EXPORT ProfileSampleCallStackSuspendFrame
    : public ProfileSampleCallStackFrame {
  /// Subtype of the Suspend frame.
  enum class SuspendFrameKind {
    GC, /// Frame that suspends the execution of the VM due to GC.
    Debugger, /// Frame that suspends the execution of the VM due to debugger.
    Multiple, /// Multiple suspensions have occurred.
  };

 public:
  explicit ProfileSampleCallStackSuspendFrame(
      const SuspendFrameKind suspendFrameKind)
      : ProfileSampleCallStackFrame(ProfileSampleCallStackFrame::Kind::Suspend),
        suspendFrameKind_(suspendFrameKind) {}

  /// \return subtype of the suspend frame.
  SuspendFrameKind getSuspendFrameKind() const {
    return suspendFrameKind_;
  }

 private:
  SuspendFrameKind suspendFrameKind_;
};

/// A pair of a timestamp and a snapshot of the call stack at this point in
/// time.
struct HERMES_EXPORT ProfileSample {
 public:
  ProfileSample(
      uint64_t timestamp,
      uint64_t threadId,
      std::vector<ProfileSampleCallStackFrame *> callStack)
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
  const std::vector<ProfileSampleCallStackFrame *> &getCallStack() const {
    return callStack_;
  }

 private:
  /// When the call stack snapshot was taken (μs).
  uint64_t timestamp_;
  /// Thread id where sample was recorded.
  uint64_t threadId_;
  /// Snapshot of the call stack. The first element of the vector is
  /// the lowest frame in the stack.
  std::vector<ProfileSampleCallStackFrame *> callStack_;
};

/// Contains relevant information about the sampled trace from start to finish.
struct HERMES_EXPORT Profile {
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
