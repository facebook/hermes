/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

void HermesValue::setInGC(HermesValue hv, GC &gc) {
  setNoBarrier(hv);
  assert(gc.calledByGC());
}

template <typename T>
inline PinnedHermesValue &PinnedHermesValue::operator=(PseudoHandle<T> &&hv) {
  setNoBarrier(hv.getHermesValue());
  hv.invalidate();
  return *this;
}

template <typename HVType>
template <typename NeedsBarriers>
GCHermesValueBase<HVType>::GCHermesValueBase(HVType hv, GC &gc) : HVType{hv} {
  assert(!hv.isPointer() || hv.getPointer());
  if (NeedsBarriers::value)
    gc.constructorWriteBarrier(this, hv);
}

template <typename HVType>
template <typename NeedsBarriers>
GCHermesValueBase<HVType>::GCHermesValueBase(HVType hv, GC &gc, std::nullptr_t)
    : HVType{hv} {
  assert(!hv.isPointer());
  // No need to invoke any write barriers here, since the old value is
  // uninitialized (so the snapshot barrier does not apply), and the new value
  // is not a pointer (so the generational/relocation barrier does not apply).
}

template <typename HVType>
template <typename NeedsBarriers>
inline void GCHermesValueBase<HVType>::set(HVType hv, GC &gc) {
  if (hv.isPointer()) {
    HERMES_SLOW_ASSERT(
        gc.validPointer(hv.getPointer(gc.getPointerBase())) &&
        "Setting an invalid pointer into a GCHermesValue");
    assert(
        NeedsBarriers::value ||
        !gc.needsWriteBarrier(
            this, static_cast<GCCell *>(hv.getPointer(gc.getPointerBase()))));
  }
  if (NeedsBarriers::value)
    gc.writeBarrier(this, hv);
  HVType::setNoBarrier(hv);
}

template <typename HVType>
void GCHermesValueBase<HVType>::setNonPtr(HVType hv, GC &gc) {
  assert(!hv.isPointer());
  gc.snapshotWriteBarrier(this);
  HVType::setNoBarrier(hv);
}

template <typename HVType>
void GCHermesValueBase<HVType>::unreachableWriteBarrier(GC &gc) {
  // Hades needs a snapshot barrier executed when something becomes unreachable.
  gc.snapshotWriteBarrier(this);
}

/*static*/
template <typename HVType>
template <typename InputIt>
inline void GCHermesValueBase<HVType>::fill(
    InputIt start,
    InputIt end,
    HVType fill,
    GC &gc) {
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
template <typename HVType>
template <typename InputIt>
inline void GCHermesValueBase<HVType>::uninitialized_fill(
    InputIt start,
    InputIt end,
    HVType fill,
    GC &gc) {
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      // Use the constructor write barrier. Assume it needs barriers.
      new (&*cur) GCHermesValueBase<HVType>(fill, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      // Use a constructor that doesn't handle pointer values.
      new (&*cur) GCHermesValueBase<HVType>(fill, gc, nullptr);
    }
  }
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueBase<HVType>::copy(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC &gc) {
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  static_assert(
      !std::is_same<InputIt, GCHermesValueBase *>::value ||
          !std::is_same<OutputIt, GCHermesValueBase *>::value,
      "Pointer arguments must invoke pointer overload.");
#endif
  for (; first != last; ++first, (void)++result) {
    result->set(*first, gc);
  }
  return result;
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueBase<HVType>::uninitialized_copy(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC &gc) {
  static_assert(
      !std::is_same<InputIt, GCHermesValueBase *>::value ||
          !std::is_same<OutputIt, GCHermesValueBase *>::value,
      "Pointer arguments must invoke pointer overload.");
  for (; first != last; ++first, (void)++result) {
    new (&*result) GCHermesValueBase<HVType>(*first, gc);
  }
  return result;
}

// Specializations using memmove can't be used in Hades, because the concurrent
// write barrier needs strict control over how assignments are done to HV fields
// which need to be atomically updated.
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
/// Specialization for raw pointers to do a ranged write barrier.
template <typename HVType>
inline GCHermesValueBase<HVType> *GCHermesValueBase<HVType>::copy(
    GCHermesValueBase<HVType> *first,
    GCHermesValueBase<HVType> *last,
    GCHermesValueBase<HVType> *result,
    GC &gc) {
  // We must use "raw" function such as memmove here, rather than a
  // function like std::copy (or copy_backward) that respects
  // constructors and operator=.  For HermesValue, those require the
  // contents not to contain pointers.  The range write barrier
  // before the copies ensure that sufficient barriers are
  // performed.
  gc.writeBarrierRange(result, last - first);
  std::memmove(
      reinterpret_cast<void *>(result),
      first,
      (last - first) * sizeof(GCHermesValueBase<HVType>));
  return result + (last - first);
}
#endif

/// Specialization for raw pointers to do a ranged write barrier.
template <typename HVType>
inline GCHermesValueBase<HVType> *GCHermesValueBase<HVType>::uninitialized_copy(
    GCHermesValueBase<HVType> *first,
    GCHermesValueBase<HVType> *last,
    GCHermesValueBase<HVType> *result,
    GC &gc) {
#ifndef NDEBUG
  uintptr_t fromFirst = reinterpret_cast<uintptr_t>(first),
            fromLast = reinterpret_cast<uintptr_t>(last);
  uintptr_t toFirst = reinterpret_cast<uintptr_t>(result),
            toLast = toFirst + fromFirst - fromLast;
  assert(
      (toLast < fromFirst || fromLast < toFirst) &&
      "Uninitialized range cannot overlap with an initialized one.");
#endif

  gc.constructorWriteBarrierRange(result, last - first);
  // memcpy is fine for an uninitialized copy.
  std::memcpy(
      reinterpret_cast<void *>(result), first, (last - first) * sizeof(HVType));
  return result + (last - first);
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueBase<HVType>::copy_backward(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC &gc) {
  while (first != last) {
    (--result)->set(*--last, gc);
  }
  return result;
}

template <typename HVType>
inline void GCHermesValueBase<HVType>::rangeUnreachableWriteBarrier(
    GCHermesValueBase<HVType> *first,
    GCHermesValueBase<HVType> *last,
    GC &gc) {
  gc.snapshotWriteBarrierRange(first, last - first);
}

inline void GCHermesValueUtil::copyToPinned(
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
