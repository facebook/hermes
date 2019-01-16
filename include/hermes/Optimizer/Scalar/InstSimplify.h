#ifndef HERMES_OPTIMIZER_SCALAR_INSTSIMPLIFY_H
#define HERMES_OPTIMIZER_SCALAR_INSTSIMPLIFY_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Performs simple peephole optimizations.
class InstSimplify : public FunctionPass {
 public:
  explicit InstSimplify() : FunctionPass("InstSimplify") {}
  ~InstSimplify() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_INSTSIMPLIFY_H
