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
// napi_open_callback_scope / napi_close_callback_scope tests
//===========================================================================

class NapiCallbackScopeTest : public NapiTestFixture {};

TEST_F(NapiCallbackScopeTest, OpenNullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_open_callback_scope(nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiCallbackScopeTest, OpenNullResult) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_open_callback_scope(env_, nullptr, nullptr, nullptr));
}

TEST_F(NapiCallbackScopeTest, CloseNullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_close_callback_scope(nullptr, nullptr));
}

TEST_F(NapiCallbackScopeTest, CloseNullScope) {
  EXPECT_EQ(napi_invalid_arg, napi_close_callback_scope(env_, nullptr));
}

TEST_F(NapiCallbackScopeTest, OpenAndClose) {
  napi_callback_scope cbScope = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope));
  EXPECT_NE(nullptr, cbScope);

  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope));
}

TEST_F(NapiCallbackScopeTest, OpenWithAsyncContext) {
  auto scope = openScope(env_);

  // Create an async context.
  napi_value name = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &name));

  napi_async_context ctx = nullptr;
  EXPECT_EQ(napi_ok, napi_async_init(env_, nullptr, name, &ctx));

  napi_callback_scope cbScope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_callback_scope(env_, nullptr, ctx, &cbScope));
  EXPECT_NE(nullptr, cbScope);

  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope));
  EXPECT_EQ(napi_ok, napi_async_destroy(env_, ctx));

  closeScope(env_, scope);
}

TEST_F(NapiCallbackScopeTest, NestedScopes) {
  napi_callback_scope cbScope1 = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope1));

  napi_callback_scope cbScope2 = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope2));

  // Close in reverse order.
  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope2));
  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope1));
}

TEST_F(NapiCallbackScopeTest, CloseMismatch) {
  // Closing when no scopes are open should return mismatch.
  // We need a non-null scope pointer to pass the null check.
  // Open and close one first to get a valid-looking pointer,
  // then try to close again.
  napi_callback_scope cbScope = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope));
  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope));

  // Now open_callback_scopes is 0. Open a new scope, close it,
  // then attempt to close a third time which is a mismatch.
  napi_callback_scope cbScope2 = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope2));
  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope2));

  // Allocate a dummy scope to pass the null check but trigger
  // the counter mismatch.
  napi_callback_scope cbScope3 = nullptr;
  EXPECT_EQ(
      napi_ok, napi_open_callback_scope(env_, nullptr, nullptr, &cbScope3));
  EXPECT_EQ(napi_ok, napi_close_callback_scope(env_, cbScope3));

  // The counter is now 0 again. Create a scope manually and try
  // to close — but we can't create one without open. Instead,
  // just verify the counter behavior by checking mismatch is
  // returned when we fabricate a non-null pointer.
  // Use reinterpret_cast to create a non-null opaque pointer.
  napi_callback_scope fakeScope = reinterpret_cast<napi_callback_scope>(1);
  EXPECT_EQ(
      napi_callback_scope_mismatch, napi_close_callback_scope(env_, fakeScope));
}

} // namespace
