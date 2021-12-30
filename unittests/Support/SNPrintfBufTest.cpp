/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SNPrintfBuf.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(SNPrintfBufTest, Basic) {
  // Starting with a small size, writing more than that to cause reallocs:
  // Final result is still what we expect.
  SNPrintfBuf buf(10);
  for (int i = 0; i < 9; i++) {
    buf.printf("|next: %s", "123456789");
  }
  EXPECT_STREQ(
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789"
      "|next: 123456789",
      buf.c_str());
}

TEST(SNPrintfBufTest, ArgsWork) {
  SNPrintfBuf buf(100);
  buf.printf("a: %s, b: %d, c: %7.2f", "abc", 100, 200.0);
  EXPECT_STREQ("a: abc, b: 100, c:  200.00", buf.c_str());
}

} // end anonymous namespace
