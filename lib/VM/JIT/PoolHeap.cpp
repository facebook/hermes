/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/PoolHeap.h"

#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>

namespace hermes {
namespace vm {

PoolHeap::PoolHeap(void *buffer, size_t size)
    : bufferStart_((char *)llvm::alignAddr(buffer, kAlignment)),
      bufferEnd_(
          (char *)llvm::alignDown((uintptr_t)buffer + size, kAlignment)) {
  assert(bufferStart_ < bufferEnd_ && "buffer is too small");

  // Insert the the whole heap as the only free block.
  freeList_[bufferStart_] = bufferEnd_ - bufferStart_;
}

void *PoolHeap::alloc(size_t size) {
  size = llvm::alignTo<kAlignment>(size);

  for (auto it = freeList_.begin(), end = freeList_.end(); it != end; ++it) {
    // Skip smaller blocks.
    if (it->second < size)
      continue;

    char *result = it->first;
    size_t remaining = it->second - size;

    freeList_.erase(it);
    if (remaining) {
      freeList_[result + size] = remaining;
    }

    allocList_[result] = size;
    return result;
  }

  // No memory.
  return nullptr;
}

void PoolHeap::freeRemaining(void *block, size_t keepSize) {
  assert(block && "block must be valid");

  keepSize = llvm::alignTo<kAlignment>(keepSize);

  // Find the block in the allocated list so we can obtain its size.
  auto allocIt = allocList_.find((char *)block);
  assert(allocIt != allocList_.end() && "block is not in allocated list");
  assert(keepSize <= allocIt->second && "keepSize is larger than size");

  freeHelper(allocIt, keepSize);
}

void PoolHeap::free(void *block) {
  if (!block)
    return;

  // Find the block in the allocated list so we can obtain its size.
  auto allocIt = allocList_.find((char *)block);
  assert(allocIt != allocList_.end() && "block is not in allocated list");

  freeHelper(allocIt, 0);
}

void PoolHeap::freeHelper(BlockMap::iterator allocIt, size_t keepSize) {
  // Address of block to add to the free list.
  char *cblock = allocIt->first;
  // Size of block to add to the free list.
  size_t size = allocIt->second;

  if (keepSize) {
    // Are we keeping the entire block?
    if (keepSize >= size)
      return;

    cblock += keepSize;
    size -= keepSize;
    allocIt->second = keepSize;
  } else {
    cblock = allocIt->first;

    // Remove from free list.
    allocList_.erase(allocIt);
  }

  // Find the next free block.
  auto nextFreeIt = freeList_.upper_bound(cblock);

  // Is there a previous free block?
  auto prevFreeIt = nextFreeIt;
  if (prevFreeIt != freeList_.begin()) {
    --prevFreeIt;
    // Can we combine with the previous block?
    if (prevFreeIt->first + prevFreeIt->second == cblock) {
      cblock = prevFreeIt->first;
      size += prevFreeIt->second;
      freeList_.erase(prevFreeIt);
    }
  }

  // Can we collapse with the next block?
  if (nextFreeIt != freeList_.end() && cblock + size == nextFreeIt->first) {
    size += nextFreeIt->second;
    freeList_.erase(nextFreeIt);
  }

  assert(
      freeList_.count(cblock) == 0 &&
      "inconsistent internal state: block already in free list");

  // Add the new block to the free list.
  freeList_[cblock] = size;
}

void PoolHeap::dump(llvm::raw_ostream &OS, bool relativePointers) {
  auto formatPtr = [relativePointers, this](const void *p) {
    if (relativePointers)
      return llvm::format_hex((const char *)p - bufferStart_, 8);
    else
      return llvm::format_hex((uintptr_t)p, 10);
  };

  OS << "== PoolHeap\n"
     << "start: " << formatPtr(bufferStart_) << "\n"
     << "size : " << (bufferEnd_ - bufferStart_) << "\n";

  char *cur = bufferStart_;
  while (cur != bufferEnd_) {
    auto it = allocList_.find(cur);
    if (it != allocList_.end()) {
      OS << "  Used at ";
    } else {
      it = freeList_.find(cur);
      assert(it != freeList_.end() && "PoolHeap is corrupt");
      OS << "  Free at ";
    }
    OS << llvm::format_decimal(cur - bufferStart_, 8) << " size " << it->second
       << "\n";
    cur += it->second;
  }
  OS << "\n";
}

} // namespace vm
} // namespace hermes
