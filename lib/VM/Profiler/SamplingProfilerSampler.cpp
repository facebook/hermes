/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SamplingProfilerSampler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "ChromeTraceSerializer.h"

#include <fcntl.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

namespace hermes {
namespace vm {
namespace sampling_profiler {

Sampler::~Sampler() = default;

void Sampler::registerRuntime(SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  profilers_.insert(profiler);
  platformRegisterRuntime(profiler);
}

void Sampler::unregisterRuntime(SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  bool succeed = profilers_.erase(profiler);
  // TODO: should we allow recursive style
  // register/register -> unregister/unregister call?
  assert(succeed && "How can runtime not registered yet?");
  (void)succeed;
  platformUnregisterRuntime(profiler);
}

bool Sampler::sampleStacks() {
  for (SamplingProfiler *localProfiler : profilers_) {
    std::lock_guard<std::mutex> lk(localProfiler->runtimeDataLock_);
    if (!sampleStack(localProfiler)) {
      return false;
    }
    platformPostSampleStack(localProfiler);
  }
  return true;
}

bool Sampler::sampleStack(SamplingProfiler *localProfiler) {
  if (localProfiler->suspendCount_ > 0) {
    // Sampling profiler is suspended. Copy pre-captured stack instead without
    // interrupting the VM thread.
    if (localProfiler->preSuspendStackDepth_ > 0) {
      sampleStorage_ = localProfiler->preSuspendStackStorage_;
      sampledStackDepth_ = localProfiler->preSuspendStackDepth_;
    } else {
      // This suspension didn't record a stack trace. For example, a GC (like
      // mallocGC) did not record JS stack.
      // TODO: fix this for all cases.
      sampledStackDepth_ = 0;
    }
  } else {
    // Ensure there are no allocations in the signal handler by keeping ample
    // reserved space.
    localProfiler->domains_.reserve(
        localProfiler->domains_.size() + SamplingProfiler::kMaxStackDepth);
    size_t domainCapacityBefore = localProfiler->domains_.capacity();
    (void)domainCapacityBefore;

    // Ditto for native functions.
    localProfiler->nativeFunctions_.reserve(
        localProfiler->nativeFunctions_.size() +
        SamplingProfiler::kMaxStackDepth);
    size_t nativeFunctionsCapacityBefore =
        localProfiler->nativeFunctions_.capacity();
    (void)nativeFunctionsCapacityBefore;

    if (!platformSuspendVMAndWalkStack(localProfiler)) {
      return false;
    }

    assert(
        localProfiler->domains_.capacity() == domainCapacityBefore &&
        "Must not dynamically allocate in signal handler");

    assert(
        localProfiler->nativeFunctions_.capacity() ==
            nativeFunctionsCapacityBefore &&
        "Must not dynamically allocate in signal handler");
  }

  assert(
      sampledStackDepth_ <= sampleStorage_.stack.size() &&
      "How can we sample more frames than storage?");
  localProfiler->sampledStacks_.emplace_back(
      sampleStorage_.tid,
      sampleStorage_.timeStamp,
      sampleStorage_.stack.begin(),
      sampleStorage_.stack.begin() + sampledStackDepth_);
  return true;
}

void Sampler::walkRuntimeStack(SamplingProfiler *profiler) {
  assert(
      profiler->suspendCount_ == 0 &&
      "Shouldn't interrupt the VM thread when the sampling profiler is "
      "suspended.");

  // Sampling stack will touch GC objects(like closure) so only do so if heap
  // is valid.
  auto &curThreadRuntime = profiler->runtime_;
  assert(
      !curThreadRuntime.getHeap().inGC() &&
      "sampling profiler should be suspended before GC");
  (void)curThreadRuntime;
  sampledStackDepth_ =
      profiler->walkRuntimeStack(sampleStorage_, SamplingProfiler::InLoom::No);
}

void Sampler::timerLoop() {
  oscompat::set_thread_name("hermes-sampling-profiler");

  constexpr double kMeanMilliseconds = 10;
  constexpr double kStdDevMilliseconds = 5;
  std::random_device rd{};
  std::mt19937 gen{rd()};
  // The amount of time that is spent sleeping comes from a normal distribution,
  // to avoid the case where the timer thread samples a stack at a predictable
  // period.
  std::normal_distribution<> distribution{
      kMeanMilliseconds, kStdDevMilliseconds};
  std::unique_lock<std::mutex> uniqueLock(profilerLock_);

  while (enabled_) {
    if (!sampleStacks()) {
      return;
    }

    const uint64_t millis = round(std::fabs(distribution(gen)));
    // TODO: make sampling rate configurable.
    enabledCondVar_.wait_for(
        uniqueLock, std::chrono::milliseconds(millis), [this]() {
          return !enabled_;
        });
  }
}

bool Sampler::enabled() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  return enabled_;
}

bool Sampler::enable() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  if (enabled_) {
    return true;
  }
  if (!platformEnable()) {
    return false;
  }
  enabled_ = true;
  // Start timer thread.
  timerThread_ = std::thread(&Sampler::timerLoop, this);
  return true;
}

bool Sampler::disable() {
  {
    std::lock_guard<std::mutex> lockGuard(profilerLock_);
    if (!enabled_) {
      // Already disabled.
      return true;
    }
    if (!platformDisable()) {
      return false;
    }
    // Telling timer thread to exit.
    enabled_ = false;
  }
  // Notify the timer thread that it has been disabled.
  enabledCondVar_.notify_all();
  // Wait for timer thread to exit. This avoids the timer thread reading from
  // memory that is freed after a main thread exits. This is outside the lock
  // on profilerLock_ since the timer thread needs to acquire that lock.
  timerThread_.join();
  return true;
}

} // namespace sampling_profiler
} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
