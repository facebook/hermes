/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/TraceInterpreter.h>
#include <hermes/hermes.h>

#include <gtest/gtest.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>

#include <memory>
#include <string>
#include <vector>

#include "tests/TestFunctions.h"

using ParamType = std::tuple<
    const char *,
    std::function<const char *()>,
    std::function<const char *()>>;

#define TEST_FUNC_MAKE_TUPLE(name)               \
  ParamType{                                     \
      #name,                                     \
      facebook::hermes::synthtest::name##Trace,  \
      facebook::hermes::synthtest::name##Source, \
  },

namespace {

using namespace facebook::hermes;
using namespace hermes::vm;

class SynthBenchmarkTestFixture : public ::testing::TestWithParam<ParamType> {
 protected:
  SynthBenchmarkTestFixture()
      : testName_(std::get<0>(GetParam())),
        trace_(std::get<1>(GetParam())()),
        source_(std::get<2>(GetParam())()) {}
  const char *testName_;
  const char *trace_;
  const char *source_;
};

TEST_P(SynthBenchmarkTestFixture, CanRunWithoutException) {
  tracing::TraceInterpreter::ExecuteOptions options;
  // Use options from the trace.
  options.minHeapSize = 0;
  options.maxHeapSize = 0;
  EXPECT_NO_THROW({
    tracing::TraceInterpreter::execFromMemoryBuffer(
        llvm::MemoryBuffer::getMemBuffer(trace_),
        llvm::MemoryBuffer::getMemBuffer(source_),
        options,
        nullptr);
  }) << "Failed on test: "
     << testName_;
}

std::vector<ParamType> testGenerator() {
  return {FOREACH_TEST(TEST_FUNC_MAKE_TUPLE)};
}
INSTANTIATE_TEST_CASE_P(
    ExceptionTest,
    SynthBenchmarkTestFixture,
    ::testing::ValuesIn(testGenerator()));

} // namespace
