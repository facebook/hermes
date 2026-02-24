/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STRINGSTORAGE_H
#define HERMES_SUPPORT_STRINGSTORAGE_H

#include "hermes/Support/OptValue.h"
#include "hermes/Support/StringTableEntry.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/StringRef.h"

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
llvh::StringRef getUTF8StringFromEntry(
    const StringTableEntry &entry,
    llvh::ArrayRef<unsigned char> storage,
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
  std::vector<unsigned char> storage_{};

 public:
  ConsecutiveStringStorage() = default;
  ConsecutiveStringStorage(ConsecutiveStringStorage &&) = default;
  ConsecutiveStringStorage &operator=(ConsecutiveStringStorage &&) = default;

  /// Construct from a list of unique strings.  Note that this is only
  /// instantiated for a small number of different \p I types.
  /// \param Force8Bit if set to std::true_type, indicates that the input
  ///     is *not* utf-8 encoded and consists of 8-bit bytes. If set to
  ///     std::false_type, the input is utf-8 encoded.
  template <typename I, typename Force8Bit>
  ConsecutiveStringStorage(I begin, I end, Force8Bit, bool optimize);

  /// Construct from a list of unique strings.
  explicit ConsecutiveStringStorage(
      llvh::ArrayRef<llvh::StringRef> strings,
      bool optimize = false)
      : ConsecutiveStringStorage(
            strings.begin(),
            strings.end(),
            std::false_type{},
            optimize) {}

  /// Construct from a table and storage.
  ConsecutiveStringStorage(
      std::vector<StringTableEntry> &&table,
      std::vector<unsigned char> &&storage)
      : strTable_(std::move(table)), storage_(std::move(storage)) {}

  /// \returns a view to the current table.
  StringTableRefTy getStringTableView() const {
    return strTable_;
  }

  MutStringTableRefTy getStringTableView() {
    return strTable_;
  }

  /// \returns the number of strings contained in this storage.
  size_t count() const {
    return strTable_.size();
  }

  /// \returns the string table and the storage.
  /// Consumes the ConsecutiveStringStorage, after which it must not be used.
  std::pair<std::vector<StringTableEntry>, std::vector<unsigned char>>
  acquireStringTableAndStorage() && {
    return {std::move(strTable_), std::move(storage_)};
  };

  /// \returns a view to the current table.
  StringStorageRefTy getStringStorageView() const {
    return storage_;
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
  llvh::StringRef getUTF8StringAtIndex(
      uint32_t idx,
      std::string &utf8ConversionStorage) const;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_SUPPORT_STRINGSTORAGE_H
