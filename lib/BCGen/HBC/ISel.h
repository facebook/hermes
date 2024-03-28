/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_ISEL_H
#define HERMES_BCGEN_HBC_ISEL_H

#include "llvh/ADT/DenseMap.h"

#include <cstdint>

namespace hermes {

class Function;
class SourceMapGenerator;
struct BytecodeGenerationOptions;

namespace hbc {

class BytecodeFunctionGenerator;
class HVMRegisterAllocator;

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

/// Generate the bytecode stream for the function.
void runHBCISel(
    Function *F,
    BytecodeFunctionGenerator *BCFGen,
    HVMRegisterAllocator &RA,
    const BytecodeGenerationOptions &options,
    FileAndSourceMapIdCache &debugIdCache,
    SourceMapGenerator *outSourceMap);

} // namespace hbc
} // namespace hermes

#endif
