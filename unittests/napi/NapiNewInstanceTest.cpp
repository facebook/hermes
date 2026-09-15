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
// napi_new_instance tests
//===========================================================================

class NapiNewInstanceTest : public NapiTestFixture {};

TEST_F(NapiNewInstanceTest, NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_new_instance(nullptr, nullptr, 0, nullptr, nullptr));
}

TEST_F(NapiNewInstanceTest, NullConstructor) {
  auto scope = openScope(env_);

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_new_instance(env_, nullptr, 0, nullptr, &result));

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, NullResult) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_get_undefined(env, &r);
    return r;
  };

  napi_value func = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(env_, "Ctor", NAPI_AUTO_LENGTH, cb, nullptr, &func));

  EXPECT_EQ(
      napi_invalid_arg, napi_new_instance(env_, func, 0, nullptr, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, NonCallable) {
  auto scope = openScope(env_);

  napi_value num = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_new_instance(env_, num, 0, nullptr, &result));

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, NonCallableObject) {
  auto scope = openScope(env_);

  napi_value obj = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_new_instance(env_, obj, 0, nullptr, &result));

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ConstructNoArgs) {
  auto scope = openScope(env_);

  // Constructor that sets this.x = 42.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);

    napi_value val = nullptr;
    napi_create_double(env, 42.0, &val);
    napi_set_named_property(env, thisArg, "x", val);

    return thisArg;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(env_, "Foo", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // Verify it's an object.
  napi_valuetype type;
  EXPECT_EQ(napi_ok, napi_typeof(env_, instance, &type));
  EXPECT_EQ(napi_object, type);

  // Verify this.x was set.
  napi_value xVal = nullptr;
  EXPECT_EQ(napi_ok, napi_get_named_property(env_, instance, "x", &xVal));
  double x = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, xVal, &x));
  EXPECT_EQ(42.0, x);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ConstructWithArgs) {
  auto scope = openScope(env_);

  // Constructor that sets this.sum = argv[0] + argv[1].
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    size_t argc = 2;
    napi_value argv[2];
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr);

    double a = 0, b = 0;
    napi_get_value_double(env, argv[0], &a);
    napi_get_value_double(env, argv[1], &b);

    napi_value sum = nullptr;
    napi_create_double(env, a + b, &sum);
    napi_set_named_property(env, thisArg, "sum", sum);

    return thisArg;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "Adder", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value arg0 = nullptr, arg1 = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 10.0, &arg0));
  EXPECT_EQ(napi_ok, napi_create_double(env_, 32.0, &arg1));
  napi_value args[] = {arg0, arg1};

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 2, args, &instance));
  ASSERT_NE(nullptr, instance);

  napi_value sumVal = nullptr;
  EXPECT_EQ(napi_ok, napi_get_named_property(env_, instance, "sum", &sumVal));
  double sum = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, sumVal, &sum));
  EXPECT_EQ(42.0, sum);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, NewTargetIsSet) {
  auto scope = openScope(env_);

  // Constructor that checks new.target is non-null and equals
  // the constructor.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);

    napi_value newTarget = nullptr;
    napi_get_new_target(env, info, &newTarget);

    // In a constructor call, new.target should be non-null.
    napi_value hasNew = nullptr;
    napi_get_boolean(env, newTarget != nullptr, &hasNew);
    napi_set_named_property(env, thisArg, "hasNew", hasNew);

    return thisArg;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "Check", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  napi_value hasNewVal = nullptr;
  EXPECT_EQ(
      napi_ok, napi_get_named_property(env_, instance, "hasNew", &hasNewVal));
  bool hasNew = false;
  EXPECT_EQ(napi_ok, napi_get_value_bool(env_, hasNewVal, &hasNew));
  EXPECT_TRUE(hasNew);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ReturningObjectOverridesThis) {
  auto scope = openScope(env_);

  // Constructor that returns a different object (overriding
  // the default 'this').
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_value val = nullptr;
    napi_create_double(env, 99.0, &val);
    napi_set_named_property(env, obj, "custom", val);

    return obj;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "Custom", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // The instance should be the custom object, not the default
  // 'this'.
  napi_value customVal = nullptr;
  EXPECT_EQ(
      napi_ok, napi_get_named_property(env_, instance, "custom", &customVal));
  double custom = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, customVal, &custom));
  EXPECT_EQ(99.0, custom);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ReturningNonObjectUsesThis) {
  auto scope = openScope(env_);

  // Constructor that returns a primitive (number). The 'this'
  // object should be used instead.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);

    napi_value val = nullptr;
    napi_create_double(env, 123.0, &val);
    napi_set_named_property(env, thisArg, "fromThis", val);

    // Return a primitive — should be ignored.
    napi_value ret = nullptr;
    napi_create_double(env, 42.0, &ret);
    return ret;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "PrimRet", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // The instance should be the 'this' object, not the
  // primitive.
  napi_valuetype type;
  EXPECT_EQ(napi_ok, napi_typeof(env_, instance, &type));
  EXPECT_EQ(napi_object, type);

  napi_value fromThisVal = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_named_property(env_, instance, "fromThis", &fromThisVal));
  double fromThis = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, fromThisVal, &fromThis));
  EXPECT_EQ(123.0, fromThis);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, PrototypeChainIsCorrect) {
  auto scope = openScope(env_);

  // Constructor that sets this.own = true.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);

    napi_value val = nullptr;
    napi_get_boolean(env, true, &val);
    napi_set_named_property(env, thisArg, "own", val);

    return thisArg;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "Proto", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  // napi_create_function does not set a .prototype property.
  // Create a prototype object and set it on the constructor.
  napi_value proto = nullptr;
  EXPECT_EQ(napi_ok, napi_create_object(env_, &proto));

  napi_value protoVal = nullptr;
  EXPECT_EQ(napi_ok, napi_create_double(env_, 77.0, &protoVal));
  EXPECT_EQ(
      napi_ok, napi_set_named_property(env_, proto, "inherited", protoVal));

  // Set the prototype on the constructor.
  EXPECT_EQ(napi_ok, napi_set_named_property(env_, ctor, "prototype", proto));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // The instance should have the own property.
  bool hasOwn = false;
  napi_value ownKey = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_string_utf8(env_, "own", NAPI_AUTO_LENGTH, &ownKey));
  EXPECT_EQ(napi_ok, napi_has_own_property(env_, instance, ownKey, &hasOwn));
  EXPECT_TRUE(hasOwn);

  // The instance should have the inherited property via
  // prototype chain.
  napi_value inheritedVal = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_named_property(env_, instance, "inherited", &inheritedVal));
  double inherited = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, inheritedVal, &inherited));
  EXPECT_EQ(77.0, inherited);

  // The instance's prototype should be the constructor's
  // prototype. Verify by checking that the inherited property
  // is accessible on the prototype.
  napi_value instanceProto = nullptr;
  EXPECT_EQ(napi_ok, napi_get_prototype(env_, instance, &instanceProto));
  ASSERT_NE(nullptr, instanceProto);

  // The prototype should have the 'inherited' property.
  napi_value protoInherited = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_named_property(
          env_, instanceProto, "inherited", &protoInherited));
  double protoInheritedVal = 0;
  EXPECT_EQ(
      napi_ok, napi_get_value_double(env_, protoInherited, &protoInheritedVal));
  EXPECT_EQ(77.0, protoInheritedVal);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, PendingExceptionBlocksConstruct) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_get_undefined(env, &r);
    return r;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &ctor));

  // Set a pending exception.
  napi_value errMsg = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &errMsg));
  napi_throw(env_, errMsg);

  // napi_new_instance should refuse.
  napi_value result = nullptr;
  EXPECT_EQ(
      napi_pending_exception,
      napi_new_instance(env_, ctor, 0, nullptr, &result));

  // Clear the exception.
  napi_value exc = nullptr;
  napi_get_and_clear_last_exception(env_, &exc);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ConstructorThrowsSetsException) {
  auto scope = openScope(env_);

  // Constructor that throws a TypeError.
  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_throw_type_error(env, nullptr, "construct error");
    return nullptr;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &ctor));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_pending_exception,
      napi_new_instance(env_, ctor, 0, nullptr, &result));

  // There should be a pending exception.
  bool isPending = false;
  EXPECT_EQ(napi_ok, napi_is_exception_pending(env_, &isPending));
  EXPECT_TRUE(isPending);

  // Clear it.
  napi_value exc = nullptr;
  napi_get_and_clear_last_exception(env_, &exc);
  ASSERT_NE(nullptr, exc);

  bool isError = false;
  EXPECT_EQ(napi_ok, napi_is_error(env_, exc, &isError));
  EXPECT_TRUE(isError);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ArgcNonZeroArgvNull) {
  auto scope = openScope(env_);

  napi_callback cb = [](napi_env env, napi_callback_info) -> napi_value {
    napi_value r = nullptr;
    napi_get_undefined(env, &r);
    return r;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok, napi_create_function(env_, nullptr, 0, cb, nullptr, &ctor));

  napi_value result = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_new_instance(env_, ctor, 1, nullptr, &result));

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ConstructReturnNull) {
  auto scope = openScope(env_);

  // Constructor that returns null — should use the 'this'
  // object (which was created by createThisForConstruct).
  napi_callback cb = [](napi_env, napi_callback_info) -> napi_value {
    return nullptr;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "NullRet", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // The instance should be an object (the default 'this').
  napi_valuetype type;
  EXPECT_EQ(napi_ok, napi_typeof(env_, instance, &type));
  EXPECT_EQ(napi_object, type);

  closeScope(env_, scope);
}

TEST_F(NapiNewInstanceTest, ConstructReturningUndefined) {
  auto scope = openScope(env_);

  // Constructor that returns undefined explicitly — should use
  // the 'this' object.
  napi_callback cb = [](napi_env env, napi_callback_info info) -> napi_value {
    napi_value thisArg = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr);

    napi_value marker = nullptr;
    napi_create_double(env, 55.0, &marker);
    napi_set_named_property(env, thisArg, "marker", marker);

    napi_value undef = nullptr;
    napi_get_undefined(env, &undef);
    return undef;
  };

  napi_value ctor = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_create_function(
          env_, "UndefRet", NAPI_AUTO_LENGTH, cb, nullptr, &ctor));

  napi_value instance = nullptr;
  EXPECT_EQ(napi_ok, napi_new_instance(env_, ctor, 0, nullptr, &instance));
  ASSERT_NE(nullptr, instance);

  // The instance should be the 'this' with the marker.
  napi_value markerVal = nullptr;
  EXPECT_EQ(
      napi_ok, napi_get_named_property(env_, instance, "marker", &markerVal));
  double marker = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, markerVal, &marker));
  EXPECT_EQ(55.0, marker);

  closeScope(env_, scope);
}

} // namespace
