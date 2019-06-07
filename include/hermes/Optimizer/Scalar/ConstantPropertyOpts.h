/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_OPTIMIZER_SCALAR_CONSTANT_PROPERTY_OPTS_H
#define HERMES_OPTIMIZER_SCALAR_CONSTANT_PROPERTY_OPTS_H

#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/CallGraphProvider.h"

namespace hermes {

/// Implementation of propagation of constant elements in object
/// literals to relevant property load instruction.
class ConstantPropertyOpts : public ModulePass {
  /// Reference to the call graph provider which contained
  /// the results from the closure analysis (or local analysis)
  CallGraphProvider *cgp_;

  bool runOnFunction(Function *F);

  /// \returns a Literal* that LoadProperty returns, or nullptr.
  Literal *simplifyLoadPropertyInst(LoadPropertyInst *LPI);

 public:
  explicit ConstantPropertyOpts() : ModulePass("ConstantPropertyOpts") {}
  ~ConstantPropertyOpts() override = default;

  bool runOnModule(Module *M) override;
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_CONSTANT_PROPERTY_OPTS_H
