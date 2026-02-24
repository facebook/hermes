/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "VMRuntimeTestHelpers.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/LimitedStorageProvider.h"

#include <deque>

using namespace hermes;
using namespace hermes::vm;

namespace {
struct GCLazySegmentNCTest : public ::testing::Test {};

using GCLazySegmentNCDeathTest = GCLazySegmentNCTest;

using SegmentCell = EmptyCell<FixedSizeHeapSegment::maxSize()>;
constexpr size_t kSegmentCellStorageSize = FixedSizeHeapSegment::storageSize();

constexpr size_t kHeapSizeHint = FixedSizeHeapSegment::maxSize() * 10;
const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);
constexpr size_t kHeapVA = FixedSizeHeapSegment::storageSize() * 10;
constexpr size_t kHeapVALimited =
    kHeapVA / 2 + FixedSizeHeapSegment::storageSize() - 1;

/// We are able to materialize every segment.
TEST_F(GCLazySegmentNCTest, MaterializeAll) {
  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  struct : Locals {
    PinnedValue<SegmentCell> handles[100]; // Sufficient for typical test cases
  } lv;
  DummyLocalsRAII lraii{rt, &lv};

  auto N = kHeapSizeHint / kSegmentCellStorageSize;
  for (size_t i = 0; i < N; ++i) {
    lv.handles[i] = SegmentCell::create(rt);
  }
}

/// Could not allocate every segment to cover the whole heap, but can allocate
/// enough segments for the used portion of the heap.
TEST_F(GCLazySegmentNCTest, MaterializeEnough) {
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), kHeapVALimited);
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  struct : Locals {
    PinnedValue<SegmentCell> handles[50]; // Sufficient for N/4 allocation
  } lv;
  DummyLocalsRAII lraii{rt, &lv};

  auto N = kHeapSizeHint / SegmentCell::size() / 4;
  for (size_t i = 0; i < N; ++i) {
    lv.handles[i] = SegmentCell::create(rt);
  }
}

} // namespace
#endif // !HERMESVM_GC_MALLOC
