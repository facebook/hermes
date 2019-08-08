/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/detail/IdentifierHashTable.h"

#include "hermes/VM/Deserializer.h"
#include "hermes/VM/Serializer.h"
#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm::detail;
// In GCC/CLANG, method definitions can refer to ancestor namespaces of
// the namespace that the class is declared in without namespace qualifiers.
// This is not allowed in MSVC.
using hermes::vm::Deserializer;
using hermes::vm::Serializer;
using hermes::vm::StringPrimitive;
using hermes::vm::SymbolID;

template <typename T>
uint32_t IdentifierHashTable::lookupString(
    llvm::ArrayRef<T> str,
    uint32_t hash,
    bool mustBeNew) const {
  assert(identifierTable_ && "identifier table pointer is not initialized");

  auto cap = capacity();
  assert(llvm::isPowerOf2_32(cap) && "capacity must be power of 2");
  assert(size_ < cap && "The hash table can never be full");

#ifdef HERMES_SLOW_DEBUG
  assert(hash == hashString(str) && "invalid hash");
#endif
  uint32_t idx = hash & (cap - 1);
  uint32_t base = 1;
  // deletedIndex tracks the index of a deleted entry found in the conflict
  // chain. If we could not find an entry that matches str, we would return
  // the deleted slot for insertion to be able to reuse deleted space.
  OptValue<uint32_t> deletedIndex;
  // The loop will always terminate as long as the hash table is not full.
  while (1) {
    auto &entry = storage_[idx];
    if (entry.isEmpty()) {
      // Found an empty entry, meaning that str does not exist in the table.
      // If deletedIndex is available, return it, otherwise return idx.
      return deletedIndex ? *deletedIndex : idx;
    }
    if (entry.isDeleted()) {
      assert(
          !mustBeNew &&
          "mustBeNew should never be set if there are deleted entries");
      deletedIndex = idx;
    } else if (!mustBeNew) {
      // If mustBeNew is set, we know this string does not exist in the table.
      // There is no need to compare.

      auto &lookupTableEntry =
          identifierTable_->getLookupTableEntry(entry.index);
      if (lookupTableEntry.getHash() == hash) {
        if (lookupTableEntry.isStringPrim()) {
          const StringPrimitive *strPrim = lookupTableEntry.getStringPrim();
          if (strPrim->isASCII()) {
            if (stringRefEquals(str, strPrim->castToASCIIRef())) {
              return idx;
            }
          } else {
            if (stringRefEquals(str, strPrim->castToUTF16Ref())) {
              return idx;
            }
          }
        } else if (lookupTableEntry.isLazyASCII()) {
          // Lazy ASCII string.
          if (stringRefEquals(str, lookupTableEntry.getLazyASCIIRef())) {
            return idx;
          }
        } else {
          // UTF16 string.
          if (stringRefEquals(str, lookupTableEntry.getLazyUTF16Ref())) {
            return idx;
          }
        }
      }
    }
    /// Use quadratic probing to find next probe index in the hash table.
    /// h(k, i) = (h(k) + 1/2 * i + 1/2 * i^2) mod m.
    /// This guarantees the values of h(k,i) for i in [0,m-1] are all distinct.
    idx = (idx + base) & (cap - 1);
    ++base;
  }
}

// Instantiate the templated method so it can be called from other files.

template uint32_t IdentifierHashTable::lookupString(
    llvm::ArrayRef<char> str,
    uint32_t hash,
    bool mustBeNew) const;

template uint32_t IdentifierHashTable::lookupString(
    llvm::ArrayRef<char16_t> str,
    uint32_t hash,
    bool mustBeNew) const;

uint32_t IdentifierHashTable::lookupString(
    const StringPrimitive *str,
    bool mustBeNew) const {
  if (str->isASCII()) {
    return lookupString(str->castToASCIIRef(), mustBeNew);
  } else {
    return lookupString(str->castToUTF16Ref(), mustBeNew);
  }
}

uint32_t IdentifierHashTable::lookupString(
    const StringPrimitive *str,
    uint32_t hash,
    bool mustBeNew) const {
  if (str->isASCII()) {
    return lookupString(str->castToASCIIRef(), hash, mustBeNew);
  } else {
    return lookupString(str->castToUTF16Ref(), hash, mustBeNew);
  }
}

void IdentifierHashTable::insert(uint32_t idx, SymbolID id) {
  assert(!storage_[idx].isValid() && "Cannot insert into a valid entry");
  new (&storage_[idx]) HashTableEntry(id.unsafeGetIndex());
  ++size_;
  ++nonEmptyEntryCount_;

  if (shouldGrow()) {
    growAndRehash(capacity() * 2);
  }
}

void IdentifierHashTable::remove(const StringPrimitive *str) {
  if (str->isASCII()) {
    remove(str->castToASCIIRef());
  } else {
    remove(str->castToUTF16Ref());
  }
}

void IdentifierHashTable::growAndRehash(uint32_t newCapacity) {
  assert(llvm::isPowerOf2_32(newCapacity) && "capacity must be power of 2");
  std::vector<HashTableEntry> tmpTable(newCapacity);
  std::swap(storage_, tmpTable);
  for (auto &entry : tmpTable) {
    if (!entry.isValid()) {
      continue;
    }
    // Pass true as second argument as we know this string is not in the table.
    uint32_t idx = 0;
    auto &lookupTableEntry = identifierTable_->getLookupTableEntry(entry.index);
    uint32_t hash = lookupTableEntry.getHash();
    if (lookupTableEntry.isStringPrim()) {
      idx = lookupString(lookupTableEntry.getStringPrim(), hash, true);
    } else if (lookupTableEntry.isLazyASCII()) {
      idx = lookupString(lookupTableEntry.getLazyASCIIRef(), hash, true);
    } else if (lookupTableEntry.isLazyUTF16()) {
      idx = lookupString(lookupTableEntry.getLazyUTF16Ref(), hash, true);
    }
    storage_[idx] = entry;
  }
  nonEmptyEntryCount_ = size_;
}

void IdentifierHashTable::serialize(Serializer &s) {
  // Serialize uint32_t size_{0};
  s.writeInt<uint32_t>(size_);

  // Serialize uint32_t nonEmptyEntryCount_{0};
  s.writeInt<uint32_t>(nonEmptyEntryCount_);

  // We don't serialize IdentifierTable *identifierTable_{};
  // It is set by constructor, don't need to change.

  // Serialize std::vector<HashTableEntry> storage_{};
  size_t size = storage_.size();
  s.writeInt<uint32_t>(size);
  for (size_t i = 0; i < size; i++) {
    // Serialize each entry.
    s.writeInt<uint32_t>(storage_[i].index);
  }
}

void IdentifierHashTable::deserialize(Deserializer &d) {
  size_ = d.readInt<uint32_t>();
  nonEmptyEntryCount_ = d.readInt<uint32_t>();

  size_t size = d.readInt<uint32_t>();
  storage_.resize(size);
  for (size_t i = 0; i < size; i++) {
    // Deserialize each entry.
    storage_[i].index = d.readInt<uint32_t>();
  }
}
