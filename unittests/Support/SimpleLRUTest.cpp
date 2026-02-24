/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/ADT/SimpleLRU.h"

namespace {

using namespace hermes;

TEST(SimpleLRUTest, Empty) {
  SimpleLRU<int> lru;
  EXPECT_TRUE(lru.empty());
}

TEST(SimpleLRUTest, AddAndLeastRecent) {
  SimpleLRU<int> lru;

  lru.add(10);
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(10, *lru.leastRecent());

  lru.add(20);
  // First added is least recent.
  EXPECT_EQ(10, *lru.leastRecent());

  lru.add(30);
  EXPECT_EQ(10, *lru.leastRecent());
}

TEST(SimpleLRUTest, Use) {
  SimpleLRU<int> lru;

  int *p10 = lru.add(10);
  int *p20 = lru.add(20);
  int *p30 = lru.add(30);

  // Order is now: 30 (most recent) -> 20 -> 10 (least recent)
  EXPECT_EQ(10, *lru.leastRecent());

  // Mark 10 as used, making it most recent.
  lru.use(p10);
  // Order is now: 10 -> 30 -> 20
  EXPECT_EQ(20, *lru.leastRecent());

  // Mark 20 as used.
  lru.use(p20);
  // Order is now: 20 -> 10 -> 30
  EXPECT_EQ(30, *lru.leastRecent());

  // Using the most recent should be a no-op.
  lru.use(p20);
  EXPECT_EQ(30, *lru.leastRecent());

  (void)p30;
}

TEST(SimpleLRUTest, Remove) {
  SimpleLRU<int> lru;

  int *p10 = lru.add(10);
  int *p20 = lru.add(20);
  int *p30 = lru.add(30);

  // Order: 30 -> 20 -> 10
  EXPECT_EQ(10, *lru.leastRecent());

  // Remove the least recent.
  lru.remove(p10);
  EXPECT_EQ(20, *lru.leastRecent());

  // Remove from the middle.
  lru.remove(p20);
  EXPECT_EQ(30, *lru.leastRecent());

  // Remove the last one.
  lru.remove(p30);
  EXPECT_TRUE(lru.empty());
}

TEST(SimpleLRUTest, RemoveAndReuse) {
  SimpleLRU<int> lru;

  int *p10 = lru.add(10);
  lru.add(20);

  // Remove and add again - should reuse the freed node.
  lru.remove(p10);
  int *p30 = lru.add(30);
  EXPECT_EQ(30, *p30);
  EXPECT_EQ(20, *lru.leastRecent());
}

/// This test would have caught the bug where std::vector was used instead of
/// std::deque. When vector reallocates, all pointers become invalid. With ASAN
/// enabled, accessing through stale pointers would be detected as
/// use-after-free.
TEST(SimpleLRUTest, PointerStabilityAfterGrowth) {
  SimpleLRU<int> lru;

  // Store pointers to elements as we add them.
  std::vector<int *> pointers;
  constexpr int kNumElements = 1000;

  for (int i = 0; i < kNumElements; ++i) {
    pointers.push_back(lru.add(i));
  }

  // Verify all pointers are still valid and point to correct values.
  // With the old std::vector implementation, this would be use-after-free
  // because vector reallocation invalidates pointers.
  for (int i = 0; i < kNumElements; ++i) {
    EXPECT_EQ(i, *pointers[i]) << "Pointer " << i << " is invalid";
  }

  // Also verify use() works with old pointers after growth.
  lru.use(pointers[0]);
  EXPECT_EQ(1, *lru.leastRecent());

  // And remove() works too.
  lru.remove(pointers[500]);
  // Value at index 1 should now be least recent (0 was moved to front, 500
  // removed).
  EXPECT_EQ(1, *lru.leastRecent());
}

TEST(SimpleLRUTest, StructValue) {
  struct Data {
    int x;
    std::string s;
  };

  SimpleLRU<Data> lru;

  Data *p1 = lru.add({1, "one"});
  Data *p2 = lru.add({2, "two"});

  EXPECT_EQ(1, lru.leastRecent()->x);
  EXPECT_EQ("one", lru.leastRecent()->s);

  lru.use(p1);
  EXPECT_EQ(2, lru.leastRecent()->x);
  EXPECT_EQ("two", lru.leastRecent()->s);

  (void)p2;
}

} // namespace
