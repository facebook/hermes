/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include "hermes/VM/HandleRootOwner.h"
#include "hermes/VM/HermesValue.h"

namespace {

using hermes::napi::NapiTestFixture;
using namespace hermes::vm;

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
// napi_call_function tests
//===========================================================================

class NapiCallFunctionTest : public NapiTestFixture {};

TEST_F(NapiCallFunctionTest, NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_call_function(nullptr, nullptr, nullptr, 0, nullptr, nullptr));
}

TEST_F(NapiCallFunctionTest, NullRecv) {
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
      napi_call_function(env_, nullptr, func, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, NullFunc) {
  auto scope = openScope(env_);

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_call_function(env_, recv, nullptr, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, NonCallableFunc) {
  auto scope = openScope(env_);

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Pass a number as the function — should fail.
  napi_value num = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_call_function(env_, recv, num, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, NonCallableObject) {
  auto scope = openScope(env_);

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Pass a plain object as the function — should fail.
  napi_value obj = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &obj));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_call_function(env_, recv, obj, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallNoArgs) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_create_double(env, 42.0, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 0, nullptr, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(42.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallWithArgs) {
  auto scope = openScope(env_);

  // Callback that adds two numbers.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    double a = 0, b = 0;
    napi_get_value_double(env, argv[0], &a);
    napi_get_value_double(env, argv[1], &b);

    napi_value r = nullptr;
    napi_create_double(env, a + b, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(env_, "add", NAPI_AUTO_LENGTH, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value arg0 = nullptr, arg1 = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 10.0, &arg0));
  EXPECT_EQ(napi_ok, napi_create_double(env_, 32.0, &arg1));

  napi_value args[] = {arg0, arg1};
  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 2, args, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(42.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallReceivesThisArg) {
  auto scope = openScope(env_);

  // Callback that returns 'this'.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value this_arg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    return this_arg;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  // Create an object to use as 'this'.
  napi_value thisObj = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &thisObj));

  // Set a property on it so we can identify it.
  napi_value marker = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 99.0, &marker));
  EXPECT_EQ(napi_ok, napi_set_named_property(env_, thisObj, "marker", marker));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok, napi_call_function(env_, thisObj, func, 0, nullptr, &result));
  ASSERT_NE(nullptr, result);

  // The returned 'this' should have the 'marker' property.
  napi_value got = nullptr;
  EXPECT_EQ(napi_ok, napi_get_named_property(env_, result, "marker", &got));

  double markerVal = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, got, &markerVal));
  EXPECT_EQ(99.0, markerVal);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, ResultIsOptional) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_create_double(env, 42.0, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Pass nullptr for result — should succeed.
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, PendingExceptionBlocksCall) {
  auto scope = openScope(env_);

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

  // Set a pending exception.
  napi_value errMsg = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &errMsg));
  napi_throw(env_, errMsg);

  // napi_call_function should refuse.
  EXPECT_EQ(
      napi_pending_exception,
      napi_call_function(env_, recv, func, 0, nullptr, nullptr));

  // Clear the exception.
  napi_value exc = nullptr;
  napi_get_and_clear_last_exception(env_, &exc);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CalleeThrowsSetsException) {
  auto scope = openScope(env_);

  // Callback that throws a TypeError.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_throw_type_error(env, nullptr, "test error");
    return nullptr;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_pending_exception,
      napi_call_function(env_, recv, func, 0, nullptr, &result));

  // There should be a pending exception.
  bool isPending = false;
  EXPECT_EQ(napi_ok, napi_is_exception_pending(env_, &isPending));
  EXPECT_TRUE(isPending);

  // Clear it.
  napi_value exc = nullptr;
  napi_get_and_clear_last_exception(env_, &exc);
  ASSERT_NE(nullptr, exc);

  // The exception should be an error.
  bool isError = false;
  EXPECT_EQ(napi_ok, napi_is_error(env_, exc, &isError));
  EXPECT_TRUE(isError);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallReturnUndefinedWhenNull) {
  auto scope = openScope(env_);

  // Callback that returns null (meaning "return undefined").
  napi_callback cb = [](napi_env, napi_callback_info) -> napi_value {
    return nullptr;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 0, nullptr, &result));
  ASSERT_NE(nullptr, result);

  napi_valuetype type;
  EXPECT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_undefined, type);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallWithManyArgs) {
  auto scope = openScope(env_);

  // Callback that sums all arguments.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    size_t argc = 10;
    napi_value argv[10];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    double sum = 0;
    for (size_t i = 0; i < argc; ++i) {
      double v = 0;
      napi_get_value_double(env, argv[i], &v);
      sum += v;
    }

    napi_value r = nullptr;
    napi_create_double(env, sum, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(env_, "sum", NAPI_AUTO_LENGTH, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // Create 10 arguments: 1, 2, ..., 10
  napi_value args[10];
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(
        napi_ok,
        napi_create_double(env_, static_cast<double>(i + 1), &args[i]));
  }

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 10, args, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(55.0, val); // 1+2+...+10 = 55

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, ArgcNonZeroArgvNull) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env, napi_callback_info) -> napi_value {
    return nullptr;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // argc > 0 but argv is null — should fail.
  EXPECT_EQ(
      napi_invalid_arg,
      napi_call_function(env_, recv, func, 1, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallFunctionOnObject) {
  auto scope = openScope(env_);

  // Create an object with a "getValue" method that returns this.x.
  napi_value obj = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_value xVal = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 77.0, &xVal));
  EXPECT_EQ(napi_ok, napi_set_named_property(env_, obj, "x", xVal));

  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value this_arg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);

    napi_value x = nullptr;
    napi_get_named_property(env, this_arg, "x", &x);
    return x;
  };

  napi_value method = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "getValue", NAPI_AUTO_LENGTH, cb, nullptr, &method));
  EXPECT_EQ(napi_ok, napi_set_named_property(env_, obj, "getValue", method));

  // Get the method and call it with obj as 'this'.
  napi_value gotMethod = nullptr;
  EXPECT_EQ(
      napi_ok, napi_get_named_property(env_, obj, "getValue", &gotMethod));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok, napi_call_function(env_, obj, gotMethod, 0, nullptr, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(77.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallStringFunc) {
  auto scope = openScope(env_);

  // Callback that returns a string.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_call_function(env_, recv, func, 0, nullptr, &result));
  ASSERT_NE(nullptr, result);

  char buf[32] = {};
  size_t len = 0;
  EXPECT_EQ(
      napi_ok,
      napi_get_value_string_utf8(env_, result, buf, sizeof(buf), &len));
  EXPECT_EQ(std::string("hello"), std::string(buf, len));

  closeScope(env_, scope);
}

TEST_F(NapiCallFunctionTest, CallWithZeroArgcAndNonNullArgv) {
  auto scope = openScope(env_);

  // A callback that returns the argument count.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

    napi_value r = nullptr;
    napi_create_double(env, static_cast<double>(argc), &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &func));

  napi_value recv = nullptr;
  EXPECT_EQ(napi_ok, napi_get_undefined(env_, &recv));

  // argc = 0, argv is non-null — should be fine, argv is ignored.
  napi_value dummyArgv[1] = {nullptr};
  napi_value result = nullptr;
  EXPECT_EQ(
      napi_ok, napi_call_function(env_, recv, func, 0, dummyArgv, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(0.0, val);

  closeScope(env_, scope);
}

} // namespace
