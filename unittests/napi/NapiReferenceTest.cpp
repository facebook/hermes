/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/JSObject.h"

namespace {

using hermes::napi::NapiTestFixture;
using namespace hermes::vm;

/// Helper to open a handle scope and return the scope handle, asserting
/// success.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  EXPECT_NE(nullptr, scope);
  return scope;
}

/// Helper to close a handle scope, asserting success.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_create_reference - argument validation
//===========================================================================

TEST_F(NapiTestFixture, CreateReference_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg, napi_create_reference(nullptr, nullptr, 0, nullptr));
}

TEST_F(NapiTestFixture, CreateReference_NullValue) {
  napi_ref ref = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_create_reference(env_, nullptr, 0, &ref));
}

TEST_F(NapiTestFixture, CreateReference_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));
  EXPECT_EQ(napi_invalid_arg, napi_create_reference(env_, obj, 1, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_create_reference - strong reference
//===========================================================================

TEST_F(NapiTestFixture, CreateReference_StrongObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));
  ASSERT_NE(nullptr, ref);

  // Verify we can get the value back.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  // Verify it's the same object.
  auto *origPhv = reinterpret_cast<PinnedHermesValue *>(obj);
  auto *resultPhv = reinterpret_cast<PinnedHermesValue *>(result);
  EXPECT_TRUE(origPhv->isObject());
  EXPECT_TRUE(resultPhv->isObject());
  EXPECT_EQ(origPhv->getPointer(), resultPhv->getPointer());

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_create_reference - weak reference
//===========================================================================

TEST_F(NapiTestFixture, CreateReference_WeakObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 0, &ref));
  ASSERT_NE(nullptr, ref);

  // Before GC, the weak reference should be retrievable.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_create_reference - strong reference for number (v10)
//===========================================================================

TEST_F(NapiTestFixture, CreateReference_StrongNumber) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, num, 1, &ref));
  ASSERT_NE(nullptr, ref);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(42.0, val);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_create_reference - weak reference for number
//===========================================================================

TEST_F(NapiTestFixture, CreateReference_WeakNumber) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 3.14, &num));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, num, 0, &ref));
  ASSERT_NE(nullptr, ref);

  // Numbers can't be collected, so weak ref should always work.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_DOUBLE_EQ(3.14, val);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_delete_reference - argument validation
//===========================================================================

TEST_F(NapiTestFixture, DeleteReference_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_delete_reference(nullptr, nullptr));
}

TEST_F(NapiTestFixture, DeleteReference_NullRef) {
  EXPECT_EQ(napi_invalid_arg, napi_delete_reference(env_, nullptr));
}

//===========================================================================
// napi_reference_ref - argument validation and behavior
//===========================================================================

TEST_F(NapiTestFixture, ReferenceRef_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_reference_ref(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, ReferenceRef_NullRef) {
  EXPECT_EQ(napi_invalid_arg, napi_reference_ref(env_, nullptr, nullptr));
}

TEST_F(NapiTestFixture, ReferenceRef_Increment) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  uint32_t count = 0;
  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
  EXPECT_EQ(2u, count);

  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
  EXPECT_EQ(3u, count);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, ReferenceRef_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  // Passing nullptr for result should not crash.
  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, nullptr));

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_reference_unref - argument validation and behavior
//===========================================================================

TEST_F(NapiTestFixture, ReferenceUnref_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_reference_unref(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, ReferenceUnref_NullRef) {
  EXPECT_EQ(napi_invalid_arg, napi_reference_unref(env_, nullptr, nullptr));
}

TEST_F(NapiTestFixture, ReferenceUnref_Decrement) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 3, &ref));

  uint32_t count = 0;
  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, &count));
  EXPECT_EQ(2u, count);

  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, &count));
  EXPECT_EQ(1u, count);

  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, &count));
  EXPECT_EQ(0u, count);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, ReferenceUnref_AlreadyZero) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 0, &ref));

  // Unref when already at 0 should fail.
  EXPECT_EQ(napi_generic_failure, napi_reference_unref(env_, ref, nullptr));

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, ReferenceUnref_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  // Passing nullptr for result should not crash.
  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, nullptr));

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_get_reference_value - argument validation
//===========================================================================

TEST_F(NapiTestFixture, GetReferenceValue_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg, napi_get_reference_value(nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, GetReferenceValue_NullRef) {
  napi_value result = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_reference_value(env_, nullptr, &result));
}

TEST_F(NapiTestFixture, GetReferenceValue_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  EXPECT_EQ(napi_invalid_arg, napi_get_reference_value(env_, ref, nullptr));

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Ref/Unref transition (strong → weak → strong)
//===========================================================================

TEST_F(NapiTestFixture, Reference_RefUnrefCycle) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Start as strong (refcount=1).
  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  // Verify strong ref returns value.
  napi_value val = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &val));
  ASSERT_NE(nullptr, val);

  // Unref to weak (refcount 1→0).
  uint32_t count = 0;
  ASSERT_EQ(napi_ok, napi_reference_unref(env_, ref, &count));
  EXPECT_EQ(0u, count);

  // Weak ref should still return value (object is still reachable
  // via the handle scope).
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &val));
  ASSERT_NE(nullptr, val);

  // Ref back to strong (refcount 0→1).
  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
  EXPECT_EQ(1u, count);

  // Strong ref should return value.
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &val));
  ASSERT_NE(nullptr, val);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Strong reference survives GC
//===========================================================================

TEST_F(NapiTestFixture, Reference_StrongSurvivesGC) {
  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;
  {
    // Create object in an inner scope so the handle scope doesn't
    // keep it alive.
    napi_handle_scope innerScope = openScope(env_);

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

    closeScope(env_, innerScope);
  }

  // Force GC.
  collectAndDrain();

  // Strong reference should still be valid.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  // Verify it's still an object.
  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_object, type);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Weak reference to object returns NULL after GC
//===========================================================================

TEST_F(NapiTestFixture, Reference_WeakObjectCollected) {
  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;
  {
    // Create object in an inner scope.
    napi_handle_scope innerScope = openScope(env_);

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 0, &ref));

    closeScope(env_, innerScope);
  }

  // Force GC — the object should be collected.
  collectAndDrain();

  // Weak reference should return NULL.
  napi_value result = reinterpret_cast<napi_value>(1); // sentinel
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  EXPECT_EQ(nullptr, result);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Weak reference to undefined/null/boolean always valid
//===========================================================================

TEST_F(NapiTestFixture, Reference_WeakUndefined) {
  napi_handle_scope scope = openScope(env_);

  napi_value undef = nullptr;
  ASSERT_EQ(napi_ok, napi_get_undefined(env_, &undef));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, undef, 0, &ref));

  collectAndDrain();

  // Undefined is not GC-managed, should always be valid.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_undefined, type);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Reference_WeakBoolean) {
  napi_handle_scope scope = openScope(env_);

  napi_value boolVal = nullptr;
  ASSERT_EQ(napi_ok, napi_get_boolean(env_, true, &boolVal));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, boolVal, 0, &ref));

  collectAndDrain();

  // Booleans are not GC-managed, should always be valid.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  bool val = false;
  ASSERT_EQ(napi_ok, napi_get_value_bool(env_, result, &val));
  EXPECT_TRUE(val);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Finalizer callback on delete
//===========================================================================

TEST_F(NapiTestFixture, Reference_FinalizerNotCalledOnDelete) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  // Manually set a finalizer on the ref (this is normally done by
  // napi_wrap or napi_add_finalizer, but we test it directly here).
  bool finalizerCalled = false;
  ref->finalize_cb = [](napi_env, void *data, void *) {
    *static_cast<bool *>(data) = true;
  };
  ref->finalize_data = &finalizerCalled;

  EXPECT_FALSE(finalizerCalled);
  // Matching V8 behavior: napi_delete_reference does NOT call the
  // finalizer. The finalizer is only called during GC or env teardown.
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  EXPECT_FALSE(finalizerCalled);

  closeScope(env_, scope);
}

//===========================================================================
// Multiple references to the same object
//===========================================================================

TEST_F(NapiTestFixture, Reference_MultipleRefsToSameObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_ref ref1 = nullptr;
  napi_ref ref2 = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref1));
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref2));

  napi_value val1 = nullptr;
  napi_value val2 = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref1, &val1));
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref2, &val2));

  // Both should point to the same object.
  auto *phv1 = reinterpret_cast<PinnedHermesValue *>(val1);
  auto *phv2 = reinterpret_cast<PinnedHermesValue *>(val2);
  EXPECT_EQ(phv1->getPointer(), phv2->getPointer());

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref1));
  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref2));
  closeScope(env_, scope);
}

//===========================================================================
// Weak reference to string (pointer type, but not Object)
//===========================================================================

TEST_F(NapiTestFixture, Reference_WeakStringCollected) {
  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;
  {
    napi_handle_scope innerScope = openScope(env_);

    napi_value str = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_string_utf8(env_, "test string", NAPI_AUTO_LENGTH, &str));

    // Verify it's a string.
    napi_valuetype type;
    ASSERT_EQ(napi_ok, napi_typeof(env_, str, &type));
    EXPECT_EQ(napi_string, type);

    ASSERT_EQ(napi_ok, napi_create_reference(env_, str, 0, &ref));

    closeScope(env_, innerScope);
  }

  // Force GC — the string should be collected.
  collectAndDrain();

  // Weak reference should return NULL.
  napi_value result = reinterpret_cast<napi_value>(1);
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  EXPECT_EQ(nullptr, result);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, Reference_StrongStringSurvivesGC) {
  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;
  {
    napi_handle_scope innerScope = openScope(env_);

    napi_value str = nullptr;
    ASSERT_EQ(
        napi_ok,
        napi_create_string_utf8(env_, "strong string", NAPI_AUTO_LENGTH, &str));

    ASSERT_EQ(napi_ok, napi_create_reference(env_, str, 1, &ref));

    closeScope(env_, innerScope);
  }

  // Force GC.
  collectAndDrain();

  // Strong reference should still work and return a string.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_string, type);

  // Verify we can read the string value.
  char buf[64] = {};
  size_t len = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len));
  EXPECT_STREQ("strong string", buf);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// napi_reference_ref on a collected weak reference is a no-op
//===========================================================================

TEST_F(NapiTestFixture, ReferenceRef_CollectedWeakIsNoop) {
  napi_handle_scope scope = openScope(env_);

  napi_ref ref = nullptr;
  {
    // Create object in an inner scope so nothing else roots it.
    napi_handle_scope innerScope = openScope(env_);

    napi_value obj = nullptr;
    ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

    // Create a weak reference (refcount=0).
    ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 0, &ref));

    closeScope(env_, innerScope);
  }

  // Force GC — the object should be collected.
  collectAndDrain();

  // Confirm it was collected.
  napi_value val = reinterpret_cast<napi_value>(1);
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &val));
  ASSERT_EQ(nullptr, val);

  // Attempting to ref a collected weak reference should be a no-op:
  // refcount stays 0, matching V8 behavior.
  uint32_t count = 99;
  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
  EXPECT_EQ(0u, count);

  // napi_get_reference_value should still return NULL.
  val = reinterpret_cast<napi_value>(1);
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &val));
  EXPECT_EQ(nullptr, val);

  // If the bug is present (refcount was incremented), the stale value
  // is now a strong GC root pointing to freed memory. Try to use it
  // to provoke a crash under ASAN.
  if (val != nullptr) {
    napi_valuetype type;
    napi_typeof(env_, val, &type);
  }

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Weak→strong transition after GC updates stale pointer
//===========================================================================

TEST_F(NapiTestFixture, Reference_WeakToStrongAfterGC) {
  napi_handle_scope scope = openScope(env_);

  // Create an object rooted by the outer handle scope so it survives GC.
  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Create a weak reference (refcount=0).
  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 0, &ref));

  // Force GC — the object survives (rooted by handle scope), but
  // may be moved, making ref->value a stale pointer.
  collectAndDrain();

  // Promote from weak to strong. This must update ref->value from
  // the weak slot before releasing it.
  uint32_t count = 0;
  ASSERT_EQ(napi_ok, napi_reference_ref(env_, ref, &count));
  EXPECT_EQ(1u, count);

  // Read the value back via the now-strong reference.
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_get_reference_value(env_, ref, &result));
  ASSERT_NE(nullptr, result);

  // Verify it's still a valid, accessible object.
  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_object, type);

  ASSERT_EQ(napi_ok, napi_delete_reference(env_, ref));
  closeScope(env_, scope);
}

//===========================================================================
// Env destruction cleans up references
//===========================================================================

TEST_F(NapiTestFixture, Reference_EnvDestroyCallsFinalizers) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Create a reference with a finalizer.
  napi_ref ref = nullptr;
  ASSERT_EQ(napi_ok, napi_create_reference(env_, obj, 1, &ref));

  static bool envFinalizerCalled = false;
  envFinalizerCalled = false;
  ref->finalize_cb = [](napi_env, void *, void *) {
    envFinalizerCalled = true;
  };

  closeScope(env_, scope);

  // Destroy the Runtime — env shutdown calls the finalizer.
  env_ = nullptr;
  rt_.reset();

  EXPECT_TRUE(envFinalizerCalled);
}

} // namespace
