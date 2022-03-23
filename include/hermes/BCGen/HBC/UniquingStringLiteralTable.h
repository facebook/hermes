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

/// Implementation shared between the UniquingStringLiteralAccumulator, which
/// gathers strings into a storage, and the StringLiteralTable, which exposes
/// the mapping from string to numeric ID.
struct StringLiteralIDMapping {
  /// \return the number of strings in the mapping.
  inline size_t count() const;

  /// \return true if and only if no strings have been recorded.
  inline bool empty() const;

  StringLiteralIDMapping() = default;

  /// Take ownership of \p storage and decode it to seed the strings_ mapping.
  /// \p isIdentifier indicates which strings in \p storage are to be treated as
  /// identifiers (i.e. the string at ID \c i in \p storage is an identifier if
  /// and only if \c isIdentifier[i] evaluates to true).
  explicit StringLiteralIDMapping(
      ConsecutiveStringStorage storage,
      std::vector<bool> isIdentifier);

 protected:
  /// The storage that the mapping was initialised with.
  ConsecutiveStringStorage storage_;

  /// Mapping between strings and IDs.
  StringSetVector strings_;

  // Mapping such that \c isIdentifier_[i] is true if and only if the string at
  // \c strings_[i] should be treated as an identifier.
  std::vector<bool> isIdentifier_;
};

/// Exposes the mapping from strings to their IDs in a ConsecutiveStringStorage.
/// This class does not own the storage it is mapping, but also does not require
/// the storage to outlive it.
struct StringLiteralTable final : public StringLiteralIDMapping {
  StringLiteralTable() = default;
  using StringLiteralIDMapping::StringLiteralIDMapping;

  /// \return string id of an existing \p str in string table.
  inline uint32_t getStringID(llvh::StringRef str) const;

  /// \return string id of an existing \p str in string table, assuming it is
  /// marked as an identifier.
  inline uint32_t getIdentifierID(llvh::StringRef str) const;

  /// Exposes interface to extract parts of underlying ConsecutiveStringStorage
  inline std::vector<StringTableEntry> acquireStringTable();
  inline std::vector<unsigned char> acquireStringStorage();

  /// \returns a list of hashes corresponding to the strings marked as
  /// identifiers, in their order in the underlying storage.
  std::vector<uint32_t> getIdentifierHashes() const;

  /// \return a sequence of string kinds represented by a run-length encoding.
  /// The i'th kind in the abstract sequence (i.e. not the i'th entry in the
  /// returned vector, which represents a run of the same kind) is the kind of
  /// the string with ID i in the mapping (and the underlying storage it was
  /// initialised with).
  std::vector<StringKind::Entry> getStringKinds() const;
};

/// Gathers strings into a storage. Because the indices of strings in the
/// resulting storage may change between when the string is added and when the
/// storage is created, this class does not return the ID of the string when it
/// is added.
class UniquingStringLiteralAccumulator final : public StringLiteralIDMapping {
  /// The number of times a string was added as an identifier.  This information
  /// is only tracked for newly added strings (those not in the storage the
  /// accumulator may have been initialized with) and is keyed by the offset of
  /// the string in the mapping from the first newly added string.  This
  /// information is used to order the resulting storage.
  std::vector<size_t> numIdentifierRefs_;

 public:
  UniquingStringLiteralAccumulator() = default;
  using StringLiteralIDMapping::StringLiteralIDMapping;

  /// Add a new string -- \p str -- to the accumulation.  If \p isIdentifier is
  /// true, then the string is marked as potentially being used as an
  /// identifier.
  inline void addString(llvh::StringRef str, bool isIdentifier);

  /// \return a StringLiteralTable with the same strings as the accumulator
  /// \p strings.  If \p optimize is set, attempt to pack the strings to
  /// reduce the size taken up by the character buffer.  The mapping from ID
  /// to String is not preserved between \p strings and the resulting table.
  static StringLiteralTable toTable(
      UniquingStringLiteralAccumulator strings,
      bool optimize = false);
};

inline size_t StringLiteralIDMapping::count() const {
  return strings_.size();
}

inline bool StringLiteralIDMapping::empty() const {
  return strings_.size() == 0;
}

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

inline void UniquingStringLiteralAccumulator::addString(
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
