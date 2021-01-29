/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HERMESVALUE_INLINE_H
#define HERMES_VM_HERMESVALUE_INLINE_H

#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValue.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

void HermesValue::setInGC(HermesValue hv, GC *gc) {
  setNoBarrier(hv);
  assert(gc->calledByGC());
}

template <typename T>
inline PinnedHermesValue &PinnedHermesValue::operator=(PseudoHandle<T> &&hv) {
  setNoBarrier(hv.getHermesValue());
  hv.invalidate();
  return *this;
}

template <typename NeedsBarriers>
GCHermesValue::GCHermesValue(HermesValue hv, GC *gc) : HermesValue{hv} {
  if (NeedsBarriers::value)
    gc->constructorWriteBarrier(this, hv);
}

template <typename NeedsBarriers>
GCHermesValue::GCHermesValue(HermesValue hv, GC *gc, std::nullptr_t)
    : HermesValue{hv} {
  assert(!hv.isPointer() || !hv.getPointer());
  if (NeedsBarriers::value)
    gc->constructorWriteBarrier(this, hv);
}

template <typename NeedsBarriers>
inline void GCHermesValue::set(HermesValue hv, GC *gc) {
  HERMES_SLOW_ASSERT(gc && "Need a GC parameter in case of a write barrier");
  if (hv.isPointer() && hv.getPointer()) {
    HERMES_SLOW_ASSERT(
        gc->validPointer(hv.getPointer()) &&
        "Setting an invalid pointer into a GCHermesValue");
    assert(
        NeedsBarriers::value ||
        !gc->needsWriteBarrier(this, static_cast<GCCell *>(hv.getPointer())));
  }
  if (NeedsBarriers::value)
    gc->writeBarrier(this, hv);
  setNoBarrier(hv);
}

void GCHermesValue::setNonPtr(HermesValue hv, GC *gc) {
  HERMES_SLOW_ASSERT(gc && "Need a GC parameter in case of a write barrier");
  assert(!hv.isPointer() || !hv.getPointer());
  gc->snapshotWriteBarrier(this);
  setNoBarrier(hv);
}

void GCHermesValue::unreachableWriteBarrier(GC *gc) {
  // Hades needs a snapshot barrier executed when something becomes unreachable.
  gc->snapshotWriteBarrier(this);
}

/*static*/
template <typename InputIt>
inline void
GCHermesValue::fill(InputIt start, InputIt end, HermesValue fill, GC *gc) {
  HERMES_SLOW_ASSERT(gc && "Need a GC parameter in case of a write barrier");
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      cur->set(fill, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      cur->setNonPtr(fill, gc);
    }
  }
}

/*static*/
template <typename InputIt>
inline void GCHermesValue::uninitialized_fill(
    InputIt start,
    InputIt end,
    HermesValue fill,
    GC *gc) {
  HERMES_SLOW_ASSERT(gc && "Need a GC parameter in case of a write barrier");
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      // Use the constructor write barrier. Assume it needs barriers.
      new (&*cur) GCHermesValue(fill, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      // Use a constructor that doesn't handle pointer values.
      new (&*cur) GCHermesValue(fill, gc, nullptr);
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

template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValue::uninitialized_copy(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC *gc) {
  for (; first != last; ++first, (void)++result) {
    new (&*result) GCHermesValue(*first, gc);
  }
  return result;
}

// Specializations using memmove can't be used in Hades, because the concurrent
// write barrier needs strict control over how assignments are done to HV fields
// which need to be atomically updated.
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
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
  // before the copies ensure that sufficient barriers are
  // performed.
  gc->writeBarrierRange(result, last - first);
  std::memmove(
      reinterpret_cast<void *>(result),
      first,
      (last - first) * sizeof(GCHermesValue));
  return result + (last - first);
}

/// Specialization for raw pointers to do a ranged write barrier.
template <>
inline GCHermesValue *GCHermesValue::uninitialized_copy(
    GCHermesValue *first,
    GCHermesValue *last,
    GCHermesValue *result,
    GC *gc) {
  gc->constructorWriteBarrierRange(result, last - first);
  // memmove is fine for an uninitialized copy.
  std::memmove(
      reinterpret_cast<void *>(result),
      first,
      (last - first) * sizeof(GCHermesValue));
  return result + (last - first);
}
#endif

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

inline void GCHermesValue::rangeUnreachableWriteBarrier(
    GCHermesValue *first,
    GCHermesValue *last,
    GC *gc) {
  gc->snapshotWriteBarrierRange(first, last - first);
}

inline void GCHermesValue::copyToPinned(
    const GCHermesValue *first,
    const GCHermesValue *last,
    PinnedHermesValue *result) {
  // memcpy for performance. Only safe if the types have the same layout.
  static_assert(
      std::is_convertible<PinnedHermesValue *, HermesValue *>::value &&
          std::is_convertible<GCHermesValue *, HermesValue *>::value &&
          sizeof(PinnedHermesValue) == sizeof(HermesValue) &&
          sizeof(GCHermesValue) == sizeof(HermesValue) &&
          sizeof(HermesValue) == 8,
      "memcpy between different layouts");
  std::memcpy(result, first, (last - first) * sizeof(PinnedHermesValue));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HERMESVALUE_INLINE_H
