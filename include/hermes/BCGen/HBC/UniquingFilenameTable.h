/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H
#define HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H

#include "hermes/ADT/StringSetVector.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

#include "llvh/ADT/StringRef.h"

namespace hermes {
namespace hbc {

/// Gathers and uniques filenames during bytecode generation.
class UniquingFilenameTable {
  /// The storage that the mapping was initialised with.
  ConsecutiveStringStorage storage_;

  /// Mapping between strings and IDs.
  /// If the UniquingFilenameTable was initialized directly from
  /// ConsecutiveStringStorage, the mapping won't contain the strings from
  /// the storage (used for loading from a bytecode bundle, when we know there
  /// will be no more filenames added).
  StringSetVector filenames_{};

 public:
  explicit UniquingFilenameTable() = default;
  explicit UniquingFilenameTable(ConsecutiveStringStorage &&storage)
      : storage_(std::move(storage)) {}

  /// \return a view of the string table entries in the underlying storage.
  StringTableEntry::StringTableRefTy getStringTableView() const {
    return storage_.getStringTableView();
  }
  /// \return a view of the string table storage.
  StringTableEntry::StringStorageRefTy getStringStorageView() const {
    return storage_.getStringStorageView();
  }

  /// Given a \p filename, add it to the table if it does not exist. \return
  /// the index at which the filename will be found in the string storage
  /// instance this table will eventually be converted into.
  uint32_t addFilename(llvh::StringRef filename);

  /// Populate the storage with all the strings currently in the mapping.
  /// The order of index entries in the resulting storage reflects the indices
  /// returned by calls to \c addFilename.
  /// This class does not support re-ordering the storage after the fact (as
  /// call-sites of \c addFilename may rely on the order being preserved).
  void appendFilenamesToStorage();
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H
