/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/WeakValueObjectMap.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/JSObject.h"
#include "llvh/ADT/ScopeExit.h"

namespace hermes::vm {

const VTable WeakValueObjectMapImpl::vt{
    CellKind::WeakValueObjectMapKind,
    cellSize<WeakValueObjectMapImpl>(),
    /* allowLargeAlloc */ false,
    _finalizeImpl,
    _mallocSizeImpl,
    /* trimSize */ nullptr};

void WeakValueObjectMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const WeakValueObjectMapImpl *>(cell);
  mb.setVTable(&WeakValueObjectMapImpl::vt);
  mb.addField("keyStorage", &self->keys_);
}

WeakValueObjectMapImpl::~WeakValueObjectMapImpl() {
  if (values_) {
    for (size_type i = 0; i < capacity_; ++i) {
      if (allocatedWeakRefs_[i]) {
        values_[i].releaseSlot();
      }
    }
    free(values_);
  }
}

WeakValueObjectMapImpl::size_type WeakValueObjectMapImpl::findIndex(
    Runtime &runtime,
    JSObject *key) const {
  NoAllocScope noAlloc{runtime};
  if (capacity_ == 0) {
    return capacity_;
  }

  auto hash = runtime.gcStableHashJSObject(key);

  size_type cap = capacity_;
  size_type idx = hash & (cap - 1);
  size_type base = 1;

  // deletedIndex tracks the index of a deleted entry found in the conflict
  // chain. If we could not find an entry that matches str, we would return
  // the deleted slot for insertion to be able to reuse deleted space.
  OptValue<size_type> deletedIndex;

  ArrayStorage *keys = keys_.getNonNull(runtime);
  for (;;) {
    if (keys->at(idx).isEmpty()) {
      // Found an empty entry, meaning that key does not exist in the table.
      // If deletedIndex is available, return it, otherwise return idx.
      return deletedIndex ? *deletedIndex : idx;
    } else if (keys->at(idx).isNull()) {
      deletedIndex = idx;
    } else if (keys_.get(runtime)->at(idx).getObject() == key) {
      return idx;
    }

    /// Use quadratic probing to find next probe index in the hash table.
    /// h(k, i) = (h(k) + 1/2 * i + 1/2 * i^2) mod m.
    /// This guarantees the values of h(k,i) for i in [0,m-1] are all distinct.
    idx = (idx + base) & (cap - 1);
    ++base;
  }
}

void WeakValueObjectMapImpl::rehash(
    Handle<WeakValueObjectMapImpl> self,
    Runtime &runtime,
    size_type newCapacity) {
  const size_type oldCapacity = self->capacity_;

  struct : public Locals {
    PinnedValue<ArrayStorage> oldKeys;
    PinnedValue<ArrayStorage> newKeys;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  // Create new key storage.
  assert(
      newCapacity < ArrayStorage::maxCapacityNoOverflow() &&
      "must not overflow");
  auto keyStorageRes = ArrayStorage::create(runtime, newCapacity, newCapacity);
  assert(keyStorageRes != ExecutionStatus::EXCEPTION);
  lv.newKeys = vmcast<ArrayStorage>(*keyStorageRes);

  // Save old storage and values for rehashing.
  lv.oldKeys = self->keys_.get(runtime);
  auto *oldValues = self->values_;
  auto freeOldValues = llvh::make_scope_exit([oldValues] { free(oldValues); });

  // Set up new storage.
  self->keys_.set(runtime, *lv.newKeys, runtime.getHeap());
  self->values_ = static_cast<decltype(self->values_)>(
      checkedCalloc(newCapacity, sizeof(self->values_[0])));
  self->allocatedWeakRefs_.clear();
  self->allocatedWeakRefs_.resize(newCapacity, false);

  // Update capacity before rehashing.
  self->capacity_ = newCapacity;
  self->size_ = 0;

  // Rehash all valid entries.
  for (size_type i = 0; i < oldCapacity; ++i) {
    NoAllocScope noAlloc{runtime};
    HermesValue oldKey = lv.oldKeys->at(i);

    // Skip empty and tombstone keys.
    if (oldKey.isEmpty() || oldKey.isNull()) {
      continue;
    }

    // Skip invalid weak references.
    if (!oldValues[i].isValid()) {
      oldValues[i].releaseSlot();
      continue;
    }

    // Find new location for this entry.
    size_type newIdx = self->findIndex(runtime, vmcast<JSObject>(oldKey));

    // Insert entry at new location.
    lv.newKeys->set(newIdx, oldKey, runtime.getHeap());
    self->values_[newIdx] = std::move(oldValues[i]);
    self->allocatedWeakRefs_[newIdx] = true;
    ++self->size_;
  }
}

bool WeakValueObjectMapImpl::insertNew(
    Handle<WeakValueObjectMapImpl> self,
    Runtime &runtime,
    JSObject *key,
    GCCell *value) {
  // Check if we need to grow the table.
  if (self->shouldGrow(self->capacity_, self->size_ + 1)) {
    struct : public Locals {
      PinnedValue<WeakValueObjectMapImpl> self;
      PinnedValue<JSObject> key;
      PinnedValue<GCCell> value;
    } lv;
    LocalsRAII lraii{runtime, &lv};

    lv.self = *self;
    lv.key = key;
    lv.value = value;

    size_type newCapacity = lv.self->capacity_ < kMinCapacity
        ? kMinCapacity
        : lv.self->capacity_ * kGrowOrShrinkFactor;
    if (newCapacity > kMaxCapacity) {
      return false;
    }
    rehash(lv.self, runtime, newCapacity);

    NoAllocScope noAlloc{runtime};
    size_type idx = lv.self->findIndex(runtime, *lv.key);
    ArrayStorage *keys = lv.self->keys_.getNonNull(runtime);
    if (keys->at(idx).isObject()) {
      if (lv.self->values_[idx].isValid()) {
        return false;
      }
      assert(lv.self->allocatedWeakRefs_[idx]);
      lv.self->values_[idx].releaseSlot();
      lv.self->values_[idx] = WeakRef<GCCell>(runtime, lv.value);
      return true;
    } else {
      assert(!self->allocatedWeakRefs_[idx]);
      keys->set(idx, lv.key.getHermesValue(), runtime.getHeap());
      lv.self->values_[idx] = WeakRef<GCCell>(runtime, lv.value);
      lv.self->allocatedWeakRefs_[idx] = true;
      ++lv.self->size_;
      return true;
    }
  } else {
    NoAllocScope noAlloc{runtime};
    size_type idx = self->findIndex(runtime, key);
    ArrayStorage *keys = self->keys_.getNonNull(runtime);
    if (keys->at(idx).isObject()) {
      if (self->values_[idx].isValid()) {
        return false;
      }
      assert(self->allocatedWeakRefs_[idx]);
      self->values_[idx].releaseSlot();
      self->values_[idx] = WeakRef<GCCell>(runtime, runtime.getHeap(), value);
      return true;
    } else {
      // Set up the value for new or replaced entries.
      assert(!self->allocatedWeakRefs_[idx]);
      keys->set(idx, HermesValue::encodeObjectValue(key), runtime.getHeap());
      self->values_[idx] = WeakRef<GCCell>(runtime, runtime.getHeap(), value);
      self->allocatedWeakRefs_[idx] = true;
      ++self->size_;
      return true;
    }
  }
  return false;
}

bool WeakValueObjectMapImpl::erase(
    Handle<WeakValueObjectMapImpl> self,
    Runtime &runtime,
    JSObject *key,
    GC &gc) {
  size_type idx = self->findIndex(runtime, key);
  if (idx == self->capacity())
    return false;

  ArrayStorage *keys = self->keys_.getNonNull(runtime);
  HermesValue storedKey = keys->at(idx);
  if (storedKey.isObject()) {
    assert(storedKey.getObject() == key && "findIndex wrong");
    self->keys_.getNonNull(runtime)->set(
        idx, HermesValue::encodeNullValue(), runtime.getHeap());
    self->values_[idx].releaseSlot();
    self->allocatedWeakRefs_[idx] = false;
    --self->size_;
    self->recalcPruneLimit();
    if (self->shouldShrink(self->capacity_, self->size_)) {
      rehash(self, runtime, self->capacity_ / kGrowOrShrinkFactor);
    }
    return true;
  }

  return false;
}

void WeakValueObjectMapImpl::pruneInvalid(
    Handle<WeakValueObjectMapImpl> self,
    Runtime &runtime,
    GC &gc) {
  if (!self->keys_)
    return;

  NoAllocScope noAlloc{runtime};
  ArrayStorage *keys = self->keys_.getNonNull(runtime);
  for (size_type i = 0; i < self->capacity_; ++i) {
    if (keys->at(i).isObject()) {
      keys->set(i, HermesValue::encodeNullValue(), runtime.getHeap());
      self->values_[i].releaseSlot();
      self->allocatedWeakRefs_[i] = false;
      --self->size_;
    }
  }

  self->recalcPruneLimit();

  if (self->shouldShrink(self->capacity_, self->size_)) {
    rehash(self, runtime, self->capacity_ / kGrowOrShrinkFactor);
  }
}

void WeakValueObjectMapImpl::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<WeakValueObjectMapImpl>(cell);
  self->~WeakValueObjectMapImpl();
}

size_t WeakValueObjectMapImpl::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<WeakValueObjectMapImpl>(cell);
  return self->getMemorySize();
}

} // namespace hermes::vm
