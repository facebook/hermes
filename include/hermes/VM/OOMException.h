/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_OOMEXCEPTION_H
#define HERMES_VM_OOMEXCEPTION_H

#include <exception>

namespace hermes {
namespace vm {

/// Exception thrown when Hermes runs out of space in the JS heap.  Although OOM
/// is a kind of runtime error, \c std::runtime_error owns the memory containing
/// its "what" message, and so will copy the message passed into its constructor
/// into newly allocated memory.  As OOMs can occur at times when memory is
/// constrained, we want to avoid allocations as much as possible, so inherit
/// from \c std::exception instead.
struct OOMException : public std::exception {
  OOMException() = default;

  const char *what() const noexcept override;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OOMEXCEPTION_H
