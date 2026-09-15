/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace hermes {
namespace napi {

class NapiRunScriptTest : public NapiTestFixture {};

TEST_F(NapiRunScriptTest, SimpleExpression) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  // Create the script string.
  napi_value script;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "1 + 2", NAPI_AUTO_LENGTH, &script));

  // Run the script.
  napi_value result;
  ASSERT_EQ(napi_ok, napi_run_script(env_, script, &result));

  // Verify the result is 3.
  double val;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(3.0, val);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, ObjectExpression) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  napi_value script;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(
          env_, "({a: 42, b: 'hello'})", NAPI_AUTO_LENGTH, &script));

  napi_value result;
  ASSERT_EQ(napi_ok, napi_run_script(env_, script, &result));

  // Verify it's an object.
  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_object, type);

  // Verify property 'a' is 42.
  napi_value aProp;
  ASSERT_EQ(napi_ok, napi_get_named_property(env_, result, "a", &aProp));
  double aVal;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, aProp, &aVal));
  EXPECT_EQ(42.0, aVal);

  // Verify property 'b' is 'hello'.
  napi_value bProp;
  ASSERT_EQ(napi_ok, napi_get_named_property(env_, result, "b", &bProp));
  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_ok, napi_get_value_string_utf8(env_, bProp, buf, sizeof(buf), &len));
  EXPECT_STREQ("hello", buf);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, StringResult) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  napi_value script;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(
          env_, "'foo' + 'bar'", NAPI_AUTO_LENGTH, &script));

  napi_value result;
  ASSERT_EQ(napi_ok, napi_run_script(env_, script, &result));

  char buf[32];
  size_t len;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len));
  EXPECT_STREQ("foobar", buf);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, NonStringScript) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  // Pass a number instead of a string.
  napi_value notAString;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &notAString));

  napi_value result;
  EXPECT_EQ(napi_string_expected, napi_run_script(env_, notAString, &result));

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, SyntaxError) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  napi_value script;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "function(", NAPI_AUTO_LENGTH, &script));

  napi_value result;
  EXPECT_EQ(napi_pending_exception, napi_run_script(env_, script, &result));

  // Verify there's a pending exception.
  bool hasPending;
  ASSERT_EQ(napi_ok, napi_is_exception_pending(env_, &hasPending));
  EXPECT_TRUE(hasPending);

  // Clear the exception so TearDown doesn't fail.
  napi_value exception;
  napi_get_and_clear_last_exception(env_, &exception);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, RuntimeException) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  napi_value script;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(
          env_, "throw new Error('test error')", NAPI_AUTO_LENGTH, &script));

  napi_value result;
  EXPECT_EQ(napi_pending_exception, napi_run_script(env_, script, &result));

  bool hasPending;
  ASSERT_EQ(napi_ok, napi_is_exception_pending(env_, &hasPending));
  EXPECT_TRUE(hasPending);

  // Clear the exception.
  napi_value exception;
  napi_get_and_clear_last_exception(env_, &exception);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, GlobalScopeAccess) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  // Run a script that sets a global variable.
  napi_value script1;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(
          env_, "var testGlobal = 99", NAPI_AUTO_LENGTH, &script1));
  napi_value result1;
  ASSERT_EQ(napi_ok, napi_run_script(env_, script1, &result1));

  // Run another script that reads the global variable.
  napi_value script2;
  ASSERT_EQ(
      napi_ok,
      napi_create_string_utf8(
          env_, "testGlobal + 1", NAPI_AUTO_LENGTH, &script2));
  napi_value result2;
  ASSERT_EQ(napi_ok, napi_run_script(env_, script2, &result2));

  double val;
  ASSERT_EQ(napi_ok, napi_get_value_double(env_, result2, &val));
  EXPECT_EQ(100.0, val);

  napi_close_handle_scope(env_, scope);
}

TEST_F(NapiRunScriptTest, NullArgs) {
  napi_handle_scope scope;
  ASSERT_EQ(napi_ok, napi_open_handle_scope(env_, &scope));

  napi_value script;
  ASSERT_EQ(
      napi_ok, napi_create_string_utf8(env_, "1", NAPI_AUTO_LENGTH, &script));

  // Null result.
  EXPECT_EQ(napi_invalid_arg, napi_run_script(env_, script, nullptr));

  // Null script.
  napi_value result;
  EXPECT_EQ(napi_invalid_arg, napi_run_script(env_, nullptr, &result));

  // Null env.
  EXPECT_EQ(napi_invalid_arg, napi_run_script(nullptr, script, &result));

  napi_close_handle_scope(env_, scope);
}

} // namespace napi
} // namespace hermes
