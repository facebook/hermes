/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/TimeLimitMonitor.h"

namespace hermes {
namespace vm {

const uint32_t kDefaultSleepMs = 5000;

TimeLimitMonitor &TimeLimitMonitor::getInstance() {
  static TimeLimitMonitor instance;
  return instance;
}

std::chrono::steady_clock::time_point TimeLimitMonitor::getNextDeadline() {
  // A min-heap can be used to locate closest deadline more efficient, but did
  // not do it for two reasons:
  // 1. Total number of runtimes are not expected to be large.
  // 2. Performance in TimeLimitMonitor is not the bottleneck.
  auto nextDeadlineIter = std::min_element(
      timeoutMap_.begin(),
      timeoutMap_.end(),
      [](const std::pair<Runtime *, std::chrono::steady_clock::time_point>
             &elem1,
         const std::pair<Runtime *, std::chrono::steady_clock::time_point>
             &elem2) { return elem1.second < elem2.second; });
  return nextDeadlineIter != timeoutMap_.end()
      ? nextDeadlineIter->second
      : std::chrono::steady_clock::now() +
          std::chrono::milliseconds(kDefaultSleepMs);
}

void TimeLimitMonitor::processAndRemoveExpiredItems() {
  auto now = std::chrono::steady_clock::now();
  for (auto iter = timeoutMap_.begin(), end = timeoutMap_.end(); iter != end;) {
    if (iter->second <= now) {
      notifyRuntimeTimeout(iter->first);
      iter = timeoutMap_.erase(iter);
    } else {
      ++iter;
    }
  }
}

void TimeLimitMonitor::timerLoop() {
  {
    std::unique_lock<std::mutex> lock(timeoutMapMtx_);
    while (state_ == RUNNING) {
      std::chrono::steady_clock::time_point wakeupTime = getNextDeadline();

      // This wait can wakeup for three different reasons:
      // 1. timeout
      // 2. new request signal
      // 3. stopping signal
      // 4. condition variable spurious wakeup
      //
      // Regardless of the reasons we all wanted to do the same things:
      // 1. Process expired work items
      // 2. Wait for next closest deadline
      cond_.wait_until(lock, wakeupTime);

      processAndRemoveExpiredItems();
    }
    assert(state_ == STOPPING);
    state_ = STOPPED;
  }

  // Tell unwatchRuntime to join this thread.
  cond_.notify_all();
}

void TimeLimitMonitor::watchRuntime(Runtime &runtime, int timeoutInMs) {
  {
    std::unique_lock<std::mutex> lock(timeoutMapMtx_);
    // Wait for any shutdown that's in progress.
    cond_.wait(
        lock, [this]() { return state_ == RUNNING || state_ == JOINED; });
    if (state_ == JOINED) {
      assert(!timerThread_.joinable());
      timerThread_ = std::thread(&TimeLimitMonitor::timerLoop, this);
      state_ = RUNNING;
    }
    auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(timeoutInMs);
    timeoutMap_[&runtime] = deadline;
  }

  runtime.registerDestructionCallback(
      [this](Runtime &runtime) { this->unwatchRuntime(runtime); });

  // There is new work (and state may have changed).
  cond_.notify_all();
}

void TimeLimitMonitor::unwatchRuntime(Runtime &runtime) {
  {
    std::unique_lock<std::mutex> lock(timeoutMapMtx_);
    // Wait for any shutdown that's in progress.
    cond_.wait(
        lock, [this]() { return state_ == RUNNING || state_ == JOINED; });
    if (state_ == JOINED) {
      // Everything expired and shutdown was done by someone else.
      assert(timeoutMap_.empty() && "should not have work without a thread");
      return;
    }
    // unwatchRuntime should be called at least once for every runtime
    // that is watched. It orchestrates shutdown when it sees
    // timeoutMap_ empty.  The timer loop itself never initiates
    // shutdown, even if a timeout causes the timeoutMap_ to become
    // empty. So we must NOT return early here even if timeoutMap_ is
    // already empty.
    if (timeoutMap_.find(&runtime) != timeoutMap_.end()) {
      timeoutMap_.erase(&runtime);
    }
    // If there are other runtimes being watched, we're done.
    if (!timeoutMap_.empty()) {
      return;
    }
    // Else, we're responsible for shutdown (stop the loop and join the thread).
    state_ = STOPPING;
  }
  // Notify the timer loop that it should stop...
  cond_.notify_all();
  {
    std::unique_lock<std::mutex> lock(timeoutMapMtx_);
    // ... and wait for its final release.
    cond_.wait(lock, [this]() { return state_ == STOPPED; });
    if (timerThread_.joinable()) {
      timerThread_.join();
    }
    state_ = JOINED;
  }
}

} // namespace vm
} // namespace hermes
