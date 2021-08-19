/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Operations.h"

#include "hermes/VM/JSObject.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/StringView.h"

#include "TestHelpers.h"

#include <limits>

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

CallResult<HermesValue> res{ExecutionStatus ::EXCEPTION};
#define IsSameValueTest(result, v1, v2)   \
  {                                       \
    EXPECT_##result(isSameValue(v1, v2)); \
    EXPECT_##result(isSameValue(v2, v1)); \
  }

using OperationsTest = RuntimeTestFixture;

TEST_F(OperationsTest, IsSameValueTest) {
  PinnedHermesValue v1;
  PinnedHermesValue v2;
  PinnedHermesValue v3;

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeNullValue();
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeUndefinedValue();
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeUndefinedValue();
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(true);
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(false);
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeBoolValue(false);
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(123.45);
  v2 = HermesValue::encodeDoubleValue(123.45);
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(-0.0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity());
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeNumberValue(+0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(-134);
  v2 = HermesValue::encodeDoubleValue(-134.0);
  IsSameValueTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(182);
  v2 = HermesValue::encodeDoubleValue(182.54);
  IsSameValueTest(FALSE, v1, v2);

  Handle<JSObject> nullObj(runtime);
  auto obj1 = runtime->makeHandle(JSObject::create(runtime, nullObj));
  auto obj2 = runtime->makeHandle(JSObject::create(runtime, nullObj));

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(&obj2);
  IsSameValueTest(FALSE, v1, v2);

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(obj1.get());
  IsSameValueTest(TRUE, v1, v2);

  auto s1 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s2 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s3 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hi earth"));

  v1 = HermesValue::encodeStringValue(s1.get());
  v2 = HermesValue::encodeStringValue(s2.get());
  v3 = HermesValue::encodeStringValue(s3.get());
  IsSameValueTest(TRUE, v1, v2);
  IsSameValueTest(FALSE, v1, v3);

  v1 = HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(1));
  v2 = HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(2));
  IsSameValueTest(FALSE, v1, v2);
  IsSameValueTest(TRUE, v1, v1);

  v2 = HermesValue::encodeBoolValue(true);
  IsSameValueTest(FALSE, v1, v2);

  v2 = HermesValue::encodeDoubleValue(0);
  IsSameValueTest(FALSE, v1, v2);
}

#define AbstractEqualityTest(result, x, y)                          \
  {                                                                 \
    auto xHandle = runtime->makeHandle(x);                          \
    auto yHandle = runtime->makeHandle(y);                          \
    auto res = abstractEqualityTest_RJS(runtime, xHandle, yHandle); \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());          \
    EXPECT_##result(res->getBool());                                \
                                                                    \
    res = abstractEqualityTest_RJS(runtime, yHandle, xHandle);      \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());          \
    EXPECT_##result(res->getBool());                                \
  }

TEST_F(OperationsTest, AbstractEqualityTest) {
  GCScope gcScope{runtime, "OperationsTest.AbstractEqualityTest", 200};

  PinnedHermesValue v1;
  PinnedHermesValue v2;
  PinnedHermesValue v3;

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeNullValue();
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeUndefinedValue();
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeUndefinedValue();
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(true);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(false);
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeBoolValue(false);
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(123.45);
  v2 = HermesValue::encodeDoubleValue(123.45);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  v2 = HermesValue::encodeDoubleValue(123.563);
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(-0.0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity());
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeNumberValue(+0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(-134);
  v2 = HermesValue::encodeDoubleValue(-134.0);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(182);
  v2 = HermesValue::encodeDoubleValue(182.54);
  AbstractEqualityTest(FALSE, v1, v2);

  Handle<JSObject> nullObj(runtime);
  auto obj1 = runtime->makeHandle(JSObject::create(runtime, nullObj));
  auto obj2 = runtime->makeHandle(JSObject::create(runtime, nullObj));

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(obj2.get());
  AbstractEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(obj1.get());
  AbstractEqualityTest(TRUE, v1, v2);

  auto s1 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s2 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s3 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hi earth"));
  v1 = HermesValue::encodeStringValue(s1.get());
  v2 = HermesValue::encodeStringValue(s2.get());
  v3 = HermesValue::encodeStringValue(s3.get());
  AbstractEqualityTest(TRUE, v1, v2);
  AbstractEqualityTest(FALSE, v1, v3);

  auto s4 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"1000"));
  v1 = HermesValue::encodeStringValue(s4.get());
  v2 = HermesValue::encodeDoubleValue(1000);
  AbstractEqualityTest(TRUE, v1, v2);

  auto s5 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"Infinity"));
  v1 = HermesValue::encodeStringValue(s5.get());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeDoubleValue(1);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(false);
  v2 = HermesValue::encodeDoubleValue(0);
  AbstractEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeDoubleValue(153);
  AbstractEqualityTest(FALSE, v1, v2);

  // TODO: Test Object equality once Runtime::interpretFunction() is written.
}

#define StrictEqualityTest(result, v1, v2)       \
  {                                              \
    EXPECT_##result(strictEqualityTest(v1, v2)); \
    EXPECT_##result(strictEqualityTest(v2, v1)); \
  }

TEST_F(OperationsTest, StrictEquaityTest) {
  PinnedHermesValue v1;
  PinnedHermesValue v2;
  PinnedHermesValue v3;

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeNullValue();
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeUndefinedValue();
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNullValue();
  v2 = HermesValue::encodeUndefinedValue();
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(true);
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeBoolValue(true);
  v2 = HermesValue::encodeBoolValue(false);
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeUndefinedValue();
  v2 = HermesValue::encodeBoolValue(false);
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(123.45);
  v2 = HermesValue::encodeDoubleValue(123.45);
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(-0.0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  v2 = HermesValue::encodeDoubleValue(3);
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN());
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity());
  v2 = HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity());
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeNumberValue(+0);
  v2 = HermesValue::encodeDoubleValue(+0.0);
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(-134);
  v2 = HermesValue::encodeDoubleValue(-134.0);
  StrictEqualityTest(TRUE, v1, v2);

  v1 = HermesValue::encodeNumberValue(182);
  v2 = HermesValue::encodeDoubleValue(182.54);
  StrictEqualityTest(FALSE, v1, v2);

  Handle<JSObject> nullObj(runtime);
  auto obj1 = runtime->makeHandle(JSObject::create(runtime, nullObj));
  auto obj2 = runtime->makeHandle(JSObject::create(runtime, nullObj));

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(&obj2);
  StrictEqualityTest(FALSE, v1, v2);

  v1 = HermesValue::encodeObjectValue(obj1.get());
  v2 = HermesValue::encodeObjectValue(obj1.get());
  StrictEqualityTest(TRUE, v1, v2);

  auto s1 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s2 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  auto s3 =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hi earth"));

  v1 = HermesValue::encodeStringValue(s1.get());
  v2 = HermesValue::encodeStringValue(s2.get());
  v3 = HermesValue::encodeStringValue(s3.get());
  StrictEqualityTest(TRUE, v1, v2);
  StrictEqualityTest(FALSE, v1, v3);
}

TEST_F(OperationsTest, IsPrimitiveTest) {
  PinnedHermesValue v;

  v = HermesValue::encodeNumberValue(1);
  EXPECT_TRUE(isPrimitive(v));

  v = HermesValue::encodeNullValue();
  EXPECT_TRUE(isPrimitive(v));

  v = HermesValue::encodeUndefinedValue();
  EXPECT_TRUE(isPrimitive(v));

  v = HermesValue::encodeBoolValue(true);
  EXPECT_TRUE(isPrimitive(v));

  v = HermesValue::encodeDoubleValue(1.21);
  EXPECT_TRUE(isPrimitive(v));

  auto s =
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello world"));
  v = HermesValue::encodeStringValue(s.get());
  EXPECT_TRUE(isPrimitive(v));

  Handle<JSObject> nullObj(runtime);
  auto obj = JSObject::create(runtime, nullObj);
  v = HermesValue::encodeObjectValue(&obj);
  EXPECT_FALSE(isPrimitive(v));
}

TEST_F(OperationsTest, ToBooleanTest) {
  {
    EXPECT_FALSE(toBoolean(HermesValue::encodeUndefinedValue()));
    EXPECT_FALSE(toBoolean(HermesValue::encodeNullValue()));
  }

  {
    EXPECT_TRUE(toBoolean(HermesValue::encodeBoolValue(true)));
    EXPECT_FALSE(toBoolean(HermesValue::encodeBoolValue(false)));
  }

  {
    auto empty = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u""));
    EXPECT_FALSE(toBoolean(HermesValue::encodeStringValue(empty.get())));

    auto full =
        StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"full"));
    EXPECT_TRUE(toBoolean(HermesValue::encodeStringValue(full.get())));
  }

  {
    EXPECT_FALSE(toBoolean(HermesValue::encodeDoubleValue(0)));
    EXPECT_FALSE(toBoolean(HermesValue::encodeDoubleValue(-0.0)));
    EXPECT_FALSE(toBoolean(HermesValue::encodeNaNValue()));
    EXPECT_FALSE(toBoolean(HermesValue::encodeNumberValue(0)));

    EXPECT_TRUE(toBoolean(HermesValue::encodeDoubleValue(123.34)));
    EXPECT_TRUE(toBoolean(HermesValue::encodeNumberValue(123)));
  }
}

// Use macros for these tests because they're verbose.
#define ToStringTest(result, value)                                     \
  {                                                                     \
    Handle<> scopedValue = runtime->makeHandle(value);                  \
    auto strRes = toString_RJS(runtime, scopedValue);                   \
    EXPECT_EQ(ExecutionStatus::RETURNED, strRes.getStatus());           \
    EXPECT_TRUE(                                                        \
        StringPrimitive::createStringView(                              \
            runtime, runtime->makeHandle(std::move(strRes.getValue()))) \
            .equals(createUTF16Ref(result)));                           \
  }

#define DoubleToStringTest(result, value) \
  ToStringTest(result, HermesValue::encodeDoubleValue(value));

#define SmallIntToStringTest(result, value) \
  ToStringTest(result, HermesValue::encodeNumberValue(value));

TEST_F(OperationsTest, ToStringTest) {
  GCScope gcScope{runtime, "OperationsTest.ToStringTest", 128};

  // Simple tests
  {
    ToStringTest(u"null", HermesValue::encodeNullValue());
    ToStringTest(u"undefined", HermesValue::encodeUndefinedValue());
    ToStringTest(u"true", HermesValue::encodeBoolValue(true));
    ToStringTest(u"false", HermesValue::encodeBoolValue(false));
  }

  // Strings
  {
    auto str1 =
        StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"asdf"));
    ToStringTest(u"asdf", HermesValue::encodeStringValue(str1.get()));
    auto str2 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u""));
    ToStringTest(u"", HermesValue::encodeStringValue(str2.get()));
  }

  // Numbers
  {
    DoubleToStringTest(u"NaN", std::numeric_limits<double>::quiet_NaN());

    auto infinity = std::numeric_limits<double>::infinity();
    DoubleToStringTest(u"Infinity", infinity);
    DoubleToStringTest(u"-Infinity", -infinity);

    DoubleToStringTest(u"0", +0.0);
    DoubleToStringTest(u"0", -0.0);
    DoubleToStringTest(u"3", 3);
    DoubleToStringTest(u"-3", -3);
    DoubleToStringTest(u"-10000", -10000);
    DoubleToStringTest(u"1100000", 1.1e6);

    DoubleToStringTest(u"10000", 10000);
    DoubleToStringTest(u"100000", 100000);
    DoubleToStringTest(u"1000000", 1000000);
    DoubleToStringTest(u"10000000", 10000000);
    DoubleToStringTest(u"100000000000000000000", 1e20);
    DoubleToStringTest(u"1e+21", 1e21);

    DoubleToStringTest(u"0.125", 0.125);
    DoubleToStringTest(u"0.5", 0.5);
    DoubleToStringTest(u"3.14", 3.14);
    DoubleToStringTest(u"3.14", 3.14);
    DoubleToStringTest(u"14583.1832", 14583.1832);
    DoubleToStringTest(u"-14583.1832", -14583.1832);
    DoubleToStringTest(u"1.23e+25", 1.23e25);
    DoubleToStringTest(u"1.23e-25", 1.23e-25);
    DoubleToStringTest(u"5e+25", 5e+25);
    DoubleToStringTest(u"-5e+25", -5e+25);

    SmallIntToStringTest(u"0", 0);
    SmallIntToStringTest(u"12384", 12384);
    SmallIntToStringTest(u"-12384", -12384);
  }

  // TODO: Test Object toString once Runtime::interpretFunction() is written.
}

// Use macros for these tests because they're verbose.
#define ToNumberTest(result, value)                        \
  {                                                        \
    Handle<> scopedValue = runtime->makeHandle(value);     \
    res = toNumber_RJS(runtime, scopedValue);              \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_EQ((double)result, res->getDouble());           \
  }

#define InvalidToNumberTest(value)                         \
  {                                                        \
    Handle<> scopedValue = runtime->makeHandle(value);     \
    res = toNumber_RJS(runtime, scopedValue);              \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_TRUE(std::isnan(res->getDouble()));             \
  }

#define StringToNumberTest(result, string)                               \
  {                                                                      \
    auto strPrim =                                                       \
        StringPrimitive::createNoThrow(runtime, createUTF16Ref(string)); \
    ToNumberTest(result, HermesValue::encodeStringValue(strPrim.get())); \
  }

#define InvalidStringToNumberTest(string)                                \
  {                                                                      \
    auto strPrim =                                                       \
        StringPrimitive::createNoThrow(runtime, createUTF16Ref(string)); \
    InvalidToNumberTest(HermesValue::encodeStringValue(strPrim.get()));  \
  }

using OperationsLargeHeapTest = LargeHeapRuntimeTestFixture;

TEST_F(OperationsLargeHeapTest, ToNumberTest) {
  GCScope gcScope{runtime, "OperationsTest.ToNumberTest", 200};

  // Simple tests
  {
    ToNumberTest(+0.0, HermesValue::encodeNullValue());
    InvalidToNumberTest(HermesValue::encodeUndefinedValue());
    ToNumberTest(1, HermesValue::encodeBoolValue(true));
    ToNumberTest(0, HermesValue::encodeBoolValue(false));
  }

  // Strings
  {
    // Infinity
    StringToNumberTest(std::numeric_limits<double>::infinity(), u"Infinity");
    StringToNumberTest(std::numeric_limits<double>::infinity(), u"+Infinity");
    StringToNumberTest(-std::numeric_limits<double>::infinity(), u"-Infinity");
    StringToNumberTest(std::numeric_limits<double>::infinity(), u" Infinity\t");
    StringToNumberTest(std::numeric_limits<double>::infinity(), u" +Infinity ");
    StringToNumberTest(-std::numeric_limits<double>::infinity(), u" -Infinity");

    // Integers
    StringToNumberTest(0, u"");
    StringToNumberTest(0, u"\t   ");
    StringToNumberTest(0, u"   0");
    StringToNumberTest(0, u"0   ");
    StringToNumberTest(3, u"   \t  3 \t \n \u2028 \uFEFF ");
    StringToNumberTest(3, u"3");
    StringToNumberTest(3, u"00000000000000000000000000000003");
    StringToNumberTest(-3, u"-3");
    StringToNumberTest(10000, u"10000");
    StringToNumberTest(-10000, u"-10000");
    StringToNumberTest(2147483647, u"2147483647");
    StringToNumberTest(-2147483647, u"-2147483647");

    // Signed zero
    StringToNumberTest(0, u"+0.0");
    EXPECT_FALSE(std::signbit(res->getDouble()));
    StringToNumberTest(0, u"-0.0");
    EXPECT_TRUE(std::signbit(res->getDouble()));

    // Non-integrals
    StringToNumberTest(3.14, u"3.14");
    StringToNumberTest(14583.1832, u"14583.1832");
    StringToNumberTest(-14583.1832, u"-14583.1832");
    StringToNumberTest(1.23e25, u"1.23e+25");
    StringToNumberTest(1.23e-25, u"1.23e-25");
    StringToNumberTest(13e8, u"13e8");
    StringToNumberTest(.11248, u".11248");
    StringToNumberTest(6853294837411.219, u"6853294837411.2184921374291384");
    StringToNumberTest(
        1.0948273409128347e+43,
        u"10948273409128347210948271309487321094712389");

    // Scientific notation
    StringToNumberTest(1.23e+25, u"1.23e+25");
    StringToNumberTest(1.23e+25, u"1.23e25");
    StringToNumberTest(1.23e+25, u"1.23E+25");
    StringToNumberTest(1.23e+25, u"1.23E25");
    StringToNumberTest(1.23e+25, u"+1.23E25");
    StringToNumberTest(1.23e-25, u"+1.23E-25");
    StringToNumberTest(1.23e-25, u"+1.23e-25");
    StringToNumberTest(-1.23e-25, u"-1.23E-25");
    StringToNumberTest(4.3214123123e+29, u"43214.123123e25");
    StringToNumberTest(
        std::numeric_limits<double>::infinity(), u"6143812.482134891732e12904");

    // Hex
    StringToNumberTest(0x234aL, u"0x234a");
    StringToNumberTest(0x234aL, u"0X234A");
    StringToNumberTest(
        6.582018229284824e+63,
        u"0xfffffffffffffffffffffffffffffffffffffffffffffffffffff");
    StringToNumberTest(0xdeadbeefL, u"0xdeAdBeeF");

    // Special hex
    StringToNumberTest(1152921504606847200, u"0x1000000000000081");
    StringToNumberTest(1152921504606847200, u"0x1000000000000084");
    StringToNumberTest(1152921504606847000, u"0x1000000000000079");

    // Octal
    StringToNumberTest(0, u"0o0");
    StringToNumberTest(9, u"0o11");

    // Binary
    StringToNumberTest(0, u"0b0");
    StringToNumberTest(3, u"0b11");
    StringToNumberTest(1024, u"0b10000000000");
  }

  // Big hex number
  {
    auto str = StringPrimitive::createNoThrow(
        runtime, createUTF16Ref(u"0xFFFFFFFFFFFFFFFF"));
    auto scopedVal =
        runtime->makeHandle(HermesValue::encodeStringValue(str.get()));
    res = toNumber_RJS(runtime, scopedVal);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    auto strRes = toString_RJS(runtime, runtime->makeHandle(res.getValue()));
    EXPECT_EQ(ExecutionStatus::RETURNED, strRes.getStatus());
    EXPECT_TRUE(StringPrimitive::createStringView(
                    runtime, runtime->makeHandle(std::move(*strRes)))
                    .equals(createUTF16Ref(u"18446744073709552000")));
  }

  // Invalid strings
  {
    InvalidStringToNumberTest(u"NaN");
    InvalidStringToNumberTest(u"  NaN  ");
    InvalidStringToNumberTest(u"0x");
    InvalidStringToNumberTest(u"  0x");
    InvalidStringToNumberTest(u"0x123E-4");
    InvalidStringToNumberTest(u"-0x234a");
    InvalidStringToNumberTest(u"-0xdeAdBeeF");
    InvalidStringToNumberTest(u"0xDEADBEEG");
    InvalidStringToNumberTest(u"0xdeadbeeg");
    InvalidStringToNumberTest(u"aawpfeioj");
    InvalidStringToNumberTest(u"123asdf");
    InvalidStringToNumberTest(u"123.123.123.123");
    InvalidStringToNumberTest(u"4 6 7 ");
    InvalidStringToNumberTest(u" 1 a ");
    InvalidStringToNumberTest(u"!(*#&!");
    InvalidStringToNumberTest(u"1֍  ");
  }

  // TODO: Test Object toNumber once Runtime::interpretFunction() is written.
}

#define ToIntegerTest(result, value)                       \
  {                                                        \
    auto scopedValue = runtime->makeHandle(value);         \
    res = toInteger(runtime, scopedValue);                 \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_EQ(result, res->getNumber());                   \
  }

TEST_F(OperationsTest, ToIntegerTest) {
  ToIntegerTest(0, HermesValue::encodeNullValue());
  ToIntegerTest(0, HermesValue::encodeUndefinedValue());

  ToIntegerTest(2, HermesValue::encodeDoubleValue(2.0));
  ToIntegerTest(2, HermesValue::encodeDoubleValue(2.4));
  ToIntegerTest(-2, HermesValue::encodeDoubleValue(-2.4));
  ToIntegerTest(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN()));
  ToIntegerTest(0, HermesValue::encodeDoubleValue(-0.0));
  ToIntegerTest(0, HermesValue::encodeDoubleValue(+0.0));

  ToIntegerTest(
      std::numeric_limits<double>::infinity(),
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity()));
  ToIntegerTest(
      -std::numeric_limits<double>::infinity(),
      HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity()));
}

#define ToInt32Test(result, value)                         \
  {                                                        \
    auto scopedValue = runtime->makeHandle(value);         \
    res = toInt32_RJS(runtime, scopedValue);               \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_EQ(result, res->getNumber());                   \
  }

TEST_F(OperationsTest, ToInt32Test) {
  ToInt32Test(0, HermesValue::encodeNullValue());
  ToInt32Test(0, HermesValue::encodeUndefinedValue());

  ToInt32Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity()));
  ToInt32Test(
      0,
      HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity()));

  ToInt32Test(2, HermesValue::encodeDoubleValue(2.0));
  ToInt32Test(2, HermesValue::encodeDoubleValue(2.4));
  ToInt32Test(-2, HermesValue::encodeDoubleValue(-2.4));
  ToInt32Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN()));
  ToInt32Test(0, HermesValue::encodeDoubleValue(-0.0));
  ToInt32Test(0, HermesValue::encodeDoubleValue(+0.0));

  ToInt32Test(1, HermesValue::encodeDoubleValue(std::pow(2, 33) + 1.0));
  ToInt32Test(-1, HermesValue::encodeDoubleValue(std::pow(2, 33) - 1.0));
  ToInt32Test(1238, HermesValue::encodeDoubleValue(std::pow(2, 33) + 1238.0));
  ToInt32Test(-1238, HermesValue::encodeDoubleValue(std::pow(2, 33) - 1238.0));
}

#define ToUInt32Test(result, value)                        \
  {                                                        \
    auto scopedValue = runtime->makeHandle(value);         \
    res = toUInt32_RJS(runtime, scopedValue);              \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_EQ(result, res->getNumber());                   \
  }

TEST_F(OperationsTest, ToUInt32Test) {
  ToUInt32Test(0, HermesValue::encodeNullValue());
  ToUInt32Test(0, HermesValue::encodeUndefinedValue());

  ToUInt32Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity()));
  ToUInt32Test(
      0,
      HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity()));

  ToUInt32Test(2, HermesValue::encodeDoubleValue(2.0));
  ToUInt32Test(2, HermesValue::encodeDoubleValue(2.4));
  ToUInt32Test(std::pow(2, 32) - 2, HermesValue::encodeDoubleValue(-2.4));
  ToUInt32Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN()));
  ToUInt32Test(0, HermesValue::encodeDoubleValue(-0.0));
  ToUInt32Test(0, HermesValue::encodeDoubleValue(+0.0));

  ToUInt32Test(1, HermesValue::encodeDoubleValue(std::pow(2, 33) + 1.0));
  ToUInt32Test(
      std::pow(2, 32) - 1,
      HermesValue::encodeDoubleValue(std::pow(2, 33) - 1.0));
  ToUInt32Test(1238, HermesValue::encodeDoubleValue(std::pow(2, 33) + 1238.0));
  ToUInt32Test(
      std::pow(2, 32) - 1238,
      HermesValue::encodeDoubleValue(std::pow(2, 33) - 1238.0));
}

#define ToUInt16Test(result, value)                        \
  {                                                        \
    auto scopedValue = runtime->makeHandle(value);         \
    res = toUInt16(runtime, scopedValue);                  \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus()); \
    EXPECT_EQ(result, res->getNumber());                   \
  }

TEST_F(OperationsTest, ToUInt16Test) {
  ToUInt16Test(0, HermesValue::encodeNullValue());
  ToUInt16Test(0, HermesValue::encodeUndefinedValue());

  ToUInt16Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::infinity()));
  ToUInt16Test(
      0,
      HermesValue::encodeDoubleValue(-std::numeric_limits<double>::infinity()));

  ToUInt16Test(2, HermesValue::encodeDoubleValue(2.0));
  ToUInt16Test(2, HermesValue::encodeDoubleValue(2.4));
  ToUInt16Test(std::pow(2, 16) - 2, HermesValue::encodeDoubleValue(-2.4));
  ToUInt16Test(
      0,
      HermesValue::encodeDoubleValue(std::numeric_limits<double>::quiet_NaN()));
  ToUInt16Test(0, HermesValue::encodeDoubleValue(-0.0));
  ToUInt16Test(0, HermesValue::encodeDoubleValue(+0.0));

  ToUInt16Test(1, HermesValue::encodeDoubleValue(std::pow(2, 18) + 1.0));
  ToUInt16Test(
      std::pow(2, 16) - 1,
      HermesValue::encodeDoubleValue(std::pow(2, 18) - 1.0));
  ToUInt16Test(1238, HermesValue::encodeDoubleValue(std::pow(2, 33) + 1238.0));
  ToUInt16Test(
      std::pow(2, 16) - 1238,
      HermesValue::encodeDoubleValue(std::pow(2, 18) - 1238.0));
}

TEST_F(OperationsTest, ToObjectTest) {
  {
    auto scopedVal = runtime->makeHandle(HermesValue::encodeNullValue());
    EXPECT_EQ(
        ExecutionStatus::EXCEPTION, toObject(runtime, scopedVal).getStatus());
    runtime->clearThrownValue();
  }

  {
    auto scopedVal = runtime->makeHandle(HermesValue::encodeUndefinedValue());
    EXPECT_EQ(
        ExecutionStatus::EXCEPTION, toObject(runtime, scopedVal).getStatus());
    runtime->clearThrownValue();
  }

  {
    auto scopedVal = runtime->makeHandle(HermesValue::encodeBoolValue(true));
    auto res = toObject(runtime, scopedVal);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    EXPECT_TRUE(res->isObject());
    auto obj = vmcast<JSBoolean>(static_cast<GCCell *>(res->getObject()));
    EXPECT_TRUE(obj->getPrimitiveBoolean());
  }

  {
    auto m = 1529.184;
    auto scopedVal = runtime->makeHandle(HermesValue::encodeDoubleValue(m));
    auto res = toObject(runtime, scopedVal);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    EXPECT_TRUE(res->isObject());
    auto obj = vmcast<JSNumber>(static_cast<GCCell *>(res->getObject()));
    EXPECT_EQ(m, obj->getPrimitiveNumber());
  }

  {
    auto strRef = createUTF16Ref(u"hello");
    auto str = StringPrimitive::createNoThrow(runtime, strRef);
    auto scopedVal =
        runtime->makeHandle(HermesValue::encodeStringValue(str.get()));
    auto res = toObject(runtime, scopedVal);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    EXPECT_TRUE(res->isObject());
    auto obj = vmcast<JSString>(static_cast<GCCell *>(res->getObject()));
    auto objStrHandle =
        runtime->makeHandle(JSString::getPrimitiveString(obj, runtime));
    EXPECT_TRUE(StringPrimitive::createStringView(runtime, objStrHandle)
                    .equals(strRef));
  }
}

#define NumberAdditionTest(result, x, y)                                      \
  {                                                                           \
    res = addOp_RJS(runtime, runtime->makeHandle(x), runtime->makeHandle(y)); \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());                    \
    EXPECT_TRUE(res.getValue().isNumber());                                   \
    EXPECT_EQ(result, res->getNumber());                                      \
  }

#define InvalidNumberAdditionTest(x, y)                                       \
  {                                                                           \
    res = addOp_RJS(runtime, runtime->makeHandle(x), runtime->makeHandle(y)); \
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());                    \
    EXPECT_TRUE(res.getValue().isNumber());                                   \
    EXPECT_TRUE(std::isnan(res->getNumber()));                                \
  }

#define DoubleAdditionTest(result, x, y)    \
  {                                         \
    NumberAdditionTest(                     \
        result,                             \
        HermesValue::encodeDoubleValue(x),  \
        HermesValue::encodeDoubleValue(y)); \
  }

TEST_F(OperationsTest, AdditionTest) {
  GCScope gcScope{runtime, "OperationsTest.AdditionTest", 128};

  {
    auto a = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abc"));
    auto b = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"def"));
    auto aHandle = runtime->makeHandle(HermesValue::encodeStringValue(a.get()));
    auto bHandle = runtime->makeHandle(HermesValue::encodeStringValue(b.get()));
    res = addOp_RJS(runtime, aHandle, bHandle);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    EXPECT_TRUE(res.getValue().isString());
    EXPECT_TRUE(
        StringPrimitive::createStringView(
            runtime,
            runtime->makeHandle(vmcast<StringPrimitive>(res.getValue())))
            .equals(createUTF16Ref(u"abcdef")));
  }

  {
    auto a =
        StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"number: "));
    auto aHandle = runtime->makeHandle(HermesValue::encodeStringValue(a.get()));
    auto bHandle = runtime->makeHandle(HermesValue::encodeDoubleValue(1.4));
    res = addOp_RJS(runtime, aHandle, bHandle);
    EXPECT_EQ(ExecutionStatus::RETURNED, res.getStatus());
    EXPECT_TRUE(res.getValue().isString());
    EXPECT_TRUE(
        StringPrimitive::createStringView(
            runtime,
            runtime->makeHandle(vmcast<StringPrimitive>(res.getValue())))
            .equals(createUTF16Ref(u"number: 1.4")));
  }

  {
    InvalidNumberAdditionTest(
        HermesValue::encodeDoubleValue(
            std::numeric_limits<double>::quiet_NaN()),
        HermesValue::encodeDoubleValue(1));
    InvalidNumberAdditionTest(
        HermesValue::encodeDoubleValue(1),
        HermesValue::encodeDoubleValue(
            std::numeric_limits<double>::quiet_NaN()));
  }

  {
    auto inf = std::numeric_limits<double>::infinity();
    InvalidNumberAdditionTest(
        HermesValue::encodeDoubleValue(inf),
        HermesValue::encodeDoubleValue(-inf));
    DoubleAdditionTest(inf, inf, inf);
    DoubleAdditionTest(-inf, -inf, -inf);
    DoubleAdditionTest(inf, inf, 4);
    DoubleAdditionTest(-inf, -inf, 4);
  }

  {
    DoubleAdditionTest(5, 0, 5);
    DoubleAdditionTest(5, -0, 5);

    DoubleAdditionTest(+0.0, -5, 5);
    EXPECT_FALSE(std::signbit(res->getNumber()));

    DoubleAdditionTest(-0.0, -0.0, -0.0);
    EXPECT_TRUE(std::signbit(res->getNumber()));

    DoubleAdditionTest(-0.0, +0.0, +0.0);
    EXPECT_FALSE(std::signbit(res->getNumber()));

    DoubleAdditionTest(-0.0, -0.0, 0.0);
    EXPECT_FALSE(std::signbit(res->getNumber()));
  }

  {
    NumberAdditionTest(
        4.0,
        HermesValue::encodeDoubleValue(3.0),
        HermesValue::encodeBoolValue(true));
    NumberAdditionTest(
        3.0,
        HermesValue::encodeDoubleValue(3.0),
        HermesValue::encodeBoolValue(false));
  }

  {
    InvalidNumberAdditionTest(
        HermesValue::encodeDoubleValue(3.0),
        HermesValue::encodeUndefinedValue());
    NumberAdditionTest(
        3.0,
        HermesValue::encodeDoubleValue(3.0),
        HermesValue::encodeNullValue());
  }

  {
    DoubleAdditionTest(2, 1, 1);
    DoubleAdditionTest(185, 189, -4);
    DoubleAdditionTest(3528.142, 3527, 1.142);
  }
}
} // namespace
