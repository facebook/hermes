/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
#include "hermes/VM/AlignedHeapSegment.h"
#endif
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GC.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

struct GCSizingTest : public ::testing::Test {
  std::shared_ptr<DummyRuntime> runtime;
  DummyRuntime &rt;
  GCSizingTest()
      : runtime(DummyRuntime::create(GCConfig::Builder()
                                         .withInitHeapSize(256)
                                         .withMaxHeapSize(100000000)
                                         .build())),
        rt(*runtime) {}
};

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
// At present, this test only applies to the Generational collector.

TEST_F(GCSizingTest, TestOccupancyTarget) {
  // We allocate an ever-growing data structure, and also go back and
  // remove references to some objects that, in a generational system,
  // should have been tenured.  Our aim is to achieve an occupancy
  // rate in the old gen (or the heap in general, in a
  // non-generational system) that is greater than 1/2 (the default
  // occupancy target), but significantly less than 100%, so the heap
  // should grow -- but by less than a factor of two.  (There were
  // some bugs with overly-aggressive heap sizing.)

  GenGC &gc = rt.getHeap();

  GCScope topScope(&rt);

  // This will be the head of a linked list of arrays, each of whom
  // contains pointers to other objects, and a next property.
  auto head = rt.makeMutableHandle<ArrayStorage>(nullptr);

  // This will be a previous value of head, remaining kF list elements
  // behind (once the number of elements exceeds kN);
  auto headFollower = rt.makeMutableHandle<ArrayStorage>(nullptr);

  // Used temporarily in allocating a new head.
  auto newHead = rt.makeMutableHandle<ArrayStorage>(nullptr);

  const unsigned kF = 100;
  // The number of top-level elements we will allocate.
  const unsigned kN = 1000;
  // The size of a list element array.
  const unsigned kP = 100;
  // The size of a leaf element.
  const unsigned kLeafSize = 10;

  // We use the first two elements of the arrays as prev and next pointers.
  const unsigned kPrevFieldIndex = 0;
  const unsigned kNextFieldIndex = 1;
  const unsigned kNumFieldPtrIndices = 2;

  const unsigned kGCFrequency = 200;

  for (unsigned i = 0; i < kN; i++) {
    newHead = ArrayStorage::createForTest(&gc, kP + kNumFieldPtrIndices);
    // Link in the new head.
    if (*head) {
      newHead->set(kNextFieldIndex, HermesValue::encodeObjectValue(*head), &gc);
      head->set(kPrevFieldIndex, HermesValue::encodeObjectValue(*newHead), &gc);
    } else {
      newHead->set(kNextFieldIndex, HermesValue::encodeNullValue(), &gc);
    }
    head = *newHead;
    for (unsigned j = 0; j < kP; j++) {
      auto *newLeaf = ArrayStorage::createForTest(&gc, kLeafSize);
      head->set(
          j + kNumFieldPtrIndices,
          HermesValue::encodeObjectValue(newLeaf),
          &gc);
    }
    if (i == 0) {
      headFollower = *head;
    } else if (i > kF) {
      // Remove the pointers to 30% of the leaf arrays in
      // headFollower.
      unsigned numToClear =
          static_cast<unsigned>(static_cast<double>(kP) * 0.3);
      for (unsigned k = 0; k < numToClear; k++) {
        headFollower->set(
            k + kNumFieldPtrIndices, HermesValue::encodeNullValue(), &gc);
      }
      headFollower = vmcast<ArrayStorage>(headFollower->at(kPrevFieldIndex));
    }
    if ((i % kGCFrequency) == 0) {
      rt.collect();

      GCBase::HeapInfo info;
      gc.getHeapInfo(info);
      double targetRate = gc.occupancyTarget();
      // The actual size may be rounded up by different amounts, depending on
      // the collector type: GenGC by page sizes, but GenGC can round by as much
      // as a full segment size.
      const size_t kRoundAmount = AlignedHeapSegment::maxSize();
      if (info.heapSize >= (2 * kRoundAmount)) {
        // Take the page size and heap size into account.  We round to the
        // page size, so the "true" desired heap size might be as much
        // as a page more or less.
        double occupancyRateLow = static_cast<double>(info.allocatedBytes) /
            static_cast<double>(info.heapSize + kRoundAmount);
        double occupancyRateHigh = static_cast<double>(info.allocatedBytes) /
            static_cast<double>(info.heapSize - kRoundAmount);
        // Assert that the actual occupancy is within +/- 5% of the
        // target.  Use the low and high estimates defined above appropriately.
        ASSERT_LE(0.95, occupancyRateHigh / targetRate);
        ASSERT_GE(1.05, occupancyRateLow / targetRate);
      }
    }
  }
}
#endif

// Heap shrinking is only implemented in NCGen at present.
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
TEST_F(GCSizingTest, TestHeapShrinks) {
  // If we do a collection with high live data, the heap grows; but then if we
  // do several with a low occupancy, the heap size shrinks again.

  GenGC &gc = rt.getHeap();

  GCScope topScope(&rt);

  // An array containing pointers to other arrays, 32KB each.
  auto spineArr = rt.makeMutableHandle<ArrayStorage>(nullptr);

  const size_t kArraysPerMB = 1024 * 1024 / (32 * 1024);
  // We can hold up to 100MB worth of arrays.
  spineArr = ArrayStorage::createForTest(&gc, 100 * kArraysPerMB);

  // A holder for intermediate values.
  auto tmpArr = rt.makeMutableHandle<ArrayStorage>(nullptr);

  // How many elements in an array to get to 32KB?  This assumes 8-byte
  // HermesValues in the arrays.
  const size_t kElemsPer32KB = 32 * 1024 / 8;
  // We want to reach 50 MB live, so we should get near the max heap size.
  for (unsigned i = 0; i < 50 * kArraysPerMB; i++) {
    tmpArr = ArrayStorage::createForTest(&gc, kElemsPer32KB);
    spineArr->set(i, HermesValue::encodeObjectValue(*tmpArr), &gc);
  }
  tmpArr = nullptr;

  // Now do an explicit full GC, so we will resize when we have 50MB live.
  rt.collect();

  GCBase::HeapInfo info;
  gc.getHeapInfo(info);

  gcheapsize_t expectedLargeHeap = 90 * 1000000;
  // We expect the size to reach at least 90M.
  EXPECT_LT(expectedLargeHeap, info.heapSize);

  // Now decrease the live data down to 10MB.
  for (unsigned i = 10 * kArraysPerMB; i < 50 * kArraysPerMB; i++) {
    spineArr->set(i, HermesValue::encodeNullValue(), &gc);
  }

  // Now do 5 full GCs; the heap size should start to approach 2X the live data;
  // we'll allow 3X to account for convergence, so 30 MB.
  rt.collect();
  rt.collect();
  rt.collect();
  rt.collect();
  rt.collect();
  gc.getHeapInfo(info);

  gcheapsize_t expectedSmallHeap = 30 * 1024 * 1024;
  EXPECT_GT(expectedSmallHeap, info.heapSize);
}

// This is not a member of the test class because we want to specify
// different parameters.
TEST(GCSizingMinHeapTest, TestHeapDoesNotShrinkPastMinSize) {
  // If we do a collection with high live data, the heap grows; but then if we
  // do several with a low occupancy, the heap size shrinks again.
  const gcheapsize_t kMinHeap = 40 * 1000000;
  const gcheapsize_t kMaxHeap = 100 * 1000000;
  auto runtime = DummyRuntime::create(GCConfig::Builder()
                                          .withMinHeapSize(kMinHeap)
                                          .withInitHeapSize(kMinHeap)
                                          .withMaxHeapSize(kMaxHeap)
                                          .build());
  DummyRuntime &rt = *runtime;

  GenGC &gc = rt.getHeap();

  GCScope topScope(&rt);

  // An array containing pointers to other arrays, 32KB each.
  auto spineArr = rt.makeMutableHandle<ArrayStorage>(nullptr);
  const size_t kArraysPerMB = 1024 * 1024 / (32 * 1024);
  // We can hold up to 100MB worth of arrays.
  spineArr = ArrayStorage::createForTest(&gc, 100 * kArraysPerMB);

  // A holder for intermediate values.
  auto tmpArr = rt.makeMutableHandle<ArrayStorage>(nullptr);

  // How many elements in an array to get to 32KB?  This assumes 8-byte
  // HermesValues in the arrays.
  const size_t kElemsPer32KB = 32 * 1024 / 8;

  // We want to reach 50 MB live, so we should get near the max heap size.
  for (unsigned i = 0; i < 50 * kArraysPerMB; i++) {
    tmpArr = ArrayStorage::createForTest(&gc, kElemsPer32KB);
    spineArr->set(i, HermesValue::encodeObjectValue(*tmpArr), &gc);
  }
  tmpArr = nullptr;

  // Now do an explicit full GC, so we will resize when we have 50MB live.
  rt.collect();

  GCBase::HeapInfo info;
  gc.getHeapInfo(info);

  const gcheapsize_t kExpectedLargeHeap = 90 * 1000000;
  // We expect the size to reach at least 90M.
  EXPECT_LT(kExpectedLargeHeap, info.heapSize);

  // Now decrease the live data down to 10MB.
  for (unsigned i = 10 * kArraysPerMB; i < 50 * kArraysPerMB; i++) {
    spineArr->set(i, HermesValue::encodeNullValue(), &gc);
  }

  // Now do 10 full GCs; the heap size should start to approach 2X the
  // live data, or 20MB.  But we've specified the minimum heap at
  // 40MB, so it should not shrink beyond that.
  for (unsigned i = 0; i < 10; i++) {
    rt.collect();
  }
  gc.getHeapInfo(info);

  EXPECT_GE(info.heapSize, kMinHeap);
}
#endif

} // namespace
