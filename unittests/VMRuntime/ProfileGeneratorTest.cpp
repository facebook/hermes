/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"

#include <gtest/gtest.h>

namespace {

// Defines the sampling rate for the Sampling Profiler.
// This frequency allows recording frames with a duration of at least 0.1ms, but
// because the implementation of Sampling Profiler is platform specific, it is
// not guaranteed to take a snapshot every 0.1ms.
// Same frequency is specified in React Native.
static constexpr int kSamplingProfilerFrequencyHz = 10000;

std::shared_ptr<hermes::vm::Runtime> makeRuntime() {
  auto cfg = hermes::vm::RuntimeConfig::Builder()
                 .withEnableSampleProfiling(true)
                 .build();
  return hermes::vm::Runtime::create(cfg);
}

facebook::hermes::sampling_profiler::Profile executeWhileSampling(
    std::shared_ptr<hermes::vm::Runtime> runtime,
    const std::function<void()> &callback) {
  runtime->samplingProfiler->enable(kSamplingProfilerFrequencyHz);

  callback();

  runtime->samplingProfiler->disable();
  return runtime->samplingProfiler->dumpAsProfile();
}

size_t countUniqueSourceScriptURLs(
    const facebook::hermes::sampling_profiler::Profile &profile) {
  std::set<std::string> capturedUniqueSourceScriptURLs;
  for (auto const &sample : profile.getSamplesRange()) {
    for (auto const &callFrame : sample.getCallStackFramesRange()) {
      if (std::holds_alternative<facebook::hermes::sampling_profiler::
                                     ProfileSampleCallStackJSFunctionFrame>(
              callFrame)) {
        auto jsFunctionFrame =
            std::get<facebook::hermes::sampling_profiler::
                         ProfileSampleCallStackJSFunctionFrame>(callFrame);
        capturedUniqueSourceScriptURLs.insert(
            std::string(jsFunctionFrame.getScriptUrl()));
      }
    }
  }

  return capturedUniqueSourceScriptURLs.size();
}

TEST(ProfileGeneratorTest, Empty) {
  auto runtime = makeRuntime();
  auto profile = executeWhileSampling(runtime, []() {});

  EXPECT_EQ(profile.getSamplesCount(), 0);
}

TEST(ProfileGeneratorTest, MultipleUniqueScriptURLs) {
  int kNumberOfUniqueSourceScriptURLs = 64;

  auto runtime = makeRuntime();
  auto profile = executeWhileSampling(
      runtime, [runtime, kNumberOfUniqueSourceScriptURLs]() {
        std::string source = R"(
         function foo() {
           const start = Date.now();
           // Execute for 10ms to make sure we it recorded in at least one sample.
           // The sampling interval is at most 0.1ms, see kSamplingProfilerFrequencyHz.
           while (Date.now() - start < 10) {}
         }
       
         foo();
         )";
        hermes::hbc::CompileFlags flags{};

        std::vector<std::string> scriptURLs;
        for (int i = 0; i < kNumberOfUniqueSourceScriptURLs; ++i) {
          scriptURLs.push_back("file:///script" + std::to_string(i) + ".js");
          std::ignore = runtime->run(source, scriptURLs.back(), flags);
        }
      });

  EXPECT_EQ(
      countUniqueSourceScriptURLs(profile), kNumberOfUniqueSourceScriptURLs);
}

} // namespace

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
