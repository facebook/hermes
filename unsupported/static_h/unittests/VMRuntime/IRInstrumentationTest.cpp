/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_IR_INSTRUMENTATION

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

using namespace hermes::vm;
using namespace hermes::hbc;

namespace {
class IRInstrumentationTest : public ::testing::Test {
 protected:
  std::shared_ptr<Runtime> runtime_;

  void SetUp() override {
    auto config = RuntimeConfig::Builder().build();
    runtime_ = Runtime::create(config);
  }

  CallResult<HermesValue> run(llvh::StringRef ref, bool instrument) {
    CompileFlags flags;
    flags.instrumentIR = instrument;
    auto result = runtime_->run(ref, "IRInstrumentationTest.cpp", flags);
    EXPECT_EQ(ExecutionStatus::RETURNED, result.getStatus());
    return result;
  }

  CallResult<HermesValue> run(llvh::StringRef code) {
    return run(code, true);
  }

  void setHook(llvh::StringRef code) {
    auto result = run(code, false);
    ASSERT_EQ(ExecutionStatus::RETURNED, result.getStatus());
  }
};

TEST_F(IRInstrumentationTest, binaryHooksCanPassCookieAndOverride) {
  setHook(
      "__instrument.preBinary = function(iid, op, lhs, rhs) { return 41; }");
  setHook(
      "__instrument.postBinary = function(iid, cookie, op, result, lhs, rhs) { return cookie+1; }");
  ASSERT_EQ(42, run("1+2")->getDouble());
}

TEST_F(IRInstrumentationTest, unaryHooksCanPassCookieAndOverride) {
  setHook("__instrument.preUnary = function(iid, op, operand) { return 41; }");
  setHook(
      "__instrument.postUnary = function(iid, cookie, op, result, operand) { return cookie+1; }");
  ASSERT_EQ(42, run("-1234")->getDouble());
}

TEST_F(IRInstrumentationTest, assignmentHooksCanPassCookieAndOverride) {
  setHook(
      "__instrument.preAssignment = function(iid, op, lhs, rhs) { return 41; }");
  setHook(
      "__instrument.postAssignment = function(iid, cookie, op, result, lhs, rhs) { return rhs === undefined ? cookie+1 : result; }");
  ASSERT_EQ(17, run("var x; x = 17; x")->getDouble());
  ASSERT_EQ(42, run("var x = 1; x ^= undefined; x")->getDouble());
}

} // anonymous namespace

#endif // HERMES_ENABLE_IR_INSTRUMENTATION
