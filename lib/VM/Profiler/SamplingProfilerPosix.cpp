/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SamplingProfilerSampler.h"

#if defined(HERMESVM_SAMPLING_PROFILER_POSIX)

#include "hermes/Support/Semaphore.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "TraceSerializer.h"

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
namespace sampling_profiler {
namespace {

/// Name of the semaphore.
constexpr char kSamplingDoneSemaphoreName[] = "/samplingDoneSem";

struct SamplingProfilerPosix : SamplingProfiler {
  SamplingProfilerPosix(Runtime &rt);
  ~SamplingProfilerPosix() override;

  /// Thread that this profiler instance represents. This can be updated as the
  /// runtime is invoked on different threads. Must only be accessed while
  /// holding the runtimeDataLock_.
  pthread_t currentThread_;
};

struct SamplerPosix : Sampler {
  SamplerPosix();
  ~SamplerPosix() override;
  /// Pointing to the singleton SamplingProfiler instance.
  /// We need this field because accessing local static variable from
  /// signal handler is unsafe.
  static std::atomic<SamplerPosix *> instance_;

  /// Used to synchronise data writes between the timer thread and the signal
  /// handler in the runtime thread. Also used to send the target
  /// SamplingProfiler to be used during the stack walk.
  static std::atomic<SamplingProfiler *> profilerForSig_;

  /// Whether signal handler is registered or not. Protected by profilerLock_.
  bool isSigHandlerRegistered_{false};

  /// Semaphore to indicate all signal handlers have finished the sampling.
  Semaphore samplingDoneSem_;

  /// Register sampling signal handler if not done yet.
  /// \return true to indicate success.
  bool registerSignalHandlers();

  /// Unregister sampling signal handler.
  bool unregisterSignalHandler();

  /// Signal handler to walk the stack frames.
  static void profilingSignalHandler(int signo);
};
} // namespace

SamplingProfilerPosix::SamplingProfilerPosix(Runtime &rt)
    : SamplingProfiler(rt), currentThread_{pthread_self()} {
  // Note that we cannot register this in the base class constructor, because
  // all fields must be initialized before we register with the profiling
  // thread.
  sampling_profiler::Sampler::get()->registerRuntime(this);
}

SamplingProfilerPosix::~SamplingProfilerPosix() {
  // TODO(T125910634): re-introduce the requirement for destroying the sampling
  // profiler on the same thread in which it was created.
  Sampler::get()->unregisterRuntime(this);
}

std::atomic<SamplerPosix *> SamplerPosix::instance_{nullptr};

std::atomic<SamplingProfiler *> SamplerPosix::profilerForSig_{nullptr};

namespace {
/// invoke sigaction() posix API to register \p handler.
/// \return what sigaction() returns: 0 to indicate success.
static int invokeSignalAction(void (*handler)(int)) {
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  // Allows interrupted IO primitives to restart.
  actions.sa_flags = SA_RESTART;
  actions.sa_handler = handler;
  return sigaction(SIGPROF, &actions, nullptr);
}
} // namespace

bool SamplerPosix::registerSignalHandlers() {
  if (isSigHandlerRegistered_) {
    return true;
  }
  if (invokeSignalAction(profilingSignalHandler) != 0) {
    perror("signal handler registration failed");
    return false;
  }
  isSigHandlerRegistered_ = true;
  return true;
}

bool SamplerPosix::unregisterSignalHandler() {
  if (!isSigHandlerRegistered_) {
    return true;
  }
  // Restore to default.
  if (invokeSignalAction(SIG_DFL) != 0) {
    perror("signal handler unregistration failed");
    return false;
  }
  isSigHandlerRegistered_ = false;
  return true;
}

void SamplerPosix::profilingSignalHandler(int signo) {
  // Ensure that writes made on the timer thread before setting the current
  // profiler are correctly acquired.
  SamplingProfiler *localProfiler;
  while (!(localProfiler = profilerForSig_.load(std::memory_order_acquire))) {
  }

  // Avoid spoiling errno in a signal handler by storing the old version and
  // re-assigning it.
  auto oldErrno = errno;

  auto *profilerInstance = static_cast<SamplerPosix *>(instance_.load());
  assert(
      profilerInstance != nullptr &&
      "Why is SamplerPosix::instance_ not initialized yet?");

  profilerInstance->walkRuntimeStack(
      localProfiler, SamplingProfiler::MayAllocate::No);

  // Ensure that writes made in the handler are visible to the timer thread.
  profilerForSig_.store(nullptr);

  if (!profilerInstance->samplingDoneSem_.notifyOne()) {
    errno = oldErrno;
    abort(); // Something is wrong.
  }
  errno = oldErrno;
}

/*static*/ Sampler *Sampler::get() {
  // We intentionally leak this memory to avoid a case where instance is
  // accessed after it is destroyed during shutdown.
  static SamplerPosix *instance = new SamplerPosix{};
  return instance;
}

SamplerPosix::~SamplerPosix() = default;

SamplerPosix::SamplerPosix() {
  instance_.store(this);
}

bool Sampler::platformEnable() {
  auto *self = static_cast<SamplerPosix *>(this);
  if (!self->samplingDoneSem_.open(kSamplingDoneSemaphoreName)) {
    return false;
  }
  if (!self->registerSignalHandlers()) {
    return false;
  }

  return true;
}

bool Sampler::platformDisable() {
  auto *self = static_cast<SamplerPosix *>(this);
  if (!self->samplingDoneSem_.close()) {
    return false;
  }
  // Unregister handlers before shutdown.
  if (!self->unregisterSignalHandler()) {
    return false;
  }

  return true;
}

void Sampler::platformRegisterRuntime(SamplingProfiler *profiler) {}

void Sampler::platformUnregisterRuntime(SamplingProfiler *profiler) {}

void Sampler::platformPostSampleStack(SamplingProfiler *localProfiler) {}

bool Sampler::platformSuspendVMAndWalkStack(SamplingProfiler *profiler) {
  auto *self = static_cast<SamplerPosix *>(this);
  auto *posixProfiler = static_cast<SamplingProfilerPosix *>(profiler);
  // Guarantee that the runtime thread will not proceed until it has
  // acquired the updates to domains_.
  self->profilerForSig_.store(profiler, std::memory_order_release);

  // Signal target runtime thread to sample stack. The runtimeDataLock is
  // held by the caller, ensuring the runtime won't start to be used on
  // another thread before sampling begins.
  pthread_kill(posixProfiler->currentThread_, SIGPROF);

  // Threading: samplingDoneSem_ will synchronise this thread with the
  // signal handler, so that we only have one active signal at a time.
  if (!self->samplingDoneSem_.wait()) {
    return false;
  }

  // Guarantee that this thread will observe all changes made to data
  // structures in the signal handler.
  while (self->profilerForSig_.load(std::memory_order_acquire) != nullptr) {
  }

  return true;
}

} // namespace sampling_profiler

std::unique_ptr<SamplingProfiler> SamplingProfiler::create(Runtime &rt) {
  return std::make_unique<sampling_profiler::SamplingProfilerPosix>(rt);
}

bool SamplingProfiler::belongsToCurrentThread() {
  auto profiler = static_cast<sampling_profiler::SamplingProfilerPosix *>(this);
  std::lock_guard<std::mutex> lock(profiler->runtimeDataLock_);
  return profiler->currentThread_ == pthread_self();
}

void SamplingProfiler::setRuntimeThread() {
  auto profiler = static_cast<sampling_profiler::SamplingProfilerPosix *>(this);
  std::lock_guard<std::mutex> lock(profiler->runtimeDataLock_);
  profiler->currentThread_ = pthread_self();
  threadID_ = oscompat::global_thread_id();
  threadNames_[threadID_] = oscompat::thread_name();
}

} // namespace vm
} // namespace hermes

#endif // defined(HERMESVM_SAMPLING_PROFILER_POSIX)
