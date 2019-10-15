/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPACTIONRESULT_H
#define HERMES_VM_COMPACTIONRESULT_H

#include "hermes/Support/ConsumableRange.h"
#include "hermes/VM/AdviseUnused.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/GCSegmentRange.h"

#include <iterator>
#include <vector>

namespace hermes {
namespace vm {

/// Forward declarations.
class AlignedHeapSegment;
class GCGeneration;

/// A CompactionResult is a region of memory, split over a number of contiguous
/// chunks, into which compaction is performed.
class CompactionResult {
 public:
  class Chunk;

  /// RAII class managing the allocation of objects into compaction result
  /// chunks. The class stores fields necessary for allocation inline, for fast
  /// updates, and commits their final values to the chunk upon its destruction.
  /// As a result, only one allocator can be alive at any one time, per chunk.
  class Allocator {
   public:
    Allocator(Chunk *chunk);

    /// Verifies that the allocator's level was written back in debug builds.
    ~Allocator();

    /// Attempt to allocate \p sz bytes at the beginning of the chunk.  Returns
    /// {nullptr, false} and marks the chunk as exhausted upon failure, and
    /// returns {ptr, true} otherwise, where ptr is the start of the \p sz byte
    /// allocation.
    AllocResult alloc(size_t sz);

    /// Write the level in the allocator back to the chunk it was created from.
    void recordLevel() const;

   private:
    char *level_;
    char *end_;
    CardTable *cardTable_;
    CardTable::Boundary boundary_;
    Chunk *chunk_;
  };

  /// An individual contiguous region that comprises part of the compaction
  /// result.  The level and end fields define the chunk.  When we compact a
  /// Segment into a Chunk, via sweepAndInstallForwardingPointers, we update
  /// level to point to the first byte after the last object compacted into the
  /// segment.  In debug builds, the numAllocated field is the number of objects
  /// compacted into this Chunk, and the exhausted field allows us to check that
  /// we're not compacting into chunks with no remaining space.
  class Chunk {
    friend Allocator;

   public:
    /// Create a chunk corresponding to the allocation region of \p segment.
    Chunk(AlignedHeapSegment *segment);

    /// \return An instance of allocator that allocates into this chunk.
    Allocator allocator();

    /// Write back the new level after compaction into \p segment.
    ///
    /// \p MU Indicate whether the newly freed pages should be returned to the
    ///     OS.
    ///
    /// \pre Assume \p segment is the up-to-date location of the segment this
    ///     Chunk was constructed with (it may have moved since that time).
    template <AdviseUnused MU = AdviseUnused::No>
    void recordLevel(AlignedHeapSegment *segment) const;

    /// \return A pointer to the generation this chunk was created from.
    GCGeneration *generation() const;

#ifndef NDEBUG
    /// Write back the number of objects that now reside in the segment, after
    /// compaction.
    void recordNumAllocated() const;

    /// \return Whether or not this chunk has been exhausted.
    bool isExhausted() const;
#endif // !NDEBUG

   private:
    char *level_;
    AlignedHeapSegment *segment_;
    GCGeneration *generation_;

#ifndef NDEBUG
    unsigned numAllocated_{0};
#endif // !NDEBUG
  };

  /// Type for incrementally processing the collection returned by usedChunks().
  using ChunksRemaining = ConsumableRange<std::vector<Chunk>::const_iterator>;

  /// Construct a new compaction result, with chunks first drawn from segments
  /// in \p ogSegs, and then \p ygSegs.  Assumes that there is at least one
  /// segment available across both the supplied ranges.
  inline CompactionResult(
      GCSegmentRange::Ptr ogSegs,
      GCSegmentRange::Ptr ygSegs);

  /// \pre this->hasChunk().
  ///
  /// \return a pointer to the chunk that we should currently be allocating
  ///     into.
  inline Chunk *activeChunk();

  /// Move the compaction result's cursor past the current chunk (if one
  /// exists).  If the next chunk exists, returns a pointer to it, otherwise
  /// returns nullptr.
  inline Chunk *nextChunk();

  /// \return a vector of the chunks that the compaction result's cursor has
  ///     been over.
  inline const std::vector<Chunk> &usedChunks() const;

 private:
  GCSegmentRange::Ptr segmentRange_;
  std::vector<Chunk> usedChunks_;
};

CompactionResult::Chunk *CompactionResult::activeChunk() {
  return &usedChunks_.back();
}

CompactionResult::Chunk *CompactionResult::nextChunk() {
  if (auto *seg = segmentRange_->next()) {
    usedChunks_.emplace_back(seg);
    return activeChunk();
  }

  return nullptr;
}

const std::vector<CompactionResult::Chunk> &CompactionResult::usedChunks()
    const {
  return usedChunks_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPACTIONRESULT_H
