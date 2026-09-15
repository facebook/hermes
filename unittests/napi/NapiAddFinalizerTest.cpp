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
// napi_add_finalizer - argument validation
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_add_finalizer(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, AddFinalizer_NullJsObject) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_add_finalizer(
          env_,
          nullptr,
          nullptr,
          [](napi_env, void *, void *) {},
          nullptr,
          nullptr));
}

TEST_F(NapiTestFixture, AddFinalizer_NullCallback) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_add_finalizer(env_, obj, nullptr, nullptr, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, AddFinalizer_NotAnObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_add_finalizer(
          env_,
          num,
          nullptr,
          [](napi_env, void *, void *) {},
          nullptr,
          nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_add_finalizer - basic finalizer
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_CalledOnGC) {
  bool finalizerCalled = false;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            nullptr,
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

TEST_F(NapiTestFixture, AddFinalizer_FinalizerData) {
  void *receivedData = nullptr;

  napi_handle_scope scope = openScope(env_);

  {
    int nativeData = 42;
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            &nativeData,
            [](napi_env, void *data, void *hint) {
              *static_cast<void **>(hint) = data;
            },
            &receivedData,
            nullptr));
  }

  closeScope(env_, scope);
  collectAndDrain();

  // The finalizer should have received the data pointer.
  EXPECT_NE(nullptr, receivedData);
}

TEST_F(NapiTestFixture, AddFinalizer_Hint) {
  int hintValue = 0;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<int *>(hint) = 123;
            },
            &hintValue,
            nullptr));
  }

  closeScope(env_, scope);
  collectAndDrain();

  EXPECT_EQ(123, hintValue);
}

//===========================================================================
// napi_add_finalizer - multiple finalizers on the same object
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_MultipleFinalizers) {
  int callCount = 0;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    // Add three finalizers to the same object.
    for (int i = 0; i < 3; ++i) {
      ASSERT_EQ(
          napi_ok,
          napi_add_finalizer(
              env_,
              obj,
              nullptr,
              [](napi_env, void *, void *hint) { ++*static_cast<int *>(hint); },
              &callCount,
              nullptr));
    }
  }

  closeScope(env_, scope);
  collectAndDrain();

  // All three finalizers should have been called.
  EXPECT_EQ(3, callCount);
}

//===========================================================================
// napi_add_finalizer - coexistence with napi_wrap
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_CoexistsWithWrap) {
  bool wrapFinalizerCalled = false;
  bool addFinalizerCalled = false;

  napi_handle_scope scope = openScope(env_);

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    int nativeData = 1;

    // First, wrap the object.
    ASSERT_EQ(
        napi_ok,
        napi_wrap(
            env_,
            obj,
            &nativeData,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &wrapFinalizerCalled,
            nullptr));

    // Then add a separate finalizer via napi_add_finalizer.
    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &addFinalizerCalled,
            nullptr));

    // Verify unwrap still works.
    void *data = nullptr;
    ASSERT_EQ(napi_ok, napi_unwrap(env_, obj, &data));
    EXPECT_EQ(&nativeData, data);
  }

  closeScope(env_, scope);
  collectAndDrain();

  // Both finalizers should have been called.
  EXPECT_TRUE(wrapFinalizerCalled);
  EXPECT_TRUE(addFinalizerCalled);
}

//===========================================================================
// napi_add_finalizer - with result (napi_ref)
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_WithRef) {
  bool finalizerCalled = false;

  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &finalizerCalled,
            &ref));
    ASSERT_NE(nullptr, ref);
  }

  closeScope(env_, scope);

  // The ref is weak (refcount 0), so GC should collect the object.
  collectAndDrain();

  // The finalizer should have been called.
  EXPECT_TRUE(finalizerCalled);

  // Clean up the reference.
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
}

TEST_F(NapiTestFixture, AddFinalizer_RefKeepsObjectAliveWhenStrong) {
  bool finalizerCalled = false;

  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;

  {
    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            obj,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &finalizerCalled,
            &ref));
    ASSERT_NE(nullptr, ref);

    // Make the ref strong so it keeps the object alive.
    uint32_t count = 0;
    ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
    EXPECT_EQ(1u, count);
  }

  closeScope(env_, scope);

  // GC should NOT collect the object because the ref is strong.
  collectAndDrain();
  EXPECT_FALSE(finalizerCalled);

  // Make the ref weak again.
  uint32_t count = 0;
  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, &count));
  EXPECT_EQ(0u, count);

  // Now GC should collect the object.
  collectAndDrain();
  EXPECT_TRUE(finalizerCalled);

  // Clean up.
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
}

//===========================================================================
// napi_add_finalizer - finalizer on external
//===========================================================================

TEST_F(NapiTestFixture, AddFinalizer_OnExternal) {
  bool extFinalizerCalled = false;
  bool addFinalizerCalled = false;

  napi_handle_scope scope = openScope(env_);

  {
    int nativeData = 42;
    napi_value ext = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_external(
            env_,
            &nativeData,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &extFinalizerCalled,
            &ext));

    // Add an additional finalizer to the external.
    ASSERT_EQ(
        napi_ok,
        napi_add_finalizer(
            env_,
            ext,
            nullptr,
            [](napi_env, void *, void *hint) {
              *static_cast<bool *>(hint) = true;
            },
            &addFinalizerCalled,
            nullptr));
  }

  closeScope(env_, scope);
  collectAndDrain();

  // Both the external's finalizer and the added finalizer should fire.
  EXPECT_TRUE(extFinalizerCalled);
  EXPECT_TRUE(addFinalizerCalled);
}

} // namespace
