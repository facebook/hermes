/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_POOLHEAP_H
#define HERMES_VM_JIT_POOLHEAP_H

#include <map>

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace hermes {
namespace vm {

/// Heap management inside a pre-allocated fixed size pool of memory.
class PoolHeap {
 public:
  static constexpr size_t kAlignment = 16;

  PoolHeap(void *buffer, size_t size);

  PoolHeap(const PoolHeap &) = delete;
  PoolHeap &operator=(const PoolHeap &) = delete;

  PoolHeap(PoolHeap &&) = default;
  PoolHeap &operator=(PoolHeap &&) = default;

  /// \return true if the heap doesn't contain any allocated blocks.
  bool isEntirelyFree() const {
    return allocList_.empty();
  }

  /// Allocate a block of size \p size.
  /// \return the address of the block or nullptr if no memory.
  void *alloc(size_t size);

  /// Split a previously allocated block \p block in two at offset \p keepSize.
  /// The second part of the block is freed. The block cannot be nullptr.
  void freeRemaining(void *block, size_t keepSize);

  /// Free a previously allocated block at address \p block. The operation is
  /// a no-op if \p block is \c nullptr.
  void free(void *block);

  /// \return true if the specified address is inside the pool.
  bool contains(void *p) const {
    return (char *)p >= bufferStart_ && (char *)p < bufferEnd_;
  }

  /// Dump the heap metadata to the specified output stream.
  /// \param OS the output stream to dump to.
  /// \param relativePointers if true all pointers are printed relative to the
  ///   start of the buffer.
  void dump(llvm::raw_ostream &OS, bool relativePointers = false);

 private:
  /// Points to the beginning of the heap buffer.
  char *const bufferStart_;
  /// Points to the end of the heap buffer.
  char *const bufferEnd_;

  using BlockMap = std::map<char *, size_t>;

  /// Contains all free blocks sorted by address. The key is the block address,
  /// the value is block size.
  BlockMap freeList_;

  /// Contains all allocated blocks sorted by address. The key is block address,
  /// the value is block size.
  BlockMap allocList_;

  /// Free a tail section or the entirety of the referenced allocated block. It
  /// splits the allocated block into an optional allocated block and a freed
  /// block.
  /// \param allocIt an iterator pointing to an allocated block in \c
  ///     allocList_.
  /// \param keepSize number of bytes in the beginning of the block to keep
  ///     allocated. If that number is 0, the entire block is freed. If that
  ///     number is larger or equal to the block size, the operation is ignored.
  void freeHelper(BlockMap::iterator allocIt, size_t keepSize);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_POOLHEAP_H
