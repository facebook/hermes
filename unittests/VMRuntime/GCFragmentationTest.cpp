/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"

#include <deque>

using namespace hermes::vm;

namespace {

TEST(GCFragmentationTest, TestCoalescing) {
  // Fill the heap with increasingly larger cells, in order to test
  // defragmentation code.
  static const size_t kNumSegments = 4;
  static const size_t kNumOGSegments = kNumSegments - 1;
  static const size_t kHeapSize = AlignedHeapSegment::maxSize() * kNumSegments;
  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSize);

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;

  using SixteenthCell = EmptyCell<AlignedHeapSegment::maxSize() / 16>;
  using EighthCell = EmptyCell<AlignedHeapSegment::maxSize() / 8>;
  using QuarterCell = EmptyCell<AlignedHeapSegment::maxSize() / 4>;

  {
    GCScope scope(rt);
    for (size_t i = 0; i < 16 * kNumOGSegments; i++)
      rt.makeHandle(SixteenthCell::create(rt));
  }

  // Hades needs a manually triggered full collection, since full collections
  // are started at the end of a YG GC.
#if defined(HERMESVM_GC_HADES) || defined(HERMESVM_GC_RUNTIME)
  rt.collect();
#endif

  {
    GCScope scope(rt);
    for (size_t i = 0; i < 8 * kNumOGSegments; i++)
      rt.makeHandle(EighthCell::create(rt));
  }

#if defined(HERMESVM_GC_HADES) || defined(HERMESVM_GC_RUNTIME)
  rt.collect();
#endif

  {
    GCScope scope(rt);
    for (size_t i = 0; i < 4 * kNumOGSegments; i++)
      rt.makeHandle(QuarterCell::create(rt));
  }
}

} // namespace
