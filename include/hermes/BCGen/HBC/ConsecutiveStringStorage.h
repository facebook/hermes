/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_STRINGSTORAGE_H
#define HERMES_SUPPORT_STRINGSTORAGE_H

#include "hermes/Support/OptValue.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/Support/StringTableEntry.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringRef.h"

#include <cstdint>
#include <deque>
#include <iterator>
#include <string>
#include <vector>

namespace hermes {
namespace hbc {

/// \return the string corresponding to the entry \p entry, out of character
/// storage \p storage, converting UTF16 strings to UTF8 using
/// \p utf8ConversionStorage if necessary.
/// \return a StringRef of the string.
llvm::StringRef getStringFromEntry(
    const StringTableEntry &entry,
    llvm::ArrayRef<char> storage,
    std::string &utf8ConversionStorage);

/// A data structure for storing a serialized list of strings.
class ConsecutiveStringStorage {
  ConsecutiveStringStorage &operator=(const ConsecutiveStringStorage &_) =
      delete;

  ConsecutiveStringStorage(const ConsecutiveStringStorage &) = delete;

 public:
  using StringTableRefTy = StringTableEntry::StringTableRefTy;

  using StringStorageRefTy = StringTableEntry::StringStorageRefTy;

 private:
  /// Offset and length of each string in the consecutive storage.
  std::vector<StringTableEntry> strTable_{};

  /// A consecutive storage of char sequences.
  std::vector<char> storage_{};

  /// Whether the string table is still valid to use.
  bool isTableValid_{true};

  /// Whether the string storage is still valid to use.
  bool isStorageValid_{true};

  inline void ensureTableValid() const {
    assert(isTableValid_ && "String Table no longer valid");
  }
  inline void ensureStorageValid() const {
    assert(isStorageValid_ && "String Storage no longer valid");
  }

  /// \return the hash of a given entry.
  uint32_t getEntryHash(const StringTableEntry &entry) const;

 public:
  // Optimization options for the constructor.
  enum Optimizations {
    // Attempt to pack the strings in a way that maximizes overlap, resulting in
    // a smaller storage buffer.
    OptimizePacking = 1 << 0,

    // Reorder the table index to reflect the character buffer order, improving
    // compressibility.
    OptimizeOrdering = 1 << 1,
  };
  using OptimizationFlags = uint32_t;

  ConsecutiveStringStorage() = default;
  ConsecutiveStringStorage(ConsecutiveStringStorage &&) = default;
  ConsecutiveStringStorage &operator=(ConsecutiveStringStorage &&) = default;

  /// Construct from a list of unique strings.  Note that this is only
  /// instantiated for a small number of different \p I types.
  template <typename I>
  ConsecutiveStringStorage(I begin, I end, OptimizationFlags flags = 0);

  /// Construct from a list of unique strings.
  explicit ConsecutiveStringStorage(
      llvm::ArrayRef<llvm::StringRef> strings,
      OptimizationFlags flags = 0)
      : ConsecutiveStringStorage(strings.begin(), strings.end(), flags) {}

  /// Construct from a table and storage.
  ConsecutiveStringStorage(
      std::vector<StringTableEntry> &&table,
      std::vector<char> &&storage)
      : strTable_(std::move(table)), storage_(std::move(storage)) {}

  /// \returns a view to the current storage.
  StringStorageRefTy getStorageView() const {
    ensureStorageValid();
    return storage_;
  }

  /// \returns a view to the current table.
  StringTableRefTy getStringTableView() const {
    ensureTableValid();
    return strTable_;
  }

  /// \returns a reference to the string table. Notice that whoever receives
  /// the table may temper, swap or destroy the content. Hence after this
  /// call, the string table is no longer valid to use.
  std::vector<StringTableEntry> acquireStringTable() {
    ensureTableValid();
    isTableValid_ = false;
    return std::move(strTable_);
  };

  /// \returns a reference to the string storage. Notice that whoever receives
  /// the table may temper, swap or destroy the content. Hence after this
  /// call, the string table is no longer valid to use.
  std::vector<char> acquireStringStorage() {
    ensureStorageValid();
    isStorageValid_ = false;
    return std::move(storage_);
  }

  /// Append a storage \p rhs.
  void appendStorage(ConsecutiveStringStorage &&rhs);

  /// \returns a list of translations corresponding to the entries marked as
  /// identifiers, in-order.
  std::vector<uint32_t> getIdentifierTranslations() const;

  /// A helper function to return a string at a given \p idx. This converts
  /// UTF16 strings to UTF8 using \p storage if necessary.
  /// \return a StringRef of the string, which may or may not reference
  /// \p storage.
  llvm::StringRef getStringAtIndex(
      uint32_t idx,
      std::string &utf8ConversionStorage) const;

  /// \return true if and only if the entry at index \p idx is marked as an
  /// identifier.
  bool isIdentifierAtIndex(uint32_t idx) const {
    ensureTableValid();
    return strTable_.at(idx).isIdentifier();
  }

  /// Mark the entry at a given index \p idx as an identifier.
  void markEntryAsIdentifier(uint32_t idx) {
    ensureTableValid();
    strTable_.at(idx).markAsIdentifier();
  }
};

/// Class storing a uniqued set of strings and indexes.
class UniquingStringTable final {
  /// Mapping between strings and IDs.
  StringSetVector strings_;

  // Mapping such that \c isIdentifier_[i] is true if and only if the string at
  // \c strings_[i] should be treated as an identifier.
  std::vector<bool> isIdentifier_;

  // Count of strings written to our Storage.
  // These are always at the beginning of strings_;
  uint32_t writtenStrings_{0};

  // The ConsecutiveStringStorage containing all written strings.
  ConsecutiveStringStorage storage_;

  // Write any strings that have not yet been written to our string storage.
  // If \p optimize is set, optimize the layout of the storage.
  void flushUnwrittenStringsToStorage(bool optimize);

  // Disable copying.
  UniquingStringTable(const UniquingStringTable &) = delete;
  void operator=(const UniquingStringTable &) = delete;

 public:
  UniquingStringTable() = default;
  UniquingStringTable(UniquingStringTable &&) = default;
  UniquingStringTable &operator=(UniquingStringTable &&) = default;

  /// Construct by decoding a ConsecutiveStringStorage \p css.
  explicit UniquingStringTable(ConsecutiveStringStorage &&css);

  /// Adds a string, \p str, if not already present.  Additionally marks that
  /// string as an identifier if \p isIdentifier is true (defaults to false).
  /// \returns the index identifier of the string (either existing or new).
  uint32_t addString(llvm::StringRef str, bool isIdentifier = false) {
    assert(strings_.size() == isIdentifier_.size());
    auto id = strings_.insert(str);
    if (id == isIdentifier_.size()) {
      isIdentifier_.push_back(isIdentifier);
    } else if (isIdentifier) {
      isIdentifier_[id] = true;
    }

    return id;
  }

  /// \return string id of an existing \p str in string table.
  uint32_t getExistingStringId(llvm::StringRef str) const {
    auto iter = strings_.find(str);
    assert(iter != strings_.end() && "str should exist in string table.");
    return std::distance(strings_.begin(), iter);
  }

  /// \return string id of an existing \p str in string table, assuming it is
  /// marked as an identifier.
  uint32_t getExistingIdentifierId(llvm::StringRef str) const {
    auto idx = getExistingStringId(str);
    assert(isIdentifier_[idx] && "str is not an identifier.");
    return idx;
  }

  //// \return a ConsecutiveStringStorage for the strings in the table.
  ///  If \p optimize is set, attempt to pack the strings in the storage to
  ///  reduce their size.
  ConsecutiveStringStorage generateStorage(bool optimize = false) {
    // Flush unwritten strings to our existing storage and then transfer it
    // to the caller. Mark our written strings as zero since our storage will
    // now be empty.
    flushUnwrittenStringsToStorage(optimize);
    ConsecutiveStringStorage result = std::move(storage_);

    for (uint32_t idx = 0; idx < isIdentifier_.size(); ++idx) {
      if (isIdentifier_[idx]) {
        result.markEntryAsIdentifier(idx);
      }
    }

    storage_ = ConsecutiveStringStorage{};
    writtenStrings_ = 0;
    return result;
  }

  /// \return whether the table is empty.
  bool empty() const {
    return strings_.empty();
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_SUPPORT_STRINGSTORAGE_H
