/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "llvh/ADT/DenseMap.h"

#include <cstdint>

namespace hermes::hbc {

/// The filename ID and source map URL ID of a buffer.
struct FileAndSourceMapId {
  /// ID of the filename when added to BytecodeFunctionGenerator.
  uint32_t filenameId;
  /// ID of the source map URL when added as a file to
  /// BytecodeFunctionGenerator.
  uint32_t sourceMappingUrlId;
};

/// A map from buffer ID to filelname+source map.
/// Looking up filename/sourcemap id for each instruction is pretty slow,
/// so we cache it.
using FileAndSourceMapIdCache =
    llvh::SmallDenseMap<unsigned, FileAndSourceMapId>;

} // namespace hermes::hbc
