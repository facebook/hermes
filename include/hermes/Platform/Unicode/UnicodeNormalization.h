/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_UNICODENORMALIZATION_H
#define HERMES_PLATFORMUNICODE_UNICODENORMALIZATION_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace unicode {

/// Options for normalizing Unicode strings.
/// NOTE: If these change, then AndroidUnicodeUtils.java must be updated,
/// because PlatformUnicodeJava.cpp passes the enumerator ordinal across JNI.
/// http://www.unicode.org/reports/tr15/
enum class NormalizationForm { C, D, KC, KD };

/// Normalize the UTF-16 string \p buf into the given \p form in place, using
/// the normalization forms described in Unicode Technical Report #15.
///
/// This is a self-contained implementation driven by generated tables; it
/// depends on no system library. Backends for platforms that have no
/// higher-quality normalizer available call it directly.
///
/// \p buf is treated as WTF-16: unpaired surrogates are preserved unchanged,
/// and U+0000 is an ordinary character rather than a terminator.
void normalizeUTF16(
    llvh::SmallVectorImpl<char16_t> &buf,
    NormalizationForm form);

/// \return the canonical combining class of \p cp, or 0 if it has none.
/// Exposed because the case conversion conditional rules need it and should
/// not duplicate the table.
uint8_t getCanonicalCombiningClass(uint32_t cp);

} // namespace unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_UNICODENORMALIZATION_H
