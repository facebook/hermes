/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/OrderedHashMap.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Operations.h"

namespace hermes {
namespace vm {
//===----------------------------------------------------------------------===//
// class HashMapEntry

VTable HashMapEntry::vt{CellKind::HashMapEntryKind, sizeof(HashMapEntry)};

void HashMapEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HashMapEntry *>(cell);
  mb.addField("@key", &self->key);
  mb.addField("@value", &self->value);
  mb.addField("@prevIterationEntry", &self->prevIterationEntry);
  mb.addField("@nextIterationEntry", &self->nextIterationEntry);
  mb.addField("@nextEntryInBucket", &self->nextEntryInBucket);
}

CallResult<HermesValue> HashMapEntry::create(Runtime *runtime) {
  void *mem = runtime->alloc(sizeof(HashMapEntry));
  return HermesValue::encodeObjectValue(new (mem) HashMapEntry(runtime));
}

//===----------------------------------------------------------------------===//
// class OrderedHashMap

VTable OrderedHashMap::vt{CellKind::OrderedHashMapKind, sizeof(OrderedHashMap)};

void OrderedHashMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const OrderedHashMap *>(cell);
  mb.addField("@hashTable", &self->hashTable_);
  mb.addField("@firstIterationEntry", &self->firstIterationEntry_);
  mb.addField("@lastIterationEntry", &self->lastIterationEntry_);
}

OrderedHashMap::OrderedHashMap(
    Runtime *runtime,
    Handle<ArrayStorage> hashTableStorage)
    : GCCell(&runtime->getHeap(), &vt),
      hashTable_(hashTableStorage.get(), &runtime->getHeap()) {
  ArrayStorage::resizeWithinCapacity(
      hashTableStorage, runtime, INITIAL_CAPACITY);
}

CallResult<HermesValue> OrderedHashMap::create(Runtime *runtime) {
  auto arrRes = ArrayStorage::create(runtime, INITIAL_CAPACITY);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto hashTableStorage = runtime->makeHandle<ArrayStorage>(*arrRes);

  void *mem = runtime->alloc(sizeof(OrderedHashMap));
  return HermesValue::encodeObjectValue(
      new (mem) OrderedHashMap(runtime, hashTableStorage));
}

void OrderedHashMap::removeLinkedListNode(HashMapEntry *entry, GC *gc) {
  assert(entry != lastIterationEntry_ && "Cannot remove the last entry");
  if (entry->prevIterationEntry) {
    entry->prevIterationEntry->nextIterationEntry.set(
        entry->nextIterationEntry, gc);
  }
  if (entry->nextIterationEntry) {
    entry->nextIterationEntry->prevIterationEntry.set(
        entry->prevIterationEntry, gc);
  }
  if (entry == firstIterationEntry_) {
    firstIterationEntry_.set(entry->nextIterationEntry, gc);
  }
  entry->prevIterationEntry = nullptr;
}

HashMapEntry *OrderedHashMap::lookupInBucket(uint32_t bucket, HermesValue key) {
  assert(hashTable_->size() == capacity_ && "Inconsistent capacity");
  auto *entry = dyn_vmcast<HashMapEntry>(hashTable_->at(bucket));
  while (entry && !isSameValueZero(entry->key, key)) {
    entry = entry->nextEntryInBucket;
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
      // Eventually, we should cap the max value at the
      // largest power of two <= MAX_CAPACITY.
      // For now, though leave the value capped at MAX_CAPACITY for
      // now, in case that's connected to T42745080.
      newCapacity = MAX_CAPACITY;
      // Record the fact that this overflow occurred (in case
      // it's correlated with crashes).
      HERMES_EXTRA_DEBUG(if (!self->overflowRecorded_) {
        runtime->getCrashManager().setCustomData(
            "Hermes_OrderedHashMap_overflow", "1");
        self->overflowRecorded_ = true;
      });
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
  MutableHandle<> tmpHandle{runtime};
  for (unsigned i = 0, len = self->hashTable_->size(); i < len; ++i) {
    auto entry = dyn_vmcast<HashMapEntry>(self->hashTable_->at(i));
    while (entry) {
      tmpHandle = entry->key;
      uint32_t bucket = hashToBucket(self, runtime, tmpHandle);
      HashMapEntry *oldNextInBucket = entry->nextEntryInBucket;
      if (newHashTable->at(bucket).isEmpty()) {
        // Empty bucket.
        entry->nextEntryInBucket = nullptr;
      } else {
        // There are already a bucket head.
        entry->nextEntryInBucket.set(
            vmcast<HashMapEntry>(newHashTable->at(bucket)),
            &runtime->getHeap());
      }
      // Update bucket head to the new entry.
      newHashTable->at(bucket).set(
          HermesValue::encodeObjectValue(entry), &runtime->getHeap());

      entry = oldNextInBucket;
    }
  }

  self->hashTable_.set(newHashTable.get(), &runtime->getHeap());
  assert(
      self->hashTable_->size() == self->capacity_ && "Inconsistent capacity");
  return ExecutionStatus::RETURNED;
}

bool OrderedHashMap::has(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(bucket, key.getHermesValue());
}

HashMapEntry *OrderedHashMap::find(
    Handle<OrderedHashMap> self,
    Runtime *runtime,
    Handle<> key) {
  auto bucket = hashToBucket(self, runtime, key);
  return self->lookupInBucket(bucket, key.getHermesValue());
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
  if (auto *entry = self->lookupInBucket(bucket, key.getHermesValue())) {
    // Element already exists, update value and return.
    entry->value.set(value.get(), &runtime->getHeap());
    return ExecutionStatus::RETURNED;
  }
  // Create a new entry, set the key and value.
  auto crtRes = HashMapEntry::create(runtime);
  if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newMapEntry = runtime->makeHandle<HashMapEntry>(*crtRes);
  newMapEntry->key.set(key.get(), &runtime->getHeap());
  newMapEntry->value.set(value.get(), &runtime->getHeap());
  auto *curBucketFront = dyn_vmcast<HashMapEntry>(self->hashTable_->at(bucket));
  if (curBucketFront) {
    // If the bucket we are inserting to is not empty, we maintain the
    // linked list properly.
    newMapEntry->nextEntryInBucket.set(curBucketFront, &runtime->getHeap());
  }
  // Set the newly inserted entry as the front of this bucket chain.
  self->hashTable_->at(bucket).set(
      newMapEntry.getHermesValue(), &runtime->getHeap());

  if (!self->firstIterationEntry_) {
    // If we are inserting the first ever element, update
    // first iteration entry pointer.
    self->firstIterationEntry_.set(newMapEntry.get(), &runtime->getHeap());
    self->lastIterationEntry_.set(newMapEntry.get(), &runtime->getHeap());
  } else {
    // Connect the new entry with the last entry.
    self->lastIterationEntry_->nextIterationEntry.set(
        newMapEntry.get(), &runtime->getHeap());
    newMapEntry->prevIterationEntry.set(
        self->lastIterationEntry_, &runtime->getHeap());

    HashMapEntry *previousLastEntry = self->lastIterationEntry_;
    self->lastIterationEntry_.set(newMapEntry.get(), &runtime->getHeap());

    if (previousLastEntry && previousLastEntry->isDeleted()) {
      // If the last entry was a deleted entry, we no longer need to keep it.
      self->removeLinkedListNode(previousLastEntry, &runtime->getHeap());
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
  auto *entry = dyn_vmcast<HashMapEntry>(self->hashTable_->at(bucket));
  while (entry && !isSameValueZero(entry->key, key.getHermesValue())) {
    prevEntry = entry;
    entry = entry->nextEntryInBucket;
  }
  if (!entry) {
    // Element does not exist.
    return false;
  }

  if (prevEntry) {
    // The entry we are deleting has a previous entry, update the link.
    prevEntry->nextEntryInBucket.set(
        entry->nextEntryInBucket, &runtime->getHeap());
  } else {
    // The entry we are erasing is the front entry in the bucket, we need
    // to update the bucket head to the next entry in the bucket, if not
    // any, set to empty value.
    self->hashTable_->at(bucket).set(
        entry->nextEntryInBucket
            ? HermesValue::encodeObjectValue(entry->nextEntryInBucket)
            : HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
  }

  entry->markDeleted();
  self->size_--;

  // Now delete the entry from the iteration linked list.
  // If we are deleting the last entry, don't remove it from the list yet.
  // This will ensure that if any iterator out there is currently pointing to
  // this entry, it will be able to follow on if we add new entries in the
  // future.
  if (entry != self->lastIterationEntry_) {
    self->removeLinkedListNode(entry, &runtime->getHeap());
  }

  self->rehashIfNecessary(self, runtime);

  return true;
}

HashMapEntry *OrderedHashMap::iteratorNext(HashMapEntry *entry) const {
  if (entry == nullptr) {
    // Starting a new iteration from the first entry.
    entry = firstIterationEntry_;
  } else {
    // Advance to the next entry.
    entry = entry->nextIterationEntry;
  }

  // Make sure the entry we returned (if not nullptr) must not be deleted.
  while (entry && entry->isDeleted()) {
    entry = entry->nextIterationEntry;
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
    auto entry = dyn_vmcast<HashMapEntry>(hashTable_->at(i));
    // Delete every element reachable from the hash table.
    while (entry) {
      entry->markDeleted();
      entry = entry->nextEntryInBucket;
    }
    // Clear every element in the hash table.
    hashTable_->at(i).setNonPtr(HermesValue::encodeEmptyValue());
  }
  // Resize the hash table to the initial size.
  ArrayStorage::resizeWithinCapacity(hashTable_, runtime, INITIAL_CAPACITY);
  capacity_ = INITIAL_CAPACITY;

  // After clearing, we will still keep the last deleted entry
  // in case there is an iterator out there
  // pointing to the middle of the iteration chain. We need it to be
  // able to merge back eventually.
  firstIterationEntry_.set(lastIterationEntry_, &runtime->getHeap());
  firstIterationEntry_->prevIterationEntry = nullptr;
  size_ = 0;
}

} // namespace vm
} // namespace hermes
