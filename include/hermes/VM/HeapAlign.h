/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HEAPALIGN_H
#define HERMES_VM_HEAPALIGN_H

#include "hermes/Public/GCConfig.h"

#include "llvh/Support/MathExtras.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

static const uint32_t LogHeapAlign = 3;
static const uint32_t HeapAlign = 1 << LogHeapAlign;

/// Align requested size according to the alignment requirement of the GC.
constexpr inline uint32_t heapAlignSize(gcheapsize_t size) {
  return llvh::alignTo<HeapAlign>(size);
}

/// Return true if the requested size is properly aligned according to the
/// alignment requirement of the GC.
constexpr inline bool isSizeHeapAligned(gcheapsize_t size) {
  return (size & (HeapAlign - 1)) == 0;
}

} // namespace vm
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_VM_HEAP_ALIGN_H
