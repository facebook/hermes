/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GENGCHEAPSEGMENT_H
#define HERMES_VM_GENGCHEAPSEGMENT_H

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/CompleteMarkState.h"

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

  /// The given amount of external memory is credited to objects allocated in
  /// this space.  Adjust the effective size of the space accordingly.
  void creditExternalMemory(uint32_t size);

  /// The given amount of external memory is debited to objects allocated in
  /// this space.  Adjust the effective size of the space accordingly.
  void debitExternalMemory(uint32_t size);

  /// Assume that marking from the roots has marked some cells, and the
  /// markStack found in markState is empty.  Traverse the objects in the space,
  /// finding objects whose mark bits are set.  For each such object, push its
  /// address on the markStack, then enter a loop in which we pop objects from
  /// markStack while it is non-empty, and scan their pointers, marking unmarked
  /// referents, and pushing their addresses on markStack.  If we overflow the
  /// max size of markStack sets a flag and returns.
  void completeMarking(GenGC *gc, CompleteMarkState *markState);

  /// Assumes sweeping is complete.  Traverses the live objects, scanning their
  /// pointers.  For each pointer to another heap object, update the pointer by
  /// following the referent's forwarding pointer.  Marked cells are considered
  /// live, and it is assumed that the first N values in the range
  /// kindAndSizes are the KindAndSizes of the N remaining live cells in the
  /// segment. This function is expected to consume N elements from this range,
  /// denote that those pointers have been used and accounted for.
  void updateReferences(
      GenGC *gc,
      FullMSCUpdateAcceptor *acceptor,
      SweepResult::KindAndSizesRemaining &kindAndSizes);

  /// Assumes updateReferences is complete, there are N live cells in this
  /// segment, and that the first N pointers in [*vTableBegin, vTableEnd) are
  /// the VTable pointers of the N remaining live cells.  Moves each live cell
  /// to the post-compaction address indicated by its forwarding pointer and
  /// restores its displaced KindAndSize, consuming it in the process
  /// (indicated by incrementing *kindAndSizeBegin).
  void compact(
      SweepResult::KindAndSizesRemaining &kindAndSizes,
      PointerBase *base);

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
  void sweepAndInstallForwardingPointers(GenGC *gc, SweepResult *sweepResult);

  /// TODO (T25573911): the next two methods are usually debug-only; exposed
  /// in opt for temporary old-to-young pointer traversal.

  /// Call \p callback on each cell between \p low and \p high.
  void forObjsInRange(
      const std::function<void(GCCell *)> &callback,
      char *low,
      const char *high);
  void forObjsInRange(
      const std::function<void(const GCCell *)> &callback,
      const char *low,
      const char *high) const;

  /// Call \p callback on every cell allocated in this segment.
  void forAllObjs(const std::function<void(GCCell *)> &callback);
  void forAllObjs(const std::function<void(const GCCell *)> &callback) const;

  /// Adds a representation of segments address range to *\p buf,
  /// ensuring that we don't write more than \p sz characters.  Writes
  /// min(*sz, <length of string for seg>) characters.  Updates buf to
  /// point after the last character written, and decreases *\p sz by the
  /// number of chars written.
  void addExtentToString(char **buf, int *sz);

#ifdef HERMES_SLOW_DEBUG
  /// For debugging: iterates over objects, asserting that all GCCells have
  /// vtables with valid cell kinds, and that all object pointers point to
  /// GCCells whose vtables have valid cell kinds.  Sums the external memory
  /// rooted by objects in the space, and, if \p externalMemory is non-null,
  /// sets \p *externalMemory to that sum.
  void checkWellFormed(const GenGC *gc, uint64_t *externalMemory = nullptr)
      const;
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// Extra debugging: at the end of GC we "summarize" the vtable pointers of
  /// old-gen objects.  Conceptually, this means treat them as if they
  /// were concatenated, and take the hash of a string form.
  /// TODO(T56364255): remove these when the problem is diagnosed.

  /// Summarize the vtables of objects in the segment, and record the results
  /// (including the address after the last object summarized).
  void summarizeVTables();

  /// Resummarize the vtables of the old-gen objects, and return whether the
  /// result is the same as the last summary.
  bool checkSummarizedVTables() const;

  /// Summarize the vtables of objects in the segment, up to but not
  /// including \p level.
  size_t summarizeVTablesWork(const char *level) const;
#endif

 private:
  void deleteDeadObjectIDs(GenGC *gc);
  void updateObjectIDs(
      GenGC *gc,
      SweepResult::KindAndSizesRemaining &kindAndSizes);

  /// Pointer to the generation that owns this segment.
  GCGeneration *generation_{nullptr};
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GENGCHEAPSEGMENT_H
