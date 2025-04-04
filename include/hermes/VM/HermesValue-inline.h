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
GCHermesValueImpl<HVType>::GCHermesValueImpl(HVType hv, GC &gc)
    : GCHermesValueBaseImpl<HVType>{hv} {
  assert(!hv.isPointer() || hv.getPointer());
  if (NeedsBarriers::value) {
    gc.constructorWriteBarrier(this, hv);
  } else {
    assert(
        !gc.needsWriteBarrierInCtor(this, hv) &&
        "Can't skip write barriers for this GCHermesValueBase and target value");
  }
}

template <typename HVType>
template <typename NeedsBarriers>
GCHermesValueInLargeObjImpl<HVType>::GCHermesValueInLargeObjImpl(
    HVType hv,
    const GCCell *owningObj,
    GC &gc)
    : GCHermesValueBaseImpl<HVType>{hv} {
  assert(!hv.isPointer() || hv.getPointer());
  if (NeedsBarriers::value) {
    gc.constructorWriteBarrierForLargeObj(owningObj, this, hv);
  } else {
    assert(
        !gc.needsWriteBarrierInCtor(this, hv) &&
        "This GCHermesValueBase construction cannot skip write barrier");
  }
}

template <typename HVType>
template <typename NeedsBarriers>
GCHermesValueImpl<HVType>::GCHermesValueImpl(HVType hv, GC &gc, std::nullptr_t)
    : GCHermesValueBaseImpl<HVType>(hv) {
  assert(!hv.isPointer());
  // No need to invoke any write barriers here, since the old value is
  // uninitialized (so the snapshot barrier does not apply), and the new value
  // is not a pointer (so the generational/relocation barrier does not apply).
}

template <typename HVType>
template <typename NeedsBarriers>
inline void GCHermesValueImpl<HVType>::set(HVType hv, GC &gc) {
  if (hv.isPointer()) {
    HERMES_SLOW_ASSERT(
        gc.validPointer(hv.getPointer(gc.getPointerBase())) &&
        "Setting an invalid pointer into a GCHermesValue");
  }
  assert(NeedsBarriers::value || !gc.needsWriteBarrier(this, hv));
  if (NeedsBarriers::value)
    gc.writeBarrier(this, hv);
  HVType::setNoBarrier(hv);
}

template <typename HVType>
template <typename NeedsBarriers>
GCHermesValueInLargeObjImpl<HVType>::GCHermesValueInLargeObjImpl(
    HVType hv,
    GC &gc,
    std::nullptr_t)
    : GCHermesValueBaseImpl<HVType>(hv) {
  assert(!hv.isPointer());
  // No need to invoke any write barriers here, since the old value is
  // uninitialized (so the snapshot barrier does not apply), and the new value
  // is not a pointer (so the generational/relocation barrier does not apply).
}

template <typename HVType>
template <typename NeedsBarriers>
inline void GCHermesValueInLargeObjImpl<HVType>::set(
    HVType hv,
    const GCCell *owningObj,
    GC &gc) {
  if (hv.isPointer()) {
    HERMES_SLOW_ASSERT(
        gc.validPointer(hv.getPointer(gc.getPointerBase())) &&
        "Setting an invalid pointer into a GCHermesValue");
  }
  if constexpr (NeedsBarriers::value) {
    gc.writeBarrierForLargeObj(owningObj, this, hv);
  } else {
    assert(
        !gc.needsWriteBarrier(this, hv) &&
        "Can't skip write barriers for this GCHermesValueBase and target value");
  }
  HVType::setNoBarrier(hv);
}

template <typename HVType>
void GCHermesValueBaseImpl<HVType>::setNonPtr(HVType hv, GC &gc) {
  assert(!hv.isPointer());
  gc.snapshotWriteBarrier(this);
  HVType::setNoBarrier(hv);
}

template <typename HVType>
void GCHermesValueBaseImpl<HVType>::unreachableWriteBarrier(GC &gc) {
  // Hades needs a snapshot barrier executed when something becomes unreachable.
  gc.snapshotWriteBarrier(this);
}

/*static*/
template <typename HVType>
template <typename InputIt>
inline void GCHermesValueImpl<HVType>::fill(
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
inline void GCHermesValueInLargeObjImpl<HVType>::fill(
    InputIt start,
    InputIt end,
    HVType fill,
    const GCCell *owningObj,
    GC &gc) {
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      cur->set(fill, owningObj, gc);
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
inline void GCHermesValueImpl<HVType>::uninitialized_fill(
    InputIt start,
    InputIt end,
    HVType fill,
    GC &gc) {
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      // Use the constructor write barrier. Assume it needs barriers.
      new (&*cur) GCHermesValueImpl<HVType>(fill, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      // Use a constructor that doesn't handle pointer values.
      new (&*cur) GCHermesValueImpl<HVType>(fill, gc, nullptr);
    }
  }
}

/*static*/
template <typename HVType>
template <typename InputIt>
inline void GCHermesValueInLargeObjImpl<HVType>::uninitialized_fill(
    InputIt start,
    InputIt end,
    HVType fill,
    const GCCell *owningObj,
    GC &gc) {
  if (fill.isPointer()) {
    for (auto cur = start; cur != end; ++cur) {
      // Use the constructor write barrier. Assume it needs barriers.
      new (&*cur) GCHermesValueInLargeObjImpl<HVType>(fill, owningObj, gc);
    }
  } else {
    for (auto cur = start; cur != end; ++cur) {
      // Use a constructor that doesn't handle pointer values.
      new (&*cur) GCHermesValueInLargeObjImpl<HVType>(fill, gc, nullptr);
    }
  }
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueImpl<HVType>::copy(
    InputIt first,
    InputIt last,
    OutputIt result,
    GC &gc) {
  for (; first != last; ++first, (void)++result) {
    result->set(*first, gc);
  }
  return result;
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueInLargeObjImpl<HVType>::copy(
    InputIt first,
    InputIt last,
    OutputIt result,
    const GCCell *owningObj,
    GC &gc) {
  for (; first != last; ++first, (void)++result) {
    result->set(*first, owningObj, gc);
  }
  return result;
}

/// Specialization for raw pointers to do a ranged write barrier.
template <typename HVType>
inline GCHermesValueBaseImpl<HVType> *
GCHermesValueBaseImpl<HVType>::uninitialized_copy(
    GCHermesValueBaseImpl<HVType> *first,
    GCHermesValueBaseImpl<HVType> *last,
    GCHermesValueBaseImpl<HVType> *result,
    const GCCell *owningObj,
    GC &gc) {
#ifndef NDEBUG
  uintptr_t fromFirst = reinterpret_cast<uintptr_t>(first),
            fromLast = reinterpret_cast<uintptr_t>(last);
  uintptr_t toFirst = reinterpret_cast<uintptr_t>(result),
            toLast = toFirst + fromFirst - fromLast;
  assert(fromFirst <= fromLast && "Source range is reversed.");
  assert(
      (toLast <= fromFirst || fromLast <= toFirst) &&
      "Uninitialized range cannot overlap with an initialized one.");
#endif

  gc.constructorWriteBarrierRange(owningObj, result, last - first);
  // memcpy is fine for an uninitialized copy.
  std::memcpy(
      reinterpret_cast<void *>(result), first, (last - first) * sizeof(HVType));
  return result + (last - first);
}

template <typename HVType>
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueImpl<HVType>::copy_backward(
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
template <typename InputIt, typename OutputIt>
inline OutputIt GCHermesValueInLargeObjImpl<HVType>::copy_backward(
    InputIt first,
    InputIt last,
    OutputIt result,
    const GCCell *owningObj,
    GC &gc) {
  while (first != last) {
    (--result)->set(*--last, owningObj, gc);
  }
  return result;
}

template <typename HVType>
inline void GCHermesValueBaseImpl<HVType>::rangeUnreachableWriteBarrier(
    GCHermesValueBaseImpl<HVType> *first,
    GCHermesValueBaseImpl<HVType> *last,
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
