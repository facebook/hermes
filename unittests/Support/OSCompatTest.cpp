/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes;
TEST(OSCompatTest, PeakRSS) {
  const auto beginPeakRSS = oscompat::peak_rss();
  if (!beginPeakRSS) {
    // On platforms where the RSS can't be determined, skip the rest of this
    // test.
    return;
  }
  const auto pageSize = oscompat::page_size();
  {
    // Do some allocation work to raise the peak RSS.
    std::vector<std::unique_ptr<int[]>> memories;
    for (int i = 0; i < 100; i++) {
      memories.emplace_back(new int[10 * pageSize]);
    }
    // Let the memory get freed so it doesn't count towards current memory, but
    // it does affect the peak memory.
  }
  // If the OS is exact, it should be strictly greater than, but sometimes it
  // isn't tracked with fine granularity.
  EXPECT_GE(oscompat::peak_rss(), beginPeakRSS);
}

TEST(OSCompatTest, CurrentRSS) {
  const auto beginRSS = oscompat::current_rss();
  if (!beginRSS) {
    // On platforms where the RSS can't be determined, skip the rest of this
    // test.
    return;
  }
  const auto pageSize = oscompat::page_size();
  // Do some allocation work to raise the peak RSS.
  std::vector<std::unique_ptr<int[]>> memories;
  for (int i = 0; i < 100; i++) {
    memories.emplace_back(new int[10 * pageSize]);
  }
  // If the OS is exact, it should be strictly greater than, but sometimes it
  // isn't tracked with fine granularity.
  EXPECT_GE(oscompat::current_rss(), beginRSS);
}

TEST(OSCompatTest, CpuCycles) {
  uint64_t start = oscompat::cpu_cycle_counter();
  uint64_t end = oscompat::cpu_cycle_counter();
  EXPECT_LE(start, end);
}

static constexpr size_t StorageSize = 4 * 1024 * 1024;

TEST(OSCompatTest, VmCommitNull) {
  // Committing a nullptr should not succeed on any platform
  auto result = oscompat::vm_commit(nullptr, StorageSize);
  EXPECT_FALSE(result);
}

TEST(OSCompatTest, VmDoubleCommit) {
  // Double committing the same memory should succeed on every platform
  auto result = oscompat::vm_reserve_aligned(StorageSize, StorageSize);
  EXPECT_TRUE(oscompat::vm_commit(*result, StorageSize));
  EXPECT_TRUE(oscompat::vm_commit(*result, StorageSize));
}

// TODO: T142209580
//
// These tests, which expect different results per platform, are documenting the
// fact that right now, Windows and Posix differ in semantics when it comes
// sending non-reserved addresses into vm_commit. Posix treats the address as a
// hint and will perform a commit; Windows will reject the request.
//
// Moreover, Posix internally uses MAP_FIXED flags, which will stomp existing
// commits of that address if they exist, which seems to open it up to race
// conditions? If possible, we may wish to explore the use of
// MAP_FIXED_NOREPLACE rather than MAP_FIXED.
#ifdef _MSC_VER
TEST(OSCompatTest, VmCommitAfterRelease) {
  auto result = oscompat::vm_reserve_aligned(StorageSize, StorageSize);
  oscompat::vm_release_aligned(*result, StorageSize);
  EXPECT_FALSE(oscompat::vm_commit(*result, StorageSize));
}
#endif

#ifdef __linux__
TEST(OSCompatTest, VmCommitAfterRelease) {
  // Committing reserved-then-released memory should not succeed on any platform
  auto result = oscompat::vm_reserve_aligned(StorageSize, StorageSize);
  oscompat::vm_release_aligned(*result, StorageSize);
  EXPECT_TRUE(oscompat::vm_commit(*result, StorageSize));
}
#endif

#ifdef __linux__
TEST(OSCompatTest, Scheduling) {
  // At least one CPU should be set.
  std::vector<bool> mask = oscompat::sched_getaffinity();
  EXPECT_GE(mask.size(), 1u);
  unsigned count = 0;
  for (auto b : mask)
    if (b)
      ++count;
  EXPECT_GE(count, 1u);

  int cpu = oscompat::sched_getcpu();
  ASSERT_GE(cpu, 0);
  ASSERT_LE(cpu, (int)mask.size());
  EXPECT_TRUE(mask[cpu]);
}

TEST(OSCompatTest, GetProtections) {
  {
    static const char k = 'k';
    auto modes = oscompat::get_vm_protect_modes(&k, 1);
    ASSERT_EQ(modes.size(), 1);
    EXPECT_EQ(modes[0][0], 'r');
    EXPECT_EQ(modes[0][1], '-');
  }
  {
    char arr[1];
    auto modes = oscompat::get_vm_protect_modes(arr, 1);
    ASSERT_EQ(modes.size(), 1);
    EXPECT_EQ(modes[0][0], 'r');
    EXPECT_EQ(modes[0][1], 'w');
  }
  {
    auto modes = oscompat::get_vm_protect_modes(nullptr, 0);
    EXPECT_EQ(modes.size(), 0);
  }
}
#endif

} // namespace
