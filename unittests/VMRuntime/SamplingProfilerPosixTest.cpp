/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifdef HERMESVM_SAMPLING_PROFILER_POSIX

#include "hermes/VM/Runtime.h"

#include <gtest/gtest.h>

namespace {
using namespace hermes::vm;

static pthread_t owningThread(const SamplingProfiler &sp) {
  return sp.getCurrentThread();
}

static constexpr bool withSamplingProfilerEnabled = true;
static constexpr bool withSamplingProfilerDisabled = false;

std::shared_ptr<Runtime> makeRuntime(bool withEnableSampleProfiling) {
  auto cfg = RuntimeConfig::Builder()
                 .withEnableSampleProfiling(withEnableSampleProfiling)
                 .build();
  return Runtime::create(cfg);
}

TEST(SamplingProfilerPosixTest, Invariants) {
  // No sample profiler registration by default
  EXPECT_EQ(Runtime::create(RuntimeConfig{})->samplingProfiler, nullptr);

  EXPECT_EQ(
      makeRuntime(withSamplingProfilerDisabled)->samplingProfiler, nullptr);

  auto rt = makeRuntime(withSamplingProfilerEnabled);
  ASSERT_NE(rt->samplingProfiler, nullptr);

  // The sample profiler belongs to this thread.
  EXPECT_EQ(owningThread(*rt->samplingProfiler), pthread_self());
}

TEST(SamplingProfilerPosixTest, MultipleRuntimes) {
  auto rt0 = makeRuntime(withSamplingProfilerEnabled);
  auto rt1 = makeRuntime(withSamplingProfilerEnabled);
  auto rt2 = makeRuntime(withSamplingProfilerEnabled);

  EXPECT_EQ(owningThread(*rt0->samplingProfiler), pthread_self());
  EXPECT_EQ(owningThread(*rt1->samplingProfiler), pthread_self());
  EXPECT_EQ(owningThread(*rt2->samplingProfiler), pthread_self());
}

TEST(SamplingProfilerPosixTest, MultipleProfilers) {
  auto rt = makeRuntime(withSamplingProfilerEnabled);
  auto sp0 = std::make_unique<SamplingProfiler>(*rt);
  auto sp1 = std::make_unique<SamplingProfiler>(*rt);
  auto sp2 = std::make_unique<SamplingProfiler>(*rt);
  EXPECT_EQ(owningThread(*rt->samplingProfiler), pthread_self());
  EXPECT_EQ(owningThread(*sp0), pthread_self());
  EXPECT_EQ(owningThread(*sp1), pthread_self());
  EXPECT_EQ(owningThread(*sp2), pthread_self());
}

} // namespace

#endif // HERMESVM_SAMPLING_PROFILER_POSIX
