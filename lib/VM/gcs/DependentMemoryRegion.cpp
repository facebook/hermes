/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/DependentMemoryRegion.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/GC.h"
#include "llvm/Support/MathExtras.h"

#include <cassert>

using namespace hermes;

namespace hermes {
namespace vm {

DependentMemoryRegion::DependentMemoryRegion(
    const char *name,
    char *parentLowLim,
    char *parentStart,
    char *parentEnd,
    char *parentHiLim,
    size_t logSizeRatio)
    : parentLowLim_(parentLowLim),
      parentStart_(parentStart),
      parentEnd_(parentEnd),
      parentHiLim_(parentHiLim),
      logSizeRatio_(logSizeRatio),
      storage_(maxSize(parentHiLim, parentLowLim, logSizeRatio), name),
      start_(pageStartForParentPtr(parentStart)),
      end_(pageEndForParentPtr(parentEnd)) {
  // Preconditions:
  assert((parentHiLim - parentLowLim) % oscompat::page_size() == 0);
  assert((parentEnd - parentStart) % oscompat::page_size() == 0);
  assert(
      parentLowLim <= parentStart && parentStart <= parentEnd &&
      parentEnd <= parentHiLim);
}

void DependentMemoryRegion::parentDidAdviseUnused(char *from, char *to) {
  assert(from <= to && "Bounds inverted");
  assert(parentLowLim_ <= from && "Region underflow");
  assert(to <= parentHiLim_ && "Region overflow");

  // We mark from the end of the page containing \p from because we must assume
  // that there are used addresses immediately before \p from.
  storage_.markUnused(pageEndForParentPtr(from), pageEndForParentPtr(to));
}

char *DependentMemoryRegion::pageStartForParentPtr(char *ptr) const {
  const size_t PS = oscompat::page_size();
  size_t offset = (ptr - parentLowLim_) >> logSizeRatio_;

  // Round the gap down to a page multiple.
  return lowLim() + llvm::alignDown(offset, PS);
}

char *DependentMemoryRegion::pageEndForParentPtr(char *ptr) const {
  const size_t PS = oscompat::page_size();
  size_t offset = (ptr - parentLowLim_) >> logSizeRatio_;

  // Round the gap up to a page multiple.
  return lowLim() + llvm::alignTo(offset, PS);
}

/*static*/
size_t DependentMemoryRegion::maxSize(
    char *parentHiLim,
    char *parentLowLim,
    size_t logSizeRatio) {
  size_t parentMaxSize = parentHiLim - parentLowLim;
  return llvm::alignTo(parentMaxSize >> logSizeRatio, oscompat::page_size());
}

} // namespace vm
} // namespace hermes
