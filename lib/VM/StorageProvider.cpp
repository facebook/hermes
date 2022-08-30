/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StorageProvider.h"

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedStorage.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/MathExtras.h"

#include <cassert>
#include <limits>
#include <random>
#include <stack>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

namespace {

bool isAligned(void *p) {
  return (reinterpret_cast<uintptr_t>(p) & (AlignedStorage::size() - 1)) == 0;
}

char *alignAlloc(void *p) {
  return reinterpret_cast<char *>(
      llvh::alignTo(reinterpret_cast<uintptr_t>(p), AlignedStorage::size()));
}

void *getMmapHint() {
  uintptr_t addr = std::random_device()();
  if constexpr (sizeof(uintptr_t) >= 8) {
    // std::random_device() yields an unsigned int, so combine two.
    addr = (addr << 32) | std::random_device()();
    // Don't use the entire address space, to prevent too much fragmentation.
    addr &= std::numeric_limits<uintptr_t>::max() >> 18;
  }
  return alignAlloc(reinterpret_cast<void *>(addr));
}

class VMAllocateStorageProvider final : public StorageProvider {
 public:
  llvh::ErrorOr<void *> newStorageImpl(const char *name) override;
  void deleteStorageImpl(void *storage) override;
};

class ContiguousVAStorageProvider final : public StorageProvider {
 public:
  ContiguousVAStorageProvider(size_t size)
      : size_(llvh::alignTo<AlignedStorage::size()>(size)) {
    auto result = oscompat::vm_reserve_aligned(
        size_, AlignedStorage::size(), getMmapHint());
    if (!result)
      hermes_fatal("Contiguous storage allocation failed.", result.getError());
    level_ = start_ = static_cast<char *>(*result);
    oscompat::vm_name(start_, size_, kFreeRegionName);
  }
  ~ContiguousVAStorageProvider() override {
    oscompat::vm_release_aligned(start_, size_);
  }

  llvh::ErrorOr<void *> newStorageImpl(const char *name) override {
    void *storage;
    if (!freelist_.empty()) {
      storage = freelist_.back();
      freelist_.pop_back();
    } else if (level_ < start_ + size_) {
      storage = std::exchange(level_, level_ + AlignedStorage::size());
    } else {
      return make_error_code(OOMError::MaxStorageReached);
    }
    auto res = oscompat::vm_commit(storage, AlignedStorage::size());
    if (res) {
      oscompat::vm_name(storage, AlignedStorage::size(), name);
    }
    return res;
  }

  void deleteStorageImpl(void *storage) override {
    assert(
        !llvh::alignmentAdjustment(storage, AlignedStorage::size()) &&
        "Storage not aligned");
    assert(storage >= start_ && storage < level_ && "Storage not in region");
    oscompat::vm_name(storage, AlignedStorage::size(), kFreeRegionName);
    oscompat::vm_uncommit(storage, AlignedStorage::size());
    freelist_.push_back(storage);
  }

 private:
  static constexpr const char *kFreeRegionName = "hermes-free-heap";
  size_t size_;
  char *start_;
  char *level_;
  llvh::SmallVector<void *, 0> freelist_;
};

class MallocStorageProvider final : public StorageProvider {
 public:
  llvh::ErrorOr<void *> newStorageImpl(const char *name) override;
  void deleteStorageImpl(void *storage) override;

 private:
  /// Map aligned starts to actual starts for freeing.
  /// NOTE: Since this is only used for debugging purposes, and it is rare to
  /// create and delete storage, it's ok to use a map.
  llvh::DenseMap<void *, void *> lowLimToAllocHandle_;
};

llvh::ErrorOr<void *> VMAllocateStorageProvider::newStorageImpl(
    const char *name) {
  assert(AlignedStorage::size() % oscompat::page_size() == 0);
  // Allocate the space, hoping it will be the correct alignment.
  auto result = oscompat::vm_allocate_aligned(
      AlignedStorage::size(), AlignedStorage::size(), getMmapHint());
  if (!result) {
    return result;
  }
  void *mem = *result;
  assert(isAligned(mem));
  (void)isAligned;
#ifdef HERMESVM_ALLOW_HUGE_PAGES
  oscompat::vm_hugepage(mem, AlignedStorage::size());
#endif

  // Name the memory region on platforms that support naming.
  oscompat::vm_name(mem, AlignedStorage::size(), name);
  return mem;
}

void VMAllocateStorageProvider::deleteStorageImpl(void *storage) {
  if (!storage) {
    return;
  }
  oscompat::vm_free_aligned(storage, AlignedStorage::size());
}

llvh::ErrorOr<void *> MallocStorageProvider::newStorageImpl(const char *name) {
  // name is unused, can't name malloc memory.
  (void)name;
  void *mem = checkedMalloc2(AlignedStorage::size(), 2u);
  void *lowLim = alignAlloc(mem);
  assert(isAligned(lowLim) && "New storage should be aligned");
  lowLimToAllocHandle_[lowLim] = mem;
  return lowLim;
}

void MallocStorageProvider::deleteStorageImpl(void *storage) {
  if (!storage) {
    return;
  }
  free(lowLimToAllocHandle_[storage]);
  lowLimToAllocHandle_.erase(storage);
}

} // namespace

StorageProvider::~StorageProvider() {
  assert(numLiveAllocs() == 0);
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mmapProvider() {
  return std::unique_ptr<StorageProvider>(new VMAllocateStorageProvider);
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::contiguousVAProvider(
    size_t size) {
  return std::make_unique<ContiguousVAStorageProvider>(size);
}

/* static */
std::unique_ptr<StorageProvider> StorageProvider::mallocProvider() {
  return std::unique_ptr<StorageProvider>(new MallocStorageProvider);
}

llvh::ErrorOr<void *> StorageProvider::newStorage(const char *name) {
  auto res = newStorageImpl(name);

  if (res) {
    numSucceededAllocs_++;
  } else {
    numFailedAllocs_++;
  }

  return res;
}

void StorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }

  numDeletedAllocs_++;
  deleteStorageImpl(storage);
}

llvh::ErrorOr<std::pair<void *, size_t>>
vmAllocateAllowLess(size_t sz, size_t minSz, size_t alignment) {
  assert(sz >= minSz && "Shouldn't supply a lower size than the minimum");
  assert(minSz != 0 && "Minimum size must not be zero");
  assert(sz == llvh::alignTo(sz, alignment));
  assert(minSz == llvh::alignTo(minSz, alignment));
  // Try fractions of the requested size, down to the minimum.
  // We'll do it by eighths.
  assert(sz >= 8); // Since sz is page-aligned, safe assumption.
  const size_t increment = sz / 8;
  // Store the result for the case where all attempts fail.
  llvh::ErrorOr<void *> result{std::error_code{}};
  while (sz >= minSz) {
    result = oscompat::vm_allocate_aligned(sz, alignment, getMmapHint());
    if (result) {
      assert(
          sz == llvh::alignTo(sz, alignment) &&
          "Should not return an un-aligned size");
      return std::make_pair(result.get(), sz);
    }
    if (sz < increment || sz == minSz) {
      // Would either underflow or can't reduce any lower.
      break;
    }
    sz = std::max(
        static_cast<size_t>(llvh::alignDown(sz - increment, alignment)), minSz);
  }
  assert(!result && "Must be an error if none of the allocations succeeded");
  return result.getError();
}

size_t StorageProvider::numSucceededAllocs() const {
  return numSucceededAllocs_;
}

size_t StorageProvider::numFailedAllocs() const {
  return numFailedAllocs_;
}

size_t StorageProvider::numDeletedAllocs() const {
  return numDeletedAllocs_;
}

size_t StorageProvider::numLiveAllocs() const {
  return numSucceededAllocs_ - numDeletedAllocs_;
}

} // namespace vm
} // namespace hermes
