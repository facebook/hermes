/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/StorageProvider.h"
#include "llvh/Support/MathExtras.h"

#include <ios>
#include <utility>
#include <vector>

using namespace hermes::vm;

namespace {

struct CardTableParam {
  size_t segmentSize;
};

struct CardTableNCTest : public ::testing::TestWithParam<CardTableParam> {
  CardTableNCTest();

  /// Run a test scenario whereby we dirty [dirtyStart, dirtyEnd], and then test
  /// that the address range [expectedStart, expectedEnd) has been dirtied (note
  /// that the first range is closed and the second range is half open).
  void dirtyRangeTest(
      char *expectedStart,
      char *dirtyStart,
      char *dirtyEnd,
      char *expectedEnd);

 protected:
  std::unique_ptr<StorageProvider> provider{StorageProvider::mmapProvider()};
  AlignedHeapSegment seg{
      std::move(AlignedHeapSegment::create(provider.get()).get())};
  CardTable *table{nullptr};

  // Addresses in the aligned storage to interact with during the tests.
  std::vector<char *> addrs;
};

void CardTableNCTest::dirtyRangeTest(
    char *expectedStart,
    char *dirtyStart,
    char *dirtyEnd,
    char *expectedEnd) {
  table->dirtyCardsForAddressRange(dirtyStart, dirtyEnd);

  for (char *p = expectedStart; p < expectedEnd; p += CardTable::kCardSize) {
    EXPECT_TRUE(table->isCardForAddressDirty(p));
  }
}

CardTableNCTest::CardTableNCTest() {
  auto &param = GetParam();
  table = new (seg.lowLim()) CardTable(param.segmentSize);

  // For purposes of this test, we'll assume the first writeable byte of
  // the segment comes just after the memory region that can be mapped by
  // kFirstUsedIndex bytes.
  auto first = seg.lowLim() +
      CardTable::kFirstUsedIndex * CardTable::kHeapBytesPerCardByte;
  auto last = reinterpret_cast<char *>(llvh::alignDown(
      reinterpret_cast<uintptr_t>(seg.hiLim() - 1), CardTable::kCardSize));

  addrs = {
      first,
      first + CardTable::kCardSize,
      first + CardTable::kCardSize * 42,

      last - CardTable::kCardSize * 42,
      last - CardTable::kCardSize,
      last};

  EXPECT_TRUE(std::is_sorted(addrs.begin(), addrs.end()));
}

TEST_P(CardTableNCTest, AddressToIndex) {
  // Expected indices in the card table corresponding to the probe
  // addresses into the storage.
  const size_t lastIx = table->getEndIndex() - 1;
  std::vector<size_t> indices{
      CardTable::kFirstUsedIndex,
      CardTable::kFirstUsedIndex + 1,
      CardTable::kFirstUsedIndex + 42,
      lastIx - 42,
      lastIx - 1,
      lastIx};

  for (unsigned i = 0; i < addrs.size(); i++) {
    char *addr = addrs.at(i);
    size_t ind = indices.at(i);

    EXPECT_EQ(ind, table->addressToIndex(addr))
        << "0x" << std::hex << (void *)addr << " -> " << ind;
    EXPECT_EQ(table->indexToAddress(ind), addr)
        << "0x" << std::hex << (void *)addr << " <- " << ind;
  }
}

TEST_P(CardTableNCTest, AddressToIndexBoundary) {
  // This test only works if the card table is laid out at the very beginning of
  // the storage.
  ASSERT_EQ(seg.lowLim(), reinterpret_cast<char *>(table));

  const size_t hiLim = table->getEndIndex();
  EXPECT_EQ(0, table->addressToIndex(seg.lowLim()));
  EXPECT_EQ(hiLim, table->addressToIndex(seg.hiLim()));
}

TEST_P(CardTableNCTest, DirtyAddress) {
  const size_t lastIx = table->getEndIndex() - 1;

  for (char *addr : addrs) {
    size_t ind = table->addressToIndex(addr);

    EXPECT_FALSE(ind > 0 && table->isCardForIndexDirty(ind - 1))
        << "initial " << ind << " - 1";
    EXPECT_FALSE(table->isCardForIndexDirty(ind)) << "initial " << ind;
    EXPECT_FALSE(ind < lastIx && table->isCardForIndexDirty(ind + 1))
        << "initial " << ind << " + 1";

    table->dirtyCardForAddress(addr);

    EXPECT_FALSE(ind > 0 && table->isCardForIndexDirty(ind - 1))
        << "dirty " << ind << " - 1";
    EXPECT_TRUE(table->isCardForIndexDirty(ind)) << "dirty " << ind;
    EXPECT_FALSE(ind < lastIx && table->isCardForIndexDirty(ind + 1))
        << "dirty " << ind << " + 1";

    table->clear();
  }
}

/// Dirty an emtpy range.
TEST_P(CardTableNCTest, DirtyAddressRangeEmpty) {
  char *addr = addrs.at(0);
  table->dirtyCardsForAddressRange(addr, addr);
  EXPECT_FALSE(table->findNextDirtyCard(
      CardTable::kFirstUsedIndex, table->getEndIndex()));
}

/// Dirty an address range smaller than a single card.
TEST_P(CardTableNCTest, DirtyAddressRangeSmall) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardTable::kCardSize / 2,
      /* expectedEnd */ addr + CardTable::kCardSize);
}

/// Dirty an address range corresponding exactly to a card.
TEST_P(CardTableNCTest, DirtyAddressRangeCard) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardTable::kCardSize,
      /* expectedEnd */ addr + CardTable::kCardSize);
}

/// Dirty an address range the width of a card but spread across a card
/// boundary.
TEST_P(CardTableNCTest, DirtyAddressRangeCardOverlapping) {
  char *addr = addrs.at(0);
  char *start = addr + CardTable::kCardSize / 2;
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ start,
      /* dirtyEnd */ start + CardTable::kCardSize,
      /* expectedEnd */ addr + 2 * CardTable::kCardSize);
}

/// Dirty an address range spanning multiple cards, with overhang on either
/// side.
TEST_P(CardTableNCTest, DirtyAddressRangeLarge) {
  char *addr = addrs.at(0);
  char *start = addr + CardTable::kCardSize / 2;
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ start,
      /* dirtyEnd */ start + 3 * CardTable::kCardSize,
      /* expectedEnd */ addr + 4 * CardTable::kCardSize);
}

TEST_P(CardTableNCTest, Initial) {
  for (char *addr : addrs) {
    EXPECT_FALSE(table->isCardForAddressDirty(addr));
  }
}

TEST_P(CardTableNCTest, Clear) {
  for (char *addr : addrs) {
    ASSERT_FALSE(table->isCardForAddressDirty(addr));
  }

  for (char *addr : addrs) {
    table->dirtyCardForAddress(addr);
  }

  for (char *addr : addrs) {
    ASSERT_TRUE(table->isCardForAddressDirty(addr));
  }

  table->clear();
  for (char *addr : addrs) {
    EXPECT_FALSE(table->isCardForAddressDirty(addr));
  }
}

TEST_P(CardTableNCTest, NextDirtyCardImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = table->addressToIndex(addr);

  table->dirtyCardForAddress(addr);
  auto dirty = table->findNextDirtyCard(ind, table->getEndIndex());

  ASSERT_TRUE(dirty);
  EXPECT_EQ(ind, *dirty);
}

TEST_P(CardTableNCTest, NextDirtyCard) {
  /// Empty case: No dirty cards
  EXPECT_FALSE(table->findNextDirtyCard(
      CardTable::kFirstUsedIndex, table->getEndIndex()));

  size_t from = CardTable::kFirstUsedIndex;
  for (char *addr : addrs) {
    table->dirtyCardForAddress(addr);

    auto ind = table->addressToIndex(addr);
    EXPECT_FALSE(table->findNextDirtyCard(from, ind));

    auto atEnd = table->findNextDirtyCard(from, ind + 1);
    auto inMiddle = table->findNextDirtyCard(from, table->getEndIndex());

    ASSERT_TRUE(atEnd);
    EXPECT_EQ(ind, *atEnd);

    ASSERT_TRUE(inMiddle);
    EXPECT_EQ(ind, *inMiddle);
    from = ind + 1;
  }
}

INSTANTIATE_TEST_CASE_P(
    CardTableNCTests,
    CardTableNCTest,
    ::testing::Values(
        CardTableParam{AlignedHeapSegmentBase::kSegmentUnitSize},
        CardTableParam{AlignedHeapSegmentBase::kSegmentUnitSize * 8},
        CardTableParam{AlignedHeapSegmentBase::kSegmentUnitSize * 128}));

} // namespace

#endif // HERMESVM_GC_MALLOC
