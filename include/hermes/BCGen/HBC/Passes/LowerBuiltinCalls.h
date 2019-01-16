#ifndef HERMES_BCGEN_HBC_PASSES_LOWERBUILTINCALLS_H
#define HERMES_BCGEN_HBC_PASSES_LOWERBUILTINCALLS_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace hbc {

/// Detect calls to builtin methods like `Object.keys()` and replace them with
/// HBCCallBuiltinInst.
class LowerBuiltinCalls : public FunctionPass {
 public:
  explicit LowerBuiltinCalls() : FunctionPass("LowerBuiltinCalls") {}

  bool runOnFunction(Function *F) override;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_PASSES_LOWERBUILTINCALLS_H
