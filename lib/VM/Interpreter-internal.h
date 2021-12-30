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

#endif // HERMES_VM_INTERPRETER_INTERNAL_H
