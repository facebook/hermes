/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_ISEL_H
#define HERMES_BCGEN_HBC_ISEL_H

#include "hermes/BCGen/HBC/FileAndSourceMapIdCache.h"

namespace hermes {

class Function;
class SourceMapGenerator;
struct BytecodeGenerationOptions;

namespace hbc {

class BytecodeFunctionGenerator;
class HVMRegisterAllocator;

/// Generate the bytecode stream for the function.
void runHBCISel(
    Function *F,
    BytecodeFunctionGenerator *BCFGen,
    HVMRegisterAllocator &RA,
    const BytecodeGenerationOptions &options,
    FileAndSourceMapIdCache &debugIdCache);

} // namespace hbc
} // namespace hermes

#endif
