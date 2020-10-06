/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/OrderedHashMap.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Operations.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {
//===----------------------------------------------------------------------===//
// class HashMapEntry

VTable HashMapEntry::vt{CellKind::HashMapEntryKind, cellSize<HashMapEntry>()};

void HashMapEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HashMapEntry *>(cell);
  mb.addField("key", &self->key);
  mb.addField("value", &self->value);
  mb.addField("prevIterationEntry", &self->prevIterationEntry);
  mb.addField("nextIterationEntry", &self->nextIterationEntry);
  mb.addField("nextEntryInBucket", &self->nextEntryInBucket);
}

#ifdef HERMESVM_SERIALIZE
HashMapEntry::HashMapEntry(Deserializer &d)
    : GCCell(&d.getRuntime()->getHeap(), &vt) {
  d.readHermesValue(&key);
  d.readHermesValue(&value);
  d.readRelocation(&prevIterationEntry, RelocationKind::GCPointer);
  d.readRelocation(&nextIterationEntry, RelocationKind::GCPointer);
  d.readRelocation(&nextEntryInBucket, RelocationKind::GCPointer);
}

void HashMapEntrySerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const HashMapEntry>(cell);
  s.writeHermesValue(self->key);
  s.writeHermesValue(self->value);
  s.writeRelocation(self->prevIterationEntry.get(s.getRuntime()));
  s.writeRelocation(self->nextIterationEntry.get(s.getRuntime()));
  s.writeRelocation(self->nextEntryInBucket.get(s.getRuntime()));
  s.endObject(cell);
}

void HashMapEntryDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::HashMapEntryKind && "Expected HashMapEntry");
  void *mem = d.getRuntime()->alloc(cellSize<HashMapEntry>());
  auto *cell = new (mem) HashMapEntry(d);
  d.endObject(cell);
}
#endif

CallResult<PseudoHandle<HashMapEntry>> HashMapEntry::create(Runtime *runtime) {
  void *mem = runtime->alloc(cellSize<HashMapEntry>());
  return createPseudoHandle(new (mem) HashMapEntry(runtime));
}

//===----------------------------------------------------------------------===//
// class OrderedHashMap

VTable OrderedHashMap::vt{CellKind::OrderedHashMapKind,
                          cellSize<OrderedHashMap>()};

void OrderedHashMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const OrderedHashMap *>(cell);
  mb.addField("hashTable", &self->hashTable_);
  mb.addField("firstIterationEntry", &self->firstIterationEntry_);
  mb.addField("lastIterationEntry", &self->lastIterationEntry_);
}

#ifdef HERMESVM_SERIALIZE
OrderedHashMap::OrderedHashMap(Deserializer &d)
    : GCCell(&d.getRuntime()->getHeap(), &vt) {
  if (d.readInt<uint8_t>()) {
    hashTable_.set(
        d.getRuntime(),
        ArrayStorage::deserializeArrayStorage(d),
        &d.getRuntime()->getHeap());
  }

  d.readRelocation(&firstIterationEntry_, RelocationKind::GCPointer);
  d.readRelocation(&lastIterationEntry_, RelocationKind::GCPointer);
  capacity_ = d.readInt<uint32_t>();
  size_ = d.readInt<uint32_t>();
}

void OrderedHashMapSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const OrderedHashMap>(cell);
  // If we have an ArrayStorage, it doesn't store any native pointers. Serialize
  // it here.
  bool hasArray = (bool)self->hashTable_;
  s.writeInt<uint8_t>(hasArray);
  if (hasArray) {
    ArrayStorage::serializeArrayStorage(
        s, self->hashTable_.get(s.getRuntime()));
  }

  s.writeRelocation(self->firstIterationEntry_.get(s.getRuntime()));
  s.writeRelocation(self->lastIterationEntry_.get(s.getRuntime()));
  s.writeInt<uint32_t>(self->capacity_);
  s.writeInt<uint32_t>(self->size_);

  s.endObject(cell);
}

void OrderedHashMapDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::OrderedHashMapKind && "ExpectedOrderedHashMap");
  void *mem = d.getRuntime()->alloc(cellSize<OrderedHashMap>());
  auto *cell = new (mem) OrderedHashMap(d);

  d.endObject(cell);
}
#endif

OrderedHashMap::OrderedHashMap(
    Runtime *runtime,
    Handle<ArrayStorage> hashTableStorage)
    : GCCell(&runtime->getHeap(), &vt),
      hashTable_(runtime, hashTableStorage.get(), &runtime->getHeap()) {}

CallResult<PseudoHandle<OrderedHashMap>> OrderedHashMap::create(
    Runtime *runtime) {
  auto arrRes =
      ArrayStorage::create(runtime, INITIAL_CAPACITY, INITIAL_CAPACITY);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto hashTableStorage = runtime->makeHandle<ArrayStorage>(*arrRes);

  void *mem = runtime->alloc(cellSize<OrderedHashMap>());
  return createPseudoHandle(new (mem)
                                OrderedHashMap(runtime, hashTableStorage));
}

void OrderedHashMap::removeLinkedListNode(
    Runtime *runtime,
    HashMapEntry *entry,
    GC *gc) {
  assert(
      entry != lastIterationEntry_.get(runtime) &&
      "Cannot remove the last entry");
  if (entry->prevIterationEntry) {
    entry->prevIterationEntry.get(runtime)->nextIterationEntry.set(
        runtime, entry->nextIterationEntry, gc);
  }
  if (entry->nextIterationEntry) {
    entry->nextIterationEntry.get(runtime)->prevIterationEntry.set(
        runtime, entry->prevIterationEntry, gc);
  }
  if (entry == firstIterationEntry_.get(runtime)) {
    firstIterationEntry_.set(runtime, entry->nextIterationEntry, gc);
  }
  entry->prevIterationEntry.setNull(&runtime->getHeap());
}

HashMapEntry *OrderedHashMap::lookupInBucket(
    Runtime *runtime,
    uint32_t bucket,
    HermesValue key) {
  assert(
      hashTable_.get(runtime)->size() == capacity_ && "Inconsistent capacity");
  auto *entry = dyn_vmcast<HashMapEntry>(hashTable_.get(runtime)->at(bucket));
  while (entry && !isSameValueZero(entry->key, key)) {
    entry = entry->nextEntryInBucket.get(runtime);
  }
  return entry;
}

ExecutionStatus OrderedHashMap::rehashIfNecessary(
    Handle<OrderedHashMap> self,
    Runtime *runtime) {
  uint32_t newCapacity = self->capacity_;
  // NOTE: we have ensured that self->capacity_ * 4 never overflows uint32_t by
  // setting MAX_CAPACITY to the apropriate value. self->size_ is always <=
  // self->capacity_, so this applies to self->size_ as well.
  static_assert(
      MAX_CAPACITY < UINT32_MAX / 4,
      "Avoid overflow checks on multiplying capacity by 4");
  if (self->size_ * 4 > self->capacity_ * 3) {
    // Load factor is more than 0.75, need to increase the capacity.
    newCapacity = self->capacity_ * 2;
    if (LLVM_UNLIKELY(newCapacity > MAX_CAPACITY)) {
      // We maintain the invariant that the capacity_ is a power of two.
      // Therefore, if doubling would exceed the max, we revert to the
      // previous value.  (Assert the invariant to make it clear.)
      assert(
          (self->capacity_ & (self->capacity_ - 1)) == 0 &&
          "capacity_ must be power of 2");
      newCapacity = self->capacity_;
    }
  } else if (
      self->size_ * 4 < self->capacity_ && self->capacity_ > INITIAL_CAPACITY) {
    // Load factor is less than 0.25, and we are not at initial cap.
    newCapacity = self->capacity_ / 2;
  }
  if (newCapacity == self->capacity_) {
    // No need to rehash.
    return ExecutionStatus::RETURNED;
  }
  assert(
      self->size_ && self->firstIterationEntry_ && self->lastIterationEntry_ &&
      "We should never be rehashing when there are no elements");

  // Set new capacity first to update the hash function.
  self->capacity_ = newCapacity;

  // Create a new hash table.
  auto arrRes = ArrayStorage::create(runtime, newCapacity, newCapacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newHashTable = runtime->makeHandle<ArrayStorage>(*arrRes);

  // Now re-add all entries to the hash table.
  MutableHandle<HashMapEntry> entry{runtime};
  MutableHandle<HashMapEntry> oldNextInBucket{runtime};
  MutableHandle<> keyHandle{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (unsigned i = 0, len = self->hashTable_.get(runtime)->size(); i < len;
       ++i) {
    entry = dyn_vmcast<HashMapEntry>(self->hashTable_.get(runtime)->at(i));
    while (entry) {
      marker.flush();
      keyHandle = entry->key;
      uint32_t bucket = hashToBucket(self, runtime, keyHandle);
      oldNextInBucket = entry->nextEntryInBucket.get(runtime);
      if (newHashTable->at(bucket).isEmpty()) {
        // Empty bucket.
        entry->nextEntryInBucket.setNull(&runtime->getHeap());
      } else {
        // There are already a bucket head.
        entry->nextEntryInBucket.set(
            runtime,
            vmcast<HashMapEntry>(newHashTable->at(bucket)),
            &runtime->getHeap());
      }
      // Update bucket head to the new entry.
      newHashTable->at(bucket).set(entry.getHermesValue(), &runtime->getHeap());

      entry = *oldNextInBucket;
    }
  }

  self->hashTable_.set(runtime, newHashTable.get(), &runtime->getHeap());
  assert(
      self->hashTable_.get(runtime)->size() == self->capacity_ &&
      "Inconsistent capacity");
  return ExecutionStatus::RETURNED;
}

bool OrderedHashMap::has(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(runtime, bucket, key.getHermesValue());
}

HashMapEntry *OrderedHashMap::find(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(runtime, bucket, key.getHermesValue());
}

HermesValue OrderedHashMap::get(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  auto *entry = find(self, runtime, key);
  if (!entry) {
    return HermesValue::encodeUndefinedValue();
  }
  return entry->value;
}

ExecutionStatus OrderedHashMap::insert(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key,
    Handle<> value) {
  uint32_t bucket = hashToBucket(self, runtime, key);
  if (auto *entry =
          self->lookupInBucket(runtime, bucket, key.getHermesValue())) {
    // Element already exists, update value and return.
    entry->value.set(value.get(), &runtime->getHeap());
    return ExecutionStatus::RETURNED;
  }
  // Create a new entry, set the key and value.
  auto crtRes = HashMapEntry::create(runtime);
  if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newMapEntry = runtime->makeHandle(std::move(*crtRes));
  newMapEntry->key.set(key.get(), &runtime->getHeap());
  newMapEntry->value.set(value.get(), &runtime->getHeap());
  auto *curBucketFront =
      dyn_vmcast<HashMapEntry>(self->hashTable_.get(runtime)->at(bucket));
  if (curBucketFront) {
    // If the bucket we are inserting to is not empty, we maintain the
    // linked list properly.
    newMapEntry->nextEntryInBucket.set(
        runtime, curBucketFront, &runtime->getHeap());
  }
  // Set the newly inserted entry as the front of this bucket chain.
  self->hashTable_.get(runtime)->at(bucket).set(
      newMapEntry.getHermesValue(), &runtime->getHeap());

  if (!self->firstIterationEntry_) {
    // If we are inserting the first ever element, update
    // first iteration entry pointer.
    self->firstIterationEntry_.set(
        runtime, newMapEntry.get(), &runtime->getHeap());
    self->lastIterationEntry_.set(
        runtime, newMapEntry.get(), &runtime->getHeap());
  } else {
    // Connect the new entry with the last entry.
    self->lastIterationEntry_.get(runtime)->nextIterationEntry.set(
        runtime, newMapEntry.get(), &runtime->getHeap());
    newMapEntry->prevIterationEntry.set(
        runtime, self->lastIterationEntry_, &runtime->getHeap());

    HashMapEntry *previousLastEntry = self->lastIterationEntry_.get(runtime);
    self->lastIterationEntry_.set(
        runtime, newMapEntry.get(), &runtime->getHeap());

    if (previousLastEntry && previousLastEntry->isDeleted()) {
      // If the last entry was a deleted entry, we no longer need to keep it.
      self->removeLinkedListNode(
          runtime, previousLastEntry, &runtime->getHeap());
    }
  }

  self->size_++;
  return self->rehashIfNecessary(self, runtime);
}

bool OrderedHashMap::erase(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  uint32_t bucket = hashToBucket(self, runtime, key);
  HashMapEntry *prevEntry = nullptr;
  auto *entry =
      dyn_vmcast<HashMapEntry>(self->hashTable_.get(runtime)->at(bucket));
  while (entry && !isSameValueZero(entry->key, key.getHermesValue())) {
    prevEntry = entry;
    entry = entry->nextEntryInBucket.get(runtime);
  }
  if (!entry) {
    // Element does not exist.
    return false;
  }

  if (prevEntry) {
    // The entry we are deleting has a previous entry, update the link.
    prevEntry->nextEntryInBucket.set(
        runtime, entry->nextEntryInBucket, &runtime->getHeap());
  } else {
    // The entry we are erasing is the front entry in the bucket, we need
    // to update the bucket head to the next entry in the bucket, if not
    // any, set to empty value.
    self->hashTable_.get(runtime)->at(bucket).set(
        entry->nextEntryInBucket ? HermesValue::encodeObjectValue(
                                       entry->nextEntryInBucket.get(runtime))
                                 : HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
  }

  entry->markDeleted(runtime);
  self->size_--;

  // Now delete the entry from the iteration linked list.
  // If we are deleting the last entry, don't remove it from the list yet.
  // This will ensure that if any iterator out there is currently pointing to
  // this entry, it will be able to follow on if we add new entries in the
  // future.
  if (entry != self->lastIterationEntry_.get(runtime)) {
    self->removeLinkedListNode(runtime, entry, &runtime->getHeap());
  }

  self->rehashIfNecessary(self, runtime);

  return true;
}

HashMapEntry *OrderedHashMap::iteratorNext(
    Runtime *runtime,
    HashMapEntry *entry) const {
  if (entry == nullptr) {
    // Starting a new iteration from the first entry.
    entry = firstIterationEntry_.get(runtime);
  } else {
    // Advance to the next entry.
    entry = entry->nextIterationEntry.get(runtime);
  }

  // Make sure the entry we returned (if not nullptr) must not be deleted.
  while (entry && entry->isDeleted()) {
    entry = entry->nextIterationEntry.get(runtime);
  }
  return entry;
}

void OrderedHashMap::clear(Runtime *runtime) {
  if (!firstIterationEntry_) {
    // Empty set.
    return;
  }

  // Clear the hash table.
  for (unsigned i = 0; i < capacity_; ++i) {
    auto entry = dyn_vmcast<HashMapEntry>(hashTable_.get(runtime)->at(i));
    // Delete every element reachable from the hash table.
    while (entry) {
      entry->markDeleted(runtime);
      entry = entry->nextEntryInBucket.get(runtime);
    }
    // Clear every element in the hash table.
    hashTable_.get(runtime)->at(i).setNonPtr(
        HermesValue::encodeEmptyValue(), &runtime->getHeap());
  }
  // Resize the hash table to the initial size.
  ArrayStorage::resizeWithinCapacity(
      hashTable_.getNonNull(runtime), runtime, INITIAL_CAPACITY);
  capacity_ = INITIAL_CAPACITY;

  // After clearing, we will still keep the last deleted entry
  // in case there is an iterator out there
  // pointing to the middle of the iteration chain. We need it to be
  // able to merge back eventually.
  firstIterationEntry_.set(runtime, lastIterationEntry_, &runtime->getHeap());
  firstIterationEntry_.get(runtime)->prevIterationEntry.setNull(
      &runtime->getHeap());
  size_ = 0;
}

} // namespace vm
} // namespace hermes
