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
// napi_create_external - argument validation
//===========================================================================

TEST_F(NapiTestFixture, CreateExternal_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_create_external(nullptr, nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, CreateExternal_NullResult) {
  napi_handle_scope scope = openScope(env_);
  EXPECT_EQ(
      napi_invalid_arg,
      napi_create_external(env_, nullptr, nullptr, nullptr, nullptr));
  closeScope(env_, scope);
}

//===========================================================================
// napi_create_external - basic creation
//===========================================================================

TEST_F(NapiTestFixture, CreateExternal_NullData) {
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));
  ASSERT_NE(nullptr, ext);

  // Verify typeof returns napi_external.
  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, ext, &type));
  EXPECT_EQ(napi_external, type);

  // Verify extracted data is null.
  void *data = reinterpret_cast<void *>(0xdeadbeef);
  ASSERT_EQ(napi_ok, napi_get_value_external(env_, ext, &data));
  EXPECT_EQ(nullptr, data);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateExternal_WithData) {
  napi_handle_scope scope = openScope(env_);

  int nativeData = 42;
  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, &nativeData, nullptr, nullptr, &ext));
  ASSERT_NE(nullptr, ext);

  // Verify typeof returns napi_external.
  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, ext, &type));
  EXPECT_EQ(napi_external, type);

  // Verify extracted data pointer matches.
  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_get_value_external(env_, ext, &data));
  EXPECT_EQ(&nativeData, data);

  closeScope(env_, scope);
}

//===========================================================================
// napi_get_value_external - argument validation
//===========================================================================

TEST_F(NapiTestFixture, GetValueExternal_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg, napi_get_value_external(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, GetValueExternal_NullValue) {
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_value_external(env_, nullptr, &data));
}

TEST_F(NapiTestFixture, GetValueExternal_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));
  EXPECT_EQ(napi_invalid_arg, napi_get_value_external(env_, ext, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetValueExternal_WrongType_Object) {
  napi_handle_scope scope = openScope(env_);

  // A plain object is not an external.
  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_value_external(env_, obj, &data));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetValueExternal_WrongType_Number) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 3.14, &num));
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_value_external(env_, num, &data));

  closeScope(env_, scope);
}

//===========================================================================
// napi_typeof - external detection
//===========================================================================

TEST_F(NapiTestFixture, Typeof_External) {
  napi_handle_scope scope = openScope(env_);

  int nativeData = 0;
  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, &nativeData, nullptr, nullptr, &ext));

  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, ext, &type));
  EXPECT_EQ(napi_external, type);

  // An external is NOT napi_object.
  EXPECT_NE(napi_object, type);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Typeof_ObjectIsNotExternal) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, obj, &type));
  EXPECT_EQ(napi_object, type);
  EXPECT_NE(napi_external, type);

  closeScope(env_, scope);
}

//===========================================================================
// napi_create_external - finalizer
//===========================================================================

TEST_F(NapiTestFixture, CreateExternal_FinalizerCalled) {
  bool finalizerCalled = false;
  int nativeData = 99;

  napi_handle_scope scope = openScope(env_);

  // Create the external with a weak reference and finalizer.
  {
    napi_value ext = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external(
            env_,
            &nativeData,
            [](napi_env, void *data, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &finalizerCalled,
            &ext));
    ASSERT_NE(nullptr, ext);

    // Verify the data pointer.
    void *extractedData = nullptr;
    ASSERT_EQ(napi_ok, napi_get_value_external(env_, ext, &extractedData));
    EXPECT_EQ(&nativeData, extractedData);
  }

  // Close the handle scope so the external is unreachable.
  closeScope(env_, scope);

  // Force a GC to collect the external and trigger the finalizer.
  collectAndDrain();

  EXPECT_TRUE(finalizerCalled);
}

TEST_F(NapiTestFixture, CreateExternal_FinalizerHint) {
  int hintValue = 0;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value ext = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external(
            env_,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<int *>(hint) = 123;
            },
            &hintValue,
            &ext));
    ASSERT_NE(nullptr, ext);
  }

  closeScope(env_, scope);
  collectAndDrain();

  EXPECT_EQ(123, hintValue);
}

TEST_F(NapiTestFixture, CreateExternal_NoFinalizer) {
  // Creating an external without a finalizer should work fine.
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));
  ASSERT_NE(nullptr, ext);

  closeScope(env_, scope);

  // Should not crash when the external is collected.
  collectAndDrain();
}

//===========================================================================
// External with reference
//===========================================================================

TEST_F(NapiTestFixture, CreateExternal_WithStrongRef) {
  napi_handle_scope scope = openScope(env_);

  int nativeData = 42;
  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, &nativeData, nullptr, nullptr, &ext));

  // Create a strong reference to keep the external alive.
  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, ext, 1, &ref));

  closeScope(env_, scope);

  // Even after closing the scope, the reference should keep it alive.
  collectAndDrain();

  // Get the value from the reference.
  napi_handle_scope scope2 = openScope(env_);

  napi_value refVal = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &refVal));
  ASSERT_NE(nullptr, refVal);

  // Verify it's still an external with the right data.
  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, refVal, &type));
  EXPECT_EQ(napi_external, type);

  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_get_value_external(env_, refVal, &data));
  EXPECT_EQ(&nativeData, data);

  closeScope(env_, scope2);

  // Clean up the reference.
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
}

} // namespace
