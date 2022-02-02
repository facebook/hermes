/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/VM/Profiler/SamplingProfiler.h>

#ifdef HERMESVM_SAMPLING_PROFILER_POSIX

#include "hermes/hermes.h"

#include <gtest/gtest.h>

#include <iostream>
#include <thread>

namespace {
using namespace facebook::hermes;
namespace vm = ::hermes::vm;

static constexpr bool withSamplingProfilerEnabled = true;
static constexpr bool withSamplingProfilerDisabled = false;

// Creates a HermesRuntime with a fatal handler that outputs the error string to
// std::cer.
std::unique_ptr<HermesRuntime> makeRuntime(bool withEnableSampleProfiling) {
  auto cfg = vm::RuntimeConfig::Builder()
                 .withEnableSampleProfiling(withEnableSampleProfiling)
                 .build();
  auto rt = makeHermesRuntime(cfg);
  rt->setFatalHandler(
      [](const std::string &str) { std::cerr << str << std::flush; });
  return rt;
}

TEST(SamplingProfilerPosixHermesAPITest, ReregistrationIsAnError) {
  EXPECT_DEATH(
      makeRuntime(withSamplingProfilerEnabled)->registerForProfiling(),
      "re-registering HermesVMs for profiling is not allowed");
}

TEST(SamplingProfilerPosixHermesAPITest, DeregisteringUnregisteredIsAnError) {
  EXPECT_DEATH(
      makeRuntime(withSamplingProfilerDisabled)->unregisterForProfiling(),
      "unregistering HermesVM not registered for profiling is not allowed");
}

TEST(
    SamplingProfilerPosixHermesAPITest,
    DeregisteringInDifferentThreadIsAnError) {
  EXPECT_DEATH(
      [] {
        auto rt = makeRuntime(withSamplingProfilerEnabled);
        std::thread([&] { rt->unregisterForProfiling(); }).join();
      }(),
      "SamplingProfiler should be destroyed on the same thread it is created");
}

} // namespace

#endif // HERMESVM_SAMPLING_PROFILER_POSIX
