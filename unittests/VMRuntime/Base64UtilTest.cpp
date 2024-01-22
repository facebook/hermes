/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "hermes/ADT/SafeInt.h"
#include "hermes/VM/JSLib/Base64Util.h"
#include "hermes/VM/StringBuilder.h"

using namespace hermes::vm;

namespace {

using Base64UtilTest = RuntimeTestFixture;

#define EXPECT_ENCODED(original, expected)                                  \
  {                                                                         \
    uint64_t expectedLength = ((original.size() + 2) / 3) * 4;              \
    EXPECT_LE(expectedLength, std::numeric_limits<uint32_t>::max());        \
    hermes::SafeUInt32 outputLength{static_cast<uint32_t>(expectedLength)}; \
    CallResult<StringBuilder> builder =                                     \
        StringBuilder::createStringBuilder(runtime, outputLength, true);    \
    EXPECT_NE(builder, ExecutionStatus::EXCEPTION);                         \
                                                                            \
    bool success = base64Encode(original, *builder);                        \
    EXPECT_TRUE(success);                                                   \
    EXPECT_EQ(                                                              \
        builder->getStringPrimitive()->getStringRef<char>(),                \
        createASCIIRef(expected));                                          \
  }

#define EXPECT_ENCODED_ASCII_AND_UTF16(original, expected)      \
  {                                                             \
    ASCIIRef asciiRef = createASCIIRef(original);               \
    EXPECT_ENCODED(asciiRef, expected);                         \
                                                                \
    std::vector<char16_t> converted(asciiRef.size() + 1);       \
    uint32_t i = 0;                                             \
    for (i = 0; i < asciiRef.size(); i++) {                     \
      converted[i] = asciiRef[i];                               \
    }                                                           \
    converted[i] = '\0';                                        \
    EXPECT_ENCODED(createUTF16Ref(converted.data()), expected); \
  }

TEST_F(Base64UtilTest, EdgeCases) {
  EXPECT_ENCODED_ASCII_AND_UTF16("", "");
}

TEST_F(Base64UtilTest, EncodePaddingRequired) {
  EXPECT_ENCODED_ASCII_AND_UTF16("a", "YQ==");
  EXPECT_ENCODED_ASCII_AND_UTF16("ab", "YWI=");
  EXPECT_ENCODED_ASCII_AND_UTF16("abcd", "YWJjZA==");
  EXPECT_ENCODED_ASCII_AND_UTF16("abcde", "YWJjZGU=");
  EXPECT_ENCODED_ASCII_AND_UTF16(
      "less is more than more", "bGVzcyBpcyBtb3JlIHRoYW4gbW9yZQ==");
  EXPECT_ENCODED_ASCII_AND_UTF16("<>?su", "PD4/c3U=");

  EXPECT_ENCODED(UTF16Ref(std::array<char16_t, 1>{1}), "AQ==");
  EXPECT_ENCODED(ASCIIRef(std::array<char, 1>{1}), "AQ==");
  EXPECT_ENCODED(UTF16Ref(std::array<char16_t, 2>{1, 0}), "AQA=");
  EXPECT_ENCODED(ASCIIRef(std::array<char, 2>{1, 0}), "AQA=");
}

TEST_F(Base64UtilTest, EncodePaddingNotNeeded) {
  EXPECT_ENCODED_ASCII_AND_UTF16("abc", "YWJj");
  EXPECT_ENCODED_ASCII_AND_UTF16("abcdef", "YWJjZGVm");

  EXPECT_ENCODED(UTF16Ref(std::array<char16_t, 3>{0, 0, 0}), "AAAA");
  EXPECT_ENCODED(ASCIIRef(std::array<char, 3>{0, 0, 0}), "AAAA");
  EXPECT_ENCODED(UTF16Ref(std::array<char16_t, 3>{1, 0, 0}), "AQAA");
  EXPECT_ENCODED(ASCIIRef(std::array<char, 3>{1, 0, 0}), "AQAA");
}

TEST_F(Base64UtilTest, EncodeInvalid) {
  // Just a long enough buffer. All calls in this function are expected to fail.
  hermes::SafeUInt32 outputLength{20};
  CallResult<StringBuilder> builder =
      StringBuilder::createStringBuilder(runtime, outputLength, true);
  EXPECT_NE(builder, ExecutionStatus::EXCEPTION);
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"\U0001F600"), *builder));
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"a\U0001F600"), *builder));
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"ab\U0001F600"), *builder));
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"abc\U0001F600"), *builder));
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"\U0001F600xyz"), *builder));
  EXPECT_FALSE(base64Encode(createUTF16Ref(u"abc\U0001F600xyz"), *builder));
}

} // end anonymous namespace
