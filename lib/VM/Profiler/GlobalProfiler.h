/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_GLOBALPROFILER_H
#define HERMES_VM_PROFILER_GLOBALPROFILER_H

#include "hermes/VM/Profiler/SamplingProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/Support/Semaphore.h"
#include "hermes/Support/ThreadLocal.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>

namespace hermes {
namespace vm {

struct GlobalProfiler {
  /// Pointing to the singleton SamplingProfiler instance.
  /// We need this field because accessing local static variable from
  /// signal handler is unsafe.
  static std::atomic<GlobalProfiler *> instance_;

  /// Used to synchronise data writes between the timer thread and the signal
  /// handler in the runtime thread. Also used to send the target
  /// SamplingProfiler to be used during the stack walk.
  static std::atomic<SamplingProfiler *> profilerForSig_;

  /// Lock for profiler operations and access to member fields.
  std::mutex profilerLock_;

  /// Profiler instances for all the individual runtimes that are currently
  /// registered.
  std::unordered_set<SamplingProfiler *> profilers_;

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  /// Per-thread profiler instance for loom profiling.
  /// Limitations: No recursive runtimes in one thread.
  ThreadLocal<SamplingProfiler> threadLocalProfilerForLoom_;
#endif

  /// Whether profiler is enabled or not. Protected by profilerLock_.
  bool enabled_{false};
  /// Whether signal handler is registered or not. Protected by profilerLock_.
  bool isSigHandlerRegistered_{false};

  /// Semaphore to indicate all signal handlers have finished the sampling.
  Semaphore samplingDoneSem_;

  /// Threading: load/store of sampledStackDepth_ and sampleStorage_
  /// are protected by samplingDoneSem_.
  /// Actual sampled stack depth in sampleStorage_.
  uint32_t sampledStackDepth_{0};
  /// Preallocated stack frames storage for signal handler(because
  /// allocating memory in signal handler is not allowed)
  /// This storage does not need to be protected by lock because accessing to
  /// it is serialized by samplingDoneSem_.
  SamplingProfiler::StackTrace sampleStorage_{SamplingProfiler::kMaxStackDepth};

  /// This thread starts in timerLoop_, and samples the stacks of registered
  /// runtimes periodically. It is created in \p enable() and joined in
  /// \p disable().
  std::thread timerThread_;

  /// This condition variable can be used to wait for a change in the enabled
  /// member variable.
  std::condition_variable enabledCondVar_;

  /// invoke sigaction() posix API to register \p handler.
  /// \return what sigaction() returns: 0 to indicate success.
  static int invokeSignalAction(void (*handler)(int));

  /// Register sampling signal handler if not done yet.
  /// \return true to indicate success.
  bool registerSignalHandlers();

  /// Unregister sampling signal handler.
  bool unregisterSignalHandler();

  /// Signal handler to walk the stack frames.
  static void profilingSignalHandler(int signo);

  /// Main routine to take a sample of runtime stack.
  /// \return false for failure which timer loop thread should stop.
  bool sampleStacks();
  /// Sample stack for a profiler.
  bool sampleStack(SamplingProfiler *localProfiler);

  /// Timer loop thread main routine.
  void timerLoop();

  /// Implementation of SamplingProfiler::enable/disable.
  bool enable();
  bool disable();

  /// \return true if the sampling profiler is enabled, false otherwise.
  bool enabled();

  /// Register the \p profiler associated with an active runtime.
  /// Should only be called from the thread running hermes runtime.
  void registerRuntime(SamplingProfiler *profiler);

  /// Unregister the active runtime and current thread associated with
  /// \p profiler.
  void unregisterRuntime(SamplingProfiler *profiler);

  /// \return the singleton profiler instance.
  static GlobalProfiler *get();

 private:
  GlobalProfiler();
};
} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#endif // HERMES_VM_PROFILER_GLOBALPROFILER_H
