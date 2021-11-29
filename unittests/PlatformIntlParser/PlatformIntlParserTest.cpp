/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntlBCP47Parser.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes::bcp47_parser;

TEST(PlatformIntlBCP47Parser, LanguageIdTest) {
  // language + county code
  llvh::Optional<ParsedLocaleIdentifier> parserOpt = parseLocaleId(u"en-US");
  ParsedLocaleIdentifier parser1 = parserOpt.getValue();
  EXPECT_EQ(u"en", parser1.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"US", parser1.languageIdentifier.regionSubtag);
  
  // language + script + country code
  parserOpt = parseLocaleId(u"CMn-aRab-dE");
  ParsedLocaleIdentifier parser2 = parserOpt.getValue();
  EXPECT_EQ(u"cmn", parser2.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"Arab", parser2.languageIdentifier.scriptSubtag);
  EXPECT_EQ(u"DE", parser2.languageIdentifier.regionSubtag);

  // language + region code
  parserOpt = parseLocaleId(u"zh-319");
  ParsedLocaleIdentifier parser3 = parserOpt.getValue();
  EXPECT_EQ(u"zh", parser3.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"319", parser3.languageIdentifier.regionSubtag);

  // language + region code + extension
  parserOpt = parseLocaleId(u"zh-319-u-abc-test");
  ParsedLocaleIdentifier parser4 = parserOpt.getValue();
  EXPECT_EQ(u"zh", parser4.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"319", parser4.languageIdentifier.regionSubtag);
  
  // language + variant list
  parserOpt = parseLocaleId(u"und-variant-alphabet-subtag");
  ParsedLocaleIdentifier parser5 = parserOpt.getValue();
  EXPECT_EQ(u"und", parser5.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"variant", parser5.languageIdentifier.variantSubtagList.back());
  EXPECT_EQ(u"alphabet", parser5.languageIdentifier.variantSubtagList.front());
}

TEST(PlatformIntlBCP47Parser, ExtensionText) {
  // Unicode extension test
  llvh::Optional<ParsedLocaleIdentifier> parserOpt = parseLocaleId(u"und-u-att-attr-nu-xx-latn-bob");
  ParsedLocaleIdentifier locale1 = parserOpt.getValue();
  EXPECT_EQ(u"und", locale1.languageIdentifier.languageSubtag);
  EXPECT_EQ(u"att", locale1.unicodeExtensionAttributes.front());
  EXPECT_EQ(u"attr", locale1.unicodeExtensionAttributes.back());
  auto keywordsIt = locale1.unicodeExtensionKeywords.begin();
  EXPECT_EQ(u"xx", keywordsIt->first);
  EXPECT_EQ(u"latn", keywordsIt->second.front());
  EXPECT_EQ(u"bob", keywordsIt->second.back());
  keywordsIt++;
  EXPECT_EQ(u"nu", keywordsIt->first);
  EXPECT_EQ(0, keywordsIt->second.size());

  // Transformed extension test
  parserOpt = parseLocaleId(u"und-t-en-US");

  // Other extension test
  parserOpt = parseLocaleId(u"und-o-first-second-q-1o1");
  ParsedLocaleIdentifier locale3 = parserOpt.getValue();
  auto otherIt = locale3.otherExtensionMap.begin();
  EXPECT_EQ('q', otherIt->first);
  EXPECT_EQ(u"1o1", otherIt->second.front());
  otherIt++;
  EXPECT_EQ('o', otherIt->first);
  EXPECT_EQ(u"first", otherIt->second.front());
  EXPECT_EQ(u"second", otherIt->second.back());
  
  // PU extension test
  parserOpt = parseLocaleId(u"und-x-u-123");
  ParsedLocaleIdentifier locale4 = parserOpt.getValue();
  EXPECT_EQ(u"u", locale4.puExtensions.front());
  EXPECT_EQ(u"123", locale4.puExtensions.back());
}

} // namespace
