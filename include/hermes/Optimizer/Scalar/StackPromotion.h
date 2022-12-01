/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H
#define HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Promotes variables into stack allocations.
/// Removes all trivially unreachable functions.
class StackPromotion : public ModulePass {
 public:
  explicit StackPromotion() : ModulePass("StackPromotion") {}
  ~StackPromotion() override = default;

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H
