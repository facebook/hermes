/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/BCGen/HBC/PredefinedStringIDs.h"

#include <cassert>

namespace hermes {
namespace hbc {

namespace {

/// Works out the String Kind for the string \p str depending on whether it
/// \p isIdentifier or not.
StringKind::Kind kind(llvm::StringRef str, bool isIdentifier) {
  if (isIdentifier && getPredefinedStringID(str)) {
    return StringKind::Predefined;
  } else if (isIdentifier) {
    return StringKind::Identifier;
  } else {
    return StringKind::String;
  }
}

} // namespace

StringLiteralIDMapping::StringLiteralIDMapping(
    ConsecutiveStringStorage storage,
    std::vector<bool> isIdentifier)
    : storage_(std::move(storage)), isIdentifier_(std::move(isIdentifier)) {
  // Initialize our tables by decoding our storage's string table.
  std::string utf8Storage;
  uint32_t count = storage_.count();
  assert(isIdentifier_.size() == count);
  for (uint32_t i = 0; i < count; i++) {
    uint32_t j = strings_.insert(storage_.getStringAtIndex(i, utf8Storage));
    assert(i == j && "Duplicate string in storage.");
    (void)j;

    assert(isIdentifier_[i] == storage_.isIdentifierAtIndex(i));
  }
}

std::vector<uint32_t> StringLiteralTable::getIdentifierTranslations() const {
  std::vector<uint32_t> result;
  assert(strings_.size() == isIdentifier_.size());
  for (size_t i = 0; i < strings_.size(); ++i) {
    if (!isIdentifier_[i]) {
      continue;
    }

    if (auto sym = getPredefinedStringID(strings_[i])) {
      result.push_back(sym->unsafeGetRaw());
    } else {
      result.push_back(storage_.getEntryHash(i));
    }
  }

  return result;
}

std::vector<StringKind::Entry> StringLiteralTable::getStringKinds() const {
  StringKind::Accumulator acc;

  assert(strings_.size() == isIdentifier_.size());
  for (size_t i = 0; i < strings_.size(); ++i) {
    acc.push_back(kind(strings_[i], isIdentifier_[i]));
  }

  return std::move(acc).entries();
}

/* static */ StringLiteralTable UniquingStringLiteralAccumulator::toTable(
    UniquingStringLiteralAccumulator accum,
    bool optimize) {
  auto &storage = accum.storage_;
  auto &strings = accum.strings_;
  auto &isIdentifier = accum.isIdentifier_;
  auto &numIdentifierRefs = accum.numIdentifierRefs_;

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

  /// Associates a StringTableEntry with its original index in the table.
  struct IndexedEntry {
    size_t origIndex;
    StringKind::Kind kind;
    StringTableEntry entry;

    IndexedEntry(
        size_t origIndex,
        StringKind::Kind kind,
        StringTableEntry entry)
        : origIndex(origIndex), kind(kind), entry(entry) {
      assert(
          (entry.isIdentifier() || kind == StringKind::String) &&
          "Identifiers cannot have 'String' Kind and vice-versa.");
    }

   private:
    // Key for performing comparisons with.  Ordering on this key is used to
    // optimise string index layout to compress better.
    using Key = std::tuple<StringKind::Kind, uint32_t, uint32_t>;
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
    indexedEntries.emplace_back(
        i, kind(strings[i], isIdentifier[i]), tableView[i]);
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
  const auto entriesFrom = [&indexedEntries, existingStrings](size_t ix) {
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
    isIdentifier[i] = indexedEntries[i].kind != StringKind::String;
  }

  return StringLiteralTable{std::move(storage), std::move(isIdentifier)};
}

} // namespace hbc
} // namespace hermes
