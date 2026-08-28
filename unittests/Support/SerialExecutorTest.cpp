/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SerialExecutor.h"
#include "llvh/ADT/ScopeExit.h"

#include "gtest/gtest.h"

#include <atomic>
#include <future>
#include <memory>
#include <thread>

namespace {

/// Captured state that enqueues a follow-up task when it is destroyed.
class EnqueueOnDestruction {
 public:
  EnqueueOnDestruction(
      hermes::SerialExecutor &executor,
      std::atomic<bool> &followUpRan)
      : executor_(executor), followUpRan_(followUpRan) {}

  ~EnqueueOnDestruction() {
    executor_.add([&followUpRan = followUpRan_] { followUpRan = true; });
  }

 private:
  hermes::SerialExecutor &executor_;
  std::atomic<bool> &followUpRan_;
};

/// Destroy \p executor on another thread, giving up after \p limit. Returns
/// false if the destructor is still running by then, which means it
/// deadlocked. The caller must not touch \p executor afterwards: the wedged
/// thread is detached and the executor leaked, because joining either one
/// would hang the whole test binary instead of reporting a failure.
bool destroyWithin(
    hermes::SerialExecutor *executor,
    std::chrono::seconds limit) {
  auto destroyed = std::make_shared<std::promise<void>>();
  auto finished = destroyed->get_future();
  std::thread destroyer([executor, destroyed] {
    delete executor;
    destroyed->set_value();
  });
  if (finished.wait_for(limit) == std::future_status::timeout) {
    destroyer.detach();
    return false;
  }
  destroyer.join();
  return true;
}

/// Wait until \p predicate holds, giving up after a deadline so that a broken
/// executor fails the test instead of hanging it.
template <typename Predicate>
bool waitFor(Predicate predicate) {
  const auto giveUpAt =
      std::chrono::steady_clock::now() + std::chrono::seconds(30);
  while (!predicate()) {
    if (std::chrono::steady_clock::now() > giveUpAt)
      return false;
    // Yield rather than spin: on a single-core host a tight loop here can
    // starve the worker thread we are waiting on.
    std::this_thread::yield();
  }
  return true;
}

TEST(SerialExecutorTest, TestBasic) {
  hermes::SerialExecutor executor;
  std::atomic<int> counter{0};

  for (int i = 0; i < 100; ++i)
    executor.add([&counter] { ++counter; });

  while (counter < 100) {
  }
}

TEST(SerialExecutorTest, DestructorDrainsTasks) {
  int counter{0};
  {
    hermes::SerialExecutor executor;

    for (int i = 0; i < 100; ++i)
      executor.add([&counter] { ++counter; });
  }
  ASSERT_EQ(counter, 100);
}

TEST(SerialExecutorTest, TestTimeout) {
  // Set up an executor with a short timeout.
  constexpr std::chrono::milliseconds timeout{10};
  hermes::SerialExecutor executor{0, timeout};
  std::atomic<int> counter{0};

  for (int i = 0; i < 5; ++i) {
    auto t0 = std::chrono::steady_clock::now();

    // Add a task that sets up a thread-local destructor that will increment the
    // counter. This allows us to observe when the thread is destroyed by the
    // executor.
    executor.add([&counter] {
      thread_local auto se = llvh::make_scope_exit([&counter] { ++counter; });
    });

    // Wait for the counter to be incremented when the thread is joined.
    while (counter == i) {
    }

    auto t1 = std::chrono::steady_clock::now();
    ASSERT_GE(t1 - t0, timeout);
  }
}

TEST(SerialExecutorTest, ThreadRunnerWrapsEachWorker) {
  constexpr std::chrono::milliseconds timeout{10};
  std::atomic<int> starts{0};
  std::atomic<int> exits{0};
  std::atomic<int> tasks{0};
  hermes::SerialExecutor executor{0, timeout, [&](std::function<void()> run) {
                                    ++starts;
                                    run();
                                    ++exits;
                                  }};

  EXPECT_EQ(starts, 0);
  for (int i = 1; i <= 5; ++i) {
    executor.add([&tasks] { ++tasks; });

    ASSERT_TRUE(waitFor([&tasks, i] { return tasks >= i; }))
        << "task " << i << " never ran";
    // The worker has to time out and exit before the next add() can create a
    // new one, which is what makes starts/exits countable below.
    ASSERT_TRUE(waitFor([&exits, i] { return exits >= i; }))
        << "worker " << i << " never returned from its runner";
  }

  EXPECT_EQ(starts, 5);
  EXPECT_EQ(exits, 5);
}

TEST(SerialExecutorTest, TaskDestructorCanEnqueue) {
  // A task's captured state belongs to the caller, so destroying a finished
  // task runs caller code that may enqueue more work -- a JSI finalizer that
  // releases another host object does exactly this. The worker must therefore
  // drop the task before re-acquiring mutex_, which is not recursive.
  auto executor = std::make_unique<hermes::SerialExecutor>();
  std::atomic<bool> releaseTask{false};
  std::atomic<bool> followUpRan{false};

  // The state is held through a shared_ptr so that its destructor runs exactly
  // once, when the last copy dies.
  auto owner = std::make_shared<EnqueueOnDestruction>(*executor, followUpRan);
  executor->add([owner, &releaseTask] {
    // Stay in the task body until the test has dropped its own reference, so
    // that the worker is the one holding the last one.
    while (!releaseTask)
      std::this_thread::yield();
  });
  owner.reset();
  releaseTask = true;

  const bool ranFollowUp =
      waitFor([&followUpRan] { return followUpRan.load(); });
  if (!ranFollowUp) {
    // The worker is wedged waiting on a mutex it already holds, so joining it
    // would hang the whole binary. Leak the executor so the failure is
    // reported instead.
    (void)executor.release();
  }
  ASSERT_TRUE(ranFollowUp) << "a task destructor could not call add()";
}

TEST(SerialExecutorTest, TaskDestructorCanEnqueueWhileDraining) {
  // ~SerialExecutor sets Terminating before the worker drains, so tasks
  // destroyed during the drain enqueue their follow-up work with teardown
  // already under way. That is legal from the worker itself, because the drain
  // loop runs until the queue is empty.
  //
  // Whether the drain or the worker wins the race is not controlled here, so
  // this only sometimes exercises the Terminating branch. It never reports a
  // false failure: the follow-up has to run either way.
  std::atomic<bool> followUpRan{false};
  {
    hermes::SerialExecutor executor;
    auto owner = std::make_shared<EnqueueOnDestruction>(executor, followUpRan);
    executor.add([owner] {});
    owner.reset();
  }
  EXPECT_TRUE(followUpRan) << "a follow-up enqueued while draining never ran";
}

TEST(SerialExecutorTest, TaskCanEnqueueWhileDraining) {
  // The general form of the case above, with no destructors involved: a task
  // that is still running when ~SerialExecutor begins enqueues its follow-up
  // with threadState_ already Terminating. add() must not read that as "the
  // worker is gone" and spawn a second one. Doing so overwrites tid_ out from
  // under the join, and resets threadState_ so the drain loop never sees
  // Terminating, leaving both workers parked on the condition variable.
  //
  // The timeout is finite on purpose. With the default of milliseconds::max()
  // the wait overflows to a deadline in the past and returns immediately, so
  // stranded workers exit by accident and hide the deadlock.
  std::atomic<bool> release{false};
  std::atomic<bool> followUpRan{false};
  auto *executor = new hermes::SerialExecutor(0, std::chrono::hours(1));

  executor->add([executor, &release, &followUpRan] {
    // Stay in the body until teardown has plausibly begun, so the add() below
    // runs while Terminating. Losing that race only costs coverage; it cannot
    // produce a false failure.
    while (!release)
      std::this_thread::yield();
    executor->add([&followUpRan] { followUpRan = true; });
  });

  std::thread releaser([&release] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    release = true;
  });

  const bool destroyed = destroyWithin(executor, std::chrono::seconds(10));
  releaser.join();
  ASSERT_TRUE(destroyed) << "~SerialExecutor deadlocked while draining";
  EXPECT_TRUE(followUpRan) << "a follow-up enqueued while draining never ran";
}

TEST(SerialExecutorTest, TaskDestroyedOnWorkerThread) {
  // add() must leave the queue as the sole owner of the task, so that the
  // task's captured state is destroyed on the worker rather than on whichever
  // thread called add(). JSI finalizers depend on this: keeping clean-up off
  // the calling thread is the entire point of the executor.
  //
  // The capture below is move-only, which is what makes the transfer
  // structural. Copying the task, or going back to std::function, would stop
  // compiling rather than silently reintroduce the race.
  std::atomic<std::thread::id> ranOn{std::thread::id{}};
  std::atomic<std::thread::id> destroyedOn{std::thread::id{}};

  /// Records the thread that destroys it.
  class RecordDestroyingThread {
   public:
    explicit RecordDestroyingThread(std::atomic<std::thread::id> &destroyedOn)
        : destroyedOn_(destroyedOn) {}

    ~RecordDestroyingThread() {
      destroyedOn_ = std::this_thread::get_id();
    }

   private:
    std::atomic<std::thread::id> &destroyedOn_;
  };

  {
    hermes::SerialExecutor executor;
    executor.add([&ranOn,
                  recorder = std::make_unique<RecordDestroyingThread>(
                      destroyedOn)] { ranOn = std::this_thread::get_id(); });
    // ~SerialExecutor drains and joins, so both ids are set once it returns.
  }

  ASSERT_NE(ranOn.load(), std::thread::id{}) << "the task never ran";
  EXPECT_EQ(destroyedOn.load(), ranOn.load())
      << "the task was destroyed off the worker thread";
  EXPECT_NE(destroyedOn.load(), std::this_thread::get_id())
      << "the task was destroyed on the thread that called add()";
}

} // end anonymous namespace
