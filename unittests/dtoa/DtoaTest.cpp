/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "dtoa/dtoa.h"

#include "gtest/gtest.h"

namespace {

TEST(DtoaTest, SmokeTest) {
  char buf[32];
  g_fmt(buf, 3.14);
  EXPECT_STREQ("3.14", buf);
  DtoaAllocator<> dalloc{};

  char *se;
  double val = hermes_g_strtod(buf, &se);
  ASSERT_EQ(0, *se);
  ASSERT_EQ(3.14, val);

  const char *inv = "asdf";
  val = hermes_g_strtod(inv, &se);
  ASSERT_EQ(inv, se);
  ASSERT_EQ(0, val);

#define DtoaDecimalTest(M, N, K, S, SIGN)                    \
  {                                                          \
    int n, sign;                                             \
    char *s = ::g_dtoa(dalloc, M, 0, 0, &n, &sign, nullptr); \
    int k = ::strlen(s);                                     \
    EXPECT_EQ(N, n);                                         \
    EXPECT_EQ(K, k);                                         \
    EXPECT_EQ(SIGN, sign);                                   \
    EXPECT_STREQ(S, s);                                      \
    g_freedtoa(dalloc, s);                                   \
  }

  DtoaDecimalTest(2.123, 1, 4, "2123", 0);
  DtoaDecimalTest(123, 3, 3, "123", 0);
  DtoaDecimalTest(1000, 4, 1, "1", 0);
  DtoaDecimalTest(1000000, 7, 1, "1", 0);
  DtoaDecimalTest(-1000000, 7, 1, "1", 1);
}
} // namespace
