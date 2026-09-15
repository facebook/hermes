/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {

using hermes::napi::NapiTestFixture;

/// Helper to open a handle scope and return the scope handle.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  EXPECT_NE(nullptr, scope);
  return scope;
}

/// Helper to close a handle scope.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_async_init / napi_async_destroy tests
//===========================================================================

class NapiAsyncContextTest : public NapiTestFixture {};

TEST_F(NapiAsyncContextTest, NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg, napi_async_init(nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiAsyncContextTest, NullResourceName) {
  auto scope = openScope(env_);

  napi_async_context ctx = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_async_init(env_, nullptr, nullptr, &ctx));

  closeScope(env_, scope);
}

TEST_F(NapiAsyncContextTest, NullResult) {
  auto scope = openScope(env_);

  napi_value name = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &name));

  EXPECT_EQ(napi_invalid_arg, napi_async_init(env_, nullptr, name, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiAsyncContextTest, CreateAndDestroy) {
  auto scope = openScope(env_);

  napi_value name = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "test_resource", NAPI_AUTO_LENGTH, &name));

  napi_async_context ctx = nullptr;
  EXPECT_EQ(napi_ok, napi_async_init(env_, nullptr, name, &ctx));
  EXPECT_NE(nullptr, ctx);

  EXPECT_EQ(napi_ok, napi_async_destroy(env_, ctx));

  closeScope(env_, scope);
}

TEST_F(NapiAsyncContextTest, CreateWithResource) {
  auto scope = openScope(env_);

  napi_value name = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "test_resource", NAPI_AUTO_LENGTH, &name));

  napi_value resource = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &resource));

  napi_async_context ctx = nullptr;
  EXPECT_EQ(napi_ok, napi_async_init(env_, resource, name, &ctx));
  EXPECT_NE(nullptr, ctx);

  EXPECT_EQ(napi_ok, napi_async_destroy(env_, ctx));

  closeScope(env_, scope);
}

TEST_F(NapiAsyncContextTest, DestroyNullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_async_destroy(nullptr, nullptr));
}

TEST_F(NapiAsyncContextTest, DestroyNullContext) {
  EXPECT_EQ(napi_invalid_arg, napi_async_destroy(env_, nullptr));
}

//===========================================================================
// napi_make_callback tests
//===========================================================================

class NapiMakeCallbackTest : public NapiTestFixture {};

TEST_F(NapiMakeCallbackTest, NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_make_callback(
          nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr));
}

TEST_F(NapiMakeCallbackTest, NullRecv) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_get_undefined(env, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_make_callback(env_, nullptr, nullptr, func, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, NullFunc) {
  auto scope = openScope(env_);

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_make_callback(env_, nullptr, recv, nullptr, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, SimpleCall) {
  auto scope = openScope(env_);

  // Create a function that returns 42.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_create_int32(env, 42, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_make_callback(env_, nullptr, recv, func, 0, nullptr, &result));

  int32_t val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_int32(env_, result, &val));
  EXPECT_EQ(42, val);

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, CallWithAsyncContext) {
  auto scope = openScope(env_);

  // Create an async context.
  napi_value name = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &name));

  napi_async_context ctx = nullptr;
  EXPECT_EQ(napi_ok, napi_async_init(env_, nullptr, name, &ctx));

  // Create a function that returns its first argument + 1.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t val = 0;
    napi_get_value_int32(env, argv[0], &val);
    napi_value r = nullptr;
    napi_create_int32(env, val + 1, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value arg = nullptr;
  EXPECT_EQ(napi_ok, napi_create_int32(env_, 10, &arg));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok, napi_make_callback(env_, ctx, recv, func, 1, &arg, &result));

  int32_t val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_int32(env_, result, &val));
  EXPECT_EQ(11, val);

  EXPECT_EQ(napi_ok, napi_async_destroy(env_, ctx));

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, CallWithNullResult) {
  auto scope = openScope(env_);

  // Create a simple function.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_get_undefined(env, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Passing nullptr for result should be fine — just discard the
  // return value.
  EXPECT_EQ(
      napi_ok,
      napi_make_callback(env_, nullptr, recv, func, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, NonCallableFunc) {
  auto scope = openScope(env_);

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Pass a non-callable value (a number) as func.
  napi_value num = nullptr;
  EXPECT_EQ(napi_ok, napi_create_int32(env_, 123, &num));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_make_callback(env_, nullptr, recv, num, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, ExceptionInCallback) {
  auto scope = openScope(env_);

  // Create a function that throws.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value err = nullptr;
    napi_value msg = nullptr;
    napi_create_string_utf8(env, "boom", NAPI_AUTO_LENGTH, &msg);
    napi_create_error(env, nullptr, msg, &err);
    napi_throw(env, err);
    return nullptr;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  EXPECT_EQ(
      napi_pending_exception,
      napi_make_callback(env_, nullptr, recv, func, 0, nullptr, nullptr));

  // Clear the pending exception.
  bool hasPending = false;
  napi_is_exception_pending(env_, &hasPending);
  EXPECT_TRUE(hasPending);

  napi_value exc = nullptr;
  napi_get_and_clear_last_exception(env_, &exc);

  closeScope(env_, scope);
}

TEST_F(NapiMakeCallbackTest, ThisArgPassed) {
  auto scope = openScope(env_);

  // Create a function that returns this.x.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);
    napi_value result = nullptr;
    napi_get_named_property(env, thisArg, "x", &result);
    return result;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  // Create an object with x=99.
  napi_value obj = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &obj));
  napi_value val = nullptr;
  EXPECT_EQ(napi_ok, napi_create_int32(env_, 99, &val));
  EXPECT_EQ(napi_ok, napi_set_named_property(env_, obj, "x", val));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_make_callback(env_, nullptr, obj, func, 0, nullptr, &result));

  int32_t extracted = 0;
  EXPECT_EQ(napi_ok, napi_get_value_int32(env_, result, &extracted));
  EXPECT_EQ(99, extracted);

  closeScope(env_, scope);
}

} // namespace
