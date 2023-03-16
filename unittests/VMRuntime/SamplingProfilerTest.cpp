/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/VM/Runtime.h"

#include <gtest/gtest.h>

namespace {
using namespace hermes::vm;

static constexpr bool withSamplingProfilerEnabled = true;
static constexpr bool withSamplingProfilerDisabled = false;

std::shared_ptr<Runtime> makeRuntime(bool withEnableSampleProfiling) {
  auto cfg = RuntimeConfig::Builder()
                 .withEnableSampleProfiling(withEnableSampleProfiling)
                 .build();
  return Runtime::create(cfg);
}

TEST(SamplingProfilerTest, Invariants) {
  // No sample profiler registration by default
  EXPECT_TRUE(Runtime::create(RuntimeConfig{})->samplingProfiler == nullptr);

  EXPECT_TRUE(
      makeRuntime(withSamplingProfilerDisabled)->samplingProfiler == nullptr);

  auto rt = makeRuntime(withSamplingProfilerEnabled);
  ASSERT_FALSE(rt->samplingProfiler == nullptr);

  // The sample profiler belongs to this thread.
  EXPECT_TRUE(rt->samplingProfiler->belongsToCurrentThread());
}

#ifndef __APPLE__
TEST(SamplingProfilerTest, MultipleRuntimes) {
  auto rt0 = makeRuntime(withSamplingProfilerEnabled);
  auto rt1 = makeRuntime(withSamplingProfilerEnabled);
  auto rt2 = makeRuntime(withSamplingProfilerEnabled);

  EXPECT_TRUE(rt0->samplingProfiler->belongsToCurrentThread());
  EXPECT_TRUE(rt1->samplingProfiler->belongsToCurrentThread());
  EXPECT_TRUE(rt2->samplingProfiler->belongsToCurrentThread());
}

TEST(SamplingProfilerTest, MultipleProfilers) {
  auto rt = makeRuntime(withSamplingProfilerEnabled);
  auto sp0 = SamplingProfiler::create(*rt);
  auto sp1 = SamplingProfiler::create(*rt);
  auto sp2 = SamplingProfiler::create(*rt);
  EXPECT_TRUE(rt->samplingProfiler->belongsToCurrentThread());
  EXPECT_TRUE(sp0->belongsToCurrentThread());
  EXPECT_TRUE(sp1->belongsToCurrentThread());
  EXPECT_TRUE(sp2->belongsToCurrentThread());
}
#endif

} // namespace

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
