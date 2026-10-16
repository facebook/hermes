/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_JAVA && \
    HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_CF

#include "hermes/Platform/Unicode/UnicodeCaseConversion.h"

#include "gtest/gtest.h"

#include <iterator>
#include <string>

namespace {

using namespace hermes::unicode;

#include "CaseMappingTestData.inc"

std::u16string convert(
    const std::u16string &in,
    CaseConversion c,
    CaseLocale loc = CaseLocale::Root) {
  llvh::SmallVector<char16_t, 32> buf(in.begin(), in.end());
  convertCaseUTF16(buf, c, loc);
  return std::u16string(buf.begin(), buf.end());
}

/// Append \p cp to \p out as UTF-16.
void appendCodePoint(std::u16string &out, uint32_t cp) {
  if (cp > 0xFFFF) {
    uint32_t v = cp - 0x10000;
    out.push_back((char16_t)(0xD800 + (v >> 10)));
    out.push_back((char16_t)(0xDC00 + (v & 0x3FF)));
  } else {
    out.push_back((char16_t)cp);
  }
}

/// A single code point as a UTF-16 string (one unit, or a surrogate pair).
std::u16string one(uint32_t cp) {
  std::u16string out;
  appendCodePoint(out, cp);
  return out;
}

TEST(CaseConversion, TestDataIsPopulated) {
  EXPECT_GT(std::size(SIMPLE_UPPER_EXPECTATIONS), 1300u);
  EXPECT_GT(std::size(SIMPLE_LOWER_EXPECTATIONS), 1300u);
}

// Cross-checks the delta-block compression against expectations taken
// directly from UnicodeData.txt. This is the only test that isolates the
// encoding, which is where bugs hide.
//
// The expectation lists already exclude code points with a multi-character
// mapping in that direction, so every assertion here is unconditional. Do not
// reintroduce a size guard: it would let the test pass while asserting
// nothing.
TEST(CaseConversion, AllSimpleUpperMappings) {
  for (const auto &e : SIMPLE_UPPER_EXPECTATIONS) {
    EXPECT_EQ(one(e.mapped), convert(one(e.cp), CaseConversion::ToUpper))
        << "toUpper of U+" << std::hex << e.cp;
  }
}

TEST(CaseConversion, AllSimpleLowerMappings) {
  for (const auto &e : SIMPLE_LOWER_EXPECTATIONS) {
    EXPECT_EQ(one(e.mapped), convert(one(e.cp), CaseConversion::ToLower))
        << "toLower of U+" << std::hex << e.cp;
  }
}

TEST(CaseConversion, FullMappings) {
  // U+00DF eszett uppercases to "SS".
  EXPECT_EQ(u"SS", convert(u"\u00DF", CaseConversion::ToUpper));
  // U+FB03 ffi ligature uppercases to "FFI".
  EXPECT_EQ(u"FFI", convert(u"\uFB03", CaseConversion::ToUpper));
}

TEST(CaseConversion, FinalSigma) {
  // Preceded by a cased letter and not followed by one: final form.
  EXPECT_EQ(u"a\u03C2", convert(u"A\u03A3", CaseConversion::ToLower));
  // Followed by a cased letter: non-final form.
  EXPECT_EQ(u"a\u03C3b", convert(u"A\u03A3B", CaseConversion::ToLower));
  // Not preceded by a cased letter: non-final form.
  EXPECT_EQ(u"\u03C3", convert(u"\u03A3", CaseConversion::ToLower));
  // U+180E is case-ignorable, so it does not break the context.
  EXPECT_EQ(
      u"a\u180E\u03C2", convert(u"A\u180E\u03A3", CaseConversion::ToLower));
  EXPECT_EQ(
      u"a\u180E\u03C3\u180Eb",
      convert(u"A\u180E\u03A3\u180EB", CaseConversion::ToLower));
}

TEST(CaseConversion, TurkishDottedAndDotless) {
  EXPECT_EQ(
      u"\u0130", convert(u"i", CaseConversion::ToUpper, CaseLocale::Turkish));
  EXPECT_EQ(
      u"\u0131", convert(u"I", CaseConversion::ToLower, CaseLocale::Turkish));
  EXPECT_EQ(
      u"i", convert(u"\u0130", CaseConversion::ToLower, CaseLocale::Turkish));
  // Root must be unaffected.
  EXPECT_EQ(u"I", convert(u"i", CaseConversion::ToUpper));
  EXPECT_EQ(u"i", convert(u"I", CaseConversion::ToLower));
}

TEST(CaseConversion, TurkishAfterI) {
  // After_I: the dot above is removed when lowercasing after an I.
  EXPECT_EQ(
      u"i", convert(u"I\u0307", CaseConversion::ToLower, CaseLocale::Turkish));
  // Not After_I: the dot survives.
  EXPECT_EQ(
      u"a\u0307",
      convert(u"A\u0307", CaseConversion::ToLower, CaseLocale::Turkish));
}

TEST(CaseConversion, TurkishNotBeforeDot) {
  // I followed by a dot above is not Not_Before_Dot, so it stays dotted.
  EXPECT_EQ(
      u"i", convert(u"I\u0307", CaseConversion::ToLower, CaseLocale::Turkish));
  // A bare I is Not_Before_Dot, so it becomes dotless.
  EXPECT_EQ(
      u"\u0131", convert(u"I", CaseConversion::ToLower, CaseLocale::Turkish));
}

TEST(CaseConversion, LithuanianMoreAbove) {
  // I with a following above-mark gains an explicit dot when lowercased.
  EXPECT_EQ(
      u"i\u0307\u0300",
      convert(u"I\u0300", CaseConversion::ToLower, CaseLocale::Lithuanian));
  // Without an above-mark, no dot is added.
  EXPECT_EQ(
      u"i", convert(u"I", CaseConversion::ToLower, CaseLocale::Lithuanian));
}

TEST(CaseConversion, LithuanianAfterSoftDotted) {
  // Uppercasing removes the dot above after a soft-dotted character.
  EXPECT_EQ(
      u"I",
      convert(u"i\u0307", CaseConversion::ToUpper, CaseLocale::Lithuanian));
  // Root keeps it.
  EXPECT_EQ(u"I\u0307", convert(u"i\u0307", CaseConversion::ToUpper));
}

TEST(CaseConversion, WtfSixteenHazards) {
  // Unpaired surrogates and NUL pass through untouched.
  llvh::SmallVector<char16_t, 8> buf = {u'a', 0xD800, u'\0', 0xDC00, u'b'};
  convertCaseUTF16(buf, CaseConversion::ToUpper, CaseLocale::Root);
  ASSERT_EQ(5u, buf.size());
  EXPECT_EQ(u'A', buf[0]);
  EXPECT_EQ(0xD800, buf[1]);
  EXPECT_EQ(u'\0', buf[2]);
  EXPECT_EQ(0xDC00, buf[3]);
  EXPECT_EQ(u'B', buf[4]);
}

TEST(CaseConversion, SupplementaryPlane) {
  // U+10428 DESERET SMALL LETTER LONG I uppercases to U+10400.
  // \x rather than \u: \u forbids surrogate code points, even paired ones.
  // WARNING: a \x escape consumes every hex digit that follows it, so these
  // literals are well-formed only because a non-hex-digit (the next
  // backslash, or the closing quote) immediately follows each 4-digit
  // escape. Do not append text after \xDC00/\xDC28 that starts with a hex
  // digit ('0'-'9', 'a'-'f', 'A'-'F') -- it would silently be absorbed into
  // the preceding escape instead of becoming a separate character.
  EXPECT_EQ(u"\xD801\xDC00", convert(u"\xD801\xDC28", CaseConversion::ToUpper));
}

TEST(CaseConversion, EmptyString) {
  llvh::SmallVector<char16_t, 8> buf;
  convertCaseUTF16(buf, CaseConversion::ToUpper, CaseLocale::Root);
  EXPECT_TRUE(buf.empty());
}

} // namespace

#endif // not JAVA and not CF
