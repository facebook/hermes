/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/WeakValueObjectMap.h"

#include "hermes/VM/DummyObject.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"

#include "VMRuntimeTestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using WeakValueObjectMapTest = LargeHeapRuntimeTestFixture;
using DummyObject = testhelpers::DummyObject;

TEST_F(WeakValueObjectMapTest, InsertNewTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<JSObject> key;
    PinnedValue<DummyObject> value1;
    PinnedValue<DummyObject> value2;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  lv.key = JSObject::create(runtime);
  lv.value1 = DummyObject::create(runtime.getHeap(), runtime);
  lv.value2 = DummyObject::create(runtime.getHeap(), runtime);

  EXPECT_TRUE(
      WeakValueObjectMap<DummyObject>::insertNew(
          lv.map, runtime, *lv.key, *lv.value1));

  EXPECT_FALSE(
      WeakValueObjectMap<DummyObject>::insertNew(
          lv.map, runtime, *lv.key, *lv.value2));

  // Free the value1 and trigger GC to invalidate the old WeakRef.
  lv.value1 = nullptr;
  runtime.collect("test");

  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.key));

  EXPECT_TRUE(
      WeakValueObjectMap<DummyObject>::insertNew(
          lv.map, runtime, *lv.key, *lv.value2));
}

TEST_F(WeakValueObjectMapTest, RehashTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<ArrayStorage> keys;
    PinnedValue<ArrayStorage> values;
    PinnedValue<JSObject> tempKey;
    PinnedValue<DummyObject> tempValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  lv.keys.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 32, 32));
  lv.values.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 32, 32));

  for (uint32_t i = 0; i < 10; ++i) {
    lv.tempKey = JSObject::create(runtime);
    lv.tempValue = DummyObject::create(runtime.getHeap(), runtime);

    // Store them to prevent GC.
    lv.keys->set(
        i, HermesValue::encodeObjectValue(*lv.tempKey), runtime.getHeap());
    lv.values->set(
        i, HermesValue::encodeObjectValue(*lv.tempValue), runtime.getHeap());
  }

  for (uint32_t i = 0; i < 2; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    DummyObject *expectedValue = vmcast<DummyObject>(lv.values->at(i));
    EXPECT_TRUE(
        WeakValueObjectMap<DummyObject>::insertNew(
            lv.map, runtime, key, expectedValue));
  }

  for (uint32_t i = 0; i < 2; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    DummyObject *expectedValue = vmcast<DummyObject>(lv.values->at(i));
    EXPECT_EQ(expectedValue, lv.map->lookup(runtime, key));
  }

  lv.values->set(1, HermesValue::encodeUndefinedValue(), runtime.getHeap());
  runtime.collect("test");
  ASSERT_FALSE(lv.map->containsKey(runtime, vmcast<JSObject>(lv.keys->at(1))));

  // Force the rehash with an invalid WeakRef.
  for (uint32_t i = 3; i < 10; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    DummyObject *expectedValue = vmcast<DummyObject>(lv.values->at(i));
    EXPECT_TRUE(
        WeakValueObjectMap<DummyObject>::insertNew(
            lv.map, runtime, key, expectedValue));
  }
}

TEST_F(WeakValueObjectMapTest, GrowTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<ArrayStorage> keys;
    PinnedValue<ArrayStorage> values;
    PinnedValue<JSObject> tempKey;
    PinnedValue<DummyObject> tempValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  // Create storage for keys and values to prevent GC.
  lv.keys.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 32, 32));
  lv.values.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 32, 32));

  // Initially map should be empty with 0 capacity.
  EXPECT_TRUE(lv.map->isKnownEmpty());
  EXPECT_EQ(lv.map->capacity(), 0);

  // Insert enough items to trigger multiple grows.
  // Initial capacity will be 8, so insert 20 items to grow to 16, then 32.
  const uint32_t numItems = 20;

  for (uint32_t i = 0; i < numItems; ++i) {
    // Create key and value using safe pattern.
    lv.tempKey = JSObject::create(runtime);
    lv.tempValue = DummyObject::create(runtime.getHeap(), runtime);

    // Store them to prevent GC.
    lv.keys->set(
        i, HermesValue::encodeObjectValue(*lv.tempKey), runtime.getHeap());
    lv.values->set(
        i, HermesValue::encodeObjectValue(*lv.tempValue), runtime.getHeap());

    // Insert into map.
    bool insertRes = WeakValueObjectMap<DummyObject>::insertNew(
        lv.map, runtime, *lv.tempKey, *lv.tempValue);
    EXPECT_TRUE(insertRes); // Should be inserted successfully.
  }

  // After inserting 20 items, capacity should have grown.
  // Started at 8 (first grow), then 16 (second grow), then 32.
  EXPECT_GE(lv.map->capacity(), 32);
  EXPECT_FALSE(lv.map->isKnownEmpty());

  // Verify all items are still accessible after rehashing.
  for (uint32_t i = 0; i < numItems; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    DummyObject *expectedValue = vmcast<DummyObject>(lv.values->at(i));

    // Test containsKey.
    EXPECT_TRUE(lv.map->containsKey(runtime, key));

    // Test lookup.
    DummyObject *foundValue = lv.map->lookup(runtime, key);
    ASSERT_NE(foundValue, nullptr);
    EXPECT_EQ(foundValue, expectedValue);
  }

  // Test that non-existent keys return null.
  lv.tempKey = JSObject::create(runtime);
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.tempKey));
  EXPECT_EQ(lv.map->lookup(runtime, *lv.tempKey), nullptr);
}

TEST_F(WeakValueObjectMapTest, ShrinkTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<ArrayStorage> keys;
    PinnedValue<ArrayStorage> values;
    PinnedValue<JSObject> tempKey;
    PinnedValue<DummyObject> tempValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  // Create storage for keys and values.
  lv.keys.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 64, 64));
  lv.values.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 64, 64));

  // Insert enough items to get a large capacity (64).
  const uint32_t numItems = 40;

  for (uint32_t i = 0; i < numItems; ++i) {
    lv.tempKey = JSObject::create(runtime);
    lv.tempValue = DummyObject::create(runtime.getHeap(), runtime);

    lv.keys->set(
        i, HermesValue::encodeObjectValue(*lv.tempKey), runtime.getHeap());
    lv.values->set(
        i, HermesValue::encodeObjectValue(*lv.tempValue), runtime.getHeap());

    bool insertRes = WeakValueObjectMap<DummyObject>::insertNew(
        lv.map, runtime, *lv.tempKey, *lv.tempValue);
    EXPECT_TRUE(insertRes);
  }

  // Should have grown to capacity of 64.
  EXPECT_GE(lv.map->capacity(), 64);

  // Now erase most items, keeping only a few (below 1/8 of capacity).
  // With capacity 64, keeping 3 items should trigger shrinking (3 < 64/8 = 8).
  const uint32_t itemsToKeep = 3;

  for (uint32_t i = itemsToKeep; i < numItems; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    bool erased = WeakValueObjectMap<DummyObject>::erase(
        lv.map, runtime, key, runtime.getHeap());
    EXPECT_TRUE(erased);
    key = vmcast<JSObject>(lv.keys->at(i));
    EXPECT_FALSE(lv.map->containsKey(runtime, key));

    {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      EXPECT_FALSE(lv.map->containsKey(runtime, key)) << "i = " << i << "\n";
      EXPECT_EQ(lv.map->lookup(runtime, key), nullptr);
    }

    if (i >= 27) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(27));
      EXPECT_FALSE(lv.map->containsKey(runtime, key)) << "i = " << 27 << "\n";
    }
  }

  // Capacity should've shrunk below 64.
  EXPECT_LT(lv.map->capacity(), 64);

  JSObject *key = vmcast<JSObject>(lv.keys->at(27));
  EXPECT_FALSE(lv.map->containsKey(runtime, key)) << "i = " << 27 << "\n";

  // Verify erased items are no longer accessible.
  for (uint32_t i = itemsToKeep; i < numItems; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    EXPECT_FALSE(lv.map->containsKey(runtime, key)) << "i = " << i << "\n";
    EXPECT_EQ(lv.map->lookup(runtime, key), nullptr);
  }

  // Verify the remaining items are still accessible.
  for (uint32_t i = 0; i < itemsToKeep; ++i) {
    JSObject *key = vmcast<JSObject>(lv.keys->at(i));
    DummyObject *expectedValue = vmcast<DummyObject>(lv.values->at(i));

    EXPECT_TRUE(lv.map->containsKey(runtime, key));
    DummyObject *foundValue = lv.map->lookup(runtime, key);
    ASSERT_NE(foundValue, nullptr);
    EXPECT_EQ(foundValue, expectedValue);
  }
}

TEST_F(WeakValueObjectMapTest, WeaknessWithGCTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<DummyObject> value1;
    PinnedValue<DummyObject> value2;
    PinnedValue<DummyObject> value3;
    PinnedValue<JSObject> key1;
    PinnedValue<JSObject> key2;
    PinnedValue<JSObject> key3;
    PinnedValue<JSObject> key4;
    PinnedValue<DummyObject> value4;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  // Create keys and values using safe allocation pattern.
  lv.key1 = JSObject::create(runtime);
  lv.key2 = JSObject::create(runtime);
  lv.key3 = JSObject::create(runtime);
  lv.key4 = JSObject::create(runtime);

  lv.value1 = DummyObject::create(runtime.getHeap(), runtime);
  lv.value2 = DummyObject::create(runtime.getHeap(), runtime);
  lv.value3 = DummyObject::create(runtime.getHeap(), runtime);
  lv.value4 = DummyObject::create(runtime.getHeap(), runtime);

  // Insert all values into the map.
  bool insertRes1 = WeakValueObjectMap<DummyObject>::insertNew(
      lv.map, runtime, *lv.key1, *lv.value1);
  EXPECT_TRUE(insertRes1);

  bool insertRes2 = WeakValueObjectMap<DummyObject>::insertNew(
      lv.map, runtime, *lv.key2, *lv.value2);
  EXPECT_TRUE(insertRes2);

  bool insertRes3 = WeakValueObjectMap<DummyObject>::insertNew(
      lv.map, runtime, *lv.key3, *lv.value3);
  EXPECT_TRUE(insertRes3);

  bool insertRes4 = WeakValueObjectMap<DummyObject>::insertNew(
      lv.map, runtime, *lv.key4, *lv.value4);
  EXPECT_TRUE(insertRes4);

  // All items should be accessible before GC.
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key1));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key2));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key3));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key4));

  // Clear value4 and trigger GC.
  // value4 should be collected.
  lv.value4 = nullptr;
  runtime.collect("test");

  // After GC, values 1, 2, 3 should still be accessible (strong references).
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key1));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key2));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key3));

  // value4 should no longer be accessible (weak reference was broken).
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.key4));
  EXPECT_EQ(lv.map->lookup(runtime, *lv.key4), nullptr);

  // Clear more references and test shrinking after pruning.
  lv.value1 = nullptr;
  lv.value2 = nullptr;
  runtime.collect("test");

  // Now only value3 should be accessible.
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.key1));
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.key2));
  EXPECT_TRUE(lv.map->containsKey(runtime, *lv.key3));
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.key4));
}

TEST_F(WeakValueObjectMapTest, RepeatedGrowShrinkTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<ArrayStorage> keys;
    PinnedValue<ArrayStorage> values;
    PinnedValue<JSObject> tempKey;
    PinnedValue<DummyObject> tempValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  // Create large storage for keys and values.
  lv.keys.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 100, 100));
  lv.values.castAndSetHermesValue<ArrayStorage>(
      *ArrayStorage::create(runtime, 100, 100));

  // Perform multiple cycles of grow and shrink.
  for (int cycle = 0; cycle < 3; ++cycle) {
    // Grow phase: insert many items.
    const uint32_t itemsToAdd = 30;

    for (uint32_t i = 0; i < itemsToAdd; ++i) {
      lv.tempKey = JSObject::create(runtime);
      lv.tempValue = DummyObject::create(runtime.getHeap(), runtime);

      lv.keys->set(
          i, HermesValue::encodeObjectValue(*lv.tempKey), runtime.getHeap());
      lv.values->set(
          i, HermesValue::encodeObjectValue(*lv.tempValue), runtime.getHeap());

      bool insertRes = WeakValueObjectMap<DummyObject>::insertNew(
          lv.map, runtime, *lv.tempKey, *lv.tempValue);
      EXPECT_TRUE(insertRes);
    }

    // Verify capacity has grown.
    // Should be at least 32 after adding 30 items.
    auto grownCapacity = lv.map->capacity();
    EXPECT_GE(grownCapacity, 32);

    // Verify all items are accessible.
    for (uint32_t i = 0; i < itemsToAdd; ++i) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      EXPECT_TRUE(lv.map->containsKey(runtime, key));
      EXPECT_NE(lv.map->lookup(runtime, key), nullptr);
    }

    // Shrink phase: remove most items.
    const uint32_t itemsToKeep = 2;

    for (uint32_t i = itemsToKeep; i < itemsToAdd; ++i) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      bool erased = WeakValueObjectMap<DummyObject>::erase(
          lv.map, runtime, key, runtime.getHeap());
      EXPECT_TRUE(erased);
    }

    // Verify capacity has shrunk or at least not grown further.
    auto shrunkCapacity = lv.map->capacity();
    EXPECT_LE(shrunkCapacity, grownCapacity);

    // Verify remaining items are still accessible.
    for (uint32_t i = 0; i < itemsToKeep; ++i) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      EXPECT_TRUE(lv.map->containsKey(runtime, key));
      EXPECT_NE(lv.map->lookup(runtime, key), nullptr);
    }

    // Verify removed items are no longer accessible.
    for (uint32_t i = itemsToKeep; i < itemsToAdd; ++i) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      EXPECT_FALSE(lv.map->containsKey(runtime, key));
      EXPECT_EQ(lv.map->lookup(runtime, key), nullptr);
    }

    // Clean up remaining items for next cycle.
    for (uint32_t i = 0; i < itemsToKeep; ++i) {
      JSObject *key = vmcast<JSObject>(lv.keys->at(i));
      WeakValueObjectMap<DummyObject>::erase(
          lv.map, runtime, key, runtime.getHeap());
    }
  }

  // After all cycles, map should be empty or very small.
  EXPECT_TRUE(lv.map->isKnownEmpty() || lv.map->capacity() <= 16);
}

TEST_F(WeakValueObjectMapTest, EmptyMapOperationsTest) {
  struct : public Locals {
    PinnedValue<WeakValueObjectMap<DummyObject>> map;
    PinnedValue<JSObject> tempKey;
    PinnedValue<DummyObject> tempValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.map = WeakValueObjectMap<DummyObject>::create(runtime);

  EXPECT_TRUE(lv.map->isKnownEmpty());
  EXPECT_EQ(lv.map->capacity(), 0);

  lv.tempKey = JSObject::create(runtime);
  EXPECT_FALSE(lv.map->containsKey(runtime, *lv.tempKey));
  EXPECT_EQ(lv.map->lookup(runtime, *lv.tempKey), nullptr);

  bool erased = WeakValueObjectMap<DummyObject>::erase(
      lv.map, runtime, *lv.tempKey, runtime.getHeap());
  EXPECT_FALSE(erased);

  lv.tempValue = DummyObject::create(runtime.getHeap(), runtime);
  bool insertRes = WeakValueObjectMap<DummyObject>::insertNew(
      lv.map, runtime, *lv.tempKey, *lv.tempValue);
  EXPECT_TRUE(insertRes);

  EXPECT_FALSE(lv.map->isKnownEmpty());
}
} // namespace
