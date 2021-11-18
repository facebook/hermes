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
  EXPECT_TRUE(1);
}

} // namespace
