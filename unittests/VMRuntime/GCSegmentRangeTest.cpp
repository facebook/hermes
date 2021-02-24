/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This test uses NCGen files, so should only be enabled for that build.
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

#include "gtest/gtest.h"

#include "hermes/VM/GCSegmentRange-inline.h"
#include "hermes/VM/GCSegmentRange.h"
#include "hermes/VM/GenGCHeapSegment.h"

#include <vector>

using namespace hermes;
using namespace hermes::vm;

namespace {

struct GCSegmentRangeTest : public ::testing::Test {
  GCSegmentRangeTest();
  GenGCHeapSegment newSegment();

 private:
  std::unique_ptr<StorageProvider> provider_;
};

GCSegmentRangeTest::GCSegmentRangeTest()
    : provider_{StorageProvider::mmapProvider()} {}

GenGCHeapSegment GCSegmentRangeTest::newSegment() {
  auto result = AlignedStorage::create(provider_.get());
  EXPECT_TRUE(result);
  GenGCHeapSegment seg{std::move(result.get())};
  EXPECT_TRUE(seg);

  return seg;
}

TEST_F(GCSegmentRangeTest, EmptyConsumable) {
  auto range =
      GCSegmentRange::fromConsumable<GenGCHeapSegment *>(nullptr, nullptr);
  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, SingletonConsumable) {
  auto seg = newSegment();
  auto range = GCSegmentRange::singleton(&seg);

  EXPECT_EQ(&seg, range->next());
  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, IterConsumable) {
  constexpr size_t NUM = 10;
  std::vector<GenGCHeapSegment> segs;

  for (size_t i = 0; i < NUM; ++i) {
    segs.emplace_back(newSegment());
  }

  auto range = GCSegmentRange::fromConsumable(segs.begin(), segs.end());

  for (size_t i = 0; i < NUM; ++i) {
    EXPECT_EQ(&segs[i], range->next()) << "Mismatch at " << i;
  }

  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, FuseConsumable) {
  constexpr size_t NUM = 10;
  std::vector<GenGCHeapSegment> segs;

  for (size_t i = 0; i < NUM; ++i) {
    segs.emplace_back(newSegment());
  }

  auto range = GCSegmentRange::fuse(
      GCSegmentRange::fromConsumable(segs.begin(), segs.end()));

  for (size_t i = 0; i < NUM; ++i) {
    EXPECT_EQ(&segs[i], range->next()) << "Mismatch at " << i;
  }

  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, FuseEarlyTermination) {
  constexpr size_t FAIL = 5;

  // A range that pretends to materialise segments on the fly, failing on the
  // FAIL-th allocation, but is able to continue afterwards.
  struct Generator : public GCSegmentRange {
    GenGCHeapSegment *next() override {
      if (FAIL == allocs_++) {
        return nullptr;
      }

      return &seg_;
    }

   private:
    size_t allocs_{0};
    GenGCHeapSegment seg_;
  };

  { // First verify the behaviour of Generator.
    Generator g;
    for (size_t i = 0; i < FAIL; ++i) {
      ASSERT_NE(nullptr, g.next());
    }
    ASSERT_EQ(nullptr, g.next());
    ASSERT_NE(nullptr, g.next());
  }

  { // Now make sure a fused version of Generator will not restart after a
    // failure.
    auto r = GCSegmentRange::fuse(std::make_unique<Generator>());
    for (size_t i = 0; i < FAIL; ++i) {
      EXPECT_NE(nullptr, r->next());
    }
    EXPECT_EQ(nullptr, r->next());
    EXPECT_EQ(nullptr, r->next());
  }
}

TEST_F(GCSegmentRangeTest, EmptyConcat) {
  auto range = GCSegmentRange::concat();
  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, SingletonConcat) {
  auto seg = newSegment();
  auto range = GCSegmentRange::concat(GCSegmentRange::singleton(&seg));

  EXPECT_EQ(&seg, range->next());
  EXPECT_EQ(nullptr, range->next());
}

TEST_F(GCSegmentRangeTest, ConcatMultiple) {
  constexpr size_t NUM = 10;
  std::vector<GenGCHeapSegment> init, tail;
  auto mid = newSegment();

  for (size_t i = 0; i < NUM; ++i) {
    // Adding to the tail first, to make sure we are not simply iterating in
    // creation order.
    tail.emplace_back(newSegment());
    init.emplace_back(newSegment());
  }

  auto range = GCSegmentRange::concat(
      GCSegmentRange::fromConsumable(init.begin(), init.end()),
      GCSegmentRange::singleton(&mid),
      GCSegmentRange::fromConsumable(tail.begin(), tail.end()));

  for (size_t i = 0; i < NUM; ++i) {
    EXPECT_EQ(&init[i], range->next()) << "Mismatch at Init " << i;
  }

  EXPECT_EQ(&mid, range->next()) << "Mismatch at Mid";

  for (size_t i = 0; i < NUM; ++i) {
    EXPECT_EQ(&tail[i], range->next()) << "Mismatch at Tail " << i;
  }

  EXPECT_EQ(nullptr, range->next());
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
