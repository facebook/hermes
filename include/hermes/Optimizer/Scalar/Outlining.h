/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_OUTLINING_H
#define HERMES_OPTIMIZER_SCALAR_OUTLINING_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Refactor repeated sequences of instructions into functions.
class Outlining : public ModulePass {
 public:
  explicit Outlining() : hermes::ModulePass("Outlining") {}
  ~Outlining() override = default;

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_OUTLINING_H
