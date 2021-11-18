/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntlBCP47Parser.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes::platform_intl_parser;

TEST(PlatformIntlBCP47Parser, DigitTest) {
  EXPECT_TRUE(isASCIILetter(u'X'));
	EXPECT_FALSE(isASCIILetter(u'ç'));
  EXPECT_FALSE(isASCIILetter(u'2'));
  
  EXPECT_TRUE(isASCIIDigit(u'9'));
  EXPECT_FALSE(isASCIIDigit(u'ç'));
  EXPECT_FALSE(isASCIIDigit(u'a'));
  
  EXPECT_TRUE(isASCIILetterOrDigit(u'X'));
  EXPECT_FALSE(isASCIILetterOrDigit(u'ç'));
  EXPECT_TRUE(isASCIILetterOrDigit(u'c'));
  EXPECT_TRUE(isASCIILetterOrDigit(u'9'));
  
  EXPECT_TRUE(isSubtagSeparator(u'-'));
  EXPECT_FALSE(isSubtagSeparator(u'z'));
  
}

TEST(PlatformIntlBCP47Parser, SubtagTypeTest) {
  std::u16string testString = u"us-engus-Arab-123-a12";
                              //012345678901234567890
  EXPECT_TRUE(isUnicodeLanguageSubtag(testString, 0, 1));
  EXPECT_FALSE(isUnicodeLanguageSubtag(testString, 0, 2));
  EXPECT_TRUE(isUnicodeLanguageSubtag(testString, 3, 7));
  EXPECT_FALSE(isUnicodeLanguageSubtag(testString, 9, 12));
  EXPECT_FALSE(isUnicodeLanguageSubtag(testString, 14, 16));
  EXPECT_FALSE(isUnicodeLanguageSubtag(testString, 18, 20));
  EXPECT_FALSE(isUnicodeLanguageSubtag(testString, 18, 21));
  
  EXPECT_FALSE(isUnicodeScriptSubtag(testString, 0, 1));
  EXPECT_FALSE(isUnicodeScriptSubtag(testString, 0, 2));
  EXPECT_FALSE(isUnicodeScriptSubtag(testString, 3, 7));
  EXPECT_TRUE(isUnicodeScriptSubtag(testString, 9, 12));
  EXPECT_FALSE(isUnicodeScriptSubtag(testString, 14, 16));
  EXPECT_FALSE(isUnicodeScriptSubtag(testString, 18, 20));
  
  EXPECT_TRUE(isUnicodeRegionSubtag(testString, 0, 1));
  EXPECT_FALSE(isUnicodeRegionSubtag(testString, 0, 2));
  EXPECT_FALSE(isUnicodeRegionSubtag(testString, 3, 7));
  EXPECT_FALSE(isUnicodeRegionSubtag(testString, 9, 12));
  EXPECT_TRUE(isUnicodeRegionSubtag(testString, 14, 16));
  EXPECT_FALSE(isUnicodeRegionSubtag(testString, 18, 20));
  
  EXPECT_FALSE(isUnicodeVariantSubtag(testString, 0, 1));
  EXPECT_FALSE(isUnicodeVariantSubtag(testString, 0, 2));
  EXPECT_TRUE(isUnicodeVariantSubtag(testString, 3, 7));
  EXPECT_FALSE(isUnicodeVariantSubtag(testString, 9, 12));
  EXPECT_TRUE(isUnicodeVariantSubtag(testString, 14, 16));
  EXPECT_TRUE(isUnicodeVariantSubtag(testString, 18, 20));
}

TEST(PlatformIntlBCP47Parser, ExtensionTypeTest) {
  std::u16string testString = u"co-phonebk-5i-u-cu-usd-i5";
              //                0123456789012345678901234
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 0, 1));
  EXPECT_TRUE(isUnicodeExtensionAttribute(testString, 3, 9));
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 11, 12));
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 14, 14));
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 16, 17));
  EXPECT_TRUE(isUnicodeExtensionAttribute(testString, 19, 21));
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 23, 24));
  EXPECT_FALSE(isUnicodeExtensionAttribute(testString, 23, 25));
  
  EXPECT_TRUE(isUnicodeExtensionKey(testString, 0, 1));
  EXPECT_FALSE(isUnicodeExtensionKey(testString, 3, 9));
  EXPECT_TRUE(isUnicodeExtensionKey(testString, 11, 12));
  EXPECT_FALSE(isUnicodeExtensionKey(testString, 14, 14));
  EXPECT_TRUE(isUnicodeExtensionKey(testString, 16, 17));
  EXPECT_FALSE(isUnicodeExtensionKey(testString, 19, 21));
  EXPECT_FALSE(isUnicodeExtensionKey(testString, 23, 24));
  
  EXPECT_FALSE(isTransformedExtensionKey(testString, 0, 1));
  EXPECT_FALSE(isTransformedExtensionKey(testString, 3, 9));
  EXPECT_FALSE(isTransformedExtensionKey(testString, 11, 12));
  EXPECT_FALSE(isTransformedExtensionKey(testString, 14, 14));
  EXPECT_FALSE(isTransformedExtensionKey(testString, 16, 17));
  EXPECT_FALSE(isTransformedExtensionKey(testString, 19, 21));
  EXPECT_TRUE(isTransformedExtensionKey(testString, 23, 24));
  
  EXPECT_TRUE(isPrivateUseExtension(testString, 0, 1));
  EXPECT_TRUE(isPrivateUseExtension(testString, 3, 9));
  EXPECT_TRUE(isPrivateUseExtension(testString, 11, 12));
  EXPECT_TRUE(isPrivateUseExtension(testString, 14, 14));
  EXPECT_TRUE(isPrivateUseExtension(testString, 16, 17));
  EXPECT_TRUE(isPrivateUseExtension(testString, 19, 21));
  EXPECT_TRUE(isPrivateUseExtension(testString, 23, 24));
  
  EXPECT_TRUE(isOtherExtension(testString, 0, 1));
  EXPECT_TRUE(isOtherExtension(testString, 3, 9));
  EXPECT_TRUE(isOtherExtension(testString, 11, 12));
  EXPECT_FALSE(isOtherExtension(testString, 14, 14));
  EXPECT_TRUE(isOtherExtension(testString, 16, 17));
  EXPECT_TRUE(isOtherExtension(testString, 19, 21));
  EXPECT_TRUE(isOtherExtension(testString, 23, 24));
}

TEST(PlatformIntlBCP47Parser, LanguageIdTest) {
  EXPECT_TRUE(1);
}

} // namespace
