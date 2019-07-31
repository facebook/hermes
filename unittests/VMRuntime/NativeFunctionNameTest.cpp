/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSNativeFunctions.h"

#include "hermes/VM/JSError.h"
#include "hermes/VM/JSTypedArray.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(NativeFunctionNameTest, SmokeTest) {
  ASSERT_EQ("print", getFunctionName(print));
  ASSERT_EQ(
      "dataViewPrototypeGet<int8_t>",
      getFunctionName(dataViewPrototypeGet<int8_t>));
  ASSERT_EQ(
      "dataViewPrototypeSet<int8_t, CellKind::Int8ArrayKind>",
      getFunctionName(dataViewPrototypeSet<int8_t, CellKind::Int8ArrayKind>));

  using CreatorFunction = CallResult<HermesValue>(Runtime *, Handle<JSObject>);
  CreatorFunction *func;
  func = JSError::create;
  ASSERT_EQ("JSError::create", getFunctionName(func));

  func = JSTypedArray<int16_t, CellKind::Int16ArrayKind>::create;
  ASSERT_EQ(
      "JSTypedArray<int16_t, CellKind::Int16ArrayKind>::create",
      getFunctionName(func));
}
} // namespace
