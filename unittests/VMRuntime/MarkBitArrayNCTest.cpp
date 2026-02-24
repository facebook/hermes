/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/StorageProvider.h"
#include "llvh/Support/MathExtras.h"

#include <ios>
#include <queue>
#include <utility>
#include <vector>

using namespace hermes::vm;
using MarkBitArray = FixedSizeHeapSegment::Contents::MarkBitArray;

namespace {

struct MarkBitArrayTest : public ::testing::Test {
  MarkBitArrayTest();

  size_t addressToMarkBitArrayIndex(const void *addr) {
    // Since we only test FixedSizeHeapSegment in this file, it's safe to cast
    // address in the segment to a GCCell pointer (i.e., we can always compute
    // the correct segment start address from this pointer).
    auto *cell = reinterpret_cast<const GCCell *>(addr);
    return seg.addressToMarkBitArrayIndex(cell);
  }

 protected:
  std::unique_ptr<StorageProvider> provider;
  FixedSizeHeapSegment seg;
  MarkBitArray &mba;

  // Addresses in the aligned storage to interact w ith during the tests.
  std::vector<char *> addrs;
};

MarkBitArrayTest::MarkBitArrayTest()
    : provider(StorageProvider::mmapProvider()),
      seg{std::move(FixedSizeHeapSegment::create(provider.get()).get())},
      mba(seg.markBitArray()) {
  auto first = seg.lowLim();
  auto last = reinterpret_cast<char *>(
      llvh::alignDown(reinterpret_cast<uintptr_t>(seg.hiLim() - 1), HeapAlign));

  addrs = {
      first,
      first + HeapAlign,
      first + HeapAlign * 42,

      last - HeapAlign * 42,
      last - HeapAlign,
      last};

  EXPECT_TRUE(std::is_sorted(addrs.begin(), addrs.end()));
}

TEST_F(MarkBitArrayTest, AddressToIndex) {
  // Expected indices in the mark bit array corresponding to the probe
  // addresses into the storage.
  size_t lastIx = mba.size() - 1;
  std::vector<size_t> indices{0, 1, 42, lastIx - 42, lastIx - 1, lastIx};

  for (unsigned i = 0; i < addrs.size(); i++) {
    char *addr = addrs.at(i);
    size_t ind = indices.at(i);

    EXPECT_EQ(ind, addressToMarkBitArrayIndex(addr))
        << "0x" << std::hex << (void *)addr << " -> " << ind;
    char *toAddr = seg.lowLim() + (ind << LogHeapAlign);
    EXPECT_EQ(toAddr, addr)
        << "0x" << std::hex << (void *)addr << " <- " << ind;
  }
}

TEST_F(MarkBitArrayTest, MarkGet) {
  const size_t lastIx = mba.size() - 1;

  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);

    EXPECT_FALSE(ind > 0 && mba.at(ind - 1)) << "initial " << ind << " - 1";
    EXPECT_FALSE(mba.at(ind)) << "initial " << ind;
    EXPECT_FALSE(ind < lastIx && mba.at(ind + 1))
        << "initial " << ind << " + 1";

    mba.set(ind, true);

    EXPECT_FALSE(ind > 0 && mba.at(ind - 1)) << "mark " << ind << " - 1";
    EXPECT_TRUE(mba.at(ind)) << "mark " << ind;
    EXPECT_FALSE(ind < lastIx && mba.at(ind + 1)) << "mark " << ind << " + 1";

    mba.reset();
  }
}

TEST_F(MarkBitArrayTest, Initial) {
  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);
    EXPECT_FALSE(mba.at(ind));
  }
}

TEST_F(MarkBitArrayTest, Clear) {
  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);
    ASSERT_FALSE(mba.at(ind));
  }

  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);
    mba.set(ind, true);
  }

  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);
    ASSERT_TRUE(mba.at(ind));
  }

  mba.reset();
  for (char *addr : addrs) {
    size_t ind = addressToMarkBitArrayIndex(addr);
    EXPECT_FALSE(mba.at(ind));
  }
}

TEST_F(MarkBitArrayTest, NextMarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = addressToMarkBitArrayIndex(addr);

  mba.set(ind, true);
  EXPECT_EQ(ind, mba.findNextSetBitFrom(ind));
}

TEST_F(MarkBitArrayTest, NextMarkedBit) {
  const size_t FOUND_NONE = mba.size();

  // Empty case: No marked bits
  EXPECT_EQ(FOUND_NONE, mba.findNextSetBitFrom(0));
  std::queue<size_t> indices;
  for (char *addr : addrs) {
    auto ind = addressToMarkBitArrayIndex(addr);
    mba.set(ind, true);
    indices.push(ind);
  }
  // Use the same style of loop we use elsewhere for scanning the array.
  for (size_t from = mba.findNextSetBitFrom(0); from < mba.size();
       from = mba.findNextSetBitFrom(from + 1)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

TEST_F(MarkBitArrayTest, NextUnmarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = addressToMarkBitArrayIndex(addr);
  mba.set();
  mba.set(ind, false);
  EXPECT_EQ(ind, mba.findNextZeroBitFrom(ind));
}

TEST_F(MarkBitArrayTest, NextUnmarkedBit) {
  const size_t FOUND_NONE = mba.size();
  mba.set();
  /// Full case: No unmarked bits
  EXPECT_EQ(FOUND_NONE, mba.findNextZeroBitFrom(0));
  std::queue<size_t> indices;
  for (char *addr : addrs) {
    auto ind = addressToMarkBitArrayIndex(addr);
    mba.set(ind, false);
    indices.push(ind);
  }

  // Use the same style of loop we use elsewhere for scanning the array.
  for (size_t from = mba.findNextZeroBitFrom(0); from < mba.size();
       from = mba.findNextZeroBitFrom(from + 1)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

TEST_F(MarkBitArrayTest, PrevMarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = addressToMarkBitArrayIndex(addr);
  mba.set(ind, true);
  EXPECT_EQ(ind, mba.findPrevSetBitFrom(ind + 1));
}

TEST_F(MarkBitArrayTest, PrevMarkedBit) {
  const size_t FOUND_NONE = mba.size();
  size_t from = mba.size();
  /// Empty case: No unmarked bits
  EXPECT_EQ(FOUND_NONE, mba.findPrevSetBitFrom(from));

  std::queue<size_t> indices;
  size_t addrIdx = addrs.size();
  while (addrIdx-- > 0) {
    auto ind = addressToMarkBitArrayIndex(addrs[addrIdx]);
    mba.set(ind, true);
    indices.push(ind);
  }
  for (size_t from = mba.findNextSetBitFrom(mba.size()); from != mba.size();
       from = mba.findNextSetBitFrom(from)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

TEST_F(MarkBitArrayTest, PrevUnmarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = addressToMarkBitArrayIndex(addr);
  mba.set();
  mba.set(ind, false);
  EXPECT_EQ(ind, mba.findPrevZeroBitFrom(ind + 1));
}

TEST_F(MarkBitArrayTest, PrevUnmarkedBit) {
  constexpr size_t FOUND_NONE = MarkBitArray::size();
  mba.set();
  size_t from = mba.size();
  /// Full case: No unmarked bits
  EXPECT_EQ(FOUND_NONE, mba.findPrevZeroBitFrom(from));

  std::queue<size_t> indices;
  size_t addrIdx = addrs.size();
  while (addrIdx-- > 0) {
    auto ind = addressToMarkBitArrayIndex(addrs[addrIdx]);
    mba.set(ind, false);
    indices.push(ind);
  }
  for (size_t from = mba.findPrevZeroBitFrom(mba.size()); from != mba.size();
       from = mba.findPrevZeroBitFrom(from)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

} // namespace

#endif // !HERMESVM_GC_MALLOC
