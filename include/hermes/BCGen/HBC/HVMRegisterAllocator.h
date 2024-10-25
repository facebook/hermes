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

class BaseCallInst;

namespace hbc {

class HVMRegisterAllocator : public RegisterAllocator {
 private:
  unsigned max_parameter_count_ = 0;
  unsigned spill_count_ = 0;

 protected:
  void handleInstruction(Instruction *I) override;
  void allocateCallInst(BaseCallInst *I);
  bool hasTargetSpecificLowering(Instruction *I) override;

 public:
  /// Number of additional registers that must be allocated to the call
  /// instruction, after the argument registers, for internal VM usage (the VM
  /// builds the call frame in those additional register slots).
  static constexpr unsigned CALL_EXTRA_REGISTERS =
      StackFrameLayout::CallerExtraRegistersAtEnd;

  explicit HVMRegisterAllocator(Function *func) : RegisterAllocator(func) {}

  /// \return the highest index in RegClass::Other used for call arguments.
  Register lastCallArgRegister() {
    // Spill and parameter registers are RegClass::Other, but they aren't in the
    // register file so they need to be added just like in
    // getMaxHVMRegisterUsage().
    // Subtract CALL_EXTRA_REGISTERS because those are VM internal.
    // Subtract 1 because we want a real register, not an exclusive bound.
    return Register(
        RegClass::Other,
        RegisterAllocator::getMaxRegisterUsage(RegClass::Other) + spill_count_ +
            max_parameter_count_ - CALL_EXTRA_REGISTERS - 1);
  }
  /// \return the maximum number of HVM registers used, including spills and
  /// parameters.
  unsigned getMaxHVMRegisterUsage() {
    return getMaxInstructionRegister() + spill_count_ + max_parameter_count_;
  }

  /// Get the maximum register allocated to regular instructions
  /// (i.e. not including parameter lists or spilling).
  unsigned getMaxInstructionRegister() {
    static_assert(
        (int)RegClass::Other + 1 == (int)RegClass::_last,
        "Other must be the final RegClass");
    static_assert((int)RegClass::NoOutput == 0, "NoOutput must be 0");
    unsigned maxRegCount = 0;

    // Skip NoOutput because it's not real.
    for (uint32_t i = 1; i < (uint32_t)RegClass::_last; ++i) {
      maxRegCount += RegisterAllocator::getMaxRegisterUsage((RegClass)i);
    }
    return maxRegCount;
  }

  void allocateParameterCount(unsigned count) {
    if (max_parameter_count_ < count) {
      max_parameter_count_ = count;
    }
  }
  void allocateSpillTempCount(unsigned count) {
    spill_count_ = count;
  }

  /// Registers have a RegClass and an index within that class.
  /// \return actual HVM register used by a given Register, taking into account
  /// the RegClass.
  unsigned getHVMRegisterIndex(Register r) const {
    switch (r.getClass()) {
      case RegClass::Number:
        return r.getIndexInClass();
      case RegClass::NonPtr:
        return r.getIndexInClass() + getMaxRegisterUsage(RegClass::Number);
      case RegClass::Other:
        return r.getIndexInClass() + getMaxRegisterUsage(RegClass::Number) +
            getMaxRegisterUsage(RegClass::NonPtr);
      case RegClass::NoOutput:
      case RegClass::_last:
        hermes_fatal("register has no corresponding HVM index");
    }
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_HVMREGISTERALLOCATOR_H
