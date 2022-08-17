/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_ERRORHANDLING_H
#define HERMES_SUPPORT_ERRORHANDLING_H

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/ErrorHandling.h"

#include <cstdlib>
#include <system_error>

/// This macro is always enabled.  It exists to mark code that intended to
/// do some extra checking in the service of debugging some production crash;
/// if we figure out the problem, we might wish to delete them.
#define HERMES_EXTRA_DEBUG(x) x

namespace hermes {

/// Classifies types of errors that can cause "Out of Memory" (OOM).
enum class OOMError : int {
  // Zero is reserved by std::error_code to mean "no error".
  None = 0,
  // When there is no more room in the configured heap.
  // This happens when the number of bytes used by JS reaches the max
  // heap size.
  MaxHeapReached,
  // Used for cases where an upper limit was placed on how many allocations
  // could occur, but not by the operating system.
  MaxStorageReached,
  // When the heap isn't full, but it has been doing so much collection work
  // that it is a better experience to OOM instead of continuing.
  Effective,
  // When an allocation is requested that is larger than a heap segment.
  SuperSegmentAlloc,
  // When a CopyableVector's capacity overflows what an integer can hold.
  CopyableVectorCapacityIntegerOverflow,
  // This happens when a unit test attempts to allocate more storages
  // than a test limit.
  TestVMLimitReached,
};

const std::error_category &oom_category();
std::error_code make_error_code(OOMError err);
std::string convert_error_to_message(std::error_code code);

/// Reports a serious error and aborts execution. Calls any installed error
/// handlers (or defaults to print the message to stderr), then calls exit(1).
LLVM_ATTRIBUTE_NORETURN void hermes_fatal(const char *msg);
LLVM_ATTRIBUTE_NORETURN void hermes_fatal(const std::string &msg);
LLVM_ATTRIBUTE_NORETURN void hermes_fatal(
    llvh::StringRef prefix,
    std::error_code code);

} // namespace hermes

// GCC has a bug that requires the specialization to be inside the std
// namespace.
namespace std {
template <>
struct is_error_code_enum<hermes::OOMError> : public std::true_type {};
} // namespace std

#endif // HERMES_SUPPORT_ERRORHANDLING_H
