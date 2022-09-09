/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/TimeLimitMonitor.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Runtime.h"

#include <algorithm>

namespace hermes {
namespace vm {

std::shared_ptr<TimeLimitMonitor> TimeLimitMonitor::getOrCreate() {
  /// The singleton payload, intentionally leaked to avoid destruction order
  /// fiasco.
  static struct Singleton {
    /// Synchronizes access to weakMonitor.
    std::mutex guard;

    /// Holds a weak reference to the current TimeLimitMonitor. This ensure that
    /// the monitor is destroyed when there are no more shared pointers to it,
    /// while still allowing this method to re-use the same singleton for as
    /// long as possible.
    std::weak_ptr<TimeLimitMonitor> weakMonitor;
  } *singleton = new Singleton();

  std::lock_guard<std::mutex> singletonGuard{singleton->guard};
  if (auto monitor = singleton->weakMonitor.lock()) {
    return monitor;
  }

  // Create the new monitor instance while saving a reference in the
  // singleton's weakMonitor member.
  auto monitor = std::make_shared<TimeLimitMonitor>();
  singleton->weakMonitor = monitor;
  return monitor;
}

TimeLimitMonitor::TimeLimitMonitor() {
  // Spawns a new thread that performs the time limit monitoring. This thread
  // is joined in the destructor.
  timerThread_ = std::thread(&TimeLimitMonitor::timerLoop, this);
}

TimeLimitMonitor::~TimeLimitMonitor() {
  {
    std::unique_lock<std::mutex> lockGuard(lock_);
    // Flipping enabled_ to false so the timeLoop thread will stop iterating and
    // allow the thread to complete.
    enabled_ = false;
  }

  // Tickle the timeLoop thread so it can terminate.
  timerLoopCond_.notify_all();

  /// And wait for the helper thread termination.
  timerThread_.join();
}

void TimeLimitMonitor::watchRuntime(
    Runtime &runtime,
    std::chrono::milliseconds timeout) {
  {
    std::lock_guard<std::mutex> lockGuard(lock_);
    // The execution deadline for runtime.
    auto runtimeDeadline = std::chrono::steady_clock::now() + timeout;

    // Set runtime's deadline to runtimeDeadline. This overwrites the current
    // deadline (if any).
    watchedRuntimes_[&runtime] = runtimeDeadline;
  }

  // Notify the timer loop so it updates the wait timeout in case
  // runtimeDeadline is earlier than the current earliest deadline being
  // tracked.
  timerLoopCond_.notify_all();
}

void TimeLimitMonitor::unwatchRuntime(Runtime &runtime) {
  std::lock_guard<std::mutex> lockGuard(lock_);
  watchedRuntimes_.erase(&runtime);

  // No need to notify the timer loop, even if runtime had the current earliest
  // deadline being watched -- in which case the timerLoop will run once,
  // figure out no runtimes timed out, and sleep again.
}

void TimeLimitMonitor::timerLoop() {
  oscompat::set_thread_name("time-limit-monitor");

  std::unique_lock<std::mutex> lockGuard(lock_);

  // Indicates that there currently is no deadline being "watched" for, so the
  // timerLoop should sleep until there's work to be done.
  static constexpr Deadline NoDeadline = Deadline::max();

  while (enabled_) {
    // Assume there's no deadline in watchedRuntimes_ -- i.e., no Runtimes are
    // being watched.
    Deadline nextDeadline = NoDeadline;
    Deadline now = std::chrono::steady_clock::now();

    for (auto it = watchedRuntimes_.begin(), end = watchedRuntimes_.end();
         it != end;) {
      auto curr = it++;

      if (curr->second <= now) {
        // curr's deadline has elapsed; notify the VM.
        curr->first->triggerTimeoutAsyncBreak();

        // We're done watching the VM, so remove curr from watchedRuntimes_.
        watchedRuntimes_.erase(curr);
      } else {
        // curr's deadline is in the future, but it could be ealier than the
        // nextDeadline; Update the latter accordingly.
        nextDeadline = std::min(nextDeadline, curr->second);
      }
    }

    if (nextDeadline != NoDeadline) {
      // Sleep until the next deadline is reached, or the notification comes in
      // (e.g., time limit monitoring has been disabled and the thread needs to
      // terminate). Note that spurious wake ups are OK -- it just means that no
      // timeouts will have passed, and the thread will go back to waiting.
      timerLoopCond_.wait_until(lockGuard, nextDeadline);
    } else {
      // Work around overflow issues in some libstdcxx implementations by using
      // wait() when there's no active deadline. The timerLoop will be notified
      // by the VM thread when a new Runtime is registered or destroyed.
      timerLoopCond_.wait(lockGuard);
    }
  }
}

} // namespace vm
} // namespace hermes
