/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This test uses NCGen files, so should only be enabled for that build.
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

#include "gtest/gtest.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCSegmentAddressIndex.h"

#include <algorithm>
#include <deque>
#include <iterator>

using namespace hermes::vm;

namespace {

/// Custom ordering for non-null AlignedHeapSegment pointers, by their lowLim
/// address.
bool lowLimOrder(const AlignedHeapSegment *a, const AlignedHeapSegment *b) {
  assert(a && b);
  return a->lowLim() < b->lowLim();
}

struct GCSegmentAddressIndexTest : public ::testing::Test {
  void addSegments(size_t n);
  AlignedHeapSegment *segment(size_t i);

  /// Expect the index to contain the given segments as values, sorted in order
  /// of their starting address.
  ///
  /// \p expected The expected segments
  void expectSegments(std::vector<const AlignedHeapSegment *> expected);

 protected:
  GCSegmentAddressIndex index{};

  std::unique_ptr<StorageProvider> provider_{StorageProvider::mmapProvider()};

 private:
  std::deque<AlignedHeapSegment> segments_{};
};

using GCSegmentAddressIndexDeathTest = GCSegmentAddressIndexTest;

void GCSegmentAddressIndexTest::addSegments(size_t n) {
  std::vector<AlignedHeapSegment *> insertions;

  // Create the requisite number of segments.
  for (size_t i = 0; i < n; ++i) {
    auto result = AlignedStorage::create(provider_.get());
    ASSERT_TRUE(result) << "Allocating an AlignedStorage failed";
    segments_.emplace_back(std::move(result.get()));
    insertions.push_back(&segments_.back());
  }

  // Intentionally disorder the segments by sorting the list, and then swapping
  // adjacent elements.
  std::sort(insertions.begin(), insertions.end(), lowLimOrder);
  {
    auto it = insertions.begin();
    const auto end =
        insertions.size() % 2 == 0 ? insertions.end() : insertions.end() - 1;

    for (; it != end; it += 2) {
      using std::swap;
      swap(*it, *(it + 1));
    }
  }

  if (n > 1) {
    EXPECT_FALSE(
        std::is_sorted(insertions.begin(), insertions.end(), lowLimOrder));
  }

  // Insert them into the index out-of-order.
  for (AlignedHeapSegment *segment : insertions) {
    index.update(segment);
  }
}

AlignedHeapSegment *GCSegmentAddressIndexTest::segment(size_t i) {
  return &segments_.at(i);
}

void GCSegmentAddressIndexTest::expectSegments(
    std::vector<const AlignedHeapSegment *> expected) {
  std::sort(expected.begin(), expected.end(), lowLimOrder);

  EXPECT_EQ(expected.size(), std::distance(index.begin(), index.end()));

  auto it = index.begin();
  for (auto segment : expected) {
    EXPECT_EQ(segment, *it++);
  }
}

TEST_F(GCSegmentAddressIndexTest, Size) {
  EXPECT_EQ(0, index.size());

  addSegments(1);
  EXPECT_EQ(1, index.size());

  addSegments(2);
  EXPECT_EQ(3, index.size());

  // Updating an existing entry in the segment (because the segment was
  // std::move'd) should not cause the size of the index to increase.
  AlignedHeapSegment s{std::move(*segment(0))};
  index.update(&s);
  EXPECT_EQ(3, index.size());

  std::vector<char *> del{segment(1)->lowLim(), segment(2)->lowLim()};
  std::sort(del.begin(), del.end());
  index.remove(del.begin(), del.end());
  EXPECT_EQ(1, index.size());
}

/// Adding a new segment to an index.
TEST_F(GCSegmentAddressIndexTest, Insertion) {
  expectSegments({});
  addSegments(1);
  expectSegments({segment(0)});
}

/// Updating the entry for an existing segment, because the instance that owns
/// it has moved.
TEST_F(GCSegmentAddressIndexTest, Update) {
  addSegments(1);
  expectSegments({segment(0)});

  AlignedHeapSegment s{std::move(*segment(0))};
  index.update(&s);

  expectSegments({&s});
}

/// Iteration is in order of starting address.
TEST_F(GCSegmentAddressIndexTest, Ordering) {
  addSegments(3);

  EXPECT_TRUE(std::is_sorted(
      index.begin(),
      index.end(),
      [](const AlignedHeapSegment *a, const AlignedHeapSegment *b) {
        return a->lowLim() < b->lowLim();
      }));
}

TEST_F(GCSegmentAddressIndexTest, RemoveFromEmpty) {
  std::vector<char *> del;
  index.remove(del.begin(), del.end());
  expectSegments({});
}

TEST_F(GCSegmentAddressIndexTest, RemoveNone) {
  addSegments(1);

  std::vector<char *> del;
  index.remove(del.begin(), del.end());

  expectSegments({segment(0)});
}

TEST_F(GCSegmentAddressIndexTest, RemoveOne) {
  addSegments(3);

  std::vector<char *> del{segment(1)->lowLim()};
  index.remove(del.begin(), del.end());

  expectSegments({segment(0), segment(2)});
}

TEST_F(GCSegmentAddressIndexTest, RemoveConsecutive) {
  addSegments(4);

  std::vector<char *> del{segment(1)->lowLim(), segment(2)->lowLim()};
  std::sort(del.begin(), del.end());
  index.remove(del.begin(), del.end());

  expectSegments({segment(0), segment(3)});
}

TEST_F(GCSegmentAddressIndexTest, RemoveSparse) {
  addSegments(5);

  std::vector<char *> del{segment(1)->lowLim(), segment(3)->lowLim()};
  std::sort(del.begin(), del.end());
  index.remove(del.begin(), del.end());

  expectSegments({segment(0), segment(2), segment(4)});
}

/// An attempt to remove a key that does not exist in the index is ignored.
TEST_F(GCSegmentAddressIndexTest, RemoveNonExistent) {
  addSegments(3);

  std::vector<char *> del{segment(1)->lowLim() + 1};
  index.remove(del.begin(), del.end());

  expectSegments({segment(0), segment(1), segment(2)});
}

/// Ask for the segment covering a pointer when no such segment exists.
TEST_F(GCSegmentAddressIndexTest, CoveringNonExistent) {
  addSegments(1);

  auto *s = segment(0);
  void *ptr = s->hiLim() + 1;
  EXPECT_EQ(nullptr, index.segmentCovering(ptr));
}

/// Ask for a segment covering a pointer when that segment exists but is not in
/// the index.
TEST_F(GCSegmentAddressIndexTest, CoveringNotInIndex) {
  addSegments(1);
  auto result = AlignedStorage::create(provider_.get());
  ASSERT_TRUE(result);
  AlignedHeapSegment s{std::move(result.get())};

  void *ptr = s.start();
  EXPECT_EQ(nullptr, index.segmentCovering(ptr));
}

/// Ask for the segment covering a pointer when that segment exists and is in
/// the index.
TEST_F(GCSegmentAddressIndexTest, CoveringInIndex) {
  addSegments(1);

  auto *s = segment(0);
  void *ptr = s->start();

  EXPECT_EQ(s, index.segmentCovering(ptr));
}

#ifndef NDEBUG
/// Passing an unsorted list of elements to be removed should cause an assertion
/// failure.
TEST_F(GCSegmentAddressIndexDeathTest, RemoveUnsorted) {
  addSegments(2);

  std::vector<char *> del{segment(0)->lowLim(), segment(1)->lowLim()};
  std::sort(del.begin(), del.end());
  std::reverse(del.begin(), del.end());

  EXPECT_DEATH({ index.remove(del.begin(), del.end()); }, "Assertion");
}
#endif // !NDEBUG

#ifdef HERMES_SLOW_DEBUG
/// In debug mode, attempting to iterate will abort with an assertion failure if
/// one of the index entries is stale.
TEST_F(GCSegmentAddressIndexDeathTest, Consistency) {
  addSegments(3);

  AlignedHeapSegment v{std::move(*segment(1))};

  // The consistency check relies on the fact that when we move construct one
  // segment from another, their low lim addresses do not coincide.
  ASSERT_NE(segment(1)->lowLim(), v.lowLim());

  // We moved the contents of t into v, but did not update the index, meaning
  // the entry for address \c v.lowLim() still points to t, rather than v.  In
  // debug modes this should cause an assertion failure when we iterate over the
  // segments in the index.
  EXPECT_DEATH(
      {
        for (auto segment : index)
          (void)segment;
      },
      "Assertion");
}
#endif // HERMES_SLOW_DEBUG

} // namespace

#endif
