/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BigIntPrimitive.h"

#include "TestHelpers.h"
#include "hermes/Support/BigIntTestHelpers.h"

#include <array>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace hermes::bigint;
using namespace hermes::vm;
using namespace testing;

namespace {

using BigIntPrimitiveTest = RuntimeTestFixture;

LeftToRightVector fullRawData(BigIntPrimitive *bigint) {
  auto fullBytes = bigint->getRawDataFull();
  LeftToRightVector ret;
  ret.data.insert(ret.data.end(), fullBytes.begin(), fullBytes.end());
  return ret;
}

LeftToRightVector compactRawData(BigIntPrimitive *bigint) {
  auto fullBytes = bigint->getRawDataCompact();
  LeftToRightVector ret;
  ret.data.insert(ret.data.end(), fullBytes.begin(), fullBytes.end());
  return ret;
}

LeftToRightVector fullRawData(const Handle<BigIntPrimitive> &bigint) {
  return fullRawData(bigint.get());
}

LeftToRightVector compactRawData(const Handle<BigIntPrimitive> &bigint) {
  return compactRawData(bigint.get());
}

LeftToRightVector fullRawData(HermesValue bigint) {
  return fullRawData(bigint.getBigInt());
}

LeftToRightVector compactRawData(HermesValue bigint) {
  return compactRawData(bigint.getBigInt());
}

// Zero is an important corner case -- it has no trailing digits, and it is
// always expected to be non-negative. Thus, it is tested separately.
TEST_F(BigIntPrimitiveTest, CreateZero) {
  // Must create the correct zero from a signed 0.
  auto H0 = BigIntPrimitive::fromSignedNoThrow(0, runtime);
  EXPECT_FALSE(H0->sign());
  EXPECT_EQ(compactRawData(H0), noDigits());
  EXPECT_EQ(fullRawData(H0), noDigits());

  // Must create the correct zero from the canonical (i.e., empty) digit
  // sequence.
  H0 = BigIntPrimitive::fromBytesNoThrow(noDigits(), runtime);
  EXPECT_FALSE(H0->sign());
  EXPECT_EQ(compactRawData(H0), noDigits());
  EXPECT_EQ(fullRawData(H0), noDigits());

  // Must create the correct zero from the non-canonical (i.e., non-empty)
  // digit sequence.
  H0 = BigIntPrimitive::fromBytesNoThrow(digit(0), runtime);
  EXPECT_FALSE(H0->sign());
  EXPECT_EQ(compactRawData(H0), noDigits());
  EXPECT_EQ(fullRawData(H0), noDigits());

  auto digitZero = digit(0, 0, 0, 0, 0, 0, 0, 0);
  auto lotsOfZeros = digitZero + digitZero + digitZero + digitZero + digitZero +
      digitZero + digitZero + digitZero;
  H0 = BigIntPrimitive::fromBytesNoThrow(lotsOfZeros, runtime);
  EXPECT_FALSE(H0->sign());
  EXPECT_EQ(compactRawData(H0), noDigits());
  EXPECT_EQ(fullRawData(H0), noDigits());
}

TEST_F(BigIntPrimitiveTest, Create) {
  // Need to be able to create from signed.
  auto HNeg1 = BigIntPrimitive::fromSignedNoThrow(-1, runtime);
  EXPECT_TRUE(HNeg1->sign());
  EXPECT_EQ(compactRawData(HNeg1), digit(0xff));
  EXPECT_EQ(
      fullRawData(HNeg1),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));

  auto H1 = BigIntPrimitive::fromSignedNoThrow(1, runtime);
  EXPECT_FALSE(H1->sign());
  EXPECT_EQ(compactRawData(H1), digit(1));
  EXPECT_EQ(fullRawData(H1), digit(0, 0, 0, 0, 0, 0, 0, 1));

  auto HNeg1234568 = BigIntPrimitive::fromSignedNoThrow(-1234568, runtime);
  EXPECT_TRUE(HNeg1234568->sign());
  EXPECT_EQ(compactRawData(HNeg1234568), digit(0xed, 0x29, 0x78));
  EXPECT_EQ(
      fullRawData(HNeg1234568),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xed, 0x29, 0x78));

  // Need to be able to create from short byte array.
  auto HHfe = BigIntPrimitive::fromBytesNoThrow(digit(0xfe), runtime);
  EXPECT_TRUE(HHfe->sign());
  EXPECT_EQ(compactRawData(HHfe), digit(0xfe));
  EXPECT_EQ(
      fullRawData(HHfe), digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe));

  auto HH4e = BigIntPrimitive::fromBytesNoThrow(digit(0x4e), runtime);
  EXPECT_FALSE(HH4e->sign());
  EXPECT_EQ(compactRawData(HH4e), digit(0x4e));
  EXPECT_EQ(fullRawData(HH4e), digit(0, 0, 0, 0, 0, 0, 0, 0x4e));

  auto HH8000 = BigIntPrimitive::fromBytesNoThrow(digit(0x80, 0x00), runtime);
  EXPECT_TRUE(HH8000->sign());
  EXPECT_EQ(compactRawData(HH8000), digit(0x80, 0x00));
  EXPECT_EQ(
      fullRawData(HH8000),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00));

  auto HH4000 = BigIntPrimitive::fromBytesNoThrow(digit(0x40, 0x00), runtime);
  EXPECT_FALSE(HH4000->sign());
  EXPECT_EQ(compactRawData(HH4000), digit(0x40, 0x00));
  EXPECT_EQ(fullRawData(HH4000), digit(0, 0, 0, 0, 0, 0, 0x40, 0x00));

  // Need to be able to create from array that's as long as a digit without
  // adding extra digits.
  auto HHffffffffffffffff = BigIntPrimitive::fromBytesNoThrow(
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff), runtime);
  EXPECT_TRUE(HHffffffffffffffff->sign());
  EXPECT_EQ(compactRawData(HHffffffffffffffff), digit(0xff));
  EXPECT_EQ(
      fullRawData(HHffffffffffffffff),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));

  auto HH7fffffffffffffff = BigIntPrimitive::fromBytesNoThrow(
      digit(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff), runtime);
  EXPECT_FALSE(HH7fffffffffffffff->sign());
  EXPECT_EQ(
      compactRawData(HH7fffffffffffffff),
      digit(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));
  EXPECT_EQ(
      fullRawData(HH7fffffffffffffff),
      digit(0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));

  // Need to be able to create from arrays that are over WordType in size.
  auto HH400000000000000000 = BigIntPrimitive::fromBytesNoThrow(
      digit(0x40) + digit(0, 0, 0, 0, 0, 0, 0, 0), runtime);
  EXPECT_FALSE(HH400000000000000000->sign());
  EXPECT_EQ(
      compactRawData(HH400000000000000000),
      digit(0x40) + digit(0, 0, 0, 0, 0, 0, 0, 0));
  EXPECT_EQ(
      fullRawData(HH400000000000000000),
      digit(0, 0, 0, 0, 0, 0, 0, 0x40) + digit(0, 0, 0, 0, 0, 0, 0, 0));

  auto HH800000000000000000 = BigIntPrimitive::fromBytesNoThrow(
      digit(0x80) + digit(0, 0, 0, 0, 0, 0, 0, 0), runtime);
  EXPECT_TRUE(HH800000000000000000->sign());
  EXPECT_EQ(
      compactRawData(HH800000000000000000),
      digit(0x80) + digit(0, 0, 0, 0, 0, 0, 0, 0));
  EXPECT_EQ(
      fullRawData(HH800000000000000000),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(BigIntPrimitiveTest, FromDouble) {
  auto HMaxDouble = runtime.ignoreAllocationFailure(
      BigIntPrimitive::fromDouble(std::numeric_limits<double>::max(), runtime));
  EXPECT_EQ(
      fullRawData(HMaxDouble),
      digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));
  EXPECT_EQ(
      compactRawData(HMaxDouble),
      digit(0) + digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));

  auto HNegMaxDouble =
      runtime.ignoreAllocationFailure(BigIntPrimitive::fromDouble(
          -std::numeric_limits<double>::max(), runtime));
  EXPECT_EQ(
      fullRawData(HNegMaxDouble),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff) +
          digit(0, 0, 0, 0, 0, 0, 0x08, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0));
  EXPECT_EQ(
      compactRawData(HNegMaxDouble),
      digit(0xff) + digit(0, 0, 0, 0, 0, 0, 0x08, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));

  auto H1024e32 = runtime.ignoreAllocationFailure(
      BigIntPrimitive::fromDouble(1024e32, runtime));
  EXPECT_EQ(
      fullRawData(H1024e32),
      digit(0x00, 0x13, 0xb8, 0xb5, 0xb5, 0x05, 0x6e, 0x17) +
          digit(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
  EXPECT_EQ(
      compactRawData(H1024e32),
      digit(0x13, 0xb8, 0xb5, 0xb5, 0x05, 0x6e, 0x17) +
          digit(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));

  auto HNeg123456e150 = runtime.ignoreAllocationFailure(
      BigIntPrimitive::fromDouble(-123456e150, runtime));
  EXPECT_EQ(
      fullRawData(HNeg123456e150),
      digit(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf6) +
          digit(0xca, 0xcf, 0xa4, 0x60, 0x2c, 0x09, 0x00, 0x00) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));
  EXPECT_EQ(
      compactRawData(HNeg123456e150),
      digit(0xf6) + digit(0xca, 0xcf, 0xa4, 0x60, 0x2c, 0x09, 0x00, 0x00) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0) + digit(0, 0, 0, 0, 0, 0, 0, 0) +
          digit(0, 0, 0, 0, 0, 0, 0, 0));
}
} // namespace
