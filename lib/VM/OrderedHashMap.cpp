/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/OrderedHashMap.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Operations.h"

namespace hermes {
namespace vm {
//===----------------------------------------------------------------------===//
// class HashMapEntry

const VTable HashMapEntry::vt{
    CellKind::HashMapEntryKind,
    cellSize<HashMapEntry>()};

void HashMapEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HashMapEntry *>(cell);
  mb.setVTable(&HashMapEntry::vt);
  mb.addField("key", &self->key);
  mb.addField("value", &self->value);
  mb.addField("prevIterationEntry", &self->prevIterationEntry);
  mb.addField("nextIterationEntry", &self->nextIterationEntry);
  mb.addField("nextEntryInBucket", &self->nextEntryInBucket);
}

CallResult<PseudoHandle<HashMapEntry>> HashMapEntry::create(Runtime &runtime) {
  return createPseudoHandle(runtime.makeAFixed<HashMapEntry>());
}

//===----------------------------------------------------------------------===//
// class OrderedHashMap

const VTable OrderedHashMap::vt{
    CellKind::OrderedHashMapKind,
    cellSize<OrderedHashMap>()};

void OrderedHashMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const OrderedHashMap *>(cell);
  mb.setVTable(&OrderedHashMap::vt);
  mb.addField("hashTable", &self->hashTable_);
  mb.addField("firstIterationEntry", &self->firstIterationEntry_);
  mb.addField("lastIterationEntry", &self->lastIterationEntry_);
}

OrderedHashMap::OrderedHashMap(
    Runtime &runtime,
    Handle<ArrayStorageSmall> hashTableStorage)
    : hashTable_(runtime, hashTableStorage.get(), runtime.getHeap()) {}

CallResult<PseudoHandle<OrderedHashMap>> OrderedHashMap::create(
    Runtime &runtime) {
  auto arrRes =
      ArrayStorageSmall::create(runtime, INITIAL_CAPACITY, INITIAL_CAPACITY);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto hashTableStorage = runtime.makeHandle<ArrayStorageSmall>(*arrRes);

  return createPseudoHandle(
      runtime.makeAFixed<OrderedHashMap>(runtime, hashTableStorage));
}

void OrderedHashMap::removeLinkedListNode(
    Runtime &runtime,
    HashMapEntry *entry,
    GC &gc) {
  assert(
      entry != lastIterationEntry_.get(runtime) &&
      "Cannot remove the last entry");
  if (entry->prevIterationEntry) {
    entry->prevIterationEntry.getNonNull(runtime)->nextIterationEntry.set(
        runtime, entry->nextIterationEntry, gc);
  }
  if (entry->nextIterationEntry) {
    entry->nextIterationEntry.getNonNull(runtime)->prevIterationEntry.set(
        runtime, entry->prevIterationEntry, gc);
  }
  if (entry == firstIterationEntry_.get(runtime)) {
    firstIterationEntry_.set(runtime, entry->nextIterationEntry, gc);
  }
  entry->prevIterationEntry.setNull(runtime.getHeap());
}

static HashMapEntry *castToMapEntry(SmallHermesValue shv, PointerBase &base) {
  if (shv.isEmpty())
    return nullptr;
  return vmcast<HashMapEntry>(shv.getObject(base));
}

HashMapEntry *OrderedHashMap::lookupInBucket(
    Runtime &runtime,
    uint32_t bucket,
    HermesValue key) {
  assert(
      hashTable_.getNonNull(runtime)->size() == capacity_ &&
      "Inconsistent capacity");
  auto *entry =
      castToMapEntry(hashTable_.getNonNull(runtime)->at(bucket), runtime);
  while (entry && !isSameValueZero(entry->key, key)) {
    entry = entry->nextEntryInBucket.get(runtime);
  }
  return entry;
}

ExecutionStatus OrderedHashMap::rehashIfNecessary(
    Handle<OrderedHashMap> self,
    Runtime &runtime) {
  uint32_t newCapacity = self->capacity_;
  // NOTE: we have ensured that self->capacity_ * 4 never overflows uint32_t by
  // setting MAX_CAPACITY to the appropriate value. self->size_ is always <=
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
  auto arrRes = ArrayStorageSmall::create(runtime, newCapacity, newCapacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newHashTable = runtime.makeHandle<ArrayStorageSmall>(*arrRes);

  // Now re-add all entries to the hash table.
  MutableHandle<HashMapEntry> entry{runtime};
  MutableHandle<HashMapEntry> oldNextInBucket{runtime};
  MutableHandle<> keyHandle{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (unsigned i = 0, len = self->hashTable_.getNonNull(runtime)->size();
       i < len;
       ++i) {
    entry =
        castToMapEntry(self->hashTable_.getNonNull(runtime)->at(i), runtime);
    while (entry) {
      marker.flush();
      keyHandle = entry->key;
      uint32_t bucket = hashToBucket(self, runtime, keyHandle);
      oldNextInBucket = entry->nextEntryInBucket.get(runtime);
      if (newHashTable->at(bucket).isEmpty()) {
        // Empty bucket.
        entry->nextEntryInBucket.setNull(runtime.getHeap());
      } else {
        // There are already a bucket head.
        entry->nextEntryInBucket.set(
            runtime,
            vmcast<HashMapEntry>(newHashTable->at(bucket).getObject(runtime)),
            runtime.getHeap());
      }
      // Update bucket head to the new entry.
      newHashTable->set(
          bucket,
          SmallHermesValue::encodeObjectValue(*entry, runtime),
          runtime.getHeap());

      entry = *oldNextInBucket;
    }
  }

  self->hashTable_.setNonNull(runtime, newHashTable.get(), runtime.getHeap());
  assert(
      self->hashTable_.getNonNull(runtime)->size() == self->capacity_ &&
      "Inconsistent capacity");
  return ExecutionStatus::RETURNED;
}

bool OrderedHashMap::has(
    Handle<OrderedHashMap> self,
    Runtime &runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(runtime, bucket, key.getHermesValue());
}

HashMapEntry *OrderedHashMap::find(
    Handle<OrderedHashMap> self,
    Runtime &runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(runtime, bucket, key.getHermesValue());
}

HermesValue OrderedHashMap::get(
    Handle<OrderedHashMap> self,
    Runtime &runtime,
    Handle<> key) {
  auto *entry = find(self, runtime, key);
  if (!entry) {
    return HermesValue::encodeUndefinedValue();
  }
  return entry->value;
}

ExecutionStatus OrderedHashMap::insert(
    Handle<OrderedHashMap> self,
    Runtime &runtime,
    Handle<> key,
    Handle<> value) {
  uint32_t bucket = hashToBucket(self, runtime, key);
  if (auto *entry =
          self->lookupInBucket(runtime, bucket, key.getHermesValue())) {
    // Element already exists, update value and return.
    entry->value.set(value.get(), runtime.getHeap());
    return ExecutionStatus::RETURNED;
  }
  // Create a new entry, set the key and value.
  auto crtRes = HashMapEntry::create(runtime);
  if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newMapEntry = runtime.makeHandle(std::move(*crtRes));
  newMapEntry->key.set(key.get(), runtime.getHeap());
  newMapEntry->value.set(value.get(), runtime.getHeap());
  auto *curBucketFront =
      castToMapEntry(self->hashTable_.getNonNull(runtime)->at(bucket), runtime);
  if (curBucketFront) {
    // If the bucket we are inserting to is not empty, we maintain the
    // linked list properly.
    newMapEntry->nextEntryInBucket.set(
        runtime, curBucketFront, runtime.getHeap());
  }
  // Set the newly inserted entry as the front of this bucket chain.
  self->hashTable_.getNonNull(runtime)->set(
      bucket,
      SmallHermesValue::encodeObjectValue(*newMapEntry, runtime),
      runtime.getHeap());

  if (!self->firstIterationEntry_) {
    // If we are inserting the first ever element, update
    // first iteration entry pointer.
    self->firstIterationEntry_.set(
        runtime, newMapEntry.get(), runtime.getHeap());
    self->lastIterationEntry_.set(
        runtime, newMapEntry.get(), runtime.getHeap());
  } else {
    // Connect the new entry with the last entry.
    self->lastIterationEntry_.getNonNull(runtime)->nextIterationEntry.set(
        runtime, newMapEntry.get(), runtime.getHeap());
    newMapEntry->prevIterationEntry.set(
        runtime, self->lastIterationEntry_, runtime.getHeap());

    HashMapEntry *previousLastEntry = self->lastIterationEntry_.get(runtime);
    self->lastIterationEntry_.set(
        runtime, newMapEntry.get(), runtime.getHeap());

    if (previousLastEntry && previousLastEntry->isDeleted()) {
      // If the last entry was a deleted entry, we no longer need to keep it.
      self->removeLinkedListNode(runtime, previousLastEntry, runtime.getHeap());
    }
  }

  self->size_++;
  return self->rehashIfNecessary(self, runtime);
}

bool OrderedHashMap::erase(
    Handle<OrderedHashMap> self,
    Runtime &runtime,
    Handle<> key) {
  uint32_t bucket = hashToBucket(self, runtime, key);
  HashMapEntry *prevEntry = nullptr;
  auto *entry =
      castToMapEntry(self->hashTable_.getNonNull(runtime)->at(bucket), runtime);
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
        runtime, entry->nextEntryInBucket, runtime.getHeap());
  } else {
    // The entry we are erasing is the front entry in the bucket, we need
    // to update the bucket head to the next entry in the bucket, if not
    // any, set to empty value.
    self->hashTable_.getNonNull(runtime)->set(
        bucket,
        entry->nextEntryInBucket
            ? SmallHermesValue::encodeObjectValue(entry->nextEntryInBucket)
            : SmallHermesValue::encodeEmptyValue(),
        runtime.getHeap());
  }

  entry->markDeleted(runtime);
  self->size_--;

  // Now delete the entry from the iteration linked list.
  // If we are deleting the last entry, don't remove it from the list yet.
  // This will ensure that if any iterator out there is currently pointing to
  // this entry, it will be able to follow on if we add new entries in the
  // future.
  if (entry != self->lastIterationEntry_.get(runtime)) {
    self->removeLinkedListNode(runtime, entry, runtime.getHeap());
  }

  self->rehashIfNecessary(self, runtime);

  return true;
}

HashMapEntry *OrderedHashMap::iteratorNext(
    Runtime &runtime,
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

void OrderedHashMap::clear(Runtime &runtime) {
  if (!firstIterationEntry_) {
    // Empty set.
    return;
  }

  // Clear the hash table.
  for (unsigned i = 0; i < capacity_; ++i) {
    auto entry = castToMapEntry(hashTable_.getNonNull(runtime)->at(i), runtime);
    // Delete every element reachable from the hash table.
    while (entry) {
      entry->markDeleted(runtime);
      entry = entry->nextEntryInBucket.get(runtime);
    }
    // Clear every element in the hash table.
    hashTable_.getNonNull(runtime)->setNonPtr(
        i, SmallHermesValue::encodeEmptyValue(), runtime.getHeap());
  }
  // Resize the hash table to the initial size.
  ArrayStorageSmall::resizeWithinCapacity(
      hashTable_.getNonNull(runtime), runtime, INITIAL_CAPACITY);
  capacity_ = INITIAL_CAPACITY;

  // After clearing, we will still keep the last deleted entry
  // in case there is an iterator out there
  // pointing to the middle of the iteration chain. We need it to be
  // able to merge back eventually.
  firstIterationEntry_.set(runtime, lastIterationEntry_, runtime.getHeap());
  firstIterationEntry_.getNonNull(runtime)->prevIterationEntry.setNull(
      runtime.getHeap());
  size_ = 0;
}

} // namespace vm
} // namespace hermes
