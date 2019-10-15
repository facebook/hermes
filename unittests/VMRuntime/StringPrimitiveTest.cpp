/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

#include "llvm/Support/AlignOf.h"

#include "TestHelpers.h"

#include <climits>
#include <random>

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

TEST_F(StringPrimTest, CreateEfficientTest) {
  auto handlefy = [&](CallResult<HermesValue> cr) {
    return Handle<StringPrimitive>::vmcast(runtime, *cr);
  };

  static_assert(
      StringPrimitive::EXTERNAL_STRING_MIN_SIZE > 3,
      "Test strings should be shorter than EXTERNAL_STRING_MIN_SIZE");
  std::string narrowShort = "foo";
  std::u16string wideShort(u"foo");
  auto s1 = handlefy(StringPrimitive::createEfficient(
      runtime, createASCIIRef(narrowShort.c_str())));
  auto s2 = handlefy(StringPrimitive::createEfficient(
      runtime, createUTF16Ref(wideShort.c_str())));
  auto s3 = handlefy(
      StringPrimitive::createEfficient(runtime, std::move(narrowShort)));
  auto s4 =
      handlefy(StringPrimitive::createEfficient(runtime, std::move(wideShort)));
  for (auto s : {s1, s2, s3, s4}) {
    EXPECT_TRUE(s->equals(s1.get()));
    EXPECT_EQ(3, s->getStringLength());
    EXPECT_FALSE(s->isExternal());
  }

  size_t longLength = StringPrimitive::EXTERNAL_STRING_MIN_SIZE;
  std::string narrowLong(longLength, '!');
  std::u16string wideLong(longLength, u'!');
  auto e1 = handlefy(StringPrimitive::createEfficient(
      runtime, createASCIIRef(narrowLong.c_str())));
  auto e2 = handlefy(StringPrimitive::createEfficient(
      runtime, createUTF16Ref(wideLong.c_str())));
  auto e3 = handlefy(
      StringPrimitive::createEfficient(runtime, std::move(narrowLong)));
  auto e4 =
      handlefy(StringPrimitive::createEfficient(runtime, std::move(wideLong)));
  for (auto s : {e1, e2, e3, e4}) {
    EXPECT_TRUE(s->equals(e1.get()));
    EXPECT_EQ(longLength, s->getStringLength());
    if (s == e3 || s == e4) {
      EXPECT_TRUE(s->isExternal());
    } else {
      EXPECT_FALSE(s->isExternal());
    }
  }
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

// This attempts to test that strings above a sufficient length may be freely
// memcpy'd around. This would not be true if the small-string optimization used
// an interior pointer, or if someone else maintained a pointer to the string.
template <typename StringType>
void test1StringMemcpySafety(const StringType &s) {
  // Grub through the string's bits, looking for pointers to the interior of the
  // string.
  intptr_t start = reinterpret_cast<intptr_t>(&s);
  intptr_t end = reinterpret_cast<intptr_t>(&s + 1);
  const intptr_t *stringGuts = reinterpret_cast<const intptr_t *>(&s);
  for (size_t i = 0; i < sizeof(StringType) / sizeof(void *); i++) {
    intptr_t p = stringGuts[i];
    EXPECT_FALSE(start <= p && p <= end) << "string size: " << s.size();
    // Mask off low bits in case it uses them as flags.
    p &= ~(intptr_t)1;
    EXPECT_FALSE(start <= p && p <= end) << "string size: " << s.size();
    p &= ~(intptr_t)2;
    EXPECT_FALSE(start <= p && p <= end) << "string size: " << s.size();
  }

  // memcpy our string and verify the buffer, when interpreted as a string, is
  // equal to the original.
  llvm::AlignedCharArrayUnion<StringType> mb;
  memcpy(mb.buffer, &s, sizeof(StringType));
  const StringType *memcpydStr =
      reinterpret_cast<const StringType *>(mb.buffer);
  EXPECT_EQ(s, *memcpydStr) << "string size: " << s.size();
  EXPECT_EQ(s.size(), memcpydStr->size()) << "string size: " << s.size();
  EXPECT_TRUE(std::equal(s.begin(), s.end(), memcpydStr->begin()))
      << "string size: " << s.size();

  // std::move a string. If any bits changed, it means the string is not memcpy
  // safe, because the move constructor does something nontrivial. Note it is
  // not the case in general that a moved-to string will be bitwise identical to
  // its source; the string may have an internal buffer whose last bytes are
  // unused when the string is externally allocated, and these last bytes may
  // not be copied by the move ctor. That is, the move ctor may be conceptually
  // a memcpy of only a prefix of the string. So copy the bits of the moved-from
  // string into a buffer, and then use placement new with move ctor; this
  // ensures that any ignored bytes have the same contents in both strings.
  StringType s1 = s;
  llvm::AlignedCharArrayUnion<StringType> savedBits;
  memcpy(savedBits.buffer, &s1, sizeof(StringType));

  llvm::AlignedCharArrayUnion<StringType> b = savedBits;
  StringType *movedString = new (b.buffer) StringType(std::move(s1));
  // We expect the move-constructed string to be bitwise identical to the string
  // before it was moved from. That is, the move ctor should be a memcpy.
  EXPECT_EQ(0, memcmp(b.buffer, &savedBits.buffer, sizeof(StringType)))
      << "string size: " << s.size();
  // Sanity check: the moved-from string should have changed, e.g. zeroed
  // pointer.
  EXPECT_NE(0, memcmp(&s1, &savedBits.buffer, sizeof(StringType)))
      << "string size: " << s.size();
  movedString->~StringType();
}

TEST_F(StringPrimTest, StringsAreMemcpySafe) {
  for (size_t length = StringPrimitive::EXTERNAL_STRING_MIN_SIZE; length < 2048;
       length++) {
    test1StringMemcpySafety(std::string(length, '!'));
    test1StringMemcpySafety(std::u16string(length, u'!'));
  }
}

} // namespace
