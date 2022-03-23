/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SAMPLINGTHREAD_H
#define HERMES_VM_SAMPLINGTHREAD_H

#include "hermes/VM/instrumentation/ProcessStats.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace hermes {
namespace vm {

/// Manages a thread that samples process statistics at a given interval.
/// Despite managing another thread itself, this class is not thread-safe.
class StatSamplingThread {
 public:
  using Clock = std::chrono::steady_clock;
  using Duration = Clock::duration;

  /// Creates a new instance
  /// \p interval The period of time to wait between samples.
  StatSamplingThread(Duration interval);

  ~StatSamplingThread();

  /// \return True if and only if the sampling thread is still running.
  bool isRunning() const;

  /// Stop sampling and \return the statistics collected so far.
  ProcessStats::Info stop();

  /// \return The statistics collected when the sampling thread was running.
  /// \pre The sampling thread must be stopped prior to calling this function.
  ProcessStats::Info info() const;

 private:
  using ExitGuard = std::unique_lock<std::mutex>;

  /// Used to co-ordinate the shutdown of the sampling thread.
  bool stop_{false};

  /// Guards stop_
  std::mutex mExit_;
  std::condition_variable exitMonitor_;

  Duration interval_;
  ProcessStats stats_;
  std::thread sampler_;

  /// The code to run in the sampling thread.
  void run();
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SAMPLINGTHREAD_H
