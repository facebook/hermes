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
class ReorderRegisters : public FunctionPass {
 public:
  explicit ReorderRegisters(HVMRegisterAllocator &RA)
      : FunctionPass("ReorderRegisters"), RA_(RA) {}

  bool runOnFunction(Function *F) override;

 private:
  HVMRegisterAllocator &RA_;
};

} // namespace hermes::hbc

#endif // HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H
