/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_BCGEN_TESTHELPERS_H
#define HERMES_UNITTESTS_BCGEN_TESTHELPERS_H

#include "hermes/AST/Context.h"

#include <stdint.h>
#include <vector>
#include "hermes/BCGen/HBC/HBC.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {

/// Compile source code \p source into Hermes bytecode module, asserting that it
/// can be compiled successfully. \return the bytecode module.
std::unique_ptr<hbc::BytecodeModule> bytecodeModuleForSource(
    const char *source,
    BytecodeGenerationOptions opts = BytecodeGenerationOptions::defaults());

/// Compile source code \p source into Hermes bytecode, asserting that it can be
/// compiled successfully. \return the bytecode as a vector of bytes.
std::vector<uint8_t> bytecodeForSource(
    const char *source,
    BytecodeGenerationOptions opts = BytecodeGenerationOptions::defaults());

} // namespace hermes

#endif
