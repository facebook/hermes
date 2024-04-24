/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SerialExecutor.h"

#include <glog/logging.h>

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

SerialExecutor::SerialExecutor() {
  workerThread_ = std::thread([this]() { this->run(); });
}

SerialExecutor::~SerialExecutor() {
  // Tell the worker thread to stop, then wait for it to do so.
  {
    std::unique_lock<std::mutex> lock(mutex_);
    shouldStop_ = true;
    wakeUpSig_.notify_one();
  }
  workerThread_.join();
}

void SerialExecutor::add(std::function<void()> task) {
  std::unique_lock<std::mutex> lock(mutex_);
  tasks_.push_back(task);
  wakeUpSig_.notify_one();
}

void SerialExecutor::run() {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      wakeUpSig_.wait(lock, [this] { return !tasks_.empty() || shouldStop_; });
    }

    if (shouldStop_) {
      if (tasks_.size()) {
        LOG(WARNING) << "Executor shutdown with " << tasks_.size()
                     << " tasks running";
      }
      return;
    }

    // Make sure we do *NOT* hold a lock to mutex_ as we execute the given
    // task. Otherwise, this can lead to a deadlock if the given task calls
    // add(), which in turn requests a lock to mutex_.  However, we do want to
    // hold the lock anytime that we interact with the tasks queue.
    std::unique_lock<std::mutex> lock(mutex_);
    std::function<void()> task = tasks_.front();
    lock.unlock();
    task();
    lock.lock();
    tasks_.pop_front();
  }
}

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
