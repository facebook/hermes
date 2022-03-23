/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_CSE_H
#define HERMES_OPTIMIZER_SCALAR_CSE_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Deletes simple dead code patterns.
class CSE : public FunctionPass {
 public:
  explicit CSE() : FunctionPass("CSE") {}
  ~CSE() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_CSE_H
