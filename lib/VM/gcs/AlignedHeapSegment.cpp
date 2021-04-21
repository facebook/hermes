/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/AlignedHeapSegment.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HiddenClass.h"

#include "llvh/Support/MathExtras.h"

#include <algorithm>
#include <cassert>
#include <cstring>

using namespace hermes;

namespace hermes {
namespace vm {

void AlignedHeapSegment::Contents::protectGuardPage(
    oscompat::ProtectMode mode) {
  char *begin = &paddedGuardPage_[kGuardPagePadding];
  size_t size = sizeof(paddedGuardPage_) - kGuardPagePadding;
  size_t PS = oscompat::page_size();
  // Only protect if the actual system page size matches expectations.
  if (reinterpret_cast<uintptr_t>(begin) % PS == 0 && PS <= size) {
    oscompat::vm_protect(begin, PS, mode);
  }
}

AlignedHeapSegment::~AlignedHeapSegment() {
  if (lowLim() == nullptr) {
    return;
  }
  contents()->protectGuardPage(oscompat::ProtectMode::ReadWrite);
  contents()->~Contents();
  __asan_unpoison_memory_region(start(), end() - start());
}

void AlignedHeapSegment::markUnused(char *start, char *end) {
  assert(
      !llvh::alignmentAdjustment(start, oscompat::page_size()) &&
      !llvh::alignmentAdjustment(end, oscompat::page_size()));
  // Some kernels seems to require all pages in the mapping to have the same
  // permissions for the advise to "take", so suspend guard page protection
  // temporarily.
  contents()->protectGuardPage(oscompat::ProtectMode::ReadWrite);
  storage_.markUnused(start, end);
  contents()->protectGuardPage(oscompat::ProtectMode::None);
}

template <AdviseUnused MU>
void AlignedHeapSegment::setLevel(char *lvl) {
  assert(dbgContainsLevel(lvl));
  if (lvl < level_) {
#ifndef NDEBUG
    clear(lvl, level_);
#else
    if (MU == AdviseUnused::Yes) {
      const size_t PS = oscompat::page_size();
      auto nextPageAfter = reinterpret_cast<char *>(
          llvh::alignTo(reinterpret_cast<uintptr_t>(lvl), PS));
      auto nextPageBefore = reinterpret_cast<char *>(
          llvh::alignTo(reinterpret_cast<uintptr_t>(level_), PS));

      markUnused(nextPageAfter, nextPageBefore);
    }
#endif
  }
  level_ = lvl;
}

/// Explicit template instantiations for setLevel
template void AlignedHeapSegment::setLevel<AdviseUnused::Yes>(char *lvl);
template void AlignedHeapSegment::setLevel<AdviseUnused::No>(char *lvl);

template <AdviseUnused MU>
void AlignedHeapSegment::resetLevel() {
  setLevel<MU>(start());
#ifdef HERMES_EXTRA_DEBUG
  lastVTableSummaryLevel_ = start();
  lastVTableSummary_ = 0;
#endif
}

/// Explicit template instantiations for resetLevel
template void AlignedHeapSegment::resetLevel<AdviseUnused::Yes>();
template void AlignedHeapSegment::resetLevel<AdviseUnused::No>();

void AlignedHeapSegment::setEffectiveEnd(char *effectiveEnd) {
  assert(
      start() <= effectiveEnd && effectiveEnd <= end() &&
      "Must be valid end for segment.");
  assert(level() <= effectiveEnd && "Must not set effective end below level");
  effectiveEnd_ = effectiveEnd;
}

void AlignedHeapSegment::clearExternalMemoryCharge() {
  setEffectiveEnd(end());
}

void AlignedHeapSegment::growTo(size_t desired) {
  assert(desired <= maxSize() && "Cannot request more than the max size");
  assert(
      isSizeHeapAligned(desired) &&
      "Cannot grow to a size that's not heap aligned");

  if (size() >= desired) {
    return;
  }

  char *newEnd = start() + desired;
#ifndef NDEBUG
  clear(end_, newEnd);
#endif

  end_ = newEnd;
}

void AlignedHeapSegment::shrinkTo(size_t desired) {
  assert(desired > 0 && "precondition");
  assert(desired <= maxSize() && "Cannot request more than the max size");
  assert(desired >= used() && "Cannot shrink to less than used data.");

  if (size() <= desired) {
    return;
  }

  end_ = start() + desired;
}

bool AlignedHeapSegment::growToFit(size_t amount) {
  assert(
      isSizeHeapAligned(amount) &&
      "Cannot grow by a size that's not heap aligned");
  // Insufficient space
  if (static_cast<size_t>(hiLim() - level_) < amount) {
    return false;
  }

  growTo(llvh::alignTo(used() + amount, oscompat::page_size()));
  return true;
}

void AlignedHeapSegment::recreateCardTableBoundaries() {
  const char *ptr = start();
  const char *const lim = level();
  CardTable::Boundary boundary = cardTable().nextBoundary(ptr);

  assert(
      cardTable().indexToAddress(cardTable().addressToIndex(ptr)) == ptr &&
      "ptr must be card aligned.");

  while (ptr < lim) {
    const GCCell *cell = reinterpret_cast<const GCCell *>(ptr);
    const char *nextPtr = ptr + cell->getAllocatedSize();
    if (boundary.address() < nextPtr) {
      cardTable().updateBoundaries(&boundary, ptr, nextPtr);
    }
    ptr = nextPtr;
  }
}

#ifndef NDEBUG
bool AlignedHeapSegment::dbgContainsLevel(const void *lvl) const {
  return contains(lvl) || lvl == hiLim();
}

bool AlignedHeapSegment::validPointer(const void *p) const {
  return start() <= p && p < level() &&
      static_cast<const GCCell *>(p)->isValid();
}

void AlignedHeapSegment::clear() {
  clear(start(), end());
}

/* static */ void AlignedHeapSegment::clear(char *start, char *end) {
#if LLVM_ADDRESS_SANITIZER_BUILD
  __asan_poison_memory_region(start, end - start);
#else
  std::memset(start, kInvalidHeapValue, end - start);
#endif
}

/* static */ void AlignedHeapSegment::checkUnwritten(char *start, char *end) {
#if !LLVM_ADDRESS_SANITIZER_BUILD && defined(HERMES_SLOW_DEBUG)
  // Check that the space was not written into.
  std::for_each(
      start, end, [](char value) { assert(value == kInvalidHeapValue); });
#endif
}
#endif // !NDEBUG

} // namespace vm
} // namespace hermes
