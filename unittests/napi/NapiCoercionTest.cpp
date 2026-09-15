/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {
using namespace hermes::napi;

class NapiCoercionTest : public NapiTestFixture {
 protected:
  /// Open a handle scope. Must be called in each test that uses
  /// napi_value handles.
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
// napi_coerce_to_bool
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToBoolFromUndefined) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, undefined, &result), napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, result, &type), napi_ok);
  EXPECT_EQ(type, napi_boolean);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_FALSE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromNull) {
  openScope();
  napi_value null;
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, null, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_FALSE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromTrue) {
  openScope();
  napi_value trueVal;
  ASSERT_EQ(napi_get_boolean(env_, true, &trueVal), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, trueVal, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_TRUE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromZero) {
  openScope();
  napi_value zero;
  ASSERT_EQ(napi_create_double(env_, 0.0, &zero), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, zero, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_FALSE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromNumber) {
  openScope();
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, num, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_TRUE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromEmptyString) {
  openScope();
  napi_value str;
  ASSERT_EQ(napi_create_string_utf8(env_, "", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, str, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_FALSE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromNonEmptyString) {
  openScope();
  napi_value str;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, str, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_TRUE(boolVal);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolFromObject) {
  openScope();
  napi_value obj;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_bool(env_, obj, &result), napi_ok);

  bool boolVal;
  ASSERT_EQ(napi_get_value_bool(env_, result, &boolVal), napi_ok);
  EXPECT_TRUE(boolVal);
  closeScope();
}

//===========================================================================
// napi_coerce_to_number
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToNumberFromUndefined) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, undefined, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_TRUE(std::isnan(dval));
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromNull) {
  openScope();
  napi_value null;
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, null, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_EQ(dval, 0.0);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromTrue) {
  openScope();
  napi_value trueVal;
  ASSERT_EQ(napi_get_boolean(env_, true, &trueVal), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, trueVal, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_EQ(dval, 1.0);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromFalse) {
  openScope();
  napi_value falseVal;
  ASSERT_EQ(napi_get_boolean(env_, false, &falseVal), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, falseVal, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_EQ(dval, 0.0);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromNumericString) {
  openScope();
  napi_value str;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "42", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, str, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_EQ(dval, 42.0);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromNonNumericString) {
  openScope();
  napi_value str;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, str, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_TRUE(std::isnan(dval));
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberFromNumber) {
  openScope();
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 3.14, &num), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_number(env_, num, &result), napi_ok);

  double dval;
  ASSERT_EQ(napi_get_value_double(env_, result, &dval), napi_ok);
  EXPECT_EQ(dval, 3.14);
  closeScope();
}

//===========================================================================
// napi_coerce_to_object
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToObjectFromNumber) {
  openScope();
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_object(env_, num, &result), napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, result, &type), napi_ok);
  EXPECT_EQ(type, napi_object);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToObjectFromString) {
  openScope();
  napi_value str;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_object(env_, str, &result), napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, result, &type), napi_ok);
  EXPECT_EQ(type, napi_object);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToObjectFromObject) {
  openScope();
  napi_value obj;
  ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_object(env_, obj, &result), napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, result, &type), napi_ok);
  EXPECT_EQ(type, napi_object);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToObjectFromUndefinedFails) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);

  napi_value result;
  // ToObject(undefined) throws TypeError.
  EXPECT_EQ(
      napi_coerce_to_object(env_, undefined, &result), napi_pending_exception);

  // Clear the pending exception.
  bool isExc;
  ASSERT_EQ(napi_is_exception_pending(env_, &isExc), napi_ok);
  EXPECT_TRUE(isExc);

  napi_value exc;
  ASSERT_EQ(napi_get_and_clear_last_exception(env_, &exc), napi_ok);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToObjectFromNullFails) {
  openScope();
  napi_value null;
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);

  napi_value result;
  // ToObject(null) throws TypeError.
  EXPECT_EQ(napi_coerce_to_object(env_, null, &result), napi_pending_exception);

  // Clear the pending exception.
  napi_value exc;
  ASSERT_EQ(napi_get_and_clear_last_exception(env_, &exc), napi_ok);
  closeScope();
}

//===========================================================================
// napi_coerce_to_string
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToStringFromUndefined) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_string(env_, undefined, &result), napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, result, &type), napi_ok);
  EXPECT_EQ(type, napi_string);

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len),
      napi_ok);
  EXPECT_STREQ(buf, "undefined");
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToStringFromNull) {
  openScope();
  napi_value null;
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_string(env_, null, &result), napi_ok);

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len),
      napi_ok);
  EXPECT_STREQ(buf, "null");
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToStringFromTrue) {
  openScope();
  napi_value trueVal;
  ASSERT_EQ(napi_get_boolean(env_, true, &trueVal), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_string(env_, trueVal, &result), napi_ok);

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len),
      napi_ok);
  EXPECT_STREQ(buf, "true");
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToStringFromNumber) {
  openScope();
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_string(env_, num, &result), napi_ok);

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len),
      napi_ok);
  EXPECT_STREQ(buf, "42");
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToStringFromString) {
  openScope();
  napi_value str;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &str), napi_ok);

  napi_value result;
  ASSERT_EQ(napi_coerce_to_string(env_, str, &result), napi_ok);

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len),
      napi_ok);
  EXPECT_STREQ(buf, "hello");
  closeScope();
}

//===========================================================================
// Null argument checks
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToBoolNullEnv) {
  EXPECT_EQ(napi_coerce_to_bool(nullptr, nullptr, nullptr), napi_invalid_arg);
}

TEST_F(NapiCoercionTest, CoerceToBoolNullValue) {
  openScope();
  napi_value result;
  EXPECT_EQ(napi_coerce_to_bool(env_, nullptr, &result), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToBoolNullResult) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);
  EXPECT_EQ(napi_coerce_to_bool(env_, undefined, nullptr), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToNumberNullResult) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);
  EXPECT_EQ(napi_coerce_to_number(env_, undefined, nullptr), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToObjectNullResult) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);
  EXPECT_EQ(napi_coerce_to_object(env_, undefined, nullptr), napi_invalid_arg);
  closeScope();
}

TEST_F(NapiCoercionTest, CoerceToStringNullResult) {
  openScope();
  napi_value undefined;
  ASSERT_EQ(napi_get_undefined(env_, &undefined), napi_ok);
  EXPECT_EQ(napi_coerce_to_string(env_, undefined, nullptr), napi_invalid_arg);
  closeScope();
}

//===========================================================================
// Pending exception checks
//===========================================================================

TEST_F(NapiCoercionTest, CoerceToNumberWithPendingException) {
  openScope();
  // Set a pending exception by coercing null to object (throws TypeError).
  napi_value null;
  ASSERT_EQ(napi_get_null(env_, &null), napi_ok);
  napi_value result;
  EXPECT_EQ(napi_coerce_to_object(env_, null, &result), napi_pending_exception);

  // Now try to coerce — should fail because exception is pending.
  napi_value num;
  ASSERT_EQ(napi_create_double(env_, 42.0, &num), napi_ok);
  EXPECT_EQ(napi_coerce_to_number(env_, num, &result), napi_pending_exception);

  // Clear exception.
  napi_value exc;
  ASSERT_EQ(napi_get_and_clear_last_exception(env_, &exc), napi_ok);
  closeScope();
}

} // anonymous namespace
