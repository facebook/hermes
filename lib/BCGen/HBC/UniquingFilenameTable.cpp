/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/UniquingFilenameTable.h"

#include <iterator>

namespace hermes {
namespace hbc {

uint32_t UniquingFilenameTable::addFilename(llvh::StringRef filename) {
  return filenames_.insert(filename);
}

void UniquingFilenameTable::appendFilenamesToStorage() {
  const size_t existingStrings = storage_.count();

  storage_.appendStorage(
      ConsecutiveStringStorage{
          filenames_.begin() + existingStrings,
          filenames_.end(),
          std::false_type{},
          false});

  assert(storage_.count() == filenames_.size() && "must map all strings");
}

} // namespace hbc
} // namespace hermes
