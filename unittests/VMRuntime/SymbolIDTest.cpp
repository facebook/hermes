/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "VMRuntimeTestHelpers.h"
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
  MutableHandle<JSArray> array{runtime};
  array = std::move(*arrayResult);

  auto otherArrayResult = JSArray::create(runtime, 100, 100);
  ASSERT_FALSE(isException(otherArrayResult));
  MutableHandle<JSArray> otherArray{runtime};
  otherArray = std::move(*otherArrayResult);

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
TEST_F(SymbolIDRuntimeTest, WeakSymbol) {
  // Declare two weak symbols which the GC knows about.
  WeakRootSymbolID weakSymA;
  WeakRootSymbolID weakSymB;

  // Declare a weak symbol that we will point to a lazy symbol. This should
  // never be cleared.
  WeakRootSymbolID weakSymLazy;
  runtime.addCustomWeakRootsFunction(
      [&](vm::GC *, vm::WeakRootAcceptor &acceptor) {
        acceptor.acceptWeakSym(weakSymA);
        acceptor.acceptWeakSym(weakSymB);
        acceptor.acceptWeakSym(weakSymLazy);
      });

  struct : Locals {
    PinnedValue<SymbolID> symA;
    PinnedValue<SymbolID> symB;
    PinnedValue<StringPrimitive> str;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  std::string asciiStr = "teststring";
  auto strRes = StringPrimitive::create(
      runtime, ASCIIRef{asciiStr.c_str(), asciiStr.length()});
  ASSERT_FALSE(isException(strRes));
  lv.str = vmcast<StringPrimitive>(*strRes);
  MutableHandle<StringPrimitive> str{lv.str};

  // Init PV and weak symbol for A.
  auto symResA =
      runtime.getIdentifierTable().createNotUniquedSymbol(runtime, str);
  ASSERT_FALSE(isException(symResA));
  lv.symA = *symResA;
  weakSymA = *symResA;

  // Init PV and weak symbol for B.
  auto symResB =
      runtime.getIdentifierTable().createNotUniquedSymbol(runtime, str);
  ASSERT_FALSE(isException(symResB));
  lv.symB = *symResB;
  weakSymB = *symResB;

  // Init the lazy symbol.
  weakSymLazy = runtime.getIdentifierTable().registerLazyIdentifier(
      "I am the laziest symbol in the world!");

  // Perform a GC. Both weak symbols should be fine since A and B both are being
  // referenced in live PVs.
  runtime.collect("test");
  ASSERT_FALSE(weakSymA.isInvalid());
  ASSERT_FALSE(weakSymB.isInvalid());

  // The lazy symbol should never be cleared.
  ASSERT_FALSE(weakSymLazy.isInvalid());

  // Invalidate PV symB, then perform a GC. This should result in only
  // weak symbol B being set to invalid.
  lv.symB = SymbolID{};
  runtime.collect("test");
  ASSERT_FALSE(weakSymA.isInvalid());
  ASSERT_TRUE(weakSymB.isInvalid());

  // The lazy symbol should never be cleared.
  ASSERT_FALSE(weakSymLazy.isInvalid());

  // Invalidate PV symA, then perform a GC. At this point, both weak
  // symbols should be cleared.
  lv.symA = SymbolID{};
  runtime.collect("test");
  ASSERT_TRUE(weakSymA.isInvalid());
  ASSERT_TRUE(weakSymB.isInvalid());

  // The lazy symbol should never be cleared.
  ASSERT_FALSE(weakSymLazy.isInvalid());
}

TEST_F(SymbolIDRuntimeTest, SymbolAllocDuringGC) {
  WeakRootSymbolID weakSym;
  runtime.addCustomWeakRootsFunction(
      [&](vm::GC *, vm::WeakRootAcceptor &acceptor) {
        acceptor.acceptWeakSym(weakSym);
      });

  std::string asciiStr = "teststring";
  auto strRes = StringPrimitive::create(
      runtime, ASCIIRef{asciiStr.c_str(), asciiStr.length()});
  ASSERT_FALSE(isException(strRes));
  MutableHandle<StringPrimitive> str{runtime};
  str = vmcast<StringPrimitive>(*strRes);
  (void)str;

  struct : Locals {
    /// Array holding all the strong symbols.
    PinnedValue<ArrayStorage> strongSymbols;
    PinnedValue<SymbolID> curSym;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  lv.strongSymbols = vmcast<ArrayStorage>(*ArrayStorage::create(runtime, 4));
  MutableHandle<ArrayStorage> strongSymbols(lv.strongSymbols);

  GCScopeMarkerRAII marker{runtime};
  for (size_t i = 0; i < 1000; i++) {
    // Allocate a new SymbolID which is strongly held.
    {
      auto symRes =
          runtime.getIdentifierTable().createNotUniquedSymbol(runtime, str);
      ASSERT_FALSE(isException(symRes));
      lv.curSym = *symRes;
      (void)ArrayStorage::push_back(strongSymbols, runtime, lv.curSym);
      weakSym = *symRes;
      // The weak symbol should never be invalid.
      marker.flush();
    }
    for (size_t i = 0; i < 1000; i++) {
      JSObject::create(runtime);
      // The weak symbol should never be invalid.
      ASSERT_FALSE(weakSym.isInvalid());
    }
  }
}

} // namespace
