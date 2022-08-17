/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Regex/Regex.h"
#include "hermes/Regex/Executor.h"
#include "hermes/Regex/RegexTraits.h"

#include <string>
#include "gtest/gtest.h"

using namespace hermes::regex;

namespace {

using cregex = Regex<UTF16RegexTraits>;
using cmatch = std::vector<CapturedRange>;

bool search(
    const char16_t *start,
    const char16_t *end,
    cmatch &m,
    const cregex &cr,
    constants::MatchFlagType flags = constants::matchDefault) {
  return searchWithBytecode(cr.compile(), start, 0, end - start, &m, flags) ==
      MatchRuntimeResult::Match;
}

bool search(
    const char16_t *str,
    cmatch &m,
    const cregex &cr,
    constants::MatchFlagType flags = constants::matchDefault) {
  return search(
      str, str + std::char_traits<char16_t>::length(str), m, cr, flags);
}

bool search(
    const std::u16string &text,
    cmatch &m,
    const cregex &cr,
    constants::MatchFlagType flags = constants::matchDefault) {
  return search(text.c_str(), m, cr, flags);
}

// Flatten a list of captured ranges into a string representation like:
// "(0-5) (1-3)"
std::string flatten(const std::vector<CapturedRange> &ranges) {
  std::string result;
  for (const auto &range : ranges) {
    if (!result.empty())
      result += " ";
    if (!range.matched()) {
      result += "nm";
    } else {
      result += '(';
      result += std::to_string(range.start);
      result += '-';
      result += std::to_string(range.end);
      result += ')';
    }
  }
  return result;
}

TEST(Regex, Short) {
  std::u16string pattern = u"a([0-9]+)b";
  cregex reg(pattern.c_str());
  std::vector<CapturedRange> matchRanges;

  const std::u16string text1 = u"a0b";
  EXPECT_TRUE(search(text1, matchRanges, reg, constants::matchDefault));
  EXPECT_EQ("(0-3) (1-2)", flatten(matchRanges));

  const std::u16string text2 = u"a1234b";
  EXPECT_TRUE(search(text2, matchRanges, reg, constants::matchDefault));
  EXPECT_EQ("(0-6) (1-5)", flatten(matchRanges));
}

MatchRuntimeResult searchResult(
    const char16_t *text,
    const char16_t *pattern,
    const char16_t *flags = u"") {
  cmatch m;
  return searchWithBytecode(
      cregex(pattern, flags).compile(),
      text,
      0,
      std::char_traits<char16_t>::length(text),
      &m,
      constants::matchDefault);
}

TEST(Regex, ExecFail) {
  EXPECT_EQ(
      searchResult(u"", u"(){999999999}"), MatchRuntimeResult::StackOverflow);
  EXPECT_EQ(
      searchResult(u"", u"(?=^){999999999}"),
      MatchRuntimeResult::StackOverflow);
}

/* The following tests are adapted from re.alg.search/ecma.pass.cpp in libc++ */
TEST(Regex, FromLibCXX) {
  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"a")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ab";
    EXPECT_TRUE(search(s, m, cregex(u"ab")));
    EXPECT_EQ("(0-2)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ab";
    EXPECT_FALSE(search(s, m, cregex(u"ba")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"aab";
    EXPECT_TRUE(search(s, m, cregex(u"ab")));
    EXPECT_EQ("(1-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcd";
    EXPECT_TRUE(search(s, m, cregex(u"bc")));
    EXPECT_EQ("(1-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab*c")));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ababc";
    EXPECT_TRUE(search(s, m, cregex(u"(ab)*c")));
    EXPECT_EQ("(0-5) (2-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcdefghijk";
    EXPECT_TRUE(search(s, m, cregex(u"cd((e)fg)hi")));
    EXPECT_EQ("(2-9) (4-7) (4-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcd";
    EXPECT_TRUE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"aabc";
    EXPECT_FALSE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"efabc";
    EXPECT_TRUE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("(2-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"efabcg";
    EXPECT_FALSE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"acc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"acc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcdef";
    EXPECT_TRUE(search(s, m, cregex(u"(.*).*")));
    EXPECT_EQ("(0-6) (0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"bc";
    EXPECT_TRUE(search(s, m, cregex(u"(a*)*")));
    EXPECT_EQ("(0-0) nm", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ("(0-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("(0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbbbc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adec";
    EXPECT_FALSE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefgc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefghc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefghic";
    EXPECT_FALSE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"tournament";
    EXPECT_TRUE(search(s, m, cregex(u"tour|to|tournament")));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"tournamenttotour";
    EXPECT_TRUE(search(s, m, cregex(u"(tour|to|tournament)+")));
    EXPECT_EQ("(0-4) (0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ttotour";
    EXPECT_TRUE(search(s, m, cregex(u"(tour|to|t)+")));
    EXPECT_EQ("(0-7) (3-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"-ab,ab-";
    EXPECT_FALSE(search(s, m, cregex(u"-(.*),\1-")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"-ab,ab-";
    EXPECT_TRUE(search(s, m, cregex(u"-.*,.*-")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"^[a]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"^[ab]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"c";
    EXPECT_TRUE(search(s, m, cregex(u"^[a-f]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"g";
    EXPECT_FALSE(search(s, m, cregex(u"^[a-f]$")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Iraqi";
    EXPECT_TRUE(search(s, m, cregex(u"q[^u]")));
    EXPECT_EQ("(3-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Iraq";
    EXPECT_FALSE(search(s, m, cregex(u"q[^u]")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"01a45cef9";
    EXPECT_TRUE(search(s, m, cregex(u"[ace1-9]*")));
    EXPECT_EQ("(0-0)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"01a45cef9";
    EXPECT_TRUE(search(s, m, cregex(u"[ace1-9]+")));
    EXPECT_EQ("(1-7)", flatten(m));
  }
  {
    const char16_t r[] = u"^[-+]?[0-9]+[CF]$";
    std::ptrdiff_t sr = std::char_traits<char16_t>::length(r);
    using FI = const char16_t *;
    using BI = const char16_t *;
    cregex regex(llvh::ArrayRef<char16_t>(FI(r), FI(r + sr)));
    cmatch m;
    const char16_t s[] = u"-40C";
    std::ptrdiff_t ss = std::char_traits<char16_t>::length(s);
    EXPECT_TRUE(search(BI(s), BI(s + ss), m, regex));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Jeff Jeffs u";
    EXPECT_TRUE(search(s, m, cregex(u"Jeff(?=s\\b)")));
    EXPECT_EQ("(5-9)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Jeffs Jeff";
    EXPECT_TRUE(search(s, m, cregex(u"Jeff(?!s\\b)")));
    EXPECT_EQ("(6-10)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"5%k";
    EXPECT_TRUE(search(s, m, cregex(u"\\d[\\W]k")));
    EXPECT_EQ("(0-3)", flatten(m));
  }

  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"a")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ab";
    EXPECT_TRUE(search(s, m, cregex(u"ab")));
    EXPECT_EQ("(0-2)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ab";
    EXPECT_FALSE(search(s, m, cregex(u"ba")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"aab";
    EXPECT_TRUE(search(s, m, cregex(u"ab")));
    EXPECT_EQ("(1-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcd";
    EXPECT_TRUE(search(s, m, cregex(u"bc")));
    EXPECT_EQ("(1-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab*c")));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ababc";
    EXPECT_TRUE(search(s, m, cregex(u"(ab)*c")));
    EXPECT_EQ("(0-5) (2-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcdefghijk";
    EXPECT_TRUE(search(s, m, cregex(u"cd((e)fg)hi")));
    EXPECT_EQ("(2-9) (4-7) (4-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcd";
    EXPECT_TRUE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"aabc";
    EXPECT_FALSE(search(s, m, cregex(u"^abc")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"efabc";
    EXPECT_TRUE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("(2-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"efabcg";
    EXPECT_FALSE(search(s, m, cregex(u"abc$")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"acc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"acc";
    EXPECT_TRUE(search(s, m, cregex(u"a.c")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abcdef";
    EXPECT_TRUE(search(s, m, cregex(u"(.*).*")));
    EXPECT_EQ("(0-6) (0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"bc";
    EXPECT_TRUE(search(s, m, cregex(u"(a*)*")));
    EXPECT_EQ("(0-0) nm", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("(0-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("(0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbbc";
    EXPECT_TRUE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"abbbbbbc";
    EXPECT_FALSE(search(s, m, cregex(u"ab{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adec";
    EXPECT_FALSE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefgc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-6)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefghc";
    EXPECT_TRUE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"adefghic";
    EXPECT_FALSE(search(s, m, cregex(u"a.{3,5}c")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"tournament";
    EXPECT_TRUE(search(s, m, cregex(u"tour|to|tournament")));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"tournamenttotour";
    EXPECT_TRUE(search(s, m, cregex(u"(tour|to|tournament)+")));
    EXPECT_EQ("(0-4) (0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"ttotour";
    EXPECT_TRUE(search(s, m, cregex(u"(tour|to|t)+")));
    EXPECT_EQ("(0-7) (3-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"-ab,ab-";
    EXPECT_FALSE(search(s, m, cregex(u"-(.*),\1-")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"-ab,ab-";
    EXPECT_TRUE(search(s, m, cregex(u"-.*,.*-")));
    EXPECT_EQ("(0-7)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"^[a]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"a";
    EXPECT_TRUE(search(s, m, cregex(u"^[ab]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"c";
    EXPECT_TRUE(search(s, m, cregex(u"^[a-f]$")));
    EXPECT_EQ("(0-1)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"g";
    EXPECT_FALSE(search(s, m, cregex(u"^[a-f]$")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Iraqi";
    EXPECT_TRUE(search(s, m, cregex(u"q[^u]")));
    EXPECT_EQ("(3-5)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Iraq";
    EXPECT_FALSE(search(s, m, cregex(u"q[^u]")));
    EXPECT_EQ("", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"01a45cef9";
    EXPECT_TRUE(search(s, m, cregex(u"[ace1-9]*")));
    EXPECT_EQ("(0-0)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"01a45cef9";
    EXPECT_TRUE(search(s, m, cregex(u"[ace1-9]+")));
    EXPECT_EQ("(1-7)", flatten(m));
  }
  {
    const char16_t r[] = u"^[-+]?[0-9]+[CF]$";
    std::ptrdiff_t sr = std::char_traits<char16_t>::length(r);
    using FI = const char16_t *;
    using BI = const char16_t *;
    cregex regex(llvh::ArrayRef<char16_t>(FI(r), FI(r + sr)));
    cmatch m;
    const char16_t s[] = u"-40C";
    std::ptrdiff_t ss = std::char_traits<char16_t>::length(s);
    EXPECT_TRUE(search(BI(s), BI(s + ss), m, regex));
    EXPECT_EQ("(0-4)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Jeff Jeffs u";
    EXPECT_TRUE(search(s, m, cregex(u"Jeff(?=s\\b)")));
    EXPECT_EQ("(5-9)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"Jeffs Jeff";
    EXPECT_TRUE(search(s, m, cregex(u"Jeff(?!s\\b)")));
    EXPECT_EQ("(6-10)", flatten(m));
  }
  {
    cmatch m;
    const char16_t s[] = u"5%k";
    EXPECT_TRUE(search(s, m, cregex(u"\\d[\\W]k")));
    EXPECT_EQ("(0-3)", flatten(m));
  }
}

static constants::ErrorType error_for(
    const char16_t *pattern,
    const char16_t *flags = u"") {
  return cregex(pattern, flags).getError();
};

TEST(Regex, Invalid) {
  using namespace constants;
  EXPECT_EQ(error_for(u""), ErrorType::None);
  EXPECT_EQ(error_for(u"abc\\"), ErrorType::EscapeIncomplete);
  EXPECT_EQ(error_for(u"abc\\5"), ErrorType::None);
  EXPECT_EQ(error_for(u"abc\\5def"), ErrorType::None);
  EXPECT_EQ(error_for(u"abc\\99999999999999999999999999def"), ErrorType::None);
  EXPECT_EQ(error_for(u"abc[def"), ErrorType::UnbalancedBracket);
  EXPECT_EQ(error_for(u"abc(def"), ErrorType::UnbalancedParenthesis);
  EXPECT_EQ(error_for(u"abc{3"), ErrorType::None);
  EXPECT_EQ(error_for(u"abc{3,e}"), ErrorType::None);
  EXPECT_EQ(error_for(u"{3,,5}"), ErrorType::None);
  EXPECT_EQ(error_for(u"{3,5}"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"abc{10,3}"), ErrorType::BraceRange);
  EXPECT_EQ(error_for(u"abc[b-a]"), ErrorType::CharacterRange);
  EXPECT_EQ(error_for(u"(*)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"(+)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"({1})"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"(?)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"abc", u"a"), ErrorType::InvalidFlags);
  EXPECT_EQ(error_for(u"abc", u"gg"), ErrorType::InvalidFlags);
  EXPECT_EQ(error_for(u"(?=a){1}", u"u"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"(?<=a){1}", u"u"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"(?=a){1}"), ErrorType::None);
  EXPECT_EQ(error_for(u"(?!a){1}"), ErrorType::None);
  EXPECT_EQ(error_for(u"(?<=a){1}"), ErrorType::InvalidRepeat);
}

TEST(Regex, InvalidFromLibCXX) {
  using namespace constants;
  const char16_t *pat1 = u"a(b)c\\1234";
  EXPECT_EQ(
      cregex(llvh::ArrayRef<char16_t>(pat1, pat1 + 7)).getError(),
      ErrorType::None);

  EXPECT_EQ(error_for(u"[\\a]"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\a"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\"), ErrorType::EscapeIncomplete);
  EXPECT_EQ(error_for(u"[\\e]"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\e"), ErrorType::None);
  EXPECT_EQ(error_for(u"[\\c:]"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\c:"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\c"), ErrorType::None);
  EXPECT_EQ(error_for(u"[\\cA]"), ErrorType::None);
  EXPECT_EQ(error_for(u"[\\CA]"), ErrorType::None);
  EXPECT_EQ(error_for(u"\\cA"), ErrorType::None);

  EXPECT_EQ(error_for(u"?a"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"*a"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"+a"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"{a"), ErrorType::None);

  EXPECT_EQ(error_for(u"?(a+)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"*(a+)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"+(a+)"), ErrorType::InvalidRepeat);
  EXPECT_EQ(error_for(u"{(a+)"), ErrorType::None);
}

TEST(RegExp, RangeChecks) {
  using namespace constants;
  EXPECT_EQ(error_for(u"[a-b]"), ErrorType::None);
  EXPECT_EQ(error_for(u"[b-a]"), ErrorType::CharacterRange);
  EXPECT_EQ(error_for(u"[\\uD800-\\uDFFF]"), ErrorType::None);
  EXPECT_EQ(error_for(u"[\\uDFFF-\\uD800]"), ErrorType::CharacterRange);
  // In the following we are required to interpret - as a ClassAtom and not
  // part of a range.
  EXPECT_EQ(error_for(u"[a\\s-b]"), ErrorType::None);
  EXPECT_EQ(error_for(u"[x-\\S]"), ErrorType::None);
  EXPECT_EQ(error_for(u"[\\d-\\S]"), ErrorType::None);
}

static bool regexIsAnchored(
    const char16_t *pattern,
    const char16_t *flags = u"") {
  return cregex(pattern, flags).matchConstraints() &
      MatchConstraintAnchoredAtStart;
}

static bool regexIsNonASCII(
    const char16_t *pattern,
    const char16_t *flags = u"") {
  return cregex(pattern, flags).matchConstraints() & MatchConstraintNonASCII;
}

TEST(Regex, LineStartAnchoring) {
  EXPECT_TRUE(regexIsAnchored(u"^abc"));
  EXPECT_TRUE(regexIsAnchored(u"abc^"));
  EXPECT_TRUE(regexIsAnchored(u"abc^"));
  EXPECT_TRUE(regexIsAnchored(u"abc^|^def"));
  EXPECT_TRUE(regexIsAnchored(u"abc^|^def"));
  EXPECT_TRUE(regexIsAnchored(u"^foo|^bar|^baz"));
  EXPECT_TRUE(regexIsAnchored(u"(^bar)"));
  EXPECT_TRUE(regexIsAnchored(u"(?=^bar)\\w+"));
  EXPECT_FALSE(regexIsAnchored(u"(?!^bar)\\w+"));
  EXPECT_FALSE(regexIsAnchored(u"(?=^bar)|\\w+"));
  EXPECT_FALSE(regexIsAnchored(u"abc"));
  EXPECT_FALSE(regexIsAnchored(u"abc|^def"));
  EXPECT_FALSE(regexIsAnchored(u"abc^|^def|\\w"));
  EXPECT_FALSE(regexIsAnchored(u"abc^", u"m"));
  EXPECT_FALSE(regexIsAnchored(u"abc", u"m"));
}

TEST(Regex, NonASCII) {
  EXPECT_TRUE(regexIsNonASCII(u"ab\xFF"));
  EXPECT_TRUE(regexIsNonASCII(u"ab(?:\xFF)"));
  EXPECT_TRUE(regexIsNonASCII(u"[\xFE]"));
  EXPECT_FALSE(regexIsNonASCII(u"[0\xFE]"));
  EXPECT_FALSE(regexIsNonASCII(u"[^\xFE]"));
  EXPECT_FALSE(regexIsNonASCII(u"[\\w\xFE]"));
  EXPECT_FALSE(regexIsNonASCII(u"[\x7F\xFE]"));
  EXPECT_FALSE(regexIsNonASCII(u"abFF"));
  EXPECT_FALSE(regexIsNonASCII(u"^abFF"));
  EXPECT_FALSE(regexIsNonASCII(u"ab(\xFF|c)"));

  // This regex will take a very long time unless we exploit the non-ASCII
  // optimization.

  const std::u16string ascii = u"This is a pretty long all-ASCII string";
  cmatch matchRanges;
  EXPECT_FALSE(search(
      ascii,
      matchRanges,
      cregex(u"(((.*)*)*)*\xFF"),
      constants::matchInputAllAscii));
  EXPECT_FALSE(search(
      ascii,
      matchRanges,
      cregex(u"(?!(((.*)*)*)*\xFF)abc"),
      constants::matchInputAllAscii));
}

} // end anonymous namespace
