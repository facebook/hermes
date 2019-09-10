/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/WeakValueMap.h"

#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

// This test relies on getHeapInfo, which is only enabled in debug builds.
#ifndef NDEBUG

using namespace hermes::vm;

namespace {

using WeakValueMapTest = LargeHeapRuntimeTestFixture;

TEST_F(WeakValueMapTest, SmokeTest) {
  runtime->collect();

  WeakValueMap<int, JSNumber> wvp{};

  runtime->addCustomWeakRootsFunction(
      [&](GC *, WeakRefAcceptor &acceptor) { wvp.markWeakRefs(acceptor); });

  auto makeNumber = [&](int n) -> JSNumber * {
    auto numRes = JSNumber::create(
        runtime, (double)n, Runtime::makeNullHandle<JSObject>());
    assert(numRes != ExecutionStatus::EXCEPTION);
    return (JSNumber *)numRes->getPointer();
  };

  MutableHandle<JSNumber> h1{runtime};
  MutableHandle<JSNumber> h2{runtime};
  MutableHandle<JSNumber> h3{runtime};

  auto marker = gcScope.createMarker();

  h1 = makeNumber(1);
  h2 = makeNumber(2);
  h3 = makeNumber(3);

  gcScope.flushToMarker(marker);

  EXPECT_TRUE(wvp.insertNew(&runtime->getHeap(), 1, h1));
  EXPECT_TRUE(wvp.insertNew(&runtime->getHeap(), 2, h2));
  EXPECT_TRUE(wvp.insertNew(&runtime->getHeap(), 3, h3));
  EXPECT_FALSE(wvp.insertNew(&runtime->getHeap(), 2, h2));
  EXPECT_FALSE(wvp.insertNew(&runtime->getHeap(), 3, h3));

  // Make sure enumaration covers all cases.
  {
    std::set<int> intset{};
    auto it = wvp.begin();
    ASSERT_TRUE(it != wvp.end());
    ASSERT_TRUE(intset.insert(it->first).second);
    ++it;
    ASSERT_TRUE(it != wvp.end());
    ASSERT_TRUE(intset.insert(it->first).second);
    ++it;
    ASSERT_TRUE(it != wvp.end());
    ASSERT_TRUE(intset.insert(it->first).second);
    ++it;
    ASSERT_TRUE(it == wvp.end());
  }

  // Validate erase. Erase 1 and 2.
  auto it = wvp.find(1);
  ASSERT_TRUE(it != wvp.end());
  wvp.erase(it);
  it = wvp.find(1);
  ASSERT_TRUE(it == wvp.end());
  ASSERT_FALSE(wvp.erase(1));
  ASSERT_TRUE(wvp.erase(2));
  ASSERT_TRUE(wvp.find(2) == wvp.end());

  // Add 1 and 2 again.
  EXPECT_TRUE(wvp.insertNew(&runtime->getHeap(), 1, h1));
  EXPECT_TRUE(wvp.insertNew(&runtime->getHeap(), 2, h2));

  // Now make sure 1 gets garbage collected.
  ASSERT_TRUE(wvp.find(1) != wvp.end());
  h1.clear();
  // Make sure no temporary handles exist.
  gcScope.flushToMarker(marker);

  runtime->collect();
  GCBase::DebugHeapInfo debugInfo;
  runtime->getHeap().getDebugHeapInfo(debugInfo);
  // We can't be sure how many cells precisely this will collect.
  ASSERT_TRUE(
      debugInfo.numCollectedObjects > 0 && debugInfo.numCollectedObjects <= 5);
  // Make sure we can't find the collected value.
  ASSERT_TRUE(wvp.find(1) == wvp.end());

  // Now make sure 2 gets garbage collected, but check differently.
  h2.clear();
  // Make sure no temporary handles exist.
  gcScope.flushToMarker(marker);

  runtime->collect();
  runtime->getHeap().getDebugHeapInfo(debugInfo);
  // We can't be sure how many cells precisely this will collect.
  ASSERT_TRUE(
      debugInfo.numCollectedObjects > 0 && debugInfo.numCollectedObjects <= 5);

  it = wvp.begin();
  ASSERT_TRUE(it != wvp.end());
  ASSERT_EQ(3, it->first);
  ++it;
  ASSERT_TRUE(it == wvp.end());

  // Test lookup.
  ASSERT_TRUE(wvp.lookup(runtime, 3));
  ASSERT_FALSE(wvp.lookup(runtime, 300));
}
} // namespace

#endif // NDEBUG
