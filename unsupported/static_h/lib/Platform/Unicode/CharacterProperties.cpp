/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"

#include <algorithm>
#include <climits>
#include <iterator>
#include <utility>

namespace hermes {

#include "UnicodeData.inc"

namespace {
struct UnicodeRangeComp {
  bool operator()(UnicodeRange p, uint32_t s) const {
    return p.second < s;
  }
  bool operator()(uint32_t s, UnicodeRange p) const {
    return s < p.first;
  }
};
} // namespace

template <typename UnicodeRangeTable>
inline bool lookup(const UnicodeRangeTable &table, const uint32_t cp) {
  return std::binary_search(
      std::begin(table), std::end(table), cp, UnicodeRangeComp());
}

bool isUnicodeOnlyLetter(uint32_t cp) {
  // "any character in the Unicode categories “Uppercase letter (Lu)”,
  // “Lowercase letter (Ll)”, “Titlecase letter (Lt)”, “Modifier letter (Lm)”,
  // “Other letter (Lo)”, or “Letter number (Nl)”".
  // ASCII characters are not "UnicodeOnly" and so we return false.
  if (cp <= 0x7F)
    return false;

  return lookup(UNICODE_LETTERS, cp);
}

// Special cased due to small number of separate values.
bool isUnicodeOnlySpace(uint32_t cp) {
  // "Other category “Zs”: Any other Unicode “space separator”"
  // Exclude ASCII.
  if (cp <= 0x7F)
    return false;

  switch (cp) {
    case 0xa0:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200a:
    case 0x202f:
    case 0x205f:
    case 0x3000:
      return true;
    default:
      return false;
  }
}

bool isUnicodeCombiningMark(uint32_t cp) {
  // "any character in the Unicode categories “Non-spacing mark (Mn)” or
  // “Combining spacing mark (Mc)”"
  return lookup(UNICODE_COMBINING_MARK, cp);
}

bool isUnicodeDigit(uint32_t cp) {
  // "any character in the Unicode category “Decimal number (Nd)”"
  // 0-9 is the common case.
  return (cp >= '0' && cp <= '9') || lookup(UNICODE_DIGIT, cp);
};

bool isUnicodeConnectorPunctuation(uint32_t cp) {
  // "any character in the Unicode category “Connector punctuation (Pc)"
  // _ is the common case.
  return cp == '_' || lookup(UNICODE_CONNECTOR_PUNCTUATION, cp);
}

static uint32_t applyTransform(const UnicodeTransformRange &r, uint32_t cp) {
  assert(
      r.start <= cp && cp < r.start + r.count &&
      "range does not contain this cp");
  assert(cp <= INT32_MAX && "cp is too big");
  // Check if our code point has a mod of 0.
  if ((cp - r.start) % r.modulo == 0) {
    int32_t cps = static_cast<int32_t>(cp) + r.delta;
    assert(cps >= 0 && "delta underflowed");
    return static_cast<uint32_t>(cps);
  }
  return cp;
}

// Predicate used to enable binary search on UnicodeTransformRange.
// Here we return true if the last element of the range is < cp, so <= is
// correct.
static bool operator<(const UnicodeTransformRange &m, uint32_t cp) {
  return m.start + m.count <= cp;
}

/// Find all code points which canonicalize to a value in \p range, and add them
/// to \p receiver. This is a slow linear search across all ranges.
static void addPrecanonicalCharacters(
    CodePointRange range,
    CodePointSet *receiver,
    bool unicode) {
  if (range.length == 0)
    return;
  // TODO: if range is ASCII and unicode is not set, we can stop the search
  // after the ASCII part as nothing outside ASCII can canonicalize to an ASCII
  // character.
  const auto start =
      unicode ? std::begin(UNICODE_FOLDS) : std::begin(LEGACY_CANONS);
  const auto end = unicode ? std::end(UNICODE_FOLDS) : std::end(LEGACY_CANONS);
  for (auto iter = start; iter != end; ++iter) {
    const UnicodeTransformRange &transform = *iter;
    // Get the range of transformed-from and transformed-to characters.
    CodePointRange fromRange{transform.start, transform.count};
    CodePointRange toRange = fromRange;
    toRange.first += transform.delta;

    // See if some character in our range will be transformed-to.
    if (!range.overlaps(toRange))
      continue;

    // Looks like it. Add everything.
    for (uint32_t cp = fromRange.first; cp < fromRange.end(); cp++) {
      uint32_t tcp = applyTransform(transform, cp);
      if (tcp != cp && range.first <= tcp && tcp < range.end()) {
        receiver->add(cp);
      }
    }
  }
}

/// For each code point in \p range, canonicalize it and add the canonicalized
/// values to \p receiver.
static void
canonicalizeRange(CodePointRange range, CodePointSet *receiver, bool unicode) {
  assert(range.length > 0 && "Range should never be empty");

  /// Find the first transform that contains or starts after our range.
  const auto start =
      unicode ? std::begin(UNICODE_FOLDS) : std::begin(LEGACY_CANONS);
  const auto end = unicode ? std::end(UNICODE_FOLDS) : std::end(LEGACY_CANONS);
  auto transform = std::lower_bound(start, end, range.first);

  uint32_t curcp = range.first;
  uint32_t endcp = range.end();
  while (curcp < endcp && transform != end) {
    uint32_t transformEnd = transform->start + transform->count;
    assert(transformEnd > curcp && "transform should end after our current cp");
    if (transform->start > curcp) {
      // Our transform started after the current value, so skip to the
      // transform.
      curcp = transform->start;
    } else {
      // Apply this transform for the code points that are in the transform and
      // also in our range.
      for (; curcp < transformEnd && curcp < endcp; curcp++) {
        receiver->add(applyTransform(*transform, curcp));
      }
      // We have now exhausted this transform; go to the next one.
      ++transform;
    }
  }
}

CodePointSet makeCanonicallyEquivalent(const CodePointSet &set, bool unicode) {
  // Canonicalize all characters in the set, and then find all characters which
  // canonicalize to some element in the set.
  CodePointSet canonicalized = set;
  for (const auto &range : set.ranges()) {
    canonicalizeRange(range, &canonicalized, unicode);
  }

  CodePointSet result = canonicalized;
  for (const auto &range : canonicalized.ranges()) {
    addPrecanonicalCharacters(range, &result, unicode);
  }
  return result;
}

uint32_t canonicalize(uint32_t cp, bool unicode) {
  const auto start =
      unicode ? std::begin(UNICODE_FOLDS) : std::begin(LEGACY_CANONS);
  const auto end = unicode ? std::end(UNICODE_FOLDS) : std::end(LEGACY_CANONS);
  auto where = std::lower_bound(start, end, cp);
  if (where != end && where->start <= cp && cp < where->start + where->count) {
    return applyTransform(*where, cp);
  } else {
    // No transform for this character, so it canonicalizes to itself.
    return cp;
  }
}

} // namespace hermes
