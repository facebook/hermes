/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMESVM_GC_NONCONTIG_GENERATIONAL) && defined(NDEBUG)

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"

#include "hermes/VM/GenGCHeapSegment.h"

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

const GCConfig kGCConfig = TestGCConfigFixedSize(16 << 20);

const MetadataTableForTests getMetadataTable() {
  static const Metadata table[] = {Metadata()};
  return MetadataTableForTests(table);
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

  using HalfCell = EmptyCell<GenGCHeapSegment::maxSize() / 2>;

  // Allocate cells directly in the old generation.
  auto *cell1 = HalfCell::createLongLived(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell1));

  auto *cell2 = HalfCell::createLongLived(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell2));

  auto before = gc.getVMFootprintForTest();
  ASSERT_TRUE(before);

  // Make the pages dirty
  (void)cell1->touch();
  (void)cell2->touch();

  auto touched = gc.getVMFootprintForTest();
  ASSERT_TRUE(touched);

  rt.pointerRoots.erase(rt.pointerRoots.begin());

  // Collect should return the unused memory back to the OS.
  rt.collect();

  auto collected = gc.getVMFootprintForTest();
  ASSERT_TRUE(collected);

  EXPECT_LT(*before, *touched);
  EXPECT_GT(*touched, *collected);
#endif // _WINDOWS
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL, NDEBUG
