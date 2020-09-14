/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

using namespace hermes::vm;

namespace hermes {
namespace unittest {
namespace CodeCoverageTest {

class CodeCoverageProfilerTest : public RuntimeTestFixture {
 public:
  CodeCoverageProfilerTest() : profiler(CodeCoverageProfiler::getInstance()) {
    profiler->enable();
  }

 protected:
  static CodeCoverageProfiler::FuncInfo getFuncInfo(Handle<JSFunction> func) {
    auto bcProvider = func->getCodeBlock()->getRuntimeModule()->getBytecode();
    const uint32_t moduleId = bcProvider->getCJSModuleOffset();
    const uint32_t funcVirtualOffset = bcProvider->getVirtualOffsetForFunction(
        func->getCodeBlock()->getFunctionID());
    return {moduleId, funcVirtualOffset};
  }

  // Check if the function is executed or not.
  static bool isFuncExecuted(
      const std::vector<CodeCoverageProfiler::FuncInfo> &executedFuncInfos,
      Handle<JSFunction> checkFunc) {
    CodeCoverageProfiler::FuncInfo checkFuncInfo = getFuncInfo(checkFunc);
    return std::find(
               executedFuncInfos.begin(),
               executedFuncInfos.end(),
               checkFuncInfo) != executedFuncInfos.end();
  }

  // Whether \p func1 and \p func2 have different FuncInfo
  // or not.
  static ::testing::AssertionResult hasDifferentInfo(
      Handle<JSFunction> func1,
      Handle<JSFunction> func2) {
    if (getFuncInfo(func1) == getFuncInfo(func2)) {
      return ::testing::AssertionFailure()
          << "func1 and func2 has same func info.";
    } else {
      return ::testing::AssertionSuccess();
    }
  }

 protected:
  std::shared_ptr<CodeCoverageProfiler> profiler;
};

TEST_F(CodeCoverageProfilerTest, BasicFunctionUsedUnused) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res = runtime->run(
      "function used() {}; function unused() {}; used(); [used, unused];",
      "file:///fake.js",
      flags);
  EXPECT_FALSE(isException(res));

  std::vector<CodeCoverageProfiler::FuncInfo> executedFuncInfos =
      profiler->getExecutedFunctions();

  Handle<JSArray> funcArr = runtime->makeHandle(vmcast<JSArray>(*res));
  Handle<JSFunction> funcUsed =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 0)));
  Handle<JSFunction> funcUnused =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 1)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(funcUsed, funcUnused));

  // Global + used.
  EXPECT_EQ(executedFuncInfos.size(), 2);
  EXPECT_TRUE(isFuncExecuted(executedFuncInfos, funcUsed));
  EXPECT_FALSE(isFuncExecuted(executedFuncInfos, funcUnused));
}

TEST_F(CodeCoverageProfilerTest, FunctionsFromMultipleModules) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res1 =
      runtime->run("function foo() {}; foo(); foo;", "file:///fake1.js", flags);
  EXPECT_FALSE(isException(res1));
  Handle<JSFunction> funcFoo = runtime->makeHandle(vmcast<JSFunction>(*res1));

  CallResult<HermesValue> res2 = runtime->run(
      "\n  function bar() {}; function bar() {}; function unused() {}; bar(); [bar, unused];",
      "file:///fake2.js",
      flags);
  EXPECT_FALSE(isException(res2));

  std::vector<CodeCoverageProfiler::FuncInfo> executedFuncInfos =
      profiler->getExecutedFunctions();

  Handle<JSArray> funcArr = runtime->makeHandle(vmcast<JSArray>(*res2));
  Handle<JSFunction> funcBar =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 0)));
  Handle<JSFunction> funcUnused =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 1)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(funcFoo, funcUnused));
  EXPECT_TRUE(hasDifferentInfo(funcBar, funcUnused));

  EXPECT_EQ(executedFuncInfos.size(), 4);
  EXPECT_TRUE(isFuncExecuted(executedFuncInfos, funcFoo));
  EXPECT_TRUE(isFuncExecuted(executedFuncInfos, funcBar));
  EXPECT_FALSE(isFuncExecuted(executedFuncInfos, funcUnused));
}

TEST_F(CodeCoverageProfilerTest, FunctionsFromMultipleDomains) {
  hbc::CompileFlags flags;
  flags.lazy = false;
  CallResult<HermesValue> res = runtime->run(
      "var eval1 = eval('function used1() {}; used1(); used1;'); "
      "var eval2 = eval('function unused() {}; function used2() {}; used2(); [used2, unused]');"
      "[eval1, eval2[0], eval2[1]];",
      "file:///fake.js",
      flags);
  EXPECT_FALSE(isException(res));

  std::vector<CodeCoverageProfiler::FuncInfo> executedFuncInfos =
      profiler->getExecutedFunctions();

  Handle<JSArray> funcArr = runtime->makeHandle(vmcast<JSArray>(*res));
  Handle<JSFunction> funcUsed1 =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 0)));
  Handle<JSFunction> funcUsed2 =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 1)));
  Handle<JSFunction> funcUnused =
      runtime->makeHandle(vmcast<JSFunction>(funcArr->at(runtime, 2)));

  // Used and unused functions should have different info.
  EXPECT_TRUE(hasDifferentInfo(funcUsed1, funcUnused));
  EXPECT_TRUE(hasDifferentInfo(funcUsed2, funcUnused));

  // Global + two eval() code + used1 + used2.
  EXPECT_EQ(executedFuncInfos.size(), 5);
  EXPECT_TRUE(isFuncExecuted(executedFuncInfos, funcUsed1));
  EXPECT_TRUE(isFuncExecuted(executedFuncInfos, funcUsed2));
  EXPECT_FALSE(isFuncExecuted(executedFuncInfos, funcUnused));
}

} // namespace CodeCoverageTest
} // namespace unittest
} // namespace hermes
