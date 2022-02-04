/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H
#define HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/Support/StringSetVector.h"

#include "llvh/ADT/StringRef.h"

namespace hermes {
namespace hbc {

/// Gathers and uniques filenames during bytecode generation.
struct UniquingFilenameTable {
  /// Given a \p filename, add it to the table if it does not exist. \return
  /// the index at which the filename will be found in the string storage
  /// instance this table will eventually be converted into.
  uint32_t addFilename(llvh::StringRef filename);

  /// Converts \p table into a string storage instance.  The order of index
  /// entries in the resulting storage reflects the indices returned by calls to
  /// \c addFilename.  This class does not support re-ordering the storage after
  /// the fact (as call-sites of \c addFilename may rely on the order being
  /// preserved).
  static ConsecutiveStringStorage toStorage(UniquingFilenameTable table);

 private:
  StringSetVector filenames_;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_UNIQUINGFILENAMETABLE_H
