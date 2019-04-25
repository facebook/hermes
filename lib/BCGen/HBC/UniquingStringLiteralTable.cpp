/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"

#include <cassert>

namespace hermes {
namespace hbc {

StringLiteralIDMapping::StringLiteralIDMapping(
    const ConsecutiveStringStorage &css) {
  // Initialize our tables by decoding our storage's string table.
  std::string utf8Storage;
  uint32_t count = css.count();
  isIdentifier_.reserve(count);
  for (uint32_t i = 0; i < count; i++) {
    uint32_t j = strings_.insert(css.getStringAtIndex(i, utf8Storage));
    assert(i == j && "Duplicate string in storage.");
    (void)j;

    isIdentifier_.push_back(css.isIdentifierAtIndex(i));
  }
}

/* static */ ConsecutiveStringStorage
UniquingStringLiteralAccumulator::toStorage(
    UniquingStringLiteralAccumulator table,
    bool optimize) {
  auto &storage = table.storage_;
  auto &strings = table.strings_;
  auto &isIdentifier = table.isIdentifier_;
  auto &numIdentifierRefs = table.numIdentifierRefs_;

  const size_t existingStrings = storage.count();
  assert(
      existingStrings <= strings.size() &&
      "Cannot have more written strings than strings");

  if (existingStrings < strings.size()) {
    auto unwritten = strings.begin() + existingStrings;
    storage.appendStorage({unwritten, strings.end(), optimize});
  }

  auto tableView = storage.getStringTableView();
  for (size_t i = 0; i < strings.size(); ++i) {
    if (isIdentifier[i]) {
      tableView[i].markAsIdentifier();
    }
  }

  enum class StringKind {
    String = 0,
    Identifier,
  };

  /// Associates a StringTableEntry with its original index in the table.
  struct IndexedEntry {
    size_t origIndex;
    StringKind kind;
    StringTableEntry entry;

    IndexedEntry(size_t origIndex, StringKind kind, StringTableEntry entry)
        : origIndex(origIndex), kind(kind), entry(entry) {
      assert(
          (entry.isIdentifier() || kind == StringKind::String) &&
          "Identifiers cannot have 'String' Kind and vice-versa.");
    }

   private:
    // Key for performing comparisons with.  Ordering on this key is used to
    // optimise string index layout to compress better.
    using Key = std::tuple<StringKind, uint32_t, uint32_t>;
    inline Key key() const {
      return std::make_tuple(kind, entry.getOffset(), entry.getLength());
    }

   public:
    inline bool operator<(const IndexedEntry &that) const {
      return this->key() < that.key();
    }
  };

  std::vector<IndexedEntry> indexedEntries;
  indexedEntries.reserve(strings.size());

  // Associate string index entries with their original indices in the table.
  for (size_t i = 0; i < strings.size(); ++i) {
    StringKind kind =
        isIdentifier[i] ? StringKind::Identifier : StringKind::String;

    indexedEntries.emplace_back(i, kind, tableView[i]);
  }

  // Sort the new strings by frequency of identifier references.
  std::sort(
      indexedEntries.begin() + existingStrings,
      indexedEntries.end(),
      [&numIdentifierRefs, existingStrings](
          const IndexedEntry &a, const IndexedEntry &b) {
        assert(a.origIndex >= existingStrings && "Sorting an old string");
        assert(b.origIndex >= existingStrings && "Sorting an old string");

        auto ai = a.origIndex - existingStrings;
        auto bi = b.origIndex - existingStrings;

        return numIdentifierRefs[ai] > numIdentifierRefs[bi];
      });

  // Translates from an index in the string storage to an iterator into
  // indexedEntries.  Clamps indices that are too large or too small.
  auto entriesFrom = [&indexedEntries, existingStrings](size_t ix) {
    // Bound from below -- we only wish to sort new entries.
    ix = std::max(existingStrings, ix);

    // Bound from above by the number of indexed entries.
    ix = std::min(ix, indexedEntries.size());

    return indexedEntries.begin() + ix;
  };

  std::sort(entriesFrom(0), entriesFrom(UINT8_MAX));
  std::sort(entriesFrom(UINT8_MAX), entriesFrom(UINT16_MAX));
  std::sort(entriesFrom(UINT16_MAX), entriesFrom(SIZE_MAX));

  // Write the re-ordered entries back into the table.
  for (size_t i = existingStrings; i < strings.size(); ++i) {
    tableView[i] = indexedEntries[i].entry;
  }

  return std::move(storage);
}

} // namespace hbc
} // namespace hermes
