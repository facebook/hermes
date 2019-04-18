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
  uint32_t count = storage_.getStringTableView().size();
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
  writtenStrings_ = count;
}

void UniquingStringLiteralTable::flushUnwrittenStringsToStorage(bool optimize) {
  assert(
      writtenStrings_ <= strings_.size() &&
      "Cannot have more written strings than strings");
  if (writtenStrings_ == strings_.size())
    return;

  uint32_t flags = optimize ? ConsecutiveStringStorage::OptimizePacking : 0;
  auto unwritten = strings_.begin() + writtenStrings_;
  storage_.appendStorage(
      ConsecutiveStringStorage{unwritten, strings_.end(), flags});
  writtenStrings_ = strings_.size();
}

} // namespace hbc
} // namespace hermes
