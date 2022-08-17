/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AdditionalSlots.h"
#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/JSTypedArray.h"

using namespace hermes::vm;

namespace {

using NativeFunctionTest = RuntimeTestFixture;

TEST_F(NativeFunctionTest, AdditionalSlots) {
  GCScope scope{runtime, "NativeFunctionTest"};
  auto handle = NativeFunction::createWithoutPrototype(
      runtime,
      nullptr,
      nullptr,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      numAdditionalSlotsForTest<NativeFunction>());
  testAdditionalSlots(runtime, handle);
}

TEST(NativeFunctionNameTest, SmokeTest) {
  EXPECT_STREQ("print", getFunctionName(print));
  EXPECT_STREQ(
      "dataViewPrototypeGetInt8", getFunctionName(dataViewPrototypeGetInt8));
  EXPECT_STREQ(
      "dataViewPrototypeSetInt8", getFunctionName(dataViewPrototypeSetInt8));

  using CreatorFunction = NativeConstructor::CreatorFunction;
  CreatorFunction *func = NativeConstructor::creatorFunction<JSError>;
  EXPECT_STREQ(
      "NativeConstructor::creatorFunction<JSError>", getFunctionName(func));

  func = NativeConstructor::creatorFunction<
      JSTypedArray<int16_t, CellKind::Int16ArrayKind>>;
  EXPECT_STREQ(
      "NativeConstructor::creatorFunction<Int16Array>", getFunctionName(func));
}
} // namespace
