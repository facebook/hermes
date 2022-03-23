/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"

#include "gtest/gtest.h"

namespace {
using namespace hermes::platform_intl;

TEST(BCP47Parser, LanguageIdTest) {
  // language + county code
  {
    auto res = *ParsedLocaleIdentifier::parse(u"en-US");
    EXPECT_EQ(u"en", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(u"us", res.languageIdentifier.regionSubtag);
    EXPECT_EQ(u"en-US", res.canonicalize());
  }

  // language + script + country code
  {
    auto res = *ParsedLocaleIdentifier::parse(u"CMn-aRab-dE");
    EXPECT_EQ(u"cmn", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(u"arab", res.languageIdentifier.scriptSubtag);
    EXPECT_EQ(u"de", res.languageIdentifier.regionSubtag);
    EXPECT_EQ(u"cmn-Arab-DE", res.canonicalize());
  }

  // language + region code
  {
    auto res = *ParsedLocaleIdentifier::parse(u"zh-319");
    EXPECT_EQ(u"zh", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(u"319", res.languageIdentifier.regionSubtag);
    EXPECT_EQ(u"zh-319", res.canonicalize());
  }

  // language + region code + extension
  {
    auto res = *ParsedLocaleIdentifier::parse(u"zh-319-u-abc-test");
    EXPECT_EQ(u"zh", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(u"319", res.languageIdentifier.regionSubtag);
    EXPECT_EQ(u"zh-319-u-abc-test", res.canonicalize());
  }

  // language + variant list
  {
    auto res = *ParsedLocaleIdentifier::parse(u"und-variant-alphabet-subtag");
    EXPECT_EQ(u"und", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(u"variant", *res.languageIdentifier.variantSubtagList.rbegin());
    EXPECT_EQ(u"alphabet", *res.languageIdentifier.variantSubtagList.begin());
    EXPECT_EQ(u"und-alphabet-subtag-variant", res.canonicalize());
  }
}

TEST(BCP47Parser, ExtensionText) {
  // Unicode extension test
  {
    auto res = *ParsedLocaleIdentifier::parse(
        u"und-u-att-attr-attr-nu-xx-latn-bob-xx-bob");
    EXPECT_EQ(u"und", res.languageIdentifier.languageSubtag);
    EXPECT_EQ(2, res.unicodeExtensionAttributes.size());
    EXPECT_EQ(1, res.unicodeExtensionAttributes.count(u"att"));
    EXPECT_EQ(1, res.unicodeExtensionAttributes.count(u"attr"));

    const auto &extKeys = res.unicodeExtensionKeywords;
    EXPECT_EQ(2, extKeys.size());
    EXPECT_TRUE(extKeys.at(u"nu").empty());
    EXPECT_EQ(u"latn-bob", extKeys.at(u"xx"));

    EXPECT_EQ(u"und-u-att-attr-nu-xx-latn-bob", res.canonicalize());
  }

  // Transformed extension test
  {
    auto res =
        *ParsedLocaleIdentifier::parse(u"und-t-en-test-US-a9-ecma402-262test");
    EXPECT_EQ(u"en", res.transformedLanguageIdentifier.languageSubtag);
    EXPECT_EQ(u"test", res.transformedLanguageIdentifier.scriptSubtag);
    EXPECT_EQ(u"us", res.transformedLanguageIdentifier.regionSubtag);

    const auto &tef = res.transformedExtensionFields;
    EXPECT_EQ(1, tef.size());
    EXPECT_EQ(u"ecma402-262test", tef.at(u"a9"));

    EXPECT_EQ(u"und-t-en-test-us-a9-ecma402-262test", res.canonicalize());
  }

  // Other extension test
  {
    auto res = *ParsedLocaleIdentifier::parse(u"und-o-first-second-q-1o1");
    const auto &oem = res.otherExtensionMap;
    EXPECT_EQ(u"first-second", oem.at(u'o'));
    EXPECT_EQ(u"1o1", oem.at(u'q'));

    EXPECT_EQ(u"und-o-first-second-q-1o1", res.canonicalize());
  }

  // PU extension test
  {
    auto res = *ParsedLocaleIdentifier::parse(u"und-x-u-123");
    EXPECT_EQ(u"u-123", res.puExtensions);
    EXPECT_EQ(u"und-x-u-123", res.canonicalize());
  }
}

TEST(BCP47Parser, ErrorTest) {
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-US-"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-US--u-att"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"-en-US"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"-"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u""));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-US-x-"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-a-foo-b-bar-a-baz"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-u-xx-u-yy"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-t-xx-t-yy"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-t-xx-u-xx-s-yy-t-ww"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-scouse-fonipa-scouse"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-scouse-fonipa-scouse"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-t-en-us-arab"));
  EXPECT_FALSE(ParsedLocaleIdentifier::parse(u"en-t-m0-foo-m0-bar"));
}

TEST(BCP47Parser, Canonicalization) {
  auto cano = [](std::u16string str) {
    return ParsedLocaleIdentifier::parse(str)->canonicalize();
  };

  EXPECT_EQ(u"en-US", cano(u"En-uS"));
  EXPECT_EQ(u"en-Scrt-US", cano(u"En-scrt-us"));
  EXPECT_EQ(u"en-US-u-attr1-attr2", cano(u"en-us-u-attr2-attr1-attr1"));
  EXPECT_EQ(u"en-US-u-attr-bb-bbb-aaa", cano(u"en-US-u-attr-bb-bbb-aaa"));
  EXPECT_EQ(u"en-US-u-aa-ccc-bb-bbb", cano(u"en-US-u-bb-bbb-bb-aaa-aa-ccc"));
  EXPECT_EQ(u"en-US-fonipa-scouse", cano(u"en-US-scouse-fonipa"));
  EXPECT_EQ(
      u"en-US-a-xx-s-ff-t-en-us-u-attr-z-aa-bb",
      cano(u"en-us-z-aa-bb-u-attr-t-en-US-a-xx-s-ff"));
  EXPECT_EQ(
      u"en-u-bar-foo-ca-buddhist-kk-nu-thai",
      cano(u"en-u-foo-bar-nu-thai-ca-buddhist-kk-true"));
  EXPECT_EQ(u"en-u-1a-5a-true-true1", cano(u"en-u-5a-true-true1-1a-true"));
  EXPECT_EQ(
      u"en-US-a-xx-t-a1-foo-u-attr-z-aa-bb-x-abcd-efgh5",
      cano(u"en-us-z-aa-bb-u-attr-t-a1-foo-a-xx-x-abcd-efgh5"));
}

} // namespace
