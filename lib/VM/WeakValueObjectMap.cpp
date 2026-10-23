/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/WeakValueObjectMap.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/JSObject.h"

namespace hermes::vm {

/// Allocate a WeakRef to an arbitrary GCCell \p value.
/// GCCell isn't a valid Handle<T> type so this is the simplest way.
static WeakRef<GCCell> makeWeakRef(Runtime &runtime, GCCell *value) {
  return WeakRef<GCCell>(runtime.getHeap().allocWeakSlot(
      SmallHermesValue::encodeObjectValue(value, runtime)));
}

/// \return true if \p ref is a live entry.
static bool isLive(const WeakRef<GCCell> &ref) {
  return !ref.isEmpty() && ref.isValid();
}

/// Hash \p key, including the valid null key.
static uint32_t hashKey(Runtime &runtime, JSObject *key) {
  return key ? runtime.gcStableHashJSObject(key) : 0;
}

WeakValueObjectMapImpl::~WeakValueObjectMapImpl() {
  if (values_) {
    // Calloc zero-fills values_ so unconstructed entries have null slot_.
    // Every other entry, live or dead, owns its slot.
    for (size_type i = 0; i < capacity_; ++i) {
      if (!values_[i].isEmpty()) {
        values_[i].releaseSlot();
      }
    }
    free(values_);
  }
}

WeakValueObjectMapImpl::size_type WeakValueObjectMapImpl::findIndex(
    Runtime &runtime,
    JSObject *key,
    KeyOfFn keyOf) const {
  NoAllocScope noAlloc{runtime};
  if (capacity_ == 0) {
    return capacity_;
  }

  auto hash = hashKey(runtime, key);

  size_type cap = capacity_;
  size_type idx = hash & (cap - 1);
  size_type base = 1;

  // The first dead entry found in the conflict chain. If key is not found, it
  // is returned for insertion to reuse its space.
  OptValue<size_type> deadIndex;

  // Loop will terminate because there is always at least one empty entry.
  for (;;) {
    const WeakRef<GCCell> &ref = values_[idx];
    if (ref.isEmpty()) {
      return deadIndex ? *deadIndex : idx;
    } else if (ref.isValid()) {
      // The value is only used to compute its key, so it needs no read
      // barrier.
      if (keyOf(ref.getNoBarrierUnsafe(runtime), runtime) == key)
        return idx;
    } else if (!deadIndex) {
      deadIndex = idx;
    }

    /// Use quadratic probing to find next probe index in the hash table.
    /// h(k, i) = (h(k) + 1/2 * i + 1/2 * i^2) mod m.
    /// This guarantees the values of h(k,i) for i in [0,m-1] are all distinct.
    idx = (idx + base) & (cap - 1);
    ++base;
  }
}

GCCell *WeakValueObjectMapImpl::lookup(
    Runtime &runtime,
    JSObject *key,
    KeyOfFn keyOf) const {
  NoAllocScope noAlloc{runtime};
  size_type idx = findIndex(runtime, key, keyOf);
  if (idx < capacity_ && isLive(values_[idx]))
    return values_[idx].get(runtime);
  return nullptr;
}

void WeakValueObjectMapImpl::rehash(Runtime &runtime, KeyOfFn keyOf) {
  NoAllocScope noAlloc{runtime};
  const size_type oldCapacity = capacity_;
  WeakRef<GCCell> *oldValues = values_;

  size_type liveCount = 0;
  for (size_type i = 0; i < oldCapacity; ++i) {
    if (isLive(oldValues[i]))
      ++liveCount;
  }

  // Fill at most half the table, counting the entry about to be inserted.
  // A full table holds 3/4 capacity live entries, so this doubles it.
  size_type newCapacity = kMinCapacity;
  while (newCapacity / 2 < liveCount + 1) {
    if (LLVM_UNLIKELY(
            newCapacity > std::numeric_limits<size_type>::max() / 2)) {
      hermes_fatal("WeakValueObjectMap capacity overflow");
    }
    newCapacity *= 2;
  }

  // Calloc zero-fills, so every WeakRef's slot_ is null (i.e. isEmpty() is
  // true) until we construct an entry.
  values_ = static_cast<WeakRef<GCCell> *>(
      checkedCalloc(newCapacity, sizeof(oldValues[0])));
  capacity_ = newCapacity;
  nonEmptyEntryCount_ = 0;

  for (size_type i = 0; i < oldCapacity; ++i) {
    if (oldValues[i].isEmpty())
      continue;
    if (!oldValues[i].isValid()) {
      oldValues[i].releaseSlot();
      continue;
    }
    // The value is only used to compute its key, so it needs no read barrier.
    JSObject *key = keyOf(oldValues[i].getNoBarrierUnsafe(runtime), runtime);
    size_type newIdx = findIndex(runtime, key, keyOf);
    // Overwriting a non-empty entry would leak its slot.
    assert(
        values_[newIdx].isEmpty() &&
        "duplicate key: keyOf changed after insert");
    // Moving the WeakRef copies the slot pointer; the source in oldValues is
    // discarded when the buffer is freed.
    values_[newIdx] = std::move(oldValues[i]);
    ++nonEmptyEntryCount_;
  }
  free(oldValues);
}

void WeakValueObjectMapImpl::insert(
    Runtime &runtime,
    JSObject *key,
    GCCell *value,
    KeyOfFn keyOf) {
  // Nothing here allocates on the GC heap, so raw pointers (including this
  // map, which may be embedded in a GCCell) stay valid.
  NoAllocScope noAlloc{runtime};
  assert(keyOf(value, runtime) == key && "value must map to its own key");

  // Overwrite a live entry for key before considering a rehash, since
  // overwriting doesn't add an entry.
  size_type idx = findIndex(runtime, key, keyOf);
  if (idx < capacity_ && isLive(values_[idx])) {
    values_[idx].releaseSlot();
    values_[idx] = makeWeakRef(runtime, value);
    return;
  }

  if (shouldRehashBeforeInsert()) {
    rehash(runtime, keyOf);
    idx = findIndex(runtime, key, keyOf);
  }

  WeakRef<GCCell> &ref = values_[idx];
  if (ref.isEmpty()) {
    ++nonEmptyEntryCount_;
  } else {
    // Reuse a dead entry.
    ref.releaseSlot();
  }
  ref = makeWeakRef(runtime, value);
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void WeakValueObjectMapImpl::snapshotAddEdges(
    GC &gc,
    HeapSnapshot &snap,
    llvh::StringRef name) const {
  uint32_t edgeIndex = 0;
  for (size_type i = 0; i < capacity_; ++i) {
    if (!isLive(values_[i]))
      continue;
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Weak,
        name.str() + std::to_string(edgeIndex++),
        gc.getObjectID(values_[i].getNoBarrierUnsafe(gc.getPointerBase())));
  }
}
#endif

} // namespace hermes::vm
