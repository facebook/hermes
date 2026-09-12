/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/UnicodeCaseConversion.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include <algorithm>
#include <iterator>

namespace hermes {
namespace unicode {

#include "CaseData.inc"

namespace {

/// U+0307 COMBINING DOT ABOVE, the dot that Table 3-17's Lithuanian and
/// Turkish/Azeri rules insert, delete, or test for.
constexpr uint32_t kCombiningDotAbove = 0x0307;
/// U+0049 LATIN CAPITAL LETTER I.
constexpr uint32_t kCapitalI = 0x0049;
/// The Canonical_Combining_Class value "Above" (ccc=230), used by several
/// Table 3-17 conditions to find the nearest preceding/following character
/// that is neither a base (ccc=0) nor another "Above" mark.
constexpr uint8_t kCccAbove = 230;

/// Code points driving the 16 conditional entries of SpecialCasing.txt,
/// Unicode Standard section 3.13, Table 3-17.
constexpr uint32_t kGreekCapitalSigma = 0x03A3; // Sigma
constexpr uint32_t kGreekSmallFinalSigma = 0x03C2; // final form
constexpr uint32_t kCapitalJ = 0x004A;
constexpr uint32_t kCapitalIWithOgonek = 0x012E;
constexpr uint32_t kSmallIWithOgonek = 0x012F;
constexpr uint32_t kCapitalIWithGrave = 0x00CC;
constexpr uint32_t kCapitalIWithAcute = 0x00CD;
constexpr uint32_t kCapitalIWithTilde = 0x0128;
constexpr uint32_t kCapitalIWithDotAbove = 0x0130; // Turkish/Azeri I-dot
constexpr uint32_t kSmallDotlessI = 0x0131;
constexpr uint32_t kSmallI = 0x0069;
constexpr uint32_t kCombiningGraveAbove = 0x0300;
constexpr uint32_t kCombiningAcuteAbove = 0x0301;
constexpr uint32_t kCombiningTildeAbove = 0x0303;

/// \return true if \p cp falls in one of the sorted, non-overlapping
/// [first, last] ranges of \p table.
bool inRanges(llvh::ArrayRef<CaseRange> table, uint32_t cp) {
  auto *it = std::lower_bound(
      table.begin(), table.end(), cp, [](const CaseRange &r, uint32_t cp) {
        return r.last < cp;
      });
  return it != table.end() && cp >= it->first;
}

/// \return true if \p cp has the Unicode Cased property (Table 3-17's
/// building block for Final_Sigma).
bool isCased(uint32_t cp) {
  return inRanges(CASED_RANGES, cp);
}
/// \return true if \p cp has the Unicode Case_Ignorable property (Table
/// 3-17's building block for Final_Sigma).
bool isCaseIgnorable(uint32_t cp) {
  return inRanges(CASE_IGNORABLE_RANGES, cp);
}
/// \return true if \p cp has the Unicode Soft_Dotted property (Table 3-17's
/// building block for After_Soft_Dotted).
bool isSoftDotted(uint32_t cp) {
  return inRanges(SOFT_DOTTED_RANGES, cp);
}

/// Apply the delta-block table, mirroring applyTransform in
/// CharacterProperties.cpp.
uint32_t applyDelta(llvh::ArrayRef<CaseDelta> table, uint32_t cp) {
  auto *it = std::lower_bound(
      table.begin(), table.end(), cp, [](const CaseDelta &r, uint32_t cp) {
        return r.start + r.count <= cp;
      });
  if (it == table.end() || cp < it->start)
    return cp;
  if ((cp - it->start) % it->modulo != 0)
    return cp;
  return (uint32_t)((int32_t)cp + it->delta);
}

/// \return the entry for \p cp in the sorted-by-codepoint table \p t, or
/// nullptr if \p cp has no full (possibly multi-character) case mapping.
const FullCaseEntry *findFull(llvh::ArrayRef<FullCaseEntry> t, uint32_t cp) {
  auto *it = std::lower_bound(
      t.begin(), t.end(), cp, [](const FullCaseEntry &e, uint32_t cp) {
        return e.cp < cp;
      });
  return (it == t.end() || it->cp != cp) ? nullptr : it;
}

/// Table 3-17 Final_Sigma: preceded by a cased letter then zero or more
/// case-ignorable characters, and not followed by zero or more
/// case-ignorable characters then a cased letter.
/// NOTE: this checks Case_Ignorable before Cased at each scanned character,
/// so for the ~200 code points that are both Cased and Case_Ignorable, they
/// are treated as ignorable rather than cased. This is a deliberate,
/// intentional departure from a literal reading of the definition above; it
/// is what ICU does, and matching ICU exactly is the goal. Do not "fix" it.
bool isFinalSigma(llvh::ArrayRef<char32_t> s, size_t i) {
  size_t j = i;
  bool precededByCased = false;
  while (j > 0) {
    uint32_t c = s[--j];
    if (isCaseIgnorable(c))
      continue;
    precededByCased = isCased(c);
    break;
  }
  if (!precededByCased)
    return false;
  for (size_t k = i + 1; k < s.size(); ++k) {
    uint32_t c = s[k];
    if (isCaseIgnorable(c))
      continue;
    return !isCased(c);
  }
  return true;
}

/// Table 3-17 After_I: an uppercase I before C, with no intervening character
/// of combining class 230 or 0.
bool isAfterI(llvh::ArrayRef<char32_t> s, size_t i) {
  for (size_t j = i; j > 0;) {
    uint32_t c = s[--j];
    if (c == kCapitalI)
      return true;
    uint8_t ccc = getCanonicalCombiningClass(c);
    if (ccc == 0 || ccc == kCccAbove)
      return false;
  }
  return false;
}

/// Table 3-17 More_Above: followed by a character of combining class 230,
/// with no intervening character of combining class 0 or 230.
bool isMoreAbove(llvh::ArrayRef<char32_t> s, size_t i) {
  for (size_t j = i + 1; j < s.size(); ++j) {
    uint8_t ccc = getCanonicalCombiningClass(s[j]);
    if (ccc == kCccAbove)
      return true;
    if (ccc == 0)
      return false;
  }
  return false;
}

/// Table 3-17 After_Soft_Dotted: a Soft_Dotted character before C, with no
/// intervening character of combining class 0 or 230.
bool isAfterSoftDotted(llvh::ArrayRef<char32_t> s, size_t i) {
  for (size_t j = i; j > 0;) {
    uint32_t c = s[--j];
    if (isSoftDotted(c))
      return true;
    uint8_t ccc = getCanonicalCombiningClass(c);
    if (ccc == 0 || ccc == kCccAbove)
      return false;
  }
  return false;
}

/// Table 3-17 Before_Dot: followed by U+0307, with only characters of
/// combining class neither 0 nor 230 intervening.
bool isBeforeDot(llvh::ArrayRef<char32_t> s, size_t i) {
  for (size_t j = i + 1; j < s.size(); ++j) {
    uint32_t c = s[j];
    if (c == kCombiningDotAbove)
      return true;
    uint8_t ccc = getCanonicalCombiningClass(c);
    if (ccc == 0 || ccc == kCccAbove)
      return false;
  }
  return false;
}

/// Apply the conditional entries of SpecialCasing.txt that are specific to
/// \p locale, plus the locale-independent Final_Sigma rule. \return true if
/// a rule applied and appended \p s[i]'s mapping (possibly nothing, for the
/// entries that delete the character) to \p out; false if the caller should
/// fall through to the default mapping.
bool applyConditional(
    llvh::ArrayRef<char32_t> s,
    size_t i,
    CaseConversion targetCase,
    CaseLocale locale,
    llvh::SmallVectorImpl<char32_t> &out) {
  uint32_t cp = s[i];
  if (targetCase == CaseConversion::ToLower) {
    // Final_Sigma is locale-independent.
    if (cp == kGreekCapitalSigma && isFinalSigma(s, i)) {
      out.push_back(kGreekSmallFinalSigma);
      return true;
    }
    if (locale == CaseLocale::Lithuanian) {
      switch (cp) {
        case kCapitalI:
          if (isMoreAbove(s, i)) {
            out.push_back(kSmallI);
            out.push_back(kCombiningDotAbove);
            return true;
          }
          break;
        case kCapitalJ:
          if (isMoreAbove(s, i)) {
            out.push_back(0x006A); // "j"
            out.push_back(kCombiningDotAbove);
            return true;
          }
          break;
        case kCapitalIWithOgonek:
          if (isMoreAbove(s, i)) {
            out.push_back(kSmallIWithOgonek);
            out.push_back(kCombiningDotAbove);
            return true;
          }
          break;
        case kCapitalIWithGrave:
          out.push_back(kSmallI);
          out.push_back(kCombiningDotAbove);
          out.push_back(kCombiningGraveAbove);
          return true;
        case kCapitalIWithAcute:
          out.push_back(kSmallI);
          out.push_back(kCombiningDotAbove);
          out.push_back(kCombiningAcuteAbove);
          return true;
        case kCapitalIWithTilde:
          out.push_back(kSmallI);
          out.push_back(kCombiningDotAbove);
          out.push_back(kCombiningTildeAbove);
          return true;
        default:
          break;
      }
    } else if (locale == CaseLocale::Turkish) {
      switch (cp) {
        case kCapitalIWithDotAbove:
          out.push_back(kSmallI);
          return true;
        case kCombiningDotAbove:
          // After_I: the dot is deleted, i.e. nothing is appended.
          if (isAfterI(s, i))
            return true;
          break;
        case kCapitalI:
          // Not_Before_Dot: negation of Before_Dot.
          if (!isBeforeDot(s, i)) {
            out.push_back(kSmallDotlessI);
            return true;
          }
          break;
        default:
          break;
      }
    }
  } else {
    // CaseConversion::ToUpper.
    if (locale == CaseLocale::Lithuanian && cp == kCombiningDotAbove &&
        isAfterSoftDotted(s, i)) {
      // After_Soft_Dotted: the dot is deleted, i.e. nothing is appended.
      return true;
    }
    if (locale == CaseLocale::Turkish && cp == kSmallI) {
      out.push_back(kCapitalIWithDotAbove);
      return true;
    }
  }
  return false;
}

/// Append the default (unconditional) case mapping of \p cp to \p out: the
/// full mapping if one exists, otherwise the simple delta mapping.
void appendDefaultMapping(
    uint32_t cp,
    CaseConversion targetCase,
    llvh::SmallVectorImpl<char32_t> &out) {
  const bool up = targetCase == CaseConversion::ToUpper;
  llvh::ArrayRef<FullCaseEntry> fullTable;
  if (up)
    fullTable = FULL_UPPER;
  else
    fullTable = FULL_LOWER;
  if (const FullCaseEntry *e = findFull(fullTable, cp)) {
    const char16_t *p = FULL_CASE_POOL + e->offset;
    out.append(p, p + e->length);
    return;
  }
  llvh::ArrayRef<CaseDelta> deltaTable;
  if (up)
    deltaTable = TO_UPPER_DELTAS;
  else
    deltaTable = TO_LOWER_DELTAS;
  out.push_back(applyDelta(deltaTable, cp));
}

/// Append the case mapping of \p s[i] to \p out, applying the conditional
/// rules for \p locale before falling back to the default mapping.
void convertCodePoint(
    llvh::ArrayRef<char32_t> s,
    size_t i,
    CaseConversion targetCase,
    CaseLocale locale,
    llvh::SmallVectorImpl<char32_t> &out) {
  if (applyConditional(s, i, targetCase, locale, out))
    return;
  appendDefaultMapping(s[i], targetCase, out);
}

} // namespace

void convertCaseUTF16(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    CaseLocale locale) {
  llvh::SmallVector<char32_t, 64> cps;
  cps.reserve(buf.size());
  for (size_t i = 0; i < buf.size();)
    cps.push_back(nextCodePoint(buf, i));

  llvh::SmallVector<char32_t, 64> out;
  out.reserve(cps.size());
  for (size_t i = 0; i < cps.size(); ++i)
    convertCodePoint(cps, i, targetCase, locale, out);

  buf.clear();
  for (char32_t cp : out) {
    if (cp > UNICODE_LAST_BMP) {
      uint32_t v = cp - 0x10000;
      buf.push_back((char16_t)(UTF16_HIGH_SURROGATE + (v >> 10)));
      buf.push_back((char16_t)(UTF16_LOW_SURROGATE + (v & 0x3FF)));
    } else {
      buf.push_back((char16_t)cp);
    }
  }
}

} // namespace unicode
} // namespace hermes
