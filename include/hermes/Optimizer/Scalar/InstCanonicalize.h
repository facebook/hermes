/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_INSTCANONICALIZE_H
#define HERMES_OPTIMIZER_SCALAR_INSTCANONICALIZE_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Performs simple canonicalization.
class InstCanonicalize : public FunctionPass {
 public:
  explicit InstCanonicalize() : FunctionPass("InstCanonicalize") {}
  ~InstCanonicalize() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_INSTCANONICALIZE_H
