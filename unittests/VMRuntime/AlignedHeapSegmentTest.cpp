/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HeapAlign.h"

namespace {

using namespace hermes;
using namespace hermes::vm;

/// Calculate the length of the longest prefix of the sequence from \p begin
/// (inclusive) to \p end (exclusive) containing at most one unique value.
static size_t runLength(const size_t *begin, const size_t *end) {
  const size_t *it = begin;
  size_t count = 0;
  while (it < end && *it++ == *begin)
    count++;
  return count;
}

struct AlignedHeapSegmentTest : public ::testing::Test {
  AlignedHeapSegmentTest()
      : provider_(StorageProvider::mmapProvider()),
        s(std::move(AlignedStorage::create(provider_.get()).get())) {}

  ~AlignedHeapSegmentTest() = default;

  std::unique_ptr<StorageProvider> provider_;
  AlignedHeapSegment s;
};

TEST_F(AlignedHeapSegmentTest, AllocTest) {
  s.growToLimit();

  const size_t INIT_BYTES = heapAlignSize(sizeof(GCCell));
  const size_t STEP_BYTES = HeapAlign;

  size_t allocated = 0;
  size_t size = INIT_BYTES;
  const size_t capacity = s.available();
  EXPECT_LE(capacity, s.size());

  AllocResult res = s.alloc(size);
  while (res.success) {
    allocated += size;

    auto raw = reinterpret_cast<size_t *>(res.ptr);
    std::fill(raw, raw + size / sizeof(size_t), size);

    EXPECT_EQ(allocated, s.used());
    EXPECT_EQ(capacity, s.used() + s.available());

    size += STEP_BYTES;
    res = s.alloc(size);
  }

  // This is the only reason why the allocation could have failed.
  EXPECT_LT(s.available(), size);

  const size_t INIT_SIZE = INIT_BYTES / sizeof(size_t);
  const size_t STEP_SIZE = STEP_BYTES / sizeof(size_t);
  const size_t FINAL_SIZE = size / sizeof(size_t);

  // Check the bit pattern we wrote during allocation
  size_t *data = reinterpret_cast<size_t *>(s.start());
  size_t *const end = reinterpret_cast<size_t *>(s.start() + allocated);

  for (size_t run = INIT_SIZE; run < FINAL_SIZE; run += STEP_SIZE) {
    size_t actualRun = runLength(data, end);
    EXPECT_EQ(run, actualRun) << "Run " << run;
    data += run;
  }
}

TEST_F(AlignedHeapSegmentTest, FullSize) {
  s.growToLimit();

  EXPECT_EQ(s.size(), AlignedHeapSegment::maxSize());
  EXPECT_EQ(s.size(), s.available());
  EXPECT_EQ(s.size(), s.hiLim() - s.start());

  // Try and allocate the entire region.
  AllocResult res = s.alloc(s.size());

  EXPECT_TRUE(res.success);
  EXPECT_TRUE(nullptr != res.ptr);
}

TEST_F(AlignedHeapSegmentTest, SmallSize) {
  const size_t PS = oscompat::page_size();
  s.growTo(PS);
  s.clearExternalMemoryCharge();

  EXPECT_GE(PS, s.size());

  AllocResult failed = s.alloc(heapAlignSize(s.available() + 1));
  EXPECT_FALSE(failed.success);
  EXPECT_TRUE(nullptr == failed.ptr);

  AllocResult res = s.alloc(PS);
  EXPECT_TRUE(res.ptr);
  EXPECT_TRUE(nullptr != res.ptr);
}

TEST_F(AlignedHeapSegmentTest, ResetLevel) {
  s.growToLimit();

  // Make the level different from the start of the region.
  AllocResult res = s.alloc(cellSize<GCCell>());
  ASSERT_TRUE(res.success);
  ASSERT_NE(s.start(), s.level());

  s.resetLevel();
  EXPECT_EQ(s.start(), s.level());
}

TEST_F(AlignedHeapSegmentTest, SuccessfulGrowTo) {
  const size_t PS = oscompat::page_size();

  s.growTo(PS);
  EXPECT_EQ(PS, s.size());

  s.growTo(2 * PS);
  EXPECT_EQ(2 * PS, s.size());
}

TEST_F(AlignedHeapSegmentTest, GrowToSmaller) {
  const size_t PS = oscompat::page_size();

  s.growTo(2 * PS);
  EXPECT_EQ(2 * PS, s.size());

  s.growTo(PS);
  EXPECT_EQ(2 * PS, s.size());
}

TEST_F(AlignedHeapSegmentTest, SuccessfulGrowToFit) {
  const size_t PS = oscompat::page_size();

  EXPECT_TRUE(s.growToFit(PS));
  EXPECT_LE(PS, s.size());
}

TEST_F(AlignedHeapSegmentTest, GrowToFitTooBig) {
  const size_t TOO_BIG = heapAlignSize(AlignedStorage::size() + 1);

  EXPECT_FALSE(s.growToFit(TOO_BIG));
  EXPECT_EQ(0, s.size());
}

TEST_F(AlignedHeapSegmentTest, PageAlignedGrowToFit) {
  const size_t PS = oscompat::page_size();

  EXPECT_TRUE(s.growToFit(PS / 2));
  EXPECT_EQ(0, reinterpret_cast<uintptr_t>(s.end()) % PS);
}

TEST_F(AlignedHeapSegmentTest, GrowToFitUnnecessary) {
  const size_t PS = oscompat::page_size();

  s.growTo(PS);

  auto sizeBefore = s.size();
  EXPECT_TRUE(s.growToFit(PS / 2));
  EXPECT_EQ(sizeBefore, s.size());
}

#ifndef NDEBUG

using AlignedHeapSegmentDeathTest = AlignedHeapSegmentTest;

// Allocating into a null segment causes an assertion failure on !NDEBUG builds.
TEST_F(AlignedHeapSegmentDeathTest, NullAlloc) {
  AlignedHeapSegment s;
  constexpr uint32_t SIZE = heapAlignSize(sizeof(GCCell));
  EXPECT_DEATH_IF_SUPPORTED({ s.alloc(SIZE); }, "null segment");
}

TEST_F(AlignedHeapSegmentDeathTest, GrowToTooBig) {
  const size_t PS = oscompat::page_size();

  EXPECT_DEATH_IF_SUPPORTED(
      { s.growTo(AlignedHeapSegment::maxSize() + PS); }, "max size");
}
#endif // !NDEBUG

} // namespace

#endif // !HERMESVM_GC_MALLOC
