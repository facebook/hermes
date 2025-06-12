/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProfileGenerator.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

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

} // namespace

ProfileGenerator::ProfileGenerator(
    const SamplingProfiler &samplingProfiler,
    const std::vector<SamplingProfiler::StackTrace> &sampledStacks)
    : samplingProfiler_(samplingProfiler), sampledStacks_(sampledStacks) {}

fhsp::ProfileSampleCallStackFrame ProfileGenerator::processStackFrame(
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

fhsp::StringEntry ProfileGenerator::getNativeFunctionName(
    const SamplingProfiler::StackFrame &frame) {
  assert(
      (frame.kind == SamplingProfiler::StackFrame::FrameKind::NativeFunction ||
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
  auto nativeFunctionNameEntry = storeString(nativeFunctionName);
  nativeFunctionNameCache_.try_emplace(
      frame.nativeFrame, nativeFunctionNameEntry);

  return nativeFunctionNameEntry;
}

ProfileGenerator::JSFunctionFrameDetailsCacheValue
ProfileGenerator::getJSFunctionDetails(
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

  std::optional<fhsp::StringEntry> maybeSourceScriptURLEntry = std::nullopt;
  if (maybeSourceScriptURL.has_value()) {
    const std::string &sourceScriptURL = maybeSourceScriptURL.value();

    auto sourceScriptURLCacheIt =
        sourceScriptURLCache_.find(llvh::StringRef{sourceScriptURL});
    if (sourceScriptURLCacheIt != sourceScriptURLCache_.end()) {
      maybeSourceScriptURLEntry.emplace(sourceScriptURLCacheIt->second);
    } else {
      fhsp::StringEntry sourceScriptURLEntry = storeString(sourceScriptURL);
      maybeSourceScriptURLEntry.emplace(sourceScriptURLEntry);

      // The string from stringStorage_ is referenced here, because it
      // will outlive sourceScriptURLCache_. The lifetime of
      // sourceScriptURLCache_ is bound to the lifetime of ProfileGenerator,
      // whereas stringStorage_'s lifetime is bound to the lifetime of the
      // Profile object. Once ProfileGenerator::generate() finishes, the
      // stringStorage_ is moved to the Profile and no longer valid within
      // ProfileGenerator.
      std::string_view sourceScriptURLView = sourceScriptURLEntry.getView();
      sourceScriptURLCache_.try_emplace(
          llvh::StringRef{
              sourceScriptURLView.data(), sourceScriptURLView.size()},
          sourceScriptURLEntry);
    }
  }

  auto valueToCache = std::make_tuple(
      functionNameEntry, maybeSourceScriptURLEntry, debugSourceLocation);
  jsFunctionFrameCache_.try_emplace(key, valueToCache);

  return valueToCache;
}

fhsp::StringEntry ProfileGenerator::storeString(const std::string &str) {
  auto offset = stringStorage_->size();
  stringStorage_->push_back(str);

  return fhsp::StringEntry(*stringStorage_.get(), offset);
};

fhsp::Profile ProfileGenerator::generate() {
  stringStorage_ = std::make_unique<std::vector<std::string>>();

  std::vector<fhsp::ProfileSample> samples;
  samples.reserve(sampledStacks_.size());
  for (const SamplingProfiler::StackTrace &sampledStack : sampledStacks_) {
    uint64_t timestamp = convertTimestampToMicroseconds(sampledStack.timeStamp);

    std::vector<fhsp::ProfileSampleCallStackFrame> callFrames;
    callFrames.reserve(sampledStack.stack.size());
    for (const SamplingProfiler::StackFrame &frame : sampledStack.stack) {
      callFrames.push_back(processStackFrame(frame));
    }

    samples.emplace_back(timestamp, sampledStack.tid, std::move(callFrames));
  }

  return fhsp::Profile(std::move(samples), std::move(stringStorage_));
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
