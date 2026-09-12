/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

// The table-driven collator is only compiled for the backends that call it;
// Android and Apple use their platform collators instead. See
// lib/Platform/Unicode/CMakeLists.txt and the matching select() in BUCK.
#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_JAVA && \
    HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_CF

#include "hermes/Platform/Unicode/UnicodeCollation.h"

#include "gtest/gtest.h"

#include <ios>
#include <string>
#include <utility>

namespace {

using namespace hermes::unicode;

/// Compare two UTF-16 buffers, so the tests read as comparisons rather than
/// as ArrayRef construction.
int cmp(
    const char16_t *left,
    size_t leftLen,
    const char16_t *right,
    size_t rightLen) {
  return compareUTF16({left, leftLen}, {right, rightLen});
}

/// Compare two UTF-16 string literals, deducing each length from the array
/// bound and dropping the terminating NUL. Tests that need an embedded NUL,
/// or a code point that renders ambiguously, use the four-argument form with
/// an explicit array instead.
template <size_t N, size_t M>
int cmp(const char16_t (&left)[N], const char16_t (&right)[M]) {
  return cmp(left, N - 1, right, M - 1);
}

TEST(Collation, OrdersByPrimaryWeight) {
  // 'a' has primary 9196, 'b' 9222, 'Z' 9966, so both order by letter rather
  // than by code unit. The 'a' vs 'Z' case is the one string-functions.js
  // already asserts against ICU.
  EXPECT_LT(cmp(u"a", u"b"), 0);
  EXPECT_LT(cmp(u"a", u"Z"), 0);
  EXPECT_GT(cmp(u"b", u"a"), 0);
  EXPECT_EQ(cmp(u"abc", u"abc"), 0);
  EXPECT_LT(cmp(u"abc", u"abd"), 0);
  // A prefix sorts before the longer string.
  EXPECT_LT(cmp(u"ab", u"abc"), 0);
}

TEST(Collation, OrdersCaseAtTertiaryLevel) {
  // 'a' and 'A' share primary 9196 and secondary 32; they differ only at the
  // tertiary level, 2 against 8. Lowercase sorts first.
  EXPECT_LT(cmp(u"a", u"A"), 0);
  EXPECT_GT(cmp(u"A", u"a"), 0);
}

TEST(Collation, OrdersAccentsAtSecondaryLevel) {
  // Combining acute (U+0301) has primary 0, so it is invisible at level 1;
  // the unaccented form wins at level 2 by being shorter.
  const char16_t a[] = {0x0061};
  const char16_t aAcute[] = {0x0061, 0x0301};
  EXPECT_LT(cmp(a, 1, aAcute, 2), 0);
  EXPECT_GT(cmp(aAcute, 2, a, 1), 0);
}

TEST(Collation, CanonicallyEquivalentStringsAreEqual) {
  // Precomposed and decomposed forms normalize to the same NFD sequence.
  // Written as explicit code point arrays: the two spellings of each pair
  // render identically, so a literal would be unreadable and easy to
  // corrupt.
  const char16_t oDiaeresis[] = {0x00F6}; // LATIN SMALL LETTER O WITH
                                          // DIAERESIS
  const char16_t oCombining[] = {0x006F, 0x0308};
  EXPECT_EQ(cmp(oDiaeresis, 1, oCombining, 2), 0);

  const char16_t angstrom[] = {0x212B}; // ANGSTROM SIGN
  const char16_t aRing[] = {0x0041, 0x030A};
  EXPECT_EQ(cmp(angstrom, 1, aRing, 2), 0);

  // Diacritic reordering is canonically equivalent: canonical ordering sorts
  // U+0323 (ccc 220) before U+0307 (ccc 230) regardless of input order.
  const char16_t sBelowAbove[] = {0x0053, 0x0323, 0x0307};
  const char16_t sAboveBelow[] = {0x0053, 0x0307, 0x0323};
  EXPECT_EQ(cmp(sBelowAbove, 3, sAboveBelow, 3), 0);
}

TEST(Collation, CompletelyIgnorableCharactersDoNotAffectOrder) {
  // U+0000 has all-zero weights, so it contributes at no level.
  const char16_t withNul[] = {0x0061, 0x0000, 0x0062};
  EXPECT_EQ(cmp(withNul, 3, u"ab", 2), 0);
}

TEST(Collation, HandlesExpansions) {
  // U+00E6 expands to three elements with primaries 9196, 0 and 9299, the
  // same primaries as "ae", so the two differ only through the middle
  // element's secondary weight of 287.
  const char16_t ae[] = {0x00E6}; // LATIN SMALL LETTER AE
  EXPECT_GT(cmp(ae, 1, u"ae", 2), 0);
  EXPECT_LT(cmp(ae, 1, u"af", 2), 0);
  EXPECT_LT(cmp(u"a", 1, ae, 1), 0);
}

TEST(Collation, AppliesContractions) {
  // U+0419 normalizes to U+0418 U+0306, which contracts to a single element
  // with primary 10337 rather than being treated as U+0418 (primary 10324)
  // followed by a breve. U+0406 sits between the two at primary 10332, so
  // the sign of this comparison flips if the contraction is not applied.
  const char16_t shortI[] = {0x0419}; // CYRILLIC SHORT I
  const char16_t i[] = {0x0418}; // CYRILLIC I
  const char16_t iUkr[] = {0x0406}; // CYRILLIC BYELORUSSIAN-
                                    // UKRAINIAN I
  EXPECT_GT(cmp(shortI, 1, iUkr, 1), 0);
  EXPECT_LT(cmp(i, 1, iUkr, 1), 0);
  // Spelling the contraction out explicitly must give the same answer.
  const char16_t decomposed[] = {0x0418, 0x0306};
  EXPECT_EQ(cmp(shortI, 1, decomposed, 2), 0);
}

TEST(Collation, AppliesDiscontiguousContractions) {
  // U+0418 U+0323 U+0306: the dot below has ccc 220 and the breve ccc 230,
  // so the breve is not blocked from U+0418 and the contraction U+0418
  // U+0306 applies across it, giving primary 10337. Without the rule the
  // sequence weighs as U+0418 alone at primary 10324. U+0406 sits between
  // the two at 10332, so the sign of this comparison flips.
  const char16_t seq[] = {0x0418, 0x0323, 0x0306};
  const char16_t i406[] = {0x0406};
  EXPECT_GT(cmp(seq, 3, i406, 1), 0);

  // An absorbed character must not also be counted in its original
  // position. U+0F71 U+0F71 U+0F72 (Tibetan vowel signs) exercises that
  // directly, unlike the pair above: 0F71 and 0F72 do not decompose and
  // their ccc values (129, 130) are in canonical order, so NFD is a no-op and
  // the two sides below do not share an NFD form. ccc(0F71) = 129,
  // ccc(0F72) = 130, so the first 0F71 is unblocked from the second and the
  // contraction U+0F71 U+0F72 (primary 14415) applies discontiguously,
  // absorbing the trailing 0F72. If that absorbed 0F72 were still visible
  // to the next call, it would be matched again against the leftover 0F71,
  // double-counting the contraction (primary 14415 a second time) instead
  // of weighing the leftover 0F71 on its own (primary 14413). 14413 is less
  // than 0F72's own primary of 14414, so the two sides differ.
  const char16_t tibetanAbsorbed[] = {0x0F71, 0x0F71, 0x0F72};
  const char16_t tibetanNotAbsorbed[] = {0x0F71, 0x0F72, 0x0F72};
  EXPECT_LT(cmp(tibetanAbsorbed, 3, tibetanNotAbsorbed, 3), 0);
}

TEST(Collation, RespectsBlockingInDiscontiguousMatching) {
  // U+0418 U+0301 U+0306: the acute and the breve both have ccc 230, so the
  // acute blocks the breve and the contraction must NOT apply. The sequence
  // therefore weighs as U+0418 at primary 10324, below U+0406 at 10332.
  const char16_t blocked[] = {0x0418, 0x0301, 0x0306};
  const char16_t i406[] = {0x0406};
  EXPECT_LT(cmp(blocked, 3, i406, 1), 0);
}

TEST(Collation, LongCombiningRunIsNotQuadratic) {
  // The S2.1.1 discontiguous scan used to run at every position, walking to
  // the end of the surrounding combining run each time, so a run of N
  // non-starters cost Theta(N^2) with a combining-class binary search in the
  // inner loop. U+0301 begins no contraction, so the early exit in
  // matchContraction reduces this to one binary search per position.
  //
  // What is asserted is ordinary correctness -- the longer run wins at the
  // secondary level, since U+0301 is ignorable at the primary level. The
  // point of the test is that it finishes: at this length the scan without
  // the early exit took 44 seconds, against 13 milliseconds with it.
  const size_t kLength = 40000;
  std::u16string longRun(kLength, u'\u0301');
  std::u16string shortRun(kLength - 1, u'\u0301');
  EXPECT_GT(
      cmp(longRun.data(), longRun.size(), shortRun.data(), shortRun.size()), 0);
  EXPECT_EQ(
      cmp(longRun.data(), longRun.size(), longRun.data(), longRun.size()), 0);
}

TEST(Collation, EmbeddedNulAfterContractionDoesNotSkipFollowingCharacter) {
  // matchContraction probes a three-code-point key (cp0, cp1, cp2), and the
  // table stores two-code-point contractions with cp2 == 0. An embedded
  // U+0000 right after a two-code-point contraction therefore makes that
  // probe degenerate into the two-code-point key, so the matched row's own
  // key length -- not the probe length -- must determine how many input
  // code points were consumed. U+0000 carries all-zero weights at every
  // level (see CompletelyIgnorableCharactersDoNotAffectOrder), so getting
  // this wrong cannot be observed as a sign flip; this instead guards
  // against the code point following the embedded U+0000 being silently
  // dropped.
  const char16_t seq[] = {0x0418, 0x0306, 0x0000, 0x0418};
  const char16_t expected[] = {0x0419, 0x0418};
  EXPECT_EQ(cmp(seq, 4, expected, 2), 0);
}

TEST(Collation, UsesImplicitWeightsForUntabledCodePoints) {
  // Han is not listed in allkeys.txt; its weights come from the formula in
  // UTS #10 section 10.1.3, which orders by code point within a base.
  const char16_t han1[] = {0x4E00};
  const char16_t han2[] = {0x4E01};
  EXPECT_LT(cmp(han1, 1, han2, 1), 0);
  // A Unified_Ideograph uses base 0xFB40; an unassigned code point uses
  // 0xFBC0, so every Han character sorts before every unassigned one.
  const char16_t unassigned[] = {0x0378};
  EXPECT_LT(cmp(han1, 1, unassigned, 1), 0);
}

TEST(Collation, ImplicitWeightsFollowUTS10) {
  // The implicit weights of UTS #10 section 10.1.3 cannot be checked through
  // compareUTF16: DUCET assigns no reachable code point an explicit primary
  // anywhere in 0xFAFA..0xFB3F, so any assignment of the @implicitweights
  // bases that keeps them in that gap and stays monotonic orders identically
  // and passes even the 208,070-row UCA conformance suite. The weights are
  // therefore asserted directly.
  //
  // An @implicitweights range takes its base verbatim as the first primary
  // and measures the second from the base's *anchor*, the start of the first
  // range using that base. Bases 0xFB00 and 0xFB01 are each shared by two
  // ranges, so the second range of each continues its predecessor's offsets
  // instead of restarting at 0x8000.
  //
  // Every expectation below is the sort key the upstream
  // CollationTest_NON_IGNORABLE.txt records for that code point.
  struct Case {
    uint32_t cp;
    uint16_t first;
    uint16_t second;
    const char *why;
  };
  static const Case kCases[] = {
      // 17000..187FF; FB00 -- the first range on base 0xFB00, so it is also
      // that base's anchor.
      {0x17000, 0xFB00, 0x8000, "first range on FB00, at its anchor"},
      {0x187FF, 0xFB00, 0x97FF, "first range on FB00, at its end"},
      // 18D00..18D7F; FB00 -- a *second* range on base 0xFB00. Its offsets
      // are measured from U+17000, not from U+18D00; a naive implementation
      // restarts them at 0x8000 and gets this one wrong.
      {0x18D00, 0xFB00, 0x9D00, "second range on FB00, anchored at 17000"},
      // 18800..18AFF; FB01 -- the anchor of base 0xFB01.
      {0x18800, 0xFB01, 0x8000, "first range on FB01, at its anchor"},
      // 18D80..18DFF; FB01 -- the second range on base 0xFB01.
      {0x18D80, 0xFB01, 0x8580, "second range on FB01, anchored at 18800"},
      // Bases used by exactly one range each.
      {0x1B170, 0xFB02, 0x8000, "only range on FB02"},
      {0x18B00, 0xFB03, 0x8000, "only range on FB03"},
      // The Han and unassigned paths are unaffected by the anchor rule and
      // do use the base + (cp >> 15) split, so they are pinned here too.
      {0x4E00, 0xFB40, 0xCE00, "Unified_Ideograph in a core block"},
      {0x20000, 0xFB84, 0x8000, "Unified_Ideograph outside the core blocks"},
      {0x0378, 0xFBC0, 0x8378, "unassigned"},
  };
  for (const Case &c : kCases) {
    std::pair<uint16_t, uint16_t> got = implicitPrimariesForTesting(c.cp);
    EXPECT_EQ(got.first, c.first) << "U+" << std::hex << c.cp << ": " << c.why;
    EXPECT_EQ(got.second, c.second)
        << "U+" << std::hex << c.cp << ": " << c.why;
  }
}

TEST(Collation, HandlesSupplementaryPlaneAndUnpairedSurrogates) {
  // A surrogate pair decodes to one code point rather than two code units.
  EXPECT_LT(cmp(u"\U00020000", u"\U00020001"), 0);
  // Unpaired surrogates get implicit weights and must not crash or be
  // replaced. Written as numeric array initializers because C++ forbids
  // naming surrogates with \u.
  const char16_t lone[] = {0xD800};
  const char16_t lone2[] = {0xD801};
  EXPECT_LT(cmp(lone, 1, lone2, 1), 0);
  EXPECT_EQ(cmp(lone, 1, lone, 1), 0);
}

TEST(Collation, IsSymmetricAndReflexive) {
  // ECMA-262 requires a consistent comparison function.
  // Samples as code point arrays with explicit lengths, so the non-ASCII
  // entries cannot be corrupted by an editor or a copy-paste.
  struct Sample {
    const char16_t *p;
    size_t n;
  };
  static const char16_t kA[] = {0x0061}, kUpperA[] = {0x0041}, kAe[] = {0x00E6},
                        kShortI[] = {0x0419}, kHan[] = {0x4E00},
                        kZz[] = {0x007A, 0x007A}, kAAcute[] = {0x0061, 0x0301};
  const Sample samples[] = {
      {kA, 1},
      {kUpperA, 1},
      {kAe, 1},
      {kShortI, 1},
      {kHan, 1},
      {nullptr, 0},
      {kZz, 2},
      {kAAcute, 2}};
  for (const Sample &x : samples) {
    EXPECT_EQ(cmp(x.p, x.n, x.p, x.n), 0) << "not reflexive";
    for (const Sample &y : samples) {
      EXPECT_EQ(cmp(x.p, x.n, y.p, y.n), -cmp(y.p, y.n, x.p, x.n))
          << "not antisymmetric";
    }
  }
}

TEST(Collation, AsciiFastPathMatchesGeneralPath) {
  // Every pair below is pure ASCII and so takes the fast path. The expected
  // values are the ones the general path produces, asserted explicitly so
  // that disabling the fast path cannot change any of them.
  EXPECT_LT(cmp(u"a", u"A"), 0);
  EXPECT_LT(cmp(u"A", u"b"), 0);
  EXPECT_LT(cmp(u"abc", u"abd"), 0);
  EXPECT_LT(cmp(u"ab", u"abc"), 0);
  EXPECT_EQ(cmp(u"", u""), 0);
  EXPECT_LT(cmp(u"", u"a"), 0);
  EXPECT_EQ(cmp(u"Hello", u"Hello"), 0);
  EXPECT_LT(cmp(u"Hello", u"World"), 0);
  // Digits sort before letters at the primary level.
  EXPECT_LT(cmp(u"1", u"a"), 0);
  // A mixed-case pair that differs at the primary level, so the tertiary
  // difference must not win.
  EXPECT_LT(cmp(u"aZ", u"bA"), 0);

  // A string with one non-ASCII character must not take the fast path.
  const char16_t aAcute[] = {0x0061, 0x0301};
  EXPECT_LT(cmp(u"a", 1, aAcute, 2), 0);
  const char16_t hi[] = {0x0041, 0x00E9};
  const char16_t lo[] = {0x0041, 0x0065};
  EXPECT_GT(cmp(hi, 2, lo, 2), 0);
}

} // namespace

#endif // not JAVA and not CF
