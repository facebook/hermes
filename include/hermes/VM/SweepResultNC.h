/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SWEEPRESULTNC_H
#define HERMES_VM_SWEEPRESULTNC_H

#include "hermes/ADT/ConsumableRange.h"
#include "hermes/VM/CompactionResult.h"
#include "hermes/VM/VTable.h"

#include <vector>

namespace hermes {
namespace vm {

/// Holds the state of a collection's sweep phase over all segments managed by
/// the GC.
struct SweepResult {
  /// Type used for incrementally processing the displaced KindAndSize values.
  using KindAndSizesRemaining =
      ConsumableRange<std::vector<KindAndSize>::const_iterator>;

  /// The VTable pointers that were displaced in order to write forwarding
  /// pointers, in the order they were displaced.
  std::vector<KindAndSize> displacedKinds;

  /// An abstraction over the space available to compact into, as well as how
  /// much to use, and the next address to compact into.
  CompactionResult compactionResult;

  SweepResult(CompactionResult compactionResult)
      : compactionResult(std::move(compactionResult)) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SWEEPRESULTNC_H
