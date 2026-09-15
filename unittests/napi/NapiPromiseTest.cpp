/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/JSObject.h"

namespace {

using namespace hermes::vm;

/// A fixture with microtask queue enabled, needed for Promise tests.
class NapiPromiseTest : public ::testing::Test {
 protected:
  std::shared_ptr<Runtime> rt_;
  napi_env env_ = nullptr;

  void SetUp() override {
    auto config = RuntimeConfig::Builder()
                      .withGCConfig(
                          GCConfig::Builder()
                              .withInitHeapSize(1 << 16)
                              .withMaxHeapSize(1 << 19)
                              .build())
                      .withMicrotaskQueue(true)
                      .build();
    rt_ = Runtime::create(config);
    env_ = hermes_napi_create_env(&*rt_);
  }

  void TearDown() override {
    // The env is owned by the Runtime and torn down as part of
    // ~Runtime. Drop the borrowed pointer and reset the runtime.
    env_ = nullptr;
    rt_.reset();
  }
};

/// Helper to open a handle scope and return the scope handle.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  return scope;
}

/// Helper to close a handle scope.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_create_promise
//===========================================================================

TEST_F(NapiPromiseTest, CreatePromise_Basic) {
  napi_handle_scope scope = openScope(env_);

  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));
  ASSERT_NE(nullptr, deferred);
  ASSERT_NE(nullptr, promise);

  // The returned value should be a promise.
  bool is_promise = false;
  ASSERT_EQ(napi_ok, napi_is_promise(env_, promise, &is_promise));
  EXPECT_TRUE(is_promise);

  // Clean up: resolve to avoid dangling deferred.
  napi_value undef = nullptr;
  ASSERT_EQ(napi_ok, napi_get_undefined(env_, &undef));
  ASSERT_EQ(napi_ok, napi_resolve_deferred(env_, deferred, undef));

  closeScope(env_, scope);
}

TEST_F(NapiPromiseTest, CreatePromise_NullDeferred) {
  napi_handle_scope scope = openScope(env_);

  napi_value promise = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_create_promise(env_, nullptr, &promise));

  closeScope(env_, scope);
}

TEST_F(NapiPromiseTest, CreatePromise_NullPromise) {
  napi_handle_scope scope = openScope(env_);

  napi_deferred deferred = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_create_promise(env_, &deferred, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_resolve_deferred
//===========================================================================

TEST_F(NapiPromiseTest, ResolveDeferred_Basic) {
  napi_handle_scope scope = openScope(env_);

  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));

  // Resolve with a number value.
  napi_value resolution = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &resolution));
  ASSERT_EQ(napi_ok, napi_resolve_deferred(env_, deferred, resolution));

  closeScope(env_, scope);
}

//===========================================================================
// napi_reject_deferred
//===========================================================================

TEST_F(NapiPromiseTest, RejectDeferred_Basic) {
  napi_handle_scope scope = openScope(env_);

  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));

  // Reject with a string value.
  napi_value rejection = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "error", NAPI_AUTO_LENGTH, &rejection));
  ASSERT_EQ(napi_ok, napi_reject_deferred(env_, deferred, rejection));

  // Drain microtask queue so rejection is processed.
  rt_->drainJobs();

  closeScope(env_, scope);
}

//===========================================================================
// napi_is_promise
//===========================================================================

TEST_F(NapiPromiseTest, IsPromise_NonPromise) {
  napi_handle_scope scope = openScope(env_);

  // Regular object is not a promise.
  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));
  bool is_promise = true;
  ASSERT_EQ(napi_ok, napi_is_promise(env_, obj, &is_promise));
  EXPECT_FALSE(is_promise);

  // Number is not a promise.
  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));
  is_promise = true;
  ASSERT_EQ(napi_ok, napi_is_promise(env_, num, &is_promise));
  EXPECT_FALSE(is_promise);

  // Undefined is not a promise.
  napi_value undef = nullptr;
  ASSERT_EQ(napi_ok, napi_get_undefined(env_, &undef));
  is_promise = true;
  ASSERT_EQ(napi_ok, napi_is_promise(env_, undef, &is_promise));
  EXPECT_FALSE(is_promise);

  closeScope(env_, scope);
}

TEST_F(NapiPromiseTest, IsPromise_CreatedPromise) {
  napi_handle_scope scope = openScope(env_);

  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));

  bool is_promise = false;
  ASSERT_EQ(napi_ok, napi_is_promise(env_, promise, &is_promise));
  EXPECT_TRUE(is_promise);

  // Clean up.
  napi_value undef = nullptr;
  ASSERT_EQ(napi_ok, napi_get_undefined(env_, &undef));
  ASSERT_EQ(napi_ok, napi_resolve_deferred(env_, deferred, undef));

  closeScope(env_, scope);
}

TEST_F(NapiPromiseTest, IsPromise_NullValue) {
  bool is_promise = false;
  EXPECT_EQ(napi_invalid_arg, napi_is_promise(env_, nullptr, &is_promise));
}

TEST_F(NapiPromiseTest, IsPromise_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));
  EXPECT_EQ(napi_invalid_arg, napi_is_promise(env_, obj, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// Promise resolve/reject with .then()/.catch() via drainJobs
//===========================================================================

/// A native callback that stores a value into a static variable.
static napi_value capturedValue = nullptr;
static napi_env capturedEnv = nullptr;

static napi_value captureCallback(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  if (argc > 0) {
    // Store the value for later inspection.
    capturedEnv = env;
    capturedValue = argv[0];
  }
  return nullptr;
}

TEST_F(NapiPromiseTest, ResolveDeferred_ThenCallback) {
  napi_handle_scope scope = openScope(env_);

  capturedValue = nullptr;
  capturedEnv = nullptr;

  // Create promise.
  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));

  // Create a callback function.
  napi_value cb = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_function(
          env_, "capture", NAPI_AUTO_LENGTH, captureCallback, nullptr, &cb));

  // Call promise.then(cb).
  napi_value then_result = nullptr;
  ASSERT_EQ(
      napi_ok, napi_get_named_property(env_, promise, "then", &then_result));
  napi_value then_promise = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_call_function(env_, promise, then_result, 1, &cb, &then_promise));

  // Resolve the promise with 42.
  napi_value resolution = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &resolution));
  ASSERT_EQ(napi_ok, napi_resolve_deferred(env_, deferred, resolution));

  // Drain the microtask queue so .then() runs.
  ASSERT_EQ(ExecutionStatus::RETURNED, rt_->drainJobs());

  // The callback should have been called with the resolution value.
  ASSERT_NE(nullptr, capturedValue);
  double val = 0;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, capturedValue, &val));
  EXPECT_EQ(42.0, val);

  capturedValue = nullptr;
  capturedEnv = nullptr;

  closeScope(env_, scope);
}

TEST_F(NapiPromiseTest, RejectDeferred_CatchCallback) {
  napi_handle_scope scope = openScope(env_);

  capturedValue = nullptr;
  capturedEnv = nullptr;

  // Create promise.
  napi_deferred deferred = nullptr;
  napi_value promise = nullptr;
  ASSERT_EQ(napi_ok, napi_create_promise(env_, &deferred, &promise));

  // Create a callback function.
  napi_value cb = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_function(
          env_, "capture", NAPI_AUTO_LENGTH, captureCallback, nullptr, &cb));

  // Call promise.catch(cb).
  napi_value catch_fn = nullptr;
  ASSERT_EQ(
      napi_ok, napi_get_named_property(env_, promise, "catch", &catch_fn));
  napi_value catch_promise = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_call_function(env_, promise, catch_fn, 1, &cb, &catch_promise));

  // Reject the promise with "oops".
  napi_value rejection = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "oops", NAPI_AUTO_LENGTH, &rejection));
  ASSERT_EQ(napi_ok, napi_reject_deferred(env_, deferred, rejection));

  // Drain the microtask queue so .catch() runs.
  ASSERT_EQ(ExecutionStatus::RETURNED, rt_->drainJobs());

  // The callback should have been called with the rejection value.
  ASSERT_NE(nullptr, capturedValue);
  char buf[32];
  size_t len = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_string_utf8(env_, capturedValue, buf, sizeof(buf), &len));
  EXPECT_STREQ("oops", buf);

  capturedValue = nullptr;
  capturedEnv = nullptr;

  closeScope(env_, scope);
}

} // anonymous namespace
