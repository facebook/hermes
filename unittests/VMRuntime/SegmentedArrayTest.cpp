/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SegmentedArray.h"

#include "TestHelpers.h"

using namespace hermes::vm;

namespace {

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
// Only NCGen has a limit that makes this allocation feasible.

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
#endif

using SegmentedArrayTest = RuntimeTestFixture;

TEST_F(SegmentedArrayTest, AllocLargeArrayThrowsRangeError) {
  // Should fail with a RangeError for allocations above the maxElements.
  auto res = SegmentedArray::create(runtime, SegmentedArray::maxElements() + 1);
  EXPECT_EQ(res, ExecutionStatus::EXCEPTION)
      << "Allocating an array slightly larger than its max size should throw";
  HermesValue hv = runtime->getThrownValue();
  EXPECT_EQ(
      vmcast<JSObject>(hv)->getParent(runtime),
      vmcast<JSObject>(runtime->RangeErrorPrototype))
      << "Exception thrown was not a RangeError";
}

} // namespace
