/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GENGCHEAPSEGMENT_H
#define HERMES_VM_GENGCHEAPSEGMENT_H

#include "hermes/VM/AlignedHeapSegment.h"

namespace hermes {
namespace vm {

class GenGCHeapSegment final : public AlignedHeapSegment {
  friend CompactionResult::Allocator;
  friend CompactionResult::Chunk;

 public:
  GenGCHeapSegment() = default;

  /// Construct a new GenGCHeapSegment. Its allocation region is initially
  /// empty, and must be grown to an appropriate size.
  ///
  /// \p storage The storage that this segment manages.
  /// \p owner The generation that owns this segment.
  GenGCHeapSegment(AlignedStorage &&storage, GCGeneration *owner = nullptr);

  GenGCHeapSegment(GenGCHeapSegment &&) = default;
  GenGCHeapSegment &operator=(GenGCHeapSegment &&) = default;

  /// Assumes marking is complete.  Scans the heap, determining, for each live
  /// object, the address to which it will later be compacted.  Objects are
  /// compacted into chunks, in the order they are provided by
  /// sweepResult->compactionResult.  The first object that doesn't fit in a
  /// chunk causes this chunk to be complete, and further allocation to occur in
  /// the next.  For each live object, installs a forwarding pointer to the
  /// post-compaction address in the VTable slot, after saving the VTable value
  /// in sweepResult->displacedVtablePtrs, which becomes the return result of
  /// the call.  Dead objects with finalizers have their finalizers executed.
  /// Sequences of dead objects have a DeadRegion containing a pointer to the
  /// next live object inserted, allowing them to be skipped efficiently n
  /// subsequent heap traversals.
  void sweepAndInstallForwardingPointers(GC *gc, SweepResult *sweepResult);

 private:
  /// Pointer to the generation that owns this segment.
  GCGeneration *generation_{nullptr};
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GENGCHEAPSEGMENT_H
