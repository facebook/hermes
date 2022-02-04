/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_DEBUGHELPERS_H
#define HERMES_SUPPORT_DEBUGHELPERS_H 1

#include "llvh/Support/Format.h"

namespace hermes {

/// Returns an object which, if passed to an LLVM raw_ostream via the
/// << operator, will print the given \p ptr as a hex string.
inline llvh::FormattedNumber format_ptr(const void *ptr) {
  // Each byte has two hex digits, so the width is twice the sizeof of a
  // a pointer, plus 2 for the initial 0x.
  return llvh::format_hex(
      reinterpret_cast<uintptr_t>(ptr), sizeof(uintptr_t) * 2 + 2);
}

} // namespace hermes

#endif // HERMES_SUPPORT_DEBUGHELPERS_H
