/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_DEPENDENTMEMORYREGION_H
#define HERMES_VM_DEPENDENTMEMORYREGION_H

#include <cassert>
#include <cstddef>

#include "hermes/VM/detail/BackingStorage.h"

namespace hermes {
namespace vm {

using namespace detail;

/// This represents a virtual address region (that is, address range),
/// of which a contiguous subrange is currently "in use."  The region's
/// overall size, and portion in use, is linked the size and in-use
/// portion of another, larger region, with a constant of
/// proportionality.  For example, we might allocated a maximum-size
/// virtual address range for a GC heap, and use some prefix of it at
/// any given time.  We might want other data structures that map to
/// the heap: a marking bitmap, a card table, etc.  Each of these
/// would have a different (power-of-two) constant of proportionality.
///
/// Note that the child region is conservative: it ensures that every
/// address on a page in the parent [start, end) corresponds to a
/// used address in the dependent region -- the page alignment may
/// make the dependent region larger than the "image" of the parent
/// region.
class DependentMemoryRegion {
 public:
  /// The limits here are those of the "parent" region on which this
  /// will depend.  They are required to be page-size-aligned.  The
  /// region created needs 1 byte for every 2^logSizeRatio bytes of
  /// the parent region.  Allocated a virtual memory region that
  /// covers the parent region with that ratio.  On supported
  /// platforms, the allocated region is annotated with \p name in the
  /// OS VM map.
  DependentMemoryRegion(
      const char *name,
      char *parentLowLim,
      char *parentStart,
      char *parentEnd,
      char *parentHiLim,
      size_t logSizeRatio);

  /// Frees the virtual memory region allocted in the constructor.
  virtual ~DependentMemoryRegion() = default;

  /// The used region of the parent has changed to the given [start,
  /// end) region.  Adjust the used region of the child region accordingly.
  /// If this is a decrease of the child used region in either
  /// direction, may declare previously used pages unused to the OS.
  virtual void resizeParentUsedRegion(char *start, char *end) {
    assert(parentLowLim_ <= start && start <= end && end <= parentHiLim_);
    parentStart_ = start;
    parentEnd_ = end;

    start_ = pageStartForParentPtr(parentStart_);
    end_ = pageEndForParentPtr(parentEnd_);
  }

  /// The parent region has moved to an entirely new location, \p delta bytes
  /// away from its current location.
  virtual void moveParentRegion(ptrdiff_t delta) {
    parentLowLim_ += delta;
    parentStart_ += delta;
    parentEnd_ += delta;
    parentHiLim_ += delta;
  }

  /// Reclaims the corresponding pages in the dependent region to the OS, when
  /// the parent region's pages are marked unused. This operation is
  /// conservative in that it will only mark a dependent page as unused if all
  /// addresses in its pre-image in the parent region are themselves unused.
  ///
  /// We assume that the parent region grows only on one side, which is up (and
  /// correspondingly shrinks down), so all addresses higher than \p to are
  /// already unused.
  ///
  /// \p from The start of the unused range in the parent (inclusive).
  /// \p to The end of the unused range in the parent (exclusive).
  ///
  /// \pre from <= to
  /// \pre [from, to) is entirely contained within the parent [start, end)
  ///     region.
  /// \pre The region [to, parentHiLim) is already unused.
  ///
  /// \post Every page in the dependent region whose preimage is wholly
  ///     contained in [from, parentHiLim) of the parent region, is returned to
  ///     the OS.
  void parentDidAdviseUnused(char *from, char *to);

  /// The bounds of the dependent region: it's low and hi limit, and the
  /// [start(), end()) used subregion.
  inline char *lowLim() const;
  inline char *start() const;
  inline char *end() const;
  inline char *hiLim() const;

  /// The size of the used region; that is, end() - start().
  inline unsigned used() const;

 protected:
  /// \p ptr A pointer into the parent region.
  ///
  /// \return A pointer to the beginning of the page in this region (inclusive)
  ///     containing the address corresponding to \p ptr in the parent.
  char *pageStartForParentPtr(char *ptr) const;

  /// \p ptr A pointer into the parent region.
  ///
  /// \return A pointer to the end of the page in this region (exclusive)
  ///     containing the address corresponding to \p ptr in the parent.
  char *pageEndForParentPtr(char *ptr) const;

  /// Return the maximum size of the dependent region, as a function
  /// of the parents hi and low limit, and the (base 2) log of the
  /// size ratio.  That is, (parentHiLim - parentLowLim) >>
  /// logSizeRatio, rounded up to a page boundary.
  static size_t
  maxSize(char *parentHiLim, char *parentLowLim, size_t logSizeRatio);

  /// The bounds of the parent region.
  char *parentLowLim_;
  char *parentStart_;
  char *parentEnd_;
  const char *parentHiLim_;

  /// The log (base 2) of the size ratio between the parent and
  /// child.  That is, there are 2^logSizeRatio_ bytes in the parent
  /// region for every byte in the dependent region.
  const size_t logSizeRatio_;

  /// Storage for this dependent region.
  BackingStorage storage_;

  /// The bounds of the child region.  These are also page-size
  /// aligned.  We ensure that every byte in the used region [parentStart_,
  /// parentEnd_) of the parent maps to a byte in the used region
  /// [start_, end_) of the child.  This, combined with page-size
  /// alignment, requires us to be conservative, rounding start_ down
  /// and end_ up as necessary.
  char *start_;
  char *end_;
};

inline char *DependentMemoryRegion::lowLim() const {
  return storage_.lowLim();
}
inline char *DependentMemoryRegion::start() const {
  return start_;
}
inline char *DependentMemoryRegion::end() const {
  return end_;
}
inline char *DependentMemoryRegion::hiLim() const {
  return storage_.hiLim();
}

inline unsigned DependentMemoryRegion::used() const {
  return end() - start();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_DEPENDENTMEMORYREGION_H
