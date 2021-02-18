/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes::platform_unicode;

TEST(PlatformUnicode, CaseTest) {
  llvh::SmallVector<char16_t, 16> str = {u'a', u'B', u'c', u'\u00df'};
  convertToCase(str, CaseConversion::ToUpper, false /* useCurrentLocale */);
// When using Windows NLS APIs, the character 'ß'(u'\u00df') stays the same when converted to upper case. 
// While, ICU (and all ICU based apps) APIs maps the code point to 'SS' (Ref: https://github.com/unicode-org/icu/blob/3ca5d8ca1559fb52d08bad61e350f0285f3edb37/icu4c/source/data/unidata/CaseFolding.txt#L121)
// The WinGlob behaviour is observable in internet explorer and non-chromium based Edge browser.
// This behariour can be observed in current Office apps too (Excel UPPER function, Word Upper Case transformation etc. ).
// I'd like to add that this discrepancy doesn't seem to be caused by any recent changes in the unicode character database.
// This ICU mapping can be tracked to year old versions of ICU. And the same is observed with Windows apps.
#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_WINGLOB
  ASSERT_EQ(4, str.size());
  EXPECT_EQ(u'A', str[0]);
  EXPECT_EQ(u'B', str[1]);
  EXPECT_EQ(u'C', str[2]);
  EXPECT_EQ(u'\u00df', str[3]);
#else 
  ASSERT_EQ(5, str.size());
  EXPECT_EQ(u'A', str[0]);
  EXPECT_EQ(u'B', str[1]);
  EXPECT_EQ(u'C', str[2]);
  EXPECT_EQ(u'S', str[3]);
  EXPECT_EQ(u'S', str[4]);
#endif
}

TEST(PlatformUnicode, VersionCheck) {
  // Make sure we have an up-to-date version of ICU.
  llvh::SmallVector<char16_t, 16> str = {u'A', u'\u180e', u'\u03a3'};
  convertToCase(str, CaseConversion::ToLower, false /* useCurrentLocale */);
  ASSERT_EQ(3, str.size());
  EXPECT_EQ(u'a', str[0]);
  EXPECT_EQ(u'\u180e', str[1]);

// Interestingly, ICU maps Σ (U+03A3) to ς (U+03C2) 
// While, Windows APIs maps Σ (U+03A3) to σ (U+03C3)
// Windows behaviour seems right and matches the Unicode data. Ref. (https://github.com/unicode-org/icu/blob/maint/maint-68/icu4c/source/data/unidata/CaseFolding.txt)
#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_WINGLOB
  EXPECT_EQ(u'\u03c3', str[2]);
#else
  EXPECT_EQ(u'\u03c2', str[2]);
#endif
}

TEST(PlatformUnicode, Normalize) {
  llvh::SmallVector<char16_t, 16> str = {u'\u1e9b', u'\u0323'};
  normalize(str, NormalizationForm::D);
  ASSERT_EQ(3, str.size());
  EXPECT_EQ(u'\u017f', str[0]);
  EXPECT_EQ(u'\u0323', str[1]);
  EXPECT_EQ(u'\u0307', str[2]);
}

} // namespace
