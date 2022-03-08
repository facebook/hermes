/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/DictPropertyMap.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using DictPropertyMapTest = LargeHeapRuntimeTestFixture;

TEST_F(DictPropertyMapTest, SmokeTest) {
  auto id1 = SymbolID::unsafeCreate(1);
  auto id2 = SymbolID::unsafeCreate(2);
  auto id3 = SymbolID::unsafeCreate(3);
  auto id4 = SymbolID::unsafeCreate(4);

  NamedPropertyDescriptor desc1{};

  auto res = DictPropertyMap::create(runtime, 2);
  ASSERT_FALSE(isException(res));
  MutableHandle<DictPropertyMap> map{runtime, res->get()};
  auto saveMap = map.get();

  // Try to find a property in the empty map.
  ASSERT_FALSE(DictPropertyMap::find(map.get(), id1));

  // Add prop1.
  DictPropertyMap::add(map, runtime, id1, desc1);
  ASSERT_EQ(1u, map->size());

  auto found = DictPropertyMap::find(*map, id1);
  ASSERT_TRUE(found);
  ASSERT_EQ(id1, DictPropertyMap::getDescriptorPair(*map, *found)->first);

  // Add prop2.
  DictPropertyMap::add(map, runtime, id2, desc1);
  ASSERT_EQ(2u, map->size());

  // Find prop1, prop2.
  found = DictPropertyMap::find(*map, id1);
  ASSERT_TRUE(found);
  ASSERT_EQ(id1, DictPropertyMap::getDescriptorPair(*map, *found)->first);
  found = DictPropertyMap::find(*map, id2);
  ASSERT_TRUE(found);
  ASSERT_EQ(id2, DictPropertyMap::getDescriptorPair(*map, *found)->first);

  // Make sure we haven't reallocated.
  ASSERT_EQ(saveMap, map.get());

  // Add a prop3, causing a reallocation.
  DictPropertyMap::add(map, runtime, id3, desc1);
  // Make sure we reallocated.
  ASSERT_NE(saveMap, map.get());
  saveMap = map.get();
  for (unsigned i = 0; i < 3; ++i) {
    auto sym = SymbolID::unsafeCreate(id1.unsafeGetIndex() + i);
    found = DictPropertyMap::find(*map, sym);
    ASSERT_TRUE(found);
    ASSERT_EQ(sym, DictPropertyMap::getDescriptorPair(*map, *found)->first);
  }

  // Add a prop4.
  DictPropertyMap::add(map, runtime, id4, desc1);
  ASSERT_EQ(saveMap, map.get());
  for (unsigned i = 0; i < 4; ++i) {
    auto sym = SymbolID::unsafeCreate(id1.unsafeGetIndex() + i);
    found = DictPropertyMap::find(*map, sym);
    ASSERT_TRUE(found);
    ASSERT_EQ(sym, DictPropertyMap::getDescriptorPair(*map, *found)->first);
  }

  // Verify the enumeration order.
  {
    unsigned symIndex = 1;
    DictPropertyMap::forEachProperty(
        map, runtime, [&](SymbolID id, NamedPropertyDescriptor desc) {
          ASSERT_EQ(symIndex, id.unsafeGetIndex());
          ++symIndex;
        });
  }

  // Delete prop2.
  found = DictPropertyMap::find(*map, id2);
  DictPropertyMap::erase(*map, runtime, *found);
  ASSERT_EQ(saveMap, map.get());
  ASSERT_EQ(3u, map->size());

  {
    static const unsigned symbols[] = {1, 3, 4};
    for (unsigned i = 0; i < 3; ++i) {
      auto sym = SymbolID::unsafeCreate(symbols[i]);
      found = DictPropertyMap::find(*map, sym);
      ASSERT_TRUE(found);
      ASSERT_EQ(sym, DictPropertyMap::getDescriptorPair(*map, *found)->first);
    }
    unsigned i = 0;
    DictPropertyMap::forEachProperty(
        map, runtime, [&](SymbolID id, NamedPropertyDescriptor desc) {
          ASSERT_EQ(symbols[i], id.unsafeGetIndex());
          ++i;
        });
  }

  // Add prop2 again.
  DictPropertyMap::add(map, runtime, id2, desc1);
  ASSERT_EQ(4u, map->size());

  {
    static const unsigned symbols[] = {1, 3, 4, 2};
    for (unsigned i = 0; i < 4; ++i) {
      auto sym = SymbolID::unsafeCreate(symbols[i]);
      found = DictPropertyMap::find(*map, sym);
      ASSERT_TRUE(found);
      ASSERT_EQ(sym, DictPropertyMap::getDescriptorPair(*map, *found)->first);
    }
    unsigned i = 0;
    DictPropertyMap::forEachProperty(
        map, runtime, [&](SymbolID id, NamedPropertyDescriptor desc) {
          ASSERT_EQ(symbols[i], id.unsafeGetIndex());
          ++i;
        });
  }
}

TEST_F(DictPropertyMapTest, CreateOverCapacityTest) {
  (void)DictPropertyMap::create(runtime);
  ASSERT_EQ(
      ExecutionStatus::EXCEPTION,
      DictPropertyMap::create(runtime, DictPropertyMap::getMaxCapacity() + 1));
}

TEST_F(DictPropertyMapTest, GrowOverCapacityTest) {
  // Hades can't handle doing a span of large allocations, because it has
  // fragmentation in its heap space.
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  // Don't do the test if it requires too many properties. Just cross our
  // fingers and hope it works.
  auto const maxCapacity = DictPropertyMap::getMaxCapacity();
  if (maxCapacity > 500000)
    return;

  auto res = DictPropertyMap::create(runtime);
  ASSERT_RETURNED(res);
  MutableHandle<DictPropertyMap> map{runtime, res->get()};

  MutableHandle<> value{runtime};
  NamedPropertyDescriptor desc(PropertyFlags{}, 0);

  auto marker = gcScope.createMarker();
  for (unsigned i = 0; i < maxCapacity; ++i) {
    value.set(HermesValue::encodeNumberValue(i));
    auto symRes = valueToSymbolID(runtime, value);
    ASSERT_RETURNED(symRes);
    ASSERT_RETURNED(DictPropertyMap::add(map, runtime, **symRes, desc));

    gcScope.flushToMarker(marker);
  }

  value.set(HermesValue::encodeNumberValue(maxCapacity));
  auto symRes = valueToSymbolID(runtime, value);
  ASSERT_RETURNED(symRes);
  ASSERT_EQ(
      ExecutionStatus::EXCEPTION,
      DictPropertyMap::add(map, runtime, **symRes, desc));
  runtime.clearThrownValue();

  // Try it again.
  value.set(HermesValue::encodeNumberValue(maxCapacity + 1));
  symRes = valueToSymbolID(runtime, value);
  ASSERT_RETURNED(symRes);
  ASSERT_EQ(
      ExecutionStatus::EXCEPTION,
      DictPropertyMap::add(map, runtime, **symRes, desc));
  runtime.clearThrownValue();
#endif
}
} // namespace
