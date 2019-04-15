/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H
#define HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H

#include <vector>

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/IR/IR.h"
#include "hermes/Utils/Options.h"

namespace hermes {
namespace hbc {

/// \return a ConsecutiveStringStorage pre-loaded with the strings from the
/// module \p M, in a way optimized for size and to take advantage of the small
/// string index instructions in hbc.
/// \param functionsToGenerate functions for which to output strings. If empty,
/// output strings for all functions.
ConsecutiveStringStorage getOrderedStringStorage(
    Module *M,
    const BytecodeGenerationOptions &options,
    const llvm::DenseSet<Function *> &functionsToGenerate);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H
