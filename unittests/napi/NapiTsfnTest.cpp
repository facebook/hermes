/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi.h"

#include "hermes/VM/Runtime.h"

#include <gtest/gtest.h>

#include <utility>
#include <vector>

namespace hermes {
namespace napi {

//===========================================================================
// Mock event loop for testing tsfn loop-source integration
//===========================================================================

/// Records post_task calls in a queue (so the test can drain them
/// manually on the test thread) and tracks outstanding loop refs as a
/// counter (matching the host's per-env contract — see hermes_napi.h:
/// ref_loop / unref_loop are paired and non-nested within an env).
struct MockTsfnLoop {
  std::vector<std::pair<void *, void (*)(void *)>> tasks;
  int loopRefs = 0;

  /// Drain queued tasks in FIFO order. Tasks may schedule more tasks
  /// during execution; those will be drained too.
  void drainAll() {
    while (!tasks.empty()) {
      auto item = tasks.front();
      tasks.erase(tasks.begin());
      item.second(item.first);
    }
  }
};

static void
mockPostTask(void *loop_data, void *task_data, void (*callback)(void *)) {
  auto *loop = static_cast<MockTsfnLoop *>(loop_data);
  loop->tasks.push_back({task_data, callback});
}

static void mockRefLoop(void *loop_data) {
  auto *loop = static_cast<MockTsfnLoop *>(loop_data);
  ++loop->loopRefs;
}

static void mockUnrefLoop(void *loop_data) {
  auto *loop = static_cast<MockTsfnLoop *>(loop_data);
  --loop->loopRefs;
}

static void noopCallJs(napi_env, napi_value, void *, void *) {}

//===========================================================================
// Test fixture
//===========================================================================

class NapiTsfnTest : public ::testing::Test {
 protected:
  std::shared_ptr<vm::Runtime> rt_;
  napi_env env_ = nullptr;
  MockTsfnLoop loop_;
  hermes_napi_host host_{};

  void SetUp() override {
    auto config = vm::RuntimeConfig::Builder()
                      .withGCConfig(
                          vm::GCConfig::Builder()
                              .withInitHeapSize(1 << 16)
                              .withMaxHeapSize(1 << 19)
                              .build())
                      .build();
    rt_ = vm::Runtime::create(config);

    host_.data = &loop_;
    host_.post_task = mockPostTask;
    host_.ref_loop = mockRefLoop;
    host_.unref_loop = mockUnrefLoop;

    env_ = hermes_napi_create_env(&*rt_, &host_);
  }

  void TearDown() override {
    // The env is owned by the Runtime and torn down as part of
    // ~Runtime. Drop the borrowed pointer and reset the runtime.
    env_ = nullptr;
    rt_.reset();
  }

  napi_threadsafe_function makeTsfn() {
    napi_value name;
    EXPECT_EQ(
        napi_create_string_utf8(env_, "tsfn", NAPI_AUTO_LENGTH, &name),
        napi_ok);
    napi_threadsafe_function tsfn = nullptr;
    EXPECT_EQ(
        napi_create_threadsafe_function(
            env_,
            nullptr, // func: use call_js_cb instead
            nullptr, // async_resource
            name,
            0, // max_queue_size: unlimited
            1, // initial_thread_count
            nullptr, // thread_finalize_data
            nullptr, // thread_finalize_cb
            nullptr, // context
            noopCallJs,
            &tsfn),
        napi_ok);
    return tsfn;
  }
};

//===========================================================================
// Tests
//===========================================================================

TEST_F(NapiTsfnTest, Create_RegistersOneLoopSource) {
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);

  EXPECT_TRUE(loop_.loopRefs == 0);
  napi_threadsafe_function tsfn = makeTsfn();
  ASSERT_NE(tsfn, nullptr);
  // A referenced tsfn (default) holds exactly one loop source.
  EXPECT_EQ(loop_.loopRefs, 1);

  // Release brings refcount to 0 → final dispatch posted → drain runs
  // it on the JS thread → finalize → loop source released.
  EXPECT_EQ(napi_release_threadsafe_function(tsfn, napi_tsfn_release), napi_ok);
  loop_.drainAll();
  EXPECT_TRUE(loop_.loopRefs == 0);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiTsfnTest, Unref_ReleasesLoopSource) {
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);

  napi_threadsafe_function tsfn = makeTsfn();
  ASSERT_EQ(loop_.loopRefs, 1);

  EXPECT_EQ(napi_unref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_TRUE(loop_.loopRefs == 0);

  // Re-ref re-takes the loop reference.
  EXPECT_EQ(napi_ref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_EQ(loop_.loopRefs, 1);

  EXPECT_EQ(napi_release_threadsafe_function(tsfn, napi_tsfn_release), napi_ok);
  loop_.drainAll();
  EXPECT_TRUE(loop_.loopRefs == 0);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiTsfnTest, RepeatedUnrefAndRef_AreIdempotent) {
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);

  napi_threadsafe_function tsfn = makeTsfn();
  ASSERT_EQ(loop_.loopRefs, 1);

  // Two unrefs: only the first releases the source.
  EXPECT_EQ(napi_unref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_TRUE(loop_.loopRefs == 0);
  EXPECT_EQ(napi_unref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_TRUE(loop_.loopRefs == 0);

  // Two refs: only the first re-registers.
  EXPECT_EQ(napi_ref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_EQ(loop_.loopRefs, 1);
  EXPECT_EQ(napi_ref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_EQ(loop_.loopRefs, 1);

  EXPECT_EQ(napi_release_threadsafe_function(tsfn, napi_tsfn_release), napi_ok);
  loop_.drainAll();
  EXPECT_TRUE(loop_.loopRefs == 0);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiTsfnTest, EnvTeardown_ReleasesLoopSource) {
  // Create a tsfn but never release it on the test thread. Env teardown
  // (via Runtime destruction in TearDown) must release the source.
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);
  napi_threadsafe_function tsfn = makeTsfn();
  ASSERT_EQ(loop_.loopRefs, 1);
  napi_close_handle_scope(env_, scope);
  (void)tsfn;

  // Destroy the Runtime — env tsfn cleanup runs as part of
  // shutdown() and releases the loop ref.
  env_ = nullptr;
  rt_.reset();
  EXPECT_TRUE(loop_.loopRefs == 0);
}

TEST_F(NapiTsfnTest, MultipleTsfns_ShareOneLoopSource) {
  // Two referenced tsfns coalesce into a single host loop source —
  // the host should see exactly one register, not one per tsfn.
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);

  napi_threadsafe_function a = makeTsfn();
  EXPECT_EQ(loop_.loopRefs, 1);
  napi_threadsafe_function b = makeTsfn();
  // Still one source — the env reference-counts internally.
  EXPECT_EQ(loop_.loopRefs, 1);

  // Unref one: still one source held (b is still referenced).
  EXPECT_EQ(napi_unref_threadsafe_function(env_, a), napi_ok);
  EXPECT_EQ(loop_.loopRefs, 1);
  // Unref the other: source released, no host calls outstanding.
  EXPECT_EQ(napi_unref_threadsafe_function(env_, b), napi_ok);
  EXPECT_TRUE(loop_.loopRefs == 0);

  EXPECT_EQ(napi_release_threadsafe_function(a, napi_tsfn_release), napi_ok);
  EXPECT_EQ(napi_release_threadsafe_function(b, napi_tsfn_release), napi_ok);
  loop_.drainAll();
  EXPECT_TRUE(loop_.loopRefs == 0);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiTsfnTest, UnrefedTsfn_HoldsNoSourceOverItsLifetime) {
  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env_, &scope);

  napi_threadsafe_function tsfn = makeTsfn();
  ASSERT_EQ(loop_.loopRefs, 1);
  EXPECT_EQ(napi_unref_threadsafe_function(env_, tsfn), napi_ok);
  EXPECT_TRUE(loop_.loopRefs == 0);

  // While unrefed: no source held.
  EXPECT_EQ(napi_release_threadsafe_function(tsfn, napi_tsfn_release), napi_ok);
  // Finalize runs via the dispatch posted by release. Source set must
  // remain empty (nothing to unregister).
  loop_.drainAll();
  EXPECT_TRUE(loop_.loopRefs == 0);

  napi_close_handle_scope(env_, scope);
}

//===========================================================================
// Backward-compat: host without register/unregister callbacks
//===========================================================================

TEST(NapiTsfnNoLoopSourceTest, TsfnWorksWithoutLoopSourceCallbacks) {
  auto config = vm::RuntimeConfig::Builder()
                    .withGCConfig(
                        vm::GCConfig::Builder()
                            .withInitHeapSize(1 << 16)
                            .withMaxHeapSize(1 << 19)
                            .build())
                    .build();
  auto rt = vm::Runtime::create(config);

  MockTsfnLoop loop;
  hermes_napi_host host{};
  host.data = &loop;
  host.post_task = mockPostTask;
  // register_loop_source / unregister_loop_source intentionally null —
  // hosts that don't model loop sources must still be able to use
  // tsfn for API compatibility.

  napi_env env = hermes_napi_create_env(&*rt, &host);

  napi_handle_scope scope = nullptr;
  napi_open_handle_scope(env, &scope);

  napi_value name;
  ASSERT_EQ(
      napi_create_string_utf8(env, "tsfn", NAPI_AUTO_LENGTH, &name), napi_ok);
  napi_threadsafe_function tsfn = nullptr;
  ASSERT_EQ(
      napi_create_threadsafe_function(
          env,
          nullptr,
          nullptr,
          name,
          0,
          1,
          nullptr,
          nullptr,
          nullptr,
          noopCallJs,
          &tsfn),
      napi_ok);

  // Source set stays empty — callbacks are null, no source registered.
  EXPECT_TRUE(loop.loopRefs == 0);

  // ref/unref must succeed without calling into the (null) loop-source
  // callbacks.
  EXPECT_EQ(napi_unref_threadsafe_function(env, tsfn), napi_ok);
  EXPECT_EQ(napi_ref_threadsafe_function(env, tsfn), napi_ok);

  EXPECT_EQ(napi_release_threadsafe_function(tsfn, napi_tsfn_release), napi_ok);
  loop.drainAll();

  napi_close_handle_scope(env, scope);
  // env is owned by the Runtime; it dies with `rt` going out of scope.
}

} // namespace napi
} // namespace hermes
