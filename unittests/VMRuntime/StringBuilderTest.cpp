/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringBuilder.h"
#include "hermes/ADT/SafeInt.h"
#include "hermes/VM/StringView.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using StringBuilderTest = RuntimeTestFixture;

TEST_F(StringBuilderTest, NormalASCIIBuildTest) {
  auto builder =
      StringBuilder::createStringBuilder(runtime, hermes::SafeUInt32{10}, true);
  ASSERT_NE(builder, ExecutionStatus::EXCEPTION);
  builder->appendASCIIRef(createASCIIRef("abc"));
  builder->appendCharacter('d');
  builder->appendStringPrim(StringPrimitive::createNoThrow(runtime, "efghij"));
  auto result = builder->getStringPrimitive();
  ASSERT_TRUE(result->isASCII());
  ASSERT_EQ(result->getStringLength(), 10u);
  auto view = StringPrimitive::createStringView(runtime, result);
  ASSERT_TRUE(view.equals(createASCIIRef("abcdefghij")));
}

TEST_F(StringBuilderTest, NormalUTF16BuildTest) {
  auto builder =
      StringBuilder::createStringBuilder(runtime, hermes::SafeUInt32{10});
  ASSERT_NE(builder, ExecutionStatus::EXCEPTION);
  builder->appendASCIIRef(createASCIIRef("abc"));
  builder->appendUTF16Ref(createUTF16Ref(u"def"));
  builder->appendCharacter(u'g');
  builder->appendStringPrim(
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hi")));
  builder->appendStringPrim(StringPrimitive::createNoThrow(runtime, "j"));
  auto result = builder->getStringPrimitive();
  ASSERT_FALSE(result->isASCII());
  ASSERT_EQ(result->getStringLength(), 10u);
  auto view = StringPrimitive::createStringView(runtime, result);
  ASSERT_TRUE(view.equals(createUTF16Ref(u"abcdefghij")));
}

TEST_F(StringBuilderTest, AbnormalBuildTest) {
  // In this test, we will start with a ASCII string builder, and turn it
  // into an UTF16 builder by appending UTF16 strings.
  auto builder =
      StringBuilder::createStringBuilder(runtime, hermes::SafeUInt32{6}, true);
  ASSERT_NE(builder, ExecutionStatus::EXCEPTION);
  builder->appendASCIIRef(createASCIIRef("abc"));
  builder->appendUTF16Ref(createUTF16Ref(u"def"));
  auto result1 = builder->getStringPrimitive();
  ASSERT_FALSE(result1->isASCII());
  ASSERT_EQ(result1->getStringLength(), 6u);
  auto view1 = StringPrimitive::createStringView(runtime, result1);
  ASSERT_TRUE(view1.equals(createUTF16Ref(u"abcdef")));

  builder =
      StringBuilder::createStringBuilder(runtime, hermes::SafeUInt32{4}, true);
  ASSERT_NE(builder, ExecutionStatus::EXCEPTION);
  builder->appendASCIIRef(createASCIIRef("abc"));
  builder->appendCharacter(static_cast<char16_t>(256));
  auto result2 = builder->getStringPrimitive();
  ASSERT_FALSE(result2->isASCII());
  ASSERT_EQ(result2->getStringLength(), 4u);
  auto view2 = StringPrimitive::createStringView(runtime, result2);
  ASSERT_TRUE(view2.equals(createUTF16Ref(u"abc\x100")));

  builder =
      StringBuilder::createStringBuilder(runtime, hermes::SafeUInt32{8}, true);
  ASSERT_NE(builder, ExecutionStatus::EXCEPTION);
  builder->appendASCIIRef(createASCIIRef("abc"));
  builder->appendStringPrim(
      StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"defg\x100")));
  auto result3 = builder->getStringPrimitive();
  ASSERT_FALSE(result3->isASCII());
  ASSERT_EQ(result3->getStringLength(), 8u);
  auto view3 = StringPrimitive::createStringView(runtime, result3);
  ASSERT_TRUE(view3.equals(createUTF16Ref(u"abcdefg\x100")));
}
} // namespace
