/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODELIST_H
#define HERMES_BCGEN_HBC_BYTECODELIST_H

#include "hermes/Support/Conversions.h"

#include <vector>

namespace hermes {
namespace hbc {

/// Universal type of all operands passed to emit methods.
using param_t = int64_t;

/// offset_t should only be used by instruction emitting to indicate the current
/// stream location.
using offset_t = uint32_t;

/// opcode_atom_t represents the smallest numeric type that we use for opcodes,
/// opeands and offsets. Instructions can use multiple atoms to represent larger
/// data types like constants or long jumps.
using opcode_atom_t = uint8_t;

enum Operator {
// Generate enums for each opcode:
// JmpOp,
// MovOp,
// ...
#define DEFINE_OPCODE(name) name##Op,
#include "hermes/BCGen/HBC/BytecodeList.def"
};

class BytecodeInstructionGenerator {
 protected:
  /// A list of opcodes.
  std::vector<opcode_atom_t> opcodes_{};

 public:
  /// Returns the current location of the bytecode stream.
  offset_t getCurrentLocation() {
    return opcodes_.size();
  }

  offset_t emitOpcode(Operator op) {
    auto loc = getCurrentLocation();
    emitUInt8(op);
    return loc;
  }
  /// Encode the parameter \p t in little-endian using \p size number of bytes.
  void emitOperand(param_t t, int size) {
    while (size--) {
      opcodes_.push_back((opcode_atom_t)t);
      t >>= 8;
    }
  }

  /// Take over the opcodes. opcodes_ will become empty after this call.
  std::vector<opcode_atom_t> acquireBytecode() {
    return std::move(opcodes_);
  }

// Generate emitters per operand type, such as:
// void emitUInt8(param_t value) { ... }
// void emitReg8(param_t value) { ... }
// ...
// We also assert that the value can fit into ctype.
// For integer values, ((param_t)(ctype)value) == value will do the job;
// We also want doubles to pass the check unconditionally.
#define DEFINE_OPERAND_TYPE(name, ctype)          \
  void emit##name(param_t value) {                \
    assert(                                       \
        (((param_t)(ctype)value) == value ||      \
         std::is_floating_point<ctype>::value) && \
        "Value does not fit in " #ctype);         \
    emitOperand(value, sizeof(ctype));            \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"

// Generate instruction emitters, such as:
// void emitJmp(param_t t1) { ...; }
// void emitMov(param_t t1, param_t t2) { ...; }
// ...
#define DEFINE_OPCODE_0(name)        \
  offset_t emit##name() {            \
    auto loc = emitOpcode(name##Op); \
    return loc;                      \
  };
#define DEFINE_OPCODE_1(name, t1)    \
  offset_t emit##name(param_t p1) {  \
    auto loc = emitOpcode(name##Op); \
    emit##t1(p1);                    \
    return loc;                      \
  };
#define DEFINE_OPCODE_2(name, t1, t2)           \
  offset_t emit##name(param_t p1, param_t p2) { \
    auto loc = emitOpcode(name##Op);            \
    emit##t1(p1);                               \
    emit##t2(p2);                               \
    return loc;                                 \
  };
#define DEFINE_OPCODE_3(name, t1, t2, t3)                   \
  offset_t emit##name(param_t p1, param_t p2, param_t p3) { \
    auto loc = emitOpcode(name##Op);                        \
    emit##t1(p1);                                           \
    emit##t2(p2);                                           \
    emit##t3(p3);                                           \
    return loc;                                             \
  };
#define DEFINE_OPCODE_4(name, t1, t2, t3, t4)                           \
  offset_t emit##name(param_t p1, param_t p2, param_t p3, param_t p4) { \
    auto loc = emitOpcode(name##Op);                                    \
    emit##t1(p1);                                                       \
    emit##t2(p2);                                                       \
    emit##t3(p3);                                                       \
    emit##t4(p4);                                                       \
    return loc;                                                         \
  };
#define DEFINE_OPCODE_5(name, t1, t2, t3, t4, t5)                   \
  offset_t emit##name(                                              \
      param_t p1, param_t p2, param_t p3, param_t p4, param_t p5) { \
    auto loc = emitOpcode(name##Op);                                \
    emit##t1(p1);                                                   \
    emit##t2(p2);                                                   \
    emit##t3(p3);                                                   \
    emit##t4(p4);                                                   \
    emit##t5(p5);                                                   \
    return loc;                                                     \
  };
#define DEFINE_OPCODE_6(name, t1, t2, t3, t4, t5, t6) \
  offset_t emit##name(                                \
      param_t p1,                                     \
      param_t p2,                                     \
      param_t p3,                                     \
      param_t p4,                                     \
      param_t p5,                                     \
      param_t p6) {                                   \
    auto loc = emitOpcode(name##Op);                  \
    emit##t1(p1);                                     \
    emit##t2(p2);                                     \
    emit##t3(p3);                                     \
    emit##t4(p4);                                     \
    emit##t5(p5);                                     \
    emit##t6(p6);                                     \
    return loc;                                       \
  };
#include "hermes/BCGen/HBC/BytecodeList.def"

  /// Wrapper around emitLoadConstDouble, allow us to pass
  /// a raw double value instead of having to convert it
  /// to int64_t.
  offset_t emitLoadConstDoubleDirect(param_t dst, double value) {
    return emitLoadConstDouble(dst, llvh::DoubleToBits(value));
  }
};
} // namespace hbc
} // namespace hermes

#endif
