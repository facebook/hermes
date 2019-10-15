/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// The x86-64 binary instruction emitter.
/// For convenience most definitions are defined inside the
/// hermes::vm::x86_64::detail namespace. The public ones are then explicitly
/// exported into hermes::vm::x86_64:
///   - Emitter
///   - CJumpOp
//===----------------------------------------------------------------------===//

#ifndef HERMES_VM_JIT_X86_64_EMITTER_H
#define HERMES_VM_JIT_X86_64_EMITTER_H

#include "hermes/Support/Compiler.h"

#include <cassert>

namespace hermes {
namespace vm {
namespace x86_64 {

/// A x86-64 architecture register. Note that many of these values overlap. They
/// are selected to make binary emission as efficient as possible. They cannot,
/// for example, be used to differentiate between integer and floating point
/// registers.
enum class Reg {
  rax = 0,
  eax = 0,
  ax = 0,
  al = 0,
  rcx = 1,
  ecx = 1,
  cx = 1,
  cl = 1,
  rdx = 2,
  edx = 2,
  dx = 2,
  dl = 2,
  rbx = 3,
  ebx = 3,
  bx = 3,
  bl = 3,
  rsp = 4,
  esp = 4,
  sp = 4,
  ah = 4,
  rbp = 5,
  ebp = 5,
  bp = 5,
  ch = 5,
  rsi = 6,
  esi = 6,
  si = 6,
  dh = 6,
  rdi = 7,
  edi = 7,
  di = 7,
  bh = 7,
  r8 = 8,
  r8d = 8,
  r8w = 8,
  r8b = 8,
  r9 = 9,
  r9d = 9,
  r9w = 9,
  r9b = 9,
  r10 = 10,
  r10d = 10,
  r11 = 11,
  r11d = 11,
  r12 = 12,
  r12d = 12,
  r13 = 13,
  r13d = 13,
  r14 = 14,
  r14d = 14,
  r15 = 15,
  r15d = 15,

  MM0 = 0,
  XMM0 = 0,
  MM1 = 1,
  XMM1 = 1,
  MM2 = 2,
  XMM2 = 2,
  MM3 = 3,
  XMM3 = 3,
  MM4 = 4,
  XMM4 = 4,
  MM5 = 5,
  XMM5 = 5,
  MM6 = 6,
  XMM6 = 6,
  MM7 = 7,
  XMM7 = 7,

  none = 0,

  /// Disable index in SIB.
  NoIndex = 4,

  /// Enable SIB byte in Mod.
  _ModSIB = 5,
};

/// A magic reserved value for scale, indicating direct register access.
constexpr unsigned ScaleRegAccess = 254;

/// A magic reserved value for scale, indicating RIP relative addressing.
constexpr unsigned ScaleRIPAddr32 = 255;

/// The of operand offset.
enum class OffsetType {
  Auto,
  Zero,
  Int8,
  Int32,
};

/// The width of a flowing point operand.
enum class FP {
  Float,
  Double,
};

/// Conditional codes.
enum class CCode {
  A = 0, // 77
  AE = 1, // 73
  B = 2, // 72
  BE = 3, // 76
  C = 2, // 72
  RCXZ = 4, // E3
  E = 5, // 74
  G = 6, // 7F
  GE = 7, // 7D
  L = 8, // 7C
  LE = 9, // 7E
  NA = 3, // 76
  NAE = 2, // 72
  NB = 1, // 73
  NBE = 0, // 77
  NC = 1, // 73
  NE = 10, // 75
  NG = 9, // 7E
  NGE = 8, // 7C
  NL = 7, // 7D
  NLE = 6, // 7F
  NO = 11, // 71
  NP = 12, // 7B
  NS = 13, // 79
  NZ = 10, // 75
  O = 14, // 70
  P = 15, // 7A
  PE = 15, // 7A
  PO = 12, // 7B
  S = 16, // 78
  Z = 5, // 74
};

/// Width of integer instruction operands.
enum class S {
  B,
  W,
  L,
  SLQ, // Sign extend Long to Quad
  Q,
};

namespace detail {

inline constexpr uint8_t ord(Reg reg) {
  return (uint8_t)reg;
}
inline constexpr uint8_t ord7(Reg reg) {
  return ord(reg) & 7;
}

/// 0mMM.00SS
///  m: Special case for [disp32]
/// MM: Mod bits
/// SS: 3 cases of SIB, starting from 1
enum class AddrMode {
  /// [EAX]           0000.0000
  AtReg = 0x00,
  /// [EBX]           0000.0001
  AtBase = 0x01,
  /// [EBX + ECX*2]   0000.0010
  AtBaseScaledIndex = 0x02,
  /// [ECX*2]         0000.0011
  AtScaledIndex = 0x03,
  /// [disp32]        0100.0000
  AtRIPDisp32 = 0x40,

  /// [EAX + disp8]   0001.0000
  AtRegDisp8 = 0x10,
  /// [EBX + disp8]   0001.0001
  AtBaseDisp8 = 0x11,
  /// [EBX + ECX*2 + disp8] 0001.0010
  AtBaseScaledIndexDisp8 = 0x12,
  /// [ECX*2 + disp8] 0001.0011
  AtScaledIndexDisp8 = 0x13,

  /// [EAX + disp32]  0010.0000
  AtRegDisp32 = 0x20,
  /// [EBX + disp32]  0010.0001
  AtBaseDisp32 = 0x21,
  /// [EBX + ECX*2 + disp32] 0010.0010
  AtBaseScaledIndexDisp32 = 0x22,
  /// [ECX*2 + disp32] 0010.0011
  AtScaledIndexDisp32 = 0x23,

  /// EAX             0011.0000
  Reg = 0x30,
};

#define _X86_64_EMIT_MAP(Base, KEY, VALUE) \
  template <>                              \
  struct Base<KEY> {                       \
    static constexpr VALUE;                \
  }

template <unsigned scale>
struct Log2Scale {};
_X86_64_EMIT_MAP(Log2Scale, 1, uint8_t N = 0);
_X86_64_EMIT_MAP(Log2Scale, 2, uint8_t N = 1);
_X86_64_EMIT_MAP(Log2Scale, 4, uint8_t N = 2);
_X86_64_EMIT_MAP(Log2Scale, 8, uint8_t N = 3);

template <CCode cc>
struct CJumpOp {};

_X86_64_EMIT_MAP(CJumpOp, CCode::A, uint8_t OP = 0x77);
_X86_64_EMIT_MAP(CJumpOp, CCode::AE, uint8_t OP = 0x73);
_X86_64_EMIT_MAP(CJumpOp, CCode::B, uint8_t OP = 0x72);
_X86_64_EMIT_MAP(CJumpOp, CCode::BE, uint8_t OP = 0x76);
_X86_64_EMIT_MAP(CJumpOp, CCode::RCXZ, uint8_t OP = 0xE3);
_X86_64_EMIT_MAP(CJumpOp, CCode::E, uint8_t OP = 0x74);
_X86_64_EMIT_MAP(CJumpOp, CCode::G, uint8_t OP = 0x7F);
_X86_64_EMIT_MAP(CJumpOp, CCode::GE, uint8_t OP = 0x7D);
_X86_64_EMIT_MAP(CJumpOp, CCode::L, uint8_t OP = 0x7C);
_X86_64_EMIT_MAP(CJumpOp, CCode::LE, uint8_t OP = 0x7E);
_X86_64_EMIT_MAP(CJumpOp, CCode::NE, uint8_t OP = 0x75);
_X86_64_EMIT_MAP(CJumpOp, CCode::NO, uint8_t OP = 0x71);
_X86_64_EMIT_MAP(CJumpOp, CCode::NP, uint8_t OP = 0x7B);
_X86_64_EMIT_MAP(CJumpOp, CCode::NS, uint8_t OP = 0x79);
_X86_64_EMIT_MAP(CJumpOp, CCode::O, uint8_t OP = 0x70);
_X86_64_EMIT_MAP(CJumpOp, CCode::P, uint8_t OP = 0x7A);
_X86_64_EMIT_MAP(CJumpOp, CCode::S, uint8_t OP = 0x78);

template <S s>
struct OperandType {};
template <>
struct OperandType<S::B> {
  using type = uint8_t;
};
template <>
struct OperandType<S::W> {
  using type = uint16_t;
};
template <>
struct OperandType<S::L> {
  using type = uint32_t;
};
template <>
struct OperandType<S::SLQ> {
  using type = int32_t;
};
template <>
struct OperandType<S::Q> {
  using type = uint64_t;
};

#undef _X86_64_EMIT_MAP

inline constexpr uint8_t ord(AddrMode mode) {
  return (uint8_t)mode;
}

inline constexpr uint8_t addrModeMod(AddrMode mode) {
  return (ord(mode) >> 4) & 7;
}

inline constexpr uint8_t addrModeSIB(AddrMode mode) {
  return ord(mode) & 3;
}

template <typename T>
inline constexpr bool isInt8(T x) {
  return (T)(int8_t)(uint8_t)x == x;
}
template <typename T>
inline constexpr bool isInt16(T x) {
  return (T)(int16_t)(uint16_t)x == x;
}
template <typename T>
inline constexpr bool isInt32(T x) {
  return (T)(int32_t)(uint32_t)x == x;
}

template <AddrMode AM>
inline constexpr uint8_t encodeModRM(uint8_t rm, uint8_t regOpCode) {
  // Mod:2 Reg/Opcode:3 R/M:3
  return (addrModeMod(AM) << 6) | ((regOpCode & 7) << 3) | (rm & 7);
}

/// If base == EBP, MOD bits determine the addressing:
///   MOD = 00 -> [scaled index] + disp32
///   MOD = 01 -> [scaled index] + disp8 + [EBP]
///   MOD = 00 -> [scaled index] + disp32 + [EBP]
template <unsigned scale>
inline constexpr uint8_t encodeSIB(Reg base, Reg index) {
  // Scale:2 Index:3 Base:3
  return (Log2Scale<scale>::N << 6) | (ord7(index) << 3) | ord7(base);
}

template <S s>
inline constexpr uint8_t encodeREX(uint8_t r, uint8_t x, uint8_t b) {
  return (s == S::Q || s == S::SLQ ? 0x48 : 0x40) | (r << 2) | (x << 1) | b;
}

template <S s>
void emitREX(uint8_t *&out, Reg rm, Reg sibIndex, uint8_t regOpCode) {
  uint8_t b = ord(rm) >> 3;
  uint8_t x = ord(sibIndex) >> 3;
  uint8_t r = regOpCode >> 3;
  if (s == S::Q || s == S::SLQ || b || x || r) {
    *out++ = encodeREX<s>(r, x, b);
  }
}

template <S s>
void emitOperandSizeOverride(uint8_t *&out) {
  if (s == S::W)
    *out++ = 0x66;
}

template <S s>
void emitAddressSizeOverride(uint8_t *&out) {
  if (s != S::Q)
    *out++ = 0x67;
}

template <typename T>
void emitConst(uint8_t *&out, T x) {
  *reinterpret_cast<T *>(out) = x;
  out += sizeof(T);
}

template <AddrMode AM, bool isSIB = addrModeSIB(AM)>
struct ModeSel {};

template <AddrMode AM>
struct ModeSel<AM, false> {
  static uint8_t modRM(Reg rm, uint8_t regOpCode) {
    return encodeModRM<AM>(ord7(rm), regOpCode & 7);
  }
};

template <>
struct ModeSel<AddrMode::AtRIPDisp32, false> {
  static constexpr auto AM = AddrMode::AtRIPDisp32;

  static uint8_t modRM(Reg rm, uint8_t regOpCode) {
    return encodeModRM<AM>(5, regOpCode & 7);
  }
};

template <AddrMode AM>
struct ModeSel<AM, true> {
  static uint8_t modRM(Reg rm, uint8_t regOpCode) {
    return encodeModRM<AM>(4, regOpCode & 7);
  }
};

template <S s, uint8_t opCode, unsigned scale>
struct EmitModRM {
  static void emitModRM(
      uint8_t *&out,
      Reg base,
      Reg index,
      int32_t rmOffset,
      uint8_t regOpCode) {
    if (rmOffset == 0) {
      *out++ = ModeSel<AddrMode::AtBase>::modRM(Reg::_ModSIB, regOpCode);
      *out++ = encodeSIB<scale>(base, index);
    } else if (isInt8(rmOffset)) {
      *out++ = ModeSel<AddrMode::AtBaseDisp8>::modRM(Reg::_ModSIB, regOpCode);
      *out++ = encodeSIB<scale>(base, index);
      emitConst(out, (int8_t)rmOffset);
    } else {
      *out++ = ModeSel<AddrMode::AtBaseDisp32>::modRM(Reg::_ModSIB, regOpCode);
      *out++ = encodeSIB<scale>(base, index);
      emitConst(out, rmOffset);
    }
  }

  static void emitFull(
      uint8_t *&out,
      Reg base,
      Reg index,
      int32_t rmOffset,
      uint8_t regOpCode) {
    emitREX<s>(out, base, index, regOpCode);
    *out++ = opCode;
    emitModRM(out, base, index, rmOffset, regOpCode);
  }
};

template <S s, uint8_t opCode>
struct EmitModRM<s, opCode, 0> {
  static void emitModRM(
      uint8_t *&out,
      Reg rm,
      Reg /*unused index*/,
      int32_t rmOffset,
      uint8_t regOpCode) {
    if (ord7(rm) == ord7(Reg::rsp)) {
      EmitModRM<s, 0, 1>::emitModRM(out, rm, Reg::NoIndex, rmOffset, regOpCode);
      return;
    }

    if (rmOffset == 0) {
      *out++ = ModeSel<AddrMode::AtReg>::modRM(rm, regOpCode);
    } else if (isInt8(rmOffset)) {
      *out++ = ModeSel<AddrMode::AtRegDisp8>::modRM(rm, regOpCode);
      emitConst(out, (int8_t)rmOffset);
    } else {
      *out++ = ModeSel<AddrMode::AtRegDisp32>::modRM(rm, regOpCode);
      emitConst(out, rmOffset);
    }
  }

  static void emitFull(
      uint8_t *&out,
      Reg rm,
      Reg index,
      int32_t rmOffset,
      uint8_t regOpCode) {
    emitREX<s>(out, rm, Reg::none, regOpCode);
    *out++ = opCode;
    emitModRM(out, rm, index, rmOffset, regOpCode);
  }
};

template <S s, uint8_t opCode>
struct EmitModRM<s, opCode, ScaleRegAccess> {
  static void emitModRM(
      uint8_t *&out,
      Reg rm,
      Reg /*unused index*/,
      int32_t /*unused rmOffset*/,
      uint8_t regOpCode) {
    // This is a special case.
    *out++ = ModeSel<AddrMode::Reg>::modRM(rm, regOpCode);
  }

  static void emitFull(
      uint8_t *&out,
      Reg rm,
      Reg /*unused index*/,
      int32_t /*unused rmOffset*/,
      uint8_t regOpCode) {
    emitREX<s>(out, rm, Reg::none, regOpCode);
    *out++ = opCode;
    emitModRM(out, rm, Reg::NoIndex, 0, regOpCode);
  }
};

template <S s, uint8_t opCode>
struct EmitModRM<s, opCode, ScaleRIPAddr32> {
  static void emitModRM(
      uint8_t *&out,
      Reg /*unused rm*/,
      Reg /*unused index*/,
      int32_t rmOffset,
      uint8_t regOpCode) {
    // This is a special case.
    *out++ = ModeSel<AddrMode::AtRIPDisp32>::modRM(Reg::_ModSIB, regOpCode);
    emitConst(out, rmOffset);
  }

  static void emitFull(
      uint8_t *&out,
      Reg rm,
      Reg index,
      int32_t rmOffset,
      uint8_t regOpCode) {
    emitREX<s>(out, rm, Reg::none, regOpCode);
    *out++ = opCode;
    emitModRM(out, rm, index, rmOffset, regOpCode);
  }
};

/// A light-weight templated x86-64 binary instruction emitter. It is
/// initialized with an output pointer and emits by incrementing it. It is the
/// caller's responsibility to check whether the output buffer has enough space.
/// The recommended approach is to check for a certain amount of avaialble bytes
/// before every instruction or before a group of instructions, knowing that
/// maximum instruction length is 15 (declared as a constant in the class).
///
/// This class is extremely cheap to create and destroy - it is trivially
/// copyable and doesn't have any state except the output pointer, which means
/// that it can be kept in a register and passed and returned from functions in
/// a register.
///
/// It is optimized for static code generation - in other words, aspects of the
/// instruction like the operand width or the addressing mode must be compile
/// time constants.
class Emitter {
 public:
  static constexpr unsigned MAX_INSTRUCTION_LENGTH = 15;

  explicit Emitter(uint8_t *buf) : out(buf) {}

  Emitter(const Emitter &) = default;
  Emitter &operator=(const Emitter &) = default;
  ~Emitter() = default;

  /// \return the current output pointer.
  uint8_t *current() const {
    return out;
  }

  /// Set the current output pointer.
  void setCurrent(uint8_t *buf) {
    out = buf;
  }

  template <uintptr_t x>
  void align() {
    out = reinterpret_cast<uint8_t *>(
        (reinterpret_cast<uintptr_t>(out) + (x - 1)) & ~(x - 1));
  }

  template <typename T>
  void numericConst(T x) {
    emitConst(out, x);
  }

  void pushqReg(Reg reg) {
    emitREX<S::L>(out, reg, Reg::none, 0);
    *out++ = 0x50 + ord7(reg);
  }
  void popqReg(Reg reg) {
    emitREX<S::L>(out, reg, Reg::none, 0);
    *out++ = 0x58 + ord7(reg);
  }

  template <unsigned scale = 0>
  void pushqRM(Reg srcBase, Reg srcIndex, int32_t srcOffset) {
    EmitModRM<S::Q, 0xFF, scale>::emitFull(
        out, srcBase, srcIndex, srcOffset, 6);
  }
  template <unsigned scale = 0>
  void popqRM(Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    EmitModRM<S::Q, 0x8F, scale>::emitFull(
        out, dstBase, dstIndex, dstOffset, 0);
  }

  template <S s>
  void movRegToReg(Reg src, Reg dst) {
    movRegToRM<s, ScaleRegAccess>(src, dst, Reg::NoIndex, 0);
  }
  template <S s, unsigned scale = 0>
  void movRegToRM(Reg src, Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    _opRegToRM<s, scale, 0x88>(src, dstBase, dstIndex, dstOffset);
  }
  template <S s, unsigned scale = 0>
  void movRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _opRMToReg<s, scale, 0x8A>(srcBase, srcIndex, srcOffset, dst);
  }

  template <S s>
  void movImmToReg(typename OperandType<s>::type imm, Reg reg) {
    static_assert(s != S::SLQ, "SLQ not supported");
    emitOperandSizeOverride<s>(out);
    emitREX<s>(out, reg, Reg::none, 0);
    *out++ = (s == S::B ? 0xB0 : 0xB8) + ord7(reg);
    emitConst(out, imm);
  }

  /// Mov a 64-bit immediate to 64-bit dest, but efficiently.
  template <unsigned scale = 0>
  void movqImmToReg(uint64_t imm, Reg dstReg) {
    if ((uint64_t)(int32_t)imm == imm) {
      emitREX<S::Q>(out, dstReg, Reg::NoIndex, 0);
      *out++ = 0xC7;
      *out++ = ModeSel<AddrMode::Reg>::modRM(dstReg, 0);
      emitConst(out, (int32_t)imm);
    } else {
      movImmToReg<S::Q>(imm, dstReg);
    }
  }

  template <S s, unsigned scale = 0>
  void movImmToRM(
      typename OperandType<s>::type imm,
      Reg dstBase,
      Reg dstIndex,
      int32_t dstOffset) {
    _opImmToRm<s, scale, 0xC6, 0>(imm, dstBase, dstIndex, dstOffset);
  }

  template <S s, S addressSize = s, unsigned scale = 0>
  void leaRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    static_assert(s != S::B && s != S::SLQ, "S::B and S::SLQ not supported");
    emitAddressSizeOverride<addressSize>(out);
    emitOperandSizeOverride<s>(out);
    EmitModRM<s, 0x8D, scale>::emitFull(
        out, srcBase, srcIndex, srcOffset, ord(dst));
  }

  template <S s, unsigned scale = 0>
  void cmpImmToRM(
      typename OperandType<s>::type imm,
      Reg dstBase,
      Reg dstIndex,
      int32_t dstOffset) {
    // Handle special cases for comparison to register.
    if (scale == ScaleRegAccess) {
      // If we are comparing an int8 value against a direct non-byte register,
      // use the 0x83 encoding.
      if (s != S::B && isInt8(imm)) {
        emitOperandSizeOverride<s>(out);
        EmitModRM<s, 0x83, scale>::emitFull(
            out, dstBase, dstIndex, dstOffset, 7);
        emitConst(out, (int8_t)imm);
        return;
      }
      if (s == S::B && dstBase == Reg::al && isInt8(imm)) {
        *out++ = 0x3C;
        emitConst(out, (int8_t)imm);
        return;
      }
      if (s == S::W && dstBase == Reg::ax && isInt16(imm)) {
        emitOperandSizeOverride<s>(out);
        *out++ = 0x3D;
        emitConst(out, (int16_t)imm);
        return;
      }
      if (s == S::L && dstBase == Reg::eax && isInt32(imm)) {
        *out++ = 0x3D;
        emitConst(out, (int32_t)imm);
        return;
      }
      if (s == S::SLQ && dstBase == Reg::rax && isInt32(imm)) {
        emitREX<S::Q>(out, Reg::rax, Reg::none, 0);
        *out++ = 0x3D;
        emitConst(out, (int32_t)imm);
        return;
      }
    }
    _opImmToRm<s, scale, 0x80, 7>(imm, dstBase, dstIndex, dstOffset);
  }

  template <S s, unsigned scale = 0>
  void testImmToRM(
      typename OperandType<s>::type imm,
      Reg dstBase,
      Reg dstIndex,
      int32_t dstOffset) {
    // Handle special cases for comparison to register.
    if (scale == ScaleRegAccess) {
      if (s == S::B && dstBase == Reg::al && isInt8(imm)) {
        *out++ = 0xA8;
        emitConst(out, (int8_t)imm);
        return;
      }
      if (s == S::W && dstBase == Reg::ax && isInt16(imm)) {
        emitOperandSizeOverride<s>(out);
        *out++ = 0xA9;
        emitConst(out, (int16_t)imm);
        return;
      }
      if (s == S::L && dstBase == Reg::eax && isInt32(imm)) {
        *out++ = 0xA9;
        emitConst(out, (int32_t)imm);
        return;
      }
      if (s == S::SLQ && dstBase == Reg::rax && isInt32(imm)) {
        emitREX<S::Q>(out, Reg::rax, Reg::none, 0);
        *out++ = 0xA9;
        emitConst(out, (int32_t)imm);
        return;
      }
    }

    _opImmToRm<s, scale, 0xF6, 0>(imm, dstBase, dstIndex, dstOffset);
  }

  template <S s, unsigned scale = 0>
  void testRmToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _opRMToReg<s, scale, 0x84>(srcBase, srcIndex, srcOffset, dst);
  };
  template <S s, unsigned scale = 0>
  void testRegToReg(Reg src, Reg dst) {
    testRmToReg<s, ScaleRegAccess>(dst, Reg::NoIndex, 0, src);
  };

  template <S s>
  void xorRegToReg(Reg src, Reg dst) {
    _opRegToRM<s, ScaleRegAccess, 0x30>(src, dst, Reg::NoIndex, 0);
  }
  // r/m64 AND imm32 sign extended to 64-bits if s = S::L and reg is 64 bits
  template <S s>
  void xorImmToReg(typename OperandType<s>::type imm, Reg reg) {
    if (s == S::B && reg == Reg::al && isInt8(imm)) {
      *out++ = 0x34;
      emitConst(out, (int8_t)imm);
      return;
    }
    if (s == S::W && reg == Reg::ax && isInt16(imm)) {
      emitOperandSizeOverride<s>(out);
      *out++ = 0x35;
      emitConst(out, (int16_t)imm);
      return;
    }
    if (s == S::L && reg == Reg::eax && isInt32(imm)) {
      *out++ = 0x35;
      emitConst(out, (int32_t)imm);
      return;
    }
    if (s == S::SLQ && reg == Reg::rax && isInt32(imm)) {
      emitREX<S::Q>(out, Reg::rax, Reg::none, 0);
      *out++ = 0x35;
      emitConst(out, (int32_t)imm);
      return;
    }
    _opImmToRm<s, ScaleRegAccess, 0x80, 6>(imm, reg, Reg::none, 0);
  }
  // sign-extended to imm8 if reg is 16, 32, or 64 bits
  template <S s = S::B>
  void xorImm8ToReg(typename OperandType<s>::type imm, Reg reg) {
    _opImmToRm<s, ScaleRegAccess, 0x83, 6>(imm, reg, Reg::none, 0);
  }
  template <S s, unsigned scale = 0>
  void xorImmToRM(
      typename OperandType<s>::type imm,
      Reg dstBase,
      Reg dstIndex,
      int32_t dstOffset) {
    _opImmToRm<s, scale, 0x80, 6>(imm, dstBase, dstIndex, dstOffset);
  }

  template <S s, unsigned scale = 0>
  void xorRmToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _opRMToReg<s, scale, 0x32>(srcBase, srcIndex, srcOffset, dst);
  }

  template <FP fp = FP::Double>
  void xorfpRegToReg(Reg src, Reg dst) {
    _dpfpRegToReg<fp, 0xEF>(src, dst);
  }

  template <S s, unsigned scale = 0>
  void orRmToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _opRMToReg<s, scale, 0x0a>(srcBase, srcIndex, srcOffset, dst);
  }
  template <S s>
  void orRegToReg(Reg src, Reg dst) {
    _opRegToRM<s, ScaleRegAccess, 0x08>(src, dst, Reg::NoIndex, 0);
  }
  // r/m64 AND imm32 sign extended to 64-bits if s = S::L and reg is 64 bits
  template <S s>
  void andImmToReg(typename OperandType<s>::type imm, Reg reg) {
    _opImmToRm<s, ScaleRegAccess, 0x80, 4>(imm, reg, Reg::none, 0);
  }

  // sign-extended to imm8 if reg is 16, 32, or 64 bits
  template <S s = S::B>
  void andImm8ToReg(typename OperandType<s>::type imm, Reg reg) {
    _opImmToRm<s, ScaleRegAccess, 0x83, 4>(imm, reg, Reg::none, 0);
  }

  /// One's Complement Negation
  template <S s>
  void notReg(Reg dst) {
    EmitModRM<s, s == S::B ? 0xF6 : 0xF7, ScaleRegAccess>::emitFull(
        out, dst, Reg::NoIndex, 0, 2);
  }

  /// unsigned shift \p reg to the right by \p imm bits
  void shrImm8ToReg(typename OperandType<S::B>::type imm, Reg reg) {
    emitREX<S::Q>(out, reg, Reg::none, 5);
    *out++ = 0xc1;
    *out++ = ModeSel<AddrMode::Reg>::modRM(reg, 5);
    emitConst(out, imm);
  }

  void retq() {
    *out++ = 0xc3;
  }

  void call(const uint8_t *target) {
    auto offset = target - out - 5;
    assert(isInt32(offset) && "operand must be 32-bit");
    *out++ = 0xE8;
    emitConst(out, (int32_t)offset);
  }

  template <unsigned scale = 0>
  void callRM(Reg base, Reg index, int32_t offset) {
    EmitModRM<S::L, 0xFF, scale>::emitFull(out, base, index, offset, 2);
  }
  void callReg(Reg dst) {
    EmitModRM<S::L, 0xFF, ScaleRegAccess>::emitFull(
        out, dst, Reg::NoIndex, 0, 2);
  }

  /// Emit a "jmp" instruction.
  /// \return the offset type: either Int8 or Int32.
  template <OffsetType OT>
  OffsetType jmp(const uint8_t *target) {
    if (OT == OffsetType::Auto) {
      if (isInt8(target - out - 2))
        return jmp<OffsetType::Int8>(target);
      else
        return jmp<OffsetType::Int32>(target);
    } else if (OT == OffsetType::Int8 || OT == OffsetType::Zero) {
      auto offset = target - out - 2;
      assert(isInt8(offset) && "operand must be 8-bit");
      *out++ = 0xEB;
      emitConst(out, (int8_t)offset);
      return OffsetType::Int8;
    } else {
      auto offset = target - out - 5;
      assert(isInt32(offset) && "operand must be 32-bit");
      *out++ = 0xE9;
      emitConst(out, (int32_t)offset);
      return OffsetType::Int32;
    }
  }

  template <unsigned scale = 0>
  void jmpRM(Reg base, Reg index, int32_t offset) {
    EmitModRM<S::L, 0xFF, scale>::emitFull(out, base, index, offset, 4);
  }

  /// Emit a conditional jmp instruction.
  /// \return the offset type: either Int8 or Int32.
  template <CCode cc, OffsetType OT>
  OffsetType cjump(const uint8_t *target) {
    if (OT == OffsetType::Auto) {
      if (isInt8(target - out - 2))
        return cjump<cc, OffsetType::Int8>(target);
      else
        return cjump<cc, OffsetType::Int32>(target);
    } else if (OT == OffsetType::Int8 || OT == OffsetType::Zero) {
      auto offset = target - out - 2;
      assert(isInt8(offset) && "operand must be 8-bit");
      *out++ = CJumpOp<cc>::OP;
      emitConst(out, (int8_t)offset);
      return OffsetType::Int8;
    } else {
      auto offset = target - out - 6;
      assert(isInt32(offset) && "operand must be 32-bit");
      *out++ = 0x0F;
      *out++ = CJumpOp<cc>::OP + 0x10;
      emitConst(out, (int32_t)offset);
      return OffsetType::Int32;
    }
  }

  /// Convert between a CCode and a conditional jump opCode at runtime.
  static uint8_t ccodeToOpCode(CCode cc) {
    static const uint8_t tab[16] = {
        CJumpOp<(CCode)0>::OP,
        CJumpOp<(CCode)1>::OP,
        CJumpOp<(CCode)2>::OP,
        CJumpOp<(CCode)3>::OP,
        CJumpOp<(CCode)4>::OP,
        CJumpOp<(CCode)5>::OP,
        CJumpOp<(CCode)6>::OP,
        CJumpOp<(CCode)7>::OP,
        CJumpOp<(CCode)8>::OP,
        CJumpOp<(CCode)9>::OP,
        CJumpOp<(CCode)10>::OP,
        CJumpOp<(CCode)11>::OP,
        CJumpOp<(CCode)12>::OP,
        CJumpOp<(CCode)13>::OP,
        CJumpOp<(CCode)14>::OP,
        CJumpOp<(CCode)15>::OP,
    };
    return tab[(unsigned)cc];
  }

  /// Conditional jump when the condition code is not known at compile time.
  /// \param opCode is the opcode corresponding to the conditional jump,
  ///   using the CJumpOp<cc>::OP mapping. See \c ccodeToOpCode(CCode).
  /// \return the offset type: either Int8 or Int32.
  template <OffsetType OT>
  void cjumpOP(uint8_t opCode, const uint8_t *target) {
    if (OT == OffsetType::Auto) {
      if (isInt8(target - out - 2))
        cjumpOP<OffsetType::Int8>(opCode, target);
      else
        cjumpOP<OffsetType::Int32>(opCode, target);
    } else if (OT == OffsetType::Int8 || OT == OffsetType::Zero) {
      auto offset = target - out - 2;
      assert(isInt8(offset) && "operand must be 8-bit");
      *out++ = opCode;
      emitConst(out, (int8_t)offset);
    } else {
      auto offset = target - out - 6;
      assert(isInt32(offset) && "operand must be 32-bit");
      *out++ = 0x0F;
      *out++ = opCode + 0x10;
      emitConst(out, (int32_t)offset);
    }
  }

  /// Emit a conditional set instruction.
  /// \param opCode conditional set opcode is always 0x20 bigger than
  /// conditional jmp for the same condition code in the CJumpOp<cc>::OP
  /// mapping. See \c ccodeToOpCode(CCode).
  void csetOP(uint8_t opCode, Reg dst) {
    opCode += 0x20;
    emitREX<S::B>(out, dst, Reg::none, 0);
    *out++ = 0x0F;
    *out++ = opCode;
    *out++ = ModeSel<AddrMode::Reg>::modRM(dst, ord(dst));
  }

  /// Emit a conditional set instruction.
  template <CCode cc>
  void csetOP(Reg dst) {
    emitREX<S::B>(out, dst, Reg::none, 0);
    *out++ = 0x0F;
    *out++ = CJumpOp<cc>::OP + 0x20;
    *out++ = ModeSel<AddrMode::Reg>::modRM(dst, ord(dst));
  }

  template <FP fp = FP::Double>
  void movfpRegToReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x10>(src, dst);
  }

  template <unsigned scale = 0, FP fp = FP::Double>
  void movfpRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fpRMToReg<scale, fp, 0x10>(srcBase, srcIndex, srcOffset, dst);
  }
  template <unsigned scale = 0, FP fp = FP::Double>
  void movfpRegToRM(Reg src, Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    _fpRegToRM<scale, fp, 0x11>(src, dstBase, dstIndex, dstOffset);
  }

  template <FP fp = FP::Double>
  void subfpRegFromReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x5C>(src, dst);
  }
  template <unsigned scale = 0, FP fp = FP::Double>
  void subfpRMFromReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fpRMToReg<scale, fp, 0x5C>(srcBase, srcIndex, srcOffset, dst);
  }
  template <FP fp = FP::Double>
  void divfpRegFromReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x5E>(src, dst);
  }
  template <unsigned scale = 0, FP fp = FP::Double>
  void divfpRMFromReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fpRMToReg<scale, fp, 0x5E>(srcBase, srcIndex, srcOffset, dst);
  }

  template <FP fp = FP::Double>
  void addfpRegToReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x58>(src, dst);
  }
  template <unsigned scale = 0, FP fp = FP::Double>
  void addfpRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fpRMToReg<scale, fp, 0x58>(srcBase, srcIndex, srcOffset, dst);
  }

  template <FP fp = FP::Double>
  void mulfpRegToReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x59>(src, dst);
  }
  template <unsigned scale = 0, FP fp = FP::Double>
  void mulfpRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fpRMToReg<scale, fp, 0x59>(srcBase, srcIndex, srcOffset, dst);
  }

  template <unsigned scale = 0, FP fp = FP::Double>
  void ucomisRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _dpfpRMToReg<scale, fp, 0x2E>(srcBase, srcIndex, srcOffset, dst);
  }

  template <FP fp = FP::Double>
  void ucomisRegToReg(Reg src, Reg dst) {
    _dpfpRegToReg<fp, 0x2E>(src, dst);
  }

  /// Convert with Truncation Scalar Double-Precision Floating-Point Value to
  /// Signed Integer
  template <FP fp = FP::Double>
  void cvttsd2siRegToReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x2C>(src, dst);
  }

  /// Convert Doubleword Integer to Scalar Double-Precision Floating-Point Value
  template <FP fp = FP::Double>
  void cvtsi2sdRegToReg(Reg src, Reg dst) {
    _fpRegToReg<fp, 0x2A>(src, dst);
  }

 private:
  uint8_t *out;

  template <FP fp>
  void _fptype() {
    *out++ = fp == FP::Double ? 0xF2 : 0xF3;
  }

  template <FP fp, uint8_t op>
  void _fpRegToReg(Reg src, Reg dst) {
    _fptype<fp>();
    *out++ = 0x0F;
    *out++ = op;
    *out++ = ModeSel<AddrMode::Reg>::modRM(src, ord(dst));
  }
  template <unsigned scale, FP fp, uint8_t op>
  void _fpRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    _fptype<fp>();
    emitREX<S::L>(out, srcBase, srcIndex, ord(dst));
    *out++ = 0x0F;
    *out++ = op;
    EmitModRM<S::L, 0, scale>::emitModRM(
        out, srcBase, srcIndex, srcOffset, ord(dst));
  }
  template <unsigned scale, FP fp, uint8_t op>
  void _fpRegToRM(Reg src, Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    _fptype<fp>();
    emitREX<S::L>(out, dstBase, dstIndex, ord(src));
    *out++ = 0x0F;
    *out++ = op;
    EmitModRM<S::L, 0, scale>::emitModRM(
        out, dstBase, dstIndex, dstOffset, ord(src));
  }

  template <FP fp, uint8_t op>
  void _dpfpRegToReg(Reg src, Reg dst) {
    if (fp == FP::Double)
      *out++ = 0x66;
    *out++ = 0x0F;
    *out++ = op;
    *out++ = ModeSel<AddrMode::Reg>::modRM(src, ord(dst));
  }
  template <unsigned scale, FP fp, uint8_t op>
  void _dpfpRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    if (fp == FP::Double)
      *out++ = 0x66;
    emitREX<S::L>(out, srcBase, srcIndex, ord(dst));
    *out++ = 0x0F;
    *out++ = op;
    EmitModRM<S::L, 0, scale>::emitModRM(
        out, srcBase, srcIndex, srcOffset, ord(dst));
  }
  template <unsigned scale, FP fp, uint8_t op>
  void _dpfpRegToRM(Reg src, Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    if (fp == FP::Double)
      *out++ = 0x66;
    emitREX<S::L>(out, dstBase, dstIndex, ord(src));
    *out++ = 0x0F;
    *out++ = op;
    EmitModRM<S::L, 0, scale>::emitModRM(
        out, dstBase, dstIndex, dstOffset, ord(src));
  }

  template <S s, unsigned scale, uint8_t opcode, uint8_t opcodeExtra>
  void _opImmToRm(
      typename OperandType<s>::type imm,
      Reg dstBase,
      Reg dstIndex,
      int32_t dstOffset) {
    static_assert(s != S::Q, "Q not supported");
    emitOperandSizeOverride<s>(out);
    EmitModRM<s, s == S::B ? opcode : opcode + 1, scale>::emitFull(
        out, dstBase, dstIndex, dstOffset, opcodeExtra);
    emitConst(out, imm);
  }

  template <S s, unsigned scale, uint8_t opCode>
  void _opRegToRM(Reg src, Reg dstBase, Reg dstIndex, int32_t dstOffset) {
    static_assert(s != S::SLQ, "SLQ not supported");
    emitOperandSizeOverride<s>(out);
    EmitModRM<s, s == S::B ? opCode : opCode + 1, scale>::emitFull(
        out, dstBase, dstIndex, dstOffset, ord(src));
  }

  template <S s, unsigned scale, uint8_t opCode>
  void _opRMToReg(Reg srcBase, Reg srcIndex, int32_t srcOffset, Reg dst) {
    static_assert(s != S::SLQ, "SLQ not supported");
    emitOperandSizeOverride<s>(out);
    EmitModRM<s, s == S::B ? opCode : opCode + 1, scale>::emitFull(
        out, srcBase, srcIndex, srcOffset, ord(dst));
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

static_assert(
    IsTriviallyCopyable<Emitter, true>::value,
    "Emitter must be trivially copyable");

} // namespace detail

using detail::CJumpOp;
using detail::Emitter;

} // namespace x86_64
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_X86_64_EMITTER_H
