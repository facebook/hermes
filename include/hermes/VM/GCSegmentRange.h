/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCSEGMENTRANGE_H
#define HERMES_VM_GCSEGMENTRANGE_H

#include "hermes/Support/ConsumableRange.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Compiler.h"

#include <memory>
#include <vector>

namespace hermes {
namespace vm {

class AlignedHeapSegment;

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

  /// Factory function for creating a fused range from any other type conforming
  /// to the interface described in the documentation of \c GCSegmentRange.
  /// Provided as a function for smoother template deduction.
  ///
  /// The resulting range behaves like the \p underlying one, until a call to
  /// \c next on \p underlying fails.  From then on, all subsequent calls to \c
  /// next() on the fused range will fail immediately, without delegating the
  /// request to \p underlying.
  static inline GCSegmentRange::Ptr fuse(GCSegmentRange::Ptr underlying);

  /// Factory function for constructing a concatenation of ranges from its
  /// component parts.
  ///
  /// \p ranges The sequence of component ranges, in the order we wish to
  ///     concatenate them in.
  ///
  /// TODO(T40821815) Consider removing this workaround when updating MSVC
  /// Note: std::unique_ptr<Ranges, std::default_delete<Ranges>> is equivalent
  /// to std::unique_ptr<Ranges>. Writing it the more verbose way is necessary
  /// to workaround a MSVC bug. MSVC otherwise emits a compiler error
  /// when processing the definition of this method (C2244: unable to match
  /// function definition to an existing declaration).
  template <typename... Ranges>
  static inline GCSegmentRange::Ptr concat(
      std::unique_ptr<Ranges, std::default_delete<Ranges>>... ranges);

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

  struct Fused;

  struct Concat;
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

/// Please refer to the documentation for GCSegmentRange::fuse for details on
/// this implementation's behaviour.
struct GCSegmentRange::Fused : public GCSegmentRange {
  /// Construct a new fused range from the \p delegate range.  Fused takes
  /// ownership of its delegate.
  inline Fused(GCSegmentRange::Ptr delegate);

  /// If calls to \c delegate_.next() have all succeeded so far, forwards the
  /// request for a segment to the delegate, otherwise fails immediately.
  inline AlignedHeapSegment *next() override;

 private:
  bool hasFailed_{false};
  GCSegmentRange::Ptr delegate_;
};

/// A range composed of the concatenation of a sequence of component ranges.
struct GCSegmentRange::Concat : public GCSegmentRange {
  using Spine = std::vector<GCSegmentRange::Ptr>;

  /// Construct a range from its component ranges.
  ///
  /// \p spine A vector containing the component ranges.
  inline Concat(Spine spine);

  /// See docs for \c GCSegmentRange.
  ///
  /// Let I be the index of the last range we successfully requested a segment
  /// from (initially 0).  The next segment of the concatenation of ranges
  /// R0, R1, ..., RN is the  next segment of the first range in the sequence
  /// RI, R(I + 1), ..., RN to return successfully from a call to \c next, or
  /// \c nullptr if all ranges were unsuccessful in producing a next segment.
  inline AlignedHeapSegment *next() override;

 private:
  Spine ranges_;
  Spine::iterator cursor_;
};

template <typename I>
inline GCSegmentRange::Consumable<I>::Consumable(ConsumableRange<I> consumable)
    : consumable_{std::move(consumable)} {}

template <typename I>
inline AlignedHeapSegment *GCSegmentRange::Consumable<I>::next() {
  return consumable_.hasNext() ? &consumable_.next() : nullptr;
}

inline GCSegmentRange::Fused::Fused(GCSegmentRange::Ptr delegate)
    : delegate_{std::move(delegate)} {}

inline AlignedHeapSegment *GCSegmentRange::Fused::next() {
  if (LLVM_UNLIKELY(hasFailed_))
    return nullptr;

  auto *res = delegate_->next();
  hasFailed_ = res == nullptr;

  return res;
}

inline GCSegmentRange::Concat::Concat(Spine ranges)
    : ranges_{std::move(ranges)}, cursor_{ranges_.begin()} {}

inline AlignedHeapSegment *GCSegmentRange::Concat::next() {
  while (LLVM_LIKELY(cursor_ != ranges_.end())) {
    if (AlignedHeapSegment *res = (*cursor_)->next()) {
      return res;
    } else {
      cursor_++;
    }
  }

  return nullptr;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSEGMENTRANGE_H
