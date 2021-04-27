/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
#ifndef NDEBUG

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GenGCHeapSegment.h"
#include "hermes/VM/HeapAlign.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

const MetadataTableForTests getMetadataTable() {
  static const Metadata table[] = {Metadata()};
  return MetadataTableForTests(table);
}

TEST(GCObjectIterationTest, ForAllObjsGetsAllObjects) {
  auto runtime = DummyRuntime::create(getMetadataTable(), kTestGCConfigLarge);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();

  // 2/3 the size of a segment.
  constexpr size_t kLargeSize =
      heapAlignSize((GenGCHeapSegment::maxSize() / 3) * 2);
  using LargeCell = VarSizedEmptyCell<kLargeSize>;
  // Divide by 8 bytes per HermesValue to get elements.
  GCCell *largeCell0 = LargeCell::create(rt);
  rt.pointerRoots.push_back(&largeCell0);
  GCCell *largeCell1 = LargeCell::create(rt);
  rt.pointerRoots.push_back(&largeCell1);
  // Should move both to the old gen, in separate segments.
  rt.collect();
  // A smaller size, in the young generation.
  constexpr size_t kSmallSize = 80;
  using SmallCell = EmptyCell<kSmallSize>;
  GCCell *smallCell = SmallCell::create(rt);
  rt.pointerRoots.push_back(&smallCell);

  size_t num = 0;
  size_t sizeSum = 0;
  gc.forAllObjs([&num, &sizeSum](GCCell *cell) {
    num++;
    sizeSum += cell->getAllocatedSize();
  });
  EXPECT_EQ(3, num);
  EXPECT_EQ(2 * kLargeSize + kSmallSize, sizeSum);
}

} // namespace

#endif // !NDEBUG
#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
