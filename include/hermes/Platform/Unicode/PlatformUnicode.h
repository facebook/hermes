/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H
#define HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H

#include "hermes/Platform/Unicode/UnicodeCaseConversion.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"

/// Decide on which Unicode implementation to use.
#define HERMES_PLATFORM_UNICODE_JAVA 1
#define HERMES_PLATFORM_UNICODE_CF 2
#define HERMES_PLATFORM_UNICODE_EMSCRIPTEN 4
#define HERMES_PLATFORM_UNICODE_LITE 5
/// Self-contained implementation using Hermes' own generated Unicode tables,
/// with no dependency on a system library.
#define HERMES_PLATFORM_UNICODE_HERMES 6

#ifndef HERMES_PLATFORM_UNICODE
#if defined(__ANDROID__)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_JAVA
#elif defined(__APPLE__)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_CF
#elif defined(__EMSCRIPTEN__)
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_EMSCRIPTEN
#else
#define HERMES_PLATFORM_UNICODE HERMES_PLATFORM_UNICODE_HERMES
#endif
#endif

namespace hermes {
namespace platform_unicode {

/// Compare the strings \p left and \p right. \return -1, 0, or 1
/// corresponding to whether \p left compares less than, equal to, or greater
/// than \p right.
///
/// The ordering is backend-specific, and only some backends use the host
/// locale at all. The Hermes and LITE backends apply the DUCET root
/// collation of UTS #10 from a generated table, with non-ignorable variable
/// weighting and no identical level; they consult no locale, so a Swedish or
/// a Turkish host sorts exactly as any other. The CoreFoundation and Java
/// backends hand the strings to the platform collator under the host's
/// current locale, which is CLDR-tailored rather than plain DUCET. The
/// Emscripten backend calls the JavaScript engine's own
/// String.prototype.localeCompare, so it inherits whatever the host browser
/// or runtime does.
///
/// Every backend is a consistent comparison function, which is all ECMA-262
/// requires of String.prototype.localeCompare; none of them is a
/// locale-sensitive ordering callers may depend on.
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

/// Options for case conversions. Defined in UnicodeCaseConversion.h so that
/// the table-driven implementation does not depend on backend selection.
using CaseConversion = unicode::CaseConversion;

/// Convert the string \p cs to the given \p targetCase, returning it in-place.
/// If \p useCurrentLocale is true, do this using the user's locale; otherwise
/// use a locale-independent conversion.
void convertToCase(
    llvh::SmallVectorImpl<char16_t> &cs,
    CaseConversion targetCase,
    bool useCurrentLocale);

/// \return whether the host's current locale changes the result of case
/// conversion (e.g. a Turkish, Azerbaijani, or Lithuanian locale). The
/// Hermes backend computes this once per process and caches the result; the
/// CoreFoundation backend caches only the underlying locale object and
/// recomputes the result on every call; the Java backend queries the
/// current locale afresh on every call; the Emscripten and LITE backends
/// have no host locale source and always return a hardcoded false.
bool localeAffectsCasing();

/// Options for normalizing Unicode strings. Defined in
/// UnicodeNormalization.h so that the table-driven normalizer does not have to
/// depend on backend selection.
using NormalizationForm = unicode::NormalizationForm;

/// Normalize Unicode string \p buf into the given \p form, returning in place.
/// Use the normalization forms described in Technical Report #15.
/// http://www.unicode.org/reports/tr15/
void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form);

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_PLATFORMUNICODE_H
