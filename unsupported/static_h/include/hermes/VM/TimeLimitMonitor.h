/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_TIMELIMITMONITOR_H
#define HERMES_VM_TIMELIMITMONITOR_H

#include "hermes/VM/Runtime.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

namespace hermes {
namespace vm {

/// In charge of monitoring runtime execution time for a specified limit.
/// Once a runtime exceeds its execution time limit, it will be notified via
/// an async break request which an AsyncBreakCheck instruction will check and
/// perform corresponding action (e.g., terminate execution, if the monitor is
/// being used to prevent infinite executions...).
class TimeLimitMonitor {
 public:
  /// \return the singleton instance reference.
  static TimeLimitMonitor &getInstance();

  /// Watch \p runtime for timeout after \p timeoutInMs.
  void watchRuntime(Runtime &runtime, int timeoutInMs);

  /// Unwatch \p runtime.
  void unwatchRuntime(Runtime &runtime);

 private:
  TimeLimitMonitor() = default;

  /// \return next closest deadline to wakeup.
  std::chrono::steady_clock::time_point getNextDeadline();

  /// Process any expired work items in timeoutMap_ and remove them.
  /// This method assumes caller has acquired timeoutMapMtx_.
  void processAndRemoveExpiredItems();

  /// Timer loop that wake periodically to process expired work items.
  void timerLoop();

  /// Notify \p runtime to check timeout.
  void notifyRuntimeTimeout(Runtime *runtime) {
    runtime->triggerTimeoutAsyncBreak();
  }

 private:
  /// Mutex that protects timeoutMap_, and state_.
  std::mutex timeoutMapMtx_;
  /// Map from runtime to its deadline time point.
  std::unordered_map<Runtime *, std::chrono::steady_clock::time_point>
      timeoutMap_;

  /// State of the timer loop.
  enum State {
    JOINED, // Thread was joined or has not yet been created.
    RUNNING, // Loop can process work items.
    STOPPING, // Transient state: loop will stop.
    STOPPED, // Transient state: thread will be joined.
  };
  State state_{JOINED};

  /// Signals an addition to timeoutMap_ or change to state_.
  std::condition_variable cond_;

  std::thread timerThread_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_TIMELIMITMONITOR_H
