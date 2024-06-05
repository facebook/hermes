/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "TestHelpers1.h"
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

TEST(SamplingProfilerTest, MultipleThreads) {
  constexpr uint32_t kThreadCount = 10;

  auto rt = makeRuntime(withSamplingProfilerEnabled);
  rt->samplingProfiler->enable();

  // Take turns evaluating JavaScript on a different thread each time.
  auto evaluate = [&]() {
    rt->samplingProfiler->setRuntimeThread();
    auto bytecode = hermes::bytecodeForSource(R"(
              (function count() {
                let x = 0;
                function inc() {
                  x++;
                  return (x < 100000);
                }
                while(inc()){}
              })();
            )");
    std::shared_ptr<hermes::hbc::BCProviderFromBuffer> bcProvider =
        hermes::hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
            std::make_unique<hermes::Buffer>(bytecode.data(), bytecode.size()))
            .first;
    RuntimeModuleFlags runtimeModuleFlags;
    runtimeModuleFlags.persistent = false;
    auto result = rt->runBytecode(
        std::move(bcProvider),
        runtimeModuleFlags,
        "test.js.hbc",
        Runtime::makeNullHandle<Environment>());
    EXPECT_NE(result.getStatus(), ExecutionStatus::EXCEPTION);
  };
  for (uint32_t threadNumber = 0; threadNumber < kThreadCount; ++threadNumber) {
    std::thread(evaluate).join();
  }

  rt->samplingProfiler->disable();
}

} // namespace

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
