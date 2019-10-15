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
#include "hermes/VM/GC.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes::vm;

namespace {

MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      Metadata() // Uninitialized
  };
  return MetadataTableForTests(storage);
}

/// A cell size (in bytes) that is likely to be larger than the alignment
/// requirements on the size of the GC's young generation. So that when we try
/// and request the young gen be half the size of this cell, we have a
/// reasonably good chance that the the young gen ends up smaller than the cell.
static constexpr size_t kCellSize = 64 * 1024;

/// Allocating a fixed size cell that is too big for the young gen should cause
/// an OOM
TEST(GCOOMFixedSizeDeathTest, Test) {
  const size_t kHeapSizeHint = kCellSize * GC::kYoungGenFractionDenom / 2;
  using FixedCell = EmptyCell<kCellSize, /* FixedSize */ true>;

  auto runtime = DummyRuntime::create(
      getMetadataTable(), TestGCConfigFixedSize(kHeapSizeHint));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  const size_t kYGSize = gc.youngGenSize(kHeapSizeHint);
  const size_t kOGSizeHint = kHeapSizeHint - kYGSize;

  ASSERT_GT(FixedCell::size(), kYGSize);
  ASSERT_LE(FixedCell::size(), kOGSizeHint);

  // Fixed size cells should not be too big to fit in the young gen.
  EXPECT_OOM(FixedCell::create(rt));
}

// Allocating a variable sized cell that is too big for the young gen (but fits
// in the old gen) should be fine.
TEST(GCOOMVarSizeTest, Test) {
  const size_t kHeapSizeHint = kCellSize * GC::kYoungGenFractionDenom / 2;
  using VarCell = EmptyCell<kCellSize, /* FixedSize */ false>;

  auto runtime = DummyRuntime::create(
      getMetadataTable(), TestGCConfigFixedSize(kHeapSizeHint));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  const size_t kYGSize = gc.youngGenSize(kHeapSizeHint);
  const size_t kOGSizeHint = kHeapSizeHint - kYGSize;

  ASSERT_GT(VarCell::size(), kYGSize);
  ASSERT_LE(VarCell::size(), kOGSizeHint);

  // Even though this is too big to fit in the young gen, this should be okay,
  // because it will fit in the old gen.
  VarCell::create(rt);
}

TEST(GCOOMFragmentationDeathTest, Test) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;
  // Only one of these cells will fit into a segment, with the maximum amount of
  // space wasted in the segment.
  using AwkwardCell =
      EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1, /* FixedSize */ false>;

  auto runtime = DummyRuntime::create(
      getMetadataTable(), TestGCConfigFixedSize(kHeapSizeHint));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  // Ensure that the heap will be big enough for the YoungGen to accommodate its
  // full size, and the OldGen is segment aligned.
  const size_t kSegments = GC::kYoungGenFractionDenom;
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

TEST(GCEffectiveOOMDeathTest, UnitTest) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;

  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      TestGCConfigFixedSize(kHeapSizeHint)
          .rebuild()
          .withEffectiveOOMThreshold(3)
          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  // tick...
  gc.collect(/* canEffectiveOOM */ true);

  // tick...
  gc.collect(/* canEffectiveOOM */ true);

  // ...
  gc.collect(/* canEffectiveOOM */ false);

  // ...
  gc.collect(/* canEffectiveOOM */ false);

  // ...BOOM!
  EXPECT_OOM(gc.collect(/* canEffectiveOOM */ true));
}

TEST(GCEffectiveOOMDeathTest, IntegrationTest) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;
  using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;

  const unsigned kOOMThreshold = 3;
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      TestGCConfigFixedSize(kHeapSizeHint)
          .rebuild()
          .withEffectiveOOMThreshold(kOOMThreshold)
          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  std::deque<GCCell *> roots;

  // Fill up the heap.
  unsigned numCells = GC::kYoungGenFractionDenom;
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
TEST(GCOOMVALimitFullGCTest, Test) {
  const size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;

  using FullCell = EmptyCell<AlignedHeapSegment::maxSize()>;
  using HalfCell = EmptyCell<AlignedHeapSegment::maxSize() / 2>;

  const GCConfig config = TestGCConfigFixedSize(kHeapSizeHint);

  constexpr size_t kExtraSegments =
#ifdef HERMESVM_COMPRESSED_POINTERS
      // Allow one extra segment for compressed pointers cases.
      // This is a hack, LimitedStorageProvider doesn't know if its underlying
      // StorageProvider needs some excess storage for the runtime.
      1
#else
      0
#endif
      ;

  // Only space for two segments.
  auto provider = llvm::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(config),
      AlignedStorage::size() * (2 + kExtraSegments));

  auto runtime =
      DummyRuntime::create(getMetadataTable(), config, std::move(provider));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

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

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
