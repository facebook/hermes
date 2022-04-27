/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/WeakValueMap.h"

#include "hermes/VM/DummyObject.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

// This test relies on getHeapInfo, which is only enabled in debug builds.
#ifndef NDEBUG

using namespace hermes::vm;

namespace {

using WeakValueMapTest = LargeHeapRuntimeTestFixture;
using testhelpers::DummyObject;

TEST_F(WeakValueMapTest, SmokeTest) {
  runtime.collect("test");

  // Since the lifetime of the runtime is longer than the lifetime of the map,
  // we give the runtime a shared_ptr to the map.
  auto wvpPtr = std::make_shared<WeakValueMap<int, JSNumber>>();
  auto &wvp = *wvpPtr;

  auto dummyObj =
      runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  dummyObj->markWeakCallback = std::make_unique<DummyObject::MarkWeakCallback>(
      [wvpPtr](GCCell *, WeakRefAcceptor &acceptor) {
        wvpPtr->markWeakRefs(acceptor);
      });

  auto makeNumber = [&](int n) -> JSNumber * {
    return JSNumber::create(
               runtime, (double)n, Runtime::makeNullHandle<JSObject>())
        .get();
  };

  MutableHandle<JSNumber> h1{runtime};
  MutableHandle<JSNumber> h2{runtime};
  MutableHandle<JSNumber> h3{runtime};

  auto marker = gcScope.createMarker();

  h1 = makeNumber(1);
  h2 = makeNumber(2);
  h3 = makeNumber(3);

  gcScope.flushToMarker(marker);

  EXPECT_TRUE(wvp.insertNew(runtime, 1, h1));
  EXPECT_TRUE(wvp.insertNew(runtime, 2, h2));
  EXPECT_TRUE(wvp.insertNew(runtime, 3, h3));
  EXPECT_FALSE(wvp.insertNew(runtime, 2, h2));
  EXPECT_FALSE(wvp.insertNew(runtime, 3, h3));

  // Make sure enumaration covers all cases.
  {
    std::set<int> intset{};
    wvp.forEachEntry(
        [&intset](int key, const WeakRef<JSNumber> &) { intset.insert(key); });
    ASSERT_EQ(intset.size(), 3);
  }

  // Validate erase. Erase 1 and 2.
  ASSERT_TRUE(wvp.containsKey(1));
  ASSERT_TRUE(wvp.containsKey(2));
  wvp.erase(1, runtime.getHeap());
  ASSERT_FALSE(wvp.containsKey(1));
  ASSERT_FALSE(wvp.erase(1, runtime.getHeap()));
  ASSERT_TRUE(wvp.erase(2, runtime.getHeap()));
  ASSERT_FALSE(wvp.containsKey(2));

  // Add 1 and 2 again.
  EXPECT_TRUE(wvp.insertNew(runtime, 1, h1));
  EXPECT_TRUE(wvp.insertNew(runtime, 2, h2));

  // Now make sure 1 gets garbage collected.
  ASSERT_TRUE(wvp.containsKey(1));
  h1.clear();
  // Make sure no temporary handles exist.
  gcScope.flushToMarker(marker);

  runtime.collect("test");
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  // Hades doesn't support DebugHeapInfo yet.
  GCBase::DebugHeapInfo debugInfo;
  runtime.getHeap().getDebugHeapInfo(debugInfo);
  // We can't be sure how many cells precisely this will collect.
  ASSERT_TRUE(
      debugInfo.numCollectedObjects > 0 && debugInfo.numCollectedObjects <= 5);
#endif
  // Make sure we can't find the collected value.
  ASSERT_FALSE(wvp.containsKey(1));

  // Now make sure 2 gets garbage collected, but check differently.
  h2.clear();
  // Make sure no temporary handles exist.
  gcScope.flushToMarker(marker);

  runtime.collect("test");
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  // Hades doesn't support debugInfo yet.
  runtime.getHeap().getDebugHeapInfo(debugInfo);
  // We can't be sure how many cells precisely this will collect.
  ASSERT_TRUE(
      debugInfo.numCollectedObjects > 0 && debugInfo.numCollectedObjects <= 5);
#endif

  ASSERT_TRUE(wvp.containsKey(3));

  // Test lookup.
  ASSERT_TRUE(wvp.lookup(runtime, 3));
  ASSERT_FALSE(wvp.lookup(runtime, 300));
}
} // namespace

#endif // NDEBUG
