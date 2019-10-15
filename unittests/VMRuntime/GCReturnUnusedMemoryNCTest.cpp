/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMESVM_GC_NONCONTIG_GENERATIONAL) && defined(NDEBUG)

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "Footprint.h"
#include "TestHelpers.h"

#include "hermes/VM/AlignedHeapSegment.h"

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

constexpr size_t FAILED = SIZE_MAX;
const size_t kPageSize = hermes::oscompat::page_size();

const GCConfig kGCConfig = TestGCConfigFixedSize(16 << 20);

const MetadataTableForTests getMetadataTable() {
  static const Metadata table[] = {Metadata()};
  return MetadataTableForTests(table);
}

/// Approximate the dirty memory footprint of the GC's heap.  Note that on
/// linux, this does not return the number of dirty pages in the heap, but
/// instead returns a number that goes up if pages are dirtied, and goes down
/// if pages are cleaned.
///
/// The reason for this is that the implementation relies upon regionFootprint,
/// which provides the footprint of the memory mapping containing the requested
/// range, and linux often coallesces adjacent mappings, causing the algorithm
/// to double count dirty pages in segments that share mappings.
size_t gcRegionFootprint(const GC &gc) {
  size_t footprint = 0;
  for (AlignedHeapSegment *seg : gc.segmentIndex()) {
    const size_t segFootprint = regionFootprint(seg->start(), seg->hiLim());
    if (segFootprint == FAILED) {
      return FAILED;
    }

    footprint += segFootprint;
  }

  return footprint;
}

TEST(GCReturnUnusedMemoryNCTest, CollectReturnsFreeMemory) {
  // TODO(T40416012) Re-enable this test when vm_unused is fixed.
  // Skip this test in Windows because vm_unused has a no-op implementation.
#ifndef _WINDOWS
  // Use an mmap-based storage for this test.
  std::unique_ptr<StorageProvider> provider = StorageProvider::mmapProvider();
  auto runtime =
      DummyRuntime::create(getMetadataTable(), kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  using HalfCell = EmptyCell<AlignedHeapSegment::maxSize() / 2>;
  ASSERT_EQ(0, HalfCell::size() % kPageSize);

  // Allocate cells directly in the old generation.
  auto *cell1 = HalfCell::createLongLived(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell1));

  auto *cell2 = HalfCell::createLongLived(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell2));

  size_t before = gcRegionFootprint(gc);
  ASSERT_NE(before, FAILED);

  // Make the pages dirty
  (void)cell1->touch();
  (void)cell2->touch();

  size_t touched = gcRegionFootprint(gc);
  ASSERT_NE(touched, FAILED);

  rt.pointerRoots.erase(rt.pointerRoots.begin());

  // Collect should return the unused memory back to the OS.
  gc.collect();

  size_t collected = gcRegionFootprint(gc);
  ASSERT_NE(collected, FAILED);

  EXPECT_LT(before, touched);
  EXPECT_GT(touched, collected);
#endif // _WINDOWS
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL, NDEBUG
