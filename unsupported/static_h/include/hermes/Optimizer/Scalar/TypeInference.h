/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_TYPEINFERENCE_H
#define HERMES_OPTIMIZER_SCALAR_TYPEINFERENCE_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Infers the types of instructions and enables other optimization passes to
/// optimize based on the deduced types.
class TypeInference : public ModulePass {
 public:
  explicit TypeInference() : ModulePass("TypeInference") {}
  ~TypeInference() override = default;

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_TYPEINFERENCE_H
