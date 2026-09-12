/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/UnicodeCollation.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"
#include "llvh/ADT/SmallVector.h"

#include <algorithm>
#include <tuple>
#include <utility>

namespace hermes {
namespace unicode {
namespace {

#include "CollationData.inc"

/// A collation element with its weights resolved out of the style pool.
/// The variable flag is deliberately absent: under non-ignorable variable
/// weighting, which is what this implementation and ICU's default both use,
/// variable characters keep their weights and the flag is never consulted.
struct Weights {
  uint16_t primary;
  uint16_t secondary;
  uint8_t tertiary;
};

/// \return the weights of the packed collation element \p e.
Weights resolve(CollElem e) {
  const CollStyle &s = COLL_STYLES[e.style];
  return {e.primary, s.secondary, s.tertiary};
}

/// \return true if \p cp lies in one of the sorted, disjoint \p ranges.
bool inRanges(llvh::ArrayRef<CollRange> ranges, uint32_t cp) {
  auto *it = std::upper_bound(
      ranges.begin(), ranges.end(), cp, [](uint32_t v, const CollRange &r) {
        return v < r.first;
      });
  return it != ranges.begin() && cp <= (it - 1)->last;
}

/// \return true if \p cp has a single-element mapping, storing it in \p out.
bool lookupRun(uint32_t cp, Weights &out) {
  auto *it = std::upper_bound(
      std::begin(COLL_RUNS),
      std::end(COLL_RUNS),
      cp,
      [](uint32_t v, const CollRun &r) { return v < r.first; });
  if (it == std::begin(COLL_RUNS))
    return false;
  --it;
  uint32_t offset = cp - it->first;
  if (offset >= it->count)
    return false;
  const CollStyle &s = COLL_STYLES[it->style];
  out = {(uint16_t)(it->primary + offset * it->step), s.secondary, s.tertiary};
  return true;
}

/// \return true if \p cp expands, appending its elements to \p out.
bool lookupExpansion(uint32_t cp, llvh::SmallVectorImpl<Weights> &out) {
  auto *it = std::lower_bound(
      std::begin(COLL_EXPANSIONS),
      std::end(COLL_EXPANSIONS),
      cp,
      [](const CollExpansion &e, uint32_t v) { return e.cp < v; });
  if (it == std::end(COLL_EXPANSIONS) || it->cp != cp)
    return false;
  for (unsigned k = 0; k < it->length; ++k)
    out.push_back(resolve(COLL_CE_POOL[it->offset + k]));
  return true;
}

/// \return the two primary weights UTS #10 section 10.1.3 assigns to \p cp,
/// which is assumed to have no entry in the collation table.
std::pair<uint16_t, uint16_t> implicitPrimaries(uint32_t cp) {
  // An @implicitweights range uses its base verbatim as the first primary
  // and measures the second one from the base's anchor -- the start of the
  // first range using that base, which is not this range's own start when
  // two ranges share a base. It does not use the cp >> 15 / cp & 0x7FFF
  // split below, which would spread a single base over several primaries
  // and land the following ranges' bases inside it.
  for (const CollImplicitRange &r : COLL_IMPLICIT_RANGES) {
    if (cp >= r.first && cp <= r.last)
      return {r.base, (uint16_t)((cp - r.anchor) | 0x8000)};
  }
  uint16_t base = COLL_UNASSIGNED_BASE;
  if (inRanges(COLL_UNIFIED_IDEOGRAPHS, cp)) {
    base = inRanges(COLL_HAN_CORE_BLOCKS, cp) ? COLL_HAN_CORE_BASE
                                              : COLL_HAN_OTHER_BASE;
  }
  return {(uint16_t)(base + (cp >> 15)), (uint16_t)((cp & 0x7FFF) | 0x8000)};
}

/// Append the implicit weights of \p cp to \p out, per UTS #10 section
/// 10.1.3. Every implicitly weighted code point yields two elements.
void appendImplicit(uint32_t cp, llvh::SmallVectorImpl<Weights> &out) {
  std::pair<uint16_t, uint16_t> primaries = implicitPrimaries(cp);
  out.push_back({primaries.first, 0x0020, 0x02});
  out.push_back({primaries.second, 0x0000, 0x00});
}

/// \return the offset into COLL_CONTRACTIONS of the contraction whose key is
/// exactly (\p a, \p b, \p c), or nullptr if there is none. \p c is 0 for a
/// two-code-point key, which is unambiguous because 0 is never a member.
const CollContraction *findContraction(uint32_t a, uint32_t b, uint32_t c) {
  auto *it = std::lower_bound(
      std::begin(COLL_CONTRACTIONS),
      std::end(COLL_CONTRACTIONS),
      std::make_tuple(a, b, c),
      [](const CollContraction &e,
         const std::tuple<uint32_t, uint32_t, uint32_t> &k) {
        return std::make_tuple(e.cp0, e.cp1, e.cp2) < k;
      });
  if (it == std::end(COLL_CONTRACTIONS))
    return nullptr;
  if (it->cp0 != a || it->cp1 != b || it->cp2 != c)
    return nullptr;
  return it;
}

/// \return true if \p cp is the first code point of some contraction.
/// COLL_CONTRACTIONS is sorted lexicographically by (cp0, cp1, cp2), so the
/// rows sharing a cp0 are adjacent and one binary search settles it.
bool beginsContraction(uint32_t cp) {
  auto *it = std::lower_bound(
      std::begin(COLL_CONTRACTIONS),
      std::end(COLL_CONTRACTIONS),
      cp,
      [](const CollContraction &e, uint32_t v) { return e.cp0 < v; });
  return it != std::end(COLL_CONTRACTIONS) && it->cp0 == cp;
}

/// Append the elements of \p contraction to \p out.
void appendContraction(
    const CollContraction &contraction,
    llvh::SmallVectorImpl<Weights> &out) {
  for (unsigned k = 0; k < contraction.length; ++k)
    out.push_back(resolve(COLL_CE_POOL[contraction.offset + k]));
}

/// The key array and the discontiguous scan below are sized for a
/// three-code-point contraction; a regenerated table with longer keys must
/// not silently mis-collate.
static_assert(
    COLL_MAX_CONTRACTION_LENGTH == 3,
    "matchContraction hardcodes a 3-code-point contraction key");

/// Match a contraction beginning at \p cps[i].
///
/// Applies the longest contiguous match of UTS #10 S2.1, then the
/// discontiguous extension of S2.1.1 through S2.1.3: a following non-starter
/// that is not blocked from the matched substring may be absorbed into it
/// even though it is not adjacent. A non-starter C is blocked if some
/// character B between the end of the match and C has ccc(B) == 0 or
/// ccc(B) >= ccc(C).
///
/// \p absorbed marks positions that an earlier call already folded into a
/// discontiguous match; those positions are removed from the sequence and
/// must be treated as absent by every later call, both when reading them
/// (they contribute nothing and cannot be re-matched or re-weighed) and when
/// scanning past them (an absorbed position cannot act as a blocker either).
/// This call marks any *new* absorbed positions it finds beyond its own
/// contiguous run so the caller does not weigh them a second time.
/// \return the number of code points consumed from \p i by the contiguous
/// part of the match, or 1 if the match was purely discontiguous (the
/// initial code point plus one or more absorbed, non-contiguous positions),
/// or 0 if no contraction matched at all.
size_t matchContraction(
    llvh::ArrayRef<uint32_t> cps,
    size_t i,
    llvh::SmallVectorImpl<bool> &absorbed,
    llvh::SmallVectorImpl<Weights> &out) {
  // Neither the contiguous probe nor the S2.1.1 scan below can match unless
  // cps[i] begins some contraction, so settle that first. Without this the
  // scan runs at every position and walks to the end of the surrounding
  // combining run, costing Theta(N^2) with a combining-class binary search
  // inside: 100,000 copies of U+0301 hang, and that is one line of
  // JavaScript away. U+0301 is not a contraction lead, so the check retires
  // that input, and it also short-circuits the common case, since most
  // characters begin no contraction at all.
  //
  // It does not make the scan linear in general. A code point that *is* a
  // contraction lead still pays the full scan at every position, so a long
  // run of U+0F71 stays quadratic.
  if (!beginsContraction(cps[i]))
    return 0;

  // S2.1: the longest contiguous match. The key is padded with zeros, which
  // is unambiguous because 0 is never a contraction member. The 3-code-point
  // probe below degenerates into a 2-code-point key when cps[i + 2] is 0
  // (e.g. an embedded U+0000), so the matched row's own cp2 -- not the probe
  // length -- determines how many input code points were truly consumed.
  uint32_t key[3] = {cps[i], 0, 0};
  size_t keyLen = 1;
  const CollContraction *best = nullptr;
  size_t consumed = 0;

  size_t maxLen = std::min(COLL_MAX_CONTRACTION_LENGTH, cps.size() - i);
  // An absorbed position was removed by an earlier discontiguous match, so
  // the probe must not read it as though it were still part of the
  // sequence. Capping the probe length handles an absorbed cps[i + 1] or
  // cps[i + 2] directly. It does not go further and reconstruct a
  // contiguous key by skipping over an absorbed position to reach a later
  // starter (e.g. X, <absorbed>, Y where X, Y is itself a contraction); that
  // rarer case is left unhandled.
  if (maxLen >= 2 && absorbed[i + 1])
    maxLen = 1;
  else if (maxLen >= 3 && absorbed[i + 2])
    maxLen = 2;
  for (size_t len = maxLen; len >= 2; --len) {
    const CollContraction *hit =
        findContraction(cps[i], cps[i + 1], len >= 3 ? cps[i + 2] : 0);
    if (hit) {
      best = hit;
      keyLen = hit->cp2 == 0 ? 2 : 3;
      consumed = keyLen;
      for (size_t k = 1; k < keyLen; ++k)
        key[k] = cps[i + k];
      break;
    }
  }

  // S2.1.1-S2.1.3: extend the match across unblocked non-starters. The scan
  // starts after whatever was consumed contiguously, or after the initial
  // code point if nothing was.
  size_t scanFrom = i + (consumed ? consumed : 1);
  uint8_t blockingCCC = 0;
  for (size_t k = scanFrom; k < cps.size() && keyLen < 3; ++k) {
    if (absorbed[k])
      continue;
    uint8_t ccc = getCanonicalCombiningClass(cps[k]);
    // A starter ends the scan: nothing past it can be part of this
    // contraction.
    if (ccc == 0)
      break;
    if (blockingCCC >= ccc) {
      // Blocked by an earlier mark of equal or higher class. It stays where
      // it is and becomes a blocker for anything after it.
      blockingCCC = ccc;
      continue;
    }
    key[keyLen] = cps[k];
    const CollContraction *hit = findContraction(key[0], key[1], key[2]);
    if (hit) {
      best = hit;
      ++keyLen;
      absorbed[k] = true;
      if (!consumed)
        consumed = 1;
      // The absorbed character is gone, so it cannot block anything.
    } else {
      key[keyLen] = 0;
      blockingCCC = ccc;
    }
  }

  if (!best)
    return 0;
  appendContraction(*best, out);
  return consumed;
}

/// Decompose \p s to NFD and append its collation elements to \p out.
void buildElements(
    llvh::ArrayRef<char16_t> s,
    llvh::SmallVectorImpl<Weights> &out) {
  // NFD is a precondition of the table, not merely a nicety: entries for
  // canonically decomposable code points are not generated at all, because
  // they can never be reached after this step. It also gives the
  // canonical-equivalence behavior ICU provides via UCOL_NORMALIZATION_MODE.
  llvh::SmallVector<char16_t, 64> buf(s.begin(), s.end());
  normalizeUTF16(buf, NormalizationForm::D);

  llvh::SmallVector<uint32_t, 64> cps;
  cps.reserve(buf.size());
  for (size_t i = 0; i < buf.size();)
    cps.push_back(nextCodePoint(buf, i));

  llvh::SmallVector<bool, 64> absorbed(cps.size(), false);
  for (size_t i = 0; i < cps.size();) {
    if (absorbed[i]) {
      ++i;
      continue;
    }
    if (size_t consumed = matchContraction(cps, i, absorbed, out)) {
      i += consumed;
      continue;
    }
    Weights w;
    if (lookupRun(cps[i], w)) {
      out.push_back(w);
    } else if (!lookupExpansion(cps[i], out)) {
      appendImplicit(cps[i], out);
    }
    ++i;
  }
}

/// Compare one level of two collation element arrays, skipping zero weights
/// as the sort key construction of UTS #10 section 7.3 does.
/// \p get extracts the weight for the level being compared.
template <typename Get>
int compareLevel(
    llvh::ArrayRef<Weights> left,
    llvh::ArrayRef<Weights> right,
    Get get) {
  size_t i = 0, j = 0;
  for (;;) {
    while (i < left.size() && get(left[i]) == 0)
      ++i;
    while (j < right.size() && get(right[j]) == 0)
      ++j;
    if (i == left.size() || j == right.size())
      break;
    if (get(left[i]) != get(right[j]))
      return get(left[i]) < get(right[j]) ? -1 : 1;
    ++i;
    ++j;
  }
  if (i == left.size() && j == right.size())
    return 0;
  return i == left.size() ? -1 : 1;
}

/// Compare \p left and \p right by the full UTS #10 algorithm: normalize to
/// NFD, build collation element arrays, then walk the three levels.
int compareGeneral(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  llvh::SmallVector<Weights, 64> l, r;
  buildElements(left, l);
  buildElements(right, r);

  if (int c = compareLevel(
          l, r, [](const Weights &w) { return (uint32_t)w.primary; }))
    return c;
  if (int c = compareLevel(
          l, r, [](const Weights &w) { return (uint32_t)w.secondary; }))
    return c;
  return compareLevel(
      l, r, [](const Weights &w) { return (uint32_t)w.tertiary; });
}

/// \return true if every code unit of \p s is ASCII.
bool isAllASCII(llvh::ArrayRef<char16_t> s) {
  for (char16_t c : s)
    if (c >= 0x80)
      return false;
  return true;
}

/// Compare two all-ASCII strings at the primary level without normalizing or
/// building collation element arrays.
///
/// This is sound because ASCII's NFD-stability is guaranteed by Unicode's
/// stability policy. The other two preconditions -- that every ASCII code
/// point has a single-element mapping, and that no contraction is entirely
/// ASCII -- are properties of the DUCET data rather than of Unicode itself,
/// so the generator asserts both rather than assuming them. Sorting ASCII
/// strings is the common use of localeCompare, and the general path
/// allocates two element arrays for every comparison.
///
/// Strings that tie at the primary level fall through to \c compareGeneral,
/// which redoes the primary-level walk from scratch, so a case-only
/// difference such as "a" vs "A" does strictly more work than skipping the
/// fast path entirely would; the trade-off is accepted because it keeps the
/// level 2 and 3 walks in exactly one place.
int compareASCII(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  // Some ASCII code points are completely ignorable, so this skips zero
  // primaries exactly as compareLevel does.
  size_t i = 0, j = 0;
  Weights lw{}, rw{};
  for (;;) {
    while (i < left.size() && (!lookupRun(left[i], lw) || lw.primary == 0))
      ++i;
    while (j < right.size() && (!lookupRun(right[j], rw) || rw.primary == 0))
      ++j;
    if (i == left.size() || j == right.size())
      break;
    if (lw.primary != rw.primary)
      return lw.primary < rw.primary ? -1 : 1;
    ++i;
    ++j;
  }
  if (i != left.size() || j != right.size())
    return i == left.size() ? -1 : 1;
  return compareGeneral(left, right);
}

} // namespace

int compareUTF16(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  if (isAllASCII(left) && isAllASCII(right))
    return compareASCII(left, right);
  return compareGeneral(left, right);
}

std::pair<uint16_t, uint16_t> implicitPrimariesForTesting(uint32_t cp) {
  return implicitPrimaries(cp);
}

} // namespace unicode
} // namespace hermes
