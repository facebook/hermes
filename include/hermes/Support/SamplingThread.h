/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SAMPLINGTHREAD_H
#define HERMES_SUPPORT_SAMPLINGTHREAD_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace hermes {

/// Manages a thread that samples an atomic T at a given interval.
/// Despite managing another thread itself, this class is not thread-safe.
template <typename T>
class SamplingThread {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Duration = Clock::duration;
  using Samples = std::vector<std::pair<TimePoint, T>>;

  /// Creates a new instance
  /// \p value The value to be sampled.
  /// \p interval The approximate period of time to wait between samples.
  SamplingThread(const std::atomic<T> &value, Duration interval)
      : mExit_(),
        value_(value),
        interval_(interval),
        startTime_(Clock::now()),
        samples_(),
        sampler_(&SamplingThread::run, this) {}

  ~SamplingThread() {
    if (isRunning()) {
      (void)stop();
    }
  }

  /// \return True if and only if the sampling thread is still running.
  bool isRunning() const {
    return sampler_.joinable();
  }

  /// When sampling began.
  TimePoint startTime() const {
    return startTime_;
  }

  /// Stop sampling and \return the samples collected. The thread must be
  /// running when calling this method.
  Samples stop() {
    assert(isRunning() && "Can only stop thread once.");
    {
      ExitGuard g(mExit_);
      stop_ = true;
    }
    exitMonitor_.notify_one();
    sampler_.join();
    return std::move(samples_);
  }

 private:
  using ExitGuard = std::unique_lock<std::mutex>;

  /// Used to co-ordinate the shutdown of the sampling thread.
  bool stop_{false};

  /// Guards stop_
  std::mutex mExit_;
  std::condition_variable exitMonitor_;

  const std::atomic<T> &value_;
  Duration interval_;
  const std::chrono::time_point<Clock> startTime_;
  Samples samples_;
  std::thread sampler_;

  /// The code to run in the sampling thread.
  void run() {
    ExitGuard l(mExit_);
    while (!stop_) {
      samples_.emplace_back(
          Clock::now(), value_.load(std::memory_order_relaxed));
      exitMonitor_.wait_for(l, interval_);
    }
  }
};

} // namespace hermes

#endif // HERMES_SUPPORT_SAMPLINGTHREAD_H
