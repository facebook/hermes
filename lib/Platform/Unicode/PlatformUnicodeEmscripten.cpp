/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_EMSCRIPTEN

#include "hermes/Platform/Unicode/UnicodeCaseConversion.h"
#include "hermes/Platform/Unicode/UnicodeNormalization.h"

#include <emscripten.h>

namespace hermes {
namespace platform_unicode {

// clang-format off

EM_JS(int, js_platform_unicode_localeCompare, (
    const char16_t *a, size_t aLen, const char16_t *b, size_t bLen), {
  function copyStr(p, len) {
    var res = "";
    p >>>= 1;
    for (; len > 0; --len, ++p) {
      res += String.fromCharCode(HEAPU16[p])
    }
    return res;
  }
  var strA = copyStr(a, aLen);
  var strB = copyStr(b, bLen);
  return strA.localeCompare(strB);
});

// clang-format on

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  return js_platform_unicode_localeCompare(
      left.data(), left.size(), right.data(), right.size());
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
    llvh::SmallVectorImpl<char16_t> &str,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  // Root is deliberate: there is no locale source on this platform yet.
  unicode::convertCaseUTF16(str, targetCase, unicode::CaseLocale::Root);
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  unicode::normalizeUTF16(buf, form);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_JAVA
