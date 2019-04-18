/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H
#define HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

namespace hermes {
namespace hbc {

/// Class storing a uniqued set of strings and indexes.
class UniquingStringLiteralTable final {
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

 public:
  UniquingStringLiteralTable() = default;

  /// Construct by decoding a ConsecutiveStringStorage \p css.
  explicit UniquingStringLiteralTable(ConsecutiveStringStorage &&css);

  /// Adds a string, \p str, if not already present.  Additionally marks that
  /// string as an identifier if \p isIdentifier is true (defaults to false).
  /// \returns the index identifier of the string (either existing or new).
  inline uint32_t addString(llvm::StringRef str, bool isIdentifier = false);

  /// \return string id of an existing \p str in string table.
  inline uint32_t getExistingStringId(llvm::StringRef str) const;

  /// \return string id of an existing \p str in string table, assuming it is
  /// marked as an identifier.
  inline uint32_t getExistingIdentifierId(llvm::StringRef str) const;

  //// \return a ConsecutiveStringStorage for the strings in the table.
  ///  If \p optimize is set, attempt to pack the strings in the storage to
  ///  reduce their size.
  inline ConsecutiveStringStorage generateStorage(bool optimize = false);

  /// \return whether the table is empty.
  inline bool empty() const;
};

inline uint32_t UniquingStringLiteralTable::addString(
    llvm::StringRef str,
    bool isIdentifier) {
  assert(strings_.size() == isIdentifier_.size());
  auto id = strings_.insert(str);
  if (id == isIdentifier_.size()) {
    isIdentifier_.push_back(isIdentifier);
  } else if (isIdentifier) {
    isIdentifier_[id] = true;
  }

  return id;
}

inline uint32_t UniquingStringLiteralTable::getExistingStringId(
    llvm::StringRef str) const {
  auto iter = strings_.find(str);
  assert(iter != strings_.end() && "str should exist in string table.");
  return std::distance(strings_.begin(), iter);
}

inline uint32_t UniquingStringLiteralTable::getExistingIdentifierId(
    llvm::StringRef str) const {
  auto idx = getExistingStringId(str);
  assert(isIdentifier_[idx] && "str is not an identifier.");
  return idx;
}

inline ConsecutiveStringStorage UniquingStringLiteralTable::generateStorage(
    bool optimize) {
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

inline bool UniquingStringLiteralTable::empty() const {
  return strings_.empty();
}

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H
