/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_ALLOCATOR_H
#define HERMES_SUPPORT_ALLOCATOR_H

#include "hermes/Support/CheckedMalloc.h"

#include "llvh/ADT/STLExtras.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/MathExtras.h"

#include <cstdint>
#include <memory>
#include <vector>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {

/// A slab based BumpPtrAllocator where the pointer can be saved and restored.
class BacktrackingBumpPtrAllocator {
 private:
  /// The size of slabs we allocate.
  // For lazy parsing, we don't need to be efficient for huge amounts of huge
  // allocations or tiny amounts of tiny allocations. Just relevant amounts of
  // AST subtrees. Therefore, we just use a fixed size instead of growing it.
  static const unsigned SlabSize = 256 << 10;

  /// The default memory alignment.
  static const unsigned kDefaultPlatformAlignment = sizeof(double);

  /// A slab of memory that can be allocated.
  struct Slab {
    char data[SlabSize];
  };
  /// The currently allocated slabs in order.
  std::vector<std::unique_ptr<Slab>> slabs_{};

  /// The state of a scope, specifically the current bump pointer position and
  /// any extra allocations.
  class State {
   public:
    /// The currently used slab.
    unsigned slab;
    /// The current offset within the slab.
    uintptr_t offset;
    /// A place to store allocations that can't fit into the slabs, if any.
    llvh::SmallVector<std::unique_ptr<void, decltype(free) *>, 0> hugeAllocs{};
    /// The state of the previous scope.
    State *previous;

    /// Construct a new scope shadowing a previous one.
    State(State *previous)
        : slab(previous->slab), offset(previous->offset), previous(previous) {}
    /// Construct the initial scope.
    State() : slab(0), offset(0), previous(nullptr) {}
  };

  /// The current state of the bump pointer.
  State *state_;

  /// Allocate memory that can't fit within a single slab.
  void *allocateHuge(int size) {
    auto *ptr = checkedMalloc(size);
    state_->hugeAllocs.push_back(
        std::unique_ptr<void, decltype(free) *>(ptr, free));
    return ptr;
  }

  /// Align an offset from a base pointer.
  static uintptr_t
  alignOffset(uintptr_t base, uintptr_t offset, uintptr_t alignment) {
    return llvh::alignTo(base + offset, alignment) - base;
  }

  /// Allocate a size in a new slab. This is the Allocate slow path.
  void *allocateNewSlab(size_t size, size_t alignment = sizeof(double));

 public:
  explicit BacktrackingBumpPtrAllocator() {
    state_ = new State();
    slabs_.push_back(std::make_unique<Slab>());
  }
  ~BacktrackingBumpPtrAllocator() {
    while (state_)
      popScope();
  }

  /// Create and push a new scope on the stack. All allocations made after this
  /// can be unallocated with a corresponding popScope().
  void pushScope() {
    state_ = new State(state_);
  }

  /// Remove all allocations since the last pushScope().
  void popScope() {
    assert(state_ && "No previous allocation scope pushed");
    auto *top = state_;
    state_ = state_->previous;
    delete top;
    // We could also clean up unnecessary slabs, but we're likely to need
    // then again so don't bother.
  }

  /// Allocate space for N elements of type T.
  template <typename T>
  inline T *Allocate(size_t num = 1, size_t alignment = sizeof(double)) {
    int size = sizeof(T) * num;
    return static_cast<T *>(Allocate(size, alignment));
  }

  /// Allocate \p size bytes of memory.
  inline void *Allocate(
      size_t size,
      size_t alignment = kDefaultPlatformAlignment) {
    auto currentSlab =
        reinterpret_cast<uintptr_t>(&slabs_[state_->slab].get()->data);
    state_->offset = alignOffset(currentSlab, state_->offset, alignment);

    if (LLVM_UNLIKELY(size > SlabSize || state_->offset + size > SlabSize)) {
      return allocateNewSlab(size, alignment);
    }

    auto *ptr = (void *)(currentSlab + state_->offset);
    state_->offset += size;
    return ptr;
  }
};

/// RAII object for automatically deleting all allocations made in its lifetime.
class AllocationScope {
 private:
  /// The allocator we're creating a scope in.
  BacktrackingBumpPtrAllocator &allocator_;

 public:
  AllocationScope(BacktrackingBumpPtrAllocator &allocator)
      : allocator_(allocator) {
    allocator_.pushScope();
  }
  ~AllocationScope() {
    allocator_.popScope();
  }
};

// To more conveniently replace llvh::BumpPtrAllocator.
using BumpPtrAllocator = BacktrackingBumpPtrAllocator;
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_SUPPORT_ALLOCATOR_H
