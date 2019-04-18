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

  // The ConsecutiveStringStorage containing all written strings.
  ConsecutiveStringStorage storage_;

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

  /// \return whether the table is empty.
  inline bool empty() const;

  /// \return a ConsecutiveStringStorage for the strings in \p table.  If \p
  /// optimize is set, attempt to pack the strings in the storage to reduce
  /// their size.
  static ConsecutiveStringStorage toStorage(
      UniquingStringLiteralTable &table,
      bool optimize = false);
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

inline bool UniquingStringLiteralTable::empty() const {
  return strings_.empty();
}

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H
