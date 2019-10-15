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
#include "LogSuccessStorageProvider.h"
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

const MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      Metadata() // Uninitialized
  };
  return MetadataTableForTests(storage);
}

struct GCLazySegmentNCTest : public ::testing::Test {
  ~GCLazySegmentNCTest() {
    oscompat::unset_test_vm_allocate_limit();
  }
};

using GCLazySegmentNCDeathTest = GCLazySegmentNCTest;

constexpr size_t kHeapSizeHint =
    AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;

const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);

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

constexpr size_t kHeapVA =
    AlignedStorage::size() * (GC::kYoungGenFractionDenom + kExtraSegments);

constexpr size_t kHeapVALimited = kHeapVA / 2 + AlignedStorage::size() - 1;

using SegmentCell = EmptyCell<
    AlignedHeapSegment::maxSize(),
    /* FixedSize */ true>;

/// We are able to materialize every segment.
TEST_F(GCLazySegmentNCTest, MaterializeAll) {
  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size();
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
}

/// Could not allocate every segment to cover the whole heap, but can allocate
/// enough segments for the used portion of the heap.
TEST_F(GCLazySegmentNCTest, MaterializeEnough) {
  auto provider = llvm::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(kGCConfig), kHeapVALimited);
  auto runtime =
      DummyRuntime::create(getMetadataTable(), kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size() / 4;
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
}

/// There is enough space in the old generation to perform a young gen
/// collection, but we could not materialize a segment, so we must do a full
/// collection instead.
TEST_F(GCLazySegmentNCTest, YoungGenNoMaterialize) {
  auto provider = llvm::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(kGCConfig), kHeapVALimited);
  auto runtime =
      DummyRuntime::create(getMetadataTable(), kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Unroot the first segment, which is now in the old generation, thus freeing
  // up space.
  rt.pointerRoots.erase(rt.pointerRoots.begin());

  auto fullGCBefore = rt.gc.numFullGCs();
  auto youngGCBefore = rt.gc.numYoungGCs();

  // Attempt to allocate a new cell in the young gen.  There is no space, and
  // the old gen has all its materialized segments filled, so we should attempt
  // a full collection.
  SegmentCell::create(rt);

  auto fullGCAfter = rt.gc.numFullGCs();
  auto youngGCAfter = rt.gc.numYoungGCs();

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
  auto provider = std::make_shared<LogSuccessStorageProvider>(
      DummyRuntime::defaultProvider(config));
  auto &counter = *provider;
  auto runtime = DummyRuntime::create(getMetadataTable(), config, provider);
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size() - 1;

  // Fill the Old Generation up with segments.
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::createLongLived(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  ASSERT_EQ(N + 1 + kExtraSegments, counter.numAllocated());
  ASSERT_EQ(0, rt.gc.numFullGCs());

  // Trigger a full collection, resize and one new segment to be materialised.
  roots.push_back(SegmentCell::createLongLived(rt));
  rt.pointerRoots.push_back(&roots.back());

  EXPECT_EQ(1, rt.gc.numFullGCs());
  EXPECT_EQ(N + 2 + kExtraSegments, counter.numAllocated());
}

/// We failed to materialize a segment that we needed to allocate in.
TEST_F(GCLazySegmentNCDeathTest, FailToMaterialize) {
  auto provider = llvm::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(kGCConfig), kHeapVALimited);
  auto runtime =
      DummyRuntime::create(getMetadataTable(), kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  EXPECT_OOM(SegmentCell::create(rt));
}

TEST_F(GCLazySegmentNCDeathTest, FailToMaterializeContinue) {
  auto provider = llvm::make_unique<LimitedStorageProvider>(
      DummyRuntime::defaultProvider(kGCConfig), kHeapVALimited);
  auto runtime =
      DummyRuntime::create(getMetadataTable(), kGCConfig, std::move(provider));
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  auto N = kHeapSizeHint / SegmentCell::size() / 2;
  for (size_t i = 0; i < N; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Discard one allocated cell.
  rt.pointerRoots.pop_back();
  roots.pop_back();

  ASSERT_EQ(0, rt.gc.numFailedSegmentMaterializations());
  ASSERT_EQ(0, rt.gc.numFullGCs());

  SegmentCell::create(rt);

  // Allocating a cell caused us to attempt to materialize a segment which
  // should fail.  We should however still be able to continue by making room
  // through a full collection.
  EXPECT_EQ(1, rt.gc.numFailedSegmentMaterializations());
  EXPECT_EQ(1, rt.gc.numFullGCs());
}

} // namespace

#endif
#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
