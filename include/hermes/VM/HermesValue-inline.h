/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HERMESVALUE_INLINE_H
#define HERMES_VM_HERMESVALUE_INLINE_H

#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/HermesValue.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

void HermesValue::setInGC(HermesValue hv, GC *gc) {
  setNoBarrier(hv);
  assert(gc->inGC());
}

template <typename NeedsBarriers>
inline void GCHermesValue::set(HermesValue hv, GC *gc) {
  HERMES_SLOW_ASSERT(
      gc &&
      "Must pass a valid GC in case this is a pointer, if not use setNonPtr");
  if (hv.isPointer() && hv.getPointer()) {
    HERMES_SLOW_ASSERT(
        gc->validPointer(hv.getPointer()) &&
        "Setting an invalid pointer into a GCHermesValue");
    assert(
        NeedsBarriers::value || !gc->needsWriteBarrier(this, hv.getPointer()));
  }
  setNoBarrier(hv);
  if (NeedsBarriers::value)
    gc->writeBarrier(this, hv);
}

void GCHermesValue::setNonPtr(HermesValue hv) {
  assert(!hv.isPointer() || !hv.getPointer());
  setNoBarrier(hv);
}

/*static*/
template <typename InputIt>
inline void
GCHermesValue::fill(InputIt start, InputIt end, HermesValue fill, GC *gc) {
  if (fill.isPointer()) {
    assert(gc && "must provide GC for pointer fill.");
    for (auto cur = start; cur != end; ++cur) {
      cur->set(fill, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      cur->setNonPtr(fill);
    }
  }
}

template <typename InputIt, typename OutputIt>
inline OutputIt
GCHermesValue::copy(InputIt first, InputIt last, OutputIt result, GC *gc) {
  for (; first != last; ++first, (void)++result) {
    result->set(*first, gc);
  }
  return result;
}

/// Specialization for raw pointers to do a ranged write barrier.
template <>
inline GCHermesValue *GCHermesValue::copy(
    GCHermesValue *first,
    GCHermesValue *last,
    GCHermesValue *result,
    GC *gc) {
  // We must use "raw" function such as memmove here, rather than a
  // function like std::copy (or copy_backward) that respects
  // constructors and operator=.  For HermesValue, those require the
  // contents not to contain pointers.  The range write barrier
  // after the the copies ensure that sufficient barriers are
  // performed.
  std::memmove(
      reinterpret_cast<void *>(result),
      first,
      (last - first) * sizeof(GCHermesValue));
  gc->writeBarrierRange(result, last - first);
  return result + (last - first);
}

template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValue::copy_backward(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC *gc) {
  while (first != last) {
    (--result)->set(*--last, gc);
  }
  return result;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HERMESVALUE_INLINE_H
