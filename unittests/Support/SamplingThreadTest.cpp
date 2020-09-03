/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/Support/SamplingThread.h"

namespace {

TEST(SamplingThreadTest, Counter) {
  constexpr int kStart = 100;
  constexpr int kStep = 2;
  // Chosen to make the test take O(100ms) on my test machine,
  constexpr int kIterations = 10 * 1000 * 1000;

  std::atomic<int> counter{kStart};
  hermes::SamplingThread<int> sampler{counter, std::chrono::milliseconds(1)};

  for (unsigned i = 0; i < kIterations; ++i) {
    counter.fetch_add(kStep, std::memory_order_relaxed);
  }

  auto samples = sampler.stop();
  for (unsigned i = 0; i < samples.size(); ++i) {
    // Sampled values should be even numbers within the expected range.
    int v = samples[i].second;
    EXPECT_LE(kStart, v);
    EXPECT_LE(v, counter.load());
    EXPECT_EQ(0, v % kStep);

    // Both the timestamps and observed counts should progress monotonically.
    if (i > 0) {
      EXPECT_LE(samples[i - 1].first, samples[i].first);
      EXPECT_LE(samples[i - 1].second, samples[i].second);
    }
  }
}

} // namespace
