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
class StackPromotion : public FunctionPass {
 public:
  explicit StackPromotion() : FunctionPass("StackPromotion") {}
  ~StackPromotion() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_STACKPROMOTION_H
