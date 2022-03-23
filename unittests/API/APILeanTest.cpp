/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <hermes/hermes.h>
#include <jsi/jsi.h>

using namespace facebook::jsi;
using namespace facebook::hermes;

namespace {

class HermesLeanRuntimeTest : public ::testing::Test {
 public:
  HermesLeanRuntimeTest() : rt(makeHermesRuntime()) {}

 protected:
  std::unique_ptr<HermesRuntime> rt;
};

TEST_F(HermesLeanRuntimeTest, PropertyTest) {
  rt->global().setProperty(*rt, "answer", Value(42));
  EXPECT_EQ(rt->global().getProperty(*rt, "answer").getNumber(), 42);
}

TEST_F(HermesLeanRuntimeTest, EvalTest) {
  auto evalFn = rt->global().getPropertyAsFunction(*rt, "eval");
  ASSERT_THROW(evalFn.call(*rt, "var x = 0;"), JSIException);
}

TEST_F(HermesLeanRuntimeTest, ArrayTest) {
  auto array = Array::createWithElements(*rt, 5, 3, 1, 0, 2, 4);
  array.getPropertyAsFunction(*rt, "sort").callWithThis(*rt, array);
  for (size_t i = 0; i < 6; ++i)
    EXPECT_EQ(array.getValueAtIndex(*rt, i).asNumber(), i);
}

} // namespace
