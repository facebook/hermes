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

#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>

namespace hermes {
namespace vm {

/// GlobalProfiler manages the SamplingProfiler's sampling thread, and abstracts
/// away platform-specific code for suspending the VM thread and performing the
/// JS stack walk.
///
/// Each suported platform (e.g., Posix), must have its own "flavor" of the
/// GlobalProfiler (e.g., GlobalProfilerPosix) which must implement the
/// GlobalProfiler::platform<<*>> methods.
///
/// Usually, the platform-agnostic GlobalProfiler invokes the platform-specific
/// ones. For example, GlobalProfiler::enable() performs some checks, then call
/// GlobalProfiler::platformEnable() for completing the initialization.
///
/// Sampling happens on a separate thread whose lifetime is managed by the
/// GlobalProfiler's platform-specific code. The sampling thread runs the
/// GlobalProfiler::timerLoop function, which will periodically traverse the
/// list of registered runtimes, and perform the stack walk, which is
/// accomplished as follows.
///
/// GlobalProfiler::timerLoop invokes GlobalProfiler::sampleStacks, which will
/// invoke GlobalProfiler::sampleStack for each registered runtime.
///
/// GlobalProfiler::sampleStack invoke the platform-specific
/// GlobalProfiler::suspendVMAndWalkStack method. This hook should be
/// implemented on every supported platform, and it is responsible for
/// suspending VM execution. With the stopped VM, the platform-specific code
/// calls back into platform-agnostic code (GlobalProfiler::walkRuntimeStack) so
/// stack walking can continue.
///
/// When GlobalProfiler::walkRuntimeStack runs the VM is suspended, but this
/// suspension can happen when the VM is in the middle of, e.g., memory
/// allocation, or while it is holding some lock. Thus,
/// GlobalProfiler::walkRuntimeStack should not acquire locks, or even allocate
/// memory. It should perform stack walking quickly and expeditiously. All
/// buffers used for stack walking are pre-allocated before calling into this
/// function.
///
/// Finally, when GlobalProfiler::walkRuntimeStack returns to
/// GlobalProfiler::suspendVMAndWalkStack, VM execution should be resumed.
struct GlobalProfiler {
  virtual ~GlobalProfiler();

  /// Lock for profiler operations and access to member fields.
  std::mutex profilerLock_;

  /// Profiler instances for all the individual runtimes that are currently
  /// registered.
  std::unordered_set<SamplingProfiler *> profilers_;

  /// Whether profiler is enabled or not. Protected by profilerLock_.
  bool enabled_{false};

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

  /// Main routine to take a sample of runtime stack.
  /// \return false for failure which timer loop thread should stop.
  bool sampleStacks();

  /// Timer loop thread main routine.
  void timerLoop();

  /// Implementation of SamplingProfiler::enable/disable.
  bool enable();
  bool disable();

  /// \return true if the sampling profiler is enabled, false otherwise.
  bool enabled();

  /// Register the \p profiler associated with an active runtime.
  /// Should only be called from the thread running the hermes runtime
  /// associated with \p profiler.
  void registerRuntime(SamplingProfiler *profiler);

  /// Unregister the active runtime and current thread associated with
  /// \p profiler.
  void unregisterRuntime(SamplingProfiler *profiler);

  /// \return the singleton profiler instance.
  static GlobalProfiler *get();

 protected:
  GlobalProfiler() = default;

  void walkRuntimeStack(SamplingProfiler *profiler);

 private:
  /// Sample stack for a profiler.
  bool sampleStack(SamplingProfiler *localProfiler);

  // Platform-specific hooks.

  /// Platform-specific hook invoked while enabling the sampling profiler. This
  /// hook is invoked prior to creating the sampling thread, and with
  /// this->profilerLock_ lock held. It must \return true if the sampling
  /// profiler can be enabled, and false otherwise.
  bool platformEnable();

  /// Platform-specific hook invoked while disabling the sampling profiler.
  /// This hook is invoked prior to terminating the sampling thread, and with
  /// this->profilerLock_ lock held. It must \return true if the sampling
  /// profiler can be disabled, and false otherwise.
  bool platformDisable();

  /// Platform-specific hook invoked after \p profiler is registered for
  /// sampling profiling. It is invoked with this->profilerLock_ lock held.
  void platformRegisterRuntime(SamplingProfiler *profiler);

  /// Platform-specific hook invoked after \p profiler is removed from the list
  /// of known profilers -- i.e., profiling should stop on \p profiler. It is
  /// invoked with this->profilerLock_ lock held.
  void platformUnregisterRuntime(SamplingProfiler *profiler);

  /// Platform-specific hook invoked after sampleStack() collects the current
  /// stack of \p localProfiler. This is invoked with the
  /// \p localProfiler->runtimeDataLock_ lock held outside of the VM thread.
  void platformPostSampleStack(SamplingProfiler *localProfiler);

  /// Platform-specific hook invoked to suspend the VM thread and perform stack
  /// sampling. Note that this method is invoked with both this->profilerLock_
  /// and \p profiler->runtimeDataLock_ locks held.
  bool platformSuspendVMAndWalkStack(SamplingProfiler *profiler);
};
} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#endif // HERMES_VM_PROFILER_GLOBALPROFILER_H
