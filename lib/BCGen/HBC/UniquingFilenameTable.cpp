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

/* static */ ConsecutiveStringStorage UniquingFilenameTable::toStorage(
    UniquingFilenameTable table) {
  auto &filenames = table.filenames_;
  return ConsecutiveStringStorage{filenames.begin(), filenames.end()};
}

} // namespace hbc
} // namespace hermes
