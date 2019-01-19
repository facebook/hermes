/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_RUNTIMESTATS_H
#define HERMES_VM_RUNTIMESTATS_H

#include <stdint.h>

namespace hermes {
namespace vm {
namespace instrumentation {

class RAIITimer;

/// RuntimeStats contains statistics which may be manipulated by users of
/// Runtime.
struct RuntimeStats {
  /// The following properties are tracked on a sampling basis due to the kernel
  /// calls necessary to fetch them.
  struct Sampled {
    int64_t threadMinorFaults{0};
    int64_t threadMajorFaults{0};
  };

  /// A Statistic tracks duration in wall and CPU time, (optionally) the number
  /// of minor and major faults, and a count.  All times are in seconds.
  struct Statistic {
    double wallDuration{0};
    double cpuDuration{0};
    Sampled sampled{};
    uint64_t count{0};
  };

  RuntimeStats(bool shouldSample) : shouldSample(shouldSample) {}

  /// Measure of host function callouts (outgoing from VM).
  Statistic hostFunction;

  /// Measure of evaluateJavaScript calls.
  Statistic evaluateJS;

  /// Measure of of jsi Function calls (incoming to VM).
  Statistic incomingFunction;

  /// The topmost RAIITimer in the stack.
  RAIITimer *timerStack{nullptr};

  /// Whether to collect sampled statistics. This should be set at runtime
  /// initialization. It is const to enforce that fact that it cannot be toggled
  /// later (if it were so toggled, the results would be missing some data).
  const bool shouldSample{false};

  /// Flush all timers pending in our timer stack.
  inline void flushPendingTimers();
};

/// An RAII-style class for updating a Statistic.
class RAIITimer {
  friend RuntimeStats;
  /// The RuntimeStats we are updating. This is stored so we can manipulate its
  /// timerStack.
  RuntimeStats &runtimeStats_;

  /// The particular statistic we are updating.
  RuntimeStats::Statistic &stat_;

  /// The parent timer. This link forms a stack. At the point the stats are
  /// collected, all existing RAIITimers are flushed so that pending data can be
  /// collected.
  RAIITimer *const parent_;

  /// The initial value of the wall time.
  std::chrono::steady_clock::time_point wallTimeStart_;

  /// The initial value of the CPU time.
  std::chrono::microseconds cpuTimeStart_;

  /// Initial values of sampled statistics, or default values if sampling is
  /// disabled.
  RuntimeStats::Sampled sampledStart_{};

  /// If sampling is enabled, collect the sampled stats and return them.
  /// If sampling is not enabled, or on error, returns default stats.
  /// \return the sampled or default results.
  RuntimeStats::Sampled trySampling() const {
    if (!runtimeStats_.shouldSample)
      return {};

    RuntimeStats::Sampled result{};
    if (!oscompat::thread_page_fault_count(
            &result.threadMinorFaults, &result.threadMajorFaults))
      return {};
    return result;
  }

 public:
  explicit RAIITimer(RuntimeStats &runtimeStats, RuntimeStats::Statistic &stat)
      : runtimeStats_(runtimeStats),
        stat_(stat),
        parent_(runtimeStats.timerStack),
        wallTimeStart_(std::chrono::steady_clock::now()),
        cpuTimeStart_(oscompat::thread_cpu_time()),
        sampledStart_(trySampling()) {
    runtimeStats.timerStack = this;
    stat_.count += 1;
  }

  /// Flush the timer to the referenced statistic, resetting the start times.
  /// Note that 'count' is incremented when the RAIITimer is created and so is
  /// unaffected. This is invoked when the timer is destroyed, but also invoked
  /// when data is collected to include any aggregate data.
  void flush() {
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
    wallTimeStart_ = currentWallTime;
    cpuTimeStart_ = currentCPUTime;
    sampledStart_ = currentSampled;
  }

  ~RAIITimer() {
    flush();
    assert(
        runtimeStats_.timerStack == this &&
        "Destroyed RAIITimer is not at top of stack");
    runtimeStats_.timerStack = parent_;
  }
};

void RuntimeStats::flushPendingTimers() {
  for (auto cursor = timerStack; cursor != nullptr; cursor = cursor->parent_) {
    cursor->flush();
  }
}

} // namespace instrumentation
} // namespace vm
} // namespace hermes

#endif
