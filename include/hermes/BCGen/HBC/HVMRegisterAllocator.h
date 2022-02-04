/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_HVMREGISTERALLOCATOR_H
#define HERMES_BCGEN_HBC_HVMREGISTERALLOCATOR_H

#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/BCGen/RegAlloc.h"
#include "hermes/IR/IR.h"

namespace hermes {
namespace hbc {

class HVMRegisterAllocator : public RegisterAllocator {
 private:
  unsigned max_parameter_count_ = 0;
  unsigned spill_count_ = 0;

 protected:
  void handleInstruction(Instruction *I) override;
  void allocateCallInst(CallInst *I);
  bool hasTargetSpecificLowering(Instruction *I) override;

 public:
  /// Number of additional registers that must be allocated to the call
  /// instruction, after the argument registers, for internal VM usage (the VM
  /// builds the call frame in those additional register slots).
  static constexpr unsigned CALL_EXTRA_REGISTERS =
      StackFrameLayout::CallerExtraRegistersAtEnd;

  explicit HVMRegisterAllocator(Function *func) : RegisterAllocator(func) {}

  Register getLastRegister() {
    return Register(getMaxRegisterUsage() - 1);
  }

  /// Get the maximum number of registers used.
  unsigned getMaxRegisterUsage() override {
    return getMaxInstructionRegister() + spill_count_ + max_parameter_count_;
  }

  /// Get the maximum register allocated to regular instructions
  /// (i.e. not including parameter lists or spilling).
  unsigned getMaxInstructionRegister() {
    return RegisterAllocator::getMaxRegisterUsage();
  }

  void allocateParameterCount(unsigned count) {
    if (max_parameter_count_ < count) {
      max_parameter_count_ = count;
    }
  }
  void allocateSpillTempCount(unsigned count) {
    spill_count_ = count;
  }
  unsigned getSpillOffset() {
    return RegisterAllocator::getMaxRegisterUsage();
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_HVMREGISTERALLOCATOR_H
