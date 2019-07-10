/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SWEEPRESULT_H
#define HERMES_VM_SWEEPRESULT_H

#include "hermes/VM/VTable.h"

#include <vector>

namespace hermes {
namespace vm {

/// The result of a sweep of a space: the address that will be the new level_ of
/// the space after compaction, and the vector of displaced VTable pointers.  In
/// debug builds, also records the number of objects allocated in the space.
struct SweepResult {
  std::vector<const VTable *> displacedVtablePtrs;
  char *level{nullptr};
#ifndef NDEBUG
  unsigned numAllocated{0};
#endif

  SweepResult() = default;

  SweepResult(std::vector<const VTable *> &&displacedVtablePtrs)
      : displacedVtablePtrs(std::move(displacedVtablePtrs)) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SWEEPRESULT_H
