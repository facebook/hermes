/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include <cstring>

#ifndef NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED

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
// napi_create_external_arraybuffer - argument validation
//===========================================================================

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_create_external_arraybuffer(
          nullptr, nullptr, 0, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_NullResult) {
  napi_handle_scope scope = openScope(env_);
  EXPECT_EQ(
      napi_invalid_arg,
      napi_create_external_arraybuffer(
          env_, nullptr, 0, nullptr, nullptr, nullptr));
  closeScope(env_, scope);
}

//===========================================================================
// napi_create_external_arraybuffer - basic creation
//===========================================================================

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_NullData) {
  napi_handle_scope scope = openScope(env_);

  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(
          env_, nullptr, 0, nullptr, nullptr, &ab));
  ASSERT_NE(nullptr, ab);

  // Verify it is an ArrayBuffer.
  bool isAB = false;
  ASSERT_EQ(napi_ok, napi_is_arraybuffer(env_, ab, &isAB));
  EXPECT_TRUE(isAB);

  // Verify info.
  void *data = nullptr;
  size_t byteLen = 99;
  ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, ab, &data, &byteLen));
  EXPECT_EQ(nullptr, data);
  EXPECT_EQ(0u, byteLen);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_WithData) {
  napi_handle_scope scope = openScope(env_);

  // Allocate external data.
  uint8_t externalBuf[16];
  for (int i = 0; i < 16; ++i)
    externalBuf[i] = static_cast<uint8_t>(i + 1);

  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(
          env_, externalBuf, 16, nullptr, nullptr, &ab));
  ASSERT_NE(nullptr, ab);

  // Verify it is an ArrayBuffer.
  bool isAB = false;
  ASSERT_EQ(napi_ok, napi_is_arraybuffer(env_, ab, &isAB));
  EXPECT_TRUE(isAB);

  // Verify info returns the same data pointer and correct length.
  void *data = nullptr;
  size_t byteLen = 0;
  ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, ab, &data, &byteLen));
  EXPECT_EQ(externalBuf, data);
  EXPECT_EQ(16u, byteLen);

  // Verify data contents are accessible.
  auto *bytes = static_cast<uint8_t *>(data);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(static_cast<uint8_t>(i + 1), bytes[i]);
  }

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_WriteAndRead) {
  napi_handle_scope scope = openScope(env_);

  // Allocate external data.
  uint8_t externalBuf[8] = {0};

  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(
          env_, externalBuf, 8, nullptr, nullptr, &ab));

  // Write through the external pointer.
  externalBuf[0] = 0xAA;
  externalBuf[7] = 0xBB;

  // Read back via napi_get_arraybuffer_info.
  void *data = nullptr;
  size_t byteLen = 0;
  ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, ab, &data, &byteLen));
  EXPECT_EQ(8u, byteLen);

  auto *bytes = static_cast<uint8_t *>(data);
  EXPECT_EQ(0xAA, bytes[0]);
  EXPECT_EQ(0xBB, bytes[7]);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_TypeofIsObject) {
  napi_handle_scope scope = openScope(env_);

  uint8_t buf[4] = {0};
  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(env_, buf, 4, nullptr, nullptr, &ab));

  napi_valuetype type = napi_undefined;
  ASSERT_EQ(napi_ok, napi_typeof(env_, ab, &type));
  EXPECT_EQ(napi_object, type);

  closeScope(env_, scope);
}

//===========================================================================
// napi_create_external_arraybuffer - finalizer
//===========================================================================

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_FinalizerCalled) {
  bool finalizerCalled = false;

  {
    napi_handle_scope scope = openScope(env_);

    auto *externalBuf = new uint8_t[32];
    napi_value ab = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external_arraybuffer(
            env_,
            externalBuf,
            32,
            [](napi_env, void *data, void *hint) {
              *static_cast<bool *>(hint) = true;
              delete[] static_cast<uint8_t *>(data);
            },
            &finalizerCalled,
            &ab));

    // Capture the data pointer for verification.
    void *infoData = nullptr;
    ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, ab, &infoData, nullptr));
    EXPECT_EQ(externalBuf, infoData);

    closeScope(env_, scope);
  }

  // Force GC to collect the ArrayBuffer and trigger the finalizer.
  collectAndDrain();

  EXPECT_TRUE(finalizerCalled);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_FinalizerHint) {
  int hintValue = 0;

  {
    napi_handle_scope scope = openScope(env_);

    uint8_t externalBuf[4] = {0};
    napi_value ab = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external_arraybuffer(
            env_,
            externalBuf,
            4,
            [](napi_env, void *, void *hint) {
              *static_cast<int *>(hint) = 42;
            },
            &hintValue,
            &ab));

    closeScope(env_, scope);
  }

  collectAndDrain();

  EXPECT_EQ(42, hintValue);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_NoFinalizerNoCrash) {
  {
    napi_handle_scope scope = openScope(env_);

    uint8_t buf[8] = {0};
    napi_value ab = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external_arraybuffer(env_, buf, 8, nullptr, nullptr, &ab));

    closeScope(env_, scope);
  }

  // Should not crash when the ArrayBuffer is collected without
  // a finalizer.
  collectAndDrain();
}

//===========================================================================
// napi_create_external_arraybuffer - data pointer stability
//===========================================================================

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_DataPointerStable) {
  // Verify that the data pointer returned by napi_get_arraybuffer_info
  // is the same as the pointer passed to
  // napi_create_external_arraybuffer, even after GC.
  napi_handle_scope scope = openScope(env_);

  auto *externalBuf = new uint8_t[64];

  // Create a reference to prevent GC collection.
  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(
          env_,
          externalBuf,
          64,
          [](napi_env, void *data, void *) {
            delete[] static_cast<uint8_t *>(data);
          },
          nullptr,
          &ab));

  // Create a strong reference so it survives GC.
  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, ab, 1, &ref));

  // Force GC.
  collectAndDrain();

  // Get the value from the reference.
  napi_value refVal = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &refVal));
  ASSERT_NE(nullptr, refVal);

  // Verify data pointer is still the original external buffer.
  void *data = nullptr;
  size_t byteLen = 0;
  ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, refVal, &data, &byteLen));
  EXPECT_EQ(externalBuf, data);
  EXPECT_EQ(64u, byteLen);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateExternalArrayBuffer_ZeroLengthWithData) {
  // External ArrayBuffer with zero length but non-null data pointer.
  napi_handle_scope scope = openScope(env_);

  uint8_t buf[1] = {0xCC};
  napi_value ab = nullptr;
  ASSERT_EQ(
      napi_ok,
      napi_create_external_arraybuffer(env_, buf, 0, nullptr, nullptr, &ab));

  void *data = nullptr;
  size_t byteLen = 99;
  ASSERT_EQ(napi_ok, napi_get_arraybuffer_info(env_, ab, &data, &byteLen));
  // The data pointer should be the external pointer even for zero
  // length.
  EXPECT_EQ(buf, data);
  EXPECT_EQ(0u, byteLen);

  closeScope(env_, scope);
}

} // namespace

#endif // NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
