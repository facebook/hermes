/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
  ASSERT_EQ(5, str.size());
  EXPECT_EQ(u'A', str[0]);
  EXPECT_EQ(u'B', str[1]);
  EXPECT_EQ(u'C', str[2]);
  EXPECT_EQ(u'S', str[3]);
  EXPECT_EQ(u'S', str[4]);
}

TEST(PlatformUnicode, VersionCheck) {
  // Make sure we have an up-to-date version of ICU.
  llvh::SmallVector<char16_t, 16> str = {u'A', u'\u180e', u'\u03a3'};
  convertToCase(str, CaseConversion::ToLower, false /* useCurrentLocale */);
  ASSERT_EQ(3, str.size());
  EXPECT_EQ(u'a', str[0]);
  EXPECT_EQ(u'\u180e', str[1]);
  EXPECT_EQ(u'\u03c2', str[2]);
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
