/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/RuntimeStats.h"

#include "hermes/Support/OSCompat.h"

namespace hermes {
namespace vm {
namespace instrumentation {

void RuntimeStats::flushPendingTimers() {
  for (auto cursor = timerStack; cursor != nullptr; cursor = cursor->parent_) {
    cursor->flush();
  }
}

RuntimeStats::Sampled RAIITimer::trySampling() const {
  if (!runtimeStats_.shouldSample)
    return {};

  RuntimeStats::Sampled result{};
  if (!oscompat::thread_page_fault_count(
          &result.threadMinorFaults, &result.threadMajorFaults) ||
      !oscompat::num_context_switches(
          result.volCtxSwitches, result.involCtxSwitches))
    return {};
  return result;
}

RAIITimer::RAIITimer(
    const char *name,
    RuntimeStats &runtimeStats,
    RuntimeStats::Statistic &stat)
    : perfSection_(name),
      runtimeStats_(runtimeStats),
      stat_(stat),
      parent_(runtimeStats.timerStack),
      wallTimeStart_(std::chrono::steady_clock::now()),
      cpuTimeStart_(oscompat::thread_cpu_time()),
      sampledStart_(trySampling()) {
  runtimeStats.timerStack = this;
  stat_.count += 1;
}

void RAIITimer::flush() {
  auto currentCPUTime = oscompat::thread_cpu_time();
  auto currentWallTime = std::chrono::steady_clock::now();
  auto currentSampled = trySampling();
  stat_.wallDuration +=
      std::chrono::duration<double>(currentWallTime - wallTimeStart_).count();
  stat_.cpuDuration +=
      std::chrono::duration<double>(currentCPUTime - cpuTimeStart_).count();
  stat_.sampled.threadMinorFaults +=
      currentSampled.threadMinorFaults - sampledStart_.threadMinorFaults;
  stat_.sampled.threadMajorFaults +=
      currentSampled.threadMajorFaults - sampledStart_.threadMajorFaults;
  stat_.sampled.volCtxSwitches +=
      currentSampled.volCtxSwitches - sampledStart_.volCtxSwitches;
  stat_.sampled.involCtxSwitches +=
      currentSampled.involCtxSwitches - sampledStart_.involCtxSwitches;
  wallTimeStart_ = currentWallTime;
  cpuTimeStart_ = currentCPUTime;
  sampledStart_ = currentSampled;
}

RAIITimer::~RAIITimer() {
  flush();
  assert(
      runtimeStats_.timerStack == this &&
      "Destroyed RAIITimer is not at top of stack");
  runtimeStats_.timerStack = parent_;
}

} // namespace instrumentation
} // namespace vm
} // namespace hermes
