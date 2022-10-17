/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_H
#define HERMES_BCGEN_HBC_PASSES_H

#include "hermes/BCGen/BCOpt.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/ISel.h"
#include "hermes/BCGen/HBC/Passes.h"

namespace hermes {
namespace hbc {

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

class LoadConstants : public FunctionPass {
  /// Check whether a particular operand of an instruction must stay
  /// as literal and hence cannot be lowered into load_const instruction.
  bool operandMustBeLiteral(Instruction *Inst, unsigned opIndex);

 public:
  explicit LoadConstants(bool optimizationEnabled)
      : FunctionPass("LoadConstants"),
        optimizationEnabled_(optimizationEnabled) {}
  ~LoadConstants() override = default;

  bool runOnFunction(Function *F) override;

 private:
  bool const optimizationEnabled_;
};

class LoadParameters : public FunctionPass {
 public:
  explicit LoadParameters() : FunctionPass("LoadParameters") {}
  ~LoadParameters() override = default;
  bool runOnFunction(Function *F) override;
};

/// Lower LoadFrameInst, StoreFrameInst and CreateFunctionInst.
class LowerLoadStoreFrameInst : public FunctionPass {
  /// Decide the correct scope to use when dealing with given variable.
  ScopeCreationInst *
  getScope(IRBuilder &builder, Variable *var, ScopeCreationInst *environment);

 public:
  explicit LowerLoadStoreFrameInst()
      : FunctionPass("LowerLoadStoreFrameInst") {}
  ~LowerLoadStoreFrameInst() override = default;
  bool runOnFunction(Function *F) override;
};

// Lower uses of the JS `arguments` array into HBC*Arguments* instructions.
class LowerArgumentsArray : public FunctionPass {
 private:
  CreateArgumentsInst *getCreateArgumentsInst(Function *F);

 public:
  explicit LowerArgumentsArray() : FunctionPass("LowerArgumentsArray") {}
  ~LowerArgumentsArray() override = default;
  bool runOnFunction(Function *F) override;
};

/// HBCReifyArguments writes are idempotent, but CSE and such doesn't support
/// this. This pass dedups by block and then by dominators.
class DedupReifyArguments : public FunctionPass {
 public:
  explicit DedupReifyArguments() : FunctionPass("DedupReifyArguments") {}
  ~DedupReifyArguments() override = default;
  bool runOnFunction(Function *F) override;
};

class LowerConstruction : public FunctionPass {
 public:
  explicit LowerConstruction() : FunctionPass("LowerConstruction") {}
  ~LowerConstruction() override = default;
  bool runOnFunction(Function *F) override;
};

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
class LowerCalls : public FunctionPass {
 public:
  explicit LowerCalls(HVMRegisterAllocator &RA)
      : FunctionPass("LowerCalls"), RA_(RA) {}
  ~LowerCalls() override = default;
  bool runOnFunction(Function *F) override;

 protected:
  HVMRegisterAllocator &RA_;
};

/// A new LoadConstTrue is 2 bytes, while copying an old one with a Mov is 3.
/// This pass replaces Movs with Loads when it saves space. Must run after RA
/// since that's when Movs are introduced.
class RecreateCheapValues : public FunctionPass {
 public:
  explicit RecreateCheapValues(HVMRegisterAllocator &RA)
      : FunctionPass("RecreateCheapValues"), RA_(RA) {}
  ~RecreateCheapValues() override = default;
  bool runOnFunction(Function *F) override;

 private:
  HVMRegisterAllocator &RA_;
};

/// This pass removes unnecessary Movs and HBCLoadConstInsts that have been
/// introduced after CSE by register allocation.
/// It keeps track of all registers that are written to by HBCLoadConstInst
/// or by Movs whose operand is a HBCLoadConstInst, and removes all successive
/// instructions that write the same value to that register until it is written
/// to by a different instruction.
class LoadConstantValueNumbering : public FunctionPass {
 public:
  explicit LoadConstantValueNumbering(HVMRegisterAllocator &RA)
      : FunctionPass("LoadConstantValueNumbering"), RA_(RA) {}
  ~LoadConstantValueNumbering() override = default;
  bool runOnFunction(Function *F) override;

 private:
  HVMRegisterAllocator &RA_;
};

/// Add required Movs for instructions that ended up in Reg256+ that can't
/// fit in a Reg8 in our bytecode.
class SpillRegisters : public FunctionPass {
 public:
  explicit SpillRegisters(HVMRegisterAllocator &RA)
      : FunctionPass("SpillRegisters"), RA_(RA) {}
  ~SpillRegisters() override = default;
  bool runOnFunction(Function *F) override;

 protected:
  HVMRegisterAllocator &RA_;
  /// The first "high" register.
  static const int boundary_ = 256;
  /// The registers from 0 to reserved_ are used for temp space.
  /// The most we'll currently need appears to be 6 for GetByPNameInst.
  static const int reserved_ = 6;

  bool requiresShortOutput(Instruction *I);
  bool requiresShortOperand(Instruction *I, int op);
  bool modifiesOperandRegister(Instruction *I, int op);

  bool isShort(Register reg) {
    return reg.getIndex() < boundary_;
  }
  Register getReserved(int i) {
    assert(i < reserved_ && "Using too many reserved regs.");
    return Register(i);
  }
  // Push up all register from 0 to reserved_ to use as temp space.
  void reserveLowRegisters(Function *F) {
    RA_.allocateSpillTempCount(reserved_);
    for (auto &BB : F->getBasicBlockList()) {
      for (auto &inst : BB) {
        if (RA_.isAllocated(&inst)) {
          auto reg = RA_.getRegister(&inst);
          RA_.updateRegister(&inst, reg.getConsecutive(reserved_));
        }
      }
    }
  }
};

/// Attempt to lower a switch statement into a jump table.
/// Must be run prior to the pass lowering switches into linear search.
/// Currently this only affects switches that are suffciently dense and with
/// positive cases.
class LowerSwitchIntoJumpTables : public FunctionPass {
 public:
  explicit LowerSwitchIntoJumpTables()
      : FunctionPass("LowerSwitchIntoJumpTables") {}
  ~LowerSwitchIntoJumpTables() override = default;
  bool runOnFunction(Function *F) override;

 private:
  bool lowerIntoJumpTable(SwitchInst *switchInst);
};

} // namespace hbc
} // namespace hermes
#endif
