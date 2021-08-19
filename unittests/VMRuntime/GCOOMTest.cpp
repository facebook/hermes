/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes::vm;

namespace {

TEST(GCOOMDeathTest, SuperSegment) {
  using SuperSegmentCell = EmptyCell<GC::maxAllocationSize() * 2>;
  auto runtime = DummyRuntime::create(kTestGCConfig);
  EXPECT_OOM(SuperSegmentCell::create(*runtime));
}

static void exceedMaxHeap(
    GCConfig::Builder baseConfig = kTestGCConfigBaseBuilder) {
  static constexpr size_t kSegments = 10;
  static constexpr size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * kSegments;
  // Only one of these cells will fit into a segment, with the maximum amount of
  // space wasted in the segment.
  using AwkwardCell = EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1>;

  auto runtime =
      DummyRuntime::create(TestGCConfigFixedSize(kHeapSizeHint, baseConfig));
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

  // Exceed the maximum size of the heap. Note we need 2 extra segments instead
  // of just one because Hades can sometimes hide the memory for a segment
  // during compaction.
  for (size_t i = 0; i < kSegments + 2; ++i)
    rt.makeHandle(AwkwardCell::create(rt));
}

TEST(GCOOMDeathTest, Fragmentation) {
  EXPECT_OOM(exceedMaxHeap());
}

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
/// A cell size (in bytes) that is likely to be larger than the alignment
/// requirements on the size of the GC's young generation. So that when we try
/// and request the young gen be half the size of this cell, we have a
/// reasonably good chance that the the young gen ends up smaller than the cell.
static constexpr size_t kCellSize = 256 * 1024;

// Allocating a variable sized cell that is too big for the young gen (but fits
// in the old gen) should be fine.
TEST(GCOOMTest, VarSize) {
  const size_t kHeapSizeHint = kCellSize * GenGC::kYoungGenFractionDenom / 2;
  using VarCell = EmptyCell<kCellSize>;

  auto runtime = DummyRuntime::create(TestGCConfigFixedSize(kHeapSizeHint));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  const size_t kYGSize = gc.youngGenSize(kHeapSizeHint);
  const size_t kOGSizeHint = kHeapSizeHint - kYGSize;

  ASSERT_GT(VarCell::size(), kYGSize);
  ASSERT_LE(VarCell::size(), kOGSizeHint);

  // Even though this is too big to fit in the young gen, this should be okay,
  // because it will fit in the old gen.
  VarCell::create(rt);
}

TEST(GCOOMDeathTest, Effective) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;

  auto runtime = DummyRuntime::create(TestGCConfigFixedSize(kHeapSizeHint)
                                          .rebuild()
                                          .withEffectiveOOMThreshold(3)
                                          .build());
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // tick...
  gc.collect("test", /* canEffectiveOOM */ true);

  // tick...
  gc.collect("test", /* canEffectiveOOM */ true);

  // ...
  gc.collect("test", /* canEffectiveOOM */ false);

  // ...
  gc.collect("test", /* canEffectiveOOM */ false);

  // ...BOOM!
  EXPECT_OOM(gc.collect("test", /* canEffectiveOOM */ true));
}

TEST(GCOOMDeathTest, EffectiveIntegration) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;
  using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;

  const unsigned kOOMThreshold = 3;
  auto runtime =
      DummyRuntime::create(TestGCConfigFixedSize(kHeapSizeHint)
                               .rebuild()
                               .withEffectiveOOMThreshold(kOOMThreshold)
                               .build());
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  // Fill up the heap.
  unsigned numCells = GenGC::kYoungGenFractionDenom;
  for (unsigned i = 0; i < numCells - 1; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }

  // Create one element that can be removed later
  auto marker = scope.createMarker();
  rt.makeHandle(SegmentCell::create(rt));

  ASSERT_EQ(0, gc.numFullGCs());

  const auto oneForOne = [&scope, &marker, &rt, &gc]() {
    auto fullGCsBefore = gc.numFullGCs();
    scope.flushToMarker(marker);
    rt.makeHandle(SegmentCell::create(rt));

    EXPECT_EQ(fullGCsBefore + 1, gc.numFullGCs());
  };

  // Keep trading one object for another in a full heap.  Each iteration causing
  // another full collection to make room.
  for (unsigned i = 0; i < kOOMThreshold - 1; ++i) {
    oneForOne();
  }

  // The last allocation should cause us to effectively OOM.
  EXPECT_OOM(oneForOne());
}

/// Even if we run out of virtual memory during a full collection, we should
/// be able to finish the GC.  This is a regression test as NCGen used to
/// trigger an OOM if it could not materialise segments in the OldGen, during
/// full collection, due to VA exhaustion.
TEST(GCOOMTest, VALimitFullGC) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;

  using FullCell = EmptyCell<AlignedHeapSegment::maxSize()>;
  using HalfCell = EmptyCell<AlignedHeapSegment::maxSize() / 2>;

  const GCConfig config = TestGCConfigFixedSize(kHeapSizeHint);

  // Only space for two segments.
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), AlignedStorage::size() * 2);

  auto runtime = DummyRuntime::create(config, std::move(provider));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  GCScope scope{&rt};
  runtime->makeHandle(FullCell::createLongLived(rt));
  runtime->makeHandle(HalfCell::create(rt));

  HalfCell::create(rt);

  // The heap has now filled the materializable segments.  Allocating the next
  // HalfCell should result in the dead HalfCell getting collected, with the
  // new allocation taking its place.  It should not OOM.
  ASSERT_EQ(0, gc.numGCs());
  HalfCell::create(rt);
  EXPECT_EQ(0, gc.numYoungGCs());
  EXPECT_EQ(1, gc.numFullGCs());
}
#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL

} // namespace

#endif
