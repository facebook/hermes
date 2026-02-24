/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// This file is a copy of the LLVM Allocator.h file, with changes to satisfy
/// hermes use cases.
///
/// Concrete changes:
/// 1. Add a `State` struct, `States` and `SlabIndex` fields to
/// `BumpPtrAllocatorImpl`. Update destructor, move constructor, move
/// assignment operator and `Reset()` to reset the two fields.
/// 2. Add `pushScope()` function, which pushes a new State onto the top of the
/// `States` vector. Add `popScope()`, which pops out the top state of the
/// `State` vector and restore to previous State. Add `AllocationScope` to
/// manage the push and pop operation.
/// 3. In `StartNewSlab()`, increase `SlabIndex` by 1 everytime it's called (or
/// set it to 0 if it's None). When `SlabIndex` equals to the size of `Slabs`
/// array, allocate a new Slab, otherwise, reuse the existing slab at given
/// index.
/// 4. Change the namespace from `llvh` to `hermes`.
/// 5. Removing `PrintStats()` and `printBumpPtrAllocatorStats()`.
/// 6. Change the initial Slab size to 16KB, since modern OSs commonly use 16KB
/// page.
/// 7. Formatting changes, since LLVM uses different formating than Hermes.

#pragma once

#include "llvh/ADT/Optional.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/MathExtras.h"
#include "llvh/Support/MemAlloc.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <type_traits>
#include <utility>

namespace hermes {

/// CRTP base class providing obvious overloads for the core \c
/// Allocate() methods of LLVM-style allocators.
///
/// This base class both documents the full public interface exposed by all
/// LLVM-style allocators, and redirects all of the overloads to a single core
/// set of methods which the derived class must define.
template <typename DerivedT>
class AllocatorBase {
 public:
  /// Allocate \a Size bytes of \a Alignment aligned memory. This method
  /// must be implemented by \c DerivedT.
  void *Allocate(size_t Size, size_t Alignment) {
#ifdef __clang__
    static_assert(
        static_cast<void *(AllocatorBase::*)(size_t, size_t)>(
            &AllocatorBase::Allocate) !=
            static_cast<void *(DerivedT::*)(size_t, size_t)>(
                &DerivedT::Allocate),
        "Class derives from AllocatorBase without implementing the "
        "core Allocate(size_t, size_t) overload!");
#endif
    return static_cast<DerivedT *>(this)->Allocate(Size, Alignment);
  }

  /// Deallocate \a Ptr to \a Size bytes of memory allocated by this
  /// allocator.
  void Deallocate(const void *Ptr, size_t Size) {
#ifdef __clang__
    static_assert(
        static_cast<void (AllocatorBase::*)(const void *, size_t)>(
            &AllocatorBase::Deallocate) !=
            static_cast<void (DerivedT::*)(const void *, size_t)>(
                &DerivedT::Deallocate),
        "Class derives from AllocatorBase without implementing the "
        "core Deallocate(void *) overload!");
#endif
    return static_cast<DerivedT *>(this)->Deallocate(Ptr, Size);
  }

  // The rest of these methods are helpers that redirect to one of the above
  // core methods.

  /// Allocate space for a sequence of objects without constructing them.
  template <typename T>
  T *Allocate(size_t Num = 1, size_t alignment = alignof(T)) {
    return static_cast<T *>(Allocate(Num * sizeof(T), alignment));
  }

  /// Deallocate space for a sequence of objects without constructing them.
  template <typename T>
  typename std::enable_if<
      !std::is_same<typename std::remove_cv<T>::type, void>::value,
      void>::type
  Deallocate(T *Ptr, size_t Num = 1) {
    Deallocate(static_cast<const void *>(Ptr), Num * sizeof(T));
  }
};

class MallocAllocator : public AllocatorBase<MallocAllocator> {
 public:
  void Reset() {}

  LLVM_ATTRIBUTE_RETURNS_NONNULL void *Allocate(
      size_t Size,
      size_t /*Alignment*/) {
    return llvh::safe_malloc(Size);
  }

  // Pull in base class overloads.
  using AllocatorBase<MallocAllocator>::Allocate;

  void Deallocate(const void *Ptr, size_t /*Size*/) {
    free(const_cast<void *>(Ptr));
  }

  // Pull in base class overloads.
  using AllocatorBase<MallocAllocator>::Deallocate;
};

/// Allocate memory in an ever growing pool, as if by bump-pointer.
///
/// This isn't strictly a bump-pointer allocator as it uses backing slabs of
/// memory rather than relying on a boundless contiguous heap. However, it has
/// bump-pointer semantics in that it is a monotonically growing pool of memory
/// where every allocation is found by merely allocating the next N bytes in
/// the slab, or the next N bytes in the next slab.
///
/// Note that this also has a threshold for forcing allocations above a certain
/// size into their own slab.
///
/// The BumpPtrAllocatorImpl template defaults to using a MallocAllocator
/// object, which wraps malloc, to allocate memory, but it can be changed to
/// use a custom allocator.
template <
    typename AllocatorT = MallocAllocator,
    size_t SlabSize = 16 * 1024,
    size_t SizeThreshold = SlabSize>
class BumpPtrAllocatorImpl
    : public AllocatorBase<
          BumpPtrAllocatorImpl<AllocatorT, SlabSize, SizeThreshold>> {
 public:
  static_assert(
      SizeThreshold <= SlabSize,
      "The SizeThreshold must be at most the SlabSize to ensure "
      "that objects larger than a slab go into their own memory "
      "allocation.");

  BumpPtrAllocatorImpl() = default;

  template <typename T>
  BumpPtrAllocatorImpl(T &&Allocator)
      : Allocator(std::forward<T &&>(Allocator)) {}

  // Manually implement a move constructor as we must clear the old allocator's
  // slabs as a matter of correctness.
  BumpPtrAllocatorImpl(BumpPtrAllocatorImpl &&Old)
      : States(std::move(Old.States)),
        CurPtr(Old.CurPtr),
        End(Old.End),
        SlabIndex(Old.SlabIndex),
        Slabs(std::move(Old.Slabs)),
        CustomSizedSlabs(std::move(Old.CustomSizedSlabs)),
        BytesAllocated(Old.BytesAllocated),
        RedZoneSize(Old.RedZoneSize),
        Allocator(std::move(Old.Allocator)) {
    Old.States = {};
    Old.CurPtr = Old.End = nullptr;
    Old.SlabIndex = llvh::None;
    Old.BytesAllocated = 0;
    Old.Slabs.clear();
    Old.CustomSizedSlabs.clear();
  }

  ~BumpPtrAllocatorImpl() {
    DeallocateSlabs(Slabs.begin(), Slabs.end());
    DeallocateCustomSizedSlabs();
  }

  BumpPtrAllocatorImpl &operator=(BumpPtrAllocatorImpl &&RHS) {
    DeallocateSlabs(Slabs.begin(), Slabs.end());
    DeallocateCustomSizedSlabs();

    States = std::move(RHS.States);
    CurPtr = RHS.CurPtr;
    End = RHS.End;
    SlabIndex = RHS.SlabIndex;
    BytesAllocated = RHS.BytesAllocated;
    RedZoneSize = RHS.RedZoneSize;
    Slabs = std::move(RHS.Slabs);
    CustomSizedSlabs = std::move(RHS.CustomSizedSlabs);
    Allocator = std::move(RHS.Allocator);

    RHS.States = {};
    RHS.CurPtr = RHS.End = nullptr;
    RHS.SlabIndex = llvh::None;
    RHS.BytesAllocated = 0;
    RHS.Slabs.clear();
    RHS.CustomSizedSlabs.clear();
    return *this;
  }

  /// Deallocate all but the current slab and reset the current pointer
  /// to the beginning of it, freeing all memory allocated so far.
  void Reset() {
    // Deallocate all but the first slab, and deallocate all custom-sized slabs.
    DeallocateCustomSizedSlabs();
    CustomSizedSlabs.clear();
    // Clean up any remaining scope states.
    States.clear();
    States.reserve(kInitStateStackSize);

    if (Slabs.empty())
      return;

    // Reset the state.
    BytesAllocated = 0;
    CurPtr = (char *)Slabs.front();
    End = CurPtr + SlabSize;
    SlabIndex = 0;

    __asan_poison_memory_region(*Slabs.begin(), computeSlabSize(0));
    DeallocateSlabs(std::next(Slabs.begin()), Slabs.end());
    Slabs.erase(std::next(Slabs.begin()), Slabs.end());
  }

  /// Create and push a new scope on the stack. All allocations made after this
  /// can be unallocated with a corresponding popScope().
  void pushScope() {
    // Push a new slab if none exists before we enter a new scope.
    if (Slabs.empty()) {
      StartNewSlab();
    }
    States.emplace_back(CurPtr, *SlabIndex, CustomSizedSlabs.size());
  }

  /// Remove all allocations since the last pushScope().
  void popScope() {
    assert(!States.empty() && "No previous allocation scope pushed");

    // Deallocate custom sized slabs that were allocated after the scope was
    // created.
    auto &CurrentState = States.back();
    while (CustomSizedSlabs.size() > CurrentState.NumCustomSizedSlabs) {
      auto &PtrAndSize = CustomSizedSlabs.back();
      Allocator.Deallocate(PtrAndSize.first, PtrAndSize.second);
      CustomSizedSlabs.pop_back();
    }

    // Restore the bump pointer to the saved position.
    CurPtr = CurrentState.CurPtr;
    SlabIndex = CurrentState.SlabIndex;
    End = ((char *)Slabs[*SlabIndex]) + computeSlabSize(*SlabIndex);

    States.pop_back();
    // Note: We don't deallocate slabs that were allocated after the scope was
    // created because we're likely to need them again.
  }

  /// Allocate space at the specified alignment.
  LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS void *Allocate(
      size_t Size,
      size_t Alignment) {
    assert(Alignment > 0 && "0-byte alignment is not allowed. Use 1 instead.");

    // Keep track of how many bytes we've allocated.
    BytesAllocated += Size;

    size_t Adjustment = llvh::alignmentAdjustment(CurPtr, Alignment);
    assert(Adjustment + Size >= Size && "Adjustment + Size must not overflow");

    size_t SizeToAllocate = Size;
#if LLVM_ADDRESS_SANITIZER_BUILD
    // Add trailing bytes as a "red zone" under ASan.
    SizeToAllocate += RedZoneSize;
#endif

    // Check if we have enough space.
    if (Adjustment + SizeToAllocate <= size_t(End - CurPtr)) {
      char *AlignedPtr = CurPtr + Adjustment;
      CurPtr = AlignedPtr + SizeToAllocate;
      // Update the allocation point of this memory block in MemorySanitizer.
      // Without this, MemorySanitizer messages for values originated from here
      // will point to the allocation of the entire slab.
      __msan_allocated_memory(AlignedPtr, Size);
      // Similarly, tell ASan about this space.
      __asan_unpoison_memory_region(AlignedPtr, Size);
      return AlignedPtr;
    }

    // If Size is really big, allocate a separate slab for it.
    size_t PaddedSize = SizeToAllocate + Alignment - 1;
    if (PaddedSize > SizeThreshold) {
      void *NewSlab = Allocator.Allocate(PaddedSize, 0);
      // We own the new slab and don't want anyone reading anything other than
      // pieces returned from this method.  So poison the whole slab.
      __asan_poison_memory_region(NewSlab, PaddedSize);
      CustomSizedSlabs.push_back(std::make_pair(NewSlab, PaddedSize));

      uintptr_t AlignedAddr = llvh::alignAddr(NewSlab, Alignment);
      assert(AlignedAddr + Size <= (uintptr_t)NewSlab + PaddedSize);
      char *AlignedPtr = (char *)AlignedAddr;
      __msan_allocated_memory(AlignedPtr, Size);
      __asan_unpoison_memory_region(AlignedPtr, Size);
      return AlignedPtr;
    }

    // Otherwise, start a new slab and try again.
    StartNewSlab();
    uintptr_t AlignedAddr = llvh::alignAddr(CurPtr, Alignment);
    assert(
        AlignedAddr + SizeToAllocate <= (uintptr_t)End &&
        "Unable to allocate memory!");
    char *AlignedPtr = (char *)AlignedAddr;
    CurPtr = AlignedPtr + SizeToAllocate;
    __msan_allocated_memory(AlignedPtr, Size);
    __asan_unpoison_memory_region(AlignedPtr, Size);
    return AlignedPtr;
  }

  // Pull in base class overloads.
  using AllocatorBase<BumpPtrAllocatorImpl>::Allocate;

  // Bump pointer allocators are expected to never free their storage; and
  // clients expect pointers to remain valid for non-dereferencing uses even
  // after deallocation.
  void Deallocate(const void *Ptr, size_t Size) {
    __asan_poison_memory_region(Ptr, Size);
  }

  // Pull in base class overloads.
  using AllocatorBase<BumpPtrAllocatorImpl>::Deallocate;

  size_t GetNumSlabs() const {
    return Slabs.size() + CustomSizedSlabs.size();
  }

  /// \return An index uniquely and reproducibly identifying
  /// an input pointer \p Ptr in the given allocator.
  /// The returned value is negative iff the object is inside a custom-size
  /// slab.
  /// Returns an empty optional if the pointer is not found in the allocator.
  llvh::Optional<int64_t> identifyObject(const void *Ptr) {
    const char *P = static_cast<const char *>(Ptr);
    int64_t InSlabIdx = 0;
    for (size_t Idx = 0, E = Slabs.size(); Idx < E; Idx++) {
      const char *S = static_cast<const char *>(Slabs[Idx]);
      if (P >= S && P < S + computeSlabSize(Idx))
        return InSlabIdx + static_cast<int64_t>(P - S);
      InSlabIdx += static_cast<int64_t>(computeSlabSize(Idx));
    }

    // Use negative index to denote custom sized slabs.
    int64_t InCustomSizedSlabIdx = -1;
    for (size_t Idx = 0, E = CustomSizedSlabs.size(); Idx < E; Idx++) {
      const char *S = static_cast<const char *>(CustomSizedSlabs[Idx].first);
      size_t Size = CustomSizedSlabs[Idx].second;
      if (P >= S && P < S + Size)
        return InCustomSizedSlabIdx - static_cast<int64_t>(P - S);
      InCustomSizedSlabIdx -= static_cast<int64_t>(Size);
    }
    return llvh::None;
  }

  size_t getTotalMemory() const {
    size_t TotalMemory = 0;
    for (auto I = Slabs.begin(), E = Slabs.end(); I != E; ++I)
      TotalMemory += computeSlabSize(std::distance(Slabs.begin(), I));
    for (auto &PtrAndSize : CustomSizedSlabs)
      TotalMemory += PtrAndSize.second;
    return TotalMemory;
  }

  size_t getBytesAllocated() const {
    return BytesAllocated;
  }

  void setRedZoneSize(size_t NewSize) {
    RedZoneSize = NewSize;
  }

 private:
  /// The state of a scope, specifically the current bump pointer position and
  /// the number of custom sized slabs at the time of the scope creation.
  struct State {
    /// The current pointer into the current slab when the scope was created.
    char *CurPtr;
    /// The current slab index when the scope was created.
    size_t SlabIndex;
    /// The number of custom sized slabs when the scope was created.
    size_t NumCustomSizedSlabs;

    /// Construct a new scope shadowing a previous one.
    State(char *CurPtr, size_t SlabIndex, size_t NumCustomSizedSlabs)
        : CurPtr(CurPtr),
          SlabIndex(SlabIndex),
          NumCustomSizedSlabs(NumCustomSizedSlabs) {}
  };

  static constexpr size_t kInitStateStackSize = 8;

  /// The states stack. The top state is the current one.
  llvh::SmallVector<State, kInitStateStackSize> States;

  /// The current pointer into the current slab.
  ///
  /// This points to the next free byte in the slab.
  char *CurPtr = nullptr;

  /// The end of the current slab.
  char *End = nullptr;

  /// The current slab index.
  llvh::Optional<size_t> SlabIndex = llvh::None;

  /// The slabs allocated so far.
  llvh::SmallVector<void *, 4> Slabs;

  /// Custom-sized slabs allocated for too-large allocation requests.
  llvh::SmallVector<std::pair<void *, size_t>, 0> CustomSizedSlabs;

  /// How many bytes we've allocated.
  ///
  /// Used so that we can compute how much space was wasted.
  size_t BytesAllocated = 0;

  /// The number of bytes to put between allocations when running under
  /// a sanitizer.
  size_t RedZoneSize = 1;

  /// The allocator instance we use to get slabs of memory.
  AllocatorT Allocator;

  static size_t computeSlabSize(unsigned SlabIdx) {
    // Scale the actual allocated slab size based on the number of slabs
    // allocated. Every 128 slabs allocated, we double the allocated size to
    // reduce allocation frequency, but saturate at multiplying the slab size by
    // 2^30.
    return SlabSize * ((size_t)1 << std::min<size_t>(30, SlabIdx / 128));
  }

  /// Allocate a new slab and move the bump pointers over into the new
  /// slab, modifying CurPtr and End.
  void StartNewSlab() {
    if (!SlabIndex.hasValue()) {
      SlabIndex = 0;
    } else {
      SlabIndex = *SlabIndex + 1;
    }
    size_t AllocatedSlabSize = computeSlabSize(*SlabIndex);
    void *NewSlab = nullptr;
    if (SlabIndex == Slabs.size()) {
      NewSlab = Allocator.Allocate(AllocatedSlabSize, 0);
      // We own the new slab and don't want anyone reading anything other than
      // pieces returned from this method.  So poison the whole slab.
      __asan_poison_memory_region(NewSlab, AllocatedSlabSize);
      Slabs.push_back(NewSlab);
    } else {
      NewSlab = Slabs[*SlabIndex];
    }
    CurPtr = (char *)(NewSlab);
    End = ((char *)NewSlab) + AllocatedSlabSize;
  }

  /// Deallocate a sequence of slabs.
  void DeallocateSlabs(
      llvh::SmallVectorImpl<void *>::iterator I,
      llvh::SmallVectorImpl<void *>::iterator E) {
    for (; I != E; ++I) {
      size_t AllocatedSlabSize =
          computeSlabSize(std::distance(Slabs.begin(), I));
      Allocator.Deallocate(*I, AllocatedSlabSize);
    }
  }

  /// Deallocate all memory for custom sized slabs.
  void DeallocateCustomSizedSlabs() {
    for (auto &PtrAndSize : CustomSizedSlabs) {
      void *Ptr = PtrAndSize.first;
      size_t Size = PtrAndSize.second;
      Allocator.Deallocate(Ptr, Size);
    }
  }

  template <typename T>
  friend class SpecificBumpPtrAllocator;
};

/// The standard BumpPtrAllocator which just uses the default template
/// parameters.
typedef BumpPtrAllocatorImpl<> BumpPtrAllocator;

/// RAII object for automatically deleting all allocations made in its lifetime.
class AllocationScope {
 private:
  /// The allocator we're creating a scope in.
  BumpPtrAllocator &Allocator_;

 public:
  explicit AllocationScope(BumpPtrAllocator &Allocator)
      : Allocator_(Allocator) {
    Allocator_.pushScope();
  }
  ~AllocationScope() {
    Allocator_.popScope();
  }

  // Non-copyable and non-movable.
  AllocationScope(const AllocationScope &) = delete;
  AllocationScope &operator=(const AllocationScope &) = delete;
  AllocationScope(AllocationScope &&) = delete;
  AllocationScope &operator=(AllocationScope &&) = delete;
};

/// A BumpPtrAllocator that allows only elements of a specific type to be
/// allocated.
///
/// This allows calling the destructor in DestroyAll() and when the allocator is
/// destroyed.
template <typename T>
class SpecificBumpPtrAllocator {
  BumpPtrAllocator Allocator;

 public:
  SpecificBumpPtrAllocator() {
    // Because SpecificBumpPtrAllocator walks the memory to call destructors,
    // it can't have red zones between allocations.
    Allocator.setRedZoneSize(0);
  }
  SpecificBumpPtrAllocator(SpecificBumpPtrAllocator &&Old)
      : Allocator(std::move(Old.Allocator)) {}
  ~SpecificBumpPtrAllocator() {
    DestroyAll();
  }

  SpecificBumpPtrAllocator &operator=(SpecificBumpPtrAllocator &&RHS) {
    Allocator = std::move(RHS.Allocator);
    return *this;
  }

  /// Call the destructor of each allocated object and deallocate all but the
  /// current slab and reset the current pointer to the beginning of it, freeing
  /// all memory allocated so far.
  void DestroyAll() {
    auto DestroyElements = [](char *Begin, char *End) {
      assert(Begin == (char *)llvh::alignAddr(Begin, alignof(T)));
      for (char *Ptr = Begin; Ptr + sizeof(T) <= End; Ptr += sizeof(T))
        reinterpret_cast<T *>(Ptr)->~T();
    };

    for (auto I = Allocator.Slabs.begin(), E = Allocator.Slabs.end(); I != E;
         ++I) {
      size_t AllocatedSlabSize = BumpPtrAllocator::computeSlabSize(
          std::distance(Allocator.Slabs.begin(), I));
      char *Begin = (char *)llvh::alignAddr(*I, alignof(T));
      char *End = *I == Allocator.Slabs.back() ? Allocator.CurPtr
                                               : (char *)*I + AllocatedSlabSize;

      DestroyElements(Begin, End);
    }

    for (auto &PtrAndSize : Allocator.CustomSizedSlabs) {
      void *Ptr = PtrAndSize.first;
      size_t Size = PtrAndSize.second;
      DestroyElements(
          (char *)llvh::alignAddr(Ptr, alignof(T)), (char *)Ptr + Size);
    }

    Allocator.Reset();
  }

  /// Allocate space for an array of objects without constructing them.
  T *Allocate(size_t num = 1) {
    return Allocator.Allocate<T>(num);
  }
};

} // namespace hermes

template <typename AllocatorT, size_t SlabSize, size_t SizeThreshold>
void *operator new(
    size_t Size,
    hermes::BumpPtrAllocatorImpl<AllocatorT, SlabSize, SizeThreshold>
        &Allocator) {
  struct S {
    char c;
    union {
      double D;
      long double LD;
      long long L;
      void *P;
    } x;
  };
  return Allocator.Allocate(
      Size, std::min((size_t)llvh::NextPowerOf2(Size), offsetof(S, x)));
}

template <typename AllocatorT, size_t SlabSize, size_t SizeThreshold>
void operator delete(
    void *,
    hermes::BumpPtrAllocatorImpl<AllocatorT, SlabSize, SizeThreshold> &) {}
