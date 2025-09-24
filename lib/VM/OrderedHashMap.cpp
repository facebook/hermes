/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/OrderedHashMap.h"
#include "hermes/VM/JSMapImpl.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Operations.h"

namespace hermes {
namespace vm {
//===----------------------------------------------------------------------===//
// class HashMapEntryBase

template <typename Data>
const VTable HashMapEntryBase<Data>::vt{
    CellKind::HashMapEntryKind,
    cellSize<HashMapEntryBase<Data>>()};

void HashMapEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HashMapEntry *>(cell);
  mb.setVTable(&HashMapEntry::vt);
  mb.addField("key", &self->key);
  mb.addField("value", &self->value);
  mb.addField("prevIterationEntry", &self->prevIterationEntry);
  mb.addField("nextIterationEntry", &self->nextIterationEntry);
}

void HashSetEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HashSetEntry *>(cell);
  mb.setVTable(&HashSetEntry::vt);
  mb.addField("key", &self->key);
  mb.addField("prevIterationEntry", &self->prevIterationEntry);
  mb.addField("nextIterationEntry", &self->nextIterationEntry);
}

template <typename Data>
CallResult<PseudoHandle<HashMapEntryBase<Data>>> HashMapEntryBase<Data>::create(
    Runtime &runtime,
    uint32_t dataTableKeyIndex) {
  PseudoHandle entry =
      createPseudoHandle(runtime.makeAFixed<HashMapEntryBase<Data>>());
  entry->dataTableKeyIndex = dataTableKeyIndex;
  return entry;
}

template <typename Data>
CallResult<PseudoHandle<HashMapEntryBase<Data>>> HashMapEntryBase<
    Data>::createLongLived(Runtime &runtime, uint32_t dataTableKeyIndex) {
  PseudoHandle entry = createPseudoHandle(runtime.makeAFixed<
                                          HashMapEntryBase<Data>,
                                          HasFinalizer::No,
                                          LongLived::Yes>());
  entry->dataTableKeyIndex = dataTableKeyIndex;
  return entry;
}

template class HashMapEntryBase<HashMapEntryKeyValue>;
template class HashMapEntryBase<HashMapEntryKey>;

//===----------------------------------------------------------------------===//
// class OrderedHashMapBase

template <typename BucketType, typename Derived>
void OrderedHashMapBase<BucketType, Derived>::IteratorContext::reset() {
  if (index_ != nullptr) {
    index_->free();
  }
  index_ = nullptr;
  storage_ = nullptr;
}

/// A ManagedChunkedList element that stores the iterator index that should be
/// accessed next.
template <typename BucketType, typename Derived>
class OrderedHashMapBase<BucketType, Derived>::IteratorIndex {
 public:
  /// \return the iterator index value
  uint32_t getValue() const {
    assert(!isFree() && "Value not present");
    return value_;
  }

  /// Sets the iterator index value
  void setValue(uint32_t value) {
    assert(!isFree() && "Value not present");
    value_ = value;
  }

  /// \return whether an initial iteration has been done using this
  /// IteratorIndex
  bool getIsFirstIteration() const {
    assert(!isFree() && "Value not present");
    return isFirstIteration_;
  }

  /// Sets the state indicating whether an initial iteration has been done
  /// using this IteratorIndex. This is needed because we want to skip
  /// advancing the index value one time after a clear or if it's newly
  /// created.
  void setIsFirstIteration(bool isFirstIteration) {
    assert(!isFree() && "Value not present");
    isFirstIteration_ = isFirstIteration;
    if (isFirstIteration)
      value_ = 0;
  }

  /// Set the element to free, this can be called from either the mutator
  /// (when destroying an iterator) or the background thread (in finalizer).
  void free() {
    /// There are three operations related to this atomic variable:
    /// 1. Freeing the element on background thread.
    /// 2. Checking if the element is free on the mutator.
    /// 3. Freeing and reusing the element on the mutator.
    /// Since the only operation that background thread can do with the
    /// element is freeing it, we don't need a barrier to order the store.
    free_.store(true, std::memory_order_relaxed);
  }

  // Methods required by ManagedChunkedList.

  IteratorIndex() = default;

  /// \return true if the element has been freed. As noted in free(), since
  /// background thread can only free a element, we don't need a stricter
  /// order.
  bool isFree() const {
    return free_.load(std::memory_order_relaxed);
  }

  /// Get the next free element. Must not be called when this instance is
  /// occupied with a value.
  IteratorIndex *getNextFree() {
    assert(isFree() && "Can only get nextFree on a free element");
    return nextFree_;
  }

  /// Set the next free element. Must not be called when this instance is
  /// occupied with a value.
  void setNextFree(IteratorIndex *nextFree) {
    assert(isFree() && "Can only set nextFree on a free element");
    nextFree_ = nextFree;
  }

  /// Emplace new value to this element.
  void emplace() {
    assert(isFree() && "Emplacing already occupied element");
    free_.store(false, std::memory_order_relaxed);
    setIsFirstIteration(true);
  }

 private:
  union {
    /// Stores the index to the data table.
    uint32_t value_;
    IteratorIndex *nextFree_;
  };
  /// Indicates whether an iteration has been performed using this
  /// IteratorIndex. This is used for newly created IteratorIndex and any
  /// active IteratorIndex when a clear operation is performed on the hash
  /// table. Used by the advanceIterator function to know whether to update
  /// the index value.
  bool isFirstIteration_;
  /// Atomic state represents whether the element is free. It can be written
  /// by background thread (in finalizer) and read/written by mutator.
  std::atomic<bool> free_{true};
};

template <typename BucketType, typename Derived>
OrderedHashMapBase<BucketType, Derived>::OrderedHashMapBase() {}

template <typename BucketType, typename Derived>
void OrderedHashMapBase<BucketType, Derived>::buildMetadata(
    const GCCell *cell,
    Metadata::Builder &mb) {
  const auto *self = static_cast<const Derived *>(cell);
  mb.addField("hashTable", &self->hashTable_);
  mb.addField("dataTable", &self->dataTable_);
  mb.addField("firstIterationEntry", &self->firstIterationEntry_);
  mb.addField("lastIterationEntry", &self->lastIterationEntry_);
}

/// Allocate the internal element storage.
template <typename BucketType, typename Derived>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::initializeStorage(
    Handle<Derived> self,
    Runtime &runtime) {
  auto hashTableRes =
      StorageType::create(runtime, INITIAL_CAPACITY, INITIAL_CAPACITY);
  if (LLVM_UNLIKELY(hashTableRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto *hashTable = vmcast<StorageType>(*hashTableRes);
  self->hashTable_.setNonNull(runtime, hashTable, runtime.getHeap());

  // Create data table with corresponding capacity, but start off with 0 size.
  // Entries will be appended as the hash table receives elements.
  auto dataTableRes = StorageType::create(
      runtime, dataTableAllocationSize(INITIAL_CAPACITY), 0 /* size */);
  if (LLVM_UNLIKELY(dataTableRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto *dataTable = vmcast<StorageType>(*dataTableRes);
  self->dataTable_.setNonNull(runtime, dataTable, runtime.getHeap());

  // Create iterator metadata storage
  self->iteratorIndices_ = std::make_shared<ManagedChunkedList<IteratorIndex>>(
      0.5 /* occupancyRatio */, 0.5 /* sizingWeight */);

  return ExecutionStatus::RETURNED;
}

template <typename BucketType, typename Derived>
void OrderedHashMapBase<BucketType, Derived>::removeLinkedListNode(
    Runtime &runtime,
    BucketType *entry,
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

template <typename BucketType, typename Derived>
std::pair<BucketType *, uint32_t>
OrderedHashMapBase<BucketType, Derived>::lookupInBucket(
    Runtime &runtime,
    uint32_t bucket,
    HermesValue key) const {
  assert(
      hashTable_.getNonNull(runtime)->size() == capacity_ &&
      "Inconsistent capacity");
  [[maybe_unused]] const uint32_t firstBucket = bucket;

  NoAllocScope noAlloc{runtime};
  NoHandleScope noHandle{runtime};

  // We can deref the handles because we are in NoAllocScope.
  StorageType *hashTable = hashTable_.getNonNull(runtime);
  const uint32_t mask = capacity_ - 1;

  // Each bucket must be either empty, deleted (Null) or BucketType.
  while (true) {
    auto shv = hashTable->at(bucket);
    if (shv.isEmpty()) {
      break;
    }

    if (!isDeleted(shv)) {
      assert(shv.isObject());
      auto *entry = vmcast<BucketType>(shv.getObject(runtime));
      if (isSameValueZero(entry->key.unboxToHV(runtime), key)) {
        return {entry, bucket};
      }
    }

    bucket = (bucket + 1) & mask;
    assert(bucket != firstBucket && "Hash table has wrapped around");
  }

  return {nullptr, bucket};
}

template <typename BucketType, typename Derived>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::rehash(
    Handle<Derived> self,
    Runtime &runtime,
    bool beforeAdd) {
  struct : public Locals {
    PinnedValue<StorageType> newHashTable;
    PinnedValue<StorageType> newDataTable;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  const CallResult<uint32_t> newCapacity = checkedNextCapacity(
      runtime, self->capacity_, self->size_ + (beforeAdd ? 1 : 0));
  if (LLVM_UNLIKELY(newCapacity == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Set new capacity first to update the hash function.
  self->capacity_ = *newCapacity;

  // Create a new hash table.
  auto hashTableRes = StorageType::create(runtime, *newCapacity, *newCapacity);
  if (LLVM_UNLIKELY(hashTableRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.newHashTable.template castAndSetHermesValue<StorageType>(*hashTableRes);

  // Create a new data table.
  auto dataTableRes = StorageType::create(
      runtime,
      dataTableAllocationSize(*newCapacity),
      self->size_ * BucketType::kElementsPerEntry);
  if (LLVM_UNLIKELY(dataTableRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.newDataTable.template castAndSetHermesValue<StorageType>(*dataTableRes);

  // Now re-add all entries to the hash table.
  NoAllocScope noAlloc{runtime};
  MutableHandle<> keyHandle{runtime};

  NoHandleScope noHandle{runtime};

  // We can deref the handles because we are in NoAllocScope.
  StorageType *newHashTable = lv.newHashTable.get();
  StorageType *newDataTable = lv.newDataTable.get();
  OrderedHashMapBase<BucketType, Derived> *rawSelf = *self;
  const uint32_t mask = rawSelf->capacity_ - 1;
  auto entry = rawSelf->firstIterationEntry_.get(runtime);
  uint32_t newDataTableKeyIndex = 0;
  while (entry) {
    if (!entry->isDeleted()) {
      keyHandle = entry->key.unboxToHV(runtime);
      uint32_t bucket = hashToBucket(rawSelf->capacity_, runtime, *keyHandle);
      [[maybe_unused]] const uint32_t firstBucket = bucket;
      while (!newHashTable->at(bucket).isEmpty()) {
        // Find another bucket if it is not empty.
        bucket = (bucket + 1) & mask;
        assert(firstBucket != bucket && "Hash table is full!");
      }
      // Set the new entry to the bucket.
      newDataTable->set(newDataTableKeyIndex, entry->key, runtime.getHeap());
      if constexpr (std::is_same_v<BucketType, HashMapEntry>) {
        newDataTable->set(
            newDataTableKeyIndex + 1, entry->value, runtime.getHeap());
      }
      entry->dataTableKeyIndex = newDataTableKeyIndex;
      newHashTable->set(
          bucket,
          SmallHermesValue::encodeObjectValue(entry, runtime),
          runtime.getHeap());
      newDataTableKeyIndex += BucketType::kElementsPerEntry;
    }
    entry = entry->nextIterationEntry.get(runtime);
  }

  // Adjust any active iterators to handle the fact that we removed deleted
  // entries in the new data table.
  rawSelf->updateIteratorIndicesForRehash(runtime);

  rawSelf->deletedCount_ = 0;
  rawSelf->hashTable_.setNonNull(runtime, newHashTable, runtime.getHeap());
  assert(
      rawSelf->hashTable_.getNonNull(runtime)->size() == rawSelf->capacity_ &&
      "Inconsistent capacity");
  rawSelf->dataTable_.setNonNull(runtime, newDataTable, runtime.getHeap());
  return ExecutionStatus::RETURNED;
}

template <typename BucketType, typename Derived>
void OrderedHashMapBase<BucketType, Derived>::updateIteratorIndicesForRehash(
    Runtime &runtime) {
  // For each of the deleted entry, we store the entry index to help iterators
  // adjust their entry index in the next step.
  // Suppose you have the following data table (kElementsPerEntry = 1), and 'x'
  // marks the deleted entries.
  // entry index: 0 1 2 3 4 5 6 7 8
  //            : 0 x 2 x 4 x 6 x 8
  // We'll record the deleted indices as: [1 3 5 7]
  std::vector<uint32_t> deletedEntryIndices;
  deletedEntryIndices.reserve(deletedCount_);
  uint32_t oldDataTableKeyIndex = 0;
  for (uint32_t i = 0; i < size_ + deletedCount_; i++) {
    auto shv = dataTable_.getNonNull(runtime)->at(oldDataTableKeyIndex);
    if (shv.isEmpty()) {
      // Entry is deleted
      deletedEntryIndices.push_back(i);
    }
    oldDataTableKeyIndex += BucketType::kElementsPerEntry;
  }
  assert(
      deletedEntryIndices.size() == deletedCount_ &&
      "Should encounter same number of deleted elements as the deletedCount_");

  // Update all iterators to adjust their indices for the rehash. We take the
  // IteratorIndex's value and reduce it by the number of deleted elements that
  // occur before that index.
  //
  // For example, if we have the following data table (kElementsPerEntry = 1):
  // entry index: 0 1 2 3 4 5 6 7 8
  // values     : 0 x 2 x 4 x 6 x 8
  // And the iterator has already navigated and printed the #4 entry, then the
  // IteratorIndex value will be 5. Since there are 2 deleted entries already
  // traversed (entry 1 and entry 3), the IteratorIndex value should become 3
  // after the rehash (representing that we already navigated. #0, #2, and #4
  // entries).
  //
  // In the new data table, the values are as follows:
  // entry index: 0 1 2 3 4
  // values     : 0 2 4 6 8
  // Therefore, IteratorIndex value = 3 will allow the iterator to print the
  // next value, which should be 6.
  iteratorIndices_->forEach([&deletedEntryIndices](IteratorIndex &index) {
    // Perform the std::lower_bound algorithm to do a binary search to find
    // the first deleted entry index >= IteratorIndex value. Using the same
    // example as above, the \p deletedEntryIndices would be [1 3 5 7].
    // So for IteratorIndex value = 5, then \p firstEqualOrGreaterIter will
    // be calculated to be the iterator pointing at index 2 of \p
    // deletedEntryIndices. We then know the number of deleted entry indices
    // that appeared before (entry 1 and entry 3).
    //
    // If, however, IteratorIndex value = 8 and there aren't any deleted
    // entry index that is >= 8, then \p firstEqualOrGreaterIter will be
    // deletedEntryIndices.end().
    const uint32_t iteratorIndex = index.getValue();
    auto firstEqualOrGreaterIter = std::lower_bound(
        deletedEntryIndices.begin(), deletedEntryIndices.end(), iteratorIndex);

    uint32_t numDeletedBeforeIndex =
        std::distance(deletedEntryIndices.begin(), firstEqualOrGreaterIter);
    index.setValue(iteratorIndex - numDeletedBeforeIndex);
  });
}

template <typename BucketType, typename Derived>
bool OrderedHashMapBase<BucketType, Derived>::has(
    Runtime &runtime,
    HermesValue key) const {
  NoAllocScope noAlloc{runtime};
  assertInitialized();
  auto bucket = hashToBucket(capacity_, runtime, key);
  return lookupInBucket(runtime, bucket, key).first;
}

template <typename BucketType, typename Derived>
SmallHermesValue OrderedHashMapBase<BucketType, Derived>::get(
    Runtime &runtime,
    HermesValue key) const {
  NoAllocScope noAlloc{runtime};
  assertInitialized();
  auto bucket = hashToBucket(capacity_, runtime, key);
  auto *entry = lookupInBucket(runtime, bucket, key).first;
  if (!entry) {
    return SmallHermesValue::encodeUndefinedValue();
  }
  return entry->getValue();
}

template <typename BucketType, typename Derived>
template <typename>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::insert(
    Handle<Derived> self,
    Runtime &runtime,
    Handle<> key,
    Handle<> value) {
  self->assertInitialized();
  uint32_t bucket = hashToBucket(self->capacity_, runtime, *key);

  // Find the bucket for this key. If the entry already exists, update the value
  // and return.
  {
    // Note that SmallHermesValue::encodeHermesValue() may allocate, so we need
    // to call it before getting raw pointer for entry.
    auto shv =
        SmallHermesValue::encodeHermesValue(value.getHermesValue(), runtime);
    BucketType *entry = nullptr;
    std::tie(entry, bucket) =
        self->lookupInBucket(runtime, bucket, key.getHermesValue());
    if (entry) {
      // Element for the key already exists, update value and return.
      entry->value.set(shv, runtime.getHeap());
      self->dataTable_.getNonNull(runtime)->set(
          entry->dataTableKeyIndex + 1, shv, runtime.getHeap());
      return ExecutionStatus::RETURNED;
    }
  }

  return doInsert(self, runtime, bucket, key, value);
}

template <typename BucketType, typename Derived>
template <typename>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::insert(
    Handle<Derived> self,
    Runtime &runtime,
    Handle<> key) {
  self->assertInitialized();
  uint32_t bucket = hashToBucket(self->capacity_, runtime, *key);

  // Find the bucket for this key. If the entry already exists, then return.
  {
    BucketType *entry = nullptr;
    std::tie(entry, bucket) =
        self->lookupInBucket(runtime, bucket, key.getHermesValue());
    if (entry) {
      return ExecutionStatus::RETURNED;
    }
  }

  return doInsert(
      self, runtime, bucket, key, HandleRootOwner::getUndefinedValue());
}

template <typename BucketType, typename Derived>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::doInsert(
    Handle<Derived> self,
    Runtime &runtime,
    uint32_t bucket,
    Handle<> key,
    Handle<> value) {
  struct : public Locals {
    PinnedValue<StorageType> dataTable;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  // Run rehash if necessary before inserting.
  if (shouldRehash(self->capacity_, self->size_, self->deletedCount_)) {
    if (LLVM_UNLIKELY(
            self->rehash(self, runtime, true) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // Find a new empty bucket after rehash.
    bucket = hashToBucket(self->capacity_, runtime, *key);
    BucketType *entry = nullptr;
    std::tie(entry, bucket) =
        self->lookupInBucket(runtime, bucket, key.getHermesValue());
    assert(!entry && "After rehash, we must be able to find an empty bucket");
  }

  // Create a new entry, set the key and value.
  // Allocate the new entry in the same generation as the hash table to avoid
  // pointers from old gen to young gen.
  auto crtRes = runtime.getHeap().inYoungGen(self->hashTable_.get(runtime))
      ? BucketType::create(
            runtime, self->dataTable_.getNonNull(runtime)->size())
      : BucketType::createLongLived(
            runtime, self->dataTable_.getNonNull(runtime)->size());
  if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Note that SmallHermesValue::encodeHermesValue() may allocate, so we need to
  // call it and set to newMapEntry one at a time.
  lv.dataTable = self->dataTable_.getNonNull(runtime);
  auto newMapEntry = runtime.makeHandle(std::move(*crtRes));
  auto k = SmallHermesValue::encodeHermesValue(key.getHermesValue(), runtime);
  newMapEntry->key.set(k, runtime.getHeap());
  assert(
      lv.dataTable->size() < lv.dataTable->capacity() &&
      "Data table should always have enough capacity");
  lv.dataTable->pushWithinCapacity(runtime, k);
  if constexpr (std::is_same_v<BucketType, HashMapEntry>) {
    auto v =
        SmallHermesValue::encodeHermesValue(value.getHermesValue(), runtime);
    newMapEntry->value.set(v, runtime.getHeap());
    lv.dataTable->pushWithinCapacity(runtime, v);
  }

  // After here, no allocation
  NoAllocScope noAlloc{runtime};
  NoHandleScope noHandle{runtime};

  // We can deref in NoAllocScope.
  OrderedHashMapBase<BucketType, Derived> *rawSelf = *self;
  GC &heap = runtime.getHeap();

  // Set the newly inserted entry as the front of this bucket chain.
  rawSelf->hashTable_.getNonNull(runtime)->set(
      bucket,
      SmallHermesValue::encodeObjectValue(*newMapEntry, runtime),
      runtime.getHeap());

  if (!rawSelf->firstIterationEntry_) {
    // If we are inserting the first ever element, update
    // first iteration entry pointer.
    rawSelf->firstIterationEntry_.set(runtime, newMapEntry.get(), heap);
    rawSelf->lastIterationEntry_.set(runtime, newMapEntry.get(), heap);
  } else {
    // Connect the new entry with the last entry.
    rawSelf->lastIterationEntry_.getNonNull(runtime)->nextIterationEntry.set(
        runtime, newMapEntry.get(), heap);
    newMapEntry->prevIterationEntry.set(
        runtime, rawSelf->lastIterationEntry_, heap);

    BucketType *previousLastEntry = rawSelf->lastIterationEntry_.get(runtime);
    rawSelf->lastIterationEntry_.set(runtime, newMapEntry.get(), heap);

    if (previousLastEntry && previousLastEntry->isDeleted()) {
      // If the last entry was a deleted entry, we no longer need to keep it.
      rawSelf->removeLinkedListNode(runtime, previousLastEntry, heap);
    }
  }

  rawSelf->size_++;
  return ExecutionStatus::RETURNED;
}

template <typename BucketType, typename Derived>
bool OrderedHashMapBase<BucketType, Derived>::erase(
    Handle<Derived> self,
    Runtime &runtime,
    Handle<> key) {
  self->assertInitialized();
  uint32_t bucket = hashToBucket(self->capacity_, runtime, *key);
  BucketType *entry = nullptr;
  std::tie(entry, bucket) =
      self->lookupInBucket(runtime, bucket, key.getHermesValue());
  if (!entry) {
    // Element does not exist.
    return false;
  }

  // Mark the bucket as deleted. We remove this in rehash.
  deleteBucket(self, runtime, bucket);
  entry->markDeleted(runtime);
  self->deletedCount_++;
  self->size_--;

  // Now delete the entry from the iteration linked list.
  // If we are deleting the last entry, don't remove it from the list yet.
  // This will ensure that if any iterator out there is currently pointing to
  // this entry, it will be able to follow on if we add new entries in the
  // future.
  if (entry != self->lastIterationEntry_.get(runtime)) {
    self->removeLinkedListNode(runtime, entry, runtime.getHeap());
  }

  if (shouldShrink(self->capacity_, self->size_))
    self->rehash(self, runtime);

  return true;
}

template <typename BucketType, typename Derived>
typename OrderedHashMapBase<BucketType, Derived>::IteratorContext
OrderedHashMapBase<BucketType, Derived>::newIterator(Runtime &runtime) {
  return IteratorContext(iteratorIndices_->add(), iteratorIndices_);
}

template <typename BucketType, typename Derived>
bool OrderedHashMapBase<BucketType, Derived>::advanceIterator(
    Runtime &runtime,
    IteratorContext &iterCtx) {
  uint32_t i = 0;

  assert(iterCtx.initialized() && "Invalid IteratorContext");
  IteratorIndex *index = iterCtx.index_;

  // Check if the iterator was newly created or was reset due to clearing the
  // OrderedHashMap. In both cases, we won't advance the index this time around
  // since the index is already at 0.
  if (!index->getIsFirstIteration()) {
    // Advance to the next entry.
    i = index->getValue() + 1;
  } else {
    index->setIsFirstIteration(false);
  }

  // The iterator index might be pointing to a deleted element, so need to
  // iterate and find the next one that isn't deleted.
  for (; i < size_ + deletedCount_; ++i) {
    if (!dataTable_.getNonNull(runtime)
             ->at(i * BucketType::kElementsPerEntry)
             .isEmpty()) {
      // Found valid element
      index->setValue(i);
      return true;
    }
  }

  return false;
}

template <typename BucketType, typename Derived>
HermesValue OrderedHashMapBase<BucketType, Derived>::iteratorKey(
    Runtime &runtime,
    const IteratorContext &iterCtx) const {
  assert(iterCtx.initialized() && "Invalid IteratorContext");
  auto shv = dataTable_.getNonNull(runtime)->at(
      iterCtx.index_->getValue() * BucketType::kElementsPerEntry);
  assert(!shv.isEmpty() && "Invalid key encountered");
  return shv.unboxToHV(runtime);
}

template <typename BucketType, typename Derived>
HermesValue OrderedHashMapBase<BucketType, Derived>::iteratorValue(
    Runtime &runtime,
    const IteratorContext &iterCtx) const {
  assert(iterCtx.initialized() && "Invalid IteratorContext");
  if constexpr (std::is_same_v<BucketType, HashMapEntry>) {
    auto shv = dataTable_.getNonNull(runtime)->at(
        iterCtx.index_->getValue() * BucketType::kElementsPerEntry + 1);
    assert(!shv.isEmpty() && "Invalid value encountered");
    return shv.unboxToHV(runtime);
  } else {
    auto shv = dataTable_.getNonNull(runtime)->at(
        iterCtx.index_->getValue() * BucketType::kElementsPerEntry);
    assert(!shv.isEmpty() && "Invalid key encountered");
    return shv.unboxToHV(runtime);
  }
}

template <typename BucketType, typename Derived>
ExecutionStatus OrderedHashMapBase<BucketType, Derived>::clear(
    Handle<Derived> self,
    Runtime &runtime) {
  self->assertInitialized();
  if (!self->firstIterationEntry_) {
    // Empty set.
    return ExecutionStatus::RETURNED;
  }

  // Clear the hash table.
  for (unsigned i = 0; i < self->capacity_; ++i) {
    auto shv = self->hashTable_.getNonNull(runtime)->at(i);
    // Delete every element reachable from the hash table.
    if (shv.isObject()) {
      vmcast<BucketType>(shv.getObject(runtime))->markDeleted(runtime);
    }
    // Clear every element in the hash table.
    self->hashTable_.getNonNull(runtime)->setNonPtr(
        i, SmallHermesValue::encodeEmptyValue(), runtime.getHeap());
  }
  // Resize the hash table to the initial size.
  StorageType::resizeWithinCapacity(
      self->hashTable_.getNonNull(runtime), runtime, INITIAL_CAPACITY);
  self->capacity_ = INITIAL_CAPACITY;

  // Resize the data table back to 0.
  self->dataTable_.getNonNull(runtime)->clear(runtime);

  // Update all iterators back to index 0.
  self->iteratorIndices_->forEach(
      [](IteratorIndex &index) { index.setIsFirstIteration(true); });

  // After clearing, we will still keep the last deleted entry
  // in case there is an iterator out there
  // pointing to the middle of the iteration chain. We need it to be
  // able to merge back eventually.
  self->firstIterationEntry_.set(
      runtime, self->lastIterationEntry_, runtime.getHeap());
  self->firstIterationEntry_.getNonNull(runtime)->prevIterationEntry.setNull(
      runtime.getHeap());
  self->deletedCount_ = 0;
  self->size_ = 0;
  return ExecutionStatus::RETURNED;
}

template class OrderedHashMapBase<HashMapEntry, JSMapImpl<CellKind::JSMapKind>>;
template class OrderedHashMapBase<HashSetEntry, JSMapImpl<CellKind::JSSetKind>>;

template ExecutionStatus
OrderedHashMapBase<HashMapEntry, JSMapImpl<CellKind::JSMapKind>>::insert(
    Handle<JSMapImpl<CellKind::JSMapKind>> self,
    Runtime &runtime,
    Handle<> key,
    Handle<> value);
template ExecutionStatus
OrderedHashMapBase<HashSetEntry, JSMapImpl<CellKind::JSSetKind>>::insert(
    Handle<JSMapImpl<CellKind::JSSetKind>> self,
    Runtime &runtime,
    Handle<> key);

} // namespace vm
} // namespace hermes
