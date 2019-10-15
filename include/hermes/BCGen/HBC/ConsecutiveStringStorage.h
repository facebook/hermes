/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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
  using MutStringTableRefTy = StringTableEntry::MutStringTableRefTy;
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

 public:
  ConsecutiveStringStorage() = default;
  ConsecutiveStringStorage(ConsecutiveStringStorage &&) = default;
  ConsecutiveStringStorage &operator=(ConsecutiveStringStorage &&) = default;

  /// Construct from a list of unique strings.  Note that this is only
  /// instantiated for a small number of different \p I types.
  template <typename I>
  ConsecutiveStringStorage(I begin, I end, bool optimize = false);

  /// Construct from a list of unique strings.
  explicit ConsecutiveStringStorage(
      llvm::ArrayRef<llvm::StringRef> strings,
      bool optimize = false)
      : ConsecutiveStringStorage(strings.begin(), strings.end(), optimize) {}

  /// Construct from a table and storage.
  ConsecutiveStringStorage(
      std::vector<StringTableEntry> &&table,
      std::vector<char> &&storage)
      : strTable_(std::move(table)), storage_(std::move(storage)) {}

  /// \returns a view to the current table.
  StringTableRefTy getStringTableView() const {
    ensureTableValid();
    return strTable_;
  }

  MutStringTableRefTy getStringTableView() {
    ensureTableValid();
    return strTable_;
  }

  /// \returns the number of strings contained in this storage.
  size_t count() const {
    ensureTableValid();
    return strTable_.size();
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

  /// \return the hash of the string represented by the \p i'th entry, as it
  /// would be represented at runtime.
  uint32_t getEntryHash(size_t i) const;

  /// Append a storage \p rhs.
  void appendStorage(ConsecutiveStringStorage &&rhs);

  /// A helper function to return a string at a given \p idx. This converts
  /// UTF16 strings to UTF8 using \p storage if necessary.
  /// \return a StringRef of the string, which may or may not reference
  /// \p storage.
  llvm::StringRef getStringAtIndex(
      uint32_t idx,
      std::string &utf8ConversionStorage) const;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_SUPPORT_STRINGSTORAGE_H
