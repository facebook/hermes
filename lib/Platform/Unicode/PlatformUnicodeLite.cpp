/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_LITE

#include "hermes/Platform/Unicode/UnicodeCaseConversion.h"
#include "hermes/Platform/Unicode/UnicodeCollation.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"

namespace hermes {
namespace platform_unicode {

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  return unicode::compareUTF16(left, right);
}

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  // FIXME: implement this.
  llvh::ArrayRef<char> str{"dateFormat not implemented"};
  buf.assign(str.begin(), str.end());
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  // Root is deliberate: LITE must not depend on system state.
  unicode::convertCaseUTF16(buf, targetCase, unicode::CaseLocale::Root);
}

bool localeAffectsCasing() {
  // LITE never uses the host locale for casing.
  return false;
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  unicode::normalizeUTF16(buf, form);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_LITE
