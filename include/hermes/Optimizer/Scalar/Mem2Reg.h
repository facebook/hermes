#ifndef HERMES_OPTIMIZER_SCALAR_MEM2REG_H
#define HERMES_OPTIMIZER_SCALAR_MEM2REG_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// This optimization promotes stack allocations into virtual registers.
/// The algorithm is based on:
///
///  Sreedhar and Gao. A linear time algorithm for placing phi-nodes. POPL '95.
class Mem2Reg : public FunctionPass {
 public:
  explicit Mem2Reg() : hermes::FunctionPass("Mem2Reg") {}
  ~Mem2Reg() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_MEM2REG_H
