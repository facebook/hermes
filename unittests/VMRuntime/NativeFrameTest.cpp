/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

using namespace hermes::hbc;
using namespace hermes::vm;

namespace {

// Count the number of native stack frames we can make before it reports
// overflow. Also verify that our stack descends down.
static unsigned makeFramesUntilOverflow(
    Runtime &runtime,
    ScopedNativeCallFrame *prev) {
  ScopedNativeCallFrame frame{
      runtime,
      0,
      HermesValue::encodeUndefinedValue(),
      HermesValue::encodeUndefinedValue(),
      HermesValue::encodeUndefinedValue()};
  if (frame.overflowed())
    return 0;
  EXPECT_TRUE(!prev || (*prev)->ptr() < frame->ptr());
  return 1 + makeFramesUntilOverflow(runtime, &frame);
}

using NativeFrameTest = RuntimeTestFixture;

TEST_F(NativeFrameTest, OverflowTest) {
  unsigned maxDepth = makeFramesUntilOverflow(runtime, nullptr);
  // Save into a local variable in order to avoid linker errors when passed
  // to gtest.
  auto expectedMaxDepth = Runtime::MAX_NATIVE_CALL_FRAME_DEPTH;
  EXPECT_EQ(maxDepth, expectedMaxDepth);
}

#if HERMES_SLOW_DEBUG
TEST(NativeFrameDeathTest, PoisonedStackTest) {
  auto fn = [] {
    auto rt = Runtime::create({});
    auto &runtime = *rt;
    // Verify that stack frames are poisoned.
    ScopedNativeCallFrame frame{
        runtime,
        0,
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue()};
    ASSERT_FALSE(frame.overflowed());
    // We should not die after this because there were no arguments.
    runtime.collect("test");

    // Now make a frame with arguments.
    ScopedNativeCallFrame frame2{
        runtime,
        1,
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue()};
    ASSERT_FALSE(frame2.overflowed());
    // The frame should be poisoned; ensure we die after a GC.
    runtime.collect("test");
  };
  EXPECT_DEATH_IF_SUPPORTED(fn(), "Invalid");
}
#endif

} // namespace
