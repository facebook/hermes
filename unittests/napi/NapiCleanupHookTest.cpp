/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_impl.h"

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

#include <memory>
#include <vector>

namespace {

/// Record the order in which cleanup hooks are called.
static std::vector<int> callOrder;

/// Cleanup hook that records an integer value.
static void recordHook(void *arg) {
  callOrder.push_back(static_cast<int>(reinterpret_cast<intptr_t>(arg)));
}

/// Cleanup hook that removes itself during execution.
static void selfRemovingHook(void *arg);

/// Data for the self-removing hook test.
struct SelfRemoveData {
  napi_env env;
  int id;
};

static void selfRemovingHook(void *arg) {
  auto *data = static_cast<SelfRemoveData *>(arg);
  callOrder.push_back(data->id);
  napi_remove_env_cleanup_hook(data->env, selfRemovingHook, arg);
}

/// Helper to create a runtime + env pair.
struct EnvScope {
  std::shared_ptr<hermes::vm::Runtime> rt;
  napi_env env = nullptr;

  EnvScope() {
    auto config = hermes::vm::RuntimeConfig::Builder()
                      .withGCConfig(
                          hermes::vm::GCConfig::Builder()
                              .withInitHeapSize(1 << 16)
                              .withMaxHeapSize(1 << 19)
                              .build())
                      .build();
    rt = hermes::vm::Runtime::create(config);
    env = hermes_napi_create_env(&*rt);
  }

  void destroy() {
    // The env is owned by the Runtime and torn down as part of
    // ~Runtime. Drop the borrowed pointer and reset the runtime.
    env = nullptr;
    rt.reset();
  }

  ~EnvScope() {
    destroy();
  }
};

class NapiCleanupHookTest : public ::testing::Test {
 protected:
  void SetUp() override {
    callOrder.clear();
  }
};

TEST_F(NapiCleanupHookTest, AddNull) {
  EnvScope scope;
  // Null env should return napi_invalid_arg.
  EXPECT_EQ(
      napi_add_env_cleanup_hook(nullptr, recordHook, nullptr),
      napi_invalid_arg);
  // Null function should return napi_invalid_arg.
  EXPECT_EQ(
      napi_add_env_cleanup_hook(scope.env, nullptr, nullptr), napi_invalid_arg);
}

TEST_F(NapiCleanupHookTest, RemoveNull) {
  EnvScope scope;
  // Null env should return napi_invalid_arg.
  EXPECT_EQ(
      napi_remove_env_cleanup_hook(nullptr, recordHook, nullptr),
      napi_invalid_arg);
  // Null function should return napi_invalid_arg.
  EXPECT_EQ(
      napi_remove_env_cleanup_hook(scope.env, nullptr, nullptr),
      napi_invalid_arg);
}

TEST_F(NapiCleanupHookTest, LIFOOrder) {
  // Register three hooks and verify they are called in reverse order.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(1)),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(2)),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(3)),
        napi_ok);
    // Env is destroyed here, hooks should fire.
  }

  ASSERT_EQ(callOrder.size(), 3u);
  EXPECT_EQ(callOrder[0], 3);
  EXPECT_EQ(callOrder[1], 2);
  EXPECT_EQ(callOrder[2], 1);
}

TEST_F(NapiCleanupHookTest, RemoveBeforeDestroy) {
  // Register two hooks, remove the second, verify only the first fires.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(10)),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(20)),
        napi_ok);
    ASSERT_EQ(
        napi_remove_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(20)),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 1u);
  EXPECT_EQ(callOrder[0], 10);
}

TEST_F(NapiCleanupHookTest, RemoveNonExistent) {
  // Removing a hook that was never added is a no-op (returns napi_ok).
  {
    EnvScope scope;
    EXPECT_EQ(
        napi_remove_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(99)),
        napi_ok);
  }
  EXPECT_TRUE(callOrder.empty());
}

TEST_F(NapiCleanupHookTest, DuplicateAddIsNoOp) {
  // Adding the same {fun, arg} pair twice should be a no-op; the hook
  // fires only once on teardown.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(42)),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(42)),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 1u);
  EXPECT_EQ(callOrder[0], 42);
}

TEST_F(NapiCleanupHookTest, SameFuncDifferentArg) {
  // Same function with different args should be treated as separate hooks.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(1)),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(2)),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 2u);
  EXPECT_EQ(callOrder[0], 2);
  EXPECT_EQ(callOrder[1], 1);
}

TEST_F(NapiCleanupHookTest, NoHooksNoProblems) {
  // Destroying an env with no cleanup hooks should be fine.
  {
    EnvScope scope;
  }
  EXPECT_TRUE(callOrder.empty());
}

TEST_F(NapiCleanupHookTest, HookRemovesItself) {
  // A cleanup hook that removes itself during execution should not cause
  // issues (the vector is processed by popping from the back).
  SelfRemoveData data1;
  SelfRemoveData data2;
  {
    EnvScope scope;
    data1 = {scope.env, 100};
    data2 = {scope.env, 200};
    ASSERT_EQ(
        napi_add_env_cleanup_hook(scope.env, selfRemovingHook, &data1),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(scope.env, selfRemovingHook, &data2),
        napi_ok);
  }

  // Both hooks should fire; self-removal during iteration is safe
  // because we pop from the back.
  ASSERT_EQ(callOrder.size(), 2u);
  EXPECT_EQ(callOrder[0], 200);
  EXPECT_EQ(callOrder[1], 100);
}

//===========================================================================
// Async cleanup hook tests
//===========================================================================

/// Async cleanup hook that records an integer value.
static void asyncRecordHook(napi_async_cleanup_hook_handle handle, void *arg) {
  callOrder.push_back(static_cast<int>(reinterpret_cast<intptr_t>(arg)));
  // Signal completion by removing the handle.
  napi_remove_async_cleanup_hook(handle);
}

/// Async cleanup hook that does NOT remove the handle (testing that
/// the env destructor handles this gracefully by deleting it).
static void asyncRecordHookNoRemove(
    napi_async_cleanup_hook_handle /*handle*/,
    void *arg) {
  callOrder.push_back(static_cast<int>(reinterpret_cast<intptr_t>(arg)));
}

TEST_F(NapiCleanupHookTest, AsyncAddNull) {
  EnvScope scope;
  // Null env should return napi_invalid_arg.
  napi_async_cleanup_hook_handle handle;
  EXPECT_EQ(
      napi_add_async_cleanup_hook(nullptr, asyncRecordHook, nullptr, &handle),
      napi_invalid_arg);
  // Null hook should return napi_invalid_arg.
  EXPECT_EQ(
      napi_add_async_cleanup_hook(scope.env, nullptr, nullptr, &handle),
      napi_invalid_arg);
}

TEST_F(NapiCleanupHookTest, AsyncRemoveNull) {
  // Null handle should return napi_invalid_arg.
  EXPECT_EQ(napi_remove_async_cleanup_hook(nullptr), napi_invalid_arg);
}

TEST_F(NapiCleanupHookTest, AsyncLIFOOrder) {
  // Register three async hooks and verify they are called in reverse order.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(1),
            nullptr),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(2),
            nullptr),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(3),
            nullptr),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 3u);
  EXPECT_EQ(callOrder[0], 3);
  EXPECT_EQ(callOrder[1], 2);
  EXPECT_EQ(callOrder[2], 1);
}

TEST_F(NapiCleanupHookTest, AsyncRemoveBeforeDestroy) {
  // Register two async hooks, remove the second, verify only the first fires.
  {
    EnvScope scope;
    napi_async_cleanup_hook_handle handle1;
    napi_async_cleanup_hook_handle handle2;
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(10),
            &handle1),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(20),
            &handle2),
        napi_ok);
    ASSERT_EQ(napi_remove_async_cleanup_hook(handle2), napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 1u);
  EXPECT_EQ(callOrder[0], 10);
}

TEST_F(NapiCleanupHookTest, AsyncNullRemoveHandle) {
  // Passing nullptr for remove_handle should be ok (just don't return
  // the handle).
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(42),
            nullptr),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 1u);
  EXPECT_EQ(callOrder[0], 42);
}

TEST_F(NapiCleanupHookTest, AsyncHookRemovesItself) {
  // An async cleanup hook that removes itself during execution.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env, asyncRecordHook, reinterpret_cast<void *>(100), nullptr),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env, asyncRecordHook, reinterpret_cast<void *>(200), nullptr),
        napi_ok);
  }

  ASSERT_EQ(callOrder.size(), 2u);
  EXPECT_EQ(callOrder[0], 200);
  EXPECT_EQ(callOrder[1], 100);
}

TEST_F(NapiCleanupHookTest, SyncAndAsyncOrder) {
  // Sync hooks should fire before async hooks on teardown.
  {
    EnvScope scope;
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(1)),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(2),
            nullptr),
        napi_ok);
    ASSERT_EQ(
        napi_add_env_cleanup_hook(
            scope.env, recordHook, reinterpret_cast<void *>(3)),
        napi_ok);
    ASSERT_EQ(
        napi_add_async_cleanup_hook(
            scope.env,
            asyncRecordHookNoRemove,
            reinterpret_cast<void *>(4),
            nullptr),
        napi_ok);
  }

  // Sync hooks run first (LIFO: 3, 1), then async hooks (LIFO: 4, 2).
  ASSERT_EQ(callOrder.size(), 4u);
  EXPECT_EQ(callOrder[0], 3);
  EXPECT_EQ(callOrder[1], 1);
  EXPECT_EQ(callOrder[2], 4);
  EXPECT_EQ(callOrder[3], 2);
}

} // namespace
