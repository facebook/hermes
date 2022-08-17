/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSTypedArray.h"

#include "TestHelpers.h"

using namespace hermes::vm;

namespace {

using JSTypedArrayTest = RuntimeTestFixture;

TEST_F(JSTypedArrayTest, BeginAndEndMatchesOnBaseAndSub) {
  // Make sure that the begin and end pointers match for both JSTypedArrayBase,
  // and JSTypedArray<T>. Do this for each T.

#define TYPED_ARRAY(name, type)                                        \
  {                                                                    \
    auto result = name##Array::allocate(runtime, 10);                  \
    ASSERT_FALSE(isException(result));                                 \
    Handle<name##Array> array = Handle<name##Array>::vmcast(*result);  \
    EXPECT_EQ(                                                         \
        reinterpret_cast<uintptr_t>(array->begin(runtime)),            \
        reinterpret_cast<uintptr_t>(                                   \
            Handle<JSTypedArrayBase>::vmcast(array)->begin(runtime))); \
    EXPECT_EQ(                                                         \
        reinterpret_cast<uintptr_t>(array->end(runtime)),              \
        reinterpret_cast<uintptr_t>(                                   \
            Handle<JSTypedArrayBase>::vmcast(array)->end(runtime)));   \
  }
#include "hermes/VM/TypedArrays.def"
}

} // namespace
