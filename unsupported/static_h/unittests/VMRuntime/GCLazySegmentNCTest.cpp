/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes;
using namespace hermes::vm;

namespace {
struct GCLazySegmentNCTest : public ::testing::Test {};

using GCLazySegmentNCDeathTest = GCLazySegmentNCTest;

using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;

constexpr size_t kHeapSizeHint = AlignedHeapSegment::maxSize() * 10;
const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);
constexpr size_t kHeapVA = AlignedStorage::size() * 10;
constexpr size_t kHeapVALimited = kHeapVA / 2 + AlignedStorage::size() - 1;

/// We are able to materialize every segment.
TEST_F(GCLazySegmentNCTest, MaterializeAll) {
  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GCScope scope{rt};

  auto N = kHeapSizeHint / SegmentCell::size();
  for (size_t i = 0; i < N; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
}

/// Could not allocate every segment to cover the whole heap, but can allocate
/// enough segments for the used portion of the heap.
TEST_F(GCLazySegmentNCTest, MaterializeEnough) {
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), kHeapVALimited);
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  GCScope scope{rt};

  auto N = kHeapSizeHint / SegmentCell::size() / 4;
  for (size_t i = 0; i < N; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
}

} // namespace
#endif // !HERMESVM_GC_MALLOC
