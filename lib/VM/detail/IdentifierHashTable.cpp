/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/detail/IdentifierHashTable.h"

#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm::detail;
// In GCC/CLANG, method definitions can refer to ancestor namespaces of
// the namespace that the class is declared in without namespace qualifiers.
// This is not allowed in MSVC.
using hermes::vm::StringPrimitive;
using hermes::vm::SymbolID;

template <typename T>
uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<T> str,
    uint32_t hash,
    bool mustBeNew) const {
  assert(identifierTable_ && "identifier table pointer is not initialized");

  auto cap = capacity();
  assert(llvh::isPowerOf2_32(cap) && "capacity must be power of 2");
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
    if (table_.isEmpty(idx)) {
      // Found an empty entry, meaning that str does not exist in the table.
      // If deletedIndex is available, return it, otherwise return idx.
      return deletedIndex ? *deletedIndex : idx;
    } else if (table_.isDeleted(idx)) {
      assert(
          !mustBeNew &&
          "mustBeNew should never be set if there are deleted entries");
      deletedIndex = idx;
    } else if (!mustBeNew) {
      // If mustBeNew is set, we know this string does not exist in the table.
      // There is no need to compare.

      auto &lookupTableEntry =
          identifierTable_->getLookupTableEntry(table_.get(idx));
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
    llvh::ArrayRef<char> str,
    uint32_t hash,
    bool mustBeNew) const;

template uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<char16_t> str,
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
  table_.set(idx, id.unsafeGetIndex());
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
  // Guard against potential overflow in the calculation of new capacity.
  if (LLVM_UNLIKELY(newCapacity <= capacity())) {
    hermes_fatal("too many identifiers created");
  }
  assert(llvh::isPowerOf2_32(newCapacity) && "capacity must be power of 2");
  CompactTable tmpTable(newCapacity, table_.getCurrentScale());
  tmpTable.swap(table_);
  for (uint32_t oldIdx = 0; oldIdx < tmpTable.size(); ++oldIdx) {
    if (!tmpTable.isValid(oldIdx)) {
      continue;
    }
    // Pass true as second argument as we know this string is not in the table.
    uint32_t idx = 0;
    uint32_t oldVal = tmpTable.get(oldIdx);
    auto &lookupTableEntry = identifierTable_->getLookupTableEntry(oldVal);
    uint32_t hash = lookupTableEntry.getHash();
    if (lookupTableEntry.isStringPrim()) {
      idx = lookupString(lookupTableEntry.getStringPrim(), hash, true);
    } else if (lookupTableEntry.isLazyASCII()) {
      idx = lookupString(lookupTableEntry.getLazyASCIIRef(), hash, true);
    } else if (lookupTableEntry.isLazyUTF16()) {
      idx = lookupString(lookupTableEntry.getLazyUTF16Ref(), hash, true);
    }
    table_.set(idx, oldVal);
  }
  nonEmptyEntryCount_ = size_;
}
