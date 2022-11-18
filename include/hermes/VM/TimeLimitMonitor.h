/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_TIMELIMITMONITOR_H
#define HERMES_VM_TIMELIMITMONITOR_H

#include "llvh/ADT/DenseMap.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace hermes {
namespace vm {

class Runtime;

/// In charge of monitoring runtime execution time for a specified limit.
/// Once a runtime exceeds its execution time limit, it will be notified via
/// an async break request which an AsyncBreakCheck instruction will check and
/// perform corresponding action (e.g., terminate execution, if the monitor is
/// being used to prevent infinite executions...).
class TimeLimitMonitor {
 public:
  using Deadline = std::chrono::steady_clock::time_point;

  ~TimeLimitMonitor();

  /// \return The time TimeLimitMonitor singleton. Its life-time is managed by
  /// the returned shared pointer.
  static std::shared_ptr<TimeLimitMonitor> getOrCreate();

  /// Sets a deadline of now + \p timeout (ms) before \p runtime is notified of
  /// a timeout.
  void watchRuntime(Runtime &runtime, std::chrono::milliseconds timeout);

  /// Stops watching \p runtime for timeouts.
  void unwatchRuntime(Runtime &runtime);

  /// Returns the map of currently-watched runtimes. Mostly for testing.
  const llvh::DenseMap<Runtime *, Deadline> &getWatchedRuntimes() const {
    return watchedRuntimes_;
  }

 private:
  /// Timer loop that is in charge of notifying the runtimes in watchedRuntimes_
  /// when their deadline elapses.
  void timerLoop();

  /// Synchronizes access to all member variables.
  std::mutex lock_;

  /// Manages the life-time of the timerLoop thread. Created in the constructor,
  /// and joined in the destructor.
  std::thread timerThread_;

  /// Condition variable used to "tickle" the timer loop thread -- i.e., wake it
  /// up before nextDeadline_ is reached.
  std::condition_variable timerLoopCond_;

  /// Maps all Runtimes being watched to their respective deadlines.
  llvh::DenseMap<Runtime *, Deadline> watchedRuntimes_;

  /// Flag indicating whether the TimeLimitMonitor is enabled or not. Set to
  /// false during destruction to stop the timerLoop thread.
  bool enabled_{true};

 public:
  /// Constructs a new TimeLimitMonitor; only public so instances can be created
  /// with make_shared.
  TimeLimitMonitor();
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_TIMELIMITMONITOR_H
