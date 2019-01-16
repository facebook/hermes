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
