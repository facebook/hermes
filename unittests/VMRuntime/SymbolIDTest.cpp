/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "TestHelpers.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SymbolID.h"

using namespace hermes;
using namespace hermes::vm;

namespace {

TEST(SymbolIDTest, UniquedTest) {
  auto id1 = SymbolID::unsafeCreateNotUniqued(0x500001);
  EXPECT_TRUE(id1.isNotUniqued());
  EXPECT_FALSE(id1.isUniqued());
  EXPECT_EQ(0x500001 | SymbolID::NOT_UNIQUED_MASK, id1.unsafeGetRaw());
  EXPECT_EQ(0x500001, id1.unsafeGetIndex());
  auto id2 = SymbolID::unsafeCreate(0x400007);
  EXPECT_TRUE(id2.isUniqued());
  EXPECT_FALSE(id2.isNotUniqued());
  EXPECT_EQ(0x400007, id2.unsafeGetRaw());
  EXPECT_EQ(0x400007, id2.unsafeGetIndex());
}

using SymbolIDRuntimeTest = RuntimeTestFixture;

TEST_F(SymbolIDRuntimeTest, WriteBarrier) {
  // Hades adds a write barrier for symbols. Make sure it correctly captures
  // mutations.
  auto arrayResult = JSArray::create(runtime, 100, 100);
  ASSERT_FALSE(isException(arrayResult));
  MutableHandle<JSArray> array{runtime, arrayResult->get()};

  auto otherArrayResult = JSArray::create(runtime, 100, 100);
  ASSERT_FALSE(isException(otherArrayResult));
  MutableHandle<JSArray> otherArray{runtime, otherArrayResult->get()};

  MutableHandle<SymbolID> symbol{runtime};
  MutableHandle<StringPrimitive> str{runtime};
  for (JSArray::size_type i = 0; i < 100; i++) {
    GCScopeMarkerRAII marker{runtime};
    std::string iAsStr = std::to_string(i);
    auto strRes = StringPrimitive::create(
        runtime, ASCIIRef{iAsStr.c_str(), iAsStr.length()});
    ASSERT_FALSE(isException(strRes));
    str = vmcast<StringPrimitive>(*strRes);
    auto symbolRes =
        runtime.getIdentifierTable().createNotUniquedSymbol(runtime, str);
    ASSERT_FALSE(isException(symbolRes));
    symbol = *symbolRes;
    JSArray::setElementAt(array, runtime, i, symbol);
  }
  // Move everything to OG.
  runtime.collect("test");
  // Copy symbols between arrays, and delete the symbols in the old array.
  // Do some allocation along the way to try and start a second OG collection.
  // Repeat this a few times to increase confidence that a write barrier happens
  // during a GC.
  for (int repeat = 0; repeat < 5; repeat++) {
    for (JSArray::size_type i = 0; i < 100; i++) {
      symbol = array->at(runtime, i).getSymbol();
      JSArray::setElementAt(otherArray, runtime, i, symbol);
      // Set to undefined to execute a write barrier.
      JSArray::setElementAt(array, runtime, i, runtime.getUndefinedValue());
      // Create some garbage to try and start an OG collection.
      for (int allocs = 0; allocs < 100; allocs++) {
        JSObject::create(runtime);
      }
    }
    // Swap arrays and repeat.
    JSArray *tmp = array.get();
    array = otherArray.get();
    otherArray = tmp;
  }
}

} // namespace
