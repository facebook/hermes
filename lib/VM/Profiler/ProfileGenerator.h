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
#include "llvh/ADT/StringRef.h"

#include <memory>
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
  using NativeFunctionNameCache = llvh::
      DenseMap<SamplingProfiler::NativeFunctionFrameInfo, fhsp::StringEntry>;

  /// Composite key: there is no global identifier on RuntimeModule that can be
  /// used. The second argument is function identifier, not globally unique,
  /// only unique for functions inside single RuntimeModule.
  using JSFunctionFrameDetailsKey = std::pair<RuntimeModule *, uint32_t>;
  using JSFunctionFrameDetailsCacheValue = std::tuple<
      fhsp::StringEntry, // Function name.
      std::optional<fhsp::StringEntry>, // Source script URL.
      OptValue<hbc::DebugSourceLocation>>;
  using JSFunctionFrameDetailsCache = llvh::
      DenseMap<JSFunctionFrameDetailsKey, JSFunctionFrameDetailsCacheValue>;

  /// Cache for source script URLs. The key is llvh::StringRef from original
  /// std::string, the value is StringEntry that can be supplied to Frame.
  using SourceScriptURLCache =
      llvh::DenseMap<llvh::StringRef, fhsp::StringEntry>;

 public:
  ProfileGenerator(
      const SamplingProfiler &samplingProfiler,
      const std::vector<SamplingProfiler::StackTrace> &sampledStacks);

  ProfileGenerator(const ProfileGenerator &) = delete;
  ProfileGenerator &operator=(const ProfileGenerator &) = delete;

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
  /// \return A StringEntry object representing the name of the native function
  /// associated with the provided stack frame.
  fhsp::StringEntry getNativeFunctionName(
      const SamplingProfiler::StackFrame &frame);

  /// Obtains detailed information about a JavaScript function from its frame
  /// info. Supports memoization.
  /// \param frameInfo Internal SamplingProfiler::JSFunctionFrameInfo object.
  /// \return A JSFunctionFrameDetailsCacheValue tuple containing:
  /// - The function name as a StringEntry object.
  /// - An optional StringEntry object representing the source script URL.
  /// - An optional DebugSourceLocation object providing the source location.
  JSFunctionFrameDetailsCacheValue getJSFunctionDetails(
      const SamplingProfiler::JSFunctionFrameInfo &frameInfo);

  /// Places a std::string into stringStorage_.
  /// \return StringEntry that can be supplied to Frame.
  fhsp::StringEntry storeString(const std::string &str);

  /// SamplingProfiler instance expected to outlive ProfileGenerator.
  const SamplingProfiler &samplingProfiler_;
  /// Sampled stacks from SamplingProfiler.
  const std::vector<SamplingProfiler::StackTrace> &sampledStacks_;

  NativeFunctionNameCache nativeFunctionNameCache_{};
  JSFunctionFrameDetailsCache jsFunctionFrameCache_{};
  SourceScriptURLCache sourceScriptURLCache_{};

  /// Container for all strings inside the profile that is currently
  /// being constructed. It will be owned by the profile later. There could be
  /// duplicates in this storage: the uniqueness of frames is determined by
  /// internal VM concepts, not by the names of functions and string contents.
  std::unique_ptr<std::vector<std::string>> stringStorage_;
};

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#endif // HERMES_VM_PROFILER_PROFILEGENERATOR_H
