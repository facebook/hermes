/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_IDENTIFIERHASHTABLE_H
#define HERMES_VM_IDENTIFIERHASHTABLE_H

#include "hermes/Support/HashString.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/SymbolID.h"

#include "hermes/ADT/PtrOrInt.h"
#include "llvm/Support/MathExtras.h"

namespace hermes {
namespace vm {
class IdentifierTable;

class StringPrimitive;
#ifdef HERMESVM_SERIALIZE
class Serializer;
class Deserializer;
#endif

namespace detail {

/// A hash table to map from string reference (either UTF16Ref or ASCIIRef)
/// to index in the lookup vector. Use quadratic probing for conflicts.
/// Automatically grow when the size is beyond 0.75 of capacity.
class IdentifierHashTable {
  /// HashTable Entry.
  struct HashTableEntry {
    static constexpr uint32_t INVALID_ID = SymbolID::EMPTY_ID;
    static constexpr uint32_t DELETED_ID = SymbolID::DELETED_ID;

    /// Index to the lookupVector_.
    uint32_t index{INVALID_ID};

    HashTableEntry(uint32_t id) : index(id) {}

    HashTableEntry() = default;

    /// Whether this entry is empty (i.e. not occupied.)
    bool isEmpty() const {
      return index == INVALID_ID;
    }

    /// Whether this entry has been deleted.
    bool isDeleted() const {
      return index == DELETED_ID;
    }

    /// Whether this entry is valid and alive.
    bool isValid() const {
      return index < SymbolID::FIRST_INVALID_ID;
    }

    /// Mark this entry as deleted.
    void markAsDeleted() {
      index = SymbolID::DELETED_ID;
    }
  };

  /// Initial capacity of the hash table.
  static constexpr uint32_t INITIAL_CAPACITY = 1024;

  /// The hash table storage.
  std::vector<HashTableEntry> storage_{};

  /// Pointer to the identifier table that uses this hash table. We need it
  /// because we need to access the lookup vectors there.
  IdentifierTable *identifierTable_{};

  /// Number of valid entries in the hash table.
  uint32_t size_{0};

  /// Number of entries that's not empty. It is important to keep track of
  /// this value as table searches terminate based on whether we found an
  /// empty slot. When there are too few empty entries, table search will
  /// become inefficient. We should grow the table based on the ratio
  /// between nonEmptyEntryCount and capacity.
  uint32_t nonEmptyEntryCount_{0};

  /// Check whether we need to grow the hash table.
  bool shouldGrow() const {
    auto cap = capacity();
    assert(llvm::isPowerOf2_32(cap) && "capacity must be power of 2");
    // This is essentially cap * 0.75 < size.
    return cap - (cap >> 2) < nonEmptyEntryCount_;
  }

  /// Grow the hash table and rehash with \p newCapacity.
  void growAndRehash(uint32_t newCapacity);

  /// HashIteratorWrapper is a thin wrapper around char* and char16_t*
  /// to make sure that when iterating on it, *itr always return a
  /// char16_t type element. This ensures consistency in hash function.
  template <typename T>
  class HashIteratorWrapper {
    const T *ptr_;

   public:
    explicit HashIteratorWrapper(const T *ptr) : ptr_(ptr) {}

    char16_t operator*() const {
      return *ptr_;
    }

    bool operator==(const HashIteratorWrapper &other) const {
      return ptr_ == other.ptr_;
    }

    bool operator!=(const HashIteratorWrapper &other) const {
      return ptr_ != other.ptr_;
    }

    HashIteratorWrapper &operator++() {
      ++ptr_;
      return *this;
    }
  };

 public:
  explicit IdentifierHashTable(uint32_t capacity = INITIAL_CAPACITY) {
    storage_.resize(capacity);
  }

  /// Set the identifier table pointer.
  void setIdentifierTable(IdentifierTable *table) {
    identifierTable_ = table;
  }

  /// \return the size of the hash table (i.e. number of valid entries).
  uint32_t size() const {
    return size_;
  }

  /// \return the capacity of the hash table.
  uint32_t capacity() const {
    return storage_.size();
  }

  /// \return an estimate of the size of additional memory used by this
  /// IdentifierHashTable.
  size_t additionalMemorySize() const {
    return storage_.capacity() * sizeof(HashTableEntry);
  }

  /// Prepare the hash table to have sufficient capacity to contain \p count
  /// identifiers. To have less future conflicts, we make capacity at least
  /// twice large than \p count.
  void reserve(uint32_t count) {
    auto cap = capacity();
    assert(llvm::isPowerOf2_32(cap) && "capacity must be power of 2");
    if ((cap >> 1) < count) {
      // Calculate new capacity needed to contain all identifiers.
      // Always align it with power of 2.
      uint32_t newCapacity = llvm::NextPowerOf2(count * 2);
      growAndRehash(newCapacity);
    }
  }

  /// Find the index in the hash table given \p str.
  /// Computes the hash of the input string before lookup.
  /// If \p mustBeNew is true, we know that this string must be new to the
  /// table and we don't need to compare it with existing strings at all.
  /// \return index if found. If not found, \return the index to insert at.
  template <typename T>
  uint32_t lookupString(llvm::ArrayRef<T> str, bool mustBeNew = false) const {
    return lookupString(str, hermes::hashString(str), mustBeNew);
  }

  /// Find the index in the hash table given \p str.
  /// \p hash is the hash of the given string.
  /// If \p mustBeNew is true, we know that this string must be new to the
  /// table and we don't need to compare it with existing strings at all.
  /// \return index if found. If not found, \return the index to insert at.
  template <typename T>
  uint32_t lookupString(
      llvm::ArrayRef<T> str,
      uint32_t hash,
      bool mustBeNew = false) const;

  /// Similar to lookupString(llvm::ArrayRef<T>...), but provided
  /// a StringPrimitive argument.
  uint32_t lookupString(const StringPrimitive *str, bool mustBeNew = false)
      const;

  /// Similar to lookupString(llvm::ArrayRef<T>...), but provided
  /// a StringPrimitive argument, and the string hash.
  uint32_t lookupString(
      const StringPrimitive *str,
      uint32_t hash,
      bool mustBeNew = false) const;

  /// \return a const reference to the hash table storage given \p index.
  const HashTableEntry &at(uint32_t index) const {
    assert(index < capacity() && "Index out of bound");
    return storage_[index];
  }

  /// Insert an entry into index \p idx, with IdentifierID \p id.
  /// This is assuming that you have called lookupString to get \p idx.
  void insert(uint32_t idx, SymbolID id);

  /// Given the index to the storage \idx, delete it.
  void remove(uint32_t idx) {
    assert(storage_[idx].isValid() && "Deleting an invalid entry");
    storage_[idx].markAsDeleted();
    size_--;
  }

  /// Remove string \p ref from the hash table.
  /// Asserts that the string exists in the table.
  template <typename T>
  void remove(llvm::ArrayRef<T> ref) {
    remove(lookupString(ref));
  }

  /// Similar to remove(llvm::ArrayRef<T> ref), but given a StringPrimitive
  /// as argument. This function can only be called when removing an
  /// exisiting identifier, and hence the str must not be a rope.
  void remove(const StringPrimitive *str);

#ifdef HERMESVM_SERIALIZE
  void serialize(Serializer &s);

  void deserialize(Deserializer &d);
#endif
};

} // namespace detail
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_IDENTIFIERHASHTABLE_H
