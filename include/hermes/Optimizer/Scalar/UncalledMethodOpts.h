/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_OPTIMIZER_SCALAR_UNCALLED_METHOD_OPTS_H
#define HERMES_OPTIMIZER_SCALAR_UNCALLED_METHOD_OPTS_H

#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/CallGraphProvider.h"

namespace hermes {

/// Implementation of propagation of constant elements in object
/// literals to relevant property load instruction.
class UncalledMethodOpts : public ModulePass {
  /// Reference to the call graph provider which contained
  /// the results from the closure analysis (or local analysis)
  CallGraphProvider *cgp_;

  bool runOnFunction(Function *F);

  /// is the function uncalled?
  bool isUncalled(Function *F);

  /// hollow out the instructions from the Function
  bool replaceFunctionBodyWithReturn(Function *F);

 public:
  explicit UncalledMethodOpts() : ModulePass("UncalledMethodOpts") {}
  ~UncalledMethodOpts() override = default;

  bool runOnModule(Module *M) override;
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_UNCALLED_METHOD_OPTS_H
