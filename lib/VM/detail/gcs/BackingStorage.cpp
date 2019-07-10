/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/detail/BackingStorage.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/MathExtras.h"

#include <cassert>
#include <cstdlib>

using namespace hermes;

namespace hermes {
namespace vm {
namespace detail {

namespace {

/// Returns a tuple of a pointer to the allocated memory, the original pointer
/// in case it was modified to be page-aligned, and the size actually allocated,
/// which may be smaller than the requested size.
std::tuple<char *, void *, size_t>
allocForStorage(gcheapsize_t sz, gcheapsize_t minSz, AllocSource src) {
  size_t pageSize = oscompat::page_size();
  assert(sz == llvm::alignTo(sz, pageSize));
  assert(minSz == llvm::alignTo(minSz, pageSize));
  switch (src) {
    case AllocSource::VMAllocate: {
      // Try fractions of the requested size, down to the minimum.
      // We'll do it by eighths.
      assert(sz >= 8); // Since sz is page-aligned, save assumption.
      gcheapsize_t increment = sz / 8;
      // Store the result for the case where all attempts fail.
      llvm::ErrorOr<void *> result{std::error_code{}};
      while (sz >= minSz) {
        result = oscompat::vm_allocate(sz);
        if (result) {
          // No such thing as original pointer for VMAllocate, so just
          // duplicate the returned pointer.
          return std::make_tuple(
              static_cast<char *>(result.get()), result.get(), sz);
        }
        if (sz == minSz)
          break;
        sz = std::max(
            static_cast<gcheapsize_t>(llvm::alignTo(sz - increment, pageSize)),
            minSz);
      }
      hermes_fatal(
          (llvm::Twine("vm_allocate() failed to allocate backing storage: ") +
           convert_error_to_message(result.getError()))
              .str());
    }

    case AllocSource::Malloc:
      // Ask for one more page than we need, in order to enforce that it begins
      // on a page boundary.
      if (auto *origPointer = static_cast<char *>(malloc(sz + pageSize))) {
        // Round up to the nearest page to start.
        auto *res = reinterpret_cast<char *>(
            llvm::alignTo(reinterpret_cast<uintptr_t>(origPointer), pageSize));
        return std::make_tuple(res, origPointer, sz);
      }
      hermes_fatal("malloc() failed to allocate backing storage");
  }

  llvm_unreachable("Unrecognized backing storage source.");
  return std::make_tuple(nullptr, nullptr, 0);
}

} // anonymous namespace

void swap(BackingStorage &a, BackingStorage &b) {
  using std::swap;

  swap(a.lowLim_, b.lowLim_);
  swap(a.hiLim_, b.hiLim_);
  swap(a.origPointer_, b.origPointer_);
  swap(a.src_, b.src_);
}

BackingStorage::BackingStorage(
    gcheapsize_t sz,
    gcheapsize_t minSz,
    AllocSource src)
    : BackingStorage(allocForStorage(sz, minSz, src), src) {}

BackingStorage::BackingStorage(gcheapsize_t sz, AllocSource src)
    : BackingStorage(sz, /*min = desired*/ sz, src) {}

BackingStorage::BackingStorage(
    gcheapsize_t sz,
    gcheapsize_t minSz,
    const char *name)
    : BackingStorage(sz, minSz, AllocSource::VMAllocate) {
  oscompat::vm_name(lowLim_, hiLim_ - lowLim_, name);
}

BackingStorage::BackingStorage(gcheapsize_t sz, const char *name)
    : BackingStorage(sz, AllocSource::VMAllocate) {
  oscompat::vm_name(lowLim_, hiLim_ - lowLim_, name);
}

BackingStorage::BackingStorage(
    const std::tuple<char *, void *, size_t> &alloc,
    AllocSource src)
    : lowLim_(std::get<0>(alloc)),
      hiLim_(std::get<0>(alloc) + std::get<2>(alloc)),
      origPointer_(std::get<1>(alloc)),
      src_(src) {
  // Make sure that the pointer we receive is on a page boundary.
  assert(
      reinterpret_cast<uintptr_t>(lowLim_) % oscompat::page_size() == 0 &&
      "Got a pointer that does not start on a page boundary");
  assert(
      reinterpret_cast<uintptr_t>(hiLim_) % oscompat::page_size() == 0 &&
      "Got a pointer that does not end on a page boundary");
}

BackingStorage::BackingStorage(BackingStorage &&that) : BackingStorage() {
  swap(*this, that);
}

BackingStorage &BackingStorage::operator=(BackingStorage that) {
  swap(*this, that);
  return *this;
}

BackingStorage::~BackingStorage() {
  if (!lowLim_)
    return;

  switch (src_) {
    case AllocSource::VMAllocate:
      oscompat::vm_free(lowLim_, hiLim_ - lowLim_);
      break;

    case AllocSource::Malloc:
      free(origPointer_);
      break;
  }
}

void BackingStorage::markUnused(char *from, char *to) {
  assert(from <= to && "Unused region boundaries inverted");
  assert(lowLim_ <= from && to <= hiLim_ && "Unused region out-of-bounds");
  oscompat::vm_unused(from, to - from);
}

bool BackingStorage::contains(void *ptr) const {
  auto cptr = static_cast<char *>(ptr);
  return lowLim_ <= cptr && cptr < hiLim_;
}

} // namespace detail
} // namespace vm
} // namespace hermes
