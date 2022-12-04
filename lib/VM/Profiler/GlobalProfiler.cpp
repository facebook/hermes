/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GlobalProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "ChromeTraceSerializer.h"

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

namespace hermes {
namespace vm {

/// Name of the semaphore.
const char *const kSamplingDoneSemaphoreName = "/samplingDoneSem";

void GlobalProfiler::registerRuntime(SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  profilers_.insert(profiler);

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  assert(
      threadLocalProfilerForLoom_.get() == nullptr &&
      "multiple hermes runtime in the same thread");
  threadLocalProfilerForLoom_.set(profiler);
#endif
}

void GlobalProfiler::unregisterRuntime(SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  bool succeed = profilers_.erase(profiler);
  // TODO: should we allow recursive style
  // register/register -> unregister/unregister call?
  assert(succeed && "How can runtime not registered yet?");
  (void)succeed;

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  // TODO(T125910634): re-introduce the requirement for unregistering the
  // runtime in the same thread it was registered.
  threadLocalProfilerForLoom_.set(nullptr);
#endif
}

bool GlobalProfiler::sampleStacks() {
  for (SamplingProfiler *localProfiler : profilers_) {
    std::lock_guard<std::mutex> lk(localProfiler->runtimeDataLock_);
    if (!sampleStack(localProfiler)) {
      return false;
    }
#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
    if (localProfiler->shouldPushDataToLoom()) {
      localProfiler->pushLastSampledStackToLoom();
    }
#endif
  }
  return true;
}

bool GlobalProfiler::sampleStack(SamplingProfiler *localProfiler) {
  auto targetThreadId = localProfiler->currentThread_;
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

    // Guarantee that the runtime thread will not proceed until it has
    // acquired the updates to domains_.
    profilerForSig_.store(localProfiler, std::memory_order_release);

    // Signal target runtime thread to sample stack.
    pthread_kill(targetThreadId, SIGPROF);

    // Threading: samplingDoneSem_ will synchronise this thread with the
    // signal handler, so that we only have one active signal at a time.
    if (!samplingDoneSem_.wait()) {
      return false;
    }

    // Guarantee that this thread will observe all changes made to data
    // structures in the signal handler.
    while (profilerForSig_.load(std::memory_order_acquire) != nullptr) {
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

void GlobalProfiler::timerLoop() {
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

GlobalProfiler::GlobalProfiler() {
  instance_.store(this);
#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  profilo_api()->register_external_tracer_callback(
      TRACER_TYPE_JAVASCRIPT, SamplingProfiler::collectStackForLoom);
#endif
}

bool GlobalProfiler::enabled() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  return enabled_;
}

bool GlobalProfiler::enable() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  if (enabled_) {
    return true;
  }
  if (!samplingDoneSem_.open(kSamplingDoneSemaphoreName)) {
    return false;
  }
  if (!registerSignalHandlers()) {
    return false;
  }
  enabled_ = true;
  // Start timer thread.
  timerThread_ = std::thread(&GlobalProfiler::timerLoop, this);
  return true;
}

bool GlobalProfiler::disable() {
  {
    std::lock_guard<std::mutex> lockGuard(profilerLock_);
    if (!enabled_) {
      // Already disabled.
      return true;
    }
    if (!samplingDoneSem_.close()) {
      return false;
    }
    // Unregister handlers before shutdown.
    if (!unregisterSignalHandler()) {
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

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
