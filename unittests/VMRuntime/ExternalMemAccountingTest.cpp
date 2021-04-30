/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "ExtStringForTest.h"
#include "TestHelpers.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GenGCHeapSegment.h"

#include <deque>

using namespace hermes::vm;

namespace {

namespace {
const size_t kMaxYoungGenSize = GenGCHeapSegment::maxSize();
} // namespace

/// A parameterized test class, for a set of tests that allocate
/// external memory, and then may optionally, controlled by the bool
/// parameter, delete it again.
class ExtMemTests : public ::testing::TestWithParam<bool> {};

TEST_P(ExtMemTests, ExtMemInYoungTest) {
  // If we allocate an obj with enough external memory in the young gen,
  // it causes collection to happen earlier.

  // Ensure that the heap will be big enough for the young gen to accommodate
  // its full size, and the old gen is GenGC::kYoungGenFractionDenom - 1 times
  // that (so that YGSize = (YGSize + OGSize) / GenGC::kYoungGenFractionDenom).
  static const GCConfig kGCConfig =
      TestGCConfigFixedSize(kMaxYoungGenSize * GenGC::kYoungGenFractionDenom);

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // A cell a quarter of the young-gen size
  using QuarterYoungGenCell = EmptyCell<kMaxYoungGenSize / 4>;

  std::deque<GCCell *> roots;

  roots.push_back(QuarterYoungGenCell::create(rt));
  rt.pointerRoots.push_back(&roots.back());

  // The 1/4 + 1/2 + 1/4 = 1, but the size of the ExtStringForTest object itself
  // will cause the sum to exceed the YG size.
  // (If the length created is zero, the subsequent allocation does *not* cause
  // a collection, and the test fails.)
  roots.push_back(ExtStringForTest::create(rt, kMaxYoungGenSize / 2));
  rt.pointerRoots.push_back(&roots.back());

  EXPECT_EQ(0, gc.numYoungGCs());
  EXPECT_EQ(0, gc.numFullGCs());

  if (GetParam()) {
    vmcast<ExtStringForTest>(roots.back())->releaseMem(&gc);
  }

  roots.push_back(QuarterYoungGenCell::create(rt));
  rt.pointerRoots.push_back(&roots.back());

  EXPECT_EQ(GetParam() ? 0 : 1, gc.numYoungGCs());
  EXPECT_EQ(0, gc.numFullGCs());
}

TEST_P(ExtMemTests, ExtMemInOldByAllocTest) {
  // Ensure that the heap will be big enough for the young gen to accommodate
  // its full size, and the old gen is GenGC::kYoungGenFractionDenom - 1 times
  // that (so that YGSize = (YGSize + OGSize) / GenGC::kYoungGenFractionDenom).
  static const GCConfig kGCConfig =
      TestGCConfigFixedSize(kMaxYoungGenSize * GenGC::kYoungGenFractionDenom);

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // Allocate GenGC::kYoungGenFractionDenom - 3 young-gen-sized objects, and do
  // a full collection to get them into the old gen, so that the old gen has 2
  // young-gen's worth of free space.
  using YoungGenCell = EmptyCell<kMaxYoungGenSize>;
  using HalfYoungGenCell = EmptyCell<kMaxYoungGenSize / 2>;

  std::deque<GCCell *> roots;

  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    roots.push_back(YoungGenCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
  rt.collect();
  size_t ygs0 = gc.numYoungGCs();
  EXPECT_EQ(1, gc.numFullGCs());
  // The OG now has 2 free segments.

  // Allocating a young-gen-size object; should not do any collections.
  // We do not keep this as a root.
  YoungGenCell::create(rt);
  EXPECT_EQ(ygs0, gc.numYoungGCs());
  EXPECT_EQ(1, gc.numFullGCs());

  // Now do a half-young-gen object (again, not rooted). We should get a
  // young-gen collection, and then an allocation.
  HalfYoungGenCell::create(rt);
  EXPECT_EQ(ygs0 + 1, gc.numYoungGCs());
  EXPECT_EQ(1, gc.numFullGCs());

  // Now allocate an object with 1.5 young-gen-sizes of external memory.
  // (If the length is zero, instead, this test fails: the creates at the end
  // cause a YG collection rather than a full GC.)
  roots.push_back(ExtStringForTest::create(rt, 3 * kMaxYoungGenSize / 2));
  rt.pointerRoots.push_back(&roots.back());

  // Get it in the old generation.
  rt.collect();
  EXPECT_EQ(ygs0 + 1, gc.numYoungGCs());
  EXPECT_EQ(2, gc.numFullGCs());

  if (GetParam()) {
    vmcast<ExtStringForTest>(roots.back())->releaseMem(&gc);
  }

  // Now we do allocations that would cause a YG collection.  If we haven't
  // deleted the external memory, we should get a full GC instead,
  // because the YG won't fit.  If we did delete the external memory,
  // we get a young-gen collection.
  YoungGenCell::create(rt);
  YoungGenCell::create(rt);
  if (GetParam()) {
    EXPECT_EQ(ygs0 + 2, gc.numYoungGCs());
    EXPECT_EQ(2, gc.numFullGCs());
  } else {
    EXPECT_EQ(ygs0 + 1, gc.numYoungGCs());
    EXPECT_EQ(3, gc.numFullGCs());
  }
}

TEST_P(ExtMemTests, ExtMemInOldDirectTest) {
  // Ensure that the heap will be big enough for the young gen to accommodate
  // its full size, and the old gen is GenGC::kYoungGenFractionDenom - 1 times
  // that (so that YGSize = (YGSize + OGSize) / GenGC::kYoungGenFractionDenom).
  static const GCConfig kGCConfig =
      TestGCConfigFixedSize(kMaxYoungGenSize * GenGC::kYoungGenFractionDenom);

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // Allocate GenGC::kYoungGenFractionDenom - 3 young-gen-sized objects, and do
  // a full collection to get them into the old gen, so that the old gen has 2
  // young-gen's worth of free space.
  using YoungGenCell = EmptyCell<kMaxYoungGenSize>;

  std::deque<GCCell *> roots;

  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    roots.push_back(YoungGenCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
  rt.collect();
  size_t ygs0 = gc.numYoungGCs();
  EXPECT_EQ(1, gc.numFullGCs());
  // The OG now has 2 free segments.

  // Now allocate an object with 1.5 young-gen-sizes of external memory directly
  // in the old generation.
  // (If the length is zero, instead, this test fails: the creates at the end
  // cause a YG collection rather than a full GC.)
  ExtStringForTest *extString =
      ExtStringForTest::createLongLived(rt, 3 * kMaxYoungGenSize / 2);

  if (GetParam()) {
    extString->releaseMem(&gc);
  }

  // Now we do allocations that would cause a collection.  If we did
  // not delete the external memory, we should get a full GC instead,
  // because the YG won't fit.  If we did delete the external memory,
  // we should get a young-gen collection.
  YoungGenCell::create(rt);
  YoungGenCell::create(rt);
  if (GetParam()) {
    EXPECT_EQ(ygs0 + 1, gc.numYoungGCs());
    EXPECT_EQ(1, gc.numFullGCs());
  } else {
    EXPECT_EQ(ygs0, gc.numYoungGCs());
    EXPECT_EQ(2, gc.numFullGCs());
  }
}

INSTANTIATE_TEST_CASE_P(ExtMemTests, ExtMemTests, testing::Bool());

TEST(ExtMemNonParamTests, ExtMemDoesNotBreakFullGC) {
  // Ensure that the initial heap will be big enough for the young gen
  // to accommodate its full size, and the old gen is
  // GenGC::kYoungGenFractionDenom - 1 times that (so that YGSize =
  // (YGSize + OGSize) / GenGC::kYoungGenFractionDenom).  We want the max
  // size to be considerably larger than this.
  const size_t kInitSize =
      GenGCHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;
  GCConfig gcConfig = GCConfig::Builder(kTestGCConfigBuilder)
                          .withInitHeapSize(kInitSize)
                          .withMaxHeapSize(kInitSize * 4)
                          .build();

  auto runtime = DummyRuntime::create(getMetadataTable(), gcConfig);
  DummyRuntime &rt = *runtime;

  using SegmentSizeCell = EmptyCell<GenGCHeapSegment::maxSize()>;

  std::deque<GCCell *> roots;

  // Allocate GenGC::kYoungGenFractionDenom - 3 segment-sized objects in
  // the old gen, so that the old gen has 2 segments worth of free
  // space.
  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    roots.push_back(SegmentSizeCell::createLongLived(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Now allocate an object with 2.5 segment-sizes of external memory.
  // After this, the effective size of the old gen should now be
  // greater than its current size.
  roots.push_back(ExtStringForTest::createLongLived(
      rt, 5 * GenGCHeapSegment::maxSize() / 2));
  rt.pointerRoots.push_back(&roots.back());

  // Now allocate a YG object, filling the YG.
  roots.push_back(SegmentSizeCell::create(rt));
  rt.pointerRoots.push_back(&roots.back());

  // This GC should succeed: the total live memory, including the
  // external memory, is much less than the max heap size.  We don't
  // test any results here, beyond the GC completing successfully.
  // (At one point, there was a bug that caused it to OOM in this
  // situation -- this is a regression test.)
  rt.collect();
}

TEST(ExtMemNonParamDeathTest, SaturateYoungGen) {
  // Fill the heap up to such an extent that the YoungGen is forced to hold a
  // cell with an external allocation larger than the YoungGen's own size.

  // The GC size is arranged so that the old gen is the smallest it can be for
  // a young gen that is as large as it can be.
  const auto kTotalSize = kMaxYoungGenSize * GenGC::kYoungGenFractionDenom;
  const auto kYGSize = kMaxYoungGenSize;
  const auto kOGSize = kTotalSize - kYGSize;

  // A segment-sized cell, to fill out the generations.
  using SegmentCell = EmptyCell<GenGCHeapSegment::maxSize()>;

  // An external allocation size that will certainly saturate the young
  // generation.
  const auto kExtAllocSize = kOGSize - sizeof(ExtStringForTest);
  ASSERT_GT(kExtAllocSize, kYGSize);

  const auto gcConfig = TestGCConfigFixedSize(kTotalSize);
  auto runtime = DummyRuntime::create(getMetadataTable(), gcConfig);
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  // Fill up the old generation.
  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 1; ++i) {
    roots.push_back(SegmentCell::createLongLived(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Saturate the young generation.
  roots.push_back(ExtStringForTest::create(rt, kExtAllocSize));
  rt.pointerRoots.push_back(&roots.back());

  // Expect that a subsequent allocation should fail, with an OOM.
  EXPECT_OOM(ExtStringForTest::create(rt, kExtAllocSize));
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
