/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
  if (GC::maxAllocationSize() == std::numeric_limits<uint32_t>::max()) {
    // This test won't work if there is no limit on allocation sizes.
    return;
  }
  using SuperSegmentCell = EmptyCell<GC::maxAllocationSize() * 2>;
  auto runtime = DummyRuntime::create(getMetadataTable(), kTestGCConfig);
  EXPECT_OOM(SuperSegmentCell::create(*runtime));
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

  auto runtime = DummyRuntime::create(
      getMetadataTable(), TestGCConfigFixedSize(kHeapSizeHint));
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

TEST(GCOOMDeathTest, Fragmentation) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;
  // Only one of these cells will fit into a segment, with the maximum amount of
  // space wasted in the segment.
  using AwkwardCell = EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1>;

  auto runtime = DummyRuntime::create(
      getMetadataTable(), TestGCConfigFixedSize(kHeapSizeHint));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // Ensure that the heap will be big enough for the YoungGen to accommodate its
  // full size, and the OldGen is segment aligned.
  const size_t kSegments = GenGC::kYoungGenFractionDenom;
  ASSERT_EQ(kSegments * AlignedHeapSegment::maxSize(), gc.size());

  std::deque<GCCell *> roots;

  // Fill each segment in the heap with an awkward cell.
  for (size_t i = 0; i < kSegments; ++i) {
    roots.push_back(AwkwardCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Now there isn't a contiguous space big enough to fit an awkward cell, we
  // expect the GC to OOM when we try.
  EXPECT_OOM(AwkwardCell::create(rt));
}

TEST(GCOOMDeathTest, Effective) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;

  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      TestGCConfigFixedSize(kHeapSizeHint)
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
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      TestGCConfigFixedSize(kHeapSizeHint)
          .rebuild()
          .withEffectiveOOMThreshold(kOOMThreshold)
          .build());
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  std::deque<GCCell *> roots;

  // Fill up the heap.
  unsigned numCells = GenGC::kYoungGenFractionDenom;
  for (unsigned i = 0; i < numCells; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
  ASSERT_EQ(0, gc.numFullGCs());

  const auto oneForOne = [&rt, &gc, &roots]() {
    auto fullGCsBefore = gc.numFullGCs();
    rt.pointerRoots.erase(rt.pointerRoots.begin());

    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());

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

  auto runtime =
      DummyRuntime::create(getMetadataTable(), config, std::move(provider));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  std::deque<GCCell *> roots;

  roots.push_back(FullCell::createLongLived(rt));
  rt.pointerRoots.push_back(&roots.back());

  roots.push_back(HalfCell::create(rt));
  rt.pointerRoots.push_back(&roots.back());

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
