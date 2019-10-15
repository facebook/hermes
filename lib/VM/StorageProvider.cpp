/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StorageProvider.h"

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedStorage.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

#include <cassert>
#include <limits>
#include <stack>

namespace hermes {
namespace vm {

namespace {

bool isAligned(void *p) {
  return (reinterpret_cast<uintptr_t>(p) & (AlignedStorage::size() - 1)) == 0;
}

char *alignAlloc(void *p) {
  return reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(p), AlignedStorage::size()));
}

class VMAllocateStorageProvider final : public StorageProvider {
 public:
  llvm::ErrorOr<void *> newStorage(const char *name) override;
  void deleteStorage(void *storage) override;
};

class MallocStorageProvider final : public StorageProvider {
 public:
  llvm::ErrorOr<void *> newStorage(const char *name) override;
  void deleteStorage(void *storage) override;

 private:
  /// Map aligned starts to actual starts for freeing.
  /// NOTE: Since this is only used for debugging purposes, and it is rare to
  /// create and delete storage, it's ok to use a map.
  llvm::DenseMap<void *, void *> lowLimToAllocHandle_;
};

class PreAllocatedStorageProvider final : public StorageProvider {
 public:
  PreAllocatedStorageProvider(size_t totalAmount, void *region);
  ~PreAllocatedStorageProvider();

  llvm::ErrorOr<void *> newStorage(const char *name) override;
  void deleteStorage(void *storage) override;

 private:
  /// Max amount of bytes ever allocatable.
  const size_t maxBytes_;

  /// Start of storages. This is aligned on AlignedStorage::size() boundaries.
  char *const start_;

  /// End of storages. This is the upper limit.
  char *const end_;

  /// End of used storages. This can be increased up to end_.
  char *level_;

  /// Storages that are not in use, and can be re-assigned.
  /// If empty, take from the level_.
  /// Since storages all the same size, this is simply re-using the most
  /// recently deleted storage.
  std::stack<char *, std::vector<char *>> freeList_;
};

llvm::ErrorOr<void *> VMAllocateStorageProvider::newStorage(const char *name) {
  assert(AlignedStorage::size() % oscompat::page_size() == 0);
  // Allocate the space, hoping it will be the correct alignment.
  auto result = oscompat::vm_allocate_aligned(
      AlignedStorage::size(), AlignedStorage::size());
  if (!result) {
    return result;
  }
  void *mem = *result;
  assert(isAligned(mem));
  (void)isAligned;

  // Name the memory region on platforms that support naming.
  oscompat::vm_name(mem, AlignedStorage::size(), name);
  return mem;
}

void VMAllocateStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  oscompat::vm_free_aligned(storage, AlignedStorage::size());
}

llvm::ErrorOr<void *> MallocStorageProvider::newStorage(const char *name) {
  // name is unused, can't name malloc memory.
  (void)name;
  void *mem = checkedMalloc2(AlignedStorage::size(), 2u);
  void *lowLim = alignAlloc(mem);
  assert(isAligned(lowLim) && "New storage should be aligned");
  lowLimToAllocHandle_[lowLim] = mem;
  return lowLim;
}

void MallocStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  free(lowLimToAllocHandle_[storage]);
  lowLimToAllocHandle_.erase(storage);
}

PreAllocatedStorageProvider::PreAllocatedStorageProvider(
    size_t totalAmount,
    void *region)
    : maxBytes_(totalAmount),
      start_(static_cast<char *>(region)),
      end_(start_ ? start_ + maxBytes_ : nullptr),
      level_(start_) {
  assert(maxBytes_ % AlignedStorage::size() == 0 && "Un-aligned maxBytes");
}

PreAllocatedStorageProvider::~PreAllocatedStorageProvider() {
  if (start_) {
    oscompat::vm_free_aligned(start_, maxBytes_);
  }
}

llvm::ErrorOr<void *> PreAllocatedStorageProvider::newStorage(
    const char *name) {
  char *newStorage;
  if (!freeList_.empty()) {
    newStorage = freeList_.top();
    freeList_.pop();
  } else if (level_ != end_) {
    // Push the end further.
    newStorage = level_;
    level_ += AlignedStorage::size();
  } else {
    // Nothing free, and level_ is already at the end_, cannot allocate a
    // storage.
    return make_error_code(OOMError::MaxStorageReached);
  }
  oscompat::vm_name(newStorage, AlignedStorage::size(), name);
  assert(
      newStorage >= start_ && newStorage + AlignedStorage::size() <= end_ &&
      "About to return a pointer that is not owned by this storage");
  return static_cast<void *>(newStorage);
}

void PreAllocatedStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  freeList_.push(static_cast<char *>(storage));
  // Mark this memory as unused and reclaimable.
  oscompat::vm_unused(storage, AlignedStorage::size());
  // Remove the name from this memory, so if we see it in a heap dump it will
  // say that it's not in active use.
  oscompat::vm_name(storage, AlignedStorage::size(), "hermes-freelist");
}

} // namespace

/* static */
llvm::ErrorOr<std::unique_ptr<StorageProvider>>
StorageProvider::preAllocatedProvider(
    size_t amount,
    size_t minAmount,
    size_t excess) {
  assert(
      amount % AlignedStorage::size() == 0 &&
      "amount must be a multiple of AlignedStorage::size()");
  assert(
      minAmount % AlignedStorage::size() == 0 &&
      "minAmount must be a multiple of AlignedStorage::size()");
  assert(
      excess <= AlignedStorage::size() &&
      "Excess is greater than AlignedStorage::size, but storages aren't guaranteed to be contiguous");
  auto maxBytes = llvm::alignTo<AlignedStorage::size()>(amount + excess);
  const auto minBytes =
      llvm::alignTo<AlignedStorage::size()>(minAmount + excess);
  void *region = nullptr;
  if (maxBytes) {
    auto result =
        vmAllocateAllowLess(maxBytes, minBytes, AlignedStorage::size());
    if (!result) {
      return result.getError();
    }
    std::pair<void *, size_t> memAndSz = result.get();
    region = memAndSz.first;
    maxBytes = memAndSz.second;
  }
  return std::unique_ptr<StorageProvider>(
      new PreAllocatedStorageProvider(maxBytes, region));
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mmapProvider() {
  return std::unique_ptr<StorageProvider>(new VMAllocateStorageProvider);
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mallocProvider() {
  return std::unique_ptr<StorageProvider>(new MallocStorageProvider);
}

llvm::ErrorOr<std::pair<void *, size_t>>
vmAllocateAllowLess(size_t sz, size_t minSz, size_t alignment) {
  assert(sz >= minSz && "Shouldn't supply a lower size than the minimum");
  assert(minSz != 0 && "Minimum size must not be zero");
  assert(sz == llvm::alignTo(sz, alignment));
  assert(minSz == llvm::alignTo(minSz, alignment));
  // Try fractions of the requested size, down to the minimum.
  // We'll do it by eighths.
  assert(sz >= 8); // Since sz is page-aligned, safe assumption.
  const size_t increment = sz / 8;
  // Store the result for the case where all attempts fail.
  llvm::ErrorOr<void *> result{std::error_code{}};
  while (sz >= minSz) {
    result = oscompat::vm_allocate_aligned(sz, alignment);
    if (result) {
      assert(
          sz == llvm::alignTo(sz, alignment) &&
          "Should not return an un-aligned size");
      return std::make_pair(result.get(), sz);
    }
    if (sz < increment || sz == minSz) {
      // Would either underflow or can't reduce any lower.
      break;
    }
    sz = std::max(
        static_cast<size_t>(llvm::alignDown(sz - increment, alignment)), minSz);
  }
  assert(!result && "Must be an error if none of the allocations succeeded");
  return result.getError();
}

} // namespace vm
} // namespace hermes
