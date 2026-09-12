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

#include <string>

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

  // The absorbed breve must not also be counted in its original position.
  // U+0F71 U+0F71 U+0F72 (Tibetan vowel signs) exercises this directly,
  // unlike the pair above: 0F71 and 0F72 do not decompose and their ccc
  // values (129, 130) are already in canonical order, so NFD is a no-op and
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

} // namespace

#endif // not JAVA and not CF
