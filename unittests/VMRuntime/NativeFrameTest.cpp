/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

using namespace hermes::hbc;
using namespace hermes::vm;

namespace {

// This value matches the same field in Runtime, but we don't wish to make it
// public in Runtime.
constexpr unsigned MAX_NATIVE_CALL_FRAME_DEPTH = 384;

// Count the number of native stack frames we can make before it reports
// overflow. Also verify that our stack descends down.
static unsigned makeFramesUntilOverflow(
    Runtime *runtime,
    ScopedNativeCallFrame *prev) {
  ScopedNativeCallFrame frame{
      runtime, 0, nullptr, false, HermesValue::encodeUndefinedValue()};
  if (frame.overflowed())
    return 0;
  EXPECT_TRUE(!prev || (*prev)->ptr() > frame->ptr());
  return 1 + makeFramesUntilOverflow(runtime, &frame);
}

using NativeFrameTest = RuntimeTestFixture;

TEST_F(NativeFrameTest, OverflowTest) {
  unsigned maxDepth = makeFramesUntilOverflow(runtime, nullptr);
  EXPECT_EQ(maxDepth, MAX_NATIVE_CALL_FRAME_DEPTH);
}

#if HERMES_SLOW_DEBUG
TEST_F(NativeFrameTest, PoisonedStackTest) {
  // Verify that stack frames are poisoned.
  ScopedNativeCallFrame frame{
      runtime, 0, nullptr, false, HermesValue::encodeUndefinedValue()};
  ASSERT_FALSE(frame.overflowed());
  // We should not die after this because there were no arguments.
  runtime->collect();

  // Now make a frame with arguments.
  ScopedNativeCallFrame frame2{
      runtime, 1, nullptr, false, HermesValue::encodeUndefinedValue()};
  ASSERT_FALSE(frame2.overflowed());
  // The frame should be poisoned; ensure we die after a GC.
  EXPECT_DEATH(runtime->collect(), "Invalid");
}
#endif

} // namespace
