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

// TODO: These tests rely on private functions
// TODO: exposed for the purpose of testing
TEST(PlatformIntlBCP47Parser, TagTokenizerTest) {
  std::u16string str1 = u"en-US-u-phonebk";
  LanguageTagParser parser(str1);
  EXPECT_EQ(str1, parser.toString());
  EXPECT_TRUE(parser.hasMoreSubtags());
  EXPECT_TRUE(parser.nextSubtag());
  EXPECT_EQ(parser.getCurrentSubtag(), u"en");
  EXPECT_TRUE(parser.nextSubtag());
  EXPECT_EQ(parser.getCurrentSubtag(), u"US");
  EXPECT_TRUE(parser.nextSubtag());
  EXPECT_EQ(parser.getCurrentSubtag(), u"u");
  EXPECT_TRUE(parser.nextSubtag());
  EXPECT_EQ(parser.getCurrentSubtag(), u"phonebk");
  EXPECT_FALSE(parser.nextSubtag());
  EXPECT_FALSE(parser.hasMoreSubtags());
  
  std::u16string str2 = u"und--";
  LanguageTagParser badParser(str2);
  EXPECT_EQ(str2, badParser.toString());
  EXPECT_TRUE(badParser.hasMoreSubtags());
  EXPECT_TRUE(badParser.nextSubtag());
  EXPECT_EQ(badParser.getCurrentSubtag(), u"und");
  EXPECT_FALSE(badParser.nextSubtag());
}

TEST(PlatformIntlBCP47Parser, LanguageIdTest) {
  LanguageTagParser parser1(u"en-US"); // language + county code
  parser1.parseUnicodeLanguageId();
  EXPECT_EQ(u"en-US", parser1.toString());
  ParsedLanguageIdentifier lang1 = parser1.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"en", lang1.languageSubtag);
  EXPECT_EQ(u"US", lang1.regionSubtag);
  
  LanguageTagParser parser2(u"cmn-Arab-dE"); // language + script + country code
  EXPECT_EQ(u"cmn-Arab-dE", parser2.toString());
  ParsedLanguageIdentifier lang2 = parser2.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"cmn", lang2.languageSubtag);
  EXPECT_EQ(u"Arab", lang2.scriptSubtag);
  EXPECT_EQ(u"dE", lang2.regionSubtag);
  
  LanguageTagParser parser3(u"zh-319"); // language + region code
  ParsedLanguageIdentifier lang3 = parser3.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"zh", lang3.languageSubtag);
  EXPECT_EQ(u"319", lang3.regionSubtag);
  
  LanguageTagParser parser4(u"zh-319-u-abc-test"); // language + region code + extension
  ParsedLanguageIdentifier lang4 = parser4.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"zh", lang4.languageSubtag);
  EXPECT_EQ(u"319", lang4.regionSubtag);
  
  EXPECT_TRUE(1);
}

} // namespace
