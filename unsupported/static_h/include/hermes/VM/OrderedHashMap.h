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

/// HashMapEntry is a gc-managed entry in the OrderedHashMap.
/// We use HashMapEntry to form two separate linked lists,
/// one that tracks the insertion order for iteration purpose, one
/// tracks the list of entries in a hash table bucket for hash operations.
class HashMapEntry final : public GCCell {
 public:
  static const VTable vt;

  /// The key.
  GCHermesValue key;

  /// The value.
  GCHermesValue value;

  /// Previous entry in insertion order.
  GCPointer<HashMapEntry> prevIterationEntry{nullptr};

  /// Next entry in insertion order.
  GCPointer<HashMapEntry> nextIterationEntry{nullptr};

  /// Next entry in the hash table bucket.
  GCPointer<HashMapEntry> nextEntryInBucket{nullptr};

  static constexpr CellKind getCellKind() {
    return CellKind::HashMapEntryKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::HashMapEntryKind;
  }

  static CallResult<PseudoHandle<HashMapEntry>> create(Runtime &runtime);

  /// Indicates whether this entry has been deleted.
  bool isDeleted() const {
    assert(key.isEmpty() == value.isEmpty() && "Inconsistent deleted status");
    return value.isEmpty();
  }

  /// Mark this entry as deleted.
  void markDeleted(Runtime &runtime) {
    key.setNonPtr(HermesValue::encodeEmptyValue(), runtime.getHeap());
    value.setNonPtr(HermesValue::encodeEmptyValue(), runtime.getHeap());
  }
}; // HashMapEntry

/// OrderedHashMap is a gc-managed hash map that maintains insertion order.
/// The map contains two conceptual parts: a standard chained hash table,
/// to store and lookup HermesValue; a linked list to maintain the insertion
/// order.
/// When an element is added, it's always appended to the end of the linked
/// list. When an element is deleted, we mark it as deleted and remove it
/// from the linked list, unless it's the last element.
/// Iterators are pointers to the entries, which are always linked according
/// to insertion order.
/// When the size of the hash table becomes unreasonably large or small
/// comparing to the number of alive elements, we will rehash the table.
/// We always make sure that any entry at any moment can successfully find
/// the next entry according to insertion order, and rely on GC to manage
/// the "deleted" entries", free them when no more iterators are before
/// that entry.
class OrderedHashMap final : public GCCell {
  friend void OrderedHashMapBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

 public:
  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::OrderedHashMapKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::OrderedHashMapKind;
  }

  static CallResult<PseudoHandle<OrderedHashMap>> create(Runtime &runtime);

  /// \return true if the map contains a given HermesValue.
  static bool has(Handle<OrderedHashMap> self, Runtime &runtime, Handle<> key);

  /// Lookup \p key in the table and \return the value if exists.
  /// Otherwise \return undefined.
  static HermesValue
  get(Handle<OrderedHashMap> self, Runtime &runtime, Handle<> key);

  /// Lookup \p key in the table and \return the value if exists.
  /// Otherwise \return nullptr.
  static HashMapEntry *
  find(Handle<OrderedHashMap> self, Runtime &runtime, Handle<> key);

  /// Insert a key/value pair into the map, if not already existing.
  static ExecutionStatus insert(
      Handle<OrderedHashMap> self,
      Runtime &runtime,
      Handle<> key,
      Handle<> value);

  /// Erase a HermesValue from the map, \return true if succeed.
  static bool
  erase(Handle<OrderedHashMap> self, Runtime &runtime, Handle<> key);

  /// Clear the map.  The \p gc parameter is necessary for write barriers.
  void clear(Runtime &runtime);

  /// \return the size of the map.
  uint32_t size() const {
    return size_;
  }

  /// \return the next element in insertion order.
  /// If \p entry is nullptr, we are starting the iteration from the first
  /// entry that is not deleted.
  /// Otherwise, we look for the next element after \p entry that is not
  /// deleted.
  HashMapEntry *iteratorNext(Runtime &runtime, HashMapEntry *entry = nullptr)
      const;

  OrderedHashMap(Runtime &runtime, Handle<ArrayStorageSmall> hashTableStorage);

 private:
  /// The hashtable, with size always equal to capacity_. The number of
  /// reachable entries from hashTable_ should be equal to size_.
  GCPointer<ArrayStorageSmall> hashTable_{nullptr};

  /// The first entry ever inserted. We need this entry to begin an iteration.
  GCPointer<HashMapEntry> firstIterationEntry_{nullptr};

  /// The last entry inserted. We need it to add new elements afterwards.
  GCPointer<HashMapEntry> lastIterationEntry_{nullptr};

  /// Initial capacity of the hash table.
  static constexpr uint32_t INITIAL_CAPACITY = 16;

  /// Maximum capacity cannot exceed the maximum capacity of the underlying
  /// ArrayStorage.
  // It needs to be less than 1/4th the max 32-bit integer in order to use an
  // integer-based load factor check of 0.75.
  static constexpr uint32_t MAX_CAPACITY =
      std::min(ArrayStorageSmall::maxElements(), UINT32_MAX / 4);

  /// Capacity of the hash table.
  uint32_t capacity_{INITIAL_CAPACITY};

  /// Number of alive entries in the storage.
  uint32_t size_{0};

  /// Hash a HermesValue to an index to our hash table.
  static uint32_t
  hashToBucket(Handle<OrderedHashMap> self, Runtime &runtime, Handle<> key) {
    auto hash = runtime.gcStableHashHermesValue(key);
    assert(
        (self->capacity_ & (self->capacity_ - 1)) == 0 &&
        "capacity_ must be power of 2");
    return hash & (self->capacity_ - 1);
  }

  /// Remove a node from the linked list.
  void removeLinkedListNode(Runtime &runtime, HashMapEntry *entry, GC &gc);

  /// Lookup an entry with key as \p key in a given \p bucket.
  HashMapEntry *
  lookupInBucket(Runtime &runtime, uint32_t bucket, HermesValue key);

  /// Adjust the capacity of the hashtable and rehash if necessary.
  /// We make decisions based on the Load Factor (size / capacity).
  /// Rehash if load factor is outside of the range of [0.25, 0.75].
  static ExecutionStatus rehashIfNecessary(
      Handle<OrderedHashMap> self,
      Runtime &runtime);
}; // OrderedHashMap
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_ORDERED_HASHMAP_H
