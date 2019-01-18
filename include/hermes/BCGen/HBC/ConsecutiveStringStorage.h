/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_STRINGSTORAGE_H
#define HERMES_SUPPORT_STRINGSTORAGE_H

#include "hermes/Support/StringTableEntry.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringRef.h"

#include <cstdint>
#include <deque>
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

  /// Construct from a list of strings, which must be unique.
  explicit ConsecutiveStringStorage(
      llvm::ArrayRef<llvm::StringRef> strings,
      OptimizationFlags flags = 0);

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

  /// \returns a list of hashes corresponding to the entries marked as
  /// identifiers, in-order.
  std::vector<uint32_t> getIdentifierHashes() const;

  /// A helper function to return a string at a given \p idx. This converts
  /// UTF16 strings to UTF8 using \p storage if necessary.
  /// \return a StringRef of the string, which may or may not reference
  /// \p storage.
  llvm::StringRef getStringAtIndex(
      uint32_t idx,
      std::string &utf8ConversionStorage) const;

  /// Mark the entry at a given index \p idx as an identifier.
  void markEntryAsIdentifier(uint32_t idx) {
    ensureTableValid();
    strTable_.at(idx).markAsIdentifier();
  }
};

/// Class storing a uniqued set of strings and indexes.
/// This is similar to llvm's SetVector, but more efficient because it does not
/// duplicate the strings in the index.
class UniquingStringTable final {
  // The object that owns storage for the strings.
  // deque has the important property that appending does not move strings.
  std::deque<std::string> stringsStorage_;

  // A map from strings to their index in the stringsByIndex vector.
  llvm::DenseMap<llvm::StringRef, uint32_t> stringsToIndex_;

  // Our strings ordered by their index.
  std::vector<llvm::StringRef> stringsByIndex_;

  // Count of strings written to our Storage.
  // These are always at the beginning of stringsByIndex_.
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

  /// Adds a string, if not already present.
  /// \returns the index identifier of the string (either existing or new).
  uint32_t addString(llvm::StringRef str) {
    assert(stringsToIndex_.size() == stringsByIndex_.size());
    auto iter = stringsToIndex_.find(str);
    if (iter != stringsToIndex_.end()) {
      return iter->second;
    }

    // Copy the string into the deque, and insert it into the map.
    // Our string is owned by the deque.
    stringsStorage_.emplace_back(str.begin(), str.end());
    llvm::StringRef copiedString(stringsStorage_.back());
    size_t newIndex = stringsToIndex_.size();
    auto inserted = stringsToIndex_.insert({copiedString, newIndex});
    assert(inserted.second && "Unexpectedly failed to insert");
    (void)inserted;
    stringsByIndex_.push_back(copiedString);
    assert(stringsByIndex_.at(newIndex) == str && "String at unexpected index");
    return newIndex;
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
    storage_ = ConsecutiveStringStorage{};
    writtenStrings_ = 0;
    return result;
  }

  /// \return whether the table is empty.
  bool empty() const {
    return stringsByIndex_.empty();
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_SUPPORT_STRINGSTORAGE_H
