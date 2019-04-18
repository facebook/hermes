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

UniquingStringLiteralTable::UniquingStringLiteralTable(
    ConsecutiveStringStorage &&css)
    : storage_(std::move(css)) {
  // Note that we acquired the storage and made it our own.
  // Initialize our tables by decoding our storage's string table.
  std::string utf8Storage;
  uint32_t count = storage_.count();
  for (uint32_t i = 0; i < count; i++) {
    uint32_t added = addString(
        storage_.getStringAtIndex(i, utf8Storage),
        storage_.isIdentifierAtIndex(i));
    (void)added;
    assert(
        added == i &&
        "UniquingStringLiteralTable index should match acquired string storage index");
  }
  // Since we initialized from storage, all of our strings have already been
  // written to this storage.
  assert(strings_.size() == count && "Should have 'count' strings");
}

/* static */ ConsecutiveStringStorage UniquingStringLiteralTable::toStorage(
    UniquingStringLiteralTable &table,
    bool optimize) {
  auto &storage = table.storage_;
  auto &strings = table.strings_;
  auto &isIdentifier = table.isIdentifier_;

  assert(
      storage.count() <= strings.size() &&
      "Cannot have more written strings than strings");

  if (storage.count() < strings.size()) {
    uint32_t flags = optimize ? ConsecutiveStringStorage::OptimizePacking : 0;
    auto unwritten = strings.begin() + storage.count();
    storage.appendStorage({unwritten, strings.end(), flags});
  }

  for (uint32_t idx = 0; idx < isIdentifier.size(); ++idx) {
    if (isIdentifier[idx]) {
      storage.markEntryAsIdentifier(idx);
    }
  }

  return std::move(storage);
}

} // namespace hbc
} // namespace hermes
