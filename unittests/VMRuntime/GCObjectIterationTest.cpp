/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "EmptyCell.h"
#include "VMRuntimeTestHelpers.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/HeapAlign.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(GCObjectIterationTest, ForAllObjsGetsAllObjects) {
  auto runtime = DummyRuntime::create(kTestGCConfigLarge);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();
  // For Hades, ensure that we iterate across multiple segments.
  constexpr size_t kLargeSize =
#ifdef HERMESVM_GC_MALLOC
      1024 * 1024
#else
      heapAlignSize((GC::maxNormalAllocationSize() / 3) * 2)
#endif
      ;
  // A smaller size, in the young generation.
  constexpr size_t kSmallSize = 80;
  using SmallCell = EmptyCell<kSmallSize>;
  using LargeCell = EmptyCell<kLargeSize>;
  struct : Locals {
    PinnedValue<LargeCell> large1;
    PinnedValue<LargeCell> large2;
    PinnedValue<SmallCell> small;
  } lv;
  DummyLocalsRAII lraii{rt, &lv};
  // Divide by 8 bytes per HermesValue to get elements.
  lv.large1 = LargeCell::create(rt);
  lv.large2 = LargeCell::create(rt);
  // Should move both to the old gen, in separate segments.
  rt.collect();
  lv.small = SmallCell::create(rt);

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
