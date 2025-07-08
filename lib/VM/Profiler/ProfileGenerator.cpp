/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProfileGenerator.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/StringRef.h"

#include <memory>
#include <tuple>
#include <utility>

namespace fhsp = ::facebook::hermes::sampling_profiler;

namespace hermes {
namespace vm {

namespace {

/// \return timestamp as time since epoch in microseconds.
static uint64_t convertTimestampToMicroseconds(
    SamplingProfiler::TimeStampType timeStamp) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             timeStamp.time_since_epoch())
      .count();
}

static std::string getJSFunctionName(
    hbc::BCProvider *bcProvider,
    uint32_t funcId) {
  hbc::RuntimeFunctionHeader functionHeader =
      bcProvider->getFunctionHeader(funcId);
  return bcProvider->getStringRefFromID(functionHeader.functionName()).str();
}

static OptValue<hbc::DebugSourceLocation> getFunctionDefinitionSourceLocation(
    hbc::BCProvider *bcProvider,
    uint32_t funcId) {
  const hbc::DebugOffsets *debugOffsets = bcProvider->getDebugOffsets(funcId);
  if (debugOffsets &&
      debugOffsets->sourceLocations != hbc::DebugOffsets::NO_OFFSET) {
    // 0-offset is specified to get the location of the function definition, the
    // start of it.
    return bcProvider->getDebugInfo()->getLocationForAddress(
        debugOffsets->sourceLocations, 0 /* opcodeOffset */);
  }
  return llvh::None;
}

static fhsp::ProfileSampleCallStackSuspendFrame::SuspendFrameKind
formatSuspendFrameKind(SamplingProfiler::SuspendFrameInfo::Kind kind) {
  switch (kind) {
    case SamplingProfiler::SuspendFrameInfo::Kind::GC:
      return fhsp::ProfileSampleCallStackSuspendFrame::SuspendFrameKind::GC;
    case SamplingProfiler::SuspendFrameInfo::Kind::Debugger:
      return fhsp::ProfileSampleCallStackSuspendFrame::SuspendFrameKind::
          Debugger;
    case SamplingProfiler::SuspendFrameInfo::Kind::Multiple:
      return fhsp::ProfileSampleCallStackSuspendFrame::SuspendFrameKind::
          Multiple;

    default:
      llvm_unreachable("Unexpected Suspend Frame kind");
  }
}

class ProfileGenerator {
  /// NativeFunctionFrameInfo is an alias for size_t, unique identifier for
  /// native function during sampling.
  using NativeFunctionNameCache = llvh::
      DenseMap<SamplingProfiler::NativeFunctionFrameInfo, std::string_view>;

  /// Composite key: there is no global identifier on RuntimeModule that can be
  /// used. The second argument is function identifier, not globally unique,
  /// only unique for functions inside single RuntimeModule.
  using JSFunctionFrameDetailsKey = std::pair<RuntimeModule *, uint32_t>;
  using JSFunctionFrameDetailsCacheValue = std::tuple<
      std::string_view, // Function name.
      std::optional<std::string_view>, // Source script URL.
      OptValue<hbc::DebugSourceLocation>>;
  using JSFunctionFrameDetailsCache = llvh::
      DenseMap<JSFunctionFrameDetailsKey, JSFunctionFrameDetailsCacheValue>;

  /// Cache for source script URLs. The key is llvh::StringRef, the value is the
  /// std::string_view that points to the raw std::string inside stringStorage_,
  /// which guarantees the validity of the view as long as the storage is not
  /// deallocated.
  using SourceScriptURLCache =
      llvh::DenseMap<llvh::StringRef, std::string_view>;

 public:
  ProfileGenerator(
      const SamplingProfiler &samplingProfiler,
      const std::vector<SamplingProfiler::StackTrace> &sampledStacks)
      : samplingProfiler_(samplingProfiler), sampledStacks_(sampledStacks) {}

  ProfileGenerator(const ProfileGenerator &) = delete;
  ProfileGenerator &operator=(const ProfileGenerator &) = delete;

  /// Emit Profile in a single struct.
  fhsp::Profile generate() {
    stringStorage_ = std::make_unique<std::deque<std::string>>();

    std::vector<fhsp::ProfileSample> samples;
    samples.reserve(sampledStacks_.size());
    for (const SamplingProfiler::StackTrace &sampledStack : sampledStacks_) {
      uint64_t timestamp =
          convertTimestampToMicroseconds(sampledStack.timeStamp);

      std::vector<fhsp::ProfileSampleCallStackFrame> callFrames;
      callFrames.reserve(sampledStack.stack.size());
      for (const SamplingProfiler::StackFrame &frame : sampledStack.stack) {
        callFrames.push_back(processStackFrame(frame));
      }

      samples.emplace_back(timestamp, sampledStack.tid, std::move(callFrames));
    }

    return fhsp::Profile(std::move(samples), std::move(stringStorage_));
  }

 private:
  /// Process single internal stack frame.
  /// \return public ProfileSampleCallStackFrame instance, which can be used in
  /// Profile.
  fhsp::ProfileSampleCallStackFrame processStackFrame(
      const SamplingProfiler::StackFrame &frame) {
    switch (frame.kind) {
      case SamplingProfiler::StackFrame::FrameKind::SuspendFrame:
        return fhsp::ProfileSampleCallStackSuspendFrame(
            formatSuspendFrameKind(frame.suspendFrame.kind));

      case SamplingProfiler::StackFrame::FrameKind::NativeFunction:
        return fhsp::ProfileSampleCallStackNativeFunctionFrame(
            getNativeFunctionName(frame));

      case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction:
        return fhsp::ProfileSampleCallStackHostFunctionFrame(
            getNativeFunctionName(frame));

      case SamplingProfiler::StackFrame::FrameKind::JSFunction: {
        RuntimeModule *module = frame.jsFrame.module;
        auto [functionName, scriptURL, sourceLocOpt] =
            getJSFunctionDetails(frame.jsFrame);

        uint32_t scriptId = module->getScriptID();
        std::optional<uint32_t> lineNumber = std::nullopt;
        std::optional<uint32_t> columnNumber = std::nullopt;

        if (sourceLocOpt.hasValue()) {
          // hbc::DebugSourceLocation is 1-based, but initializes line and
          // column fields with 0 by default.
          uint32_t line = sourceLocOpt.getValue().line;
          uint32_t column = sourceLocOpt.getValue().column;
          if (line != 0) {
            lineNumber = line;
          }
          if (column != 0) {
            columnNumber = column;
          }
        }

        return fhsp::ProfileSampleCallStackJSFunctionFrame(
            functionName, scriptId, scriptURL, lineNumber, columnNumber);
      }

      default:
        llvm_unreachable("Unexpected Frame kind");
    }
  }

  /// Retrieves the name of a native function from a given internal stack frame.
  /// Supports memoization.
  /// \param frame Internal SamplingProfiler::StackFrame object. Has to be
  /// either NativeFunction or FinalizableNativeFunction.
  /// \return std::string_view pointing to the stored raw std::string that
  /// represents the name of the native function associated with the provided
  /// stack frame.
  std::string_view getNativeFunctionName(
      const SamplingProfiler::StackFrame &frame) {
    assert(
        (frame.kind ==
             SamplingProfiler::StackFrame::FrameKind::NativeFunction ||
         frame.kind ==
             SamplingProfiler::StackFrame::FrameKind::
                 FinalizableNativeFunction) &&
        "Expected only NativeFunction or FinalizableNativeFunction frame");

    auto it = nativeFunctionNameCache_.find(frame.nativeFrame);
    if (it != nativeFunctionNameCache_.end()) {
      return it->second;
    }

    std::string nativeFunctionName =
        samplingProfiler_.getNativeFunctionName(frame);
    auto nativeFunctionNameView = storeString(nativeFunctionName);
    nativeFunctionNameCache_.try_emplace(
        frame.nativeFrame, nativeFunctionNameView);

    return nativeFunctionNameView;
  }

  /// Obtains detailed information about a JavaScript function from its frame
  /// info. Supports memoization.
  /// \param frameInfo Internal SamplingProfiler::JSFunctionFrameInfo object.
  /// \return A JSFunctionFrameDetailsCacheValue tuple containing:
  /// - The function name as a StringEntry object.
  /// - An optional StringEntry object representing the source script URL.
  /// - An optional DebugSourceLocation object providing the source location.
  JSFunctionFrameDetailsCacheValue getJSFunctionDetails(
      const SamplingProfiler::JSFunctionFrameInfo &frameInfo) {
    JSFunctionFrameDetailsKey key(frameInfo.module, frameInfo.functionId);
    auto it = jsFunctionFrameCache_.find(key);
    if (it != jsFunctionFrameCache_.end()) {
      return it->second;
    }

    RuntimeModule *runtimeModule = frameInfo.module;
    hbc::BCProvider *bcProvider = runtimeModule->getBytecode();
    std::string functionName =
        getJSFunctionName(bcProvider, frameInfo.functionId);
    auto functionNameEntry = storeString(functionName);

    OptValue<hbc::DebugSourceLocation> debugSourceLocation =
        getFunctionDefinitionSourceLocation(bcProvider, frameInfo.functionId);
    std::optional<std::string> maybeSourceScriptURL = std::nullopt;
    if (debugSourceLocation.hasValue()) {
      // Bundle has debug info.
      auto filenameId = debugSourceLocation.getValue().filenameId;
      maybeSourceScriptURL =
          bcProvider->getDebugInfo()->getFilenameByID(filenameId);
    }

    std::optional<std::string_view> maybeSourceScriptURLView = std::nullopt;
    if (maybeSourceScriptURL.has_value()) {
      const std::string &sourceScriptURL = maybeSourceScriptURL.value();

      auto sourceScriptURLCacheIt =
          sourceScriptURLCache_.find(llvh::StringRef{sourceScriptURL});
      if (sourceScriptURLCacheIt != sourceScriptURLCache_.end()) {
        maybeSourceScriptURLView.emplace(sourceScriptURLCacheIt->second);
      } else {
        std::string_view sourceScriptURLView = storeString(sourceScriptURL);
        maybeSourceScriptURLView.emplace(sourceScriptURLView);

        sourceScriptURLCache_.try_emplace(
            llvh::StringRef{
                sourceScriptURLView.data(), sourceScriptURLView.size()},
            sourceScriptURLView);
      }
    }

    auto valueToCache = std::make_tuple(
        functionNameEntry, maybeSourceScriptURLView, debugSourceLocation);
    jsFunctionFrameCache_.try_emplace(key, valueToCache);

    return valueToCache;
  }

  /// Places a std::string into stringStorage_.
  /// \return std::string_view that can be supplied to Frame. Returned view
  /// will never be invalidated as long as the storage is not deallocated.
  std::string_view storeString(const std::string &str) {
    stringStorage_->push_back(str);
    return stringStorage_->back();
  }

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
  std::unique_ptr<std::deque<std::string>> stringStorage_;
};

} // namespace

fhsp::Profile generateProfile(
    const SamplingProfiler &samplingProfiler,
    const std::vector<SamplingProfiler::StackTrace> &sampledStacks) {
  ProfileGenerator profileGenerator(samplingProfiler, sampledStacks);
  return profileGenerator.generate();
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
