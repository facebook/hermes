/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ORDERED_HASHMAP_H
#define HERMES_VM_ORDERED_HASHMAP_H

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Runtime.h"

#include <vector>

namespace hermes {
namespace vm {

/// Data structure for the entry of OrderedHashSet.
struct HashMapEntryKey {
  /// The key.
  GCSmallHermesValue key;
};

/// Data structure for the entry of OrderedHashMap.
struct HashMapEntryKeyValue {
  /// The key.
  GCSmallHermesValue key;
  /// The value.
  GCSmallHermesValue value;
};

/// HashMapEntry is a gc-managed entry in the OrderedHashMap.
/// We use HashMapEntry to form two separate linked lists,
/// one that tracks the insertion order for iteration purpose, one
/// tracks the list of entries in a hash table bucket for hash operations.
template <typename Data>
class HashMapEntryBase final : public GCCell, public Data {
  friend void HashMapEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  friend void HashSetEntryBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 public:
  static const VTable vt;

  /// Number of elements used in data table per each entry of hash table.
  static constexpr uint8_t kElementsPerEntry =
      std::is_same_v<Data, HashMapEntryKey> ? 1 : 2;

  /// This is temporarily here to help break up the diff stack until we get rid
  /// of HashMapEntryBase. The entire HashMapEntryBase will be removed later, so
  /// there won't be this code left around.
  uint32_t dataTableKeyIndex;

  /// Previous entry in insertion order.
  GCPointer<HashMapEntryBase> prevIterationEntry{nullptr};

  /// Next entry in insertion order.
  GCPointer<HashMapEntryBase> nextIterationEntry{nullptr};

  static constexpr CellKind getCellKind() {
    if constexpr (std::is_same_v<Data, HashMapEntryKeyValue>) {
      return CellKind::HashMapEntryKind;
    } else {
      static_assert(std::is_same_v<Data, HashMapEntryKey>);
      return CellKind::HashSetEntryKind;
    }
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == getCellKind();
  }

  static CallResult<PseudoHandle<HashMapEntryBase>> create(
      Runtime &runtime,
      uint32_t dataTableKeyIndex);

  static CallResult<PseudoHandle<HashMapEntryBase>> createLongLived(
      Runtime &runtime,
      uint32_t dataTableKeyIndex);

  /// \return the value. If the Data is HashMapEntryKey, the key will be
  /// returned.
  GCSmallHermesValue getValue() const {
    if constexpr (std::is_same_v<Data, HashMapEntryKeyValue>) {
      return Data::value;
    } else {
      return Data::key;
    }
  }

  /// Indicates whether this entry has been deleted.
  bool isDeleted() const {
    if constexpr (std::is_same_v<Data, HashMapEntryKeyValue>) {
      assert(
          Data::key.isEmpty() == Data::value.isEmpty() &&
          "Inconsistent deleted status");
    }
    return Data::key.isEmpty();
  }

  /// Mark this entry as deleted.
  void markDeleted(Runtime &runtime) {
    Data::key.setNonPtr(
        SmallHermesValue::encodeEmptyValue(), runtime.getHeap());
    if constexpr (std::is_same_v<Data, HashMapEntryKeyValue>) {
      Data::value.setNonPtr(
          SmallHermesValue::encodeEmptyValue(), runtime.getHeap());
    }
  }
}; // HashMapEntryBase

/// The bucket type for OrderedHashMap.
using HashMapEntry = HashMapEntryBase<HashMapEntryKeyValue>;
/// The bucket type for OrderedHashSet.
using HashSetEntry = HashMapEntryBase<HashMapEntryKey>;

/// OrderedHashMapBase is a hash map implementation that maintains insertion
/// order.
///
/// The map contains two conceptual parts:
/// 1) An ArrayStorageSmall representing a standard hash table with open
/// addressing, to store numbers that could be used to index into the data table
/// discussed in #2.
/// 2) A data table, which is an ArrayStorageSmall that stores the entry's key
/// and value in-place. This eliminates the need to create a new GCCell for each
/// entry of the hash table.
///
/// The data table is used in insertion order. When a new entry is added to the
/// hash table, the entry is appended to the data table. The key and value of
/// the entry are stored in-place at those slots. And then the hash table stores
/// a number that allows us to index into the data table to look at the values
/// later.
///
/// When the number of alive elements and deleted elements exceeds thresholds,
/// we will perform a rehash. When rehashing, a new data table is allocated and
/// only the elements that aren't deleted are copied to the new data table.
///
/// We chose linear probing for open addressing. It's simple and possibly faster
/// than quadratic probing because of memory locality. With a simple test case
/// to insert integers as keys, we didn't see any performance gain with
/// quadratic probing.
///
/// BucketType in the class template refers to either HashMapEntry or
/// HashSetEntry depending on whether we're storing keys only, or key/value
/// pairs. Derived in the class template should be the child gc-managed class.
template <typename BucketType, typename Derived>
class OrderedHashMapBase {
 public:
  using Entry = BucketType;
  using StorageType = ArrayStorageSmall;

  static void buildMetadata(const GCCell *cell, Metadata::Builder &mb);

  /// This method takes in a runtime to compute the hash for key, but does not
  /// allocate. Thus, it is safe to take in HermesValue here.
  /// \return true if the map contains a given HermesValue.
  bool has(Runtime &runtime, HermesValue key) const;

  /// This method takes in a runtime to compute the hash for key, but does not
  /// allocate. Thus, it is safe to take in HermesValue here.
  /// Lookup \p key in the table and \return the value if exists.
  /// Otherwise \return undefined.
  SmallHermesValue get(Runtime &runtime, HermesValue key) const;

  /// Insert a key/value pair, if not already existing. Function enabled only if
  /// this is a Map.
  template <
      typename = std::enable_if<
          std::is_same_v<BucketType, HashMapEntryBase<HashMapEntryKeyValue>>>>
  static ExecutionStatus
  insert(Handle<Derived> self, Runtime &runtime, Handle<> key, Handle<> value);

  /// Insert a key, if not already existing. Function enabled only if this is a
  /// Set.
  template <
      typename = std::enable_if<
          std::is_same_v<BucketType, HashMapEntryBase<HashMapEntryKey>>>>
  static ExecutionStatus
  insert(Handle<Derived> self, Runtime &runtime, Handle<> key);

  /// Erase a HermesValue from the map, \return true if succeed.
  static bool erase(Handle<Derived> self, Runtime &runtime, Handle<> key);

  /// Clear the map.
  static ExecutionStatus clear(Handle<Derived> self, Runtime &runtime);

  /// \return the size of the map.
  uint32_t size() const {
    assertInitialized();
    return size_;
  }

  /// \return the next element in insertion order.
  /// If \p entry is nullptr, we are starting the iteration from the first
  /// entry that is not deleted.
  /// Otherwise, we look for the next element after \p entry that is not
  /// deleted.
  BucketType *iteratorNext(Runtime &runtime, BucketType *entry = nullptr) const;

  explicit OrderedHashMapBase();

  /// Allocate the internal element storage.
  static ExecutionStatus initializeStorage(
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

    return ExecutionStatus::RETURNED;
  }

 protected:
  /// Initial capacity of the hash table.
  static constexpr uint32_t INITIAL_CAPACITY = 16;

  void assertInitialized() const {
    assert(hashTable_ && "Element storage uninitialized.");
    assert(dataTable_ && "Data Table uninitialized.");
  }

 private:
  /// The hashtable, with size always equal to capacity_. The number of
  /// reachable entries from hashTable_ should be equal to size_.
  GCPointer<StorageType> hashTable_{nullptr};

  /// The data table is where the actual data for each hash table entry is
  /// stored. It's an array where each entry can use up multiple elements. For
  /// example, for a Map, the key and the value would take up 2 elements in the
  /// array. We do this so that we could avoid doing new allocation for each
  /// entry of the hash table. Less allocations mean less GC time.
  GCPointer<StorageType> dataTable_{nullptr};

  /// The first entry ever inserted. We need this entry to begin an iteration.
  GCPointer<BucketType> firstIterationEntry_{nullptr};

  /// The last entry inserted. We need it to add new elements afterwards.
  GCPointer<BucketType> lastIterationEntry_{nullptr};

  /// The multiplier that the hash table grows or shrinks by each rehash.
  static constexpr uint32_t kGrowOrShrinkFactor{2};

  /// Maximum capacity of the hash table. This is checked in rehash() and we
  /// won't grow the capacity past this number. This is calculated so that the
  /// data table size won't overflow uint32_t.
  static constexpr uint32_t MAX_CAPACITY =
      StorageType::maxCapacityNoOverflow() / kGrowOrShrinkFactor /
      BucketType::kElementsPerEntry;

  /// Capacity of the hash table.
  uint32_t capacity_{INITIAL_CAPACITY};

  /// Number of alive entries in the storage.
  uint32_t size_{0};

  /// Number of deleted entries in the storage. The count will be reset during
  /// rehash and clear.
  uint32_t deletedCount_{0};

  /// \param hashTableCapacity hash table's capacity in number of elements
  /// \return the actual number of elements to allocate for data table based on
  /// the desired hash table capacity
  static constexpr uint32_t dataTableAllocationSize(
      uint32_t hashTableCapacity) {
    // Once the hash table reaches the rehash threshold, we're going to allocate
    // a new data table. Therefore, there is no need to allocate space for the
    // entire hash table capacity. By allocating less than the whole hash table
    // capacity, we save on memory usage and get better perf.
    return rehashThreshold(hashTableCapacity) * BucketType::kElementsPerEntry;
  }

  /// Hash a HermesValue to an index to our hash table.
  static uint32_t
  hashToBucket(uint32_t capacity, Runtime &runtime, HermesValue key) {
    auto hash = runtime.gcStableHashHermesValue(key);
    assert((capacity & (capacity - 1)) == 0 && "capacity_ must be power of 2");
    return hash & (capacity - 1);
  }

  /// Remove a node from the linked list.
  void removeLinkedListNode(Runtime &runtime, BucketType *entry, GC &gc);

  /// Lookup an entry with key as \p key in a given \p bucket (hash).
  /// \return The pair of the entry found and the index for it. The entry can be
  /// nullptr if the key doesn't exist. In that case, the index is the index of
  /// the available bucket for the give key.
  std::pair<BucketType *, uint32_t>
  lookupInBucket(Runtime &runtime, uint32_t bucket, HermesValue key) const;

  /// Perform a rehash operation by removing all deleted entries and re-insert
  /// them into new allocated hash table and data table. The old hash table and
  /// data table are discarded. The new capacity will be calculated by
  /// checkedNextCapacity(). Note that this function will always run rehash even
  /// if the new capacity is same as before. In such case, all the deleted
  /// bucket will be removed and become empty bucket.
  /// \param beforeAdd if true, we use current size + 1 to calculate the new
  /// capacity. This is used when calling from doInsert() because we're about to
  /// add one more entry. Otherwise, we use current size to calculate the new
  /// capacity. This is used when calling from erase(), so the size shouldn't
  /// increase.
  static ExecutionStatus
  rehash(Handle<Derived> self, Runtime &runtime, bool beforeAdd = false);

  /// Determine if we should shrink the hash table based on the current key
  /// count and capacity.
  static bool shouldShrink(uint32_t capacity, uint32_t keyCount) {
    // Divide the capacity to get the number we want for comparison. Division
    // ensures there won't be overflow.
    return keyCount <= capacity / 8 &&
        capacity > OrderedHashMapBase::INITIAL_CAPACITY;
  }

  /// Determine if we should rehash the hash table based on the current key
  /// count, delete count and capacity.
  static constexpr bool
  shouldRehash(uint32_t capacity, uint32_t keyCount, uint32_t deleteCount) {
    return (keyCount + deleteCount) >= rehashThreshold(capacity);
  }

  /// Determine the number of elements (alive and deleted) before a rehash
  /// should happen.
  static constexpr uint32_t rehashThreshold(uint32_t capacity) {
    // `>> 1` means that we're using a load factor of 0.5
    return capacity >> 1;
  }

  /// Calculate the next capacity based on the current capacity and key count.
  /// If there are enough unused capacity, then the next capacity will shrink.
  /// Capacity will grow otherwise. In the case that capacity cannot grow
  /// anymore, due to the increase would exceed MAX_CAPACITY, then RangeError
  /// will be raised.
  static CallResult<uint32_t>
  checkedNextCapacity(Runtime &runtime, uint32_t capacity, uint32_t keyCount) {
    if (!capacity)
      return OrderedHashMapBase::INITIAL_CAPACITY;

    if (shouldShrink(capacity, keyCount)) {
      assert(
          (capacity / kGrowOrShrinkFactor) >=
          OrderedHashMapBase::INITIAL_CAPACITY);
      return capacity / kGrowOrShrinkFactor;
    }
    static_assert(
        MAX_CAPACITY <= UINT32_MAX / kGrowOrShrinkFactor,
        "Avoid overflow on multiplying capacity by kGrowOrShrinkFactor");
    uint32_t newCapacity = capacity * kGrowOrShrinkFactor;
    if (LLVM_UNLIKELY(newCapacity > MAX_CAPACITY)) {
      if constexpr (std::is_same_v<BucketType, HashMapEntry>) {
        return runtime.raiseRangeError("Cannot insert new data. Map is full.");
      } else {
        return runtime.raiseRangeError("Cannot insert new data. Set is full.");
      }
    }
    return capacity * kGrowOrShrinkFactor;
  }

  /// Check if the bucket is deleted or not.
  static bool isDeleted(SmallHermesValue bucket) {
    return bucket.isNull();
  }

  /// Mark the bucket as deleted.
  static void
  deleteBucket(Handle<Derived> self, Runtime &runtime, uint32_t bucket) {
    /// Use NullValue to indicate that the bucket is deleted.
    self->hashTable_.getNonNull(runtime)->set(
        bucket, SmallHermesValue::encodeNullValue(), runtime.getHeap());
  }

  /// Helper function for inserting key or key/value pair into the container.
  /// This is used by the public insert() functions and only for adding keys
  /// that don't already exist. This will update the hash table as well as
  /// appending key (and value if applicable) to the data table.
  static ExecutionStatus doInsert(
      Handle<Derived> self,
      Runtime &runtime,
      uint32_t bucket,
      Handle<> key,
      Handle<> value);

}; // OrderedHashMapBase

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_ORDERED_HASHMAP_H
