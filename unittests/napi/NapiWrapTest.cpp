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
  return scope;
}

/// Helper to close a handle scope.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_wrap - argument validation
//===========================================================================

TEST_F(NapiTestFixture, Wrap_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_wrap(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, Wrap_NullJsObject) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_wrap(env_, nullptr, nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, Wrap_NotAnObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));
  EXPECT_EQ(
      napi_invalid_arg,
      napi_wrap(env_, num, nullptr, nullptr, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Wrap_ResultRequiresFinalizer) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // When result is non-null, finalize_cb is required.
  napi_ref ref = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_wrap(env_, obj, nullptr, nullptr, nullptr, &ref));

  closeScope(env_, scope);
}

//===========================================================================
// napi_wrap / napi_unwrap - basic usage
//===========================================================================

TEST_F(NapiTestFixture, WrapUnwrap_Basic) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  int nativeData = 42;
  ASSERT_EQ(
      napi_ok, napi_wrap(env_, obj, &nativeData, nullptr, nullptr, nullptr));

  // Unwrap should return the same pointer.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
  EXPECT_EQ(&nativeData, data);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, WrapUnwrap_NullNativeData) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  ASSERT_EQ(napi_ok, napi_wrap(env_, obj, nullptr, nullptr, nullptr, nullptr));

  void *data = reinterpret_cast<void *>(0xdeadbeef);
  ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
  EXPECT_EQ(nullptr, data);

  closeScope(env_, scope);
}

//===========================================================================
// napi_wrap - double wrap error
//===========================================================================

TEST_F(NapiTestFixture, Wrap_DoubleWrapError) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  int data1 = 1;
  ASSERT_EQ(napi_ok, napi_wrap(env_, obj, &data1, nullptr, nullptr, nullptr));

  // Wrapping the same object again should fail.
  int data2 = 2;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_wrap(env_, obj, &data2, nullptr, nullptr, nullptr));

  // The original wrap should still be intact.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
  EXPECT_EQ(&data1, data);

  closeScope(env_, scope);
}

//===========================================================================
// napi_unwrap - argument validation
//===========================================================================

TEST_F(NapiTestFixture, Unwrap_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, Unwrap_NullJsObject) {
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, nullptr, &data));
}

TEST_F(NapiTestFixture, Unwrap_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, obj, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Unwrap_NotWrapped) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Unwrapping an object that wasn't wrapped should fail.
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, obj, &data));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Unwrap_NotAnObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value str = nullptr;
  ASSERT_EQ(napi_ok, napi_create_string_utf8(env_, "hello", 5, &str));

  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, str, &data));

  closeScope(env_, scope);
}

//===========================================================================
// napi_remove_wrap
//===========================================================================

TEST_F(NapiTestFixture, RemoveWrap_Basic) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  int nativeData = 42;
  ASSERT_EQ(
      napi_ok, napi_wrap(env_, obj, &nativeData, nullptr, nullptr, nullptr));

  // Remove the wrap.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_remove_wrap(env_, obj, &data));
  EXPECT_EQ(&nativeData, data);

  // After removing, unwrap should fail.
  data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, obj, &data));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, RemoveWrap_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  int nativeData = 42;
  ASSERT_EQ(
      napi_ok, napi_wrap(env_, obj, &nativeData, nullptr, nullptr, nullptr));

  // Remove with null result (just remove, don't retrieve).
  ASSERT_EQ(napi_ok, napi_remove_wrap(env_, obj, nullptr));

  // After removing, unwrap should fail.
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_unwrap(env_, obj, &data));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, RemoveWrap_NotWrapped) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_remove_wrap(env_, obj, &data));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, RemoveWrap_CanRewrapAfterRemove) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  int data1 = 1;
  ASSERT_EQ(napi_ok, napi_wrap(env_, obj, &data1, nullptr, nullptr, nullptr));

  void *removed = nullptr;
  ASSERT_EQ(napi_ok, napi_remove_wrap(env_, obj, &removed));
  EXPECT_EQ(&data1, removed);

  // Should be able to wrap again after removing.
  int data2 = 2;
  ASSERT_EQ(napi_ok, napi_wrap(env_, obj, &data2, nullptr, nullptr, nullptr));

  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
  EXPECT_EQ(&data2, data);

  closeScope(env_, scope);
}

//===========================================================================
// napi_wrap with finalizer (no ref)
//===========================================================================

TEST_F(NapiTestFixture, Wrap_FinalizerCalledOnGC) {
  bool finalizerCalled = false;
  int nativeData = 99;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &finalizerCalled,
            nullptr));
  }

  // Close scope so the object is unreachable.
  closeScope(env_, scope);

  // Force GC.
  collectAndDrain();

  EXPECT_TRUE(finalizerCalled);
}

TEST_F(NapiTestFixture, Wrap_RemoveWrapPreventsFinalizerOnGC) {
  bool finalizerCalled = false;
  int nativeData = 99;

  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  ASSERT_EQ(
      napi_ok,
      napi_wrap(
          env_,
          obj,
          &nativeData,
          [](napi_env, void *, void *hint) {
            *static_cast<bool *>(hint) = true;
          },
          &finalizerCalled,
          nullptr));

  // Remove the wrap before the object goes out of scope.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_remove_wrap(env_, obj, &data));

  closeScope(env_, scope);
  collectAndDrain();

  // Finalizer should NOT have been called.
  EXPECT_FALSE(finalizerCalled);
}

//===========================================================================
// napi_wrap with ref
//===========================================================================

TEST_F(NapiTestFixture, Wrap_WithRef) {
  napi_handle_scope scope = openScope(env_);

  int nativeData = 42;
  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_wrap(
          env_,
          obj,
          &nativeData,
          [](napi_env, void *, void *) {},
          nullptr,
          &ref));
  ASSERT_NE(nullptr, ref);

  // Unwrap should still work.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
  EXPECT_EQ(&nativeData, data);

  closeScope(env_, scope);

  // Clean up the reference.
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
}

//===========================================================================
// napi_remove_wrap - argument validation
//===========================================================================

TEST_F(NapiTestFixture, RemoveWrap_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_remove_wrap(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, RemoveWrap_NullJsObject) {
  EXPECT_EQ(napi_invalid_arg, napi_remove_wrap(env_, nullptr, nullptr));
}

TEST_F(NapiTestFixture, RemoveWrap_NotAnObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 3.14, &num));

  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_remove_wrap(env_, num, &data));

  closeScope(env_, scope);
}

} // namespace
