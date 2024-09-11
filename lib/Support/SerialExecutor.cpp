/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>

#include "hermes/Support/SerialExecutor.h"

namespace hermes {

SerialExecutor::~SerialExecutor() {
  // Tell the worker thread to stop, then wait for it to do so.
  ThreadState oldState;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    oldState = std::exchange(threadState_, ThreadState::Terminating);
    wakeUpSig_.notify_one();
  }
  // If there was a thread, join it.
  if (oldState != ThreadState::Uninitialized) {
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
    pthread_join(tid_, nullptr);
#else
    workerThread_.join();
#endif
  }
  assert(tasks_.empty() && "Thread should have drained all tasks.");
}

void SerialExecutor::add(std::function<void()> task) {
  std::unique_lock<std::mutex> lock(mutex_);
  assert(
      threadState_ != ThreadState::Terminating &&
      "Adding tasks during teardown.");

  // If the thread is exiting or has already exited, we need to create a new
  // one.
  if (threadState_ != ThreadState::Initialized) {
    assert(tasks_.empty() && "Exited thread should have drained tasks.");

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
    pthread_attr_t attr;

    int ret;
    (void)ret;
    ret = pthread_attr_init(&attr);
    assert(ret == 0 && "Failed pthread_attr_init");

    if (stackSize_ != 0) {
      ret = pthread_attr_setstacksize(&attr, stackSize_);
      assert(ret == 0 && "Failed pthread_attr_setstacksize");
    }

    ret = pthread_create(&tid_, &attr, SerialExecutor::threadMain, this);
    assert(ret == 0 && "Failed pthread_create");
#else
    workerThread_ = std::thread(threadMain, this);
#endif

    threadState_ = ThreadState::Initialized;
  }

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
    if (threadState_ == ThreadState::Terminating) {
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
