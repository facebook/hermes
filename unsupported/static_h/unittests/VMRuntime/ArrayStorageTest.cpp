/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/ArrayStorage.h"

#include "TestHelpers.h"

using namespace hermes::vm;

namespace {

using ArrayStorageTest = RuntimeTestFixture;

TEST_F(ArrayStorageTest, ShiftTest) {
  MutableHandle<ArrayStorage> st(runtime);
  st = vmcast<ArrayStorage>(*ArrayStorage::create(runtime, 4));

  // Resize to one element: "."
  (void)ArrayStorage::resize(st, runtime, 1);
  ASSERT_EQ(1u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(0));
  st->setNonPtr(0, 1.0_hd, runtime.getHeap());
  // "1"
  ASSERT_EQ(1.0_hd, st->at(0));

  // Resize to two elements: "1."
  (void)ArrayStorage::resize(st, runtime, 2);
  ASSERT_EQ(2u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(1.0_hd, st->at(0));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(1));
  // "12"
  st->setNonPtr(1, 2.0_hd, runtime.getHeap());

  // Resize "12" to ".12."
  (void)ArrayStorage::shift(st, runtime, 0, 1, 4);
  ASSERT_EQ(4u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(0));
  ASSERT_EQ(1.0_hd, st->at(1));
  ASSERT_EQ(2.0_hd, st->at(2));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(3));

  // Resize ".12." to "12"
  (void)ArrayStorage::shift(st, runtime, 1, 0, 2);
  ASSERT_EQ(2u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(1.0_hd, st->at(0));
  ASSERT_EQ(2.0_hd, st->at(1));
  // We want to check that resize() doesn't do more work than it has to, so we
  // are checking that the element which remained outside of size is the same.
  ASSERT_EQ(2.0_hd, st->data()[2]);

  // Move it to the right again. "12" -> "...1".
  (void)ArrayStorage::shift(st, runtime, 0, 3, 4);
  ASSERT_EQ(4u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(0));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(1));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(2));
  ASSERT_EQ(1.0_hd, st->at(3));

  // "...1" -> "1..."
  (void)ArrayStorage::shift(st, runtime, 3, 0, 4);
  ASSERT_EQ(4u, st->size());
  ASSERT_EQ(4u, st->capacity());
  ASSERT_EQ(1.0_hd, st->at(0));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(1));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(2));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(3));
  // "12.."
  st->setNonPtr(1, 2.0_hd, runtime.getHeap());

  // Now let's do a reallocation. Resize to 6. "12.." -> "...12."
  (void)ArrayStorage::shift(st, runtime, 0, 3, 6);
  ASSERT_EQ(6u, st->size());
  ASSERT_EQ(8u, st->capacity());
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(0));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(1));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(2));
  ASSERT_EQ(1.0_hd, st->at(3));
  ASSERT_EQ(2.0_hd, st->at(4));
  ASSERT_EQ(HermesValue::encodeEmptyValue(), st->at(5));
}

TEST_F(ArrayStorageTest, PushBackTest) {
  MutableHandle<ArrayStorage> st(runtime);
  st = vmcast<ArrayStorage>(*ArrayStorage::create(runtime, 4));

  (void)ArrayStorage::push_back(st, runtime, runtime.makeHandle(1.0_hd));
  (void)ArrayStorage::push_back(st, runtime, runtime.makeHandle(2.0_hd));
  (void)ArrayStorage::push_back(st, runtime, runtime.makeHandle(3.0_hd));
  (void)ArrayStorage::push_back(st, runtime, runtime.makeHandle(4.0_hd));
  (void)ArrayStorage::push_back(st, runtime, runtime.makeHandle(5.0_hd));

  ASSERT_EQ(5u, st->size());
  ASSERT_EQ(8u, st->capacity());
  ASSERT_EQ(1.0_hd, st->at(0));
  ASSERT_EQ(2.0_hd, st->at(1));
  ASSERT_EQ(3.0_hd, st->at(2));
  ASSERT_EQ(4.0_hd, st->at(3));
  ASSERT_EQ(5.0_hd, st->at(4));
}

TEST_F(ArrayStorageTest, AllowTrimming) {
  MutableHandle<ArrayStorage> st(runtime);
  constexpr ArrayStorage::size_type originalCapacity = 8;
  // Create an array and put in an element so its size is 1 and its capacity
  // is 8.
  st = vmcast<ArrayStorage>(*ArrayStorage::create(runtime, originalCapacity));
  EXPECT_LE(st->capacity(), originalCapacity);
  ASSERT_RETURNED(
      ArrayStorage::push_back(st, runtime, runtime.makeHandle(0.0_hd)));
  EXPECT_EQ(1, st->size());

  // Now force some GCs to happen.
  for (auto i = 0; i < 2; i++) {
    runtime.collect("test");
  }

  // The array should be trimmed.
  EXPECT_EQ(st->size(), st->capacity());
}

using ArrayStorageBigHeapTest = LargeHeapRuntimeTestFixture;

#ifndef HERMESVM_GC_MALLOC
// The following test allocates gigantic arrays on non-NCGen GCs.
TEST_F(ArrayStorageBigHeapTest, AllocMaxSizeArray) {
  // Should succeed, allocations up to maxElements are allowed.
  auto res = ArrayStorage::create(runtime, ArrayStorage::maxElements());
  EXPECT_EQ(res, ExecutionStatus::RETURNED)
      << "Allocating a max size array failed";
}
#endif

TEST_F(ArrayStorageBigHeapTest, AllocLargeArrayThrowsRangeError) {
  // Should fail with a RangeError for allocations above the maxElements.
  auto res = ArrayStorage::create(runtime, ArrayStorage::maxElements() + 1);
  EXPECT_EQ(res, ExecutionStatus::EXCEPTION)
      << "Allocating an array slightly larger than its max size should throw";
  HermesValue hv = runtime.getThrownValue();
  EXPECT_EQ(
      vmcast<JSObject>(hv)->getParent(runtime),
      vmcast<JSObject>(runtime.RangeErrorPrototype))
      << "Exception thrown was not a RangeError";
}

} // namespace
