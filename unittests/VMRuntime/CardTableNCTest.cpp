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
#include "llvh/Support/MathExtras.h"

#include <ios>
#include <utility>
#include <vector>

using namespace hermes::vm;

namespace {

struct CardTableNCTest : public ::testing::Test {
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
  FixedSizeHeapSegment seg{
      std::move(FixedSizeHeapSegment::create(provider.get()).get())};

  // Addresses in the aligned storage to interact with during the tests.
  std::vector<char *> addrs;
};

void CardTableNCTest::dirtyRangeTest(
    char *expectedStart,
    char *dirtyStart,
    char *dirtyEnd,
    char *expectedEnd) {
  seg.dirtyCardsForAddressRange(dirtyStart, dirtyEnd);

  for (char *p = expectedStart; p < expectedEnd;
       p += CardBoundaryTable::kCardSize) {
    EXPECT_TRUE(seg.isCardForAddressDirty(p));
  }
}

CardTableNCTest::CardTableNCTest() {
  // For purposes of this test, we'll assume the first writeable byte of
  // the segment comes just after the memory region that can be mapped by
  // kFirstUsedIndex bytes.
  auto first = seg.lowLim() +
      AlignedHeapSegment::Contents::kFirstUsedIndex *
          AlignedHeapSegment::Contents::kCardSize;
  auto last = reinterpret_cast<char *>(llvh::alignDown(
      reinterpret_cast<uintptr_t>(seg.hiLim() - 1),
      CardBoundaryTable::kCardSize));

  addrs = {
      first,
      first + CardBoundaryTable::kCardSize,
      first + CardBoundaryTable::kCardSize * 42,

      last - CardBoundaryTable::kCardSize * 42,
      last - CardBoundaryTable::kCardSize,
      last};

  EXPECT_TRUE(std::is_sorted(addrs.begin(), addrs.end()));
}

TEST_F(CardTableNCTest, AddressToIndex) {
  // Expected indices in the card table corresponding to the probe
  // addresses into the storage.
  const size_t lastIx = seg.getEndCardIndex() - 1;
  std::vector<size_t> indices{
      AlignedHeapSegment::Contents::kFirstUsedIndex,
      AlignedHeapSegment::Contents::kFirstUsedIndex + 1,
      AlignedHeapSegment::Contents::kFirstUsedIndex + 42,
      lastIx - 42,
      lastIx - 1,
      lastIx};

  for (unsigned i = 0; i < addrs.size(); i++) {
    char *addr = addrs.at(i);
    size_t ind = indices.at(i);

    EXPECT_EQ(ind, seg.addressToCardIndex(addr))
        << "0x" << std::hex << (void *)addr << " -> " << ind;
    EXPECT_EQ(seg.cardIndexToAddress(ind), addr)
        << "0x" << std::hex << (void *)addr << " <- " << ind;
  }
}

TEST_F(CardTableNCTest, AddressToIndexBoundary) {
  const size_t hiLim = seg.getEndCardIndex();
  EXPECT_EQ(0, seg.addressToCardIndex(seg.lowLim()));
  EXPECT_EQ(hiLim, seg.addressToCardIndex(seg.hiLim()));
}

TEST_F(CardTableNCTest, DirtyAddress) {
  const size_t lastIx = seg.getEndCardIndex() - 1;

  for (char *addr : addrs) {
    size_t ind = seg.addressToCardIndex(addr);

    EXPECT_FALSE(ind > 0 && seg.isCardForIndexDirty(ind - 1))
        << "initial " << ind << " - 1";
    EXPECT_FALSE(seg.isCardForIndexDirty(ind)) << "initial " << ind;
    EXPECT_FALSE(ind < lastIx && seg.isCardForIndexDirty(ind + 1))
        << "initial " << ind << " + 1";

    seg.dirtyCardForAddressInLargeObj(addr);

    EXPECT_FALSE(ind > 0 && seg.isCardForIndexDirty(ind - 1))
        << "dirty " << ind << " - 1";
    EXPECT_TRUE(seg.isCardForIndexDirty(ind)) << "dirty " << ind;
    EXPECT_FALSE(ind < lastIx && seg.isCardForIndexDirty(ind + 1))
        << "dirty " << ind << " + 1";

    seg.clearAllCards();
  }
}

/// Dirty an emtpy range.
TEST_F(CardTableNCTest, DirtyAddressRangeEmpty) {
  char *addr = addrs.at(0);
  seg.dirtyCardsForAddressRange(addr, addr);
  EXPECT_FALSE(seg.findNextDirtyCard(
      AlignedHeapSegment::Contents::kFirstUsedIndex, seg.getEndCardIndex()));
}

/// Dirty an address range smaller than a single card.
TEST_F(CardTableNCTest, DirtyAddressRangeSmall) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardBoundaryTable::kCardSize / 2,
      /* expectedEnd */ addr + CardBoundaryTable::kCardSize);
}

/// Dirty an address range corresponding exactly to a card.
TEST_F(CardTableNCTest, DirtyAddressRangeCard) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardBoundaryTable::kCardSize,
      /* expectedEnd */ addr + CardBoundaryTable::kCardSize);
}

/// Dirty an address range the width of a card but spread across a card
/// boundary.
TEST_F(CardTableNCTest, DirtyAddressRangeCardOverlapping) {
  char *addr = addrs.at(0);
  char *start = addr + CardBoundaryTable::kCardSize / 2;
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ start,
      /* dirtyEnd */ start + CardBoundaryTable::kCardSize,
      /* expectedEnd */ addr + 2 * CardBoundaryTable::kCardSize);
}

/// Dirty an address range spanning multiple cards, with overhang on either
/// side.
TEST_F(CardTableNCTest, DirtyAddressRangeLarge) {
  char *addr = addrs.at(0);
  char *start = addr + CardBoundaryTable::kCardSize / 2;
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ start,
      /* dirtyEnd */ start + 3 * CardBoundaryTable::kCardSize,
      /* expectedEnd */ addr + 4 * CardBoundaryTable::kCardSize);
}

TEST_F(CardTableNCTest, Initial) {
  for (char *addr : addrs) {
    EXPECT_FALSE(seg.isCardForAddressDirty(addr));
  }
}

TEST_F(CardTableNCTest, Clear) {
  for (char *addr : addrs) {
    ASSERT_FALSE(seg.isCardForAddressDirty(addr));
  }

  for (char *addr : addrs) {
    seg.dirtyCardForAddressInLargeObj(addr);
  }

  for (char *addr : addrs) {
    ASSERT_TRUE(seg.isCardForAddressDirty(addr));
  }

  seg.clearAllCards();
  for (char *addr : addrs) {
    EXPECT_FALSE(seg.isCardForAddressDirty(addr));
  }
}

TEST_F(CardTableNCTest, NextDirtyCardImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = seg.addressToCardIndex(addr);

  seg.dirtyCardForAddress(addr);
  auto dirty = seg.findNextDirtyCard(ind, seg.getEndCardIndex());

  ASSERT_TRUE(dirty);
  EXPECT_EQ(ind, *dirty);
}

TEST_F(CardTableNCTest, NextDirtyCard) {
  /// Empty case: No dirty cards
  EXPECT_FALSE(seg.findNextDirtyCard(
      AlignedHeapSegment::Contents::kFirstUsedIndex, seg.getEndCardIndex()));

  size_t from = AlignedHeapSegment::Contents::kFirstUsedIndex;
  for (char *addr : addrs) {
    seg.dirtyCardForAddressInLargeObj(addr);

    auto ind = seg.addressToCardIndex(addr);
    EXPECT_FALSE(seg.findNextDirtyCard(from, ind));

    auto atEnd = seg.findNextDirtyCard(from, ind + 1);
    auto inMiddle = seg.findNextDirtyCard(from, seg.getEndCardIndex());

    ASSERT_TRUE(atEnd);
    EXPECT_EQ(ind, *atEnd);

    ASSERT_TRUE(inMiddle);
    EXPECT_EQ(ind, *inMiddle);
    from = ind + 1;
  }
}

} // namespace

#endif // HERMESVM_GC_MALLOC
