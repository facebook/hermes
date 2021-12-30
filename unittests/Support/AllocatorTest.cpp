/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Allocator.h"

#include "gtest/gtest.h"
#include "llvh/Support/MathExtras.h"

#include <array>
#include <type_traits>

using namespace hermes;

TEST(AllocatorTest, SmokeTest) {
  BumpPtrAllocator alloc;
  auto *p = alloc.Allocate<uint64_t>();
  *p = 42;
}

TEST(AllocatorTest, CanAllocate) {
  constexpr unsigned size = 1 << 20;
  BumpPtrAllocator alloc;
  uint32_t **ptr = alloc.Allocate<uint32_t *>(size);
  for (unsigned i = 0; i < size; i++) {
    ptr[i] = alloc.Allocate<uint32_t>();
    *ptr[i] = i;
  }
  for (unsigned i = 0; i < size; i++) {
    EXPECT_EQ(i, *ptr[i]);
  }
}

TEST(AllocatorTest, VariableSizeAllocation) {
  // Total number of allocations to do. Size of each allocation varies.
  constexpr unsigned int allocCount = 1000;
  BumpPtrAllocator alloc;

  unsigned char **ptr = alloc.Allocate<unsigned char *>(allocCount);
  for (unsigned int allocIndex = 0; allocIndex < allocCount; ++allocIndex) {
    ptr[allocIndex] = alloc.Allocate<unsigned char>(allocIndex);

    // Write alloc index into array as test value.
    for (unsigned int byteIndex = 0; byteIndex < allocIndex; ++byteIndex) {
      // Ensure that test value fits in one byte.
      unsigned char testValue = allocIndex & 0xff;
      ptr[allocIndex][byteIndex] = testValue;
    }
  }

  // Test that bytes are read back correctly.
  for (unsigned int allocIndex = 0; allocIndex < allocCount; ++allocIndex) {
    for (unsigned int byteIndex = 0; byteIndex < allocIndex; ++byteIndex) {
      unsigned char testValue = allocIndex & 0xff;
      EXPECT_EQ(testValue, ptr[allocIndex][byteIndex]);
    }
  }
}

TEST(AllocatorTest, NewScopeSameMemory) {
  BumpPtrAllocator alloc;
  uint64_t *p;
  {
    AllocationScope scope(alloc);
    p = alloc.Allocate<uint64_t>();
  }
  EXPECT_EQ(p, alloc.Allocate<uint64_t>());
}

TEST(AllocatorTest, CanAlign) {
  BumpPtrAllocator alloc;
  for (int round = 0; round < 100; round++) {
    for (int i = 0; i < 16; i++) {
      auto p = reinterpret_cast<uintptr_t>(
          alloc.Allocate<uint64_t>(1, size_t(1) << i));
      EXPECT_EQ(p, llvh::alignTo(p, 1 << i));
    }
  }
}

TEST(AllocatorTest, CanAllocateHuge) {
  BumpPtrAllocator alloc;
  constexpr unsigned size = 1 << 24;
  auto *p = alloc.Allocate<char[size]>();
  memset(p, 'x', size);
}
