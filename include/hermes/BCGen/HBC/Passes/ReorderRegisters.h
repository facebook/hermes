/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_REORDERREGISTERS_H
#define HERMES_BCGEN_HBC_PASSES_REORDERREGISTERS_H

#include "hermes/BCGen/HBC/Passes.h"

namespace hermes::hbc {

/// Reorder registers to place number and non-pointer registers in lower
/// registers, and registers in inner loops in lower-indexed registers within
/// each of those classes.
/// This is useful for the JIT because it can allocate hardware registers
/// specially for known types.
///
/// Does NOT change the ordering of registers in RegClass::Other,
/// because it hurts compressibility of bytecode (3-5% regression in compressed
/// size based on the file). The ordering doesn't matter for the JIT for "Other"
/// registers right now so it also has no benefit.
Pass *createReorderRegisters(HVMRegisterAllocator &RA);

} // namespace hermes::hbc

#endif // HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H
