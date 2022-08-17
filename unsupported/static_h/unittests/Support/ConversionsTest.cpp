/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Conversions.h"

#include <limits>

#include "gtest/gtest.h"

using namespace hermes;

namespace {

// Work around new versions of clang treating the truncation fast path as UB
// and optimising it to not work on compile time constants.
#ifdef __clang__
LLVM_ATTRIBUTE_NOINLINE
#endif
int32_t toInt32Wrapper(double d) {
  return hermes::truncateToInt32(d);
}

TEST(ConversionsTest, toInt32Test) {
  EXPECT_EQ(0, toInt32Wrapper(0));
  EXPECT_EQ(0, toInt32Wrapper(-0.1));
  EXPECT_EQ(0, toInt32Wrapper(0.1));
  EXPECT_EQ(1, toInt32Wrapper(1));
  EXPECT_EQ(-1, toInt32Wrapper(-1.5));

  EXPECT_EQ(1661992960, toInt32Wrapper(1e20));
  EXPECT_EQ(-1661992960, toInt32Wrapper(-1e20));
  EXPECT_EQ(-2147483648, toInt32Wrapper(-2147483648));

  EXPECT_EQ(0, toInt32Wrapper(std::numeric_limits<double>::infinity()));
  EXPECT_EQ(0, toInt32Wrapper(-std::numeric_limits<double>::infinity()));
  EXPECT_EQ(0, toInt32Wrapper(-std::numeric_limits<double>::quiet_NaN()));
  EXPECT_EQ(0, toInt32Wrapper(-std::numeric_limits<double>::denorm_min()));
}

TEST(ConversionsTest, toArrayIndexTest) {
  OptValue<uint32_t> res;

  res = toArrayIndex("123");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(123u, *res);

  res = toArrayIndex("0");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(0u, *res);

  res = toArrayIndex("00");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("01");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("4294967294");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(0xFFFFFFFE, *res);

  res = toArrayIndex("4294967295");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("4294967296");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("99999999999999");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("123a");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex(" 123");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("a123");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndex("4772185884");
  ASSERT_FALSE(res.hasValue());

  // Wrapper for char16_t.
  auto toArrayIndexU = [](const char16_t *str) {
    return hermes::toArrayIndex(
        str, str + std::char_traits<char16_t>::length(str));
  };

  res = toArrayIndexU(u"123");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(123u, *res);

  res = toArrayIndexU(u"0");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(0u, *res);

  res = toArrayIndexU(u"00");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"01");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"4294967294");
  ASSERT_TRUE(res.hasValue());
  ASSERT_EQ(0xFFFFFFFE, *res);

  res = toArrayIndexU(u"4294967295");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"4294967296");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"99999999999999");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"123a");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u" 123");
  ASSERT_FALSE(res.hasValue());

  res = toArrayIndexU(u"a123");
  ASSERT_FALSE(res.hasValue());
};

TEST(ConversionsTest, numberToStringTest) {
  char buf[NUMBER_TO_STRING_BUF_SIZE];

#define DoubleToStringTest(expected, value)  \
  do {                                       \
    numberToString(value, buf, sizeof(buf)); \
    EXPECT_STREQ(expected, buf);             \
  } while (0)

  DoubleToStringTest("NaN", std::nan(""));

  auto infinity = std::numeric_limits<double>::infinity();
  DoubleToStringTest("Infinity", infinity);
  DoubleToStringTest("-Infinity", -infinity);

  DoubleToStringTest("0", +0.0);
  DoubleToStringTest("0", -0.0);
  DoubleToStringTest("3", 3);
  DoubleToStringTest("-3", -3);
  DoubleToStringTest("-10000", -10000);
  DoubleToStringTest("1100000", 1.1e6);

  DoubleToStringTest("10000", 10000);
  DoubleToStringTest("100000", 100000);
  DoubleToStringTest("1000000", 1000000);
  DoubleToStringTest("10000000", 10000000);
  DoubleToStringTest("100000000000000000000", 1e20);
  DoubleToStringTest("1e+21", 1e21);

  DoubleToStringTest("0.125", 0.125);
  DoubleToStringTest("3.14", 3.14);
  DoubleToStringTest("3.14", 3.14);
  DoubleToStringTest("14583.1832", 14583.1832);
  DoubleToStringTest("-14583.1832", -14583.1832);
  DoubleToStringTest("1.23e+25", 1.23e25);
  DoubleToStringTest("1.23e-25", 1.23e-25);
  DoubleToStringTest("5e+25", 5e+25);
  DoubleToStringTest("-5e+25", -5e+25);

  DoubleToStringTest("0", 0);
  DoubleToStringTest("12384", 12384);
  DoubleToStringTest("-12384", -12384);
}

} // end anonymous namespace
