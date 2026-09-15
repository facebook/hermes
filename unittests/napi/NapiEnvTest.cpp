/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {

using hermes::napi::NapiTestFixture;

//===========================================================================
// napi_get_version
//===========================================================================

TEST_F(NapiTestFixture, GetVersion) {
  uint32_t version = 0;
  ASSERT_EQ(napi_ok, napi_get_version(env_, &version));
  EXPECT_EQ(static_cast<uint32_t>(NAPI_VERSION), version);
}

TEST_F(NapiTestFixture, GetVersionNullEnv) {
  uint32_t version = 0;
  EXPECT_EQ(napi_invalid_arg, napi_get_version(nullptr, &version));
}

TEST_F(NapiTestFixture, GetVersionNullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_get_version(env_, nullptr));
}

//===========================================================================
// napi_get_node_version
//===========================================================================

TEST_F(NapiTestFixture, GetNodeVersion) {
  const napi_node_version *version = nullptr;
  ASSERT_EQ(napi_ok, napi_get_node_version(env_, &version));
  ASSERT_NE(nullptr, version);
  // Hermes project version is 1.0.0
  EXPECT_EQ(1u, version->major);
  EXPECT_EQ(0u, version->minor);
  EXPECT_EQ(0u, version->patch);
  EXPECT_STREQ("hermes", version->release);
}

TEST_F(NapiTestFixture, GetNodeVersionNullEnv) {
  const napi_node_version *version = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_node_version(nullptr, &version));
}

TEST_F(NapiTestFixture, GetNodeVersionNullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_get_node_version(env_, nullptr));
}

TEST_F(NapiTestFixture, GetNodeVersionReturnsStatic) {
  // Calling twice should return the same pointer (static storage).
  const napi_node_version *v1 = nullptr;
  const napi_node_version *v2 = nullptr;
  ASSERT_EQ(napi_ok, napi_get_node_version(env_, &v1));
  ASSERT_EQ(napi_ok, napi_get_node_version(env_, &v2));
  EXPECT_EQ(v1, v2);
}

//===========================================================================
// Error info
//===========================================================================

TEST_F(NapiTestFixture, ClearErrorOnSuccess) {
  // After a successful call, last_error should be cleared.
  uint32_t version = 0;
  ASSERT_EQ(napi_ok, napi_get_version(env_, &version));
  EXPECT_EQ(napi_ok, env_->last_error.error_code);
  EXPECT_EQ(nullptr, env_->last_error.error_message);
}

TEST_F(NapiTestFixture, SetErrorOnFailure) {
  // Passing a null result should set last_error.
  napi_status status = napi_get_version(env_, nullptr);
  EXPECT_EQ(napi_invalid_arg, status);
  EXPECT_EQ(napi_invalid_arg, env_->last_error.error_code);
}

//===========================================================================
// napi_get_last_error_info
//===========================================================================

TEST_F(NapiTestFixture, GetLastErrorInfoNullEnv) {
  const napi_extended_error_info *info = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_last_error_info(nullptr, &info));
}

TEST_F(NapiTestFixture, GetLastErrorInfoNullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_get_last_error_info(env_, nullptr));
}

TEST_F(NapiTestFixture, GetLastErrorInfoAfterSuccess) {
  // After a successful call, error info should report napi_ok.
  uint32_t version = 0;
  ASSERT_EQ(napi_ok, napi_get_version(env_, &version));

  const napi_extended_error_info *info = nullptr;
  ASSERT_EQ(napi_ok, napi_get_last_error_info(env_, &info));
  ASSERT_NE(nullptr, info);
  EXPECT_EQ(napi_ok, info->error_code);
  // For napi_ok, the error_message is nullptr (cleared).
  EXPECT_EQ(nullptr, info->error_message);
}

TEST_F(NapiTestFixture, GetLastErrorInfoAfterFailure) {
  // After a failing call, error info should report the error.
  napi_get_version(env_, nullptr); // triggers napi_invalid_arg

  const napi_extended_error_info *info = nullptr;
  ASSERT_EQ(napi_ok, napi_get_last_error_info(env_, &info));
  ASSERT_NE(nullptr, info);
  EXPECT_EQ(napi_invalid_arg, info->error_code);
  EXPECT_NE(nullptr, info->error_message);
  EXPECT_STREQ("Invalid argument", info->error_message);
}

TEST_F(NapiTestFixture, GetLastErrorInfoReturnsEnvPointer) {
  // The returned pointer should be the env's own last_error struct.
  const napi_extended_error_info *info = nullptr;
  ASSERT_EQ(napi_ok, napi_get_last_error_info(env_, &info));
  EXPECT_EQ(&env_->last_error, info);
}

TEST_F(NapiTestFixture, GetLastErrorInfoOverwrites) {
  // Trigger an error, query it, then trigger success — should clear.
  napi_get_version(env_, nullptr); // triggers napi_invalid_arg

  const napi_extended_error_info *info = nullptr;
  ASSERT_EQ(napi_ok, napi_get_last_error_info(env_, &info));
  EXPECT_EQ(napi_invalid_arg, info->error_code);

  // Now do a successful call.
  uint32_t version = 0;
  ASSERT_EQ(napi_ok, napi_get_version(env_, &version));

  // Query again — should now be ok.
  ASSERT_EQ(napi_ok, napi_get_last_error_info(env_, &info));
  EXPECT_EQ(napi_ok, info->error_code);
  EXPECT_EQ(nullptr, info->error_message);
}

//===========================================================================
// Environment creation and destruction
//===========================================================================

TEST_F(NapiTestFixture, EnvHasRuntime) {
  // Verify the env holds a reference to the runtime we created.
  EXPECT_EQ(&*rt_, &env_->runtime);
}

TEST_F(NapiTestFixture, EnvInitialState) {
  // Verify initial env state is clean.
  EXPECT_EQ(napi_ok, env_->last_error.error_code);
  EXPECT_FALSE(env_->hasPendingException);
  EXPECT_EQ(0, env_->open_handle_scopes);
  EXPECT_EQ(NAPI_VERSION, env_->module_api_version);
}

//===========================================================================
// Env teardown ordering: wrap finalizers can access instance data
//===========================================================================

/// Verify that wrap finalizers (via napi_ref) can call
/// napi_get_instance_data during env destruction. Instance data must
/// be finalized after persistent reference finalizers.
/// This is the pattern used by real addons (e.g., MyObject::Destructor
/// in the Node.js 6_object_wrap test).
TEST_F(NapiTestFixture, WrapRefFinalizerCanAccessInstanceDataDuringTeardown) {
  // Track what the wrap finalizer observes.
  struct FinalizerState {
    bool called = false;
    napi_status getStatus = napi_generic_failure;
    void *instanceData = nullptr;
  };
  FinalizerState state;

  // Set instance data.
  int instanceValue = 42;
  ASSERT_EQ(
      napi_ok, napi_set_instance_data(env_, &instanceValue, nullptr, nullptr));

  // Create a wrapped object with a ref (passing &ref to napi_wrap).
  // When a ref is created, the finalizer is stored on the napi_ref
  // and runs during env destruction via the refListHead_ loop.
  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 99;
    napi_ref ref = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env env, void * /*data*/, void *hint) {
              auto *s = static_cast<FinalizerState *>(hint);
              s->called = true;
              s->getStatus = napi_get_instance_data(env, &s->instanceData);
            },
            &state,
            &ref));
    ASSERT_NE(nullptr, ref);

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env_, scope));
  }

  // Destroy the Runtime, which drives env shutdown. The ref finalizer
  // must be able to read instance data because instance data
  // finalization runs after ref cleanup.
  env_ = nullptr;
  rt_.reset();

  EXPECT_TRUE(state.called);
  EXPECT_EQ(napi_ok, state.getStatus);
  EXPECT_EQ(&instanceValue, state.instanceData);
}

/// Verify that the instance data finalizer runs after ref finalizers
/// during env destruction (ordering test).
TEST_F(NapiTestFixture, InstanceDataFinalizedAfterRefFinalizers) {
  // Track the order of finalizer calls.
  struct CallOrder {
    int refOrder = -1;
    int instanceDataOrder = -1;
    int counter = 0;
  };
  CallOrder order;

  // Set instance data with a finalizer that records its call order.
  ASSERT_EQ(
      napi_ok,
      napi_set_instance_data(
          env_,
          &order,
          [](napi_env, void *data, void * /*hint*/) {
            auto *o = static_cast<CallOrder *>(data);
            o->instanceDataOrder = o->counter++;
          },
          nullptr));

  // Create a wrapped object with a ref whose finalizer records order.
  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 0;
    napi_ref ref = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env, void * /*data*/, void *hint) {
              auto *o = static_cast<CallOrder *>(hint);
              o->refOrder = o->counter++;
            },
            &order,
            &ref));
    ASSERT_NE(nullptr, ref);

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env_, scope));
  }

  // Destroy the Runtime, which drives env shutdown.
  env_ = nullptr;
  rt_.reset();

  // Both finalizers must have been called.
  EXPECT_NE(-1, order.refOrder);
  EXPECT_NE(-1, order.instanceDataOrder);
  // Ref finalizer must run before instance data finalizer.
  EXPECT_LT(order.refOrder, order.instanceDataOrder);
}

//===========================================================================
// DrainJobs callback drains pending finalizers
//===========================================================================

/// Verify that pending NAPI finalizers are drained when
/// Runtime::drainJobs() is called. This is a standalone test (not
/// using NapiTestFixture) because it needs withMicrotaskQueue(true).
TEST(NapiEnvDrainJobsTest, DrainJobsDrainsPendingFinalizers) {
  using namespace hermes::vm;

  auto config = RuntimeConfig::Builder()
                    .withGCConfig(
                        GCConfig::Builder()
                            .withInitHeapSize(1 << 16)
                            .withMaxHeapSize(1 << 19)
                            .build())
                    .withMicrotaskQueue(true)
                    .build();
  auto rt = Runtime::create(config);
  napi_env env = hermes_napi_create_env(&*rt);

  bool finalizerCalled = false;

  // Create an external-data object with a GC-triggered finalizer
  // inside a handle scope, then let it go out of scope so that GC
  // can collect it.
  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env, &scope));

    napi_value ext = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external(
            env,
            &finalizerCalled,
            [](napi_env, void *data, void *) {
              *static_cast<bool *>(data) = true;
            },
            nullptr,
            &ext));

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env, scope));
  }

  // Trigger GC — the external object is unreachable, so its
  // destructor queues the finalizer via queuePendingFinalizer.
  rt->collect("test");
  EXPECT_FALSE(finalizerCalled)
      << "Finalizer should be queued, not called during GC";

  // drainJobs() should invoke the drainJobsCallback, which calls
  // drainPendingFinalizers().
  rt->drainJobs();
  EXPECT_TRUE(finalizerCalled)
      << "Finalizer should have been drained by drainJobs()";

  // Env is owned by the Runtime; rt going out of scope tears it down.
}

//===========================================================================
// Wrapped-object finalizers fire with a live env at Runtime teardown
//===========================================================================

/// Wrap a JS object, drop the only reference from JS, then destroy the
/// Runtime. The wrap finalizer must fire exactly once with a non-null
/// env — calling it with nullptr would crash any addon built on
/// node-addon-api's ObjectWrap (its FinalizeCallback derefs env via
/// HandleScope). This is the central invariant of the new lifetime
/// model: the env is owned by the Runtime and outlives every GC cycle,
/// so user finalizers never see a null env.
TEST_F(NapiTestFixture, WrappedObjectFinalizerFiresWithLiveEnvAtTeardown) {
  struct FinalizerState {
    int callCount = 0;
    bool sawNullEnv = false;
  };
  FinalizerState state;

  // Create a wrapped object inside a scope so it becomes unreachable
  // after the scope closes.
  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 0;
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env env, void *, void *hint) {
              auto *s = static_cast<FinalizerState *>(hint);
              ++s->callCount;
              if (env == nullptr)
                s->sawNullEnv = true;
            },
            &state,
            /*result=*/nullptr));

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env_, scope));
  }

  // Destroy the Runtime. The wrapped object is finalized by
  // getHeap().finalizeAll(), which queues the callback into the
  // env (the env is kept alive past finalizeAll precisely for this
  // reason). The post-shutdown deleter then drains the queue with
  // a non-null env on the mutator. The heap is post-mortem at
  // that point, but a callback that only touches native state
  // (the typical case) works normally.
  env_ = nullptr;
  rt_.reset();

  EXPECT_EQ(1, state.callCount);
  EXPECT_FALSE(state.sawNullEnv);
}

/// A finalizer that runs during the post-finalizeAll drain may try to
/// call back into JS (spec says it shouldn't, but the case is real).
/// NAPI_PREAMBLE detects this via inPostShutdownDrain_ and returns
/// napi_cannot_run_js — a defined error instead of an undefined
/// crash. Spec-conformant finalizers (which only free native
/// resources) ignore this and complete normally.
TEST_F(NapiTestFixture, FinalizerDuringPostShutdownDrainGetsCannotRunJs) {
  struct FinalizerState {
    int callCount = 0;
    napi_env receivedEnv = nullptr;
    napi_status createObjectStatus = napi_ok;
  };
  FinalizerState state;

  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

    // Stash the wrapped object on globalThis so it survives shutdown's
    // proactive GC and ends up finalized by ~Runtime's finalizeAll.
    napi_value global = nullptr;
    ASSERT_EQ(napi_ok, napi_get_global(env_, &global));

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 0;
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env env, void *, void *hint) {
              auto *s = static_cast<FinalizerState *>(hint);
              ++s->callCount;
              s->receivedEnv = env;
              // Attempt to allocate — NAPI_PREAMBLE must reject with
              // napi_cannot_run_js while inPostShutdownDrain_ is set.
              napi_value v = nullptr;
              s->createObjectStatus = napi_create_object(env, &v);
            },
            &state,
            /*result=*/nullptr));

    ASSERT_EQ(napi_ok, napi_set_named_property(env_, global, "kept", obj));

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env_, scope));
  }

  env_ = nullptr;
  rt_.reset();

  // Callback fired exactly once with a live env.
  EXPECT_EQ(1, state.callCount);
  EXPECT_NE(nullptr, state.receivedEnv);
  // Allocation attempt was rejected cleanly.
  EXPECT_EQ(napi_cannot_run_js, state.createObjectStatus);
}

/// A wrapped object that becomes unreachable DURING the env's
/// shutdown phase (via user code in a cleanup hook that drops its
/// only ref and forces a GC) has its wrap finalizer drained at the
/// end of shutdown(), with the heap still functional and
/// inPostShutdownDrain_ NOT yet set. The finalizer therefore has
/// full env access — napi_create_object succeeds.
///
/// Contrast with FinalizerDuringPostShutdownDrainGetsCannotRunJs,
/// where the wrapped object survives shutdown() entirely and is
/// only finalized by finalizeAll, putting its callback in the
/// constrained post-shutdown drain.
TEST_F(NapiTestFixture, FinalizerDuringShutdownHasFunctionalEnv) {
  struct State {
    napi_env env = nullptr;
    napi_ref keepAliveRef = nullptr;
    int callCount = 0;
    napi_env receivedEnv = nullptr;
    napi_status createObjectStatus = napi_invalid_arg;
  };
  State state;
  state.env = env_;

  // Wrap a JS object with finalize_cb stored on the NapiObjectData
  // (result=nullptr so the finalizer goes through the deferred
  // queue, not the ref-list walk). Hold it alive with a separate
  // strong napi_ref so it survives until the cleanup hook drops it.
  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 0;
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env env, void *, void *hint) {
              auto *s = static_cast<State *>(hint);
              ++s->callCount;
              s->receivedEnv = env;
              // The heap should be fully functional — this is the
              // distinguishing assertion vs. the post-shutdown
              // drain case.
              napi_value v = nullptr;
              s->createObjectStatus = napi_create_object(env, &v);
            },
            &state,
            /*result=*/nullptr));

    ASSERT_EQ(
        napi_ok, napi_create_reference(env_, obj, 1, &state.keepAliveRef));

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env_, scope));
  }

  // Cleanup hook drops the strong ref and forces a GC. Both happen
  // during shutdown()'s cleanup-hook phase, so the wrapped object's
  // wrap callback queues into pendingFinalizers_ via
  // finalizeNapiObjectData and is drained by the final
  // drainPendingFinalizers() at the end of shutdown().
  ASSERT_EQ(
      napi_ok,
      napi_add_env_cleanup_hook(
          env_,
          [](void *arg) {
            auto *s = static_cast<State *>(arg);
            ASSERT_EQ(napi_ok, napi_delete_reference(s->env, s->keepAliveRef));
            s->env->runtime.collect("test-shutdown-gc");
          },
          &state));

  env_ = nullptr;
  rt_.reset();

  EXPECT_EQ(1, state.callCount);
  EXPECT_NE(nullptr, state.receivedEnv);
  // Full env access — napi_create_object succeeds because the
  // heap is still functional when the drain runs.
  EXPECT_EQ(napi_ok, state.createObjectStatus);
}

/// If the wrapped object becomes unreachable AND is collected before
/// Runtime teardown (so its finalizer queues during normal operation
/// and drains via the drainJobs hook or NAPI_PREAMBLE), the finalizer
/// MUST fire with a non-null env.
TEST(NapiEnvTeardownLiveEnvTest, FinalizerBeforeTeardownGetsLiveEnv) {
  using namespace hermes::vm;

  auto config = RuntimeConfig::Builder()
                    .withGCConfig(
                        GCConfig::Builder()
                            .withInitHeapSize(1 << 16)
                            .withMaxHeapSize(1 << 19)
                            .build())
                    .build();
  auto rt = Runtime::create(config);
  napi_env env = hermes_napi_create_env(&*rt);

  struct FinalizerState {
    int callCount = 0;
    bool sawNullEnv = false;
  } state;

  {
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_ok, napi_open_handle_scope(env, &scope));

    napi_value ext = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external(
            env,
            &state,
            [](napi_env e, void *data, void *) {
              auto *s = static_cast<FinalizerState *>(data);
              ++s->callCount;
              if (e == nullptr)
                s->sawNullEnv = true;
            },
            nullptr,
            &ext));

    ASSERT_EQ(napi_ok, napi_close_handle_scope(env, scope));
  }

  rt->collect("test");
  env->drainPendingFinalizers();

  EXPECT_EQ(1, state.callCount);
  EXPECT_FALSE(state.sawNullEnv);

  // Env is owned by rt; teardown is implicit.
}

} // namespace
