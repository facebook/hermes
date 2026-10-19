/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_UNICODECASECONVERSION_H
#define HERMES_PLATFORMUNICODE_UNICODECASECONVERSION_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace unicode {

/// Options for case conversions: to uppercase or to lowercase.
/// NOTE: If these change, then AndroidUnicodeUtils.java must be updated,
/// because PlatformUnicodeJava.cpp passes the enumerator ordinal across JNI.
enum class CaseConversion { ToUpper, ToLower };

/// The language-specific casing rules to apply. Azerbaijani shares Turkish's
/// rules exactly, so it has no separate enumerator.
enum class CaseLocale { Root, Turkish, Lithuanian };

/// Convert \p buf to \p targetCase in place, applying the full Unicode case
/// mappings and the conditional rules for \p locale.
///
/// \p buf is treated as WTF-16: unpaired surrogates are preserved unchanged
/// and U+0000 is an ordinary character.
void convertCaseUTF16(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    CaseLocale locale);

} // namespace unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_UNICODECASECONVERSION_H
