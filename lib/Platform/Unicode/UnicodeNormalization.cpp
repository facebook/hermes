/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace hermes {
namespace unicode {

#include "NormalizationData.inc"

uint8_t getCanonicalCombiningClass(uint32_t cp) {
  auto *it = std::lower_bound(
      std::begin(CCC_RANGES),
      std::end(CCC_RANGES),
      cp,
      [](const CCCRange &r, uint32_t cp) { return r.last < cp; });
  if (it == std::end(CCC_RANGES) || cp < it->first)
    return 0;
  return it->ccc;
}

namespace {

/// Hangul composition constants, UAX #15 section 10.
constexpr uint32_t kSBase = 0xAC00;
constexpr uint32_t kLBase = 0x1100;
constexpr uint32_t kVBase = 0x1161;
/// One below the first trailing jamo, so that a T index of 0 means "absent".
constexpr uint32_t kTBase = 0x11A7;
constexpr uint32_t kLCount = 19;
constexpr uint32_t kVCount = 21;
constexpr uint32_t kTCount = 28;
constexpr uint32_t kNCount = kVCount * kTCount;
constexpr uint32_t kSCount = kLCount * kNCount;

/// A combining class greater than every real one, used to mark that no starter
/// has been seen yet so that nothing may compose onto it.
constexpr uint8_t kNoStarter = 255;

/// \return true if \p cp has Quick_Check No or Maybe for \p form, meaning the
/// fast path must not be taken.
bool isQCNotYes(uint32_t cp, NormalizationForm form) {
  llvh::ArrayRef<NormRange> table;
  switch (form) {
    case NormalizationForm::C:
      table = NFC_QC_NOT_YES;
      break;
    case NormalizationForm::D:
      table = NFD_QC_NOT_YES;
      break;
    case NormalizationForm::KC:
      table = NFKC_QC_NOT_YES;
      break;
    case NormalizationForm::KD:
      table = NFKD_QC_NOT_YES;
      break;
  }
  auto *it = std::lower_bound(
      table.begin(), table.end(), cp, [](const NormRange &r, uint32_t cp) {
        return r.last < cp;
      });
  return it != table.end() && cp >= it->first;
}

/// \return the entry for \p cp in \p table, or nullptr.
const DecompEntry *findDecomp(llvh::ArrayRef<DecompEntry> table, uint32_t cp) {
  auto *it = std::lower_bound(
      table.begin(), table.end(), cp, [](const DecompEntry &e, uint32_t cp) {
        return e.cp < cp;
      });
  return (it == table.end() || it->cp != cp) ? nullptr : it;
}

/// Append the decomposition of \p cp to \p out.
/// \return false if \p cp has no decomposition, leaving \p out untouched.
bool appendDecomposition(
    uint32_t cp,
    bool compat,
    llvh::SmallVectorImpl<char32_t> &out) {
  // A compatibility mapping is only stored when it differs from the canonical
  // one, so fall back to the canonical table when there is no compatibility
  // entry.
  const DecompEntry *e = compat ? findDecomp(COMPAT_DECOMP, cp) : nullptr;
  if (!e)
    e = findDecomp(CANON_DECOMP, cp);
  if (!e)
    return false;
  if (e->wide) {
    const char32_t *p = DECOMP_POOL32 + e->offset;
    out.append(p, p + e->length);
  } else {
    const char16_t *p = DECOMP_POOL16 + e->offset;
    out.append(p, p + e->length);
  }
  return true;
}

/// \return the canonical composition of \p a and \p b, or 0 if they do not
/// compose.
uint32_t composePair(uint32_t a, uint32_t b) {
  // Hangul L + V.
  uint32_t lIndex = a - kLBase;
  if (lIndex < kLCount) {
    uint32_t vIndex = b - kVBase;
    if (vIndex < kVCount)
      return kSBase + (lIndex * kVCount + vIndex) * kTCount;
  }
  // Hangul LV + T. Only an LV syllable, whose T index is 0, accepts a T.
  uint32_t sIndex = a - kSBase;
  if (sIndex < kSCount && sIndex % kTCount == 0) {
    uint32_t tIndex = b - kTBase;
    if (tIndex > 0 && tIndex < kTCount)
      return a + tIndex;
  }
  auto *it = std::lower_bound(
      std::begin(CANON_COMP),
      std::end(CANON_COMP),
      std::make_pair(a, b),
      [](const CompEntry &e, const std::pair<uint32_t, uint32_t> &k) {
        return std::make_pair(e.starter, e.combining) < k;
      });
  if (it == std::end(CANON_COMP) || it->starter != a || it->combining != b)
    return 0;
  return it->composite;
}

/// \return true if \p buf is already in \p form, per the UAX #15 quick check.
/// A Maybe result is reported as false so the caller runs the full algorithm.
bool isAlreadyNormalized(llvh::ArrayRef<char16_t> buf, NormalizationForm form) {
  uint8_t lastCCC = 0;
  for (size_t i = 0; i < buf.size();) {
    uint32_t cp = nextCodePoint(buf, i);
    uint8_t ccc = getCanonicalCombiningClass(cp);
    if (ccc != 0 && ccc < lastCCC)
      return false;
    if (isQCNotYes(cp, form))
      return false;
    lastCCC = ccc;
  }
  return true;
}

/// Append the full decomposition of \p cp to \p out, or \p cp itself when it
/// has none. Hangul syllables decompose arithmetically.
void decomposeCodePoint(
    uint32_t cp,
    bool compat,
    llvh::SmallVectorImpl<char32_t> &out) {
  uint32_t sIndex = cp - kSBase;
  if (sIndex < kSCount) {
    out.push_back(kLBase + sIndex / kNCount);
    out.push_back(kVBase + (sIndex % kNCount) / kTCount);
    uint32_t tIndex = sIndex % kTCount;
    if (tIndex != 0)
      out.push_back(kTBase + tIndex);
    return;
  }
  if (!appendDecomposition(cp, compat, out))
    out.push_back(cp);
}

/// Sort each maximal run of non-starters by combining class, stably.
void canonicalOrder(llvh::SmallVectorImpl<char32_t> &s) {
  for (size_t i = 1; i < s.size(); ++i) {
    uint8_t ccc = getCanonicalCombiningClass(s[i]);
    if (ccc == 0)
      continue;
    char32_t c = s[i];
    size_t j = i;
    while (j > 0) {
      uint8_t prevCCC = getCanonicalCombiningClass(s[j - 1]);
      if (prevCCC == 0 || prevCCC <= ccc)
        break;
      s[j] = s[j - 1];
      --j;
    }
    s[j] = c;
  }
}

/// Apply the UAX #15 canonical composition algorithm in place.
void composeInPlace(llvh::SmallVectorImpl<char32_t> &s) {
  if (s.empty())
    return;
  size_t starterPos = 0;
  uint32_t starter = s[0];
  size_t outPos = 1;
  uint8_t lastCCC = getCanonicalCombiningClass(starter) != 0 ? kNoStarter : 0;
  for (size_t i = 1; i < s.size(); ++i) {
    uint32_t ch = s[i];
    uint8_t chCCC = getCanonicalCombiningClass(ch);
    uint32_t composite = composePair(starter, ch);
    if (composite != 0 && (lastCCC < chCCC || lastCCC == 0)) {
      s[starterPos] = composite;
      starter = composite;
      continue;
    }
    if (chCCC == 0) {
      starterPos = outPos;
      starter = ch;
    }
    lastCCC = chCCC;
    s[outPos++] = ch;
  }
  s.resize(outPos);
}

} // namespace

void normalizeUTF16(
    llvh::SmallVectorImpl<char16_t> &buf,
    NormalizationForm form) {
  if (isAlreadyNormalized(buf, form))
    return;

  const bool compat =
      form == NormalizationForm::KC || form == NormalizationForm::KD;
  const bool composing =
      form == NormalizationForm::C || form == NormalizationForm::KC;

  llvh::SmallVector<char32_t, 64> cps;
  cps.reserve(buf.size());
  for (size_t i = 0; i < buf.size();)
    decomposeCodePoint(nextCodePoint(buf, i), compat, cps);

  canonicalOrder(cps);

  if (composing)
    composeInPlace(cps);

  buf.clear();
  for (char32_t cp : cps) {
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
