/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>

#include "hermes/Support/SerialExecutor.h"

namespace hermes {

SerialExecutor::SerialExecutor(size_t stackSize) {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  pthread_attr_t attr;

  int ret;
  (void)ret;
  ret = pthread_attr_init(&attr);
  assert(ret == 0 && "Failed pthread_attr_init");

  if (stackSize != 0) {
    ret = pthread_attr_setstacksize(&attr, stackSize);
    assert(ret == 0 && "Failed pthread_attr_setstacksize");
  }

  ret = pthread_create(&tid_, &attr, SerialExecutor::threadMain, this);
  assert(ret == 0 && "Failed pthread_create");

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

    // Make sure we do *NOT* hold a lock to mutex_ as we execute the given
    // task. Otherwise, this can lead to a deadlock if the given task calls
    // add(), which in turn requests a lock to mutex_.  However, we do want to
    // hold the lock anytime that we interact with the tasks queue.
    std::unique_lock<std::mutex> lock(mutex_);

    while (!tasks_.empty()) {
      std::function<void()> task = tasks_.front();
      lock.unlock();
      task();
      lock.lock();
      tasks_.pop_front();
    }

    if (shouldStop_) {
      return;
    }
  }
}

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
void *SerialExecutor::threadMain(void *p) {
  static_cast<SerialExecutor *>(p)->run();
  pthread_exit(nullptr);
  return nullptr;
}
#endif

} // namespace hermes
