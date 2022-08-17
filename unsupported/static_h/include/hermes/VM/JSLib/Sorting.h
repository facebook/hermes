/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_SORTING_H
#define HERMES_VM_JSLIB_SORTING_H

#include <cstdint>

#include "hermes/VM/CallResult.h"

/// Defines custom sorting routines used in cases that we can't use std::sort.
/// std::sort doesn't always use std::swap, performing operations that bypass
/// the user-defined swap routines. When calling [[Put]] and [[Delete]], we
/// can't bypass those operations, so we use a custom SortModel and define our
/// own swap and less functions, which are used by these sorting functions.

namespace hermes {
namespace vm {

/// Abstraction to define a comparison-based sorting routine.  The
/// SortModel has two operations: swap and compare.  Note that some
/// implementation of \c swap and \c less can call property accessors
/// which evaluate JavaScript.  For now, we don't rename these
/// methods.
class SortModel {
 public:
  // Swap elements at indices a and b.
  virtual ExecutionStatus swap(uint32_t a, uint32_t b) = 0;

  // Compare elements at index a and at index b.
  // Return negative if [a] < [b], positive if [a] > [b], 0 if [a] = [b]
  virtual CallResult<int> compare(uint32_t a, uint32_t b) = 0;

  virtual ~SortModel() = 0;
};

/// QuickSort to sort elements in the range [begin, end). Returns immediately
/// with ExecutionStatus::EXCEPTION if any compare or swap operations fail.
ExecutionStatus quickSort(SortModel *sm, uint32_t begin, uint32_t end);

} // namespace vm
} // namespace hermes

#endif
