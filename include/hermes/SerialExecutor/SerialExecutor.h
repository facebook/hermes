/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SERIALEXECUTOR_SERIALEXECUTOR_H
#define HERMES_SERIALEXECUTOR_SERIALEXECUTOR_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <thread>
#endif

namespace hermes {

// Simple executor that guarantees serial execution of tasks. If there are
// remaining tasks in the queue when the SerialExecutor destructs, they will not
// be processed.
class SerialExecutor {
 private:
  // The thread on which all work is done.
#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  pthread_t tid_;
#else
  std::thread workerThread_;
#endif

  // A list of functions to execute on the worker thread.
  std::deque<std::function<void()>> tasks_;

  // Access to tasks_ is guarded by this mutex.
  std::mutex mutex_;

  // This is used to put the executor thread to sleep when there is nothing to
  // do, and wake it up when that changes.
  std::condition_variable wakeUpSig_;

  // Indicates to run() that it should stop.
  bool shouldStop_{false};

  /// This is executed on a new thread. It will run forever, executing tasks as
  /// they are posted. This stops running when shouldStop_ is set to true.
  void run();

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
  /// Main function of the new thread.
  static void *threadMain(void *p);
#endif

 public:
  /// Construct a thread which will run for the duration of this object's
  /// lifetime.
  SerialExecutor(size_t stackSize = 0);

  /// Make sure that the spawned thread has terminated. Will block if there is a
  /// long-running task currently being executed.
  ~SerialExecutor();

  /// Push a task to the back of the queue.
  void add(std::function<void()> task);
};
} // namespace hermes

#endif // HERMES_SERIALEXECUTOR_SERIALEXECUTOR_H
