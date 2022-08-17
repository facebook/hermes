/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SegmentedArray.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/HermesValueTraits.h"

#include "TestHelpers.h"

using namespace hermes::vm;

namespace {

struct SegmentedArrayBigHeapTest : public RuntimeTestFixtureBase {
  static const GCConfig kMassiveTestGCConfig;
  static const RuntimeConfig kTestRTConfig;
  SegmentedArrayBigHeapTest() : RuntimeTestFixtureBase(kTestRTConfig) {}
};

// The GC size must be huge in order to attempt to allocate a SegmentedArray
// near its max elements limit.
const GCConfig SegmentedArrayBigHeapTest::kMassiveTestGCConfig =
    GCConfig::Builder(kTestGCConfigBuilder)
        .withInitHeapSize(1 << 26)
        .withMaxHeapSize(1 << 29)
        .build();

const RuntimeConfig SegmentedArrayBigHeapTest::kTestRTConfig =
    RuntimeConfig::Builder()
        .withGCConfig(SegmentedArrayBigHeapTest::kMassiveTestGCConfig)
        .build();

TEST_F(SegmentedArrayBigHeapTest, AllocMaxSizeArray) {
  // Should succeed, allocations up to maxElements are allowed.
  auto res = SegmentedArray::create(runtime, SegmentedArray::maxElements());
  EXPECT_EQ(res, ExecutionStatus::RETURNED)
      << "Allocating a max size array failed";
}

using SegmentedArrayTest = RuntimeTestFixture;

TEST_F(SegmentedArrayTest, AllocLargeArrayThrowsRangeError) {
  // Should fail with a RangeError for allocations above the maxElements.
  auto res = SegmentedArray::create(runtime, SegmentedArray::maxElements() + 1);
  EXPECT_EQ(res, ExecutionStatus::EXCEPTION)
      << "Allocating an array slightly larger than its max size should throw";
  HermesValue hv = runtime.getThrownValue();
  EXPECT_EQ(
      vmcast<JSObject>(hv)->getParent(runtime),
      vmcast<JSObject>(runtime.RangeErrorPrototype))
      << "Exception thrown was not a RangeError";
}

TEST_F(SegmentedArrayTest, AllowTrimming) {
  MutableHandle<SegmentedArray> array(runtime);
  constexpr SegmentedArray::size_type originalCapacity = 8;
  // Create an array and put in an element so its size is 1 and its capacity
  // is 8.
  array = std::move(*SegmentedArray::create(runtime, originalCapacity));
  // The capacity is not guaranteed to match the input parameter, it is taken
  // as a hint, so check for <=.
  EXPECT_LE(array->capacity(), originalCapacity);
  ASSERT_RETURNED(
      SegmentedArray::push_back(array, runtime, runtime.makeHandle(0.0_hd)));
  EXPECT_EQ(1, array->size(runtime));

  // Now force some GCs to happen.
  for (auto i = 0; i < 2; i++) {
    runtime.collect("test");
  }

  // The array should be trimmed.
  EXPECT_EQ(array->size(runtime), array->capacity());
}

} // namespace
