/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H
#define HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"

/// Decide on which Unicode implementation to use.
#define HERMES_PLATFORM_UNICODE_JAVA 1
#define HERMES_PLATFORM_UNICODE_CF 2
#define HERMES_PLATFORM_UNICODE_ICU 3
#define HERMES_PLATFORM_UNICODE_EMSCRIPTEN 4
#define HERMES_PLATFORM_UNICODE_LITE 5

#ifndef HERMES_PLATFORM_UNICODE
#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_JAVA
#elif defined(__APPLE__)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_CF
#elif defined(__EMSCRIPTEN__)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_EMSCRIPTEN
#else
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_ICU
#endif
#endif

namespace hermes {
namespace platform_unicode {

/// Compare the strings \p left and \p right according to the user's preferred
/// locale. \return -1, 0, or 1 corresponding to whether \p left compares less
/// than, equal to, or greater than \p right.
int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right);

/// Format the given timestamp \p unixtimeMs according to the user's preferred
/// locale. Include date and time formatting corresponding to \p formatDate and
/// \p formatTime respectively. Assigns the result into \p buf.
void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf);

/// Options for case conversions: to uppercase or to lowercase.
enum class CaseConversion { ToUpper, ToLower };

/// Convert the string \p cs to the given \p targetCase, returning it in-place.
/// If \p useCurrentLocale is true, do this using the user's locale; otherwise
/// use a locale-independent conversion.
void convertToCase(
    llvh::SmallVectorImpl<char16_t> &cs,
    CaseConversion targetCase,
    bool useCurrentLocale);

/// Options for normalizing Unicode strings.
/// NOTE: If these change, then AndroidUnicodeUtils.java must be updated.
/// http://www.unicode.org/reports/tr15/
enum class NormalizationForm { C, D, KC, KD };

/// Normalize Unicode string \p buf into the given \p form, returning in place.
/// Use the normalization forms described in Technical Report #15.
/// http://www.unicode.org/reports/tr15/
void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form);

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H
