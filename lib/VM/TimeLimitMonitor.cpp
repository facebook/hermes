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

TimeLimitMonitor::~TimeLimitMonitor() {
  // Signal worker thread to exit before shutting down. Otherwise
  // main thread may deadlock waiting for destroying newRequestCond_ condition
  // variable.
  {
    std::lock_guard<std::mutex> lock(timeoutMapMtx_);
    shouldExit_ = true;
  }
  newRequestCond_.notify_one();
  // Since newRequestCond_  may still be used by worker thread, wait it to die
  // before destroying member fields.
  if (timerThread_.joinable()) {
    timerThread_.join();
  }
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
  std::unique_lock<std::mutex> lock(timeoutMapMtx_);
  while (!shouldExit_) {
    std::chrono::steady_clock::time_point wakeupTime = getNextDeadline();

    // This wait can wakeup for three different reasons:
    // 1. timeout
    // 2. new request signal
    // 3. condition variable spurious wakeup
    //
    // Regardless of the reasons we all wanted to do the same things:
    // 1. Process expired work items
    // 2. Wait for next closest deadline
    newRequestCond_.wait_until(lock, wakeupTime);

    processAndRemoveExpiredItems();
  }
}

void TimeLimitMonitor::watchRuntime(Runtime &runtime, int timeoutInMs) {
  {
    std::lock_guard<std::mutex> lock(timeoutMapMtx_);
    createTimerLoopIfNeeded();
    auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(timeoutInMs);
    timeoutMap_[&runtime] = deadline;
  }

  runtime.registerDestructionCallback(
      [this](Runtime &runtime) { this->unwatchRuntime(runtime); });

  // There is only one thread anyway.
  newRequestCond_.notify_one();
}

void TimeLimitMonitor::unwatchRuntime(Runtime &runtime) {
  std::lock_guard<std::mutex> lock(timeoutMapMtx_);
  // unwatchRuntime() may be called multiple times for the same runtime.
  if (timeoutMap_.find(&runtime) != timeoutMap_.end()) {
    timeoutMap_.erase(&runtime);
  }
}

} // namespace vm
} // namespace hermes
