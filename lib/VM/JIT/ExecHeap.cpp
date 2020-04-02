/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/ExecHeap.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>

namespace hermes {
namespace vm {

ExecHeap::ExecHeap(
    size_t firstHeapSize,
    size_t secondHeapSize,
    size_t maxMemory)
    : firstHeapSize_(firstHeapSize),
      secondHeapSize_(secondHeapSize),
      maxPools_(maxMemory / (firstHeapSize + secondHeapSize)) {}

ExecHeap::~ExecHeap() = default;

ExecHeap::DualPool *ExecHeap::addPool() {
  // Have we reached the maximum number of pools?
  if (pools_.size() == maxPools_)
    return nullptr;

  // Allocate a new one.
  std::error_code EC;
  const unsigned kRWX = llvm::sys::Memory::MF_READ |
      llvm::sys::Memory::MF_WRITE | llvm::sys::Memory::MF_EXEC;
  llvm::sys::OwningMemoryBlock mb{llvm::sys::Memory::allocateMappedMemory(
      firstHeapSize_ + secondHeapSize_, nullptr, kRWX, EC)};
  if (!mb.base())
    return nullptr;

  pools_.emplace_back(std::move(mb), firstHeapSize_, secondHeapSize_);
  return &pools_.back();
}

llvm::Optional<ExecHeap::BlockPair> ExecHeap::alloc(SizePair sizes) {
  // Can't allocate blocks larger than the individual heap sizes.
  // TODO: in the future we might consider allocating a separate pool for these.
  if (sizes.first > firstHeapSize_ || sizes.second > secondHeapSize_)
    return llvm::None;

  // If nothing is requested, return nothing.
  if (!sizes.first && !sizes.second)
    return llvm::None;

  // Try to allocate in every pool in order.
  for (auto &pool : pools_) {
    if (auto res = pool.alloc(sizes))
      return res;
  }

  // Not enough memory.
  return llvm::None;
}

void ExecHeap::free(BlockPair blocks) {
  // If nothing is requested, do nothing.
  if (!blocks.first && !blocks.second)
    return;

  auto pool = findPool(blocks);
  pool->free(blocks);
  if (pool->isEntirelyFree())
    pools_.erase(pool);
}

void ExecHeap::freeRemaining(BlockPair blocks, SizePair keepSizes) {
  auto pool = findPool(blocks);
  pool->freeRemaining(blocks, keepSizes);
  if (pool->isEntirelyFree())
    pools_.erase(pool);
}

ExecHeap::PoolList::iterator ExecHeap::findPool(BlockPair blocks) {
  assert((blocks.first || blocks.second) && "at least one block must be valid");

  if (blocks.first) {
    for (auto it = pools_.begin(), e = pools_.end(); it != e; ++it) {
      if (it->contains(blocks.first)) {
        assert(
            (!blocks.second || it->contains(blocks.second)) &&
            "the second block is not contained by the same pool");
        return it;
      }
    }
  } else {
    for (auto it = pools_.begin(), e = pools_.end(); it != e; ++it) {
      if (it->contains(blocks.second)) {
        return it;
      }
    }
  }

  assert(false && "blocks could not be found in heap");
  // Return something to make the compiler happy.
  return pools_.end();
}

void ExecHeap::dump(llvm::raw_ostream &OS, bool relativePointers) {
  OS << "== ExecHeap " << firstHeapSize_ << "+" << secondHeapSize_ << "\n"
     << "  maxPools:" << maxPools_ << "\n"
     << "  numPools:" << pools_.size() << "\n";

  for (auto &pool : pools_) {
    OS << "First ";
    pool.getFirstHeap().dump(OS, relativePointers);
    OS << "Second ";
    pool.getSecondHeap().dump(OS, relativePointers);
  }

  OS << "\n";
}

ExecHeap::DualPool::DualPool(
    llvm::sys::OwningMemoryBlock &&memBlock,
    size_t firstSize,
    size_t secondSize)
    : memBlock_(std::move(memBlock)),
      firstHeap_(memBlock_.base(), firstSize),
      secondHeap_((char *)memBlock_.base() + firstSize, secondSize) {
  assert(
      (memBlock_.size() >= firstSize + secondSize) && "memBlock is too small");
}

llvm::Optional<ExecHeap::BlockPair> ExecHeap::DualPool::alloc(SizePair sizes) {
  // Note that either of the sizes can be 0, meaning we don't want to allocate
  // from that pool.

  void *first = nullptr;
  if (sizes.first) {
    first = firstHeap_.alloc(sizes.first);
    if (!first)
      return llvm::None;
  }

  void *second = nullptr;
  if (sizes.second) {
    second = secondHeap_.alloc(sizes.second);
    if (!second) {
      // If we failed to allocate the second, free the first.
      firstHeap_.free(first);
      return llvm::None;
    }
  }

  return BlockPair{(uint8_t *)first, (uint8_t *)second};
}

void ExecHeap::DualPool::free(BlockPair blocks) {
  firstHeap_.free(blocks.first);
  secondHeap_.free(blocks.second);
}

void ExecHeap::DualPool::freeRemaining(BlockPair blocks, SizePair keepSizes) {
  if (blocks.first)
    firstHeap_.freeRemaining(blocks.first, keepSizes.first);
  if (blocks.second)
    secondHeap_.freeRemaining(blocks.second, keepSizes.second);
}

} // namespace vm
} // namespace hermes
