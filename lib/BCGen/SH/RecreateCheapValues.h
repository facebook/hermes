#ifndef HERMES_BCGEN_SH_RECREATECHEAPVALUES_H
#define HERMES_BCGEN_SH_RECREATECHEAPVALUES_H

#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes::sh {

/// This pass replaces Movs with LoadConstant when the constant is not a
/// pointer.
/// Must run after RA since that's when Movs are introduced.
class RecreateCheapValues : public FunctionPass {
 public:
  explicit RecreateCheapValues(hbc::HVMRegisterAllocator &RA)
      : FunctionPass("RecreateCheapValues"), RA_(RA) {}
  ~RecreateCheapValues() override = default;
  bool runOnFunction(Function *F) override {
    IRBuilder builder(F);
    llvh::SmallPtrSet<Instruction *, 4> potentiallyUnused;
    bool changed = false;

    for (auto &BB : *F) {
      IRBuilder::InstructionDestroyer destroyer;
      for (auto &I : BB) {
        auto *mov = llvh::dyn_cast<MovInst>(&I);
        if (!mov)
          continue;
        auto *load = llvh::dyn_cast<HBCLoadConstInst>(mov->getSingleOperand());
        if (!load)
          continue;
        Literal *literal = load->getConst();

        if (!literal->getType().isNonPtr())
          continue;

        builder.setInsertionPoint(mov);
        auto *recreation = builder.createHBCLoadConstInst(literal);
        RA_.updateRegister(recreation, RA_.getRegister(mov));
        mov->replaceAllUsesWith(recreation);
        destroyer.add(mov);
        // Defer checking whether the load is unused until after any Mov(s)
        // using it have been destroyed.
        potentiallyUnused.insert(load);
        changed = true;
      }
    }

    {
      IRBuilder::InstructionDestroyer destroyer;
      for (auto *inst : potentiallyUnused) {
        if (!inst->hasUsers()) {
          destroyer.add(inst);
        }
      }
    }
    return changed;
  }

 private:
  hbc::HVMRegisterAllocator &RA_;
};

} // namespace hermes::sh

#endif
