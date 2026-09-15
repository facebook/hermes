/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include <cstring>

namespace hermes {
namespace napi {

class NapiDefineClassTest : public NapiTestFixture {
 protected:
  /// Helper: open a handle scope in SetUp, close in TearDown.
  napi_handle_scope scope_ = nullptr;

  void SetUp() override {
    NapiTestFixture::SetUp();
    ASSERT_EQ(napi_open_handle_scope(env_, &scope_), napi_ok);
  }

  void TearDown() override {
    if (scope_) {
      napi_close_handle_scope(env_, scope_);
      scope_ = nullptr;
    }
    NapiTestFixture::TearDown();
  }

  /// Compare two napi_values by raw HermesValue bits. This is a
  /// stand-in for napi_strict_equals which is not yet implemented.
  bool sameValue(napi_value a, napi_value b) {
    auto *phvA = reinterpret_cast<vm::PinnedHermesValue *>(a);
    auto *phvB = reinterpret_cast<vm::PinnedHermesValue *>(b);
    vm::HermesValue hvA, hvB;
    std::memcpy(&hvA, phvA, sizeof(vm::HermesValue));
    std::memcpy(&hvB, phvB, sizeof(vm::HermesValue));
    return hvA.getRaw() == hvB.getRaw();
  }
};

//===========================================================================
// Basic class definition
//===========================================================================

// A simple counter to verify constructor was called.
static int g_constructorCallCount = 0;
static void *g_constructorData = nullptr;

static napi_value TestConstructor(napi_env env, napi_callback_info info) {
  napi_value thisArg;
  void *data;
  EXPECT_EQ(
      napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data), napi_ok);
  g_constructorCallCount++;
  g_constructorData = data;

  // Verify new.target is set (we're being called as constructor).
  napi_value newTarget;
  EXPECT_EQ(napi_get_new_target(env, info, &newTarget), napi_ok);
  EXPECT_NE(newTarget, nullptr);

  return thisArg;
}

TEST_F(NapiDefineClassTest, BasicClassDefinition) {
  g_constructorCallCount = 0;
  g_constructorData = nullptr;

  void *cbData = reinterpret_cast<void *>(0x42);

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "MyClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          cbData,
          0,
          nullptr,
          &cls),
      napi_ok);
  ASSERT_NE(cls, nullptr);

  // Verify the constructor is a function.
  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, cls, &type), napi_ok);
  EXPECT_EQ(type, napi_function);
}

TEST_F(NapiDefineClassTest, NullArgValidation) {
  napi_value cls;

  // Null result.
  EXPECT_EQ(
      napi_define_class(
          env_,
          "MyClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          0,
          nullptr,
          nullptr),
      napi_invalid_arg);

  // Null constructor.
  EXPECT_EQ(
      napi_define_class(
          env_,
          "MyClass",
          NAPI_AUTO_LENGTH,
          nullptr,
          nullptr,
          0,
          nullptr,
          &cls),
      napi_invalid_arg);

  // Null properties when count > 0.
  EXPECT_EQ(
      napi_define_class(
          env_,
          "MyClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          nullptr,
          &cls),
      napi_invalid_arg);
}

//===========================================================================
// Constructor invocation via napi_new_instance
//===========================================================================

TEST_F(NapiDefineClassTest, ConstructorCalledViaNewInstance) {
  g_constructorCallCount = 0;
  g_constructorData = nullptr;

  void *cbData = reinterpret_cast<void *>(0x99);
  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "TestClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          cbData,
          0,
          nullptr,
          &cls),
      napi_ok);

  // Create an instance.
  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  EXPECT_EQ(g_constructorCallCount, 1);
  EXPECT_EQ(g_constructorData, cbData);

  // Instance should be an object.
  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, instance, &type), napi_ok);
  EXPECT_EQ(type, napi_object);
}

//===========================================================================
// Prototype chain
//===========================================================================

TEST_F(NapiDefineClassTest, PrototypeChain) {
  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "ProtoClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          0,
          nullptr,
          &cls),
      napi_ok);

  // Get constructor.prototype
  napi_value proto;
  ASSERT_EQ(napi_get_named_property(env_, cls, "prototype", &proto), napi_ok);

  napi_valuetype protoType;
  ASSERT_EQ(napi_typeof(env_, proto, &protoType), napi_ok);
  EXPECT_EQ(protoType, napi_object);

  // Verify prototype.constructor === cls
  napi_value ctorProp;
  ASSERT_EQ(
      napi_get_named_property(env_, proto, "constructor", &ctorProp), napi_ok);
  EXPECT_TRUE(sameValue(cls, ctorProp));

  // Create an instance and check its prototype.
  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  napi_value instanceProto;
  ASSERT_EQ(napi_get_prototype(env_, instance, &instanceProto), napi_ok);
  EXPECT_TRUE(sameValue(proto, instanceProto));
}

//===========================================================================
// Instance methods
//===========================================================================

static napi_value GetValue42(napi_env env, napi_callback_info info) {
  napi_value result;
  EXPECT_EQ(napi_create_int32(env, 42, &result), napi_ok);
  return result;
}

TEST_F(NapiDefineClassTest, InstanceMethod) {
  napi_property_descriptor props[] = {
      {"getValue",
       nullptr,
       GetValue42,
       nullptr,
       nullptr,
       nullptr,
       napi_default,
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "MethodClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  // Create an instance.
  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  // Call the instance method.
  napi_value method;
  ASSERT_EQ(
      napi_get_named_property(env_, instance, "getValue", &method), napi_ok);

  napi_value result;
  ASSERT_EQ(
      napi_call_function(env_, instance, method, 0, nullptr, &result), napi_ok);

  int32_t val;
  ASSERT_EQ(napi_get_value_int32(env_, result, &val), napi_ok);
  EXPECT_EQ(val, 42);
}

//===========================================================================
// Static methods
//===========================================================================

static napi_value StaticHello(napi_env env, napi_callback_info info) {
  napi_value result;
  EXPECT_EQ(
      napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &result),
      napi_ok);
  return result;
}

TEST_F(NapiDefineClassTest, StaticMethodNotOnInstance) {
  napi_property_descriptor props[] = {
      {"staticFn",
       nullptr,
       StaticHello,
       nullptr,
       nullptr,
       nullptr,
       static_cast<napi_property_attributes>(napi_default | napi_static),
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "StaticOnly",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  // Static method should be on constructor.
  bool hasOnCtor;
  ASSERT_EQ(
      napi_has_named_property(env_, cls, "staticFn", &hasOnCtor), napi_ok);
  EXPECT_TRUE(hasOnCtor);

  // Static method should NOT be an own property of instances.
  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  napi_value key;
  ASSERT_EQ(
      napi_create_string_utf8(env_, "staticFn", NAPI_AUTO_LENGTH, &key),
      napi_ok);

  bool hasOnInstance;
  ASSERT_EQ(
      napi_has_own_property(env_, instance, key, &hasOnInstance), napi_ok);
  EXPECT_FALSE(hasOnInstance);
}

TEST_F(NapiDefineClassTest, StaticMethodCallable) {
  napi_property_descriptor props[] = {
      {"staticFn",
       nullptr,
       StaticHello,
       nullptr,
       nullptr,
       nullptr,
       static_cast<napi_property_attributes>(napi_default | napi_static),
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "StaticCallable",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  // Call the static method on the constructor.
  napi_value method;
  ASSERT_EQ(napi_get_named_property(env_, cls, "staticFn", &method), napi_ok);

  napi_value global;
  ASSERT_EQ(napi_get_global(env_, &global), napi_ok);

  napi_value result;
  ASSERT_EQ(
      napi_call_function(env_, global, method, 0, nullptr, &result), napi_ok);

  // Result should be "hello".
  size_t len;
  ASSERT_EQ(
      napi_get_value_string_utf8(env_, result, nullptr, 0, &len), napi_ok);
  EXPECT_EQ(len, static_cast<size_t>(5));
}

//===========================================================================
// Getter / Setter on prototype
//===========================================================================

static int g_accessorValue = 0;

static napi_value GetAccessorValue(napi_env env, napi_callback_info info) {
  napi_value result;
  EXPECT_EQ(napi_create_int32(env, g_accessorValue, &result), napi_ok);
  return result;
}

static napi_value SetAccessorValue(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  EXPECT_EQ(
      napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr), napi_ok);
  EXPECT_EQ(argc, static_cast<size_t>(1));

  int32_t val;
  EXPECT_EQ(napi_get_value_int32(env, argv[0], &val), napi_ok);
  g_accessorValue = val;

  return nullptr;
}

TEST_F(NapiDefineClassTest, InstanceAccessor) {
  g_accessorValue = 0;

  napi_property_descriptor props[] = {
      {"value",
       nullptr,
       nullptr,
       GetAccessorValue,
       SetAccessorValue,
       nullptr,
       napi_default,
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "AccessorClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  // Set via accessor.
  napi_value setVal;
  ASSERT_EQ(napi_create_int32(env_, 100, &setVal), napi_ok);
  ASSERT_EQ(napi_set_named_property(env_, instance, "value", setVal), napi_ok);
  EXPECT_EQ(g_accessorValue, 100);

  // Get via accessor.
  napi_value getVal;
  ASSERT_EQ(napi_get_named_property(env_, instance, "value", &getVal), napi_ok);

  int32_t result;
  ASSERT_EQ(napi_get_value_int32(env_, getVal, &result), napi_ok);
  EXPECT_EQ(result, 100);
}

//===========================================================================
// Data property on prototype
//===========================================================================

TEST_F(NapiDefineClassTest, InstanceDataProperty) {
  napi_value dataVal;
  ASSERT_EQ(napi_create_int32(env_, 777, &dataVal), napi_ok);

  napi_property_descriptor props[] = {
      {"myData",
       nullptr,
       nullptr,
       nullptr,
       nullptr,
       dataVal,
       static_cast<napi_property_attributes>(
           napi_writable | napi_enumerable | napi_configurable),
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "DataClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  // Access the data property via the prototype chain.
  napi_value result;
  ASSERT_EQ(
      napi_get_named_property(env_, instance, "myData", &result), napi_ok);

  int32_t val;
  ASSERT_EQ(napi_get_value_int32(env_, result, &val), napi_ok);
  EXPECT_EQ(val, 777);
}

//===========================================================================
// Constructor with arguments
//===========================================================================

static napi_value ConstructorWithArgs(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_value thisArg;
  EXPECT_EQ(
      napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr), napi_ok);

  if (argc >= 1) {
    // Store argument as .ctorArg on the instance.
    EXPECT_EQ(
        napi_set_named_property(env, thisArg, "ctorArg", argv[0]), napi_ok);
  }

  return thisArg;
}

TEST_F(NapiDefineClassTest, ConstructorReceivesArguments) {
  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "ArgClass",
          NAPI_AUTO_LENGTH,
          ConstructorWithArgs,
          nullptr,
          0,
          nullptr,
          &cls),
      napi_ok);

  napi_value arg;
  ASSERT_EQ(napi_create_int32(env_, 123, &arg), napi_ok);

  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 1, &arg, &instance), napi_ok);

  // Verify the constructor stored the argument.
  napi_value result;
  ASSERT_EQ(
      napi_get_named_property(env_, instance, "ctorArg", &result), napi_ok);

  int32_t val;
  ASSERT_EQ(napi_get_value_int32(env_, result, &val), napi_ok);
  EXPECT_EQ(val, 123);
}

//===========================================================================
// Mixed instance and static properties
//===========================================================================

TEST_F(NapiDefineClassTest, MixedProperties) {
  napi_value staticVal;
  ASSERT_EQ(napi_create_int32(env_, 999, &staticVal), napi_ok);

  napi_property_descriptor props[] = {
      // Instance method
      {"instanceMethod",
       nullptr,
       GetValue42,
       nullptr,
       nullptr,
       nullptr,
       napi_default,
       nullptr},
      // Static method
      {"staticMethod",
       nullptr,
       StaticHello,
       nullptr,
       nullptr,
       nullptr,
       static_cast<napi_property_attributes>(napi_default | napi_static),
       nullptr},
      // Static data property
      {"STATIC_VAL",
       nullptr,
       nullptr,
       nullptr,
       nullptr,
       staticVal,
       static_cast<napi_property_attributes>(napi_default | napi_static),
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "MixedClass",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          3,
          props,
          &cls),
      napi_ok);

  // Instance method on prototype.
  napi_value proto;
  ASSERT_EQ(napi_get_named_property(env_, cls, "prototype", &proto), napi_ok);

  bool hasProp;
  ASSERT_EQ(
      napi_has_named_property(env_, proto, "instanceMethod", &hasProp),
      napi_ok);
  EXPECT_TRUE(hasProp);

  // Static method on constructor.
  ASSERT_EQ(
      napi_has_named_property(env_, cls, "staticMethod", &hasProp), napi_ok);
  EXPECT_TRUE(hasProp);

  // Static data on constructor.
  ASSERT_EQ(
      napi_has_named_property(env_, cls, "STATIC_VAL", &hasProp), napi_ok);
  EXPECT_TRUE(hasProp);

  napi_value staticResult;
  ASSERT_EQ(
      napi_get_named_property(env_, cls, "STATIC_VAL", &staticResult), napi_ok);

  int32_t staticResultVal;
  ASSERT_EQ(
      napi_get_value_int32(env_, staticResult, &staticResultVal), napi_ok);
  EXPECT_EQ(staticResultVal, 999);
}

//===========================================================================
// Wrapping inside constructor
//===========================================================================

struct TestData {
  int value;
};

static void TestDataFinalize(napi_env env, void *data, void *hint) {
  auto *td = static_cast<TestData *>(data);
  delete td;
}

static napi_value WrappingConstructor(napi_env env, napi_callback_info info) {
  napi_value thisArg;
  EXPECT_EQ(
      napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr),
      napi_ok);

  auto *td = new TestData{42};
  EXPECT_EQ(
      napi_wrap(env, thisArg, td, TestDataFinalize, nullptr, nullptr), napi_ok);

  return thisArg;
}

static napi_value GetWrappedValue(napi_env env, napi_callback_info info) {
  napi_value thisArg;
  EXPECT_EQ(
      napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr),
      napi_ok);

  void *data;
  EXPECT_EQ(napi_unwrap(env, thisArg, &data), napi_ok);

  auto *td = static_cast<TestData *>(data);
  napi_value result;
  EXPECT_EQ(napi_create_int32(env, td->value, &result), napi_ok);
  return result;
}

TEST_F(NapiDefineClassTest, WrapInsideConstructor) {
  napi_property_descriptor props[] = {
      {"getWrappedValue",
       nullptr,
       GetWrappedValue,
       nullptr,
       nullptr,
       nullptr,
       napi_default,
       nullptr},
  };

  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "WrappedClass",
          NAPI_AUTO_LENGTH,
          WrappingConstructor,
          nullptr,
          1,
          props,
          &cls),
      napi_ok);

  napi_value instance;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &instance), napi_ok);

  // Call getWrappedValue() — should return 42.
  napi_value method;
  ASSERT_EQ(
      napi_get_named_property(env_, instance, "getWrappedValue", &method),
      napi_ok);

  napi_value result;
  ASSERT_EQ(
      napi_call_function(env_, instance, method, 0, nullptr, &result), napi_ok);

  int32_t val;
  ASSERT_EQ(napi_get_value_int32(env_, result, &val), napi_ok);
  EXPECT_EQ(val, 42);
}

//===========================================================================
// Class with name via length parameter
//===========================================================================

TEST_F(NapiDefineClassTest, NameWithExplicitLength) {
  napi_value cls;
  // Pass "MyClass_Extra" but length=7 → name should be "MyClass"
  ASSERT_EQ(
      napi_define_class(
          env_, "MyClass_Extra", 7, TestConstructor, nullptr, 0, nullptr, &cls),
      napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, cls, &type), napi_ok);
  EXPECT_EQ(type, napi_function);
}

TEST_F(NapiDefineClassTest, EmptyName) {
  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          0,
          nullptr,
          &cls),
      napi_ok);

  napi_valuetype type;
  ASSERT_EQ(napi_typeof(env_, cls, &type), napi_ok);
  EXPECT_EQ(type, napi_function);
}

TEST_F(NapiDefineClassTest, NullName) {
  // Node.js rejects NULL name with napi_invalid_arg.
  napi_value cls;
  EXPECT_EQ(
      napi_define_class(
          env_, nullptr, 0, TestConstructor, nullptr, 0, nullptr, &cls),
      napi_invalid_arg);
}

//===========================================================================
// Multiple instances share the same prototype
//===========================================================================

TEST_F(NapiDefineClassTest, MultipleInstancesSharePrototype) {
  napi_value cls;
  ASSERT_EQ(
      napi_define_class(
          env_,
          "SharedProto",
          NAPI_AUTO_LENGTH,
          TestConstructor,
          nullptr,
          0,
          nullptr,
          &cls),
      napi_ok);

  napi_value inst1, inst2;
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &inst1), napi_ok);
  ASSERT_EQ(napi_new_instance(env_, cls, 0, nullptr, &inst2), napi_ok);

  napi_value proto1, proto2;
  ASSERT_EQ(napi_get_prototype(env_, inst1, &proto1), napi_ok);
  ASSERT_EQ(napi_get_prototype(env_, inst2, &proto2), napi_ok);

  EXPECT_TRUE(sameValue(proto1, proto2));
}

} // namespace napi
} // namespace hermes
