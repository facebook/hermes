/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

// The table-driven normalizer is only compiled for the backends that call it;
// Android and Apple use their platform normalizers instead. See
// lib/Platform/Unicode/CMakeLists.txt and the matching select() in BUCK.
#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_JAVA && \
    HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_CF

#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <string>

namespace {

using namespace hermes::unicode;

#include "NormalizationTestData.inc"

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

/// Parse one row of NormalizationTest.txt, given as "c1;c2;c3;c4;c5" where
/// each column is a space-separated list of hex code points.
std::array<std::u16string, 5> parseRow(const char *s) {
  std::array<std::u16string, 5> cols;
  size_t col = 0;
  while (*s != '\0' && col < cols.size()) {
    if (*s == ';') {
      ++col;
      ++s;
    } else if (*s == ' ') {
      ++s;
    } else {
      char *end = nullptr;
      unsigned long cp = std::strtoul(s, &end, 16);
      // strtoul leaves end == s on a character that is not a hex digit, so
      // without this the loop would spin forever instead of failing.
      assert(end != s && "malformed row: expected a hex code point");
      s = end;
      appendCodePoint(cols[col], (uint32_t)cp);
    }
  }
  return cols;
}

/// Hex dump, so a failure names the code points rather than printing mojibake.
std::string dump(const std::u16string &s) {
  std::string out;
  for (char16_t c : s) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04X ", (unsigned)c);
    out += buf;
  }
  return out;
}

std::u16string normalized(const std::u16string &in, NormalizationForm form) {
  llvh::SmallVector<char16_t, 32> buf(in.begin(), in.end());
  normalizeUTF16(buf, form);
  return std::u16string(buf.begin(), buf.end());
}

const char *formName(NormalizationForm form) {
  switch (form) {
    case NormalizationForm::C:
      return "NFC";
    case NormalizationForm::D:
      return "NFD";
    case NormalizationForm::KC:
      return "NFKC";
    case NormalizationForm::KD:
      return "NFKD";
  }
  return "?";
}

void expectNormalizes(
    const std::u16string &input,
    const std::u16string &expected,
    NormalizationForm form,
    const NormTestRow &row,
    size_t index) {
  std::u16string actual = normalized(input, form);
  EXPECT_EQ(expected, actual)
      << formName(form) << " mismatch on row " << index << " (part "
      << (unsigned)row.part << ")\n  row:      " << row.columns
      << "\n  input:    " << dump(input) << "\n  expected: " << dump(expected)
      << "\n  actual:   " << dump(actual);
}

// Guards against a generator bug emitting empty tables, which would make
// every other assertion in this file pass vacuously.
TEST(NormalizationConformance, TestDataIsPopulated) {
  EXPECT_GT(std::size(NORM_TEST_ROWS), 19000u);
  EXPECT_GT(std::size(NORM_TEST_PART1), 15000u);
}

// The invariants quoted from the header of NormalizationTest.txt:
//   NFC:  c2 == toNFC(c1)  == toNFC(c2)  == toNFC(c3)
//         c4 == toNFC(c4)  == toNFC(c5)
//   NFD:  c3 == toNFD(c1)  == toNFD(c2)  == toNFD(c3)
//         c5 == toNFD(c4)  == toNFD(c5)
//   NFKC: c4 == toNFKC(c1) == toNFKC(c2) == toNFKC(c3)
//                          == toNFKC(c4) == toNFKC(c5)
//   NFKD: c5 == toNFKD(c1) == toNFKD(c2) == toNFKD(c3)
//                          == toNFKD(c4) == toNFKD(c5)
TEST(NormalizationConformance, AllRows) {
  for (size_t i = 0; i < std::size(NORM_TEST_ROWS); ++i) {
    const NormTestRow &row = NORM_TEST_ROWS[i];
    auto c = parseRow(row.columns);
    for (size_t j : {0u, 1u, 2u}) {
      expectNormalizes(c[j], c[1], NormalizationForm::C, row, i);
      expectNormalizes(c[j], c[2], NormalizationForm::D, row, i);
    }
    for (size_t j : {3u, 4u}) {
      expectNormalizes(c[j], c[3], NormalizationForm::C, row, i);
      expectNormalizes(c[j], c[4], NormalizationForm::D, row, i);
    }
    for (size_t j : {0u, 1u, 2u, 3u, 4u}) {
      expectNormalizes(c[j], c[3], NormalizationForm::KC, row, i);
      expectNormalizes(c[j], c[4], NormalizationForm::KD, row, i);
    }
  }
}

// Part 1 invariant: a code point not listed in Part 1 normalizes to itself
// under every form. Surrogates are excluded here because they are not code
// points; UnpairedSurrogates* covers them instead.
TEST(NormalizationConformance, UnlistedCodePointsAreInvariant) {
  for (uint32_t cp = 1; cp <= 0x10FFFF; ++cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
      continue;
    if (std::binary_search(
            std::begin(NORM_TEST_PART1), std::end(NORM_TEST_PART1), cp))
      continue;
    std::u16string orig;
    appendCodePoint(orig, cp);
    for (auto form :
         {NormalizationForm::C,
          NormalizationForm::D,
          NormalizationForm::KC,
          NormalizationForm::KD}) {
      ASSERT_EQ(orig, normalized(orig, form))
          << "U+" << std::hex << cp << " should normalize to itself under "
          << formName(form);
    }
  }
}

// A string of pass-through characters takes the Quick_Check fast path, so this
// asserts the fast path preserves unpaired surrogates.
TEST(NormalizationHazards, UnpairedSurrogatesSurviveFastPath) {
  // A lone high surrogate, a lone low surrogate, and a reversed pair.
  llvh::SmallVector<char16_t, 8> buf = {
      u'a', 0xD800, u'b', 0xDC00, u'c', 0xDC00, 0xD800};
  auto orig = buf;
  normalizeUTF16(buf, NormalizationForm::C);
  EXPECT_EQ(
      std::u16string(orig.begin(), orig.end()),
      std::u16string(buf.begin(), buf.end()));
}

// U+00C0 has NFD_QC=No, which forces the full decompose-and-reorder path. This
// is the case that actually exercises the UTF-16 decode and re-encode.
TEST(NormalizationHazards, UnpairedSurrogatesSurviveFullPath) {
  llvh::SmallVector<char16_t, 8> buf = {
      u'\u00C0', 0xD800, u'b', 0xDC00, 0xDC00, 0xD800};
  normalizeUTF16(buf, NormalizationForm::D);
  ASSERT_EQ(7u, buf.size());
  EXPECT_EQ(u'A', buf[0]);
  EXPECT_EQ(u'\u0300', buf[1]);
  EXPECT_EQ(0xD800, buf[2]);
  EXPECT_EQ(u'b', buf[3]);
  EXPECT_EQ(0xDC00, buf[4]);
  EXPECT_EQ(0xDC00, buf[5]);
  EXPECT_EQ(0xD800, buf[6]);
}

TEST(NormalizationHazards, EmbeddedNulIsNotATerminator) {
  // The U+00C0 after the NUL must still decompose.
  llvh::SmallVector<char16_t, 8> buf = {u'a', u'\0', u'\u00C0'};
  normalizeUTF16(buf, NormalizationForm::D);
  ASSERT_EQ(4u, buf.size());
  EXPECT_EQ(u'a', buf[0]);
  EXPECT_EQ(u'\0', buf[1]);
  EXPECT_EQ(u'A', buf[2]);
  EXPECT_EQ(u'\u0300', buf[3]);
}

TEST(NormalizationHazards, EmptyString) {
  llvh::SmallVector<char16_t, 8> buf;
  normalizeUTF16(buf, NormalizationForm::C);
  EXPECT_TRUE(buf.empty());
}

TEST(NormalizationHazards, SupplementaryPlaneDecomposition) {
  // U+1D15E MUSICAL SYMBOL HALF NOTE decomposes to U+1D157 U+1D165. This is
  // one of the few mappings that lives in the char32_t pool.
  llvh::SmallVector<char16_t, 8> buf = {0xD834, 0xDD5E};
  normalizeUTF16(buf, NormalizationForm::D);
  ASSERT_EQ(4u, buf.size());
  EXPECT_EQ(0xD834, buf[0]);
  EXPECT_EQ(0xDD57, buf[1]);
  EXPECT_EQ(0xD834, buf[2]);
  EXPECT_EQ(0xDD65, buf[3]);
}

// U+1E9B U+0323 is the standard growth-and-recomposition example: NFD expands
// it to three code points and NFC brings it back to the original two.
TEST(NormalizationHazards, GrowsUnderNFDShrinksUnderNFC) {
  llvh::SmallVector<char16_t, 8> buf = {u'\u1E9B', u'\u0323'};
  normalizeUTF16(buf, NormalizationForm::D);
  ASSERT_EQ(3u, buf.size());
  EXPECT_EQ(u'\u017F', buf[0]);
  EXPECT_EQ(u'\u0323', buf[1]);
  EXPECT_EQ(u'\u0307', buf[2]);
  normalizeUTF16(buf, NormalizationForm::C);
  ASSERT_EQ(2u, buf.size());
  EXPECT_EQ(u'\u1E9B', buf[0]);
  EXPECT_EQ(u'\u0323', buf[1]);
}

// Hangul is handled arithmetically rather than by table lookup, so it needs
// coverage in both directions.
TEST(NormalizationHazards, HangulRoundTrips) {
  llvh::SmallVector<char16_t, 8> buf = {u'\uD4DB'};
  normalizeUTF16(buf, NormalizationForm::D);
  ASSERT_EQ(3u, buf.size());
  EXPECT_EQ(u'\u1111', buf[0]);
  EXPECT_EQ(u'\u1171', buf[1]);
  EXPECT_EQ(u'\u11B6', buf[2]);
  normalizeUTF16(buf, NormalizationForm::C);
  ASSERT_EQ(1u, buf.size());
  EXPECT_EQ(u'\uD4DB', buf[0]);
}

// NFKD must subsume NFD: the compatibility tables omit any mapping equal to
// the canonical one, so this exercises the fallback in appendDecomposition.
TEST(NormalizationHazards, CompatibilityFallsBackToCanonical) {
  // U+00C0 has a canonical mapping and no distinct compatibility mapping.
  llvh::SmallVector<char16_t, 8> buf = {u'\u00C0'};
  normalizeUTF16(buf, NormalizationForm::KD);
  ASSERT_EQ(2u, buf.size());
  EXPECT_EQ(u'A', buf[0]);
  EXPECT_EQ(u'\u0300', buf[1]);

  // U+FB01 LATIN SMALL LIGATURE FI has only a compatibility mapping, to "fi".
  llvh::SmallVector<char16_t, 8> lig = {u'\uFB01'};
  normalizeUTF16(lig, NormalizationForm::KD);
  ASSERT_EQ(2u, lig.size());
  EXPECT_EQ(u'f', lig[0]);
  EXPECT_EQ(u'i', lig[1]);

  // ...and NFD must leave it alone, since that mapping is not canonical.
  llvh::SmallVector<char16_t, 8> ligD = {u'\uFB01'};
  normalizeUTF16(ligD, NormalizationForm::D);
  ASSERT_EQ(1u, ligD.size());
  EXPECT_EQ(u'\uFB01', ligD[0]);
}

} // namespace

#endif // not JAVA and not CF
