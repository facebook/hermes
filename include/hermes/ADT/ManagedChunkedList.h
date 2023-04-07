/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_MANAGEDCHUNKEDLIST_H
#define HERMES_ADT_MANAGEDCHUNKEDLIST_H

#include "hermes/ADT/ExponentialMovingAverage.h"
#include "hermes/Support/SlowAssert.h"
#include "llvh/Support/Compiler.h"

#include <cmath>
#include <cstddef>
#include <utility>

namespace hermes {

// ManagedChunkedList stores a garbage-collected sequence of values, allocating
// storage in chunks. Each value added to the list is stored in a caller-
// defined \c Element that is responsible for reporting when the value is no
// longer in use (i.e. when the \c Element is free), as well as storing a
// pointer to a subsequent \c Element (the "nextFree") for internal use by the
// ManagedChunkList. Specifically, \c Element must provide:
// - A constructor to create a new \c Element in the free state
// - An \c emplace method to emplace a value in the \c Element
// - An \c isFree method to return whether the value has been freed
// - A \c setNextFree method to store a pointer to another \c Element
// - A \c getNextFree mthod to return the previously-stored pointer to another
//   \c Element.
template <typename Element, size_t kElementsPerChunk = 16>
class ManagedChunkedList {
 public:
  // Create a new ManagedChunkedList. The occupancy ratio determines the
  // collection threshold based on the number of values that survive
  // collection. The sizing weight determines how quickly to react to changing
  // collection thresholds. Both are in the range 0.0 - 1.0.
  ManagedChunkedList(double occupancyRatio, double sizingWeight)
      : targetChunkCount_(sizingWeight, 0.0), occupancyRatio_{occupancyRatio} {}

  // Move an existing ManagedChunkedList into this one, leaving the source's
  // target chunk count unchanged.
  ManagedChunkedList(ManagedChunkedList &&other)
      : chunks_(other.chunks_),
        chunkCount_(other.chunkCount_),
        targetChunkCount_(other.targetChunkCount_),
        freeList_(other.freeList_),
        occupancyRatio_(other.occupancyRatio_) {
    other.chunks_ = nullptr;
  }

  ManagedChunkedList(const ManagedChunkedList &) = delete;
  ManagedChunkedList &operator=(const ManagedChunkedList &) = delete;

  // Destroy the ManagedChunkedList, deleting all chunks.
  ~ManagedChunkedList() {
    while (Chunk *chunk = chunks_) {
      chunks_ = chunk->nextChunk;
      delete chunk;
    }
  }

  // Add an element to the chunk list, potentially triggering a garbage
  // collection.
  template <typename... Args>
  Element &add(Args &&...args) {
    // Check to see if the list is out of free elements.
    if (LLVM_UNLIKELY(!freeList_)) {
      // There are no free elements; need to either garbage collect or allocate
      // a new chunk.
      if (chunkCount_ < targetChunkCount_) {
        // Garbage collection is scheduled to happen at a larger chunk count;
        // just allocate a new chunk of unoccpied elements.
        allocateChunk();
      } else {
        // Reached the garbage collection threshold; collect.
        collect();

        // A new chunk may still be required (e.g. if nothing was collected).
        if (!freeList_) {
          allocateChunk();
        }
      }
    }
    assert(freeList_ && "Freelist must be populated before inserting");

    // Emplace the new value at the first location in the free list.
    Element &element = *freeList_;
    freeList_ = freeList_->getNextFree();
    assert(element.isFree() && "Premature element reuse");
    element.emplace(std::forward<Args>(args)...);
    HERMES_SLOW_ASSERT(checkWellFormed());
    return element;
  }

  // Iterate over the values contained in the list, invoking a callback for
  // each value. Execution time is proportional to the capacity of the list,
  // as free elements are examined and skipped.
  template <typename Func>
  void forEach(Func accept) const {
    for (Chunk *chunk = chunks_; chunk; chunk = chunk->nextChunk) {
      for (Element &element : chunk->elements) {
        if (!element.isFree()) {
          accept(element);
        }
      }
    }
  }

  // Determine the capacity of the list (i.e. the number of values it can hold
  // without any additional chunk allocations).
  size_t capacity() const {
    return chunkCount_ * kElementsPerChunk;
  }

  // Determine the number of values contained in the list. Only used for testing
  // purposes.
  size_t sizeForTests() const {
    size_t size = 0;
    forEach([&size](Element &) { size++; });
    return size;
  }

  // Perform a garbage collection, building a free list of all free elements,
  // and potentially deleting excess empty chunks.
  void collect() {
    HERMES_SLOW_ASSERT(checkWellFormed());

    freeList_ = nullptr;
    size_t occupiedElementCount = 0;

    // Delete chunks based on the target number of chunks from the previous
    // collection. This can make deletion lag by one collection cycle, but
    // simplifies the collection logic by allowing chunks to be deleted as they
    // are encountered, before knowing the number of surviving values.
    size_t targetChunkCount = ceil(targetChunkCount_);
    Chunk **chunkListEndLoc = &chunks_;
    for (Chunk *chunk = chunks_; chunk;) {
      size_t chunkOccupiedElementCount = 0;
      Element *newFreeList = freeList_;

      // Rebuild free list and count elements.
      for (Element &element : chunk->elements) {
        if (element.isFree()) {
          element.setNextFree(newFreeList);
          newFreeList = &element;
        } else {
          chunkOccupiedElementCount++;
        }
      }

      Chunk *nextChunk = chunk->nextChunk;
      if (chunkOccupiedElementCount == 0 && chunkCount_ > targetChunkCount) {
        // This chunk is empty, and there are more chunks than desired;
        // remove the chunk.
        *chunkListEndLoc = nextChunk;
        chunkCount_--;
        delete chunk;
      } else {
        freeList_ = newFreeList;
        occupiedElementCount += chunkOccupiedElementCount;
        chunkListEndLoc = &chunk->nextChunk;
      }
      chunk = nextChunk;
    }

    // Schedule the next collection at a multiple of the the number of suviving
    // elements (using the occupancy ratio), averaged with previous target to
    // avoid drastic jumps.
    targetChunkCount_.update(
        occupiedElementCount / occupancyRatio_ / kElementsPerChunk);

    HERMES_SLOW_ASSERT(checkWellFormed());
  }

 private:
  struct Chunk {
    Chunk *nextChunk;
    Element elements[kElementsPerChunk];
  };

  // Allocate a chunk. Should only be called when there are no free elements.
  // Upon returning, there is guaranteed to be a chunk of free elements in the
  // free list.
  void allocateChunk() {
    assert(!freeList_ && "Freelist must be empty before allocating a chunk");

    Chunk *chunk = new Chunk;

    // Create a linked list of free elements, going from high address
    // to low.
    Element *next = nullptr;
    for (Element &current : chunk->elements) {
      assert(current.isFree());
      current.setNextFree(next);
      next = &current;
    }

    // Insert into the chunk list
    chunk->nextChunk = chunks_;
    chunks_ = chunk;

    // Chunks are only allocated when the free list is empty. Just start the
    // free list at this chunk.
    freeList_ = next;

    chunkCount_++;

    HERMES_SLOW_ASSERT(checkWellFormed());
  }

  bool checkWellFormed() {
    for (Element *freeElement = freeList_; freeElement;
         freeElement = freeElement->getNextFree()) {
      if (!freeElement->isFree()) {
        return false;
      }
    }

    size_t chunkCount = 0;
    for (Chunk *chunk = chunks_; chunk; chunk = chunk->nextChunk) {
      chunkCount++;
    }
    if (chunkCount != chunkCount_) {
      return false;
    }

    return true;
  }

  Chunk *chunks_ = nullptr;
  size_t chunkCount_ = 0;
  ExponentialMovingAverage targetChunkCount_;
  Element *freeList_ = nullptr;
  double occupancyRatio_;
};

} // namespace hermes

#endif // HERMES_ADT_MANAGEDCHUNKEDLIST_H
