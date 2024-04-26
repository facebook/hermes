/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SerialExecutor.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

SerialExecutor::SerialExecutor(size_t stackSize) {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  pthread_attr_t attr;

  int ret;
  ret = pthread_attr_init(&attr);
  if (ret != 0) {
    throw std::runtime_error("Failed pthread_attr_init");
  }

  if (stackSize != 0) {
    ret = pthread_attr_setstacksize(&attr, stackSize);
    if (ret != 0) {
      throw std::runtime_error("Failed pthread_attr_setstacksize");
    }
  }

  ret = pthread_create(&tid_, &attr, SerialExecutor::threadMain, this);
  if (ret != 0) {
    throw std::runtime_error("Failed pthread_create");
  }
#else
  workerThread_ = std::thread([this]() { this->run(); });
#endif
}

SerialExecutor::~SerialExecutor() {
  // Tell the worker thread to stop, then wait for it to do so.
  {
    std::unique_lock<std::mutex> lock(mutex_);
    shouldStop_ = true;
    wakeUpSig_.notify_one();
  }
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  pthread_join(tid_, nullptr);
#else
  workerThread_.join();
#endif
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

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
void *SerialExecutor::threadMain(void *p) {
  static_cast<SerialExecutor *>(p)->run();
  pthread_exit(nullptr);
  return nullptr;
}
#endif

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
