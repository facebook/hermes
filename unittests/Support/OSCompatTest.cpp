/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
} // namespace
