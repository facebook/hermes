/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_UNICODECOLLATION_H
#define HERMES_PLATFORMUNICODE_UNICODECOLLATION_H

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>
#include <utility>

namespace hermes {
namespace unicode {

/// Compare \p left and \p right under the DUCET root collation of Unicode
/// Technical Standard #10, using non-ignorable variable weighting and
/// comparing through the tertiary level.
/// \return -1, 0, or 1 according to whether \p left compares less than, equal
/// to, or greater than \p right.
///
/// This is a self-contained implementation driven by generated tables; it
/// depends on no system library and on no locale. Strings that are equal
/// through the tertiary level compare equal, matching ICU with its identical
/// level disabled: an embedded U+0000 does not affect the result, because
/// it is completely ignorable. The result is a consistent comparison function,
/// which is all ECMA-262 requires of String.prototype.localeCompare.
///
/// Both arguments are treated as WTF-16: unpaired surrogates receive implicit
/// weights rather than being replaced or dropped, and U+0000 is an ordinary
/// character rather than a terminator.
int compareUTF16(llvh::ArrayRef<char16_t> left, llvh::ArrayRef<char16_t> right);

/// \return the pair of primary weights UTS #10 section 10.1.3 assigns to
/// \p cp, which is assumed to have no entry in the collation table.
///
/// Exposed only so a unit test can check the formula directly. The
/// @implicitweights bases are unobservable through compareUTF16 with the
/// current DUCET data: no reachable code point has an explicit primary
/// anywhere in 0xFAFA..0xFB3F, so every wrong-but-monotonic assignment of
/// those bases orders identically and even the UCA conformance suite passes.
std::pair<uint16_t, uint16_t> implicitPrimariesForTesting(uint32_t cp);

} // namespace unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_UNICODECOLLATION_H
