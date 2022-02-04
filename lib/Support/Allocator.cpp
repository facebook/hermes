/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Allocator.h"

namespace hermes {

/// Allocate a size in a new slab. This is the Allocate slow path.
void *BacktrackingBumpPtrAllocator::allocateNewSlab(
    size_t size,
    size_t alignment) {
  if (size > SlabSize) {
    return allocateHuge(size);
  }

  state_->slab++;
  state_->offset = 0;
  if (state_->slab == slabs_.size()) {
    // Use `new Slab` without parentheses to avoid zeroing out the memory
    // for the new slab.
    slabs_.emplace_back(new Slab);
  }
  auto currentSlab =
      reinterpret_cast<uintptr_t>(&slabs_[state_->slab].get()->data);
  state_->offset = alignOffset(currentSlab, state_->offset, alignment);

  if (LLVM_UNLIKELY(state_->offset + size > SlabSize)) {
    return allocateHuge(size);
  }

  auto *ptr = (void *)(currentSlab + state_->offset);
  state_->offset += size;
  return ptr;
}

} // namespace hermes
