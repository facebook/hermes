/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPACTIONRESULT_INLINE_H
#define HERMES_VM_COMPACTIONRESULT_INLINE_H

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/CompactionResult.h"
#include "hermes/VM/GCGeneration.h"
#include "hermes/VM/GCSegmentRange-inline.h"

namespace hermes {
namespace vm {

CompactionResult::CompactionResult(
    GCSegmentRange::Ptr ogSegs,
    GCSegmentRange::Ptr ygSegs)
    : segmentRange_(
          GCSegmentRange::concat(std::move(ogSegs), std::move(ygSegs))) {
  auto *seg = segmentRange_->next();
  assert(seg && "Need at least one segment in range.");
  usedChunks_.emplace_back(seg);
}

inline CompactionResult::Chunk::Chunk(AlignedHeapSegment *segment)
    : level_(segment->start()),
      segment_(segment),
      generation_(segment->generation_) {
  assert(segment->generation_ != nullptr && "Must have an owning generation");
}

inline CompactionResult::Allocator CompactionResult::Chunk::allocator() {
  assert(
      !isExhausted() && "Cannot create an allocator for an exhausted chunk.");
  return Allocator(this);
}

template <AdviseUnused MU>
inline void CompactionResult::Chunk::recordLevel(
    AlignedHeapSegment *segment) const {
  assert(segment->generation_ == generation_ && "Owning generation mismatch.");
  segment->setLevel<MU>(level_);
}

inline GCGeneration *CompactionResult::Chunk::generation() const {
  return generation_;
}

#ifndef NDEBUG
inline void CompactionResult::Chunk::recordNumAllocated() const {
  generation_->incNumAllocatedObjects(numAllocated_);
}

inline bool CompactionResult::Chunk::isExhausted() const {
  return segment_ == nullptr;
}
#endif // !NDEBUG

inline CompactionResult::Allocator::Allocator(Chunk *chunk)
    : level_(chunk->level_),
      end_(chunk->segment_->end()),
      cardTable_(&chunk->segment_->cardTable()),
      boundary_(cardTable_->nextBoundary(level_)),
      chunk_(chunk) {}

inline CompactionResult::Allocator::~Allocator() {
  assert(
      chunk_->level_ == level_ &&
      "Allocator's level out of sync with its chunk");
}

inline AllocResult CompactionResult::Allocator::alloc(size_t sz) {
  char *next = level_ + sz;
  if (next > end_) {
#ifndef NDEBUG
    chunk_->segment_ = nullptr;
#endif
    return {nullptr, false};
  }

  if (boundary_.address() < next) {
    cardTable_->updateBoundaries(&boundary_, level_, next);
  }

  char *ptr = level_;
  level_ = next;

#ifndef NDEBUG
  chunk_->numAllocated_++;
#endif

  return {reinterpret_cast<GCCell *>(ptr), true};
}

inline void CompactionResult::Allocator::recordLevel() const {
  chunk_->level_ = level_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPACTIONRESULT_INLINE_H
