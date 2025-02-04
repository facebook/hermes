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
#include "hermes/VM/AlignedHeapSegment.h"

#include "llvh/ADT/BitVector.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/MathExtras.h"

#include <cassert>
#include <limits>
#include <random>
#include <stack>

#if (defined(__linux__) || defined(__ANDROID__)) && defined(__aarch64__)
/* On Linux on ARM64 we most likely have at least 39 bits of virtual address
 * space https://github.com/torvalds/linux/blob/v6.7/arch/arm64/Kconfig#L1262 If
 * our mmap hint is above 2**39 it will likely fail. */
#define MAX_ADDR_HINT 0x37FFFFFFFF
#elif defined(__APPLE__) && defined(__aarch64__)
/* On ios/arm64 assume we have at least 39 bits of virtual address space  (
 * similar to linux on arm64). This should be true for all iOS versions >=14
 * (https://github.com/golang/go/issues/46860), older versions <14 are
 * unsupported. Note that the effective addressable space might vary, depending
 * on apps entitelmnets as well as various other factors, hence we go for a
 * conservative 39 bit address space limit, which is sufficient for most
 * applications and should be good enough for this purpose.
 */
#define MAX_ADDR_HINT 0x37FFFFFFFF
#elif (defined(__linux__) || defined(__ANDROID__)) && defined(__amd64__)
#define MAX_ADDR_HINT 0x3FFFFFFFFFFF
#elif defined(_WIN64)
/* On Windows use a 37 bit address space limit as this is the lowest
 * configuration for Windows Home
 * https://learn.microsoft.com/en-us/windows/win32/memory/memory-limits-for-windows-releases
 */
#define MAX_ADDR_HINT 0x1FFFFFFFFF
#else
/* For other non-explicitly listed configuration, be extra conservative and use
 * a 32 bit address space limit. */
#define MAX_ADDR_HINT 0xFFFFFFFF
#endif

namespace hermes {
namespace vm {

namespace {

/// Minimum segment storage size. Any larger segment size should be a multiple
/// of it.
constexpr auto kSegmentUnitSize = AlignedHeapSegment::kSegmentUnitSize;

bool isAligned(void *p) {
  return (reinterpret_cast<uintptr_t>(p) & (kSegmentUnitSize - 1)) == 0;
}

char *alignAlloc(void *p) {
  return reinterpret_cast<char *>(
      llvh::alignTo(reinterpret_cast<uintptr_t>(p), kSegmentUnitSize));
}

void *getMmapHint() {
  uintptr_t addr = std::random_device()();
  if constexpr (sizeof(uintptr_t) >= 8) {
    // std::random_device() yields an unsigned int, so combine two.
    addr = (addr << 32) | std::random_device()();
    // Don't use the entire address space, to ensure this is a valid address.
    addr &= MAX_ADDR_HINT;
  }
  return alignAlloc(reinterpret_cast<void *>(addr));
}

class VMAllocateStorageProvider final : public StorageProvider {
 public:
  llvh::ErrorOr<void *> newStorageImpl(size_t sz, const char *name) override;
  void deleteStorageImpl(void *storage, size_t sz) override;
};

class ContiguousVAStorageProvider final : public StorageProvider {
 public:
  ContiguousVAStorageProvider(size_t size)
      : size_(llvh::alignTo<kSegmentUnitSize>(size)),
        statusBits_(size_ / kSegmentUnitSize + 1) {
    auto result =
        oscompat::vm_reserve_aligned(size_, kSegmentUnitSize, getMmapHint());
    if (!result)
      hermes_fatal("Contiguous storage allocation failed.", result.getError());
    start_ = static_cast<char *>(*result);
    oscompat::vm_name(start_, size_, kFreeRegionName);
    // Set the additional bit so that we could always find a next used bit.
    statusBits_.set(statusBits_.size() - 1);
  }
  ~ContiguousVAStorageProvider() override {
    oscompat::vm_release_aligned(start_, size_);
  }

  llvh::ErrorOr<void *> newStorageImpl(size_t sz, const char *name) override {
    assert(
        statusBits_.find_first_unset() == firstFreeBit_ &&
        "firstFreeBit_ should always be the first unset bit");

    // No available space to use.
    if (LLVM_UNLIKELY(firstFreeBit_ == -1)) {
      return make_error_code(OOMError::MaxStorageReached);
    }

    void *storage;
    int numUnits = sz / kSegmentUnitSize;
    int curFreeBit = firstFreeBit_;
    // Search for a large enough continuous bit range.
    while (true) {
      int nextUsedBit = statusBits_.find_next(firstFreeBit_);
      // Note that because we add a past-the-end bit that is always set,
      // nextUsedBit will never be -1.
      if (nextUsedBit - curFreeBit >= numUnits)
        break;
      curFreeBit = statusBits_.find_next_unset(nextUsedBit);
      if (curFreeBit == -1) {
        return make_error_code(OOMError::TooLargeSegmentRequest);
      }
    }

    storage = start_ + curFreeBit * kSegmentUnitSize;
    statusBits_.set(curFreeBit, curFreeBit + numUnits);
    // Reset it to the new leftmost free bit.
    firstFreeBit_ = statusBits_.find_next_unset(firstFreeBit_);

    auto res = oscompat::vm_commit(storage, sz);
    if (res) {
      oscompat::vm_name(storage, sz, name);
    }
    return res;
  }

  void deleteStorageImpl(void *storage, size_t sz) override {
    assert(
        !llvh::alignmentAdjustment(storage, kSegmentUnitSize) &&
        "Storage not aligned");
    assert(
        storage >= start_ && storage < start_ + size_ &&
        "Storage not in region");
    oscompat::vm_name(storage, sz, kFreeRegionName);
    oscompat::vm_uncommit(storage, sz);
    size_t numUnits = sz / kSegmentUnitSize;
    // Reset all bits for this storage.
    int startIndex = (static_cast<char *>(storage) - start_) / kSegmentUnitSize;
    statusBits_.reset(startIndex, startIndex + numUnits);
    if (startIndex < firstFreeBit_)
      firstFreeBit_ = startIndex;
  }

 private:
  static constexpr const char *kFreeRegionName = "hermes-free-heap";
  size_t size_;
  char *start_;
  /// First free bit in \c statusBits_. We always make new allocation from the
  /// leftmost free bit, based on heuristics:
  /// 1. Usually the reserved address space is not full.
  /// 2. Storage with size kSegmentUnitSize is allocated and deleted more
  /// frequently than larger storage.
  /// 3. Likely small storage will find space available from leftmost free bit,
  /// leaving enough space at the right side for large storage.
  /// If no free bit available, it will be -1.
  int firstFreeBit_{0};
  /// One bit for each kSegmentUnitSize space in the entire reserved virtual
  /// address space. A bit is set if the corresponding space is used. One
  /// additional set bit is added in the end, to make finding next set bit
  /// easier (always return the last index instead of -1).
  llvh::BitVector statusBits_;
};

class MallocStorageProvider final : public StorageProvider {
 public:
  llvh::ErrorOr<void *> newStorageImpl(size_t sz, const char *name) override;
  void deleteStorageImpl(void *storage, size_t sz) override;

 private:
  /// Map aligned starts to actual starts for freeing.
  /// NOTE: Since this is only used for debugging purposes, and it is rare to
  /// create and delete storage, it's ok to use a map.
  llvh::DenseMap<void *, void *> lowLimToAllocHandle_;
};

llvh::ErrorOr<void *> VMAllocateStorageProvider::newStorageImpl(
    size_t sz,
    const char *name) {
  assert(kSegmentUnitSize % oscompat::page_size() == 0);
  // Allocate the space, hoping it will be the correct alignment.
  auto result =
      oscompat::vm_allocate_aligned(sz, kSegmentUnitSize, getMmapHint());
  if (!result) {
    return result;
  }
  void *mem = *result;
  assert(isAligned(mem));
  (void)&isAligned;
  oscompat::vm_hugepage(mem, sz);
  // Name the memory region on platforms that support naming.
  oscompat::vm_name(mem, sz, name);
  return mem;
}

void VMAllocateStorageProvider::deleteStorageImpl(void *storage, size_t sz) {
  if (!storage) {
    return;
  }
  oscompat::vm_free_aligned(storage, sz);
}

llvh::ErrorOr<void *> MallocStorageProvider::newStorageImpl(
    size_t sz,
    const char *name) {
  // name is unused, can't name malloc memory.
  (void)name;
  // Allocate size of sz + kSegmentUnitSize so that we could get an address
  // aligned to kSegmentUnitSize.
  void *mem = checkedMalloc(sz + kSegmentUnitSize);
  void *lowLim = alignAlloc(mem);
  assert(isAligned(lowLim) && "New storage should be aligned");
  lowLimToAllocHandle_[lowLim] = mem;
  return lowLim;
}

void MallocStorageProvider::deleteStorageImpl(void *storage, size_t sz) {
  // free() does not need the memory size.
  (void)sz;
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

llvh::ErrorOr<void *> StorageProvider::newStorage(size_t sz, const char *name) {
  assert(
      sz && (sz % kSegmentUnitSize == 0) &&
      "Allocated storage size must be multiples of kSegmentUnitSize");
  auto res = newStorageImpl(sz, name);

  if (res) {
    numSucceededAllocs_++;
  } else {
    numFailedAllocs_++;
  }

  return res;
}

void StorageProvider::deleteStorage(void *storage, size_t sz) {
  if (!storage) {
    return;
  }

  assert(
      sz && (sz % kSegmentUnitSize == 0) &&
      "Allocated storage size must be multiples of kSegmentUnitSize");

  numDeletedAllocs_++;
  return deleteStorageImpl(storage, sz);
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
