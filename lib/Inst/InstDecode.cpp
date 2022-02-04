/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Inst/InstDecode.h"

#include "hermes/Support/Conversions.h"

#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace inst {

/// Store the structure of each opcode: size, number of operands and their
/// types.
static const struct {
  /// Size of the entire instruction in bytes.
  uint8_t size;
  /// Number of operands.
  uint8_t numOperands;
  /// The type of each operand.
  OperandType operandType[INST_MAX_OPERANDS];
} meta[] = {
#define DEFINE_OPCODE_0(name) {sizeof(inst::name##Inst), 0, {}},
#define DEFINE_OPCODE_1(name, t1) \
  {sizeof(inst::name##Inst), 1, {OperandType::t1}},
#define DEFINE_OPCODE_2(name, t1, t2) \
  {sizeof(inst::name##Inst), 2, {OperandType::t1, OperandType::t2}},
#define DEFINE_OPCODE_3(name, t1, t2, t3) \
  {sizeof(inst::name##Inst),              \
   3,                                     \
   {OperandType::t1, OperandType::t2, OperandType::t3}},
#define DEFINE_OPCODE_4(name, t1, t2, t3, t4) \
  {sizeof(inst::name##Inst),                  \
   4,                                         \
   {OperandType::t1, OperandType::t2, OperandType::t3, OperandType::t4}},
#define DEFINE_OPCODE_5(name, t1, t2, t3, t4, t5) \
  {sizeof(inst::name##Inst),                      \
   5,                                             \
   {OperandType::t1,                              \
    OperandType::t2,                              \
    OperandType::t3,                              \
    OperandType::t4,                              \
    OperandType::t5}},
#define DEFINE_OPCODE_6(name, t1, t2, t3, t4, t5, t6) \
  {sizeof(inst::name##Inst),                          \
   6,                                                 \
   {OperandType::t1,                                  \
    OperandType::t2,                                  \
    OperandType::t3,                                  \
    OperandType::t4,                                  \
    OperandType::t5,                                  \
    OperandType::t6}},

#include "hermes/BCGen/HBC/BytecodeList.def"
};

InstMetaData getInstMetaData(OpCode opCode) {
  assert(opCode < OpCode::_last && "invalid OpCode");

  const auto *pm = meta + (int)opCode;
  InstMetaData res;

  res.opCode = opCode;
  res.size = pm->size;
  res.numOperands = pm->numOperands;
  std::copy(
      pm->operandType, pm->operandType + pm->numOperands, res.operandType);

  return res;
}

uint8_t getInstSize(OpCode opCode) {
  assert(opCode < OpCode::_last && "invalid OpCode");
  return meta[(int)opCode].size;
}

uint8_t getOperandSize(OperandType type) {
#define DEFINE_OPERAND_TYPE(name, ctype) \
  case OperandType::name:                \
    return sizeof(ctype);
  switch (type) {
#include "hermes/BCGen/HBC/BytecodeList.def"
  }
  llvm_unreachable("Invalid operand type");
}

DecodedInstruction decodeInstruction(const Inst *inst) {
  DecodedInstruction decoded;
  decoded.meta = getInstMetaData(inst->opCode);

  switch (inst->opCode) {
#define DEFINE_OPCODE_0(name) \
  case OpCode::name:          \
    break;

#define DEFINE_OPCODE_1(name, op1type)              \
  case OpCode::name:                                \
    decoded.operandValue[0].set(inst->i##name.op1); \
    break;

#define DEFINE_OPCODE_2(name, op1type, op2type)     \
  case OpCode::name:                                \
    decoded.operandValue[0].set(inst->i##name.op1); \
    decoded.operandValue[1].set(inst->i##name.op2); \
    break;

#define DEFINE_OPCODE_3(name, op1type, op2type, op3type) \
  case OpCode::name:                                     \
    decoded.operandValue[0].set(inst->i##name.op1);      \
    decoded.operandValue[1].set(inst->i##name.op2);      \
    decoded.operandValue[2].set(inst->i##name.op3);      \
    break;

#define DEFINE_OPCODE_4(name, op1type, op2type, op3type, op4type) \
  case OpCode::name:                                              \
    decoded.operandValue[0].set(inst->i##name.op1);               \
    decoded.operandValue[1].set(inst->i##name.op2);               \
    decoded.operandValue[2].set(inst->i##name.op3);               \
    decoded.operandValue[3].set(inst->i##name.op4);               \
    break;

#define DEFINE_OPCODE_5(name, op1type, op2type, op3type, op4type, op5type) \
  case OpCode::name:                                                       \
    decoded.operandValue[0].set(inst->i##name.op1);                        \
    decoded.operandValue[1].set(inst->i##name.op2);                        \
    decoded.operandValue[2].set(inst->i##name.op3);                        \
    decoded.operandValue[3].set(inst->i##name.op4);                        \
    decoded.operandValue[4].set(inst->i##name.op5);                        \
    break;

#define DEFINE_OPCODE_6(                                        \
    name, op1type, op2type, op3type, op4type, op5type, op6type) \
  case OpCode::name:                                            \
    decoded.operandValue[0].set(inst->i##name.op1);             \
    decoded.operandValue[1].set(inst->i##name.op2);             \
    decoded.operandValue[2].set(inst->i##name.op3);             \
    decoded.operandValue[3].set(inst->i##name.op4);             \
    decoded.operandValue[4].set(inst->i##name.op5);             \
    decoded.operandValue[5].set(inst->i##name.op6);             \
    break;

#include "hermes/BCGen/HBC/BytecodeList.def"
    default:
      llvm_unreachable("invalid instruction");
  }

  return decoded;
}

void dumpOperand(llvh::raw_ostream &OS, OperandType type, OperandValue value) {
  switch (type) {
    case OperandType::Reg8:
    case OperandType::Reg32:
      OS << "r" << value.integer;
      break;
    case OperandType::UInt8:
    case OperandType::UInt16:
    case OperandType::UInt32:
      OS << "$" << value.integer;
      break;
    case OperandType::Addr8:
    case OperandType::Addr32:
      OS << "@" << value.integer;
      break;
    case OperandType::Imm32:
      OS << value.integer;
      break;
    case OperandType::Double:
      char buf[NUMBER_TO_STRING_BUF_SIZE];
      hermes::numberToString(value.floating, buf, sizeof(buf));
      OS << buf;
      break;
  }
}

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &OS,
    const DecodedInstruction &decoded) {
  OS << llvh::left_justify(getOpCodeString(decoded.meta.opCode), 17);

  for (unsigned i = 0; i < decoded.meta.numOperands; ++i) {
    OS << (i == 0 ? " " : ", ");
    dumpOperand(OS, decoded.meta.operandType[i], decoded.operandValue[i]);
  }

  return OS;
}

} // namespace inst
} // namespace hermes
