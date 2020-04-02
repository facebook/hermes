/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_EXECHEAP_H
#define HERMES_VM_JIT_EXECHEAP_H

#include "hermes/VM/JIT/PoolHeap.h"

#include "llvm/ADT/Optional.h"
#include "llvm/Support/Memory.h"

#include <algorithm>
#include <list>
#include <memory>
#include <utility>

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace hermes {
namespace vm {

/// Provide heap management for executable memory.
/// This class maintains a list of blocks of executable memory of fixed size.
/// This is preferable to a single large executable block for a couple of
/// reasons:
/// - Allows us to grow the VA size incrementally.
/// - Keeps each executable block small so that relative jumps can work well
///   within it.
/// Each block (\c DualPool) is split into two heaps of specified size.
/// Allocation and deallocation functions work in both heaps at the same time.
class ExecHeap {
 public:
  using BlockPair = std::pair<uint8_t *, uint8_t *>;
  using SizePair = std::pair<size_t, size_t>;
  // Forward declaration.
  class DualPool;

  ExecHeap(const ExecHeap &) = delete;
  void operator=(const ExecHeap &) = delete;

  /// Construct an empty execitable heap. New individual pools will be allocated
  /// with the specified heap sizes up to the total of the specified \p
  /// maxMemory.
  ExecHeap(size_t firstHeapSize, size_t secondHeapSize, size_t maxMemory);
  ~ExecHeap();

  /// Allocate a new pool with the pre-configured size and add it to the list
  /// of available pools. The new pool is owned by the class, but is returned in
  /// case the caller wants to pre-allocate stuff in it.
  /// \return the new pool on success, null if the pre-configured maximum memory
  ///   limit is reached, or if there is an OS failure when allocating.
  DualPool *addPool();

  // Allocate a pair of blocks with the specified sizes in the first available
  // DualPool with enough memory to accomodate both. If a size is 0,
  // don't allocate from the corresponding pool and return nullptr for that
  // block. Fail if there is not enough memory in the currently allocated
  // pools; in that case the caller will typically invoke \c addPool() and retry
  // the allocation.
  llvm::Optional<BlockPair> alloc(SizePair sizes);

  /// Split the specified previously allocated blocks (which must have been
  /// allocated together by a call to \c alloc()) at offset keepSizes.first/
  /// .second and free the second blocks.
  void freeRemaining(BlockPair blocks, SizePair keepSizes);

  /// Free the specified previously allocated blocks (which must have been
  /// allocated together by a call to \c alloc()). This is equivalent to
  /// \c freeRemaining(blocks, {0, 0}).
  void free(BlockPair blocks);

  /// Invalidate the instruction cache for a JIT-ted block of code before
  /// executing it.
  inline void invalidateInstructionCache(void *addr, size_t len) {
    llvm::sys::Memory::InvalidateInstructionCache(addr, len);
    // Possibly use  __builtin___clear_cache() for better performance?
  }

  /// Dump the heap metadata to the specified output stream.
  /// \param OS the output stream to dump to.
  /// \param relativePointers if true all pointers are printed relative to the
  ///   start of the buffer.
  void dump(llvm::raw_ostream &OS, bool relativePointers = false);

  /// A block of executable memory split into two heaps allowing separate
  /// allocation/deallocation.
  class DualPool {
    llvm::sys::OwningMemoryBlock memBlock_;
    PoolHeap firstHeap_;
    PoolHeap secondHeap_;

   public:
    DualPool(
        llvm::sys::OwningMemoryBlock &&memBlock,
        size_t firstSize,
        size_t secondSize);

    PoolHeap &getFirstHeap() {
      return firstHeap_;
    }

    PoolHeap &getSecondHeap() {
      return secondHeap_;
    }

    /// \return true if the specified pointer is contained inside one of the
    ///   two heaps.
    bool contains(uint8_t *p) const {
      return p >= memBlock_.base() &&
          p < (uint8_t *)memBlock_.base() + memBlock_.size();
    }

    // Allocate a pair of blocks with the specified sizes. If the size is 0,
    // don't allocate from the corresponding pool and return nullptr for that
    // block.
    llvm::Optional<BlockPair> alloc(SizePair sizes);

    /// Split the specified previously allocated blocks (which must have been
    /// allocated together by a call to \c alloc()) at offset keepSizes.first/
    /// .second and free the second blocks.
    void freeRemaining(BlockPair blocks, SizePair keepSizes);

    /// Free the specified previously allocated blocks (which must have been
    /// allocated together by a call to \c alloc()). This is equivalent to
    /// \c freeRemaining(blocks, {0, 0}).
    void free(BlockPair blocks);

    /// \return true if both heaps don't contain any allocated blocks.
    bool isEntirelyFree() const {
      return firstHeap_.isEntirelyFree() && secondHeap_.isEntirelyFree();
    }
  };

 public:
  /// The size of the first heap in every new DualPool.
  size_t const firstHeapSize_;
  /// The size of the second heap in every new DualPool.
  size_t const secondHeapSize_;
  /// The maximum number of pools we are allowed to allocate.
  unsigned const maxPools_;

  using PoolList = std::list<DualPool>;

  /// The pools that we have allocated so far.
  PoolList pools_{};

  /// \return the pool that contains the blocks. This call must
  /// always succeed.
  PoolList::iterator findPool(BlockPair blocks);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_EXECHEAP_H
