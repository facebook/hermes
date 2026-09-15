/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {
using namespace hermes::napi;

class NapiComparisonTest : public NapiTestFixture {
 protected:
  void openScope() {
    napi_handle_scope scope;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope), napi_ok);
    scope_ = scope;
  }
  void closeScope() {
    ASSERT_EQ(napi_close_handle_scope(env_, scope_), napi_ok);
  }

  napi_handle_scope scope_ = nullptr;
};

//===========================================================================
// napi_strict_equals
//===========================================================================

TEST_F(NapiComparisonTest, StrictEqualsNullArgs) {
  openScope();
  napi_value val;
  ASSERT_EQ(napi_get_undefined(env_, &val), napi_ok);
  bool result;

  // Null env.
  EXPECT_EQ(napi_strict_equals(nullptr, val, val, &result), napi_invalid_arg);
  // Null lhs.
  EXPECT_EQ(napi_strict_equals(env_, nullptr, val, &result), napi_invalid_arg);
  // Null rhs.
  EXPECT_EQ(napi_strict_equals(env_, val, nullptr, &result), napi_invalid_arg);
  // Null result.
  EXPECT_EQ(napi_strict_equals(env_, val, val, nullptr), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameUndefined) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_get_undefined(env_, &a), napi_ok);
  ASSERT_EQ(napi_get_undefined(env_, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameNull) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_get_null(env_, &a), napi_ok);
  ASSERT_EQ(napi_get_null(env_, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsUndefinedVsNull) {
  openScope();
  napi_value undef, null;
  ASSERT_EQ(napi_get_undefined(env_, &undef), napi_ok);
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, undef, null, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameNumber) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_create_double(env_, 42.0, &a), napi_ok);
  ASSERT_EQ(napi_create_double(env_, 42.0, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsDifferentNumbers) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_create_double(env_, 42.0, &a), napi_ok);
  ASSERT_EQ(napi_create_double(env_, 43.0, &b), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsNaNNotEqualNaN) {
  openScope();
  napi_value a, b;
  double nan = std::numeric_limits<double>::quiet_NaN();
  ASSERT_EQ(napi_create_double(env_, nan, &a), napi_ok);
  ASSERT_EQ(napi_create_double(env_, nan, &b), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameString) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &a), napi_ok);
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsDifferentStrings) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &a), napi_ok);
  ASSERT_EQ(
      napi_create_string_utf8(env_, "world", NAPI_AUTO_LENGTH, &b), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameBooleans) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_get_boolean(env_, true, &a), napi_ok);
  ASSERT_EQ(napi_get_boolean(env_, true, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsDifferentBooleans) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_get_boolean(env_, true, &a), napi_ok);
  ASSERT_EQ(napi_get_boolean(env_, false, &b), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsSameObject) {
  openScope();
  napi_value obj;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, obj, obj, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsDifferentObjects) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_create_object(env_, &a), napi_ok);
  ASSERT_EQ(napi_create_object(env_, &b), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsNumberVsString) {
  openScope();
  napi_value num, str;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);
  ASSERT_EQ(
      napi_create_string_utf8(env_, "42", NAPI_AUTO_LENGTH, &str), napi_ok);

  bool result = true;
  ASSERT_EQ(napi_strict_equals(env_, num, str, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, StrictEqualsIntVsDouble) {
  openScope();
  napi_value a, b;
  ASSERT_EQ(napi_create_int32(env_, 5, &a), napi_ok);
  ASSERT_EQ(napi_create_double(env_, 5.0, &b), napi_ok);

  bool result = false;
  ASSERT_EQ(napi_strict_equals(env_, a, b, &result), napi_ok);
  EXPECT_TRUE(result);
  closeScope();
}

//===========================================================================
// napi_instanceof
//===========================================================================

TEST_F(NapiComparisonTest, InstanceofNullArgs) {
  openScope();
  napi_value obj, ctor;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

  // Create a constructor function.
  auto ctorCb = [](napi_env, napi_callback_info) -> napi_value {
    return nullptr;
  };
  ASSERT_EQ(
      napi_create_function(
          env_, "Ctor", NAPI_AUTO_LENGTH, ctorCb, nullptr, &ctor),
      napi_ok);

  bool result;

  // Null env.
  EXPECT_EQ(napi_instanceof(nullptr, obj, ctor, &result), napi_invalid_arg);
  // Null object.
  EXPECT_EQ(napi_instanceof(env_, nullptr, ctor, &result), napi_invalid_arg);
  // Null result.
  EXPECT_EQ(napi_instanceof(env_, obj, ctor, nullptr), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiComparisonTest, InstanceofNonFunction) {
  openScope();
  napi_value obj, notFunc;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
  ASSERT_EQ(napi_create_object(env_, &notFunc), napi_ok);

  bool result;
  // Constructor must be a function.
  EXPECT_EQ(
      napi_instanceof(env_, obj, notFunc, &result), napi_function_expected);
  closeScope();
}

TEST_F(NapiComparisonTest, InstanceofWithDefineClass) {
  openScope();

  // Create a class via napi_define_class.
  auto ctorCb = [](napi_env env, napi_callback_info info) -> napi_value {
    return nullptr;
  };

  napi_value ctor;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "MyClass",
          NAPI_AUTO_LENGTH,
          ctorCb,
          nullptr,
          0,
          nullptr,
          &ctor),
      napi_ok);

  // Create an instance.
  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &instance), napi_ok);

  // instanceof should return true.
  bool result = false;
  ASSERT_EQ(napi_instanceof(env_, instance, ctor, &result), napi_ok);
  EXPECT_TRUE(result);

  // A plain object should NOT be an instance.
  napi_value plain;
  ASSERT_EQ(napi_create_object(env_, &plain), napi_ok);

  result = true;
  ASSERT_EQ(napi_instanceof(env_, plain, ctor, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, InstanceofPrimitiveObject) {
  openScope();

  // Getting the global Array constructor to test instanceof.
  napi_value global;
  ASSERT_EQ(napi_get_global(env_, &global), napi_ok);

  napi_value arrayCtor;
  ASSERT_EQ(
      napi_get_named_property(env_, global, "Array", &arrayCtor), napi_ok);

  // Create an array.
  napi_value arr;
  ASSERT_EQ(napi_create_array(env_, &arr), napi_ok);

  // arr instanceof Array should be true.
  bool result = false;
  ASSERT_EQ(napi_instanceof(env_, arr, arrayCtor, &result), napi_ok);
  EXPECT_TRUE(result);

  // A number should not be instanceof Array.
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);

  result = true;
  ASSERT_EQ(napi_instanceof(env_, num, arrayCtor, &result), napi_ok);
  EXPECT_FALSE(result);
  closeScope();
}

TEST_F(NapiComparisonTest, InstanceofWithPendingException) {
  openScope();
  napi_value obj, ctor, errVal;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

  auto ctorCb = [](napi_env, napi_callback_info) -> napi_value {
    return nullptr;
  };
  ASSERT_EQ(
      napi_create_function(
          env_, "Ctor", NAPI_AUTO_LENGTH, ctorCb, nullptr, &ctor),
      napi_ok);

  // Throw an exception.
  ASSERT_EQ(
      napi_create_string_utf8(env_, "test error", NAPI_AUTO_LENGTH, &errVal),
      napi_ok);
  ASSERT_EQ(napi_throw(env_, errVal), napi_ok);

  // napi_instanceof should fail with pending exception.
  bool result;
  EXPECT_EQ(napi_instanceof(env_, obj, ctor, &result), napi_pending_exception);

  // Clear the exception.
  bool isPending;
  ASSERT_EQ(napi_is_exception_pending(env_, &isPending), napi_ok);
  EXPECT_TRUE(isPending);
  napi_value exc;
  ASSERT_EQ(napi_get_and_clear_last_exception(env_, &exc), napi_ok);
  closeScope();
}

} // namespace
