#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

#include "TestHelpers.h"

#include <climits>

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using StringPrimTest = RuntimeTestFixture;

TEST_F(StringPrimTest, CreateTest) {
  auto str1 = StringPrimitive::createNoThrow(runtime, "hello");
  auto str2 = StringPrimitive::createNoThrow(runtime, "foo");

  EXPECT_EQ(5u, str1->getStringLength());
  EXPECT_TRUE(StringPrimitive::createStringView(runtime, str1)
                  .equals(createUTF16Ref(u"hello")));

  EXPECT_EQ(3u, str2->getStringLength());
  EXPECT_TRUE(StringPrimitive::createStringView(runtime, str2)
                  .equals(createUTF16Ref(u"foo")));
}

TEST_F(StringPrimTest, EqualityTest) {
  auto s1 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abc"));
  auto s2 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abc"));
  auto s3 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abcdef"));
  EXPECT_TRUE(s1->equals(s2.get()));
  EXPECT_FALSE(s1->equals(s3.get()));
}

TEST_F(StringPrimTest, CreateASCIITest) {
  auto s1 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"hello"));
  auto s2 = StringPrimitive::createNoThrow(runtime, "hello");
  EXPECT_TRUE(s1->equals(s2.get()));
}

TEST_F(StringPrimTest, CompareTest) {
#define TEST_CMP(v, a, b)                                 \
  {                                                       \
    auto s1 = StringPrimitive::createNoThrow(runtime, a); \
    auto s2 = StringPrimitive::createNoThrow(runtime, b); \
    EXPECT_EQ(v, s1->compare(s2.get()));                  \
  }

  TEST_CMP(0, "", "");
  TEST_CMP(-1, "", "a");
  TEST_CMP(+1, "a", "");

  TEST_CMP(0, "123", "123");
  TEST_CMP(-1, "12", "123");
  TEST_CMP(+1, "123", "12");

  TEST_CMP(-1, "123", "4");
  TEST_CMP(+1, "4", "123");

  TEST_CMP(-1, "123", "456");
  TEST_CMP(+1, "456", "123");

#undef TEST_CMP
}

TEST_F(StringPrimTest, ConcatTest) {
  {
    auto a = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abc"));
    auto b = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"def"));
    auto strRes = StringPrimitive::concat(runtime, a, b);
    ASSERT_NE(ExecutionStatus::EXCEPTION, strRes.getStatus());
    EXPECT_TRUE(StringPrimitive::createStringView(
                    runtime, runtime->makeHandle<StringPrimitive>(*strRes))
                    .equals(createUTF16Ref(u"abcdef")));
  }

  {
    auto a = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u""));
    auto b = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"abc"));
    auto strRes = StringPrimitive::concat(runtime, a, b);
    ASSERT_NE(ExecutionStatus::EXCEPTION, strRes.getStatus());
    EXPECT_TRUE(StringPrimitive::createStringView(
                    runtime, runtime->makeHandle<StringPrimitive>(*strRes))
                    .equals(createUTF16Ref(u"abc")));
  }
}

} // namespace
