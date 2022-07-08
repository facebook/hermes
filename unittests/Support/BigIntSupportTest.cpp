/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/BigIntTestHelpers.h"

#include <vector>

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
      getSignExtValue<uint8_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint8_t>(0x80) == 0xff, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int8_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int8_t>(0x80) == -1, "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint16_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint16_t>(0x80) == 0xffff, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int16_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int16_t>(0x80) == -1, "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint32_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint32_t>(0x80) == 0xffffffff,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int32_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int32_t>(0x80) == -1, "Unexpected sign-ext value");

  static_assert(
      getSignExtValue<uint64_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<uint64_t>(0x80) == 0xffffffffffffffffull,
      "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int64_t>(0x00) == 0, "Unexpected sign-ext value");
  static_assert(
      getSignExtValue<int64_t>(0x80) == -1, "Unexpected sign-ext value");

  for (uint32_t i = 0; i < 128; ++i) {
    EXPECT_EQ(getSignExtValue<uint8_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<int8_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<uint16_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<int16_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<uint32_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<int32_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<uint64_t>(i), 0) << i;
    EXPECT_EQ(getSignExtValue<int64_t>(i), 0) << i;
  }
  for (uint32_t i = 128; i < 256; ++i) {
    EXPECT_EQ(getSignExtValue<uint8_t>(i), 0xff) << i;
    EXPECT_EQ(getSignExtValue<int8_t>(i), -1) << i;
    EXPECT_EQ(getSignExtValue<uint16_t>(i), 0xffff) << i;
    EXPECT_EQ(getSignExtValue<int16_t>(i), -1) << i;
    EXPECT_EQ(getSignExtValue<uint32_t>(i), 0xffffffff) << i;
    EXPECT_EQ(getSignExtValue<int32_t>(i), -1) << i;
    EXPECT_EQ(getSignExtValue<uint64_t>(i), 0xffffffffffffffffull) << i;
    EXPECT_EQ(getSignExtValue<int64_t>(i), -1) << i;
  }
}

TEST(BigIntTest, dropExtraSignBitsTest) {
  // Special cases: empty sequence => empty sequence
  EXPECT_TRUE(dropExtraSignBits(llvh::ArrayRef<uint8_t>()).empty());

  // Special cases: sequence of zeros => empty sequence
  EXPECT_TRUE(dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0})).empty());

  EXPECT_TRUE(dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0, 0})).empty());

  EXPECT_TRUE(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0, 0, 0})).empty());

  EXPECT_TRUE(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0, 0, 0, 0})).empty());

  EXPECT_TRUE(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0, 0, 0, 0, 0})).empty());

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0x7f})),
      llvh::makeArrayRef<uint8_t>({0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(
          llvh::makeArrayRef<uint8_t>({0x7f, 0x00, 0x00, 0x00, 0x00})),
      llvh::makeArrayRef<uint8_t>({0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff})),
      llvh::makeArrayRef<uint8_t>({0xff}));

  EXPECT_EQ(
      dropExtraSignBits(
          llvh::makeArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff})),
      llvh::makeArrayRef<uint8_t>({0xff}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>(
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
      llvh::makeArrayRef<uint8_t>(
          {0x00, 0x01, 0x02, 0x03, 0x03, 0x00, 0x00, 0x00, 0x02}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>(
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
      llvh::makeArrayRef<uint8_t>(
          {0x80, 0x81, 0x82, 0x83, 0x89, 0x00, 0x00, 0x00, 0x8a}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>(
          {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f})),
      llvh::makeArrayRef<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}));

  EXPECT_EQ(
      dropExtraSignBits(llvh::makeArrayRef<uint8_t>(
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
      llvh::makeArrayRef<uint8_t>(
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

} // namespace
