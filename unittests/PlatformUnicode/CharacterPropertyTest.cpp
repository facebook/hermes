/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"
#include "hermes/Platform/Unicode/PlatformUnicode.h"
#include "hermes/Regex/RegexTraits.h"

#include "gtest/gtest.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace hermes;

namespace {

// This might look like a silly test, but it should be useful if we
// need to add tricky optimizations to isUnicodeOnlySpace
TEST(UnicodeTest, isUnicodeOnlySpace) {
  for (uint32_t i = 1; i < 128; i++) {
    EXPECT_FALSE(hermes::isUnicodeOnlySpace(i))
        << "No ASCII character is a unicode-only space. (code: " << i << ")";
  }

  std::vector<uint32_t> unicode_space_cps{0x1680,
                                          0x2000,
                                          0x2001,
                                          0x2002,
                                          0x2003,
                                          0x2004,
                                          0x2005,
                                          0x2006,
                                          0x2007,
                                          0x2008,
                                          0x2009,
                                          0x200A,
                                          0x202F,
                                          0x205F,
                                          0x3000};

  for (auto cp : unicode_space_cps) {
    EXPECT_TRUE(hermes::isUnicodeOnlySpace(cp))
        << "Code point " << cp << " is a unicode-only space.";
  }

  // Test for off-by-one errors.
  std::vector<uint32_t> unicode_non_space_cps{0x167f,
                                              0x1681,
                                              0x1fff,
                                              0x200B,
                                              0x202E,
                                              0x2030,
                                              0x205E,
                                              0x2060,
                                              0x2fff,
                                              0x3001};

  for (auto cp : unicode_non_space_cps) {
    EXPECT_FALSE(hermes::isUnicodeOnlySpace(cp))
        << "Code point " << cp << " is not a unicode-only space.";
  }
}

using CharSet = std::unordered_set<char16_t>;

static CharSet getPrecanonicalizations(char16_t c) {
  CharSet result = {c};
  // Check for exceptional cases.
  if (auto list = getExceptionalPrecanonicalizations(uint16_t(c))) {
    // Build from the list. Note if the version of ICU on the system is earlier
    // than the one used to build the table, some characters in the table will
    // not be valid pre-canonicalizations. Thus we have to check. Also 0 is a
    // placeholder meaning none, so skip any 0s.
    for (char16_t pc : *list) {
      if (pc && regex::canonicalizeToCase(pc, regex::CaseUpper) == c) {
        result.insert(pc);
      }
    }
  } else {
    // Not exceptional; simply lowercase.
    result.insert(regex::canonicalizeToCase(c, regex::CaseLower));
  }
  return result;
}

// Verify correct precanonicalization mappings. We do this by producing a
// reverse canonicalization map the hard way, by canonicalizing every
// character. We then compare it against what our getPrecanonicalizations()
// function returns.
TEST(UnicodeTest, PrecanonicalizationMapping) {
  std::unordered_map<char16_t, CharSet> precanonMap;
  for (uint32_t cp = 0; cp <= std::numeric_limits<char16_t>::max(); cp++) {
    char16_t canon = regex::canonicalizeToCase(char16_t(cp), regex::CaseUpper);
    precanonMap[canon].insert(cp);
  }

  for (const auto &kv : precanonMap) {
    const CharSet &computed = kv.second;
    CharSet lookedUp = getPrecanonicalizations(kv.first);
    EXPECT_EQ(computed, lookedUp)
        << "mismatch for character " << uint32_t(kv.first);
  }
}

TEST(UnicodeTest, ASCIICanonicalization) {
  regex::ASCIIRegexTraits traits;
  for (int c = 0; c <= 127; c++) {
    EXPECT_EQ(toupper(c), traits.caseFold((char)c));
  }
}

static std::string flatten(const CodePointSet &cps) {
  std::string result;
  for (const auto &range : cps.ranges()) {
    EXPECT_GT(range.length, 0u);
    if (!result.empty())
      result += ", ";
    result += std::to_string(range.first);
    result += '-';
    result += std::to_string(range.end() - 1);
  }
  return result;
}

TEST(UnicodeTest, CodePointSet) {
  EXPECT_EQ(flatten(CodePointSet{}), flatten(CodePointSet{}));

  CodePointSet cps;
  cps.add(10);
  EXPECT_EQ("10-10", flatten(cps));
  cps.add(5);
  EXPECT_EQ("5-5, 10-10", flatten(cps));

  cps.add(CodePointRange{2, 2});
  EXPECT_EQ("2-3, 5-5, 10-10", flatten(cps));

  cps.add(CodePointRange{3, 4});
  EXPECT_EQ("2-6, 10-10", flatten(cps));

  cps.add(CodePointRange{100, 1});
  EXPECT_EQ("2-6, 10-10, 100-100", flatten(cps));

  cps.add(CodePointRange{101, 1});
  EXPECT_EQ("2-6, 10-10, 100-101", flatten(cps));

  cps.add(CodePointRange{0, 1001});
  EXPECT_EQ("0-1000", flatten(cps));
}

TEST(UnicodeTest, AddCanonicalEquivalentCharacters) {
  auto flattenCanonically = [](const CodePointSet &cps) {
    return flatten(makeCanonicallyEquivalent(cps));
  };

  CodePointSet cps;
  cps.add('&');
  EXPECT_EQ("38-38", flattenCanonically(cps));
  cps.add('A');
  EXPECT_EQ("38-38, 65-65, 97-97", flattenCanonically(cps));
  cps.add(CodePointRange{'A', 'Z' - 'A' + 1});
  EXPECT_EQ("38-38, 65-90, 97-122", flattenCanonically(cps));
  cps.add(0x01DF); // 479
  cps.add(0x01E3); // 483
  cps.add(0x01E6); // 486
  EXPECT_EQ(
      "38-38, 65-90, 97-122, 478-479, 482-483, 486-487",
      flattenCanonically(cps));
}

} // end anonymous namespace
