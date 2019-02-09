/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/StorageProvider.h"

#include "hermes/Support/CheckedMalloc.h"
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
  void *newStorage(const char *name) override;
  void deleteStorage(void *storage) override;
};

class MallocStorageProvider final : public StorageProvider {
 public:
  void *newStorage(const char *name) override;
  void deleteStorage(void *storage) override;

 private:
  /// Map aligned starts to actual starts for freeing.
  /// NOTE: Since this is only used for debugging purposes, and it is rare to
  /// create and delete storage, it's ok to use a map.
  llvm::DenseMap<void *, void *> lowLimToAllocHandle_;
};

class PreAllocatedStorageProvider final : public StorageProvider {
 public:
  PreAllocatedStorageProvider(size_t totalAmount);
  ~PreAllocatedStorageProvider();

  void *newStorage(const char *name) override;
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

void *VMAllocateStorageProvider::newStorage(const char *name) {
  assert(AlignedStorage::size() % oscompat::page_size() == 0);
  // Allocate the space, hoping it will be the correct alignment.
  void *mem = oscompat::vm_allocate(AlignedStorage::size());
  if (!mem) {
    // Don't attempt to do anything further if the allocation failed.
    return nullptr;
  }
  if (LLVM_UNLIKELY(!isAligned(mem))) {
    // Free and try again with a larger amount to ensure we can align it.
    oscompat::vm_free(mem, AlignedStorage::size());
    mem = oscompat::vm_allocate_aligned(
        AlignedStorage::size(), AlignedStorage::size());
    assert(isAligned(mem));
  }
  // Name the memory region on platforms that support naming.
  oscompat::vm_name(mem, AlignedStorage::size(), name);
  return mem;
}

void VMAllocateStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  oscompat::vm_free(storage, AlignedStorage::size());
}

void *MallocStorageProvider::newStorage(const char *name) {
  // name is unused, can't name malloc memory.
  (void)name;
  void *mem = checkedMalloc(2 * AlignedStorage::size());
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

PreAllocatedStorageProvider::PreAllocatedStorageProvider(size_t totalAmount)
    : maxBytes_(totalAmount),
      start_(
          maxBytes_ ? static_cast<char *>(oscompat::vm_allocate_aligned(
                          maxBytes_,
                          AlignedStorage::size()))
                    : nullptr),
      end_(start_ ? start_ + maxBytes_ : nullptr),
      level_(start_) {
  assert(maxBytes_ % AlignedStorage::size() == 0 && "Un-aligned maxBytes");
}

PreAllocatedStorageProvider::~PreAllocatedStorageProvider() {
  if (start_) {
    oscompat::vm_free(start_, maxBytes_);
  }
}

void *PreAllocatedStorageProvider::newStorage(const char *name) {
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
    return nullptr;
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
std::unique_ptr<StorageProvider> StorageProvider::defaultProvider(
    size_t maxAmount) {
  assert(
      maxAmount % AlignedStorage::size() == 0 &&
      "maxAmount must be a multiple of AlignedStorage::size()");
#ifdef HERMESVM_FLAT_ADDRESS_SPACE
  // On 64-bit builds, we have plenty of VA, allocate it before-hand.
  return preAllocatedProvider(maxAmount);
#else
  // On 32-bit builds, we have limited VA. Allocate
  // each segment as it's needed.
  return mmapProvider();
#endif
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::preAllocatedProvider(
    size_t amount) {
  assert(
      amount % AlignedStorage::size() == 0 &&
      "amount must be a multiple of AlignedStorage::size()");
  return std::unique_ptr<StorageProvider>(
      new PreAllocatedStorageProvider(amount));
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mmapProvider() {
  return std::unique_ptr<StorageProvider>(new VMAllocateStorageProvider);
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mallocProvider() {
  return std::unique_ptr<StorageProvider>(new MallocStorageProvider);
}

} // namespace vm
} // namespace hermes
