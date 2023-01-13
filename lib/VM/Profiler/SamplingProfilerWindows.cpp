/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SamplingProfilerSampler.h"

#if defined(HERMESVM_SAMPLING_PROFILER_WINDOWS)

#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Windows.h"

namespace hermes {
namespace vm {
namespace sampling_profiler {
namespace {
struct SamplingProfilerWindows : SamplingProfiler {
  SamplingProfilerWindows(Runtime &rt) : SamplingProfiler(rt) {
    currentThread_ = OpenThread(
        THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
        false,
        GetCurrentThreadId());
  }

  ~SamplingProfilerWindows() override {
    // TODO(T125910634): re-introduce the requirement for destroying the
    // sampling profiler on the same thread in which it was created.
    Sampler::get()->unregisterRuntime(this);
  }

  /// Thread that this profiler instance represents. This can currently only be
  /// set from the constructor of SamplingProfiler, so we need to construct a
  /// new SamplingProfiler every time the runtime is moved to a different
  /// thread.
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

void Sampler::platformPostSampleStack(SamplingProfiler *localProfiler) {}

bool Sampler::platformSuspendVMAndWalkStack(SamplingProfiler *profiler) {
  auto *winProfiler = static_cast<SamplingProfilerWindows *>(profiler);

  // Suspend the JS thread.
  DWORD prevSuspendCount = SuspendThread(winProfiler->currentThread_);
  if (prevSuspendCount == static_cast<DWORD>(-1)) {
    return true;
  }
  assert(prevSuspendCount == 0 && "JS thread should not be suspended");

  // Get the JS thread context. This ensures that the thread suspension is
  // completed.
  CONTEXT context;
  GetThreadContext(winProfiler->currentThread_, &context);

  // Walk the stack.
  walkRuntimeStack(profiler);

  // Resume the thread.
  prevSuspendCount = ResumeThread(winProfiler->currentThread_);
  assert(prevSuspendCount == 1);

  return true;
}
} // namespace sampling_profiler

std::unique_ptr<SamplingProfiler> SamplingProfiler::create(Runtime &rt) {
  return std::make_unique<sampling_profiler::SamplingProfilerWindows>(rt);
}

bool SamplingProfiler::belongsToCurrentThread() const {
  return GetThreadId(
             static_cast<const sampling_profiler::SamplingProfilerWindows *>(
                 this)
                 ->currentThread_) == GetCurrentThreadId();
}

} // namespace vm
} // namespace hermes
#endif // !defined(HERMESVM_SAMPLING_PROFILER_WINDOWS)
