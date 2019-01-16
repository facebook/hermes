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
