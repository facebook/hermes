/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_PROFILEGENERATOR_H
#define HERMES_VM_PROFILER_PROFILEGENERATOR_H

#include "hermes/VM/Profiler/SamplingProfilerDefs.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/VM/Profiler/SamplingProfiler.h"

#include "llvh/ADT/DenseMap.h"

#include <tuple>
#include <utility>

namespace fhsp = ::facebook::hermes::sampling_profiler;

namespace hermes {
namespace vm {

/// Generate format-agnostic data structure, which should contain relevant
/// information about the recorded Sampling Profile and may be used by third
/// parties.
class ProfileGenerator {
  /// NativeFunctionFrameInfo is an alias for size_t, unique identifier for
  /// native function during sampling.
  using NativeFunctionNameCache =
      llvh::DenseMap<SamplingProfiler::NativeFunctionFrameInfo, std::string>;

  /// Composite key: there is no global identifier on RuntimeModule that can be
  /// used. The second argument is function identifier, not globally unique,
  /// only unique for functions inside single RuntimeModule.
  using JSFunctionFrameDetailsKey = std::pair<RuntimeModule *, uint32_t>;
  using JSFunctionFrameDetailsCacheValue = std::tuple<
      std::string, // Function name.
      std::optional<std::string>, // Source script URL.
      OptValue<hbc::DebugSourceLocation>>;
  using JSFunctionFrameDetailsCache = llvh::
      DenseMap<JSFunctionFrameDetailsKey, JSFunctionFrameDetailsCacheValue>;

 public:
  ProfileGenerator(
      const SamplingProfiler &samplingProfiler,
      const std::vector<SamplingProfiler::StackTrace> &sampledStacks);

  /// Emit Profile in a single struct.
  fhsp::Profile generate();

 private:
  /// Process single internal stack frame.
  /// \return public ProfileSampleCallStackFrame instance, which can be used in
  /// Profile.
  fhsp::ProfileSampleCallStackFrame processStackFrame(
      const SamplingProfiler::StackFrame &frame);

  /// Retrieves the name of a native function from a given internal stack frame.
  /// Supports memoization.
  /// \param frame Internal SamplingProfiler::StackFrame object. Has to be
  /// either NativeFunction or FinalizableNativeFunction.
  /// \return A string representing the name of the native function associated
  /// with the provided stack frame.
  std::string getNativeFunctionName(const SamplingProfiler::StackFrame &frame);

  /// Obtains detailed information about a JavaScript function from its frame
  /// info. Supports memoization.
  /// \param frameInfo Internal SamplingProfiler::JSFunctionFrameInfo object.
  /// \return A JSFunctionFrameDetailsCacheValue tuple containing:
  /// - The function name as a string.
  /// - An optional string representing the source script URL.
  /// - An optional DebugSourceLocation object providing the source location.
  JSFunctionFrameDetailsCacheValue getJSFunctionDetails(
      const SamplingProfiler::JSFunctionFrameInfo &frameInfo);

  /// SamplingProfiler instance expected to outlive ProfileGenerator.
  const SamplingProfiler &samplingProfiler_;
  /// Sampled stacks from SamplingProfiler.
  const std::vector<SamplingProfiler::StackTrace> &sampledStacks_;

  NativeFunctionNameCache nativeFunctionNameCache_{};
  JSFunctionFrameDetailsCache jsFunctionFrameCache_{};
};

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#endif // HERMES_VM_PROFILER_PROFILEGENERATOR_H
