/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
#ifndef NDEBUG

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GenGCHeapSegment.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes;
using namespace hermes::vm;

namespace {
struct GCLazySegmentNCTest : public ::testing::Test {
  ~GCLazySegmentNCTest() {
    oscompat::unset_test_vm_allocate_limit();
  }
};

using GCLazySegmentNCDeathTest = GCLazySegmentNCTest;

constexpr size_t kHeapSizeHint =
    GenGCHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;

const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);

constexpr size_t kHeapVA =
    AlignedStorage::size() * (GenGC::kYoungGenFractionDenom);

constexpr size_t kHeapVALimited = kHeapVA / 2 + AlignedStorage::size() - 1;

using SegmentCell = EmptyCell<GenGCHeapSegment::maxSize()>;

/// We are able to materialize every segment.
TEST_F(GCLazySegmentNCTest, MaterializeAll) {
  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

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
  GCScope scope{&rt};

  auto N = kHeapSizeHint / SegmentCell::size() / 4;
  for (size_t i = 0; i < N; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
}

/// There is enough space in the old generation to perform a young gen
/// collection, but we could not materialize a segment, so we must do a full
/// collection instead.
TEST_F(GCLazySegmentNCTest, YoungGenNoMaterialize) {
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), kHeapVALimited);
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};
  GenGC &gc = rt.getHeap();

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  MutableHandle<SegmentCell> oldGenHandle =
      rt.makeMutableHandle(SegmentCell::create(rt));
  for (size_t i = 1; i < N; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }

  // Unroot the first segment, which is now in the old generation, thus freeing
  // up space.
  oldGenHandle.clear();

  auto fullGCBefore = gc.numFullGCs();
  auto youngGCBefore = gc.numYoungGCs();

  // Attempt to allocate a new cell in the young gen.  There is no space, and
  // the old gen has all its materialized segments filled, so we should attempt
  // a full collection.
  SegmentCell::create(rt);

  auto fullGCAfter = gc.numFullGCs();
  auto youngGCAfter = gc.numYoungGCs();

  EXPECT_EQ(youngGCBefore, youngGCAfter);
  EXPECT_EQ(fullGCBefore + 1, fullGCAfter);
}

/// Regression (T36931579):  At one time, when the OldGen ran out of space, it
/// would materialize all the segments up to its max size before triggering a
/// full collection and cleaning them up.  This test ensures we don't
/// materialize segments redundantly like that.
TEST_F(GCLazySegmentNCTest, OldGenAllocMaterialize) {
  const GCConfig config = GCConfig::Builder()
                              .withInitHeapSize(kHeapSizeHint)
                              .withMaxHeapSize(kHeapSizeHint * 2)
                              .build();
  auto provider = DummyRuntime::defaultProvider();
  auto &counter = *provider;
  auto runtime = DummyRuntime::create(config, std::move(provider));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  auto N = kHeapSizeHint / SegmentCell::size() - 1;

  // Fill the Old Generation up with segments.
  for (size_t i = 0; i < N; ++i) {
    rt.makeHandle(SegmentCell::createLongLived(rt));
  }

  ASSERT_EQ(N + 1, counter.numSucceededAllocs());
  ASSERT_EQ(0, gc.numFullGCs());

  // Trigger a full collection, resize and one new segment to be materialised.
  rt.makeHandle(SegmentCell::createLongLived(rt));

  EXPECT_EQ(1, gc.numFullGCs());
  EXPECT_EQ(N + 2, counter.numSucceededAllocs());
}

/// We failed to materialize a segment that we needed to allocate in.
TEST_F(GCLazySegmentNCDeathTest, FailToMaterialize) {
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), kHeapVALimited);
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  for (size_t i = 0; i < N; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }

  EXPECT_OOM(SegmentCell::create(rt));
}

TEST_F(GCLazySegmentNCDeathTest, FailToMaterializeContinue) {
  auto provider = std::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(), kHeapVALimited);
  auto runtime = DummyRuntime::create(kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  for (size_t i = 0; i < N - 1; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
  // Allocate and discard one cell.
  SegmentCell::create(rt);

  ASSERT_EQ(0, gc.numFailedSegmentMaterializations());
  ASSERT_EQ(0, gc.numFullGCs());

  SegmentCell::create(rt);

  // Allocating a cell caused us to attempt to materialize a segment which
  // should fail.  We should however still be able to continue by making room
  // through a full collection.
  EXPECT_EQ(1, gc.numFailedSegmentMaterializations());
  EXPECT_EQ(1, gc.numFullGCs());
}

} // namespace

#endif
#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
