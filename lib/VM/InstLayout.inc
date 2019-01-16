//===----------------------------------------------------------------------===//
/// \file
/// Check at compile time that instructions declared as having the same layout
/// actually do.
//===----------------------------------------------------------------------===//

namespace hermes {
namespace vm {

// Ensure that some instructions have matching layouts.
namespace validate_inst_layout {

enum OperandType {
#define DEFINE_OPERAND_TYPE(name, ...) name,
#include "hermes/BCGen/HBC/BytecodeList.def"
};

enum InstOperands {
#define DEFINE_OPCODE_0(name)                                         \
  name##_len = 0,
#define DEFINE_OPCODE_1(name, op1type)                                         \
  name##_len = 1, name##_1 = OperandType::op1type,
#define DEFINE_OPCODE_2(name, op1type, op2type)                                \
  name##_len = 2, name##_1 = OperandType::op1type,                             \
  name##_2 = OperandType::op2type,
#define DEFINE_OPCODE_3(name, op1type, op2type, op3type)                       \
  name##_len = 3, name##_1 = OperandType::op1type,                             \
  name##_2 = OperandType::op2type, name##_3 = OperandType::op3type,
#define DEFINE_OPCODE_4(name, op1type, op2type, op3type, op4type)              \
  name##_len = 4, name##_1 = OperandType::op1type,                             \
  name##_2 = OperandType::op2type, name##_3 = OperandType::op3type,            \
  name##_4 = OperandType::op4type,
#define DEFINE_OPCODE_5(name, op1type, op2type, op3type, op4type, op5type)     \
  name##_len = 5, name##_1 = OperandType::op1type,                             \
  name##_2 = OperandType::op2type, name##_3 = OperandType::op3type,            \
  name##_4 = OperandType::op4type, name##_5 = OperandType::op5type,

#include "hermes/BCGen/HBC/BytecodeList.def"
};

#define ASSERT_LAYOUT(a, b, cond)                                              \
  static_assert(cond, #a " and " #b " must have the same layout")

#define ASSERT_EQUAL_LAYOUT1(a, b)                                             \
  ASSERT_LAYOUT(a, b, a##_len >= 1 && b##_len >= 1 && a##_1 == b##_1);
#define ASSERT_EQUAL_LAYOUT2(a, b)                                             \
  ASSERT_LAYOUT(a, b, a##_len >= 2 && b##_len >= 2 && a##_1 == b##_1 &&        \
                          a##_2 == b##_2);
#define ASSERT_EQUAL_LAYOUT3(a, b)                                             \
  ASSERT_LAYOUT(a, b, a##_len >= 3 && b##_len >= 3 && a##_1 == b##_1 &&        \
                          a##_2 == b##_2 && a##_3 == b##_3);
#define ASSERT_EQUAL_LAYOUT4(a, b)                                             \
  ASSERT_LAYOUT(a, b, a##_len >= 4 && b##_len >= 4 && a##_1 == b##_1 &&        \
                          a##_2 == b##_2 && a##_3 == b##_3 && a##_4 == b##_4);
#define ASSERT_EQUAL_LAYOUT5(a, b)                                             \
  ASSERT_LAYOUT(a, b, a##_len >= 5 && b##_len >= 5 && a##_1 == b##_1 &&        \
                          a##_2 == b##_2 && a##_3 == b##_3 &&                  \
                          a##_4 == b##_4 && a##_5 == b##_5);

#include "hermes/BCGen/HBC/BytecodeList.def"

#undef ASSERT_LAYOUT
} // namespace validate_inst_layout
} // namespace vm
} // namespace hermes
