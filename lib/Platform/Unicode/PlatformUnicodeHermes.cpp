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

namespace hermes {
namespace platform_unicode {

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

// TODO(icu-removal): implement natively and drop the ICU forwarding.
void convertToCase(
    llvh::SmallVectorImpl<char16_t> &cs,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  icu_impl::convertToCase(cs, targetCase, useCurrentLocale);
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  unicode::normalizeUTF16(buf, form);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_HERMES
