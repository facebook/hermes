/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ALLOCRESULT_H
#define HERMES_VM_ALLOCRESULT_H

#include "hermes/VM/GCCell.h"

namespace hermes {
namespace vm {

/// The result of an allocation.  It turns out to be hard to
/// communicate the invariant that successful allocations yield
/// non-null results to the C++ compiler.  So, instead, we use this
/// structure to make success/failure explicit.  We will only ever use
/// constant values for success, so it should be optimized away in
/// inline function.  So all functions that return it should be
/// inlined, or not performance-sensitive.
struct AllocResult {
  void *ptr;
  bool success;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_ALLOCRESULT_H
