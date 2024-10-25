/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_H
#define HERMES_BCGEN_HBC_PASSES_H

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/BCGen/RegAlloc.h"

namespace hermes {
namespace hbc {

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

class LoadConstants : public FunctionPass {
  /// Check whether a particular operand of an instruction may stay
  /// as a literal, either because the corresponding HBC instruction encodes the
  /// value as an immediate, or because there exists a variant HBC instruction
  /// encoding the value as an immediate, and this variant is guaranteed to be
  /// selected in instruction selection.  In these cases, the literal value
  /// need not be lowered into load_const instruction.
  bool operandMustBeLiteral(Instruction *Inst, unsigned opIndex);

 public:
  explicit LoadConstants() : FunctionPass("LoadConstants") {}
  ~LoadConstants() override = default;

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

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
class InitCallFrame : public FunctionPass {
 public:
  explicit InitCallFrame(HVMRegisterAllocator &RA)
      : FunctionPass("InitCallFrame"), RA_(RA) {}
  ~InitCallFrame() override = default;
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

  /// \param regHVMIndex the HVM register index for a Register.
  /// \return whether the register \p regHVMIndex is a "short" register,
  /// false if the register requires spilling.
  static bool isShort(unsigned regHVMIndex) {
    return regHVMIndex < boundary_;
  }

 protected:
  HVMRegisterAllocator &RA_;
  /// The first "high" register.
  static const int boundary_ = 256;
  /// The "Other" registers from 0 to reserved_ are used for temp space.
  /// The most we'll currently need appears to be 6 for GetByPNameInst.
  static const int reserved_ = 6;

  bool requiresShortOutput(Instruction *I);
  bool requiresShortOperand(Instruction *I, int op);
  bool modifiesOperandRegister(Instruction *I, int op);

  /// \pre \p i < reserved_.
  /// \return the "Other" reserved register at index \p i.
  Register getReserved(int i) {
    assert(i < reserved_ && "Using too many reserved regs.");
    return Register(RegClass::Other, i);
  }
  // Push up all register from 0 to reserved_ to use as temp space.
  void reserveLowRegisters(Function *F) {
    auto numberRegCount = RA_.getMaxRegisterUsage(RegClass::Number);
    auto nonPtrRegCount = RA_.getMaxRegisterUsage(RegClass::NonPtr);
    RA_.convertTypeSpecificRegsToOther();
    RA_.allocateSpillTempCount(reserved_);
    for (auto &BB : F->getBasicBlockList()) {
      for (auto &inst : BB) {
        if (!inst.hasOutput() || !RA_.isAllocated(&inst)) {
          continue;
        }
        Register reg = RA_.getRegister(&inst);
        // Remap registers into the "Other" class while keeping the ordering the
        // same. Keep reserved_ registers at the start.
        switch (reg.getClass()) {
          case RegClass::Number:
            RA_.updateRegister(
                &inst,
                Register(RegClass::Other, reg.getIndexInClass() + reserved_));
            break;
          case RegClass::NonPtr:
            RA_.updateRegister(
                &inst,
                Register(
                    RegClass::Other,
                    reg.getIndexInClass() + reserved_ + numberRegCount));
            break;
          case RegClass::Other:
            // Shift "Other" registers up by reserved_.
            RA_.updateRegister(
                &inst,
                Register(
                    RegClass::Other,
                    reg.getIndexInClass() + reserved_ + numberRegCount +
                        nonPtrRegCount));
            break;
          case RegClass::NoOutput:
            break;
          case RegClass::_last:
            hermes_fatal("invalid register class for spilling");
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
