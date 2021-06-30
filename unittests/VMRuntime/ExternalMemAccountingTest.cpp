/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GenGCHeapSegment.h"

#include <deque>

using namespace hermes::vm;

namespace {

namespace {
const size_t kMaxYoungGenSize = GenGCHeapSegment::maxSize();
} // namespace

using testhelpers::DummyObject;

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

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  // A cell a quarter of the young-gen size
  using QuarterYoungGenCell = EmptyCell<kMaxYoungGenSize / 4>;

  rt.makeHandle(QuarterYoungGenCell::create(rt));

  auto extObj = rt.makeHandle(DummyObject::create(&gc));
  // The 1/4 + 1/2 + 1/4 = 1, but the size of the DummyObject object itself
  // will cause the sum to exceed the YG size.
  // (If the length created is zero, the subsequent allocation does *not* cause
  // a collection, and the test fails.)
  extObj->acquireExtMem(&gc, kMaxYoungGenSize / 2);

  EXPECT_EQ(0, gc.numYoungGCs());
  EXPECT_EQ(0, gc.numFullGCs());

  if (GetParam()) {
    extObj->releaseExtMem(&gc);
  }

  rt.makeHandle(QuarterYoungGenCell::create(rt));

  EXPECT_EQ(GetParam() ? 0 : 1, gc.numYoungGCs());
  EXPECT_EQ(0, gc.numFullGCs());
}

TEST_P(ExtMemTests, ExtMemInOldByAllocTest) {
  // Ensure that the heap will be big enough for the young gen to accommodate
  // its full size, and the old gen is GenGC::kYoungGenFractionDenom - 1 times
  // that (so that YGSize = (YGSize + OGSize) / GenGC::kYoungGenFractionDenom).
  static const GCConfig kGCConfig =
      TestGCConfigFixedSize(kMaxYoungGenSize * GenGC::kYoungGenFractionDenom);

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  // Allocate GenGC::kYoungGenFractionDenom - 3 young-gen-sized objects, and do
  // a full collection to get them into the old gen, so that the old gen has 2
  // young-gen's worth of free space.
  using YoungGenCell = EmptyCell<kMaxYoungGenSize>;
  using HalfYoungGenCell = EmptyCell<kMaxYoungGenSize / 2>;

  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    rt.makeHandle(YoungGenCell::create(rt));
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
  auto extObj = rt.makeHandle(DummyObject::create(&gc));
  extObj->acquireExtMem(&gc, 3 * kMaxYoungGenSize / 2);

  // Get it in the old generation.
  rt.collect();
  EXPECT_EQ(ygs0 + 1, gc.numYoungGCs());
  EXPECT_EQ(2, gc.numFullGCs());

  if (GetParam()) {
    extObj->releaseExtMem(&gc);
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

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  // Allocate GenGC::kYoungGenFractionDenom - 3 young-gen-sized objects, and do
  // a full collection to get them into the old gen, so that the old gen has 2
  // young-gen's worth of free space.
  using YoungGenCell = EmptyCell<kMaxYoungGenSize>;

  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    rt.makeHandle(YoungGenCell::create(rt));
  }
  rt.collect();
  size_t ygs0 = gc.numYoungGCs();
  EXPECT_EQ(1, gc.numFullGCs());
  // The OG now has 2 free segments.

  // Now allocate an object with 1.5 young-gen-sizes of external memory directly
  // in the old generation.
  // (If the length is zero, instead, this test fails: the creates at the end
  // cause a YG collection rather than a full GC.)
  auto *extObj = DummyObject::createLongLived(&gc);
  extObj->acquireExtMem(&gc, 3 * kMaxYoungGenSize / 2);

  if (GetParam()) {
    extObj->releaseExtMem(&gc);
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

  auto runtime = DummyRuntime::create(gcConfig);
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

  using SegmentSizeCell = EmptyCell<GenGCHeapSegment::maxSize()>;

  // Allocate GenGC::kYoungGenFractionDenom - 3 segment-sized objects in
  // the old gen, so that the old gen has 2 segments worth of free
  // space.
  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 3; i++) {
    rt.makeHandle(SegmentSizeCell::createLongLived(rt));
  }

  // Now allocate an object with 2.5 segment-sizes of external memory.
  // After this, the effective size of the old gen should now be
  // greater than its current size.
  auto extObj = rt.makeHandle(DummyObject::createLongLived(&rt.getHeap()));
  extObj->acquireExtMem(&rt.getHeap(), 5 * GenGCHeapSegment::maxSize() / 2);

  // Now allocate a YG object, filling the YG.
  rt.makeHandle(SegmentSizeCell::create(rt));

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
  const auto kExtAllocSize = kOGSize - sizeof(DummyObject);
  ASSERT_GT(kExtAllocSize, kYGSize);

  const auto gcConfig = TestGCConfigFixedSize(kTotalSize);
  auto runtime = DummyRuntime::create(gcConfig);
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

  // Fill up the old generation.
  for (size_t i = 0; i < GenGC::kYoungGenFractionDenom - 1; ++i) {
    rt.makeHandle(SegmentCell::createLongLived(rt));
  }

  // Saturate the young generation.
  auto extObj = rt.makeHandle(DummyObject::create(&rt.getHeap()));
  extObj->acquireExtMem(&rt.getHeap(), kExtAllocSize);

  // Expect that a subsequent allocation should fail, with an OOM.
  EXPECT_OOM(DummyObject::create(&rt.getHeap()));
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
