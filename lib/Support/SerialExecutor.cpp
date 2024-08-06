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
  workerThread_ = std::thread(threadMain, this);
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
  std::unique_lock<std::mutex> lock(mutex_);
  while (true) {
    while (!tasks_.empty()) {
      std::function<void()> task = std::move(tasks_.front());
      tasks_.pop_front();
      // Make sure we do *NOT* hold a lock to mutex_ as we execute the given
      // task. Otherwise, this can lead to a deadlock if the given task calls
      // add(), which in turn requests a lock to mutex_.  However, we do want to
      // hold the lock anytime that we interact with the tasks queue.
      lock.unlock();
      task();
      lock.lock();
    }
    if (shouldStop_) {
      return;
    }

    // Wait for a new task to be enqueued. Note we do this at the end to ensure
    // that any tasks added before we entered this function are executed.
    wakeUpSig_.wait(lock);
  }
}

void *SerialExecutor::threadMain(void *p) {
  static_cast<SerialExecutor *>(p)->run();
  return nullptr;
}

} // namespace hermes
