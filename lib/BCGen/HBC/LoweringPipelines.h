/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_LOWERINGPIPELINES_H
#define HERMES_BCGEN_HBC_LOWERINGPIPELINES_H

//===----------------------------------------------------------------------===//
/// \file
/// Lowering pipelines for HBC.
/// This file exists to allow us to declare all the pipelines in one place.
//===----------------------------------------------------------------------===//

namespace hermes {

class Module;
class Function;
struct BytecodeGenerationOptions;

namespace hbc {

class HVMRegisterAllocator;

/// Lower module IR to LIR, so it is suitable for register allocation.
void lowerModuleIR(Module *M, const BytecodeGenerationOptions &options);

/// Perform final lowering of a register-allocated function's IR.
void lowerAllocatedFunctionIR(
    Function *F,
    HVMRegisterAllocator &RA,
    const BytecodeGenerationOptions &options);

} // namespace hbc
} // namespace hermes

#endif
