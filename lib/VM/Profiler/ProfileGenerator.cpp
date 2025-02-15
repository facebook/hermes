/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProfileGenerator.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

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

static OptValue<hbc::DebugSourceLocation> getSourceLocation(
    hbc::BCProvider *bcProvider,
    uint32_t funcId,
    uint32_t opcodeOffset) {
  const hbc::DebugOffsets *debugOffsets = bcProvider->getDebugOffsets(funcId);
  if (debugOffsets &&
      debugOffsets->sourceLocations != hbc::DebugOffsets::NO_OFFSET) {
    return bcProvider->getDebugInfo()->getLocationForAddress(
        debugOffsets->sourceLocations, opcodeOffset);
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

/// Format VM-level frame to public interface.
static fhsp::ProfileSampleCallStackFrame *formatCallStackFrame(
    const SamplingProfiler::StackFrame &frame,
    const SamplingProfiler &samplingProfiler) {
  switch (frame.kind) {
    case SamplingProfiler::StackFrame::FrameKind::SuspendFrame: {
      return new fhsp::ProfileSampleCallStackSuspendFrame{
          formatSuspendFrameKind(frame.suspendFrame.kind),
      };
    }

    case SamplingProfiler::StackFrame::FrameKind::NativeFunction:
      return new fhsp::ProfileSampleCallStackNativeFunctionFrame{
          samplingProfiler.getNativeFunctionName(frame),
      };

    case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction:
      return new fhsp::ProfileSampleCallStackHostFunctionFrame{
          samplingProfiler.getNativeFunctionName(frame),
      };

    case SamplingProfiler::StackFrame::FrameKind::JSFunction: {
      RuntimeModule *module = frame.jsFrame.module;
      hbc::BCProvider *bcProvider = module->getBytecode();

      std::string functionName =
          getJSFunctionName(bcProvider, frame.jsFrame.functionId);
      std::optional<uint32_t> scriptId = std::nullopt;
      std::optional<std::string> url = std::nullopt;
      std::optional<uint32_t> lineNumber = std::nullopt;
      std::optional<uint32_t> columnNumber = std::nullopt;

      OptValue<hbc::DebugSourceLocation> sourceLocOpt = getSourceLocation(
          bcProvider, frame.jsFrame.functionId, frame.jsFrame.offset);
      if (sourceLocOpt.hasValue()) {
        // Bundle has debug info.
        scriptId = sourceLocOpt.getValue().filenameId;
        url = bcProvider->getDebugInfo()->getFilenameByID(scriptId.value());

        // hbc::DebugSourceLocation is 1-based, but initializes line and column
        // fields with 0 by default.
        uint32_t line = sourceLocOpt.getValue().line;
        uint32_t column = sourceLocOpt.getValue().column;
        if (line != 0) {
          lineNumber = line;
        }
        if (column != 0) {
          columnNumber = column;
        }
      }

      return new fhsp::ProfileSampleCallStackJSFunctionFrame{
          functionName,
          scriptId,
          url,
          lineNumber,
          columnNumber,
      };
    }

    default:
      llvm_unreachable("Unexpected Frame kind");
  }
}

} // namespace

/* static */ fhsp::Profile ProfileGenerator::generate(
    const SamplingProfiler &sp,
    const std::vector<SamplingProfiler::StackTrace> &sampledStacks) {
  std::vector<fhsp::ProfileSample> samples;
  samples.reserve(sampledStacks.size());
  for (const SamplingProfiler::StackTrace &sampledStack : sampledStacks) {
    uint64_t timestamp = convertTimestampToMicroseconds(sampledStack.timeStamp);

    std::vector<fhsp::ProfileSampleCallStackFrame *> callFrames;
    callFrames.reserve(sampledStack.stack.size());
    for (const SamplingProfiler::StackFrame &frame : sampledStack.stack) {
      callFrames.emplace_back(formatCallStackFrame(frame, sp));
    }

    samples.emplace_back(timestamp, sampledStack.tid, callFrames);
  }

  return fhsp::Profile{samples};
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
