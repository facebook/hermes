/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INTERPRETER_INTERNAL_H
#define HERMES_VM_INTERPRETER_INTERNAL_H

#include "hermes/VM/Interpreter.h"

// Convenient aliases for operand registers.
#if !defined(__arm__) || !defined(__clang__) || \
    !defined(HERMESVM_ALLOW_INLINE_ASM)
#define REG(index) frameRegs[index]
#else
// Work around bad codegen from clang on armv7 that causes it to emit two
// separate loads instead of a single ldrd.
#define REG(index)                                                           \
  (*({                                                                       \
    PinnedHermesValue *addr;                                                 \
    static_assert(sizeof(PinnedHermesValue) == 8, "asm depends on HV size"); \
    static_assert(sizeof(index) <= 4, "index must fit in a register");       \
    asm("add.w   %0, %1, %2, lsl #3"                                         \
        : "=r"(addr)                                                         \
        : "r"(frameRegs), "r"(index));                                       \
    addr;                                                                    \
  }))
#endif
#define O1REG(name) REG(ip->i##name.op1)
#define O2REG(name) REG(ip->i##name.op2)
#define O3REG(name) REG(ip->i##name.op3)
#define O4REG(name) REG(ip->i##name.op4)
#define O5REG(name) REG(ip->i##name.op5)
#define O6REG(name) REG(ip->i##name.op6)

/// Get a StackFramePtr from the current frameRegs.
#define FRAME StackFramePtr(frameRegs - StackFrameLayout::FirstLocal)

/// Calculate the default property access flags depending on the mode of the
/// \c CodeBlock.
#define DEFAULT_PROP_OP_FLAGS(strictMode) \
  (strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags())

/// Map from a string ID encoded in the operand to a SymbolID.
/// This string ID must be used explicitly as identifier.
#define ID(stringID) \
  (curCodeBlock->getRuntimeModule()->getSymbolIDMustExist(stringID))

// Add an arbitrary byte offset to ip.
#define IPADD(val) ((const Inst *)((const uint8_t *)ip + (val)))

// Get the current bytecode offset.
#define CUROFFSET ((const uint8_t *)ip - (const uint8_t *)curCodeBlock->begin())

// Calculate the address of the next instruction given the name of the current
// one.
#define NEXTINST(name) ((const Inst *)(&ip->i##name + 1))

namespace hermes {
namespace vm {

inline int32_t doBitAnd(int32_t x, int32_t y) {
  return x & y;
}

inline int32_t doBitOr(int32_t x, int32_t y) {
  return x | y;
}

inline int32_t doBitXor(int32_t x, int32_t y) {
  return x ^ y;
}

inline int32_t doLShift(uint32_t x, uint32_t y) {
  return x << y;
}

inline int32_t doRShift(int32_t x, uint32_t y) {
  return x >> y;
}

inline uint32_t doURshift(uint32_t x, uint32_t y) {
  return x >> y;
}

/// ToIntegral maps the \param Oper shift operation (on Number) to the function
/// used to convert the operation's lhs operand to integer.
template <auto Oper>
inline int ToIntegral;

// For LShift, we need to use toUInt32 first because lshift on negative
// numbers is undefined behavior in theory.
template <>
inline constexpr auto &ToIntegral<doLShift> = toUInt32_RJS;

template <>
inline constexpr auto &ToIntegral<doRShift> = toInt32_RJS;

template <>
inline constexpr auto &ToIntegral<doURshift> = toUInt32_RJS;

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_INTERPRETER_INTERNAL_H
