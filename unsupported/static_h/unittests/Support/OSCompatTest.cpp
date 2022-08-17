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
