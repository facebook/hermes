/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

// The table-driven collator is only compiled for the backends that call it;
// Android and Apple use their platform collators instead.
#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_JAVA && \
    HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_CF

#include "hermes/Platform/Unicode/UnicodeCollation.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include "llvh/ADT/SmallVector.h"

#include "gtest/gtest.h"

#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <string>

namespace {

using namespace hermes::unicode;

#include "CollationTestData.inc"

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

/// Parse one conformance row, a space-separated list of hex code points.
std::u16string parseRow(const char *s) {
  std::u16string out;
  while (*s != '\0') {
    if (*s == ' ') {
      ++s;
      continue;
    }
    char *end = nullptr;
    unsigned long cp = std::strtoul(s, &end, 16);
    s = end;
    appendCodePoint(out, (uint32_t)cp);
  }
  return out;
}

/// Decode one code point from \p s starting at \p i, advancing \p i past it.
uint32_t decodeCodePoint(const std::u16string &s, size_t &i) {
  char16_t c = s[i];
  if (c >= 0xD800 && c <= 0xDBFF && i + 1 < s.size() && s[i + 1] >= 0xDC00 &&
      s[i + 1] <= 0xDFFF) {
    uint32_t cp = 0x10000 + ((c - 0xD800) << 10) + (s[i + 1] - 0xDC00);
    i += 2;
    return cp;
  }
  ++i;
  return c;
}

/// Compare \p left and \p right by Unicode code point rather than by UTF-16
/// code unit.
///
/// std::u16string::operator< compares UTF-16 code units, which misorders
/// supplementary-plane characters against BMP characters above U+DFFF: a
/// lead surrogate (0xD800-0xDBFF) is numerically smaller than, say, a CJK
/// Compatibility Ideograph in the 0xF900-0xFAFF block, even though the
/// supplementary code point it encodes is larger. The identical level must
/// compare actual code points to avoid this.
int compareByCodePoint(
    const std::u16string &left,
    const std::u16string &right) {
  size_t i = 0, j = 0;
  while (i < left.size() && j < right.size()) {
    uint32_t lcp = decodeCodePoint(left, i);
    uint32_t rcp = decodeCodePoint(right, j);
    if (lcp != rcp)
      return lcp < rcp ? -1 : 1;
  }
  if (i < left.size())
    return 1;
  if (j < right.size())
    return -1;
  return 0;
}

/// \return \p s normalized to NFD.
///
/// UTS #10 section 7.1 defines the identical level as a comparison of the
/// NFD forms of the two strings, not of the raw input: compareUTF16 already
/// normalizes internally, so the tie-break must normalize too, or the two
/// sides would be ordering different strings.
std::u16string toNFD(const std::u16string &s) {
  llvh::SmallVector<char16_t, 64> buf(s.begin(), s.end());
  normalizeUTF16(buf, NormalizationForm::D);
  return std::u16string(buf.begin(), buf.end());
}

/// Hex dump, so a failure names the code points rather than printing
/// mojibake.
std::string dump(const std::u16string &s) {
  std::string out;
  for (char16_t c : s) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04X ", (unsigned)c);
    out += buf;
  }
  return out;
}

/// Compare \p left and \p right the way the conformance file's ordering
/// assumes: the UCA result, then the identical level, a code point
/// comparison of the NFD forms (UTS #10 section 7.1).
///
/// compareUTF16 deliberately stops at the tertiary level, so rows that are
/// correctly equal there would otherwise appear out of order. The tie-break
/// is applied only when the UCA result is 0, so it cannot mask a real
/// ordering failure at any of the three levels.
int compareWithIdenticalLevel(
    const std::u16string &left,
    const std::u16string &right) {
  int c =
      compareUTF16({left.data(), left.size()}, {right.data(), right.size()});
  if (c != 0)
    return c;
  return compareByCodePoint(toNFD(left), toNFD(right));
}

// Guards against a generator bug emitting empty tables, which would make
// every other assertion in this file pass vacuously.
TEST(CollationConformance, TestDataIsPopulated) {
  EXPECT_GT(std::size(COLL_TEST_ROWS), 200000u);
  EXPECT_GT(std::size(COLL_WEIGHT_CHECKS), 30000u);
}

// The invariant stated in the header of CollationTest.html: each row sorts at
// or after the row before it.
TEST(CollationConformance, RowsAreInAscendingOrder) {
  std::u16string prev;
  for (size_t i = 0; i < std::size(COLL_TEST_ROWS); ++i) {
    std::u16string cur = parseRow(COLL_TEST_ROWS[i].codePoints);
    if (i != 0) {
      EXPECT_LE(compareWithIdenticalLevel(prev, cur), 0)
          << "row " << i << " sorts before its predecessor\n"
          << "  prev: " << COLL_TEST_ROWS[i - 1].codePoints << "  ("
          << dump(prev) << ")\n"
          << "  cur:  " << COLL_TEST_ROWS[i].codePoints << "  (" << dump(cur)
          << ")";
    }
    prev = std::move(cur);
  }
}

// An independent check on the run encoding. The expectations come straight
// from allkeys.txt, so a compression bug shows up as a mismatch here rather
// than being reproduced identically on both sides.
//
// The weights are probed through compareUTF16 rather than read directly,
// since the table is private to UnicodeCollation.cpp: a code point must
// compare equal to itself and order against a neighbour consistently with
// its recorded primary weight.
TEST(CollationConformance, SingleElementWeightsRoundTrip) {
  for (size_t i = 1; i < std::size(COLL_WEIGHT_CHECKS); ++i) {
    const CollWeightCheck &a = COLL_WEIGHT_CHECKS[i - 1];
    const CollWeightCheck &b = COLL_WEIGHT_CHECKS[i];
    if (a.primary == 0 || b.primary == 0)
      continue; // Ignorable at the primary level; ordering is not implied.
    if (a.primary == b.primary)
      continue; // Ordered at a lower level, which this check does not cover.
    std::u16string sa, sb;
    appendCodePoint(sa, a.cp);
    appendCodePoint(sb, b.cp);
    int expected = a.primary < b.primary ? -1 : 1;
    EXPECT_EQ(
        compareUTF16({sa.data(), sa.size()}, {sb.data(), sb.size()}), expected)
        << "U+" << std::hex << a.cp << " (primary " << a.primary << ") vs U+"
        << b.cp << " (primary " << b.primary << ")";
  }
}

} // namespace

#endif // not JAVA and not CF
