/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/VM/JSDate.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/MockedEnvironment.h"

using namespace hermes::vm;
using namespace hermes::hbc;

namespace {

using InternalPropertiesTest = RuntimeTestFixture;

#define FORCE_DICTIONARY_MODE(obj)                                       \
  do {                                                                   \
    EXPECT_CALLRESULT_BOOL_RAW(                                          \
        TRUE,                                                            \
        JSObject::putNamed_RJS(                                          \
            obj,                                                         \
            runtime,                                                     \
            Predefined::getSymbolID(Predefined::Object),                 \
            runtime.makeHandle(HermesValue::encodeUndefinedValue())));   \
    EXPECT_CALLRESULT_BOOL_RAW(                                          \
        TRUE,                                                            \
        JSObject::deleteNamed(                                           \
            obj, runtime, Predefined::getSymbolID(Predefined::Object))); \
  } while (0)

TEST_F(InternalPropertiesTest, NamedInternalPropertyTest) {
  CallResult<PseudoHandle<>> propRes{ExecutionStatus::EXCEPTION};

  Handle<JSObject> nullObj(runtime, nullptr);
  auto obj = runtime.makeHandle(JSObject::create(runtime, nullObj));

  // A key feature of named internal properties is that they may be added to
  // any object at any point. Let's put the object in dictionary mode to test
  // this.
  FORCE_DICTIONARY_MODE(obj);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();

  auto propID = Predefined::getSymbolID(
      Predefined::InternalPropertyNamedPropForUnitTestOnly);

  EXPECT_CALLRESULT_BOOL_RAW(
      TRUE,
      JSObject::defineOwnProperty(
          obj,
          runtime,
          propID,
          dpf,
          runtime.makeHandle(HermesValue::encodeDoubleValue(10.0))));

  EXPECT_CALLRESULT_DOUBLE(10.0, JSObject::getNamed_RJS(obj, runtime, propID));

  // Internal properties are not exposed to the user in any way.
  auto propNames =
      JSObject::getOwnPropertyNames(obj, runtime, false /* onlyEnumerable */);
  ASSERT_RETURNED(propNames);
  ASSERT_EQ(JSArray::getLength(**propNames, runtime), 0);

  auto propSymbols = JSObject::getOwnPropertySymbols(obj, runtime);
  ASSERT_RETURNED(propSymbols);
  ASSERT_EQ(JSArray::getLength(**propSymbols, runtime), 0);
}

} // anonymous namespace
