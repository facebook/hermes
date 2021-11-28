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

TEST(PlatformIntlBCP47Parser, LanguageIdTest) {
  LanguageTagParser parser1(u"en-US"); // language + county code
  parser1.parseUnicodeLocaleId();
  EXPECT_EQ(u"en-US", parser1.toString());
  ParsedLanguageIdentifier lang1 = *parser1.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"en", lang1.languageSubtag);
  EXPECT_EQ(u"US", lang1.regionSubtag);
  
  LanguageTagParser parser2(u"CMn-aRab-dE "); // language + script + country code
  parser2.parseUnicodeLocaleId();
  EXPECT_EQ(u"CMn-aRab-dE", parser2.toString());
  ParsedLanguageIdentifier lang2 = *parser2.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"cmn", lang2.languageSubtag);
  EXPECT_EQ(u"Arab", lang2.scriptSubtag);
  EXPECT_EQ(u"DE", lang2.regionSubtag);

  LanguageTagParser parser3(u"zh-319"); // language + region code
  parser3.parseUnicodeLocaleId();
  ParsedLanguageIdentifier lang3 = *parser3.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"zh", lang3.languageSubtag);
  EXPECT_EQ(u"319", lang3.regionSubtag);

  LanguageTagParser parser4(u"zh-319-u-abc-test"); // language + region code + extension
  parser4.parseUnicodeLocaleId();
  ParsedLanguageIdentifier lang4 = *parser4.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"zh", lang4.languageSubtag);
  EXPECT_EQ(u"319", lang4.regionSubtag);
  
  LanguageTagParser parser5(u"und-variant-alphabet-subtag"); // language + variant list
  parser5.parseUnicodeLocaleId();
  ParsedLanguageIdentifier lang5 = *parser5.getParsedLocaleId().languageIdentifier;
  EXPECT_EQ(u"und", lang5.languageSubtag);
  EXPECT_EQ(u"variant", lang5.variantSubtagList.back());
  EXPECT_EQ(u"alphabet", lang5.variantSubtagList.front());
}

TEST(PlatformIntlBCP47Parser, ExtensionText) {
  // Unicode extension test
  LanguageTagParser parser1(u"und-u-att-attr-nu-xx-latn-bob");
  parser1.parseUnicodeLocaleId();
  EXPECT_EQ(u"und-u-att-attr-nu-xx-latn-bob", parser1.toString());
  auto locale1 = parser1.getParsedLocaleId();
  EXPECT_EQ(u"und", locale1.languageIdentifier->languageSubtag);
  EXPECT_EQ(u"att", locale1.unicodeExtensionAttributes.front());
  EXPECT_EQ(u"attr", locale1.unicodeExtensionAttributes.back());
  auto keywordsIt = locale1.unicodeExtensionKeywords.begin();
  EXPECT_EQ(u"nu", keywordsIt->first);
  EXPECT_EQ(0, keywordsIt->second->size());
  keywordsIt++;
  EXPECT_EQ(u"xx", keywordsIt->first);
  EXPECT_EQ(u"latn", keywordsIt->second->front());
  EXPECT_EQ(u"bob", keywordsIt->second->back());

  // Transformed extension test
  LanguageTagParser parser2(u"und");

  // Other extension test
  LanguageTagParser parser3(u"und-o-first-second-q-1o1");
  parser3.parseUnicodeLocaleId();
  auto locale3 = parser3.getParsedLocaleId();
  auto otherIt = locale3.otherExtensionMap.begin();
  EXPECT_EQ('o', otherIt->first);
  EXPECT_EQ(u"first", otherIt->second->front());
  EXPECT_EQ(u"second", otherIt->second->back());
  otherIt++;
  EXPECT_EQ('q', otherIt->first);
  EXPECT_EQ(u"1o1", otherIt->second->front());

  // PU extension test
  LanguageTagParser parser4(u"und-x-u-123");
  parser4.parseUnicodeLocaleId();
  auto locale4 = parser4.getParsedLocaleId();
  EXPECT_EQ(u"u", locale4.puExtensions.front());
  EXPECT_EQ(u"123", locale4.puExtensions.back());
}

} // namespace
