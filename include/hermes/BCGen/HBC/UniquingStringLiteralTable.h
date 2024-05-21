/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H
#define HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/HBC/StringKind.h"

namespace hermes {
namespace hbc {

/// A table that maps strings to unique IDs to be used in the bytecode.
/// Can be initialized with a pre-existing ConsecutiveStringStorage.
/// After constructing StringLiteralTable, calling addString will add entries to
/// the mapping, but won't immediately append them to the
/// ConsecutiveStringStorage.
/// When done adding strings, you must call populateStorage to actually write
/// their contents to the storage.
/// After populateStorage is called, getIdentifierHashes and getStringKinds can
/// be used to generate their respective tables for storage in the
/// BytecodeModule.
class StringLiteralTable {
  /// The storage that the mapping was initialised with.
  ConsecutiveStringStorage storage_;

  /// Mapping between strings and IDs.
  StringSetVector strings_;

  // Mapping such that \c isIdentifier_[i] is true if and only if the string at
  // \c strings_[i] should be treated as an identifier.
  std::vector<bool> isIdentifier_;

  /// The number of times a string was added as an identifier.  This information
  /// is only tracked for newly added strings (those not in the storage the
  /// accumulator may have been initialized with) and is keyed by the offset of
  /// the string in the mapping from the first newly added string.  This
  /// information is used to order the resulting storage.
  std::vector<size_t> numIdentifierRefs_;

 public:
  StringLiteralTable() = default;

  /// Take ownership of \p storage and decode it to seed the strings_ mapping.
  /// \p isIdentifier indicates which strings in \p storage are to be treated as
  /// identifiers (i.e. the string at ID \c i in \p storage is an identifier if
  /// and only if \c isIdentifier[i] evaluates to true).
  explicit StringLiteralTable(
      ConsecutiveStringStorage storage,
      std::vector<bool> isIdentifier);

  /// \return the number of strings in the mapping.
  inline size_t count() const {
    return strings_.size();
  }

  /// \return true if and only if no strings have been recorded.
  inline bool empty() const {
    return strings_.size() == 0;
  }

  /// \return string id of an existing \p str in string table.
  inline uint32_t getStringID(llvh::StringRef str) const;

  /// \return string id of an existing \p str in string table, assuming it is
  /// marked as an identifier.
  inline uint32_t getIdentifierID(llvh::StringRef str) const;

  /// Exposes interface to extract parts of underlying ConsecutiveStringStorage
  inline std::vector<StringTableEntry> acquireStringTable();
  inline std::vector<unsigned char> acquireStringStorage();

  /// \return a view of the string table entries in the underlying storage.
  StringTableEntry::StringTableRefTy getStringTableView() const {
    return storage_.getStringTableView();
  }
  /// \return a view of the string table storage.
  StringTableEntry::StringStorageRefTy getStringStorageView() const {
    return storage_.getStringStorageView();
  }

  /// \returns a list of hashes corresponding to the strings marked as
  /// identifiers, in their order in the underlying storage.
  std::vector<uint32_t> getIdentifierHashes() const;

  /// \return a sequence of string kinds represented by a run-length encoding.
  /// The i'th kind in the abstract sequence (i.e. not the i'th entry in the
  /// returned vector, which represents a run of the same kind) is the kind of
  /// the string with ID i in the mapping (and the underlying storage it was
  /// initialised with).
  std::vector<StringKind::Entry> getStringKinds() const;

  /// Add a new string -- \p str -- to the accumulation.  If \p isIdentifier is
  /// true, then the string is marked as potentially being used as an
  /// identifier.
  inline void addString(llvh::StringRef str, bool isIdentifier);

  /// Mode for storing new strings to the storage.
  enum class OptimizeMode {
    /// Reorders the strings before populating the storage.
    /// WARNING: The mapping from ID to String is NOT preserved.
    Reorder,
    /// Reorders the strings before populating the storage, and packs the
    /// storage to reduce the size taken up by the character buffer.
    /// WARNING: The mapping from ID to String is NOT preserved.
    ReorderAndPack,
  };

  /// Populate the storage with all the strings currently in the mapping.
  void populateStorage(OptimizeMode mode);

 private:
  /// Populate the strings table from the storage.
  /// Called after storage_ is updated to write the changes made to the
  /// storage back into the mapping.
  void populateStringsTableFromStorage();

  /// Transform into a table with identifiers at lower indices, attempting to
  /// reuse orignal strings for newly added strings when possible.
  ///
  /// \param optimize If set, attempt to pack the strings to reduce the size
  /// taken up by the character buffer.
  void sortAndRemap(bool optimize = false);
};

inline uint32_t StringLiteralTable::getStringID(llvh::StringRef str) const {
  auto iter = strings_.find(str);
  assert(
      iter != strings_.end() &&
      "The requested string is not in the mapping.  Is the part of the IR that "
      "introduces it being traversed by one of the functions in "
      "TraverseLiteralStrings.h ?");
  return std::distance(strings_.begin(), iter);
}

inline uint32_t StringLiteralTable::getIdentifierID(llvh::StringRef str) const {
  auto idx = getStringID(str);
  assert(
      isIdentifier_[idx] &&
      "The requested string exists in the mapping but was not marked as an "
      "identifier.  When it was added to the mapping during a call to one "
      "of the traversal functions in TraverseLiteralStrings.h, was the usage "
      "of the string as an identifier correctly traversed?");
  return idx;
}

inline std::vector<StringTableEntry> StringLiteralTable::acquireStringTable() {
  return storage_.acquireStringTable();
}

inline std::vector<unsigned char> StringLiteralTable::acquireStringStorage() {
  return storage_.acquireStringStorage();
}

inline void StringLiteralTable::addString(
    llvh::StringRef str,
    bool isIdentifier) {
  assert(strings_.size() == isIdentifier_.size());
  const auto fresh = strings_.size();
  auto id = strings_.insert(str);
  if (id == fresh) {
    isIdentifier_.push_back(false);
    numIdentifierRefs_.push_back(0);
  }

  if (isIdentifier) {
    isIdentifier_[id] = true;
    if (id >= storage_.count()) {
      // We only track the frequency of new strings, so the ID needs to be
      // translated.
      numIdentifierRefs_[id - storage_.count()]++;
    }
  }
}

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_UNIQUINGSTRINGLITERALTABLE_H
