/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/CardTableNC.h"
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
  AlignedStorage as{std::move(AlignedStorage::create(provider.get()).get())};
  CardTable *table{new (as.lowLim()) CardTable()};

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
  // For purposes of this test, we'll assume the first writeable byte of
  // the segment comes just after the card table (which is at the
  // start of the segment).
  auto first = as.lowLim() + sizeof(CardTable);
  auto last = reinterpret_cast<char *>(llvh::alignDown(
      reinterpret_cast<uintptr_t>(as.hiLim() - 1), CardTable::kCardSize));

  addrs = {
      first,
      first + CardTable::kCardSize,
      first + CardTable::kCardSize * 42,

      last - CardTable::kCardSize * 42,
      last - CardTable::kCardSize,
      last};

  EXPECT_TRUE(std::is_sorted(addrs.begin(), addrs.end()));
}

TEST_F(CardTableNCTest, AddressToIndex) {
  // Expected indices in the card table corresponding to the probe
  // addresses into the storage.
  const size_t lastIx = CardTable::kValidIndices - 1;
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

TEST_F(CardTableNCTest, AddressToIndexBoundary) {
  // This test only works if the card table is laid out at the very beginning of
  // the storage.
  ASSERT_EQ(as.lowLim(), reinterpret_cast<char *>(table));

  const size_t hiLim = CardTable::kValidIndices;
  EXPECT_EQ(0, table->addressToIndex(as.lowLim()));
  EXPECT_EQ(hiLim, table->addressToIndex(as.hiLim()));
}

TEST_F(CardTableNCTest, DirtyAddress) {
  const size_t lastIx = CardTable::kValidIndices - 1;

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
TEST_F(CardTableNCTest, DirtyAddressRangeEmpty) {
  char *addr = addrs.at(0);
  table->dirtyCardsForAddressRange(addr, addr);
  EXPECT_FALSE(table->findNextDirtyCard(0, CardTable::kValidIndices));
}

/// Dirty an address range smaller than a single card.
TEST_F(CardTableNCTest, DirtyAddressRangeSmall) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardTable::kCardSize / 2,
      /* expectedEnd */ addr + CardTable::kCardSize);
}

/// Dirty an address range corresponding exactly to a card.
TEST_F(CardTableNCTest, DirtyAddressRangeCard) {
  char *addr = addrs.at(0);
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ addr,
      /* dirtyEnd */ addr + CardTable::kCardSize,
      /* expectedEnd */ addr + CardTable::kCardSize);
}

/// Dirty an address range the width of a card but spread across a card
/// boundary.
TEST_F(CardTableNCTest, DirtyAddressRangeCardOverlapping) {
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
TEST_F(CardTableNCTest, DirtyAddressRangeLarge) {
  char *addr = addrs.at(0);
  char *start = addr + CardTable::kCardSize / 2;
  dirtyRangeTest(
      /* expectedStart */ addr,
      /* dirtyStart */ start,
      /* dirtyEnd */ start + 3 * CardTable::kCardSize,
      /* expectedEnd */ addr + 4 * CardTable::kCardSize);
}

TEST_F(CardTableNCTest, Initial) {
  for (char *addr : addrs) {
    EXPECT_FALSE(table->isCardForAddressDirty(addr));
  }
}

TEST_F(CardTableNCTest, Clear) {
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

TEST_F(CardTableNCTest, NextDirtyCardImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = table->addressToIndex(addr);

  table->dirtyCardForAddress(addr);
  auto dirty = table->findNextDirtyCard(ind, CardTable::kValidIndices);

  ASSERT_TRUE(dirty);
  EXPECT_EQ(ind, *dirty);
}

TEST_F(CardTableNCTest, NextDirtyCard) {
  /// Empty case: No dirty cards
  EXPECT_FALSE(table->findNextDirtyCard(0, CardTable::kValidIndices));

  size_t from = 0;
  for (char *addr : addrs) {
    table->dirtyCardForAddress(addr);

    auto ind = table->addressToIndex(addr);
    EXPECT_FALSE(table->findNextDirtyCard(from, ind));

    auto atEnd = table->findNextDirtyCard(from, ind + 1);
    auto inMiddle = table->findNextDirtyCard(from, CardTable::kValidIndices);

    ASSERT_TRUE(atEnd);
    EXPECT_EQ(ind, *atEnd);

    ASSERT_TRUE(inMiddle);
    EXPECT_EQ(ind, *inMiddle);
    from = ind + 1;
  }
}

} // namespace

#endif // HERMESVM_GC_MALLOC
