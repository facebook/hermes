/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/StorageProvider.h"

using namespace hermes::vm;

namespace {

struct CardObjectBoundaryNCTest : public ::testing::Test {
  CardObjectBoundaryNCTest();

 protected:
  /// Simulate old-gen allocation's update of card boundaries.
  void *alloc(size_t sz) {
    AllocResult res = segment.alloc(sz);
    EXPECT_TRUE(res.success);
    char *resPtr = reinterpret_cast<char *>(res.ptr);
    char *nextPtr = segment.level();
    if (boundary.address() < nextPtr) {
      segment.cardTable().updateBoundaries(&boundary, resPtr, nextPtr);
    }
    return res.ptr;
  }

  std::unique_ptr<StorageProvider> provider;
  AlignedHeapSegment segment;
  CardTable::Boundary boundary;

  size_t segStartIndex;
};

CardObjectBoundaryNCTest::CardObjectBoundaryNCTest()
    : provider(StorageProvider::mmapProvider()),
      segment(std::move(AlignedStorage::create(provider.get()).get())),
      boundary(segment.cardTable().nextBoundary(segment.start())),
      segStartIndex(boundary.index()) {
  segment.growToLimit();
}

TEST_F(CardObjectBoundaryNCTest, NextBoundaryInitial) {
  auto actual = segment.cardTable().nextBoundary(segment.start());
  EXPECT_EQ(segment.start(), actual.address());
}

TEST_F(CardObjectBoundaryNCTest, NextBoundaryIncrement) {
  auto actual = segment.cardTable().nextBoundary(segment.start() + HeapAlign);
  EXPECT_EQ(segment.start() + CardTable::kCardSize, actual.address());
}

TEST_F(CardObjectBoundaryNCTest, NextBoundaryOnBoundary) {
  auto actual =
      segment.cardTable().nextBoundary(segment.start() + CardTable::kCardSize);
  EXPECT_EQ(segment.start() + CardTable::kCardSize, actual.address());
}

TEST_F(CardObjectBoundaryNCTest, FirstAlloc) {
  void *res = alloc(cellSize<GCCell>());
  EXPECT_EQ(segment.start(), reinterpret_cast<char *>(res));
  EXPECT_EQ(res, segment.cardTable().firstObjForCard(segStartIndex));
  EXPECT_EQ(segment.start() + CardTable::kCardSize, boundary.address());
}

TEST_F(CardObjectBoundaryNCTest, CrossingSmallAlloc) {
  (void)alloc(cellSize<GCCell>());
  void *res1 = alloc(CardTable::kCardSize);
  EXPECT_EQ(res1, segment.cardTable().firstObjForCard(segStartIndex + 1));
  EXPECT_EQ(segment.start() + 2 * CardTable::kCardSize, boundary.address());
}

TEST_F(CardObjectBoundaryNCTest, CrossingSmallAllocAtCardStart) {
  // Do an alloc to get to the start of the next card.
  (void)alloc(CardTable::kCardSize);
  void *res = alloc(cellSize<GCCell>());
  EXPECT_EQ(
      reinterpret_cast<char *>(res),
      segment.cardTable().indexToAddress(segStartIndex + 1));
  EXPECT_EQ(res, segment.cardTable().firstObjForCard(segStartIndex + 1));
}

TEST_F(CardObjectBoundaryNCTest, CrossingLargeAlloc) {
  (void)alloc(cellSize<GCCell>());
  const size_t kNumCards = 20;
  void *res1 = alloc(kNumCards * CardTable::kCardSize);
  EXPECT_EQ(segment.start() + 21 * CardTable::kCardSize, boundary.address());
  for (size_t i = 0; i < kNumCards; i++) {
    EXPECT_EQ(res1, segment.cardTable().firstObjForCard(segStartIndex + 1 + i));
  }
}

} // namespace

#endif // HERMESVM_GC_MALLOC
