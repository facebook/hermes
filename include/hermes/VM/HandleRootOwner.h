/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HANDLEROOTOWNER_H
#define HERMES_VM_HANDLEROOTOWNER_H

#include "hermes/VM/Casting.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValueTraits.h"
#include "hermes/VM/SymbolID.h"

#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

// Forward declarations.
class GCScope;
class HandleBase;
template <typename T>
class Handle;
template <typename T>
class MutableHandle;

#ifndef HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES
/// In debug mode we assert if more than this number of handles are allocated in
/// a scope. The actual value doesn't matter, but having a small fixed value
/// enables us to catch cases of unbounded handle allocation.
#define HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES 48
#endif

/// Define HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES in debug mode to enable
/// statistical tracking of handles per GCScope.
//#define HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES

// We can only track handles in debug mode.
#if defined(NDEBUG) && defined(HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES)
#undef HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES
#endif

/// A base class that owns a list of \c GCScope instances and is able to
/// invoke the garbage collector to mark them as roots.
/// It also contains some convenience functions that create handles.
class HandleRootOwner {
 public:
  /// A dummy virtual destructor which creates a vtable and thus ensures that
  /// cast from Runtime to HandleRootOwner is a no-op (see assert in
  /// Runtime::Runtime).
  /// Why a destructor and not a separate dummy virtual method? Because a
  /// virtual method causes "virtual functions but non-virtual destructor"
  /// warning, and the way to silence that is to add a virtual destructor.
  virtual ~HandleRootOwner() = default;

  /// Convenience function to create a Handle.
  Handle<HermesValue> makeHandle(HermesValue value);

  /// Convenience function to create a Handle from a pointer.
  template <class T>
  Handle<T> makeHandle(T *p);

  /// Convenience function to create typed Handle given a HermesValue.
  template <class T>
  Handle<T> makeHandle(HermesValue value);

  /// Convenience function to create a Handle<SymbolID>.
  Handle<SymbolID> makeHandle(SymbolID value);

  /// Create a Handle from a valid PseudoHandle and invalidate the latter.
  template <class T>
  Handle<T> makeHandle(PseudoHandle<T> &&pseudo);

  /// Create a Handle from a valid PseudoHandle by vmcasting from HermesValue
  /// and invalidate the PseudoHandle<HermesValue>.
  template <class T>
  Handle<T> makeHandle(PseudoHandle<HermesValue> &&pseudo);

  /// Convenience function to create a MutableHandle.
  MutableHandle<HermesValue> makeMutableHandle(HermesValue value);
  /// Convenience function to create a MutableHandle from a pointer.
  template <class T>
  MutableHandle<T> makeMutableHandle(T *p);

  /// Make a null pointer of the specified type. This is more efficient that
  /// other methods because it doesn't allocate a handle.
  template <class T>
  static Handle<T> makeNullHandle();

  /// An efficient way to pass undefined to a function accepting Handle.
  static Handle<HermesValue> getUndefinedValue();

  /// An efficient way to pass null to a function accepting Handle.
  static Handle<HermesValue> getNullValue();

  /// An efficient way to pass empty to a function accepting Handle.
  static Handle<HermesValue> getEmptyValue();

  /// An efficient way to pass bools to a function accepting Handle.
  static Handle<HermesValue> getBoolValue(bool b);

  /// An efficient way to pass 0 to a function accepting Handle.
  static Handle<HermesValue> getZeroValue();

  /// An efficient way to pass 1 to a function accepting Handle.
  static Handle<HermesValue> getOneValue();

  /// An efficient way to pass -1 to a function accepting Handle.
  static Handle<HermesValue> getNegOneValue();

 protected:
  /// Used for efficient construction of Handle<>(..., nullptr).
  static const PinnedHermesValue nullPointer_;
  /// Used for efficient construction of Handle(undefined).
  static const PinnedHermesValue undefinedValue_;
  /// Used for efficient construction of Handle(null).
  static const PinnedHermesValue nullValue_;
  /// Used for efficient construction of Handle(empty).
  static const PinnedHermesValue emptyValue_;
  /// Used for efficient construction of Handle(bool).
  static const PinnedHermesValue trueValue_;
  static const PinnedHermesValue falseValue_;
  /// Used for efficient construction of Handle(0).
  static const PinnedHermesValue zeroValue_;
  /// Used for efficient construction of Handle(1).
  static const PinnedHermesValue oneValue_;
  /// Used for efficient construction of Handle(-1).
  static const PinnedHermesValue negOneValue_;

  void markGCScopes(RootAcceptor &acceptor);

  /// Mark the WeakRefs in the weakRefs_ list.
  void markWeakRefs(WeakRefAcceptor &acceptor);

  /// Return the top-most \c GCScope.
  GCScope *getTopGCScope();

 private:
  friend class GCScope;
  friend class GCScopeMarkerRAII;
  friend class HandleBase;

  /// The top-most GC scope.
  GCScope *topGCScope_{};

#ifndef NDEBUG
  /// The number of reasons why no handle allocation is allowed right now.
  uint32_t noHandleLevel_{0};

  friend class NoHandleScope;
#endif

  /// Allocate storage for a new PinnedHermesValue in the specified GCScope and
  /// initialize it with \p value.
  static PinnedHermesValue *newPinnedHermesValue(
      GCScope *inScope,
      HermesValue value);

  /// Allocate storage for a new PinnedHermesValue in the top-most GCScope and
  /// initialize it with \p value.
  PinnedHermesValue *newPinnedHermesValue(HermesValue value);
};

/// This class exists only for debugging purposes. We need to make
/// \c Handle a friend so it can access the debugging fields, but we don't
/// want to make it a friend of \c GCScope as that would expose too much.
class GCScopeDebugBase {
  friend class HandleBase;

#ifndef NDEBUG
  /// In debug mode we track the number of handles associated with this scope.
  int numActiveHandles_{0};
#endif

 public:
  ~GCScopeDebugBase() {
    assert(numActiveHandles_ == 0 && "Destroying GCScope with active handles");
  }
};

#ifdef HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES

/// A class which exists only so it can be instantiated in the global scope
/// and track the distribution of number of allocated handles in GCScope.
class GCScopeHandleTracker {
  using CountMapT = llvh::DenseMap<unsigned, std::pair<unsigned, const char *>>;
  using NameMapT = llvh::DenseMap<const char *, unsigned>;

  /// Map from a number of allocated handles to name and number of GCScope
  /// instances.
  CountMapT countMap_;

  /// Map recording the max number of handles allocated for each named scope.
  NameMapT nameMap_;

 public:
  /// The destructor prints the results.
  ~GCScopeHandleTracker();

  /// Record one more occurrence of this number of handles.
  void record(const char *name, unsigned count) {
    auto &byCountValue = countMap_[count];
    ++byCountValue.first;

    if (name) {
      if (!byCountValue.second)
        byCountValue.second = name;

      unsigned &byNameEntry = nameMap_[name];
      if (count > byNameEntry)
        byNameEntry = count;
    }
  }
};

/// The only instance of this class, used to track handle statistics
/// globally. The destructor prints out the results.
extern GCScopeHandleTracker gcScopeHandleTracker;

#endif // HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES

/// An efficient container for a set of HermesValue handles which are be
/// tracked by the GC. It must be constructed on the stack. The handles
/// become unreachable as soon as the scope is destroyed. It is relatively
/// cheap to construct as it doesn't perform any allocations.
/// Under the covers it provides a non-movable storage for a HermesValue which
/// is registered with the GC. Instead of resizing it allocates handles in
/// chunks which are never moved.
class GCScope : public GCScopeDebugBase {
  friend class HandleRootOwner;

  /// Number of handles to be allocated together in a chunk.
  static constexpr size_t CHUNK_SIZE = 16;

  /// Pointer to the runtime this scope is associated with.
  HandleRootOwner &runtime_;

#ifndef NDEBUG
  /// Maximum number of handles the scope is allowed to allocate in debug mode.
  unsigned const handlesLimit_;
  /// Number of currently allocated handles.
  unsigned numAllocatedHandles_{0};
#endif

#ifdef HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES
  /// Name to use when printing statistics.
  const char *name_;

  /// Maximum number of handles that were ever allocated in this scope. I.e. a
  /// running max of /c numAllocatedHandles_.
  unsigned maxAllocatedHandles_{0};
#endif

  /// Pointer to the previous GCScope.
  GCScope *const prevScope_;

  /// Inline storage for data to be used initially until it is exhausted.
  alignas(PinnedHermesValue) char inlineStorage_
      [sizeof(PinnedHermesValue) * CHUNK_SIZE];

  /// When the inline storage is exhausted, new storage chunks are allocated
  /// here.
  llvh::SmallVector<PinnedHermesValue *, 4> chunks_;

  /// Next handle to be allocated in the current chunk.
  PinnedHermesValue *next_ = (PinnedHermesValue *)inlineStorage_;
  /// End of the storage chunk. When we reach it, we must allocate a new one.
  PinnedHermesValue *curChunkEnd_ =
      (PinnedHermesValue *)inlineStorage_ + CHUNK_SIZE;

  /// Index of the current chunk in 'chunks_'.
  unsigned curChunkIndex_{0};

  GCScope(const GCScope &) = delete;
  void operator=(const GCScope &) = delete;

 public:
  /// Construct a new GCScope object on the stack and register it with the
  /// garbage collector, so all handles associated with it will be automatically
  /// marked.
  /// \param name   when GCScope statistics are enabled, the optional string
  ///   supplied here is recorded with the handle count.
  /// \param handlesLimit in debug mode this optionally overrides the default
  ///   handle limit (\c HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES). The intent is
  ///   to use it in cases when we know that it is OK to allocate more than the
  ///   default limit. If we don't want to set a limit at all, \c UINT_MAX
  ///   should be used.
  explicit GCScope(
      HandleRootOwner &runtime,
      const char *name = nullptr,
      unsigned handlesLimit = HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES)
      : runtime_(runtime),
#ifndef NDEBUG
        handlesLimit_(handlesLimit),
#endif
#ifdef HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES
        name_(name),
#endif
        prevScope_(runtime.topGCScope_),
        chunks_({(PinnedHermesValue *)inlineStorage_}) {
    runtime.topGCScope_ = this;
  }

  ~GCScope();

  /// \return the parent scope of this scope.
  GCScope *getParentScope() {
    return prevScope_;
  }

  /// \return the increment in which pages of handles are allocated. To be
  /// used for debugging and asserts.
  static constexpr size_t getChunkSize() {
    return CHUNK_SIZE;
  }

  /// \return true if there are no handles in the scope
  bool isEmpty() const {
    return curChunkIndex_ == 0 &&
        next_ == (const PinnedHermesValue *)inlineStorage_;
  }

#ifndef NDEBUG
  /// Return the number of handles in the scope.
  LLVM_ATTRIBUTE_ALWAYS_INLINE size_t getHandleCountDbg() const {
    return numAllocatedHandles_;
  }

  LLVM_ATTRIBUTE_ALWAYS_INLINE void setHandleCountDbg(unsigned count) {
    numAllocatedHandles_ = count;
  }
#else
  LLVM_ATTRIBUTE_ALWAYS_INLINE size_t getHandleCountDbg() const {
    return curChunkIndex_ * CHUNK_SIZE + (next_ - chunks_[curChunkIndex_]);
  }
  LLVM_ATTRIBUTE_ALWAYS_INLINE void setHandleCountDbg(unsigned /*count*/) {}
#endif

  /// An opaque object which remembers the state of GCScope. It can be used
  /// to restore the state, freeing all handles which have been allocated after
  /// the state was saved or "marked".
  /// Since handles are always allocated in a GCScope and never freed (freeing
  /// individual handles would be too expensive), this enables us to efficiently
  /// free all handles allocated after a certain point in bulk and continue
  /// using the scope.
  class Marker {
    friend class GCScope;
    PinnedHermesValue *const next;
    unsigned const curChunkIndex;

    Marker(PinnedHermesValue *next, unsigned int curChunkIndex)
        : next(next), curChunkIndex(curChunkIndex) {}

   public:
    bool operator==(Marker other) const {
      return next == other.next && curChunkIndex == other.curChunkIndex;
    }
  };

  /// Remember the state of the scope and return an opaque object identifying
  /// the state. When 'flushToMarker()' is later called, all handles which were
  /// allocated after the 'mark()' call are freed.
  Marker createMarker() const {
    return Marker(next_, curChunkIndex_);
  }

  /// Free all handles that were allocated after the marker \p marker was
  /// obtained. This method can be called multiple times with the same marker.
  void flushToMarker(Marker marker) {
    assert(
        marker.curChunkIndex <= curChunkIndex_ &&
        "trying to reset to a higher chunk");
    assert(
        (marker.curChunkIndex < curChunkIndex_ || marker.next <= next_) &&
        "trying to reset to a higher next pointer");
    PinnedHermesValue *const chunkStart = chunks_[marker.curChunkIndex];
    PinnedHermesValue *const chunkEnd = chunkStart + CHUNK_SIZE;
    // Write empty values into the soon-to-be invalid chunk slots to catch bugs.
    invalidateFreedHandleValues(marker.curChunkIndex, marker.next);
    curChunkIndex_ = marker.curChunkIndex;
    curChunkEnd_ = chunkEnd;
    // Ensure that the marker is associated with this GCScope by validating the
    // saved 'next' pointer - it must point within the chunk. Note that it can
    // be equal to curChunkEnd_ if the chunk had just been exhausted before the
    // marker was saved.
    assert(
        marker.next >= chunkStart && marker.next <= curChunkEnd_ &&
        "savedNext is not within the current chunk");
    next_ = marker.next;

    setHandleCountDbg(
        curChunkIndex_ * CHUNK_SIZE + (next_ - chunks_[curChunkIndex_]));
  }

  /// Free all handles that were allocated after the first
  /// \p numHandlesToPreserve. It must be a value less or equal than CHUNK_SIZE.
  void flushToSmallCount(unsigned numHandlesToPreserve) {
    assert(
        numHandlesToPreserve <= CHUNK_SIZE && "numHandles exceeds CHUNK_SIZE");
    assert(
        numHandlesToPreserve <= getHandleCountDbg() &&
        "numHandles exceeds the actual number of handles");

    auto *chunk = (PinnedHermesValue *)inlineStorage_;
    // Write empty values into the soon-to-be invalid chunk slots to catch bugs.
    invalidateFreedHandleValues(0, chunk + numHandlesToPreserve);
    next_ = chunk + numHandlesToPreserve;
    curChunkEnd_ = chunk + CHUNK_SIZE;
    curChunkIndex_ = 0;

    setHandleCountDbg(numHandlesToPreserve);
  }

  /// Free all handles in the scope. This is equivalent to obtaining a marker
  /// right after rhe scope was created and flushing it.
  void clearAllHandles() {
    flushToSmallCount(0);
  }

 private:
  /// Writes invalid handles into every value within every chunk from \p
  /// chunkStart to curChunkIndex_, except the first chunk, which writes values
  /// from \p valueStart to the end of the chunk.
  void invalidateFreedHandleValues(
      unsigned chunkStart,
      PinnedHermesValue *valueStart)
#ifdef HERMES_SLOW_DEBUG
      ;
#else
  {
  }
#endif

  /// Allocate storage for a new PinnedHermesValue. The garbage collector knows
  /// about it and will be able to mark it.
  PinnedHermesValue *newPinnedHermesValue(HermesValue value) {
    assert(
        getHandleCountDbg() < handlesLimit_ &&
        "Too many handles allocated in GCScope");
    assert(runtime_.noHandleLevel_ == 0 && "No handles allowed right now.");

    setHandleCountDbg(getHandleCountDbg() + 1);
#ifdef HERMESVM_DEBUG_TRACK_GCSCOPE_HANDLES
    if (numAllocatedHandles_ > maxAllocatedHandles_)
      maxAllocatedHandles_ = numAllocatedHandles_;
#endif

    /// Check the fast path: is there space in the current chunk?
    if (LLVM_LIKELY(next_ < curChunkEnd_)) {
      /// Initialize the new handle with the specified value and return.
      return new (next_++) PinnedHermesValue(value);
    }

    /// Slow path: allocate a new chunk.
    return _newChunkAndPHV(value);
  }

  /// The current chunk is full, so allocate a new chunk and allocate a
  /// PinnedHermesValue inside it. This is the allocation slow path.
  PinnedHermesValue *_newChunkAndPHV(HermesValue value);

  /// Mark all handles in this scope.
  void mark(RootAcceptor &acceptor);
};

/// A RAII class which records a GCScope marker on construction and flushes the
/// scope to the marker on destruction.
/// It is functionally equivalent to creating a nested GCScope but can be more
/// lightweight because it doesn't consume a lot of stack and because it behaves
/// better in cases when more than GCScope::CHUNK_SIZE handles are allocated in
/// a loop - they would be allocated once in the parent and reused on every
/// iteration.
class GCScopeMarkerRAII {
  GCScope *const gcScope_;
  GCScope::Marker const marker_;

  GCScopeMarkerRAII(const GCScopeMarkerRAII &) = delete;
  void operator=(const GCScopeMarkerRAII &) = delete;

 public:
  /// Record a marker for the currently active scope.
  explicit GCScopeMarkerRAII(HandleRootOwner &runtime)
      : gcScope_(runtime.getTopGCScope()), marker_(gcScope_->createMarker()) {}
  /// Record a new a marker for the specified scope.
  explicit GCScopeMarkerRAII(GCScope &gcScope)
      : gcScope_(&gcScope), marker_(gcScope_->createMarker()) {}
  /// Record an existing marker for the specified scope.
  explicit GCScopeMarkerRAII(GCScope &gcScope, const GCScope::Marker &marker)
      : gcScope_(&gcScope), marker_(marker) {}

  /// Manually flush to the specified marker.
  void flush() {
    gcScope_->flushToMarker(marker_);
  }

  ~GCScopeMarkerRAII() {
    flush();
  }
};

} // namespace vm
} // namespace hermes

#pragma GCC diagnostic pop
#endif
