/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_API_TRACE

#include <hermes/TraceInterpreter.h>
#include <hermes/hermes.h>

#include <gtest/gtest.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SHA1.h>

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
  std::vector<std::unique_ptr<llvm::MemoryBuffer>> sources;
  sources.emplace_back(llvm::MemoryBuffer::getMemBuffer(source_));
  EXPECT_NO_THROW({
    tracing::TraceInterpreter::execFromMemoryBuffer(
        llvm::MemoryBuffer::getMemBuffer(trace_),
        std::move(sources),
        options,
        nullptr);
  }) << "Failed on test: "
     << testName_;
}

TEST(SynthBenchmark, RunMultipleSourceFiles) {
  tracing::TraceInterpreter::ExecuteOptions options;
  const char *const traceFmt = R"(
    {
      "globalObjID": 1,
      "env": {
        "mathRandomSeed": 0,
        "callsToDateNow": [],
        "callsToNewDate": [],
        "callsToDateAsFunction": [],
        "callsToHermesInternalGetInstrumentedStats": [],
      },
      "trace": [
        {
          "type": "BeginExecJSRecord",
          "time": 0,
          "sourceHash": "%s"
        },
        {
          "type": "EndExecJSRecord",
          "retval": "string:hello",
          "time": 0
        },
        {
          "type": "BeginExecJSRecord",
          "time": 0,
          "sourceHash": "%s"
        },
        {
          "type": "EndExecJSRecord",
          "retval": "string:goodbye",
          "time": 0
        }
      ]
    })";
  std::string source1 = R"("hello";)";
  std::string source2 = R"("goodbye";)";
  std::string trace;
  {
    auto hash1 = llvm::SHA1::hash(llvm::makeArrayRef(
        reinterpret_cast<const uint8_t *>(source1.c_str()), source1.length()));
    auto hash2 = llvm::SHA1::hash(llvm::makeArrayRef(
        reinterpret_cast<const uint8_t *>(source2.c_str()), source2.length()));
    std::string hash1AsStr = ::hermes::hashAsString(hash1);
    std::string hash2AsStr = ::hermes::hashAsString(hash2);
    int numChars =
        snprintf(nullptr, 0, traceFmt, hash1AsStr.c_str(), hash2AsStr.c_str());
    char *buf = new char[numChars + 1];
    snprintf(
        buf, numChars + 1, traceFmt, hash1AsStr.c_str(), hash2AsStr.c_str());
    trace = std::string(buf);
    delete[] buf;
  }
  std::vector<std::unique_ptr<llvm::MemoryBuffer>> sources;
  sources.emplace_back(llvm::MemoryBuffer::getMemBuffer(source1));
  sources.emplace_back(llvm::MemoryBuffer::getMemBuffer(source2));
  EXPECT_NO_THROW({
    tracing::TraceInterpreter::execFromMemoryBuffer(
        llvm::MemoryBuffer::getMemBuffer(trace),
        std::move(sources),
        options,
        nullptr);
  });
}

std::vector<ParamType> testGenerator() {
  return {FOREACH_TEST(TEST_FUNC_MAKE_TUPLE)};
}
INSTANTIATE_TEST_CASE_P(
    ExceptionTest,
    SynthBenchmarkTestFixture,
    ::testing::ValuesIn(testGenerator()));

} // namespace

#endif
