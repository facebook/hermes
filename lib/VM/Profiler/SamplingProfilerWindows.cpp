/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SamplingProfilerSampler.h"

#if defined(HERMESVM_SAMPLING_PROFILER_WINDOWS)

#if defined(HERMES_FACEBOOK_BUILD) && defined(HERMESVM_ALLOW_LOOM)
#define HERMESVM_ENABLE_LOOM
#endif

#if defined(HERMESVM_ENABLE_LOOM)
#include <FBLoom/ExternalApi/ExternalApi.h>
#endif

#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Windows.h"

namespace hermes {
namespace vm {

/// Open the current thread and \return a handle to it. The handle must later
/// be closed with a call to the Win32 CloseHandle API.
static HANDLE openCurrentThread() {
  return OpenThread(
      THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
      false,
      GetCurrentThreadId());
}

namespace sampling_profiler {
namespace {
struct SamplingProfilerWindows : SamplingProfiler {
  SamplingProfilerWindows(Runtime &rt) : SamplingProfiler(rt) {
    currentThread_ = openCurrentThread();

#if defined(HERMESVM_ENABLE_LOOM)
    fbloom_profilo_api()->fbloom_register_enable_for_loom_callback(
        FBLoomTracerType::JAVASCRIPT, []() { return enable(); });
    fbloom_profilo_api()->fbloom_register_disable_for_loom_callback(
        FBLoomTracerType::JAVASCRIPT, disable);
    loomDataPushEnabled_ = true;
#endif // defined(HERMESVM_ENABLE_LOOM)
  }

  ~SamplingProfilerWindows() override {
    // TODO(T125910634): re-introduce the requirement for destroying the
    // sampling profiler on the same thread in which it was created.
    Sampler::get()->unregisterRuntime(this);

    CloseHandle(currentThread_);
  }

#if defined(HERMESVM_ENABLE_LOOM)
  bool shouldPushDataToLoom() const {
    auto now = std::chrono::system_clock::now();
    constexpr auto kLoomDelay = std::chrono::milliseconds(50);
    // The default sample stack interval in timerLoop() is between 5-15ms
    // which is too often for loom.
    return loomDataPushEnabled_ && (now - previousPushTs > kLoomDelay);
  }

  void collectStackForLoomCommon(
      const StackFrame &frame,
      int64_t *frames,
      uint32_t index) {
    constexpr uint64_t kNativeFrameMask = ((uint64_t)1 << 63);
    switch (frame.kind) {
      case StackFrame::FrameKind::JSFunction: {
        auto *bcProvider = frame.jsFrame.module->getBytecode();
        uint32_t virtualOffset =
            bcProvider->getVirtualOffsetForFunction(frame.jsFrame.functionId) +
            frame.jsFrame.offset;
        uint32_t segmentID = bcProvider->getSegmentID();
        uint64_t frameAddress = ((uint64_t)segmentID << 32) + virtualOffset;
        assert(
            (frameAddress & kNativeFrameMask) == 0 &&
            "Module id should take less than 32 bits");
        frames[(index)] = static_cast<int64_t>(frameAddress);
        break;
      }

      case StackFrame::FrameKind::NativeFunction:
      case StackFrame::FrameKind::FinalizableNativeFunction: {
        NativeFunctionPtr nativeFrame = getNativeFunctionPtr(frame);
        frames[(index)] = ((uint64_t)nativeFrame | kNativeFrameMask);
        break;
      }
      case StackFrame::FrameKind::SuspendFrame:
        break;
      default:
        llvm_unreachable("Loom: unknown frame kind");
    }
  }

  void pushLastSampledStackToLoom() {
    constexpr uint16_t maxDepth = 512;
    int64_t frames[maxDepth];
    uint16_t depth = 0;
    // Each element in sampledStacks_ is one call stack, access the last one
    // to get the latest stack trace.
    auto sample = sampledStacks_.back();
    for (auto iter = sample.stack.rbegin(); iter != sample.stack.rend();
         ++iter) {
      const StackFrame &frame = *iter;
      collectStackForLoomCommon(frame, frames, depth);
      depth++;
      if (depth > maxDepth) {
        return;
      }
    }
    fbloom_profilo_api()->fbloom_write_stack_to_loom(
        FBLoomTracerType::JAVASCRIPT, frames, depth);
    previousPushTs = std::chrono::system_clock::now();
    clear();
  }

  /// Previous timestamp when a push to loom occurred. The Loom API does not
  /// rate limit its callers, and thus care must be taken to not overload it.
  std::chrono::time_point<std::chrono::system_clock> previousPushTs;

  bool loomDataPushEnabled_{false};
#endif // defined(HERMESVM_ENABLE_LOOM)

  /// Thread that this profiler instance represents. This can be updated
  /// by later calls to SetRuntimeThread.
  HANDLE currentThread_;
};
} // namespace

namespace {
struct SamplerWindows : Sampler {
  SamplerWindows();
  ~SamplerWindows() override;
};
} // namespace

Sampler *Sampler::get() {
  // We intentionally leak this memory to avoid a case where instance is
  // accessed after it is destroyed during shutdown.
  static SamplerWindows *instance = new SamplerWindows{};
  return instance;
}

SamplerWindows::~SamplerWindows() = default;

SamplerWindows::SamplerWindows() = default;

bool Sampler::platformEnable() {
  return true;
}

bool Sampler::platformDisable() {
  return true;
}

void Sampler::platformRegisterRuntime(SamplingProfiler *profiler) {}

void Sampler::platformUnregisterRuntime(SamplingProfiler *profiler) {}

void Sampler::platformPostSampleStack(SamplingProfiler *localProfiler) {
#if defined(HERMESVM_ENABLE_LOOM)
  auto *windowsProfiler = static_cast<SamplingProfilerWindows *>(localProfiler);
  if (windowsProfiler->shouldPushDataToLoom()) {
    windowsProfiler->pushLastSampledStackToLoom();
  }
#endif // defined(HERMESVM_ENABLE_LOOM)
}

bool Sampler::platformSuspendVMAndWalkStack(SamplingProfiler *profiler) {
  auto *winProfiler = static_cast<SamplingProfilerWindows *>(profiler);

  // Suspend the JS thread. The runtimeDataLock is held by the caller, ensuring
  // the runtime won't start to be used on another thread before sampling
  // begins.
  DWORD prevSuspendCount = SuspendThread(winProfiler->currentThread_);
  if (prevSuspendCount == static_cast<DWORD>(-1)) {
    return true;
  }

  // Get the JS thread context. This ensures that the thread suspension is
  // completed.
  CONTEXT context;
  GetThreadContext(winProfiler->currentThread_, &context);

  // Walk the stack.
  walkRuntimeStack(profiler);

  // Resume the thread.
  prevSuspendCount = ResumeThread(winProfiler->currentThread_);
  assert(
      prevSuspendCount != static_cast<DWORD>(-1) &&
      "couldn't resume js thread");
  (void)prevSuspendCount;

  return true;
}
} // namespace sampling_profiler

std::unique_ptr<SamplingProfiler> SamplingProfiler::create(Runtime &rt) {
  return std::make_unique<sampling_profiler::SamplingProfilerWindows>(rt);
}

bool SamplingProfiler::belongsToCurrentThread() {
  auto profiler =
      static_cast<sampling_profiler::SamplingProfilerWindows *>(this);
  std::lock_guard<std::mutex> lock(profiler->runtimeDataLock_);
  return GetThreadId(profiler->currentThread_) == GetCurrentThreadId();
}

void SamplingProfiler::setRuntimeThread() {
  auto profiler =
      static_cast<sampling_profiler::SamplingProfilerWindows *>(this);
  std::lock_guard<std::mutex> lock(profiler->runtimeDataLock_);
  CloseHandle(profiler->currentThread_);
  profiler->currentThread_ = openCurrentThread();
}

} // namespace vm
} // namespace hermes
#endif // !defined(HERMESVM_SAMPLING_PROFILER_WINDOWS)
