/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_EXPECTEDPAGESIZE_H
#define HERMES_VM_EXPECTEDPAGESIZE_H

#include "hermes/Support/OSCompat.h"

#include <cstddef>

namespace hermes {
namespace vm {
namespace pagesize {

/// A constexpr value that is highly likely (but not absolutely
/// guaranteed) to be at least as large, and a multiple of,
/// oscompat::page_size().  Thus, this value can be used to compute
/// page-aligned sizes statically.  However, operations that require
/// actual page_size()-alignment for correctness (e.g., ensuring that
/// mprotect only protects what is intended) should verify the
/// correctness of this value, using the function below.
constexpr size_t kExpectedPageSize =
#if defined(__APPLE__)
    16 * 1024
#else
    4 * 1024
#endif
    ;

/// We will try to keep kExpectedPageSize accurate for all platforms...but
/// we may fail.  This method returns whether kExpectedPageSize is a
/// multiple of the actual page size.
inline bool expectedPageSizeIsSafe() {
  return oscompat::page_size() <= kExpectedPageSize &&
      (kExpectedPageSize % oscompat::page_size()) == 0;
}

} // namespace pagesize
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_EXPECTEDPAGESIZE_H
