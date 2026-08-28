/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>
#include <utility>

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

void SerialExecutor::add(llvh::unique_function<void()> task) {
  std::unique_lock<std::mutex> lock(mutex_);
  // Draining inside ~SerialExecutor destroys tasks, and a task's captured
  // state may enqueue more work from its destructor. That is fine: the drain
  // loop runs until the queue is empty, so it picks the new task up. Enqueuing
  // from any other thread once teardown has begun is a lifetime bug, because
  // the worker may already have passed the drain loop.
  assert(
      (threadState_ != ThreadState::Terminating ||
       workerThreadId_ == std::this_thread::get_id()) &&
      "Adding tasks during teardown from outside the worker thread.");

  // If the thread is exiting or has already exited, we need to create a new
  // one. Terminating is the exception: the worker is still draining and will
  // pick this task up itself. Spawning another one there would overwrite tid_
  // out from under the join in ~SerialExecutor and reset threadState_, so the
  // drain loop would never see Terminating and both workers would park.
  if (threadState_ != ThreadState::Initialized &&
      threadState_ != ThreadState::Terminating) {
    assert(tasks_.empty() && "Exited thread should have drained tasks.");

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
    pthread_attr_t attr;

    // Ensure the old thread is completely dead. Note that we don't need to
    // release the mutex here because on timeouts, ThreadState::TimedOut is set
    // by the worker right before returning.
    if (threadState_ == ThreadState::TimedOut)
      pthread_join(tid_, nullptr);

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
    // Ensure the old thread is completely dead.
    if (threadState_ == ThreadState::TimedOut)
      workerThread_.join();
    workerThread_ = std::thread(threadMain, this);
#endif

    threadState_ = ThreadState::Initialized;
  }

  // Moving a unique_function always empties the source, so the queue is the
  // sole owner of the task before we release the lock. Otherwise the caller
  // could drop the last reference to the task's captured state -- for
  // finalizers, the object being finalized -- after the worker has already
  // run, destroying it on the calling thread instead of the worker.
  tasks_.push_back(std::move(task));
  wakeUpSig_.notify_one();
}

void SerialExecutor::run() {
  std::unique_lock<std::mutex> lock(mutex_);
  workerThreadId_ = std::this_thread::get_id();
  while (true) {
    while (!tasks_.empty()) {
      // The move empties the queue element, so pop_front() destroys an empty
      // shell and cannot run the task's captured destructors under the lock.
      llvh::unique_function<void()> task = std::move(tasks_.front());
      tasks_.pop_front();
      // Make sure we do *NOT* hold a lock to mutex_ as we execute the given
      // task. Otherwise, this can lead to a deadlock if the given task calls
      // add(), which in turn requests a lock to mutex_.  However, we do want to
      // hold the lock anytime that we interact with the tasks queue.
      lock.unlock();
      task();
      // Destroy the task before re-acquiring the lock. Its captured state
      // belongs to the caller and destroying it runs caller code, which may
      // call add() just as the task body may.
      task = nullptr;
      lock.lock();
    }
    if (threadState_ == ThreadState::Terminating) {
      // This thread is on its way out, so stop claiming to be the worker.
      // Both exits do this while still holding the lock.
      workerThreadId_ = std::thread::id{};
      return;
    }

    // Wait for a new task to be enqueued, or for the timeout to expire. Note we
    // do this at the end to ensure that any tasks added before we entered this
    // function are executed.
    auto waitRes = wakeUpSig_.wait_for(lock, timeout_);

    // Timed out and there is nothing to do, exit.
    if (waitRes == std::cv_status::timeout && tasks_.empty()) {
      threadState_ = ThreadState::TimedOut;
      workerThreadId_ = std::thread::id{};
      return;
    }
  }
}

void *SerialExecutor::threadMain(void *p) {
  auto *executor = static_cast<SerialExecutor *>(p);
  if (executor->threadRunner_) {
    executor->threadRunner_([executor] { executor->run(); });
  } else {
    executor->run();
  }
  return nullptr;
}

} // namespace hermes
