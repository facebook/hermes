/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/instrumentation/StatSamplingThread.h"

#include <cassert>

namespace hermes {
namespace vm {

StatSamplingThread::StatSamplingThread(StatSamplingThread::Duration interval)
    : mExit_(),
      interval_(interval),
      stats_(),
      // Sampler thread executes this.run()
      sampler_(&StatSamplingThread::run, this) {}

StatSamplingThread::~StatSamplingThread() {
  if (isRunning()) {
    (void)stop();
  }
}

bool StatSamplingThread::isRunning() const {
  return sampler_.joinable();
}

ProcessStats::Info StatSamplingThread::stop() {
  assert(isRunning() && "Can only stop thread once.");

  {
    ExitGuard g(mExit_);
    stop_ = true;
  }
  exitMonitor_.notify_one();
  sampler_.join();

  return info();
}

ProcessStats::Info StatSamplingThread::info() const {
  assert(
      !isRunning() &&
      "Requesting info while the sampling thread is running could cause races");
  return stats_.getIntegratedInfo();
}

void StatSamplingThread::run() {
  ExitGuard l(mExit_);
  while (!stop_) {
    stats_.sample(Clock::now());
    exitMonitor_.wait_for(l, interval_);
  }
}

} // namespace vm
} // namespace hermes
