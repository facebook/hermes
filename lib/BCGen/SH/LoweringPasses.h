/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SH_LOWERINGPASSES_H
#define HERMES_BCGEN_SH_LOWERINGPASSES_H

#include "SHRegAlloc.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes::sh {

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
Pass *createLowerCalls(SHRegisterAllocator &RA);

/// This pass replaces Movs with LoadConstant when the constant is not a
/// pointer.
/// Must run after RA since that's when Movs are introduced.
Pass *createRecreateCheapValues(SHRegisterAllocator &RA);

} // namespace hermes::sh

#endif
