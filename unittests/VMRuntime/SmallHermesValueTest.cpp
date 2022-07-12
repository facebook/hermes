/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SmallHermesValue.h"

#include "TestHelpers.h"
#include "hermes/Support/Conversions.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"

#include <cfloat>
#include <climits>
#include <random>

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

int32_t getRandomInt32(int32_t lower, int32_t upper) {
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int64_t> uniform_dist(lower, upper);
  return uniform_dist(e1);
}

uint64_t getRandomUInt64(uint64_t lower, uint64_t upper) {
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<uint64_t> uniform_dist(lower, upper);
  return uniform_dist(e1);
}

double getRandomDouble(double lower, double upper) {
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_real_distribution<double> uniform_dist(lower, upper);
  return uniform_dist(e1);
}

using SmallHermesValueRuntimeTest = RuntimeTestFixture;

TEST_F(SmallHermesValueRuntimeTest, NullTest) {
  // Encode the null value.
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeNullValue(), runtime);
  EXPECT_TRUE(SHV.isNull());
  // Should not be boxed.
  EXPECT_FALSE(SHV.isPointer());
  EXPECT_TRUE(SHV.unboxToHV(runtime).isNull());
}

TEST_F(SmallHermesValueRuntimeTest, UndefinedTest) {
  // Encode the undefined value.
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeUndefinedValue(), runtime);
  EXPECT_TRUE(SHV.isUndefined());
  // Should not be boxed.
  EXPECT_FALSE(SHV.isPointer());
  auto HV = SHV.unboxToHV(runtime);
  EXPECT_TRUE(HV.isUndefined());
}

TEST_F(SmallHermesValueRuntimeTest, EmptyTest) {
  // Encode the empty value.
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeEmptyValue(), runtime);
  EXPECT_TRUE(SHV.isEmpty());
  // Should not be boxed.
  EXPECT_FALSE(SHV.isPointer());
  auto HV = SHV.unboxToHV(runtime);
  EXPECT_TRUE(HV.isEmpty());
}

TEST_F(SmallHermesValueRuntimeTest, BoolTest) {
  // Encode true.
  auto TSHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeBoolValue(true), runtime);
  EXPECT_FALSE(TSHV.isPointer());
  auto THV = TSHV.unboxToHV(runtime);
  EXPECT_TRUE(THV.isBool());
  EXPECT_TRUE(THV.getBool());

  // Encode false.
  auto FSHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeBoolValue(false), runtime);
  EXPECT_FALSE(FSHV.isPointer());
  auto FHV = FSHV.unboxToHV(runtime);
  EXPECT_TRUE(FHV.isBool());
  EXPECT_FALSE(FHV.getBool());
}

TEST_F(SmallHermesValueRuntimeTest, DoubleTest) {
  // Encode doubles.
  auto verifyDouble = [&](double d) {
    auto SHV = SmallHermesValue::encodeHermesValue(
        HermesValue::encodeNumberValue(d), runtime);
    auto HV = SHV.unboxToHV(runtime);
    EXPECT_TRUE(HV.isNumber()) << "double not encoded as number: " << d;
    // Check for bitwise equality so we can test things like NaN and -0.
    EXPECT_EQ(llvh::DoubleToBits(HV.getNumber()), llvh::DoubleToBits(d))
        << "doubles not the same: " << d;
  };

  verifyDouble(std::numeric_limits<double>::quiet_NaN());
  verifyDouble(std::numeric_limits<double>::min());
  verifyDouble(std::numeric_limits<double>::max());
  verifyDouble(std::numeric_limits<double>::epsilon());
  verifyDouble(-0.0);

  // Try some random doubles.
  for (int i = 0; i < 1000; i++)
    verifyDouble(getRandomDouble(DBL_MIN, DBL_MAX));
}

TEST_F(SmallHermesValueRuntimeTest, SmiTest) {
  // Encode SMIs.
  auto verifySmi = [&](int32_t i) {
    auto SHV = SmallHermesValue::encodeHermesValue(
        HermesValue::encodeNumberValue(i), runtime);
// When Handle-SAN is enabled, we put all numbers on the heap.
#ifndef HERMESVM_SANITIZE_HANDLES
    EXPECT_FALSE(SHV.isPointer());
#endif
    auto HV = SHV.unboxToHV(runtime);
    EXPECT_TRUE(HV.isNumber()) << "SMI not encoded as number: " << i;
    EXPECT_EQ(HV.getNumber(), i);
  };

  verifySmi(0);
  verifySmi(llvh::minIntN(29));
  verifySmi(llvh::maxIntN(29));

  // Try some random SMIs.
  for (int i = 0; i < 1000; i++)
    verifySmi(getRandomInt32(llvh::minIntN(29), llvh::maxIntN(29)));
}

TEST_F(SmallHermesValueRuntimeTest, SymbolTest) {
  auto verifySymbol = [&](uint32_t id) {
    auto sym = SymbolID::unsafeCreate(id);
    auto SHV = SmallHermesValue::encodeHermesValue(
        HermesValue::encodeSymbolValue(sym), runtime);
    EXPECT_FALSE(SHV.isPointer());
    auto HV = SHV.unboxToHV(runtime);
    EXPECT_TRUE(HV.isSymbol());
    EXPECT_EQ(sym, HV.getSymbol());
  };

  verifySymbol(0);
  verifySymbol(1);
  verifySymbol(2);
  verifySymbol(SymbolID::EMPTY_ID);
  verifySymbol(SymbolID::DELETED_ID);

  // Try some random SymbolIDs.
  for (int i = 0; i < 1000; i++)
    verifySymbol((uint32_t)getRandomUInt64(0, SymbolID::LAST_INVALID_ID));
}

TEST_F(SmallHermesValueRuntimeTest, StringTest) {
  // Encode a string.
  auto message =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello"));
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeStringValue(message.get()), runtime);
  EXPECT_TRUE(SHV.isPointer());
  EXPECT_TRUE(SHV.isString());
  auto HV = SHV.unboxToHV(runtime);
  EXPECT_TRUE(HV.isString());
  EXPECT_EQ(HV.getString(), message.get());
}

TEST_F(SmallHermesValueRuntimeTest, BigIntTest) {
  // Encode a bigint.
  auto H = BigIntPrimitive::fromSignedNoThrow(runtime, 0x123);
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeBigIntValue(H.get()), runtime);
  EXPECT_TRUE(SHV.isPointer());
  EXPECT_TRUE(SHV.isBigInt());
  auto HV = SHV.unboxToHV(runtime);
  EXPECT_TRUE(HV.isBigInt());
  EXPECT_EQ(HV.getBigInt(), H.get());
}

TEST_F(SmallHermesValueRuntimeTest, ObjectTest) {
  // Encode an object.
  auto obj = JSNumber::create(
      runtime, 3.14, Handle<JSObject>::vmcast(&runtime.numberPrototype));
  auto SHV = SmallHermesValue::encodeHermesValue(
      HermesValue::encodeObjectValue(obj.get()), runtime);
  EXPECT_TRUE(SHV.isPointer());
  EXPECT_TRUE(SHV.isObject());
  auto HV = SHV.unboxToHV(runtime);
  EXPECT_TRUE(HV.isObject());
  EXPECT_EQ(HV.getObject(), obj.get());
}

} // anonymous namespace.
