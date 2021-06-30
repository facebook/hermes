/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCGENERATION_H
#define HERMES_VM_GCGENERATION_H

#include "hermes/VM/AllocOptions.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GenGCHeapSegment.h"

#include <vector>

namespace hermes {
namespace vm {

class GenGC;

/// A class to hold the shared behaviour of the young and old generations of the
/// non-contiguous generational collector.  Sub-classes are expected to
/// conform to the following interface:
///
/// @name Allocation
/// @{
///
/// We assume that most allocation in a GCGeneration is performed directly in
/// the generation's active segment, often with the GC having claimed that
/// segment's AllocContext.
///
/// Assumes allocation in the generation's active segment has failed,
/// and that the generation's AllocContext has been yielded back if it
/// had been claimed.  Allocate a new \p size byte cell, performing a
/// collection if necessary, potentially moving allocated objects.
/// Returns {cell, true} upon success, and if \p hasFinalizer is
/// HasFinalizer::Yes, appends cell to the generation's finalizer
/// list.  Leaves the state of the generation as it was before the
/// call and throws an Out of Memory (OOM) exception upon failure.
/// This operation can fail if the heap cannot be grown to accommodate
/// the new allocation along with all the currently live objects.
///   AllocResult allocSlow(size_t size, HasFinalizer hasFinalizer);
///
/// Allocate a new \p size byte cell.  This operation will not move any existing
/// allocations in order to fulfill the request.  Returns {cell, true} upon
/// success, and if \p hasFinalizer is HasFinalizer::Yes, appends cell to the
/// generation's finalizer list.  Leaves the state of the generation as it was
/// before the cell and returns {nullptr, false} upon failure.  This operation
/// can fail if the current size of the generation's allocation region is
/// insufficient to fit the new allocation along with all the currently
/// allocated objects in this generation.
///   AllocResult allocRaw(size_t size, HasFinalizer hasFinalizer);
///
/// @}
///
/// @name Size Statistics
/// @{
///
/// Returns the size in bytes of the allocation region in this
/// generation (not necessarily contiguous).  The first version may be
/// used always; the sizeDirect version may only be used when the
/// generation owns its allocation context, but is faster.
///   size_t size() const;
///   size_t sizeDirect() const;
///
/// Returns the number of bytes in the generation that are currently
/// allocated.  The first version may be used always; the usedDirect
/// version may only be used when the generation owns its allocation
/// context, but is faster.
///   size_t used() const;
///   size_t usedDirect() const;
///
/// Returns the number of bytes in the generation that are available for
/// allocation.  The first version may be used always; the availableDirect
/// version may only be used when the generation owns its allocation
/// context, but is faster.
///   size_t available() const;
///   size_t availableDirect() const;
///
/// The size() minus external memory, but zero if external memory is larger
/// than size().
///   size_t effectiveSize();
///
/// It must also have the inner type:
///   class Size;
/// Which provides at least these functions publicly:
///   Constructor.
///     Size(gcheapsize_t min, gcheapsize_t max);
///   Return the minimum amount of bytes holdable by this generation.
///     gcheapsize_t min() const;
///   Return the maximum amount of bytes holdable by this generation.
///     gcheapsize_t max() const;
///   Return the amount of space this generation will take for a given \p amount
///   of bytes. It will clamp the size based on the min and max size of the
///   generation, and also make sure it is aligned correctly.
///     gcheapsize_t adjustSize(gcheapsize_t amount) const;
/// @}
///
/// @name Growth
/// @{
///
/// Given a \p desired size of the generation in bytes, returns a size that this
/// generation can grow to. The desired size can be either increased or
/// decreased based on the requirements of the generation (e.g. whether it has a
/// minimum or maximum size, or an alignment requirement).  The adjustment is
/// bounded above by an additive constant, and idempotent.  I.e. for sub-classes
/// of GCGeneration, there is a constant C such that for all desired sizes:
///
///     adjustSize(adjustSize(desired)) == adjustSize(desired)
///     adjustSize(desired) - desired < C
///
///   size_t adjustSize(size_t desired) const;
///
/// Increases the size of the generation by the minimum amount such that
/// this->size() >= size.  This function assumes that \p size has been adjusted,
/// i.e. this->adjustSize(size) == size.
///   void growTo(size_t size);
///
/// Decreases the size of the generation by the minimum amount such that
/// this->size() <= size.  This function assumes that \p size has been adjusted,
/// i.e. this->adjustSize(size) == size and that size >= used().
///   void shrinkTo(size_t size);
///
/// Ensure that the generation has enough available space to accommodate an \p
/// amount byte allocation, by potentially increasing its size. return true if
/// and only if a call to this->allocRaw(amount) will succeed immediately
/// following this call, and return false whilst leaving the bounds of the
/// generation unchanged otherwise.
///   bool growToFit(size_t amount);
///
/// Determine where the level should be after compaction given a range of
/// chunks that were used during compaction:
///
/// \pre A prefix of usedChunks corresponds to a prefix of the segments owned
///     by this generation. That is to say, there is an N s.t.
///
///     forall 0 <= j < N.
///       usedChunks[j].generation() == this
///     and forall N <= k < |usedChunks|.
///       usedChunks[k].generation() != this
///
/// \post N chunks have been consumed.
///   void recordLevelAfterCompaction(
///       CompactionResult::ChunksRemaining &usedChunks);
/// @}
///
/// @name Segment Iteration
/// @{
///
/// Call \p callback on the segments that hold the first min(size(), used() + 1)
/// bytes of the generation, in the order they were allocated into (That is, the
/// filled segments followed by the active segment).  Template parameter \p F
/// is a callable with void return type, accepting a reference to an \c
/// GenGCHeapSegment.  The const variant accepts a const reference.
///   template <typename F> inline void forUsedSegments(F callback);
///   template <typename F> inline void forUsedSegments(F callback) const;
///
/// Like \c forUsedSegments, but supporting early termination.  The callback
/// returns a bool, which indicates whether iteration should continue.  The
/// function returns true if and only if every invocation of the callback
/// returned true.
///   template <typename F> inline bool whileUsedSegments(F callback);
///   template <typename F> inline bool whileUsedSegments(F callback) const;
///
/// A sub-class, Gen, must have an associated template specialisation of
/// SegTraits.
///
/// The sequence of segments that represent the generation's current allocation
/// region (bytes in the generation at offset).
///   SegTraits<Gen>::Range allSegments();
///
/// @}
///
/// @name Sweeping and Referencing
/// @{
///
/// Calls sweeps and installs forwarding pointers in all the segments in the
/// generation.
///   void sweepAndInstallForwardingPointers(GC *gc, SweepResult *sweepResult);
///
/// Assumes sweeping is complete.  Traverses the live objects in the
/// generation, scanning their pointers.  For each pointer to a heap object,
/// update the pointer by following the referent's forwarding pointer.
///   void updateReferences(GC *gc, SweepResult::KindAndSizesRemaining
///   &kindAndSizes);
/// @}
///
/// @name Debug
/// @{
///
/// Return the bytes allocated since the last garbage collection.
///   gcheapsize_t bytesAllocatedSinceLastGC() const;
///
/// TODO (T26749007) Deprecate this API in favour of the segment iterator API.
/// Calls \p callback for every cell in memory managed by this generation.
///   void forAllObjs(const std::function<void(GCCell *)> &callback) const;
///
/// -UNDEBUG only:
///
/// Return true if and only if the pointer is within the current bounds of this
/// generation.
///   bool dbgContains(const void *p) const;
///
/// Call \p callback on each cell allocated since the last garbage collection.
///   void forObjsAllocatedSinceGC(const std::function<void(GCCell *)>
///   &callback);
///
/// For debugging: iterates over objects, asserting that all GCCells
/// have vtables with valid cell kinds, and that all object pointers
/// point to GCCells whose vtables have valid cell kinds.
///   void checkWellFormed(const GC *gc) const;
///
/// @}
class GCGeneration {
 public:
  /// The data structures necessary for allocation in a GCGeneration: an active
  /// segment, the list of objects with finalizers allocated in the
  /// generation, and, in debug, the number of allocated objects.
  struct AllocContext {
    /// The segment that calls to GCGeneration::allocRaw currently allocate
    /// into.
    GenGCHeapSegment activeSegment;

    /// A list of all the cells in this generation with a finalizer.
    std::vector<GCCell *> cellsWithFinalizers;

#ifndef NDEBUG
    /// @name Debug-only statistics
    /// @{

    /// Number of allocated objects.
    unsigned numAllocatedObjects{0};
    /// @}
#endif

    /// Attempts to allocate a new \p sz byte cell.  Returns
    /// {<newly-allocated cell>, true} upon success, and if \p
    /// hasFinalizer is HasFinalizer::Yes, appends the new cell to the
    /// generation's finalizer list.  Returns {nullptr, false} upon
    /// failure.
    inline AllocResult alloc(uint32_t sz, HasFinalizer hasFinalizer);

    /// Swap the two AllocContexts.
    friend void swap(AllocContext &a, AllocContext &b);

    /// True iff this context is valid -- it can be allocated into.
    inline explicit operator bool() const;
  };

  /// Create a new GC Generation.
  ///
  /// \p gc Pointer to the GC of which this is a part.
  GCGeneration(GenGC *gc);

  /// Default virtual destructor.
  virtual ~GCGeneration();

  /// Callback that is called on generations at the end of a GC.  Defaults to
  /// doing nothing.
  inline void didFinishGC() {}

  /// Registers \p cell as having a finalizer that must be called when it is
  /// collected.
  inline void addToFinalizerList(void *cell);

  /// Call the finalizer methods on cells that are not marked.
  void finalizeUnreachableObjects();

  /// Update the pointers in the finalizer list according to the forwarding
  /// pointers in the cells pointed to (assuming such forwarding pointers have
  /// been installed).
  void updateFinalizableCellListReferences();

  /// The number of finalized objects in the space.
  inline unsigned numFinalizedObjects() const;

  /// The amount of external memory credited to objects allocated in this
  /// generation.
  inline uint64_t externalMemory() const;

  /// Increase the external memory credited to the generation by the given \p
  /// size.
  inline void creditExternalMemory(size_t size);

  /// Decrease the external memory charged to the generation by the given \p
  /// size.
  inline void debitExternalMemory(size_t size);

  /// The external memory charge of the generation may have changed.  Update the
  /// effective size of the generation to reflect this.
  void updateEffectiveEndForExternalMemory();

  /// Assumes objects with associated external memory are on the generation's
  /// finalizer list.  Iterates over that list, summing and returning the
  /// external memory.
  uint64_t extSizeFromFinalizerList() const;

  /// Assumes objects with associated malloc memory are on the generation's
  /// finalizer list (GCCell asserts this). Iterates over that list, summing
  /// and returning the malloc memory.
  uint64_t mallocSizeFromFinalizerList() const;

  /// For each unmarked HiddenClass, reset its property map if possible.
  void clearUnmarkedPropertyMaps();

  /// Returns a reference to the generation's allocation context.
  inline AllocContext &allocContext();
  inline const AllocContext &allocContext() const;

  /// When/if the allocContext_ is borrowed by an owning GC, this
  /// allows that owner to set this generation's trueAllocContext to
  /// \p movedContext.
  inline void setTrueAllocContext(GCGeneration::AllocContext *movedContext);

  /// When/if the allocContext__ is restored by an owning GC, this
  /// allows that owner to reset the trueAllocContext to the address
  /// of this generation's alloc context.
  inline void resetTrueAllocContext();

  /// Returns whether the GCGeneration currently owns its
  /// allocContext (or whether it has been temporarily claimed by the GC).
  inline bool ownsAllocContext() const;

#ifdef HERMES_SLOW_DEBUG
  /// Checks that the generation's finalizable object list is well formed:
  /// each entry points to a valid object, and all objects are within the
  /// current generation.  Requires \p isInGen to provide that predicate.
  void checkFinalizableObjectsListWellFormed() const;

  /// Check the well-formedness of the generation.
  void checkWellFormed(const GC *gc) const;
#endif

#ifndef NDEBUG
  /// Returns true if \p p points into the generation.
  /// Note: the "dbg" part of the name emphasizes that this can be expensive.
  virtual bool dbgContains(const void *p) const = 0;

  // Accessors for debug only statistics
  inline unsigned numAllocatedObjects() const;
  inline unsigned numReachableObjects() const;
  inline unsigned numHiddenClasses() const;
  inline unsigned numLeafHiddenClasses() const;

  // Mutators for debug only statistics

  // Adds \p val to the statistic.
  inline void incNumAllocatedObjects(unsigned val = 1);
  inline void incNumReachableObjects(unsigned val = 1);
  inline void incNumHiddenClasses(unsigned val = 1);
  inline void incNumLeafHiddenClasses(unsigned val = 1);

  // Sets the statistic's value to 0.
  inline void resetNumAllocatedObjects();
  inline void resetNumReachableObjects();
  inline void resetNumAllHiddenClasses();
#endif // !NDEBUG

 protected:
  /// Implementation of allocRaw (see class doc comment) that allocates into the
  /// generation's active segment.
  inline AllocResult allocRaw(uint32_t sz, HasFinalizer hasFinalizer);

  /// Replace the current active segment with a new one, signaling to the GC
  /// that both the previous and current active segments have moved.
  ///
  /// \p newSeg The new segment to be made active.
  /// \p oldSegSlot A pointer to the location where we should store the
  ///     previously active segment.  This parameter is optional, and defaults
  ///     to \c nullptr, which indicates that the previously active segment
  ///     should be discarded.
  void exchangeActiveSegment(
      GenGCHeapSegment &&newSeg,
      GenGCHeapSegment *oldSegSlot = nullptr);

  /// The segment that calls to GCGeneration::allocRaw currently allocate into.
  /// These versions require that the generation own the allocation context.
  inline GenGCHeapSegment &activeSegment();
  inline const GenGCHeapSegment &activeSegment() const;
  /// These versions do require that the generation own the allocation context.
  inline GenGCHeapSegment &activeSegmentRaw();
  inline const GenGCHeapSegment &activeSegmentRaw() const;
  /// This gets the generation's active segment, even if it has been claimed
  /// by the GC.
  inline GenGCHeapSegment &trueActiveSegment();
  inline const GenGCHeapSegment &trueActiveSegment() const;

  /// A list of all the cells in this generation with a finalizer.
  inline std::vector<GCCell *> &cellsWithFinalizers();
  inline const std::vector<GCCell *> &cellsWithFinalizers() const;

 protected:
  /// A pointer to the active AllocContext: either &allocContext_, or,
  /// if the allocContext_ has been "claimed" by the containing GC, a
  /// pointer to the current location of that context structure in the
  /// GC.  This claiming leaves the GCGeneration with an invalid
  /// allocContext.  Initialized in ctor to &allocContext_.
  AllocContext *trueAllocContext_;

  /// The GenGC of which this is a part.
  GenGC *gc_;

  // The amount (in bytes) of external memory credited to objects in this
  // generation.
  uint64_t externalMemory_{0};

  // Statistics:
  //   The number of finalizable objects finalized while they were in this
  //   generation.
  unsigned numFinalizedObjects_{0};

#ifndef NDEBUG
  // Debug only statistics
  unsigned numReachableObjects_{0};
  unsigned numHiddenClasses_{0};
  unsigned numLeafHiddenClasses_{0};
#endif

 private:
  /// The AllocContext for the generation.
  AllocContext allocContext_;
};

inline GenGCHeapSegment &GCGeneration::activeSegment() {
  assert(ownsAllocContext());
  return allocContext_.activeSegment;
}

inline const GenGCHeapSegment &GCGeneration::activeSegment() const {
  assert(ownsAllocContext());
  return allocContext_.activeSegment;
}

inline GenGCHeapSegment &GCGeneration::activeSegmentRaw() {
  return allocContext_.activeSegment;
}

inline const GenGCHeapSegment &GCGeneration::activeSegmentRaw() const {
  return allocContext_.activeSegment;
}

inline GenGCHeapSegment &GCGeneration::trueActiveSegment() {
  return trueAllocContext_->activeSegment;
}

inline const GenGCHeapSegment &GCGeneration::trueActiveSegment() const {
  return trueAllocContext_->activeSegment;
}

inline std::vector<GCCell *> &GCGeneration::cellsWithFinalizers() {
  return allocContext_.cellsWithFinalizers;
}

inline const std::vector<GCCell *> &GCGeneration::cellsWithFinalizers() const {
  return allocContext_.cellsWithFinalizers;
}

inline void GCGeneration::addToFinalizerList(void *cell) {
  // We assume here that the cell will be initialized, and become a
  // GCCell, soon after this call, before it is retrieved from
  // cellsWithFinalizers_.  Therefore, we cast here to the eventual type.
  cellsWithFinalizers().push_back(reinterpret_cast<GCCell *>(cell));
}

unsigned GCGeneration::numFinalizedObjects() const {
  return numFinalizedObjects_;
}

uint64_t GCGeneration::externalMemory() const {
  return externalMemory_;
}

void GCGeneration::creditExternalMemory(size_t size) {
  externalMemory_ += size;
}

void GCGeneration::debitExternalMemory(size_t size) {
  assert(externalMemory_ >= size);
  externalMemory_ -= size;
}

inline GCGeneration::AllocContext &GCGeneration::allocContext() {
  return allocContext_;
}
inline const GCGeneration::AllocContext &GCGeneration::allocContext() const {
  return allocContext_;
}

inline void GCGeneration::setTrueAllocContext(
    GCGeneration::AllocContext *movedContext) {
  trueAllocContext_ = movedContext;
}

inline void GCGeneration::resetTrueAllocContext() {
  trueAllocContext_ = &allocContext_;
}

inline bool GCGeneration::ownsAllocContext() const {
  // If the allocContext_ has been claimed, a null active segment,
  // which coerces to false, will be swapped in.  A valid segment
  // casts to true.
  return !!allocContext_;
}

#ifndef NDEBUG
unsigned GCGeneration::numAllocatedObjects() const {
  return allocContext_.numAllocatedObjects;
}

unsigned GCGeneration::numReachableObjects() const {
  return numReachableObjects_;
}

unsigned GCGeneration::numHiddenClasses() const {
  return numHiddenClasses_;
}

unsigned GCGeneration::numLeafHiddenClasses() const {
  return numLeafHiddenClasses_;
}

void GCGeneration::incNumAllocatedObjects(unsigned val) {
  allocContext_.numAllocatedObjects += val;
}

void GCGeneration::incNumReachableObjects(unsigned val) {
  numReachableObjects_ += val;
}

void GCGeneration::incNumHiddenClasses(unsigned val) {
  numHiddenClasses_ += val;
}

void GCGeneration::incNumLeafHiddenClasses(unsigned val) {
  numLeafHiddenClasses_ += val;
}

void GCGeneration::resetNumAllocatedObjects() {
  allocContext_.numAllocatedObjects = 0;
}

void GCGeneration::resetNumReachableObjects() {
  numReachableObjects_ = 0;
}

void GCGeneration::resetNumAllHiddenClasses() {
  numHiddenClasses_ = 0;
  numLeafHiddenClasses_ = 0;
}
#endif // !NDEBUG

inline AllocResult GCGeneration::AllocContext::alloc(
    uint32_t sz,
    HasFinalizer hasFinalizer) {
  AllocResult res = activeSegment.alloc(sz);
  if (LLVM_LIKELY(res.success)) {
    if (hasFinalizer == HasFinalizer::Yes) {
      // We assume here that the cell will be initialized, and become a
      // GCCell, soon after this call, before it is retrieved from
      // cellsWithFinalizers_.  Therefore, we cast here to the eventual type.
      cellsWithFinalizers.push_back(reinterpret_cast<GCCell *>(res.ptr));
    }

#ifndef NDEBUG
    numAllocatedObjects++;
#endif

    return {res.ptr, true};
  }

  return {nullptr, false};
}

inline GCGeneration::AllocContext::operator bool() const {
  return static_cast<bool>(activeSegment);
}

AllocResult GCGeneration::allocRaw(uint32_t sz, HasFinalizer hasFinalizer) {
  return allocContext_.alloc(sz, hasFinalizer);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCGENERATION_H
