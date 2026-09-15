/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include <atomic>
#include <thread>
#include <vector>

namespace hermes {
namespace napi {

//===========================================================================
// Mock event loop for testing async work
//===========================================================================

/// A simple mock event loop that records posted work and allows
/// manual execution and completion dispatch.
struct MockEventLoop {
  struct WorkItem {
    void *work_data;
    void (*execute)(void *work_data);
    void (*complete)(void *work_data, napi_status status);
    bool cancelled = false;
    bool executed = false;
  };

  std::vector<WorkItem> items;

  /// Execute and complete all pending work items synchronously.
  void drainAll() {
    for (auto &item : items) {
      if (!item.cancelled && !item.executed) {
        item.execute(item.work_data);
        item.executed = true;
        item.complete(item.work_data, napi_ok);
      }
    }
  }

  /// Execute all pending work items on a background thread, then
  /// call complete callbacks on the current thread.
  void drainWithThread() {
    for (auto &item : items) {
      if (!item.cancelled && !item.executed) {
        std::thread t([&item]() { item.execute(item.work_data); });
        t.join();
        item.executed = true;
        item.complete(item.work_data, napi_ok);
      }
    }
  }

  /// Cancel a specific work item by work_data pointer. Returns true
  /// if found and cancelled, false if not found or already executed.
  bool cancelItem(void *work_data) {
    for (auto &item : items) {
      if (item.work_data == work_data && !item.executed) {
        item.cancelled = true;
        item.complete(item.work_data, napi_cancelled);
        return true;
      }
    }
    return false;
  }

  /// Clear all recorded items.
  void clear() {
    items.clear();
  }
};

/// C function pointers for hermes_napi_event_loop.
static void mockPostWork(
    void *loop_data,
    void *work_data,
    void (*execute)(void *work_data),
    void (*complete)(void *work_data, napi_status status)) {
  auto *loop = static_cast<MockEventLoop *>(loop_data);
  loop->items.push_back({work_data, execute, complete, false, false});
}

static bool mockCancelWork(void *loop_data, void *work_data) {
  auto *loop = static_cast<MockEventLoop *>(loop_data);
  return loop->cancelItem(work_data);
}

//===========================================================================
// Test fixture with mock event loop
//===========================================================================

class NapiAsyncWorkTest : public ::testing::Test {
 protected:
  std::shared_ptr<vm::Runtime> rt_;
  napi_env env_ = nullptr;
  MockEventLoop mockLoop_;
  hermes_napi_event_loop eventLoop_{};

  void SetUp() override {
    auto config = vm::RuntimeConfig::Builder()
                      .withGCConfig(
                          vm::GCConfig::Builder()
                              .withInitHeapSize(1 << 16)
                              .withMaxHeapSize(1 << 19)
                              .build())
                      .build();
    rt_ = vm::Runtime::create(config);

    eventLoop_.post_work = mockPostWork;
    eventLoop_.cancel_work = mockCancelWork;
    eventLoop_.data = &mockLoop_;

    env_ = hermes_napi_create_env(&*rt_, &eventLoop_);
  }

  void TearDown() override {
    // The env is owned by the Runtime and torn down as part of
    // ~Runtime. Drop the borrowed pointer and reset the runtime.
    env_ = nullptr;
    rt_.reset();
  }
};

//===========================================================================
// napi_create_async_work tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_NullEnv) {
  napi_async_work work = nullptr;
  auto execute = [](napi_env, void *) {};
  EXPECT_EQ(
      napi_create_async_work(
          nullptr, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_NullExecute) {
  napi_async_work work = nullptr;
  EXPECT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, nullptr, nullptr, nullptr, &work),
      napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_NullResult) {
  auto execute = [](napi_env, void *) {};
  EXPECT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, nullptr),
      napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_Success) {
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  EXPECT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);
  EXPECT_NE(work, nullptr);

  // Clean up.
  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_WithComplete) {
  auto execute = [](napi_env, void *) {};
  auto complete = [](napi_env, napi_status, void *) {};
  napi_async_work work = nullptr;
  EXPECT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, nullptr, &work),
      napi_ok);
  EXPECT_NE(work, nullptr);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, CreateAsyncWork_NullCompleteIsOk) {
  // complete callback is optional (can be NULL).
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  EXPECT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);
  EXPECT_NE(work, nullptr);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

//===========================================================================
// napi_delete_async_work tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, DeleteAsyncWork_NullEnv) {
  EXPECT_EQ(napi_delete_async_work(nullptr, nullptr), napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, DeleteAsyncWork_NullWork) {
  EXPECT_EQ(napi_delete_async_work(env_, nullptr), napi_invalid_arg);
}

//===========================================================================
// napi_queue_async_work tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_NullEnv) {
  EXPECT_EQ(napi_queue_async_work(nullptr, nullptr), napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_NullWork) {
  EXPECT_EQ(napi_queue_async_work(env_, nullptr), napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_PostsToEventLoop) {
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);

  EXPECT_EQ(napi_queue_async_work(env_, work), napi_ok);
  // The mock should have recorded one item.
  EXPECT_EQ(mockLoop_.items.size(), 1u);
  EXPECT_EQ(mockLoop_.items[0].work_data, work);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_ExecuteCallback) {
  // Verify that execute runs when the mock loop processes work.
  int executeCount = 0;
  struct Data {
    int *count;
  };
  Data data{&executeCount};

  auto execute = [](napi_env, void *d) {
    auto *data = static_cast<Data *>(d);
    ++(*data->count);
  };
  auto complete = [](napi_env, napi_status, void *) {};

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, &data, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  EXPECT_EQ(executeCount, 0);
  mockLoop_.drainAll();
  EXPECT_EQ(executeCount, 1);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_CompleteCallback) {
  // Verify that complete runs with napi_ok on successful execution.
  napi_status completedStatus = napi_generic_failure;
  int completeCount = 0;
  struct Data {
    napi_status *status;
    int *count;
  };
  Data data{&completedStatus, &completeCount};

  auto execute = [](napi_env, void *) {};
  auto complete = [](napi_env, napi_status s, void *d) {
    auto *data = static_cast<Data *>(d);
    *data->status = s;
    ++(*data->count);
  };

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, &data, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  mockLoop_.drainAll();
  EXPECT_EQ(completeCount, 1);
  EXPECT_EQ(completedStatus, napi_ok);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_NullCompleteDoesNotCrash) {
  // When complete is NULL, draining should not crash.
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  // This should not crash.
  mockLoop_.drainAll();

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, QueueAsyncWork_WithThread) {
  // Verify execute runs on a different thread.
  std::atomic<std::thread::id> executeThreadId{};
  struct Data {
    std::atomic<std::thread::id> *tid;
  };
  Data data{&executeThreadId};

  auto execute = [](napi_env, void *d) {
    auto *data = static_cast<Data *>(d);
    data->tid->store(std::this_thread::get_id());
  };
  auto complete = [](napi_env, napi_status, void *) {};

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, &data, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  mockLoop_.drainWithThread();

  auto execTid = executeThreadId.load();
  EXPECT_NE(execTid, std::thread::id{});
  EXPECT_NE(execTid, std::this_thread::get_id());

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

//===========================================================================
// napi_cancel_async_work tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, CancelAsyncWork_NullEnv) {
  EXPECT_EQ(napi_cancel_async_work(nullptr, nullptr), napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, CancelAsyncWork_NullWork) {
  EXPECT_EQ(napi_cancel_async_work(env_, nullptr), napi_invalid_arg);
}

TEST_F(NapiAsyncWorkTest, CancelAsyncWork_Success) {
  int executeCount = 0;
  napi_status completedStatus = napi_ok;
  struct Data {
    int *execCount;
    napi_status *compStatus;
  };
  Data data{&executeCount, &completedStatus};

  auto execute = [](napi_env, void *d) {
    auto *data = static_cast<Data *>(d);
    ++(*data->execCount);
  };
  auto complete = [](napi_env, napi_status s, void *d) {
    auto *data = static_cast<Data *>(d);
    *data->compStatus = s;
  };

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, &data, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  // Cancel before execution.
  EXPECT_EQ(napi_cancel_async_work(env_, work), napi_ok);

  // Execute should not have been called.
  EXPECT_EQ(executeCount, 0);
  // Complete should have been called with napi_cancelled.
  EXPECT_EQ(completedStatus, napi_cancelled);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, CancelAsyncWork_AlreadyExecuted) {
  auto execute = [](napi_env, void *) {};
  auto complete = [](napi_env, napi_status, void *) {};

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, nullptr, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  // Execute and complete the work.
  mockLoop_.drainAll();

  // Attempting to cancel after execution should fail.
  EXPECT_EQ(napi_cancel_async_work(env_, work), napi_generic_failure);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

//===========================================================================
// No event loop tests
//===========================================================================

class NapiAsyncWorkNoLoopTest : public NapiTestFixture {};

TEST_F(NapiAsyncWorkNoLoopTest, QueueAsyncWork_NoEventLoop) {
  // Without an event loop, queue should fail.
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);

  EXPECT_EQ(napi_queue_async_work(env_, work), napi_generic_failure);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkNoLoopTest, CancelAsyncWork_NoEventLoop) {
  // Without an event loop, cancel should fail.
  auto execute = [](napi_env, void *) {};
  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, nullptr, nullptr, &work),
      napi_ok);

  EXPECT_EQ(napi_cancel_async_work(env_, work), napi_generic_failure);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkNoLoopTest, CreateDeleteWithoutEventLoop) {
  // Create and delete should work even without an event loop.
  auto execute = [](napi_env, void *) {};
  auto complete = [](napi_env, napi_status, void *) {};
  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, nullptr, &work),
      napi_ok);
  EXPECT_NE(work, nullptr);
  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

//===========================================================================
// Data passing tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, DataPassedToCallbacks) {
  // Verify that the data pointer is passed through to both callbacks.
  int marker = 42;
  struct Data {
    int *marker;
    int executeValue;
    int completeValue;
  };
  Data data{&marker, 0, 0};

  auto execute = [](napi_env, void *d) {
    auto *data = static_cast<Data *>(d);
    data->executeValue = *data->marker;
  };
  auto complete = [](napi_env, napi_status, void *d) {
    auto *data = static_cast<Data *>(d);
    data->completeValue = *data->marker;
  };

  napi_async_work work = nullptr;
  ASSERT_EQ(
      napi_create_async_work(
          env_, nullptr, nullptr, execute, complete, &data, &work),
      napi_ok);
  ASSERT_EQ(napi_queue_async_work(env_, work), napi_ok);

  mockLoop_.drainAll();
  EXPECT_EQ(data.executeValue, 42);
  EXPECT_EQ(data.completeValue, 42);

  EXPECT_EQ(napi_delete_async_work(env_, work), napi_ok);
}

TEST_F(NapiAsyncWorkTest, MultipleWorkItems) {
  // Queue multiple work items and verify they all execute.
  int counts[3] = {0, 0, 0};

  auto execute = [](napi_env, void *d) {
    int *count = static_cast<int *>(d);
    ++(*count);
  };
  auto complete = [](napi_env, napi_status, void *) {};

  napi_async_work works[3];
  for (int i = 0; i < 3; ++i) {
    ASSERT_EQ(
        napi_create_async_work(
            env_, nullptr, nullptr, execute, complete, &counts[i], &works[i]),
        napi_ok);
    ASSERT_EQ(napi_queue_async_work(env_, works[i]), napi_ok);
  }

  mockLoop_.drainAll();
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(counts[i], 1);
    EXPECT_EQ(napi_delete_async_work(env_, works[i]), napi_ok);
  }
}

//===========================================================================
// napi_get_uv_event_loop tests
//===========================================================================

TEST_F(NapiAsyncWorkTest, GetUvEventLoopReturnsHostLoop) {
  // Set a sentinel uv_loop pointer on the host struct.
  int sentinel = 42;
  eventLoop_.uv_loop = reinterpret_cast<struct uv_loop_s *>(&sentinel);

  struct uv_loop_s *loop = nullptr;
  ASSERT_EQ(napi_get_uv_event_loop(env_, &loop), napi_ok);
  EXPECT_EQ(loop, reinterpret_cast<struct uv_loop_s *>(&sentinel));
}

TEST_F(NapiAsyncWorkTest, GetUvEventLoopFailsWhenNull) {
  // uv_loop should be zero-initialized (nullptr) in our test setup.
  eventLoop_.uv_loop = nullptr;

  struct uv_loop_s *loop = nullptr;
  ASSERT_EQ(napi_get_uv_event_loop(env_, &loop), napi_generic_failure);
}

TEST_F(NapiAsyncWorkTest, FatalExceptionCallsHostHandler) {
  struct CallInfo {
    bool called = false;
    napi_env received_env = nullptr;
    napi_value received_err = nullptr;
  } info;

  // Save and replace the data pointer for this test.
  void *savedData = eventLoop_.data;
  eventLoop_.data = &info;
  eventLoop_.fatal_exception = [](void *data, napi_env env, napi_value err) {
    auto *ci = static_cast<CallInfo *>(data);
    ci->called = true;
    ci->received_env = env;
    ci->received_err = err;
  };

  napi_handle_scope scope;
  ASSERT_EQ(napi_open_handle_scope(env_, &scope), napi_ok);

  napi_value errVal;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "test error", NAPI_AUTO_LENGTH, &errVal),
      napi_ok);

  ASSERT_EQ(napi_fatal_exception(env_, errVal), napi_ok);
  EXPECT_TRUE(info.called);
  EXPECT_EQ(info.received_env, env_);
  EXPECT_EQ(info.received_err, errVal);

  ASSERT_EQ(napi_close_handle_scope(env_, scope), napi_ok);

  // Restore.
  eventLoop_.data = savedData;
  eventLoop_.fatal_exception = nullptr;
}

} // namespace napi
} // namespace hermes
