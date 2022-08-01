/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/BigIntTestHelpers.h"

#include <tuple>
#include <vector>

#include "gmock/gmock.h"

namespace {

using namespace hermes;
using namespace hermes::bigint;

using Radix = uint32_t;

void sucessfullStringLiteralParsing(
    llvh::ArrayRef<char> src,
    llvh::ArrayRef<char> expected,
    Radix expectedRadix,
    ParsedSign expectedSign) {
  std::string outError;
  uint8_t radix;
  ParsedSign sign;
  auto bigintDigits =
      getStringIntegerLiteralDigitsAndSign(src, radix, sign, &outError);
  EXPECT_TRUE(bigintDigits)
      << "failure to parse '" << src.data() << "': " << outError.data();

  if (bigintDigits) {
    EXPECT_EQ(radix, expectedRadix) << src.data();
    EXPECT_EQ(sign, expectedSign) << src.data();
    EXPECT_EQ(*bigintDigits, expected.data());
  }
}

void failedStringLiteralParsing(llvh::ArrayRef<char> src) {
  std::string outError;
  uint8_t radix;
  ParsedSign sign;
  auto bigintDigits =
      getStringIntegerLiteralDigitsAndSign(src, radix, sign, &outError);
  EXPECT_FALSE(bigintDigits)
      << "'" << src.data() << "' was parsed as " << *bigintDigits;
}

TEST(BigIntTest, getStringIntegerLiteralDigitsAndSignTest) {
  // empty string
  sucessfullStringLiteralParsing("", "0", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing(" ", "0", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing("   ", "0", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing("     ", "0", Radix(10), ParsedSign::None);

  // string of zeros, with and without sign.
  sucessfullStringLiteralParsing("00000", "0", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing("+0000000", "0", Radix(10), ParsedSign::Plus);
  sucessfullStringLiteralParsing("-00", "0", Radix(10), ParsedSign::Minus);

  // simple decimal without sign
  sucessfullStringLiteralParsing("1", "1", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing("     1", "1", Radix(10), ParsedSign::None);
  sucessfullStringLiteralParsing("1      ", "1", Radix(10), ParsedSign::None);

  // simple decimal with a '-' sign
  sucessfullStringLiteralParsing("+1", "1", Radix(10), ParsedSign::Plus);
  sucessfullStringLiteralParsing("      +1", "1", Radix(10), ParsedSign::Plus);
  sucessfullStringLiteralParsing(
      "+1        ", "1", Radix(10), ParsedSign::Plus);
  sucessfullStringLiteralParsing(" +1 ", "1", Radix(10), ParsedSign::Plus);

  // simple decimal with a '-' sign
  sucessfullStringLiteralParsing("-1", "1", Radix(10), ParsedSign::Minus);
  sucessfullStringLiteralParsing(" -1", "1", Radix(10), ParsedSign::Minus);
  sucessfullStringLiteralParsing("-1   ", "1", Radix(10), ParsedSign::Minus);
  sucessfullStringLiteralParsing(
      "    -1    ", "1", Radix(10), ParsedSign::Minus);

  // hex number
  sucessfullStringLiteralParsing("0x1", "1", Radix(16), ParsedSign::None);
  sucessfullStringLiteralParsing("  0x123", "123", Radix(16), ParsedSign::None);
  sucessfullStringLiteralParsing("0X1a  ", "1a", Radix(16), ParsedSign::None);
  sucessfullStringLiteralParsing(
      "   0X1bde    ", "1bde", Radix(16), ParsedSign::None);

  // octal number
  sucessfullStringLiteralParsing("0o1", "1", Radix(8), ParsedSign::None);
  sucessfullStringLiteralParsing("    0o12", "12", Radix(8), ParsedSign::None);
  sucessfullStringLiteralParsing(
      "0O176        ", "176", Radix(8), ParsedSign::None);
  sucessfullStringLiteralParsing(
      "       0O150123          ", "150123", Radix(8), ParsedSign::None);

  // binary number
  sucessfullStringLiteralParsing("0b1", "1", Radix(2), ParsedSign::None);
  sucessfullStringLiteralParsing("   0b10", "10", Radix(2), ParsedSign::None);
  sucessfullStringLiteralParsing(
      "0B11111111100     ", "11111111100", Radix(2), ParsedSign::None);
  sucessfullStringLiteralParsing(
      "0B100000011", "100000011", Radix(2), ParsedSign::None);

  // +/- without digits.
  failedStringLiteralParsing("+");
  failedStringLiteralParsing("-");

  // radix prefix but no digits
  failedStringLiteralParsing("0x");
  failedStringLiteralParsing("0X");
  failedStringLiteralParsing("0o");
  failedStringLiteralParsing("0O");
  failedStringLiteralParsing("0b");
  failedStringLiteralParsing("0B");

  // input can't have the n suffix
  failedStringLiteralParsing("0n");
  failedStringLiteralParsing("1n");
  failedStringLiteralParsing("0x1n");
  failedStringLiteralParsing("0X1n");
  failedStringLiteralParsing("0o1n");
  failedStringLiteralParsing("0O1n");
  failedStringLiteralParsing("0b1n");
  failedStringLiteralParsing("0B1n");

  // leading zeros before radix prefix
  failedStringLiteralParsing("00x1");
  failedStringLiteralParsing("000X1");
  failedStringLiteralParsing("0000o1");
  failedStringLiteralParsing("00000O1");
  failedStringLiteralParsing("000000b1");
  failedStringLiteralParsing("0000000B1");

  // no +/- in non-decimal strings
  failedStringLiteralParsing("+0x1");
  failedStringLiteralParsing("-0X1");
  failedStringLiteralParsing("+0o1");
  failedStringLiteralParsing("-0O1");
  failedStringLiteralParsing("+0b1");
  failedStringLiteralParsing("-0B1");
}

void successfulParseTest(
    llvh::StringRef src,
    llvh::StringRef expected,
    Radix expectedRadix) {
  std::string outError;
  uint8_t radix;
  auto bigintDigits = getNumericValueDigits(src, radix, &outError);
  EXPECT_TRUE(bigintDigits)
      << "failure to parse " << src.data() << ": " << outError.data();

  if (bigintDigits) {
    EXPECT_EQ(radix, expectedRadix) << src.data();
    EXPECT_EQ(*bigintDigits, expected.data());
  }
}

void parsingFailureTest(llvh::StringRef src) {
  std::string outError;
  uint8_t radix;
  auto bigintDigits = getNumericValueDigits(src, radix, &outError);
  EXPECT_FALSE(bigintDigits)
      << src.data() << " was parsed as " << *bigintDigits;
}

TEST(BigIntTest, getNumericValueDigitsTest) {
  // Decimal strings.
  successfulParseTest("0n", "0", Radix(10));
  successfulParseTest("1n", "1", Radix(10));
  successfulParseTest("12n", "12", Radix(10));
  successfulParseTest("1_2n", "12", Radix(10));
  successfulParseTest("1_2_3_4_5_6_7_8_9_0_1n", "12345678901", Radix(10));
  successfulParseTest("1234_5678n", "12345678", Radix(10));
  successfulParseTest("1234_567_9n", "12345679", Radix(10));

  // Binary strings.
  successfulParseTest("0b0n", "0", Radix(2));
  successfulParseTest("0b1n", "1", Radix(2));
  successfulParseTest("0b1010_1111n", "10101111", Radix(2));
  successfulParseTest("0B1010_1111n", "10101111", Radix(2));

  // octal strings.
  successfulParseTest("0o0n", "0", Radix(8));
  successfulParseTest("0o1n", "1", Radix(8));
  successfulParseTest("0o01_234_567n", "01234567", Radix(8));
  successfulParseTest("0O01_234_567n", "01234567", Radix(8));

  // hex strings.
  successfulParseTest("0x0n", "0", Radix(16));
  successfulParseTest("0x1n", "1", Radix(16));
  successfulParseTest(
      "0x01_2345_6789_ABCD_EFab_cdefn", "0123456789ABCDEFabcdef", Radix(16));
  successfulParseTest(
      "0X01_2345_6789_ABCD_EFab_cdefn", "0123456789ABCDEFabcdef", Radix(16));

  // Failure cases
  parsingFailureTest(""); // empty string
  parsingFailureTest("n"); // n suffix without numbers
  parsingFailureTest("0"); // zero without n suffix
  parsingFailureTest("_1n"); // leading separator
  parsingFailureTest("1_n"); // separator without follow up numbers
  parsingFailureTest("12"); // missing n suffix
  parsingFailureTest("1__1n"); // multiple separators
  parsingFailureTest("1_an"); // invalid digit

  parsingFailureTest("0b"); // missing digits and suffix
  parsingFailureTest("0b0"); // missing digits and suffix
  parsingFailureTest("0bn"); // missing digits
  parsingFailureTest("0b1_n"); // missing digits after separator
  parsingFailureTest("0b1__1n"); // multiple separators
  parsingFailureTest("0b1_12n"); // invalid digit

  parsingFailureTest("0o"); // missing digits and suffix
  parsingFailureTest("0o1"); // missing digits and suffix
  parsingFailureTest("0on"); // missing digits
  parsingFailureTest("0o1_n"); // missing digits after separator
  parsingFailureTest("0o1__1n"); // multiple separators
  parsingFailureTest("0o1_19n"); // invalid digit

  parsingFailureTest("0x"); // missing digits and suffix
  parsingFailureTest("0x1"); // missing suffix
  parsingFailureTest("0xn"); // missing digits
  parsingFailureTest("0x1_n"); // missing digits after separator
  parsingFailureTest("0x1__1n"); // multiple separators
  parsingFailureTest("0x1_1Gn"); // invalid digit
}

TEST(BigIntTest, numDigitsForSizeInBytesTest) {
  EXPECT_EQ(numDigitsForSizeInBytes(0), 0);

  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 1; j <= BigIntDigitSizeInBytes; ++j) {
      EXPECT_EQ(numDigitsForSizeInBytes(BigIntDigitSizeInBytes * i + j), i + 1)
          << BigIntDigitSizeInBytes * i + j;
    }
  }
}

TEST(BigIntTest, numDigitsForSizeInBitsTest) {
  EXPECT_EQ(numDigitsForSizeInBits(0), 0);

  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 1; j <= BigIntDigitSizeInBits; ++j) {
      EXPECT_EQ(numDigitsForSizeInBits(BigIntDigitSizeInBits * i + j), i + 1)
          << BigIntDigitSizeInBits * i + j;
    }
  }
}

TEST(BigIntTest, maxCharsPerDigitInRadixTest) {
  // There are no CHECKs in this test as all possible radix values are
  // enumarated below, and we can static_assert() them. Still, having a separate
  // TEST() case allows grouping these cases together.
#define STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(radix, value) \
  static_assert(                                               \
      maxCharsPerDigitInRadix(radix) == (value),               \
      "Wrong result for maxCharsPerDigitInRadix(" #radix ")")
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(2, 64 / 2);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(3, 64 / 2);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(4, 64 / 4);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(5, 64 / 4);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(6, 64 / 4);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(7, 64 / 4);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(8, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(9, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(10, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(11, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(12, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(13, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(14, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(15, 64 / 8);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(16, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(17, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(18, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(19, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(20, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(21, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(22, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(23, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(24, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(25, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(26, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(27, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(28, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(29, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(30, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(31, 64 / 16);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(32, 64 / 32);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(33, 64 / 32);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(34, 64 / 32);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(35, 64 / 32);
  STATIC_ASSERT_MAX_CHARS_PER_DIGIT_RESULT(36, 64 / 32);
}

TEST(BigIntTest, getSignExtValueTest) {
  // sanity-check some base values.
  static_assert(
      getSignExtValue<uint8_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint8_t, uint8_t>(0x80) == 0xff,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int8_t, uint8_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int8_t, uint8_t>(0x80) == -1,
      "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint16_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint16_t, uint8_t>(0x80) == 0xffff,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int16_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int16_t, uint8_t>(0x80) == -1,
      "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint32_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint32_t, uint8_t>(0x80) == 0xffffffff,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int32_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int32_t, uint8_t>(0x80) == -1,
      "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint64_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint64_t, uint8_t>(0x80) == 0xffffffffffffffffull,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int64_t, uint8_t>(0x00) == 0,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int64_t, uint8_t>(0x80) == -1,
      "Unexpected sign-ext value");

  for (uint32_t i = 0; i < 128; ++i) {
    EXPECT_EQ((getSignExtValue<uint8_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<int8_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<uint16_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<int16_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<uint32_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<int32_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<uint64_t, uint8_t>(i)), 0) << i;
    EXPECT_EQ((getSignExtValue<int64_t, uint8_t>(i)), 0) << i;
  }
  for (uint32_t i = 128; i < 256; ++i) {
    EXPECT_EQ((getSignExtValue<uint8_t, uint8_t>(i)), 0xff) << i;
    EXPECT_EQ((getSignExtValue<int8_t, uint8_t>(i)), -1) << i;
    EXPECT_EQ((getSignExtValue<uint16_t, uint8_t>(i)), 0xffff) << i;
    EXPECT_EQ((getSignExtValue<int16_t, uint8_t>(i)), -1) << i;
    EXPECT_EQ((getSignExtValue<uint32_t, uint8_t>(i)), 0xffffffff) << i;
    EXPECT_EQ((getSignExtValue<int32_t, uint8_t>(i)), -1) << i;
    EXPECT_EQ((getSignExtValue<uint64_t, uint8_t>(i)), 0xffffffffffffffffull)
        << i;
    EXPECT_EQ((getSignExtValue<int64_t, uint8_t>(i)), -1) << i;
  }
}

TEST(BigIntTest, dropExtraSignBitsTest) {
  // Special cases: empty sequence => empty sequence
  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>()).empty());

  // Special cases: sequence of zeros => empty sequence
  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>({0})).empty());

  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>({0, 0})).empty());

  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>({0, 0, 0})).empty());

  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>({0, 0, 0, 0})).empty());

  EXPECT_TRUE(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>({0, 0, 0, 0, 0})).empty());

  EXPECT_EQ(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>({0x7f})),
      llvh::ArrayRef<uint8_t>({0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(
          llvh::ArrayRef<uint8_t>({0x7f, 0x00, 0x00, 0x00, 0x00})),
      llvh::ArrayRef<uint8_t>({0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff})),
      llvh::ArrayRef<uint8_t>({0xff}));

  EXPECT_EQ(
      dropExtraSignBits(
          llvh::ArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff})),
      llvh::ArrayRef<uint8_t>({0xff}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>(
          {0x00,
           0x01,
           0x02,
           0x03,
           0x03,
           0x00,
           0x00,
           0x00,
           0x02,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00})),
      llvh::ArrayRef<uint8_t>(
          {0x00, 0x01, 0x02, 0x03, 0x03, 0x00, 0x00, 0x00, 0x02}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>(
          {0x80,
           0x81,
           0x82,
           0x83,
           0x89,
           0x00,
           0x00,
           0x00,
           0x8a,
           0xff,
           0xff,
           0xff,
           0xff,
           0xff})),
      llvh::ArrayRef<uint8_t>(
          {0x80, 0x81, 0x82, 0x83, 0x89, 0x00, 0x00, 0x00, 0x8a}));

  EXPECT_EQ(
      dropExtraSignBits(
          llvh::ArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f})),
      llvh::ArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::ArrayRef<uint8_t>(
          {0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x80})),
      llvh::ArrayRef<uint8_t>(
          {0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x80}));
}

LeftToRightVector fillDigits(
    uint32_t numDigits,
    llvh::ArrayRef<uint8_t> bytes) {
  // initialize the resulting bytes with 0xdd helps spot bytes that are
  // uninitialized by initWithBytes.
  const BigIntDigitType kUninitialized = 0xddddddddddddddddull;

  // Always request at least 1 digit to avoid passing nullptr to
  // initWithBytes.
  std::vector<BigIntDigitType> result(std::max(1u, numDigits), kUninitialized);
  result.resize(numDigits);

  auto res = initWithBytes(MutableBigIntRef{result.data(), numDigits}, bytes);
  EXPECT_EQ(res, OperationStatus::RETURNED);

  // Create a byte view into result. Note that the number of digits in result is
  // **NOT** result.size(), but rather numDigits (which could have been modified
  // in initWithBytes).
  auto byteView = llvh::makeArrayRef(
      reinterpret_cast<const uint8_t *>(result.data()),
      numDigits * BigIntDigitSizeInBytes);

  // BigInt data is already LSB to MSB, so there's no LeftToRightVector
  // constructor that can be called. Thus create an empty LeftToRightVector and
  // populate its data member directly.
  LeftToRightVector ret;
  ret.data.insert(ret.data.end(), byteView.begin(), byteView.end());
  return ret;
}

TEST(BigIntTest, initWithBytesTest) {
  EXPECT_EQ(fillDigits(0, noDigits()), noDigits());

  EXPECT_EQ(fillDigits(1, noDigits()), noDigits());

  EXPECT_EQ(fillDigits(2, noDigits()), noDigits());

  EXPECT_EQ(fillDigits(1, digit(1, 2)), digit(0, 0, 0, 0, 0, 0, 1, 2));

  EXPECT_EQ(fillDigits(2, digit(1, 2)), digit(0, 0, 0, 0, 0, 0, 1, 2));

  EXPECT_EQ(
      fillDigits(2, digit(0) + digit(0x80, 0, 0, 0, 0, 0, 0, 0)),
      digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0x80, 0, 0, 0, 0, 0, 0, 0));

  EXPECT_EQ(
      fillDigits(2, digit(0xff) + digit(0x80, 0, 0, 0, 0, 0, 0, 0)),
      digit(0x80, 0, 0, 0, 0, 0, 0, 0));

  EXPECT_EQ(
      fillDigits(2, digit(0x80)),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80));

  EXPECT_EQ(
      fillDigits(2, digit(1, 2, 3, 4, 5, 6, 7, 8)),
      digit(1, 2, 3, 4, 5, 6, 7, 8));

  EXPECT_EQ(
      fillDigits(2, digit(9) + digit(1, 2, 3, 4, 5, 6, 7, 8)),
      digit(0, 0, 0, 0, 0, 0, 0, 9) + digit(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(BigIntTest, UniquingBigIntTable) {
  UniquingBigIntTable t;

  EXPECT_TRUE(t.empty());

  auto parse = [](std::string_view str) {
    std::string outError;
    return std::make_pair(
        ParsedBigInt::parsedBigIntFromStringIntegerLiteral(
            llvh::makeArrayRef(str.data(), str.size()), &outError),
        outError);
  };

  const struct {
    const char *str;
    uint32_t expectedIndex;
    LeftToRightVector expectedBytes;
  } kTests[] = {
      {"999999999999999999999999999999999999",
       0,
       LeftToRightVector{
           digit(0x00, 0xc0, 0x97, 0xce, 0x7b, 0xc9, 0x07, 0x15) +
           digit(0xb3, 0x4b, 0x9f, 0x0f, 0xff, 0xff, 0xff, 0xff)}},
      {"1", 1, LeftToRightVector{digit(1)}},
      {"", 2, LeftToRightVector{noDigits()}},
      {"0xaabbccddeeff1122aabbccddeeff1122aabbccddeeff1122",
       3,
       LeftToRightVector{
           digit(0x00) + digit(0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22) +
           digit(0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22) +
           digit(0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22)}},
      {"-1", 4, LeftToRightVector{digit(0xff)}},
      {"0", 2, LeftToRightVector{noDigits()}},
      {"-999999999999999999999999999999999999",
       5,
       LeftToRightVector{
           digit(0xff, 0x3f, 0x68, 0x31, 0x84, 0x36, 0xf8, 0xea) +
           digit(0x4c, 0xb4, 0x60, 0xf0, 0x00, 0x00, 0x00, 0x01)}},
  };

  auto byIndex = [&t](uint32_t index) {
    auto entries = t.getEntryList();
    assert(index < entries.size() && "bigint index out-of-bounds");
    auto digitsBuffer = t.getDigitsBuffer();
    const auto &entry = entries[index];
    assert(
        entry.offset + entry.length <= digitsBuffer.size() &&
        "bigint table entry out-of-bounds");
    auto byteView =
        llvh::makeArrayRef(digitsBuffer.data() + entry.offset, entry.length);
    LeftToRightVector ret;
    ret.data.insert(ret.data.end(), byteView.begin(), byteView.end());
    return ret;
  };

  for (const auto &test : kTests) {
    auto [maybeParsed, errorStr] = parse(test.str);
    ASSERT_TRUE(maybeParsed)
        << "failed to parse " << test.str << ": " << errorStr;
    EXPECT_EQ(t.addBigInt(*std::move(maybeParsed)), test.expectedIndex);
    EXPECT_EQ(byIndex(test.expectedIndex), test.expectedBytes)
        << "test[" << (&test - kTests) << "].str = " << test.str;
  }
}

TEST(BigIntTest, truncateToSingleDigitTest) {
  constexpr bool signedTruncation = true;
  constexpr bool unsignedTruncation = false;
  std::vector<BigIntDigitType> buffer;
  auto toImmutableRef = [&buffer](LeftToRightVector &&v) {
    buffer.resize(
        std::max<uint32_t>(1u, numDigitsForSizeInBytes(v.data.size())));
    uint32_t numDigits = buffer.size();
    auto res =
        initWithBytes(MutableBigIntRef{buffer.data(), numDigits}, v.data);
    EXPECT_EQ(res, OperationStatus::RETURNED);
    return ImmutableBigIntRef{buffer.data(), numDigits};
  };

  auto lossless = [](BigIntDigitType d) { return std::make_tuple(d, true); };
  auto lossy = [](BigIntDigitType d) { return std::make_tuple(d, false); };
  auto truncateToSingleDigit = [&](ImmutableBigIntRef src,
                                   bool signedTruncation) {
    return std::make_tuple(
        bigint::truncateToSingleDigit(src),
        bigint::isSingleDigitTruncationLossless(src, signedTruncation));
  };

  EXPECT_EQ(
      truncateToSingleDigit(toImmutableRef(noDigits()), unsignedTruncation),
      lossless(0));

  EXPECT_EQ(
      truncateToSingleDigit(toImmutableRef(noDigits()), signedTruncation),
      lossless(0));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(digit(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)),
          unsignedTruncation),
      lossless(0x7fffffffffffffffull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(digit(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)),
          signedTruncation),
      lossless(0x7fffffffffffffffull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          unsignedTruncation),
      lossy(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          signedTruncation),
      lossless(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x00) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          unsignedTruncation),
      lossless(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x00) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          signedTruncation),
      lossy(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x01) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          unsignedTruncation),
      lossy(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x01) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          signedTruncation),
      lossy(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x01) +
              digit(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          unsignedTruncation),
      lossy(0x8000000000000000ull));

  EXPECT_EQ(
      truncateToSingleDigit(
          toImmutableRef(
              digit(0x01) +
              digit(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00) +
              digit(0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)),
          signedTruncation),
      lossy(0x8000000000000000ull));
}
} // namespace
