/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

#include "gtest/gtest.h"

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/MarkBitArrayNC.h"
#include "hermes/VM/StorageProvider.h"
#include "llvh/Support/MathExtras.h"

#include <ios>
#include <queue>
#include <utility>
#include <vector>

using namespace hermes::vm;

namespace {

struct MarkBitArrayNCTest : public ::testing::Test {
  MarkBitArrayNCTest();

 protected:
  std::unique_ptr<StorageProvider> provider;
  AlignedStorage as;
  MarkBitArrayNC *mba;

  // Addresses in the aligned storage to interact with during the tests.
  std::vector<char *> addrs;
};

MarkBitArrayNCTest::MarkBitArrayNCTest()
    : provider(StorageProvider::mmapProvider()),
      as{std::move(AlignedStorage::create(provider.get()).get())},
      mba(new (as.lowLim()) MarkBitArrayNC()) {
  auto first = as.lowLim();
  auto last = reinterpret_cast<char *>(
      llvh::alignDown(reinterpret_cast<uintptr_t>(as.hiLim() - 1), HeapAlign));

  addrs = {first,
           first + HeapAlign,
           first + HeapAlign * 42,

           last - HeapAlign * 42,
           last - HeapAlign,
           last};

  EXPECT_TRUE(std::is_sorted(addrs.begin(), addrs.end()));
}

TEST_F(MarkBitArrayNCTest, AddressToIndex) {
  // Expected indices in the mark bit array corresponding to the probe
  // addresses into the storage.
  size_t lastIx = MarkBitArrayNC::kNumBits - 1;
  std::vector<size_t> indices{0, 1, 42, lastIx - 42, lastIx - 1, lastIx};

  for (unsigned i = 0; i < addrs.size(); i++) {
    char *addr = addrs.at(i);
    size_t ind = indices.at(i);

    EXPECT_EQ(ind, mba->addressToIndex(addr))
        << "0x" << std::hex << (void *)addr << " -> " << ind;
    EXPECT_EQ(mba->indexToAddress(ind), addr)
        << "0x" << std::hex << (void *)addr << " <- " << ind;
  }
}

TEST_F(MarkBitArrayNCTest, MarkGet) {
  const size_t lastIx = MarkBitArrayNC::kNumBits - 1;

  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);

    EXPECT_FALSE(ind > 0 && mba->at(ind - 1)) << "initial " << ind << " - 1";
    EXPECT_FALSE(mba->at(ind)) << "initial " << ind;
    EXPECT_FALSE(ind < lastIx && mba->at(ind + 1))
        << "initial " << ind << " + 1";

    mba->mark(ind);

    EXPECT_FALSE(ind > 0 && mba->at(ind - 1)) << "mark " << ind << " - 1";
    EXPECT_TRUE(mba->at(ind)) << "mark " << ind;
    EXPECT_FALSE(ind < lastIx && mba->at(ind + 1)) << "mark " << ind << " + 1";

    mba->clear();
  }
}

TEST_F(MarkBitArrayNCTest, Initial) {
  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);
    EXPECT_FALSE(mba->at(ind));
  }
}

TEST_F(MarkBitArrayNCTest, Clear) {
  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);
    ASSERT_FALSE(mba->at(ind));
  }

  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);
    mba->mark(ind);
  }

  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);
    ASSERT_TRUE(mba->at(ind));
  }

  mba->clear();
  for (char *addr : addrs) {
    size_t ind = mba->addressToIndex(addr);
    EXPECT_FALSE(mba->at(ind));
  }
}

TEST_F(MarkBitArrayNCTest, NextMarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = mba->addressToIndex(addr);

  mba->mark(ind);
  EXPECT_EQ(ind, mba->findNextMarkedBitFrom(ind));
}

TEST_F(MarkBitArrayNCTest, NextMarkedBit) {
  constexpr size_t FOUND_NONE = MarkBitArrayNC::kNumBits;

  // Empty case: No marked bits
  EXPECT_EQ(FOUND_NONE, mba->findNextMarkedBitFrom(0));
  std::queue<size_t> indices;
  for (char *addr : addrs) {
    auto ind = mba->addressToIndex(addr);
    mba->mark(ind);
    indices.push(ind);
  }
  // Use the same style of loop we use elsewhere for scanning the array.
  for (size_t from = mba->findNextMarkedBitFrom(0);
       from < MarkBitArrayNC::kNumBits;
       from = mba->findNextMarkedBitFrom(from + 1)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

TEST_F(MarkBitArrayNCTest, NextUnmarkedBitImmediate) {
  char *addr = addrs.at(addrs.size() / 2);
  size_t ind = mba->addressToIndex(addr);
  mba->markAll();
  mba->bitArray_.set(ind, false);
  EXPECT_EQ(ind, mba->findNextUnmarkedBitFrom(ind));
}

TEST_F(MarkBitArrayNCTest, NextUnmarkedBit) {
  constexpr size_t FOUND_NONE = MarkBitArrayNC::kNumBits;
  mba->markAll();
  /// Full case: No unmarked bits
  EXPECT_EQ(FOUND_NONE, mba->findNextUnmarkedBitFrom(0));
  std::queue<size_t> indices;
  for (char *addr : addrs) {
    auto ind = mba->addressToIndex(addr);
    mba->bitArray_.set(ind, false);
    indices.push(ind);
  }

  // Use the same style of loop we use elsewhere for scanning the array.
  for (size_t from = mba->findNextUnmarkedBitFrom(0);
       from < MarkBitArrayNC::kNumBits;
       from = mba->findNextUnmarkedBitFrom(from + 1)) {
    EXPECT_EQ(indices.front(), from);
    indices.pop();
  }
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
