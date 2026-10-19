/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_PLATFORMUNICODEICUIMPL_H
#define HERMES_PLATFORMUNICODE_PLATFORMUNICODEICUIMPL_H

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU || \
    HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_HERMES

namespace hermes {
namespace platform_unicode {
/// The ICU-backed implementations, exposed so that the Hermes backend can
/// forward the functions it has not replaced with native ones yet.
/// TODO(icu-removal): delete this header, and the forwarding it enables, once
/// all four functions are implemented natively.
namespace icu_impl {

/// See platform_unicode::localeCompare.
int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right);

/// See platform_unicode::dateFormat.
void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf);

/// See platform_unicode::convertToCase.
void convertToCase(
    llvh::SmallVectorImpl<char16_t> &cs,
    CaseConversion targetCase,
    bool useCurrentLocale);

/// See platform_unicode::normalize.
void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form);

} // namespace icu_impl
} // namespace platform_unicode
} // namespace hermes

#endif // ICU or HERMES

#endif // HERMES_PLATFORMUNICODE_PLATFORMUNICODEICUIMPL_H
