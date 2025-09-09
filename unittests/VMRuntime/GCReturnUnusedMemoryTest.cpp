/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMESVM_GC_HADES) && defined(NDEBUG) && \
    !defined(HERMESVM_ALLOW_HUGE_PAGES)

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "VMRuntimeTestHelpers.h"

#include "hermes/VM/AlignedHeapSegment.h"

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

const GCConfig kGCConfig = TestGCConfigFixedSize(16 << 20);

TEST(GCReturnUnusedMemoryTest, CollectReturnsFreeMemory) {
  // TODO(T40416012) Re-enable this test when vm_unused is fixed.
  // Skip this test in Windows because vm_unused has a no-op implementation.
#ifndef _WINDOWS
  // Use an mmap-based storage for this test.
  std::unique_ptr<StorageProvider> provider = StorageProvider::mmapProvider();
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();

  using SemiCell = EmptyCell<FixedSizeHeapSegment::maxSize() * 8 / 10>;

  llvh::ErrorOr<size_t> before = 0;
  {
    struct : Locals {
      PinnedValue<SemiCell> cell1;
      PinnedValue<SemiCell> cell2;
      PinnedValue<SemiCell> cell3;
    } lv;
    DummyLocalsRAII lraii{rt, &lv};
    // Allocate cells directly in the old generation.
    lv.cell1 = SemiCell::createLongLived(rt);
    lv.cell2 = SemiCell::createLongLived(rt);
    lv.cell3 = SemiCell::createLongLived(rt);

    before = gc.getVMFootprintForTest();
    ASSERT_TRUE(before);

    // Make the pages dirty
    (void)lv.cell1->touch();
    (void)lv.cell2->touch();
  }

  auto touched = gc.getVMFootprintForTest();
  ASSERT_TRUE(touched);

  // Collect should return the unused memory back to the OS.
  rt.collect();
  // Hades can only return memory after a compaction. The very first
  // collection will just free up the originally allocated memory. This
  // collection will identify the segment to compact and prepare it.
  rt.collect();
  // This collection will actually compact the segment.
  rt.collect();

  auto collected = gc.getVMFootprintForTest();
  ASSERT_TRUE(collected);

  EXPECT_LE(*before, *touched);
  EXPECT_GT(*touched, *collected);
#endif // _WINDOWS
}

} // namespace

#endif
