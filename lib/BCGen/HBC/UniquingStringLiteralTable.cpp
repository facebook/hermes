/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"

#include <cassert>

namespace hermes {
namespace hbc {

namespace {

/// Works out the String Kind for the string \p str depending on whether it
/// \p isIdentifier or not.
StringKind::Kind kind(llvh::StringRef str, bool isIdentifier) {
  if (isIdentifier) {
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
  }
}

std::vector<uint32_t> StringLiteralTable::getIdentifierHashes() const {
  std::vector<uint32_t> result;
  assert(strings_.size() == isIdentifier_.size());
  for (size_t i = 0; i < strings_.size(); ++i) {
    if (!isIdentifier_[i]) {
      continue;
    }
    result.push_back(storage_.getEntryHash(i));
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
  const size_t allStrings = strings.size();
  const size_t newStrings = allStrings - existingStrings;
  assert(
      existingStrings <= allStrings &&
      "Cannot have more written strings than strings");

  /// Associate a string with its original index in the table.
  struct Index {
    size_t origIndex;
    llvh::StringRef str;
    StringKind::Kind kind;

    Index(size_t origIndex, llvh::StringRef str, StringKind::Kind kind)
        : origIndex(origIndex), str(str), kind(kind) {}

    using Key = std::tuple<StringKind::Kind, llvh::StringRef>;

    inline Key key() const {
      return std::make_tuple(kind, str);
    }

    inline bool operator<(const Index &that) const {
      return this->key() < that.key();
    }
  };

  /// Associates a StringTableEntry with its String Kind.
  struct KindedEntry {
    StringKind::Kind kind;
    StringTableEntry entry;

    KindedEntry(StringKind::Kind kind, StringTableEntry entry)
        : kind(kind), entry(entry) {}

   private:
    // Key for performing comparisons with.  Ordering on this key is used to
    // optimise string index layout to compress better.
    using Key = std::tuple<StringKind::Kind, uint32_t, uint32_t>;
    inline Key key() const {
      return std::make_tuple(kind, entry.getOffset(), entry.getLength());
    }

   public:
    inline bool operator<(const KindedEntry &that) const {
      return this->key() < that.key();
    }
  };

  std::vector<Index> indices;
  indices.reserve(newStrings);

  for (size_t i = existingStrings; i < allStrings; ++i) {
    indices.emplace_back(i, strings[i], kind(strings[i], isIdentifier[i]));
  }

  // Sort indices of new strings by frequency of identifier references.
  std::stable_sort(
      indices.begin(),
      indices.end(),
      [&numIdentifierRefs, existingStrings](const Index &a, const Index &b) {
        assert(a.origIndex >= existingStrings && "Sorting an old string");
        assert(b.origIndex >= existingStrings && "Sorting an old string");

        auto ai = a.origIndex - existingStrings;
        auto bi = b.origIndex - existingStrings;

        return numIdentifierRefs[ai] > numIdentifierRefs[bi];
      });

  // Take an index into all the strings, and map it to an index in to a data
  // structure containing only the new strings.
  const auto remap = [allStrings, existingStrings](size_t ix) {
    // Skip the existing strings.
    ix = std::max(ix, existingStrings);

    // Avoid a buffer overrun;
    ix = std::min(ix, allStrings);

    // Translate by the number of strings we skipped over.
    ix -= existingStrings;

    return ix;
  };

  // Sort indices within each frequency class by kind, and alphabetically.
  const auto indicesFrom = [&remap, &indices](size_t ix) {
    return indices.begin() + remap(ix);
  };

  std::sort(indicesFrom(0), indicesFrom(UINT8_MAX));
  std::sort(indicesFrom(UINT8_MAX), indicesFrom(UINT16_MAX));
  std::sort(indicesFrom(UINT16_MAX), indicesFrom(SIZE_MAX));

  { // Add the new strings to the storage.
    std::vector<llvh::StringRef> refs;
    refs.reserve(newStrings);
    for (auto &i : indices) {
      refs.emplace_back(i.str);
    }
    ConsecutiveStringStorage newStrings(refs, optimize);
    storage.appendStorage(std::move(newStrings));
  }

  // Associate the new string table index entries with their string kind.
  auto tableView = storage.getStringTableView();
  std::vector<KindedEntry> kindedEntries;
  kindedEntries.reserve(newStrings);

  for (size_t i = 0, j = existingStrings; i < newStrings; ++i, ++j) {
    kindedEntries.emplace_back(indices[i].kind, tableView[j]);
  }

  // Sort index entries within each frequency and kind bucket by their offset
  // in the storage and length.
  const auto entriesFrom = [&remap, &kindedEntries](size_t ix) {
    return kindedEntries.begin() + remap(ix);
  };

  std::sort(entriesFrom(0), entriesFrom(UINT8_MAX));
  std::sort(entriesFrom(UINT8_MAX), entriesFrom(UINT16_MAX));
  std::sort(entriesFrom(UINT16_MAX), entriesFrom(SIZE_MAX));

  // Write the re-ordered entries back into the table.
  for (size_t i = 0, j = existingStrings; i < newStrings; ++i, ++j) {
    tableView[j] = kindedEntries[i].entry;
    isIdentifier[j] = kindedEntries[i].kind != StringKind::String;
  }

  return StringLiteralTable{std::move(storage), std::move(isIdentifier)};
}

} // namespace hbc
} // namespace hermes
