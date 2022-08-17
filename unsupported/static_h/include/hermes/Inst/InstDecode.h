/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Utilities for decoding instructions into a universal representation. To be
/// used only for debugging.
//===----------------------------------------------------------------------===//
#ifndef HERMES_INST_INSTDECODE_H
#define HERMES_INST_INSTDECODE_H

#include "hermes/Inst/Inst.h"

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace inst {

/// An enum for every possible instruction operand type.
enum class OperandType : uint8_t {
#define DEFINE_OPERAND_TYPE(name, type) name,
#include "hermes/BCGen/HBC/BytecodeList.def"
};

inline constexpr bool isOperandTypeFloating(OperandType opType) {
  return opType == OperandType::Double;
}
inline constexpr bool isOperandTypeInteger(OperandType opType) {
  return !isOperandTypeFloating(opType);
}

/// The maximum number of operands used by any instruction.
constexpr unsigned INST_MAX_OPERANDS = 6;

/// Metadata describing a single instructions: its size, number of operands
/// and the type of each operand.
struct InstMetaData {
  /// The instruction opcode.
  OpCode opCode;
  /// Size of the entire instruction in bytes.
  uint8_t size;
  /// Number of operands.
  uint8_t numOperands;
  /// The type of each operand.
  OperandType operandType[INST_MAX_OPERANDS];
};

/// A union combining all possible types of operand values.
union OperandValue {
  double floating;
  int64_t integer;

  /// Set the appropriate union member depending on the supplied argument type.
  template <typename T>
  inline void set(T val) {
    if (std::is_integral<T>::value)
      integer = (int64_t)val;
    else
      floating = (double)val;
  }
};

/// The information in a specific instance of an instruction, decoded in a way
/// that makes it easier to access by algorithms that are not specialized by
/// instruction.
struct DecodedInstruction {
  InstMetaData meta;
  OperandValue operandValue[INST_MAX_OPERANDS];
};

/// \return the size of the specified instruction in bytes.
uint8_t getInstSize(OpCode opCode);

/// \return the size of the specified operand type in bytes.
uint8_t getOperandSize(OperandType type);

/// \return the name of the instruction as string.
llvh::StringRef getOpCodeString(OpCode opCode);

/// \return the metadata for the specified opcode \p opCode.
InstMetaData getInstMetaData(OpCode opCode);

/// \return the decoded form of the specified instruction instance \p inst.
DecodedInstruction decodeInstruction(const Inst *inst);

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &OS,
    const DecodedInstruction &decoded);

/// Print a single operand to the specified stream.
void dumpOperand(llvh::raw_ostream &OS, OperandType type, OperandValue value);

} // namespace inst
} // namespace hermes

#endif // HERMES_INST_INSTDECODE_H
