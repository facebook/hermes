/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "gtest/gtest.h"

#include "hermes/VM/Profiler/CodeCoverageProfiler.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

#include <algorithm>
#include <future>
#include <unordered_map>

using namespace hermes::vm;

namespace hermes {
namespace unittest {
namespace CodeCoverageTest {

class CodeCoverageProfilerTest : public RuntimeTestFixture {
 public:
  CodeCoverageProfilerTest() {
    CodeCoverageProfiler::enableGlobal();
  }

 protected:
  static CodeCoverageProfiler::FuncInfo getFuncInfo(
      Runtime &runtime,
      Handle<JSFunction> func) {
    auto bcProvider =
        func->getCodeBlock(runtime)->getRuntimeModule()->getBytecode();
    auto functionId = func->getCodeBlock(runtime)->getFunctionID();
    auto debugInfo = bcProvider->getDebugInfo();
    auto debugOffsets = bcProvider->getDebugOffsets(functionId);
    if (debugInfo && debugOffsets &&
        debugOffsets->sourceLocations != hbc::DebugOffsets::NO_OFFSET) {
      if (auto pos = debugInfo->getLocationForAddress(
              debugOffsets->sourceLocations, 0 /* opcodeOffset */)) {
        auto file = debugInfo->getFilenameByID(pos->filenameId);
        auto line = pos->line - 1; // Normalised to zero-based
        auto column = pos->column - 1; // Normalised to zero-based
        return {line, column, file};
      }
    }
    const uint32_t segmentID = bcProvider->getSegmentID();
    const uint32_t funcVirtualOffset =
        bcProvider->getVirtualOffsetForFunction(functionId);
    const std::string sourceURL =
        func->getRuntimeModule(runtime)->getSourceURL();
    return {segmentID, funcVirtualOffset, sourceURL};
  }

  // Check if the function is executed or not.
  static bool isFuncExecuted(
      Runtime &runtime,
      const std::vector<CodeCoverageProfiler::FuncInfo> &executedFuncInfos,
      Handle<JSFunction> checkFunc) {
    CodeCoverageProfiler::FuncInfo checkFuncInfo =
        getFuncInfo(runtime, checkFunc);
    return std::find(
               executedFuncInfos.begin(),
               executedFuncInfos.end(),
               checkFuncInfo) != executedFuncInfos.end();
  }

  // Whether \p func1 and \p func2 have different FuncInfo
  // or not.
  static ::testing::AssertionResult hasDifferentInfo(
      Runtime &runtime,
      Handle<JSFunction> func1,
      Handle<JSFunction> func2) {
    if (getFuncInfo(runtime, func1) == getFuncInfo(runtime, func2)) {
      return ::testing::AssertionFailure()
          << "func1 and func2 has same func info.";
    } else {
      return ::testing::AssertionSuccess();
    }
  }
};

TEST_F(CodeCoverageProfilerTest, BasicFunctionUsedUnused) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res = runtime.run(
      "function used() {}; function unused() {}; used(); [used, unused];",
      "file:///fake.js",
      flags);
  EXPECT_FALSE(isException(res));

  std::unordered_map<std::string, std::vector<CodeCoverageProfiler::FuncInfo>>
      executedFuncInfos = CodeCoverageProfiler::getExecutedFunctions();
  std::vector<CodeCoverageProfiler::FuncInfo> testRuntimeExecutedFuncInfos =
      executedFuncInfos.find(runtime.getHeap().getName())->second;

  Handle<JSArray> funcArr = runtime.makeHandle(vmcast<JSArray>(*res));
  Handle<JSFunction> funcUsed = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 0).getObject(runtime)));
  Handle<JSFunction> funcUnused = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 1).getObject(runtime)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(runtime, funcUsed, funcUnused));

  // Global + used.
  EXPECT_EQ(testRuntimeExecutedFuncInfos.size(), 2);
  EXPECT_TRUE(isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUsed));
  EXPECT_FALSE(
      isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUnused));
}

// Right now, this just tests that we can simultaneously run two code coverage
// profilers.
TEST_F(CodeCoverageProfilerTest, BasicFunctionUsedUnusedTwoRuntimes) {
  auto runtime2 = newRuntime();
  GCScope scope{*runtime2};
  std::vector<Runtime *> runtimes = {&runtime, runtime2.get()};

  std::vector<std::future<PinnedHermesValue>> resFuts;

  for (auto *rt : runtimes) {
    resFuts.push_back(
        std::async(std::launch::async, [this, rt]() -> PinnedHermesValue {
          hbc::CompileFlags flags;
          flags.lazy = false;
          CallResult<HermesValue> res = rt->run(
              "function used() {}; function unused() {}; used(); [used, unused];",
              "file:///fake.js",
              flags);
          EXPECT_FALSE(isException(res));
          return *res;
        }));
  }

  for (size_t i = 0; i < runtimes.size(); i++) {
    Runtime &rt = *runtimes[i];
    HermesValue res = resFuts[i].get();

    std::vector<CodeCoverageProfiler::FuncInfo> executedFuncInfos =
        rt.getCodeCoverageProfiler().getExecutedFunctionsLocal();

    Handle<JSArray> funcArr = rt.makeHandle(vmcast<JSArray>(res));
    Handle<JSFunction> funcUsed =
        rt.makeHandle(vmcast<JSFunction>(funcArr->at(rt, 0).getObject(rt)));
    Handle<JSFunction> funcUnused =
        rt.makeHandle(vmcast<JSFunction>(funcArr->at(rt, 1).getObject(rt)));

    // Used and unused functions should have different info.
    EXPECT_TRUE(hasDifferentInfo(rt, funcUsed, funcUnused));

    // Global + used.
    EXPECT_EQ(executedFuncInfos.size(), 2);
    EXPECT_TRUE(isFuncExecuted(rt, executedFuncInfos, funcUsed));
    EXPECT_FALSE(isFuncExecuted(rt, executedFuncInfos, funcUnused));
  }
}

TEST_F(CodeCoverageProfilerTest, FunctionsFromMultipleModules) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res1 =
      runtime.run("function foo() {}; foo(); foo;", "file:///fake1.js", flags);
  EXPECT_FALSE(isException(res1));
  Handle<JSFunction> funcFoo = runtime.makeHandle(vmcast<JSFunction>(*res1));

  CallResult<HermesValue> res2 = runtime.run(
      "\n  function bar() {}; function bar() {}; function unused() {}; bar(); [bar, unused];",
      "file:///fake2.js",
      flags);
  EXPECT_FALSE(isException(res2));

  std::unordered_map<std::string, std::vector<CodeCoverageProfiler::FuncInfo>>
      executedFuncInfos = CodeCoverageProfiler::getExecutedFunctions();
  std::vector<CodeCoverageProfiler::FuncInfo> testRuntimeExecutedFuncInfos =
      executedFuncInfos.find(runtime.getHeap().getName())->second;

  Handle<JSArray> funcArr = runtime.makeHandle(vmcast<JSArray>(*res2));
  Handle<JSFunction> funcBar = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 0).getObject(runtime)));
  Handle<JSFunction> funcUnused = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 1).getObject(runtime)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(runtime, funcFoo, funcUnused));
  EXPECT_TRUE(hasDifferentInfo(runtime, funcBar, funcUnused));

  EXPECT_EQ(testRuntimeExecutedFuncInfos.size(), 4);
  EXPECT_TRUE(isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcFoo));
  EXPECT_TRUE(isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcBar));
  EXPECT_FALSE(
      isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUnused));
}

TEST_F(CodeCoverageProfilerTest, FunctionsFromMultipleDomains) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res = runtime.run(
      "var eval1 = eval('const a = 1; function used1() {}; used1(); used1; //# sourceURL=foo1.js'); "
      "var eval2 = eval('const b = 1; function unused() {}; function used2() {}; used2(); [used2, unused] //# sourceURL=foo2.js');"
      "[eval1, eval2[0], eval2[1]];",
      "file:///fake.js",
      flags);
  EXPECT_FALSE(isException(res));

  std::unordered_map<std::string, std::vector<CodeCoverageProfiler::FuncInfo>>
      executedFuncInfos = CodeCoverageProfiler::getExecutedFunctions();
  std::vector<CodeCoverageProfiler::FuncInfo> testRuntimeExecutedFuncInfos =
      executedFuncInfos.find(runtime.getHeap().getName())->second;

  Handle<JSArray> funcArr = runtime.makeHandle(vmcast<JSArray>(*res));
  Handle<JSFunction> funcUsed1 = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 0).getObject(runtime)));
  Handle<JSFunction> funcUsed2 = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 1).getObject(runtime)));
  Handle<JSFunction> funcUnused = runtime.makeHandle(
      vmcast<JSFunction>(funcArr->at(runtime, 2).getObject(runtime)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(runtime, funcUsed1, funcUnused));
  EXPECT_TRUE(hasDifferentInfo(runtime, funcUsed2, funcUnused));

  // Global + two eval() code + used1 + used2.
  EXPECT_EQ(testRuntimeExecutedFuncInfos.size(), 5);
  EXPECT_TRUE(isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUsed1));
  EXPECT_TRUE(isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUsed2));
  EXPECT_FALSE(
      isFuncExecuted(runtime, testRuntimeExecutedFuncInfos, funcUnused));
}

} // namespace CodeCoverageTest
} // namespace unittest
} // namespace hermes
