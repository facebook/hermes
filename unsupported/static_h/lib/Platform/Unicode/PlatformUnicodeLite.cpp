/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_LITE

namespace hermes {
namespace platform_unicode {

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  for (size_t i = 0; i < left.size(); i++) {
    if (i > right.size()) {
      return 1;
    }
    if (left[i] > right[i]) {
      return 1;
    } else if (left[i] < right[i]) {
      return -1;
    }
  }
  return left.size() < right.size() ? -1 : 0;
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
  // FIXME: implement this.
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  // FIXME: implement this.
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_LITE
