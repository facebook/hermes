/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_HERMES

#include "hermes/Platform/Unicode/PlatformUnicodeICUImpl.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include <cctype>
#include <cstdlib>
#include <string>

namespace hermes {
namespace platform_unicode {

namespace {

/// \return the casing locale implied by the host environment.
///
/// This replicates what ICU's uloc_getDefault does, as measured against the
/// ICU build: LC_ALL, then LC_MESSAGES, then LANG, first non-empty wins.
/// LC_CTYPE is deliberately NOT consulted even though POSIX would suggest it
/// for a character-handling category, because ICU ignores it and diverging
/// would silently change behavior for anyone who sets it.
/// _HERMES_TEST_LOCALE takes precedence over all of them, matching
/// PlatformUnicodeCF.cpp.
unicode::CaseLocale computeHostCaseLocale() {
  static const char *const kVars[] = {
      "_HERMES_TEST_LOCALE", "LC_ALL", "LC_MESSAGES", "LANG"};
  for (const char *var : kVars) {
    const char *value = std::getenv(var);
    if (!value || !*value)
      continue;
    // The language subtag is the text before the first separator, e.g. the
    // "tr" of "tr_TR.UTF-8", "tr-TR" or "tr".
    std::string lang;
    for (const char *p = value;
         *p && *p != '_' && *p != '-' && *p != '.' && *p != '@';
         ++p)
      lang.push_back((char)std::tolower((unsigned char)*p));
    if (lang == "tr" || lang == "az")
      return unicode::CaseLocale::Turkish;
    if (lang == "lt")
      return unicode::CaseLocale::Lithuanian;
    return unicode::CaseLocale::Root;
  }
  return unicode::CaseLocale::Root;
}

/// The host casing locale, computed once. It cannot change within a process.
unicode::CaseLocale hostCaseLocale() {
  static const unicode::CaseLocale locale = computeHostCaseLocale();
  return locale;
}

} // namespace

// TODO(icu-removal): implement natively and drop the ICU forwarding.
int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  return icu_impl::localeCompare(left, right);
}

// TODO(icu-removal): implement natively and drop the ICU forwarding.
void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  icu_impl::dateFormat(unixtimeMs, formatDate, formatTime, buf);
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &cs,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  unicode::convertCaseUTF16(
      cs,
      targetCase,
      useCurrentLocale ? hostCaseLocale() : unicode::CaseLocale::Root);
}

bool localeAffectsCasing() {
  return hostCaseLocale() != unicode::CaseLocale::Root;
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  unicode::normalizeUTF16(buf, form);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_HERMES
