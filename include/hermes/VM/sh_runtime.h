/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_RUNTIME_H
#define HERMES_SH_RUNTIME_H

#include "hermes/VM/sh_segment_info.h"

#include "libhermesvm-config.h"

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SHJmpBuf {
  struct SHJmpBuf *prev;
  jmp_buf buf;
} SHJmpBuf;

/// This class represents the context of the VM that is shared between generated
/// SH code and the VM implementation itself. It is intended to serve as the
/// base class of the C++ Runtime class.
typedef struct SHRuntime {
#if defined(HERMESVM_COMPRESSED_POINTERS) && !defined(HERMESVM_CONTIGUOUS_HEAP)
  /// To support 32-bit GC pointers in segmentIdx:offset form, segmentMap maps
  /// segment indices to "biased segment starts." This "bias" speeds up the
  /// decoding of a compressed pointer. If the segmentMap contained the actual
  /// segment starts, decoding an offset would require a shift to extract the
  /// segmentIdx, a lookup of the segment start, a subtraction of the shifted
  /// index to obtain the offset, and an addition to get the final native
  /// pointer. That is,
  ///   segmentIdx = basedPtr >> kLogSize;
  ///   segmentStart = segmentMap_[segmentIdx]
  ///   offset = basedPtr - (segmentIdx << kLogSize)
  ///   native_ptr = segmentStart + offset
  ///
  /// The bias allows us to avoid the subtraction: the segment table entry for
  /// index i is <segment-start> - (i << kLogSize) So the calculation can be:
  ///   segmentIdx = based_ptr >> kLogSize;
  ///   biasedSegmentStart = segmentMap_[segmentIdx]
  ///       [== segmentStart - (segmentIdx << kLogSize)]
  ///   native_ptr = basedPtr + biasedSegmentStart
  ///       [== basedPtr + segmentStart - (segmentIdx << kLogSize)
  ///        == segmentStart + (basedPtr - (segmentIdx << kLogSize))
  ///        == segmentStart + offset]
  ///
  /// Because we use the same 8-byte alignment in the offsets as we do in object
  /// pointers, the max heap size for 32-bit pointers is 4GB. With 4MB segments,
  /// that's 1024 segments, with the first slot reserved for null.
  void *segmentMap[1 << (32 - HERMESVM_LOG_HEAP_SEGMENT_SIZE)];
#else
  /// Dummy field to avoid warnings about an empty struct having different sizes
  /// in C and C++. Delete it once we add other fields.
  uint8_t dummy;
#endif

  /// The current top of the exception handler stack.
  SHJmpBuf *shCurJmpBuf;
} SHRuntime;

#ifdef HERMESVM_COMPRESSED_POINTERS

/// A struct representing a compressed pointer in C code.
typedef struct SHCompressedPointer {
  uint32_t raw;
} SHCompressedPointer;

#ifdef HERMESVM_CONTIGUOUS_HEAP

/// Encode the given non-null \p ptr as a compressed pointer and return it. This
/// is more efficient that _sh_cp_encode since we can elide the null check.
static inline SHCompressedPointer _sh_cp_encode_non_null(
    SHRuntime *shr,
    void *ptr) {
  assert(ptr && "Pointer must be non-null");
  // In contiguous heap mode, a non-null pointer is the offset from the start of
  // the heap, represented by shr.
  SHCompressedPointer cp = {(uint32_t)((uintptr_t)ptr - (uintptr_t)shr)};
  return cp;
}

/// Encode the given \p ptr as a compressed pointer and return the new value.
static inline SHCompressedPointer _sh_cp_encode(SHRuntime *shr, void *ptr) {
  // Null pointers require an additional check, since they are not in the range
  // of the contiguous heap.
  if (ptr)
    return _sh_cp_encode_non_null(shr, ptr);
  SHCompressedPointer cp = {0};
  return cp;
}

/// Decode the non-null compressed pointer \p ptr into a normal pointer and
/// return it. This is more efficient than _sh_cp_decode since we can elide the
/// null check.
static inline void *_sh_cp_decode_non_null(
    SHRuntime *shr,
    SHCompressedPointer ptr) {
  assert(ptr.raw && "Pointer must be non-null");
  // Decode the pointer by adding the offset back to the base.
  return (void *)(ptr.raw + (uintptr_t)shr);
}

/// Decode the compressed pointer \p ptr into a normal pointer and return it.
static inline void *_sh_cp_decode(SHRuntime *shr, SHCompressedPointer ptr) {
  // Null pointers are encoded as 0, so check for them directly.
  if (ptr.raw)
    return _sh_cp_decode_non_null(shr, ptr);
  return 0;
}

#else

/// Encode the given non-null \p ptr as a compressed pointer and return it. This
/// is more efficient that _sh_cp_encode since we can elide the null check.
static inline SHCompressedPointer _sh_cp_encode_non_null(
    SHRuntime *shr,
    void *ptr) {
  assert(ptr && "Pointer must be non-null");
  uintptr_t uptr = (uintptr_t)ptr;
  uintptr_t offsetMask = (1 << HERMESVM_LOG_HEAP_SEGMENT_SIZE) - 1;
  // Calculate the offset from the segment start.
  uint32_t offset = uptr & offsetMask;
  // Now get the segment index, and shift it so that its bits do not overlap
  // with those of offset.
  // TODO: Store the shifted index in SegmentInfo to save an operation here.
  SHSegmentInfo *info = (SHSegmentInfo *)(uptr & ~offsetMask);
  SHCompressedPointer cp;
  cp.raw = (info->index << HERMESVM_LOG_HEAP_SEGMENT_SIZE) | offset;
  return cp;
}

/// Encode the given \p ptr as a compressed pointer and return the new value.
static inline SHCompressedPointer _sh_cp_encode(SHRuntime *shr, void *ptr) {
  // Encoding relies on retrieving information from the start of the segment. We
  // need to explicitly check for null and encode it as 0, since it is not in a
  // segment.
  if (ptr)
    return _sh_cp_encode_non_null(shr, ptr);
  SHCompressedPointer cp = {0};
  return cp;
}

/// Decode the compressed pointer \p ptr into a normal pointer and return it.
static inline void *_sh_cp_decode(SHRuntime *shr, SHCompressedPointer ptr) {
  // Get the segment index by shifting out the low bits.
  unsigned segIdx = ptr.raw >> HERMESVM_LOG_HEAP_SEGMENT_SIZE;
  // Index into the segment map to retrieve the base address, and add the
  // compressed pointer to it.
  char *segBase = (char *)shr->segmentMap[segIdx];
  return segBase + ptr.raw;
}

/// Decode the non-null compressed pointer \p ptr into a normal pointer and
/// return it.
static inline void *_sh_cp_decode_non_null(
    SHRuntime *shr,
    SHCompressedPointer ptr) {
  assert(ptr.raw && "Pointer must be non-null");
  return _sh_cp_decode(shr, ptr);
}

#endif
#else

/// A struct that wraps an ordinary pointer, to be used in place of a compressed
/// pointer when compressed pointers are disabled.
typedef struct SHCompressedPointer {
  uintptr_t raw;
} SHCompressedPointer;

/// Wrap the given \p ptr in an SHCompressedPointer and return it.
static inline SHCompressedPointer _sh_cp_encode(SHRuntime *shr, void *ptr) {
  SHCompressedPointer cp = {(uintptr_t)ptr};
  return cp;
}

/// Wrap the given non-null \p ptr in an SHCompressedPointer and return it.
static inline SHCompressedPointer _sh_cp_encode_non_null(
    SHRuntime *shr,
    void *ptr) {
  assert(ptr && "Pointer must be non-null");
  return _sh_cp_encode(shr, ptr);
}

/// Unwrap the given \p ptr into a normal pointer and return it.
static inline void *_sh_cp_decode(SHRuntime *shr, SHCompressedPointer ptr) {
  return (void *)ptr.raw;
}

/// Unwrap the given non-null \p ptr into a normal pointer and return it.
static inline void *_sh_cp_decode_non_null(
    SHRuntime *shr,
    SHCompressedPointer ptr) {
  assert(ptr.raw && "Pointer must be non-null");
  return _sh_cp_decode(shr, ptr);
}

#endif

#ifdef __cplusplus
}
#endif
#endif
