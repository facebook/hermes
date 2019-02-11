/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCSEGMENTRANGE_H
#define HERMES_VM_GCSEGMENTRANGE_H

#include "hermes/Support/ConsumableRange.h"
#include "hermes/VM/AlignedHeapSegment.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Compiler.h"

#include <memory>

namespace hermes {
namespace vm {

/// An interface representing a sequence of segments, and a collection of
/// implementations and factory functions for combining and manipulating them.
struct GCSegmentRange {
  using Ptr = std::unique_ptr<GCSegmentRange>;

  /// Factory function for creating a range from an iterator ranging over a
  /// sequence of AlignedHeapSegments. Provided for convenenience as it offers
  /// better template deduction compared to the raw constructor invocation.
  ///
  /// \p begin The iterator pointing at the beginning of the sequence.
  /// \p end The iterator pointing at the end of the sequence (one past the last
  ///     segment in the sequence).
  template <typename I>
  static inline GCSegmentRange::Ptr fromConsumable(I begin, I end);

  /// Factory function for creating a range over a single \c AlignedHeapSegment.
  ///
  /// \p seg Pointer to the segment the range will refer to.
  static inline GCSegmentRange::Ptr singleton(AlignedHeapSegment *seg);

  virtual ~GCSegmentRange() = default;

  /// Returns a pointer to the next segment, moving the underlying cursor
  /// past the next segment upon success, and returns \c nullptr, leaving the
  /// underlying cursor unchanged, on failure.
  ///
  /// There is no guarantee on the lifetime of the segment pointed to by the
  /// return value, and certain implementations may experience undefined
  /// behaviour if the underlying storage is mutated whilst a range is being
  /// consumed.
  virtual AlignedHeapSegment *next() = 0;

 private:
  template <typename I>
  struct Consumable;
};

/// A range backed by a \c ConsumableRange of AlignedHeapSegments.
///
/// \p I A type representing an iterator whose value_type is expected to be
///     AlignedHeapSegment.
template <typename I>
struct GCSegmentRange::Consumable : public GCSegmentRange {
  /// Constructs a new range.
  ///
  /// \p consumable The underlying ConsumableRange of AlignedHeapSegments.
  inline Consumable(ConsumableRange<I> consumable);

  /// See docs for \c GCSegmentRange.
  ///
  /// This range will exhibit undefined behaviour if the iterators backing its
  /// underlying iterator are invalidated.
  inline AlignedHeapSegment *next() override;

 private:
  ConsumableRange<I> consumable_;
};

template <typename I>
inline GCSegmentRange::Ptr GCSegmentRange::fromConsumable(I begin, I end) {
  return std::unique_ptr<Consumable<I>>(new Consumable<I>{{begin, end}});
}

inline GCSegmentRange::Ptr GCSegmentRange::singleton(AlignedHeapSegment *seg) {
  return fromConsumable(seg, seg + 1);
}

template <typename I>
inline GCSegmentRange::Consumable<I>::Consumable(ConsumableRange<I> consumable)
    : consumable_{std::move(consumable)} {}

template <typename I>
inline AlignedHeapSegment *GCSegmentRange::Consumable<I>::next() {
  return consumable_.hasNext() ? &consumable_.next() : nullptr;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSEGMENTRANGE_H
