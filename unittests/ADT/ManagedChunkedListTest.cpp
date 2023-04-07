/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ADT/ManagedChunkedList.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes;

// A ManagedChunkedList element that indicates whether it's occupied based on a
// member boolean.
struct ManagedValue {
  ManagedValue() {}

  bool isFree() {
    return !occupied_;
  }

  void emplace(int value) {
    value_ = value;
    occupied_ = true;
  }

  void free() {
    occupied_ = false;
  }

  ManagedValue *getNextFree() {
    return nextFree_;
  }

  void setNextFree(ManagedValue *nextFree) {
    nextFree_ = nextFree;
  }

  int value() {
    return value_;
  }

 private:
  bool occupied_ = false;
  union {
    int value_;
    ManagedValue *nextFree_;
  };
};

constexpr double kOccupancyRatio = 0.5;
constexpr double kSizingWeight = 0.5;
constexpr size_t kElementsPerChunk = 16;
using IntegerList = ManagedChunkedList<ManagedValue, kElementsPerChunk>;

TEST(ManagedChunkListTest, Empty) {
  IntegerList list(kOccupancyRatio, kSizingWeight);
  EXPECT_EQ(0, list.capacity());
  EXPECT_EQ(0, list.sizeForTests());
}

TEST(ManagedChunkListTest, Add) {
  IntegerList list(kOccupancyRatio, kSizingWeight);
  list.add(1);
  EXPECT_GT(list.capacity(), 0);
  EXPECT_EQ(1, list.sizeForTests());
}

TEST(ManagedChunkListTest, Remove) {
  IntegerList list(kOccupancyRatio, kSizingWeight);

  // Insert some values
  list.add(111);
  list.add(222);
  list.add(333);
  EXPECT_EQ(3, list.sizeForTests());

  // Remove the odd values
  list.forEach([](ManagedValue &element) {
    if ((element.value() % 2) == 1) {
      element.free();
    }
  });

  // Ensure removed values are ignored.
  EXPECT_EQ(1, list.sizeForTests());
  list.forEach([](ManagedValue &element) { EXPECT_EQ(222, element.value()); });
}

TEST(ManagedChunkListTest, OnlyAllocateWhenNecessary) {
  IntegerList list(kOccupancyRatio, kSizingWeight);

  // Populate exactly one chunk
  for (size_t value = 0; value < kElementsPerChunk; value++) {
    list.add(value);
  }
  EXPECT_EQ(kElementsPerChunk, list.sizeForTests());
  EXPECT_EQ(kElementsPerChunk, list.capacity());

  // Exceed the capacity of a single chunk
  list.add(1234);

  // Ensure the capacity grew.
  EXPECT_EQ(kElementsPerChunk + 1, list.sizeForTests());
  EXPECT_GT(list.capacity(), kElementsPerChunk);
}

TEST(ManagedChunkListTest, CollectionReusesFreeItems) {
  constexpr size_t kChunkCount = 3;
  IntegerList list(kOccupancyRatio, kSizingWeight);

  // Add exactly enough values to fill kChunkCount chunks.
  size_t oddSum = 0;
  size_t evenSum = 0;
  for (size_t value = 0; value < kChunkCount * kElementsPerChunk; value++) {
    list.add(value);

    if ((value % 2) == 0) {
      evenSum += value;
    } else {
      oddSum += value;
    }
  }
  EXPECT_EQ(kChunkCount * kElementsPerChunk, list.capacity());

  // Remove odd values
  list.forEach([](ManagedValue &element) {
    if ((element.value() % 2) == 1) {
      element.free();
    }
  });
  list.collect();
  EXPECT_EQ(kChunkCount * kElementsPerChunk / 2, list.sizeForTests());
  EXPECT_EQ(kChunkCount * kElementsPerChunk, list.capacity());

  // Reuse odd slots, inserting a value 3x as large
  for (size_t value = 0; value < kChunkCount * kElementsPerChunk; value++) {
    if ((value % 2) == 1) {
      list.add(value * 3);
    }
  }

  // Ensure all elements are present with no additional capacity.
  EXPECT_EQ(kChunkCount * kElementsPerChunk, list.sizeForTests());
  EXPECT_EQ(kChunkCount * kElementsPerChunk, list.capacity());
  size_t sum = 0;
  list.forEach([&sum](ManagedValue &element) { sum += element.value(); });
  EXPECT_EQ(evenSum + oddSum * 3, sum);
}

TEST(ManagedChunkListTest, CollectionIsTriggered) {
  // Ensure storing many values one at a time doesn't allocate
  // extra chunks; collections should keep the capacity small.
  constexpr size_t kElementCount = kElementsPerChunk * 100;
  IntegerList list(kOccupancyRatio, kSizingWeight);
  for (size_t value = 0; value < kElementCount; value++) {
    list.add(value).free();
  }
  EXPECT_EQ(kElementsPerChunk, list.capacity());
}

} // namespace
