#ifndef HERMES_OPTIMIZER_SCALAR_DCE_H
#define HERMES_OPTIMIZER_SCALAR_DCE_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Deletes simple dead code patterns.
class DCE : public ModulePass {
 public:
  explicit DCE() : hermes::ModulePass("DCE") {}
  ~DCE() override = default;

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_DCE_H
