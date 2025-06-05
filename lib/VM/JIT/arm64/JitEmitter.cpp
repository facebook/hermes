/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitEmitter.h"
#include "JitImpl.h"

#include "JitHandlers.h"

#include "../RuntimeOffsets.h"
#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/BCGen/SerializedLiteralParser.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSObject-inline.h"
#include "hermes/VM/StaticHUtils.h"
#include "llvh/ADT/Statistic.h"

#define DEBUG_TYPE "jit"

STATISTIC(
    JITNumGetByIdSpec,
    "JITNumGetByIdSpec: number of GetById specialized fast paths emitted");

// Disable warnings about missing designated initializers since they occur often
// when we construct SlowPaths.
#ifdef __clang__
#if __has_warning("-Wmissing-designated-field-initializers")
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
#endif
#endif

#if defined(HERMESVM_COMPRESSED_POINTERS) && !defined(HERMESVM_CONTIGUOUS_HEAP)
#error JIT does not support non-contiguous heap with compressed pointers
#endif

namespace hermes::vm::arm64 {

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const hermes::vm::arm64::HWReg &hwReg) {
  if (hwReg.isValidGpX()) {
    os << "x" << (int)hwReg.indexInClass();
  } else if (hwReg.isValidVecD()) {
    os << "d" << (int)hwReg.indexInClass();
  } else {
    assert(!hwReg.isValid());
    os << "<invalid>";
  }
  return os;
}

namespace {

// Ensure that HermesValue tags are handled correctly by updating this every
// time the HERMESVALUE_VERSION changes, and going through the JIT and updating
// any relevant code.
static_assert(
    HERMESVALUE_VERSION == 2,
    "HermesValue version mismatch, JIT may need to be updated");

/// Get the tag for the HermesValue in \p xIn and place it in \p xOut.
/// xOut and xIn may be the same, but if they are, xIn will be clobbered.
void emit_sh_ljs_get_tag(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "kHV_NumDataBits is 48 and can be easily shifted");
  a.asr(xOut, xIn, kHV_NumDataBits);
}

/// Check whether \p xTagReg is the tag for a pointer value.
/// CPU flags are updated as result. b.hs success.
void emit_sh_ljs_tag_is_pointer(a64::Assembler &a, const a64::GpX &xTagReg) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "All tags above HVTag_FirstPointer are pointers");
  // The valid pointer tags are: 0xfc (FirstPointer) to 0xff (LastTag), but
  // all sign extended to 64 bits.
  // We need an unsigned comparison (tag >= 0xfc) to catch all pointers.
  // Doubles may have 0 in the tag bits, e.g. so we have to use
  // unsigned condition codes to make sure they don't get detected as pointers.
  a.cmn(xTagReg, -HVTag_FirstPointer);
}

/// Emit code to check whether the input reg is an object tag,
/// using the specified register.
/// The input reg is not modified.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_tag_is_object(a64::Assembler &a, const a64::GpX &xTagReg) {
  static_assert(
      (int16_t)HVTag_Object == (int16_t)(-1) && "HV_TagObject must be -1");
  a.cmn(xTagReg, -HVTag_Object);
}

/// Emit code to check whether the input reg is string tag, using the specified
/// register.
/// The input reg is not modified.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_tag_is_string(a64::Assembler &a, const a64::GpX &xTagReg) {
  static_assert(
      (int16_t)HVTag_Str == (int16_t)(-3) && "HV_TagObject must be -1");
  a.cmn(xTagReg, -HVTag_Str);
}

void emit_sh_ljs_get_pointer(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  // See:
  // https://dinfuehr.github.io/blog/encoding-of-immediate-values-on-aarch64/
  static_assert(
      HERMESVALUE_VERSION == 2,
      "kHV_DataMask is 0x000...1111... and can be encoded as a logical immediate");
  a.and_(xOut, xIn, kHV_DataMask);
}

/// Encode a native pointer into a tagged object pointer (SHLegacyValue).
/// The same register is used for input and output.
void emit_sh_ljs_object(a64::Assembler &a, const a64::GpX &inOut) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "HVTag_Object << kHV_NumDataBits is 0x1111...0000... and can be encoded as a logical immediate");
  a.movk(inOut, (uint16_t)HVTag_Object, kHV_NumDataBits);
}

/// Encode a native pointer into a tagged object pointer (SHLegacyValue).
/// Takes an input and output register, which can be the same.
/// In some sense this supersedes
void emit_sh_ljs_object2(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "HVTag_Object << kHV_NumDataBits is 0x1111...0000... and can be encoded as a logical immediate");
  a.orr(xOut, xIn, (uint64_t)HVTag_Object << kHV_NumDataBits);
}

/// Emit code to check whether the input HermesValue is a double.
/// The input reg is not modified.
/// The temp reg is modified.
/// CPU flags are updated as result. b.lo on success.
void emit_sh_ljs_is_double(
    a64::Assembler &a,
    const a64::GpX &xInput,
    const a64::GpX &xTmp) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "numbers must be lower than HVTag_First << kHV_NumDataBits");
  a.mov(xTmp, ((uint64_t)HVTag_First << kHV_NumDataBits));
  a.cmp(xInput, xTmp);
}

/// Encode a string pointer into a tagged string (SHLegacyValue).
/// The same register is used for input and output.
void emit_sh_ljs_string(a64::Assembler &a, const a64::GpX &inOut) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "HVTag_Str << kHV_NumDataBits is 0x1101...0000... and can be encoded as a logical immediate");
  a.movk(inOut, (uint16_t)HVTag_Str, kHV_NumDataBits);
}

/// Encode a string pointer into a tagged string (SmallHermesValue).
/// The same register is used for input and output.
void emit_shv_string(a64::Assembler &a, const a64::GpX &inOut) {
#ifdef HERMESVM_BOXED_DOUBLES
  static_assert(
      SmallHermesValue::kVersion == 1, "String tagging requires simple or");
  a.orr(inOut, inOut, HermesValue32::Tag::String);
#else
  emit_sh_ljs_string(a, inOut);
#endif
}

/// Emit code to check whether the input reg is an object, using the specified
/// temp register. The input reg is not
/// modified unless it is the same as the temp, which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_object(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  emit_sh_ljs_get_tag(a, xTempReg, xInputReg);
  emit_sh_ljs_tag_is_object(a, xTempReg);
}

/// Emit code to check whether the input HermesValue is a string,
/// using the specified temp register. The input reg is not
/// modified unless it is the same as the temp, which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_string(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  emit_sh_ljs_get_tag(a, xTempReg, xInputReg);
  emit_sh_ljs_tag_is_string(a, xTempReg);
}

/// Emit code to check whether the input reg is a bigint, using the specified
/// temp register. The input reg is not
/// modified unless it is the same as the temp, which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_bigint(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      (int16_t)HVTag_BigInt == (int16_t)(-2) && "HVTag_BigInt must be -2");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits);
  a.cmn(xTempReg, -HVTag_BigInt);
}

/// Emit code to check whether the input reg is empty, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_empty(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      (int16_t)HVETag_Empty == (int16_t)(-14) && "HVETag_Empty must be -14");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Empty);
}

/// Emit code to check whether the input reg is null, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_null(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      (int16_t)HVETag_Null == (int16_t)(-11) && "HVETag_Null must be -11");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Null);
}

/// Emit code to check whether the input reg is bool, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_bool(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      (int16_t)HVETag_Bool == (int16_t)(-10) && "HVETag_Bool must be -10");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Bool);
}

/// Emit code to check whether the input reg is undefined, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_undefined(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      HERMESVALUE_VERSION == 2,
      "HVETag_Undefined must be at kHV_NumDataBits - 1");
  static_assert(
      (int16_t)HVETag_Undefined == (int16_t)(-12) &&
      "HVETag_Undefined must be -12");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Undefined);
}

/// Emit code to check whether the input reg is Symbol, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_symbol(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      HERMESVALUE_VERSION == 2, "HVETag_Symbol must be at kHV_NumDataBits - 1");
  static_assert(
      (int16_t)HVETag_Symbol == (int16_t)(-9) && "HVETag_Symbol must be -9");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Symbol);
}

/// For a register \p inOut that contains a bool (i.e. either 0 or 1), turn it
/// into a HermesValue boolean by adding the corresponding tag.
void emit_sh_ljs_bool(a64::Assembler &a, const a64::GpX inOut) {
  static constexpr SHLegacyValue baseBool = HermesValue::encodeBoolValue(false);
  // We know that the ETag for bool has a 0 in its lowest bit, and is therefore
  // a shifted 16 bit value. We can exploit this to use movk to set the tag.
  static_assert(HERMESVALUE_VERSION == 2);
  static_assert(
      (llvh::isShiftedUInt<16, kHV_NumDataBits>(baseBool.raw)) &&
      "Boolean tag must be 16 bits.");
  a.lsl(inOut, inOut, kHV_BoolBitIdx);
  // Add the bool tag.
  a.movk(inOut, baseBool.raw >> kHV_NumDataBits, kHV_NumDataBits);
}

/// Store the HermesValue for the bool \p val into \p out.
void emit_sh_ljs_bool_const(a64::Assembler &a, const a64::GpX &out, bool val) {
  static constexpr SHLegacyValue baseBool = HermesValue::encodeBoolValue(false);
  // We know that the ETag for bool has a 0 in its lowest bit, and is therefore
  // a shifted 16 bit value. We can exploit this to use movk to set the tag.
  static_assert(HERMESVALUE_VERSION == 2);
  static_assert(
      (llvh::isShiftedUInt<16, kHV_NumDataBits>(baseBool.raw)) &&
      "Boolean tag must be 16 bits.");
  if (val) {
    // Put 1 at the value and add the bool tag.
    a.mov(out, (uint64_t)1 << kHV_BoolBitIdx);
    a.movk(out, baseBool.raw >> kHV_NumDataBits, kHV_NumDataBits);
  } else {
    a.mov(out, baseBool.raw);
  }
}

/// Maximum natural offset (+1) that can be encoded in a load/store instruction
/// for a given operand width. The instruction contains a 12-bit unsigned
/// offset which is multiplied by the size of the load, so the maximum offset is
/// 2^12 * \c width.
///
/// https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
/// https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDRH--immediate---Load-register-halfword--immediate--?lang=en
/// https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDRB--immediate---Load-register-byte--immediate--?lang=en
constexpr inline uint32_t maxNaturalBaseOffset(unsigned width) {
  return (1 << 12) * width;
}

/// Maximum offset (+1) that can be encoded in a load instruction, i.e. can be
/// loaded by \c emit_load_from_base_offset() without requiring an extra
/// register.
constexpr uint32_t kMaxInlineBaseOffset = maxNaturalBaseOffset(8);

/// Load a value with the specified width from the base register with an
/// unsigned immediate offset. The wrapper is necessary since the immediate
/// offset for loads is limited to 12-bits and may not always be sufficient, in
/// which case we need to use alternate techniques. The base and destination
/// registers must not be the same, since in rare cases we need to use the
/// destination register as scratch.
///
/// \param WIDTH The width of the load, in bytes. Must be one of 1, 2, 4, or 8.
/// \param INLINE_LOAD If true, the offset must be less than or equal to
///   \c kMaxInlineBaseOffset, and the load will always be emitted without
///   needing a temporary register.
/// \param destReg The destination register to load the value into.
/// \param baseReg The base register to calculate the load address.
/// \param tmpReg A temporary register that must be different from \p baseReg,
///   but could be the same as \p destReg. This is only needed when
///   \c INLINE_LOAD is false, pass {} otherwise.
/// \param offset The offset to add to the base register. Must be aligned to
///   WIDTH.
template <unsigned WIDTH, bool INLINE_LOAD = false>
void emit_load_from_base_offset(
    a64::Assembler &a,
    const a64::GpX &destReg,
    const a64::GpX &baseReg,
    const a64::GpX &tmpReg,
    uint32_t offset) {
  static_assert(
      WIDTH == 1 || WIDTH == 2 || WIDTH == 4 || WIDTH == 8,
      "Unsupported width for load");
  assert(
      (INLINE_LOAD || tmpReg.isValid()) &&
      "Temporary register must be valid when INLINE_LOAD is false");
  assert(
      (INLINE_LOAD || tmpReg != baseReg) &&
      "tmpReg must not be the same as baseReg when INLINE_LOAD is false");
  // Assert that the offset is aligned to the load width.
  assert(offset % WIDTH == 0 && "Offset must be aligned to the load width");

  // Maximum offset (+1) that can be used when loading a value with this width.
  constexpr uint32_t kMaxNaturalOffset = maxNaturalBaseOffset(WIDTH);

  if (offset < kMaxNaturalOffset) {
    // The offset fits in the natural range for the load width.
    if constexpr (WIDTH == 1) {
      a.ldrb(destReg.w(), a64::Mem(baseReg, offset));
    } else if constexpr (WIDTH == 2) {
      a.ldrh(destReg.w(), a64::Mem(baseReg, offset));
    } else if constexpr (WIDTH == 4) {
      a.ldr(destReg.w(), a64::Mem(baseReg, offset));
    } else {
      a.ldr(destReg, a64::Mem(baseReg, offset));
    }
  } else if (offset < kMaxInlineBaseOffset) {
    // The offset fits in the range for a 64-bit load.
    a.ldr(destReg, a64::Mem(baseReg, offset & ~(uint32_t)7));
    a.ubfx(destReg, destReg, (offset & 7) * 8, WIDTH * 8);
  } else if constexpr (!INLINE_LOAD) {
    // We must perform an explicit addition.
    a.mov(tmpReg, offset / WIDTH);
    if constexpr (WIDTH == 1) {
      a.ldrb(destReg.w(), a64::Mem(baseReg, tmpReg));
    } else if constexpr (WIDTH == 2) {
      a.ldrh(
          destReg.w(),
          a64::Mem(baseReg, tmpReg, a64::Shift(a64::ShiftOp::kLSL, 1)));
    } else if constexpr (WIDTH == 4) {
      a.ldr(
          destReg.w(),
          a64::Mem(baseReg, tmpReg, a64::Shift(a64::ShiftOp::kLSL, 2)));
    } else {
      a.ldr(
          destReg,
          a64::Mem(baseReg, tmpReg, a64::Shift(a64::ShiftOp::kLSL, 3)));
    }
  } else {
    a.reportError(
        asmjit::kErrorInvalidImmediate,
        "emit_load_from_base_offset(): offset is too large for INLINE_LOAD=true");
  }
}

/// For a register containing a pointer to a GCCell, retrieve its CellKind (a
/// single byte) and store it in \p wOut.
/// \p wOut and \p xIn may refer to the same register.
void emit_gccell_get_kind(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  emit_load_from_base_offset<1, true>(
      a,
      xOut,
      xIn,
      {},
      offsetof(SHGCCell, kindAndSize) + RuntimeOffsets::kindAndSizeKind);
}

/// For a register \p wIn that contains a CellKind, check whether it falls
/// within the kind range [first, last].
/// The \p wInput is not modified unless it is the same as \p wTemp, which is
/// allowed.
/// CPU flags are updated as result. b_ls on success, or b_hi on failure.
void emit_cellkind_in_range(
    a64::Assembler &a,
    const a64::GpW &wTemp,
    const a64::GpW &wInput,
    CellKind first,
    CellKind last) {
  a.sub(wTemp, wInput, first);
  a.cmp(wTemp, (uint32_t)last - (uint32_t)first);
}

/// For a register \p dInput, which contains a double, check whether it is a
/// valid signed 64-bit integer.
/// CPU flags are updated. b_eq on success.
/// If successful, \p xTemp will contain the number converted to int,
/// and \p dTemp will contain the same number as \p dInput.
/// \pre dTemp != dInput, because both are used in the comparison.
void emit_double_is_int(
    a64::Assembler &a,
    const a64::GpX &xTemp,
    const a64::VecD &dTemp,
    const a64::VecD &dInput) {
  assert(dTemp != dInput && "must use a different temp");

  // Convert the operand to a signed 64 bit integer.
  a.fcvtzs(xTemp, dInput);
  // Sign extend it from the second-to-last bit. This is necessary because
  // fcvtzs is saturating and will convert the double 2^63 to 2^63 - 1, which
  // will get converted back to 2^63 by scvtf. They will therefore incorrectly
  // compare equal after truncation.
  a.sbfx(xTemp, xTemp, 0, 63);
  // Convert back to a double and see if they compare equal.
  a.scvtf(dTemp, xTemp);
  a.fcmp(dTemp, dInput);
}

/// For a register \p dInput, which contains a double, check whether it is a
/// valid unsigned 32-bit integer.
/// CPU flags are updated. b_eq on success.
/// If successful, \p wTemp will contain the number converted to int,
/// and \p dTemp will contain the same number as \p dInput.
/// \pre dTemp != dInput, because both are used in the comparison.
void emit_double_is_uint32(
    a64::Assembler &a,
    const a64::GpW &wTemp,
    const a64::VecD &dTemp,
    const a64::VecD &dInput) {
  assert(dTemp != dInput && "must use a different temp");
  a.fcvtzu(wTemp, dInput);
  a.ucvtf(dTemp, wTemp);
  a.fcmp(dTemp, dInput);
}

/// Given a compressed pointer in \p xInOut, decompress it and place the result
/// in \p xInOut.
void emit_sh_cp_decode(a64::Assembler &a, const a64::GpX &xInOut) {
#ifdef HERMESVM_COMPRESSED_POINTERS
  a.cmp(xInOut, 0);
  a.add(xInOut, xRuntime, xInOut);
  a.csel(xInOut, a64::xzr, xInOut, a64::CondCode::kEQ);
#endif
}

/// Given a compressed pointer in \p xInOut that is known to be non-null,
/// decompress it and place the result in \p xInOut.
void emit_sh_cp_decode_non_null(a64::Assembler &a, const a64::GpX &xInOut) {
#ifdef HERMESVM_COMPRESSED_POINTERS
  a.add(xInOut, xRuntime, xInOut);
#endif
}

/// Given a pointer in \p xInOut that is known to be non-null, compress it and
/// place the result in \p xInOut.
void emit_sh_cp_encode_non_null(a64::Assembler &a, const a64::GpX &xInOut) {
#ifdef HERMESVM_COMPRESSED_POINTERS
  a.sub(xInOut, xInOut, xRuntime);
#endif
}

/// Similar to \c emit_sh_cp_decode_non_null(), but used when the input register
/// needs to be preserved. It returns the result register which must then be
/// used by the caller.
/// The result register is either \p xIn or \p xMayBeRes.
[[nodiscard]] const a64::GpX &emit_sh_cp_decode_non_null_preserve_input(
    a64::Assembler &a,
    const a64::GpX &xMayBeRes,
    const a64::GpX &xIn) {
  if constexpr (sizeof(SmallHermesValue) == 4) {
    a.add(xMayBeRes, xRuntime, xIn);
    return xMayBeRes;
  } else {
    return xIn;
  }
}

/// Load a compressed pointer from \p mem.
void emit_load_cp(
    a64::Assembler &a,
    const a64::GpX &dest,
    const a64::Mem &mem) {
#ifdef HERMESVM_COMPRESSED_POINTERS
  a.ldr(dest.w(), mem);
#else
  a.ldr(dest, mem);
#endif
}

/// Store the compressed pointer in \p val to \p mem.
void emit_store_cp(
    a64::Assembler &a,
    const a64::GpX &val,
    const a64::Mem &mem) {
  if constexpr (sizeof(CompressedPointer) == 4)
    a.str(val.w(), mem);
  else
    a.str(val, mem);
}

/// Load a SmallHermesValue from \p mem.
void emit_load_shv(
    a64::Assembler &a,
    const a64::GpX &dest,
    const a64::Mem &mem) {
  if constexpr (sizeof(SmallHermesValue) == 4) {
    a.ldr(dest.w(), mem);
  } else {
    a.ldr(dest, mem);
  }
}

/// Store a SmallHermesValue to \p mem.
asmjit::Error
emit_store_shv(a64::Assembler &a, const a64::GpX &val, const a64::Mem &mem) {
  if constexpr (sizeof(SmallHermesValue) == 4) {
    return a.str(val.w(), mem);
  } else {
    return a.str(val, mem);
  }
}

/// Emit a comparison with an immediate.
void emit_cmp_imm32(
    a64::Assembler &a,
    const a64::GpW &wReg,
    uint32_t imm32,
    const a64::GpW &wTmpReg) {
  if (a64::Utils::isAddSubImm(imm32)) {
    a.cmp(wReg, imm32);
  } else {
    a.mov(wTmpReg, imm32);
    a.cmp(wReg, wTmpReg);
  }
}

/// Load the slot16 from a cache entry.
/// \param xResReg The register to load the slot16 into.
/// \param xPropCacheReg The register containing the pointer to the start of
///   the read property cache, which must not be the same as \p xResReg.
/// \param cacheIdx The index of the cache entry to load.
void emit_load_slot16(
    a64::Assembler &a,
    const a64::GpX &xResReg,
    const a64::GpX &xPropCacheReg,
    uint8_t cacheIdx) {
  assert(
      xPropCacheReg != xResReg &&
      "xPropCacheReg must not be the same as xResReg");
  emit_load_from_base_offset<2>(
      a,
      xResReg,
      xPropCacheReg,
      xResReg,
      sizeof(SHReadPropertyCacheEntry) * cacheIdx +
          offsetof(SHReadPropertyCacheEntry, _slot16));
}

/// A class implementing SmallHermesValue to HermesValue decoding.
/// Given a SmallHermesValue in \p xInOut, decompress it and place the result in
/// \p xInOut. Jump to \p doneLab if decoding is done early (but not in the
/// fall-through case).
///
/// To facilitate sharing the decoding without an extra branch, the code is
/// split into two parts: the first case and the rest of the cases. The first
/// case checks its condition: if it doesn't match, it jumps to the rest of the
/// cases. If it matches, it performs the decode and falls through.
/// The intent is that the "rest cases" will be shared, while the first case
/// may be emitted multiple times to save a branch.
class Emit_sh_shv_decode {
#ifdef HERMESVM_BOXED_DOUBLES
  const a64::GpX xInOut;
  asmjit::Label ptrLab;
  const asmjit::Label &doneLab;
#endif
#ifndef NDEBUG
  bool restEmitted = false;
#endif

 public:
  explicit Emit_sh_shv_decode(
      a64::Assembler &a,
      const a64::GpX &xInOut,
      const asmjit::Label &doneLab)
#ifdef HERMESVM_BOXED_DOUBLES
      : xInOut(xInOut), doneLab(doneLab) {
    ptrLab = a.newLabel();
  }
#else
  {
  }
#endif

  /// Emit the code checking and decoding the first case of SHV values. Can be
  /// invoked multiple times. Emitted code looks like:
  /// \code
  ///    check
  ///    b.cond ptrLab
  ///    decode
  /// \endcode
  ///
  /// Note that it simply falls through on success.
  void emitFirstCase(a64::Assembler &a);

  /// Emit the code checking and decoding all other cases of SHV values. Can
  /// only be invoked once. Emitted code at high level looks like:
  /// \code
  ///    ptrLab:
  ///      check_ptr
  ///      b.cond otherLab
  ///      decode
  ///      b doneLab
  ///    otherLab:
  ///      decode
  /// \endcode
  ///
  /// Note that the last case does not branch to doneLab, so the caller must
  /// do it (or fall through to doneLab).
  void emitRestCases(a64::Assembler &a);

  /// Emit the first case, followed by a branch to doneLab, and the rest of the
  /// cases.
  void emitAll(a64::Assembler &a) {
    emitFirstCase(a);
#ifdef HERMESVM_BOXED_DOUBLES
    // emitFirstCase() falls through, so add an explicit branch to doneLab.
    a.b(doneLab);
#endif
    emitRestCases(a);
  }
};

void Emit_sh_shv_decode::emitFirstCase(a64::Assembler &a) {
#ifdef HERMESVM_BOXED_DOUBLES
  static_assert(HERMESVALUE_VERSION == 2, "Constructing HermesValue from bits");
  static_assert(HermesValue32::kVersion == 1, "Decoding HV32 bits");
  constexpr uint64_t kHV32TagMask = (1 << HermesValue32::kNumTagBits) - 1;

  // Start by testing the tag bits for 0, if so, this is a "compressed HV64".
  static_assert((int)HermesValue32::Tag::CompressedHV64 == 0);
  a.tst(xInOut, kHV32TagMask);
  // If the tag is non-zero, jump to the pointer check.
  a.b_ne(ptrLab);
  // CompressedHV64, just shift the bits if needed. Note the case where
  // HermesValue32 is actually 64 bits is only used for testing, so it is not
  // worth optimising here.
  if constexpr (sizeof(SmallHermesValue) < sizeof(HermesValue))
    a.lsl(xInOut, xInOut, 32);
#endif
}

void Emit_sh_shv_decode::emitRestCases(a64::Assembler &a) {
#ifndef NDEBUG
  assert(!restEmitted && "Rest cases already emitted");
  restEmitted = true;
#endif
#ifdef HERMESVM_BOXED_DOUBLES
  asmjit::Label symLab = a.newLabel();
  asmjit::Label bdLab = a.newLabel();

  static_assert(HERMESVALUE_VERSION == 2, "Constructing HermesValue from bits");
  static_assert(HermesValue32::kVersion == 1, "Decoding HV32 bits");
  constexpr uint64_t kHV32TagMask = (1 << HermesValue32::kNumTagBits) - 1;

  a.bind(ptrLab);

  // See the comments below for why the exact bits are important.
  static_assert((int)HermesValue32::Tag::String == 0b01);
  static_assert((int)HermesValue32::Tag::BigInt == 0b10);
  static_assert((int)HermesValue32::Tag::Object == 0b11);
  // We know that the tag is 3 bits, and we have already checked for 0, so we
  // can check for the 3 interesting pointer tags (which are in the range 1-3,
  // i.e., less than 2^2) just by testing the most significant bit of the tag.
  a.tbnz(xInOut, 2, symLab);
  // We can decompress the pointer right away, since the HV32 representation
  // just makes it look like an unaligned compressed pointer.
  emit_sh_cp_decode_non_null(a, xInOut);
  // Now we insert the new tag. First compute the "base" tag. The HV64 tag for a
  // pointer can be computed by adding the HV32 tag to this base.
  constexpr uint16_t baseTag =
      (uint16_t)HermesValue::Tag::Str - (uint16_t)HermesValue32::Tag::String;
  // Move in the base tag.
  a.movk(xInOut, baseTag, kHV_NumDataBits);

  // We know that the base tag has zeroes in its low 2 bits, and the HV32 tag is
  // known to be in the low 2 bits at this point. So we can use bfi to simply
  // copy those low bits over.
  static_assert((baseTag & 0b11) == 0);
  a.bfi(xInOut, xInOut, kHV_NumDataBits, 2);

  // Clear the HV32 tag in the low bits.
  a.and_(xInOut, xInOut, ~kHV32TagMask);
  a.b(doneLab);

  a.bind(symLab);
  // There are only two cases left, Symbol and BoxedDouble. We can distinguish
  // them with a single bit.
  static_assert((int)HermesValue32::Tag::BoxedDouble == 0b100);
  static_assert((int)HermesValue32::Tag::Symbol == 0b101);
  static_assert((int)HermesValue32::Tag::_Last == 0b110);
  // Test the low bit, to distinguish Symbol and BoxedDouble.
  a.tbz(xInOut, 0, bdLab);
  // SymbolID has a 17-bit tag in HV64, which means the tag cannot be inserted
  // in a single instruction. However, we can exploit the fact that HV64 tags
  // are particular double NaN's, with a sign bit of 1 and all-1 exponents, so
  // they start with 0xfff. This means that the most significant bits of the tag
  // are 1, and we can shift in those bits with an asr below as we shift out the
  // HV32 tag. First, insert the low 14 bits of the symbol tag as a 16 bit
  // immediate.
  constexpr uint16_t symTag =
      (uint16_t)((uint32_t)HVETag_Symbol << (HermesValue32::kNumTagBits - 1));
  // Assert that the top bits are preserved with an arithmetic right shift.
  static_assert(
      ((int16_t)symTag) >> (HermesValue32::kNumTagBits - 1) == HVETag_Symbol);
  a.movk(xInOut, symTag, kHV_NumDataBits);
  // Next, shift in the high 1's as we shift out the HV32 tag.
  a.asr(xInOut, xInOut, HermesValue32::kNumTagBits);
  a.b(doneLab);

  a.bind(bdLab);
  // It is a boxed double. As with pointers, we can decode it as a compressed
  // pointer first.
  emit_sh_cp_decode_non_null(a, xInOut);
  // Since the tag is still present, subtract it from the offset.
  constexpr size_t bdOffs = RuntimeOffsets::boxedDoubleValue -
      (size_t)HermesValue32::Tag::BoxedDouble;
  a.ldr(xInOut, a64::Mem(xInOut, bdOffs));
#endif
}

/// Given a SmallHermesValue in \p xInOut, decompress it into a HermesValue and
/// place the result in \p xInOut. Jump to \p doneLab if decoding is done early
/// (but not in the fall-through case).
void emit_sh_shv_decode(
    a64::Assembler &a,
    const a64::GpX &xInOut,
    const asmjit::Label &doneLab) {
  Emit_sh_shv_decode(a, xInOut, doneLab).emitAll(a);
}

/// Load the lengthAndFlags of a HermesValue that contains a StringPrimitive
/// into \p xOut.
/// xOut and xIn may be the same, but xIn will be clobbered.
/// \param xOut the output register for the length, which will be placed
///  in xOut.w().
/// \param xIn the input register containing the string HermesValue.
void emit_stringprim_get_length_and_flags(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  emit_sh_ljs_get_pointer(a, xOut, xIn);
  a.ldr(
      xOut.w(), a64::Mem(xOut, RuntimeOffsets::stringPrimitiveLengthAndFlags));
}

/// Emit code to initialize the fields on a JSObject.
/// \param xObj contains a pointer to the object to initialise.
/// \param xParent contains a compressed pointer to the parent object.
/// \param xTempOrPropStorageOpt is a temporary register. It may contain
///   a pointer to the PropStorage. If \p HasPropStorage is true, it's
///   used to initialize the PropStorage, otherwise it's used as a temporary
///   register.
///   Either way, the value in xTempOrPropStorageOpt WILL be overwritten.
/// \param xClazzOpt if invalid will use the default JSObject HiddenClass,
///   otherwise a compressed pointer to the HiddenClass of the new object.
void emit_jsobject_init(
    a64::Assembler &a,
    const a64::GpX &xObj,
    const a64::GpX &xParent,
    const a64::GpX &xTempOrPropStorageOpt,
    bool hasPropStorage,
    const a64::GpX &xClazzOpt = a64::GpX{}) {
  // obj->flags = 0
  a.str(a64::wzr, a64::Mem(xObj, offsetof(SHJSObject, flags)));

  if (hasPropStorage) {
    emit_sh_cp_encode_non_null(a, xTempOrPropStorageOpt);
    emit_store_cp(
        a,
        xTempOrPropStorageOpt,
        a64::Mem(xObj, offsetof(SHJSObject, propStorage)));
  }

  // No longer need the propStorage pointer. Alias for clarity.
  const a64::GpX &xTemp = xTempOrPropStorageOpt;

  if (!xClazzOpt.isValid()) {
    // Load the hidden class compressed pointer into xClazz.
    static_assert(
        JSObject::numOverlapSlots<JSObject>() == 0,
        "Cannot use 0 property root class.");
    a.ldr(xTemp, a64::Mem(xRuntime, RuntimeOffsets::runtimeRootClazzes));
    emit_sh_ljs_get_pointer(a, xTemp, xTemp);
    emit_sh_cp_encode_non_null(a, xTemp);
  }

  const a64::GpX &xClazz = xClazzOpt.isValid() ? xClazzOpt : xTemp;

  // Store the parent and hidden class.
  // obj->parent = xParent
  // obj->clazz = xClazz (may be the same as xTemp).
  assert(xClazz.isValid());
  static_assert(
      offsetof(SHJSObject, clazz) - offsetof(SHJSObject, parent) ==
          sizeof(CompressedPointer),
      "clazz and parent must be adjacent to use stp");
  if constexpr (sizeof(CompressedPointer) == 4)
    a.stp(
        xParent.w(), xClazz.w(), a64::Mem(xObj, offsetof(SHJSObject, parent)));
  else
    a.stp(xParent, xClazz, a64::Mem(xObj, offsetof(SHJSObject, parent)));

  // If !hasPropStorage, obj->propStorage = nullptr.

  // obj->directProps[N] = SmallHermesValue::encodeRawZeroValue()

  // We want to zero the rest of the object. To simplify things, we align the
  // size to the heap alignment of 8 bytes, which ensures that the end of the
  // fill region is aligned to 8 bytes.
  // Start the fill at propStorage if HasPropStorage is false,
  // otherwise start after propStorage in the directProps.
  size_t startZeroOffset = hasPropStorage
      ? offsetof(SHJSObjectAndDirectProps, directProps)
      : offsetof(SHJSObject, propStorage);
  size_t bytesToZero = heapAlignSize(cellSize<JSObject>()) - startZeroOffset;
  size_t zeroedBytes = 0;
  assert(bytesToZero % 4 == 0 && "Must be a multiple of 4");

  // If there is some amount that is not a multiple of 8, store that first.
  if (bytesToZero & 4) {
    a.str(a64::wzr, a64::Mem(xObj, startZeroOffset));
    zeroedBytes += 4;
  }

  // Now store any amount that is not a multiple of 16. Note that since the end
  // of the fill region is aligned to 8 bytes, we know all further stores will
  // be aligned to 8 bytes.
  if (bytesToZero & 8) {
    a.str(a64::xzr, a64::Mem(xObj, startZeroOffset + zeroedBytes));
    zeroedBytes += 8;
  }
  // Store the rest as multiples of 16.
  for (; zeroedBytes < bytesToZero; zeroedBytes += 16) {
    a.stp(a64::xzr, a64::xzr, a64::Mem(xObj, startZeroOffset + zeroedBytes));
  }
  assert(zeroedBytes == bytesToZero && "Did not zero the whole object");
}

/// Emit code to initialize the fields of a newly created environment.
/// \param xNewEnvPtr contains a pointer to the object to initialise.
/// \param xParentEnv contains a compressed pointer to the parent environment.
/// \param xTemp is a temporary register for use by the emitted code.
/// \param vTemp is a temporary vector register for use by the emitted code.
/// \param size is the number of slots in the new environment.
void emit_environment_init(
    a64::Assembler &a,
    const a64::GpX &xNewEnvPtr,
    const a64::GpX &xParentEnv,
    const a64::GpX &xTemp,
    const a64::VecV &vTemp,
    uint32_t size) {
  // Store the size and parent to the first two fields.
  a.mov(xTemp, size);
  if constexpr (sizeof(CompressedPointer) == 4) {
    a.stp(
        xParentEnv.w(),
        xTemp.w(),
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, parentEnvironment)));
  } else {
    a.str(
        xParentEnv,
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, parentEnvironment)));
    a.str(xTemp.w(), a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, size)));
  }

  // Load undefined.
  a.mov(xTemp, _sh_ljs_undefined().raw);

  auto slotsToFill = size;
  // Before we fill 4 at a time, make sure any excess slots are filled.
  if (slotsToFill % 2) {
    a.str(xTemp, a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, slots)));
    --slotsToFill;
  }
  if (slotsToFill % 4) {
    auto slotOffs = sizeof(SHLegacyValue) * (size - slotsToFill);
    a.stp(
        xTemp,
        xTemp,
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, slots) + slotOffs));
    slotsToFill -= 2;
  }

  // The remaining number of slots must be a multiple of 4, which we can fill 4
  // at a time.
  assert((slotsToFill % 4) == 0 && "Remaining slots must be multiple of 4");
  if (slotsToFill) {
    // Copy the value into a vector register, so we can fill faster.
    a.dup(vTemp.d2(), xTemp);

    auto slotOffs = sizeof(SHLegacyValue) * (size - slotsToFill);
    // Initialize the pointer to the slots.
    a.add(xTemp, xNewEnvPtr, offsetof(SHEnvironment, slots) + slotOffs);

    // Fill the slots with undefined 4 at a time.
    for (; slotsToFill; slotsToFill -= 4)
      a.stp(vTemp, vTemp, a64::Mem(xTemp).post(32));
  }
  assert(slotsToFill == 0 && "All slots must be filled");
}

/// \return true if i is a valid immediate offset to use in an stp instruction
/// storing two GpX registers, false otherwise. Note that the limits are
/// different if storing GpW registers, or vector registers.
bool isStpGpXImm(int i) {
  // These restrictions are from:
  // https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions/STP--Store-pair-of-registers-
  return (i % 8 == 0) && i <= 504 && i >= -512;
}

class OurErrorHandler : public asmjit::ErrorHandler {
  asmjit::Error &expectedError_;
  std::function<void(std::string &&message)> const longjmpError_;

 public:
  /// \param expectedError if we get an error matching this value, we ignore it.
  explicit OurErrorHandler(
      asmjit::Error &expectedError,
      const std::function<void(std::string &&message)> &longjmpError)
      : expectedError_(expectedError), longjmpError_(longjmpError) {}

  void handleError(
      asmjit::Error err,
      const char *message,
      asmjit::BaseEmitter *origin) override {
    if (err == expectedError_) {
      LLVM_DEBUG(
          llvh::outs() << "Expected AsmJit error: " << err << ": "
                       << asmjit::DebugUtils::errorAsString(err) << ": "
                       << message << "\n");
      return;
    }

    std::string formattedMsg{};
    {
      // Ensure we run any destructors for the ostream before longjmp.
      llvh::raw_string_ostream OS{formattedMsg};
      OS << "AsmJit error: " << err << ": "
         << asmjit::DebugUtils::errorAsString(err) << ": " << message;
      OS.flush();
    }

    // IMPORTANT: From here on, we MUST ensure that no destructors need to run.
    // One exception: formattedMsg will have its destructor skipped, but we're
    // moving out of it so in practice the std::string won't have anything to
    // free, avoiding leaks.
    LLVM_DEBUG(llvh::dbgs() << formattedMsg << "\n");
    longjmpError_(std::move(formattedMsg));
  }
};

/// Helper function to load a pointer to the builtin closure with index
/// \p builtinIndex and place it in \p xRes.
void emit_load_builtin_closure(
    a64::Assembler &a,
    const a64::GpX &xRes,
    uint32_t builtinIndex) {
  static_assert(
      std::is_same_v<
          TransparentOwningPtr<Callable *, llvh::FreeDeleter>,
          RuntimeOffsets::BuiltinsType>,
      "builtins_ is a list of Callable *");
  static_assert(
      offsetof(TransparentOwningPtr<Callable *>, ptr) == 0,
      "TransparentOwningPtr must be transparent");

  a.ldr(xRes, a64::Mem(xRuntime, RuntimeOffsets::builtins));
  a.ldr(xRes, a64::Mem(xRes, builtinIndex * sizeof(Callable *)));
}

/// This macro is used to catch and handle low probability instructing encoding
/// errors - i.e. when an immediate operand doesn't fit in the instruction
/// encoding. It causes Asmjit to just return an error code instead of
/// terminating the entire compilation.
///
/// \param expValue the error value that we want to handle.
/// \param code  C++ code to invoke asmjit and store the result in a variable.
#define EXPECT_ERROR(expValue, code)          \
  do {                                        \
    assert(                                   \
        expectedError_ == asmjit::kErrorOk && \
        "expectedError_ is not cleared");     \
    expectedError_ = (expValue);              \
    code;                                     \
    expectedError_ = asmjit::kErrorOk;        \
  } while (0)

#ifndef ASMJIT_NO_LOGGING
class OurLogger : public asmjit::Logger {
 private:
  a64::Assembler &a_;
  PerfJitDump *perfJitDump_{nullptr};
  bool dumpJitCode_{false};

 public:
  OurLogger(a64::Assembler &a, PerfJitDump *perfJitDump, bool dumpJitCode)
      : a_(a), perfJitDump_(perfJitDump), dumpJitCode_(dumpJitCode) {}

  ASMJIT_API asmjit::Error _log(const char *data, size_t size) noexcept
      override {
    auto str =
        (size == SIZE_MAX ? llvh::StringRef(data)
                          : llvh::StringRef(data, size));
    if (str.empty())
      return asmjit::kErrorOk;
    if (dumpJitCode_)
      llvh::outs() << str;
    if (!perfJitDump_)
      return asmjit::kErrorOk;

    // Comments by default do not have indentation, except some pseudocode
    // comments, which start with ';' after indentation.
    auto trimmed = str.ltrim();
    if (str.front() != ' ' || (!trimmed.empty() && trimmed.front() == ';')) {
      perfJitDump_->addCodeComment(str, a_.offset());
    }
    return asmjit::kErrorOk;
  }
};
#endif

} // unnamed namespace

/// Return true if the specified 64-bit value can be efficiently loaded on
/// Arm64 with up to two integer instructions. In other words, it has at most
/// two non-zero 16-bit words.
static bool isCheapConst(uint64_t k) {
  unsigned count = 0;
  for (uint64_t mask = 0xFFFF; mask != 0; mask <<= 16) {
    if (k & mask)
      ++count;
  }
  return count <= 2;
}

/// Save the current IP and emit a call to a runtime function. This should be
/// used in most cases when invoking slow paths and handlers for complex
/// functionality.
#define EMIT_RUNTIME_CALL(em, type, func)           \
  do {                                              \
    using _FnT = type;                              \
    _FnT _fn = func;                                \
    (void)_fn;                                      \
    (em).callThunkWithSavedIP((void *)func, #func); \
  } while (0)

/// Call a runtime function without saving the IP. This is intended for special
/// cases where we want to preserve the currently saved IP or if the IP is not
/// needed.
#define EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(em, type, func) \
  do {                                                     \
    using _FnT = type;                                     \
    _FnT _fn = func;                                       \
    (void)_fn;                                             \
    (em).callThunk((void *)func, #func);                   \
  } while (0)

/// Make call without creating a thunk for it or saving the IP. This is useful
/// for specific functionality where saving the IP is either unnecessary or
/// incorrect, and where the call target is only going to be invoked at most
/// once in the function.
#define EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(em, type, func) \
  do {                                                               \
    using _FnT = type;                                               \
    _FnT _fn = func;                                                 \
    (void)_fn;                                                       \
    (em).callWithoutThunk((void *)func, #func);                      \
  } while (0)

Emitter::Emitter(
    Runtime &runtime,
    JITContext::Impl &jitImpl,
    unsigned dumpJitCode,
    bool emitAsserts,
    bool emitCounters,
    PerfJitDump *perfJitDump,
    CodeBlock *codeBlock,
    const std::function<void(std::string &&message)> &longjmpError)
    : runtime_(runtime),
      jitImpl_(jitImpl),
      dumpJitCode_(dumpJitCode),
      emitAsserts_(emitAsserts),
      emitCounters_(emitCounters),
      frameRegs_(codeBlock->getFrameSize()),
      codeBlock_(codeBlock) {
  errorHandler_ = std::unique_ptr<asmjit::ErrorHandler>(
      new OurErrorHandler(expectedError_, longjmpError));

  code.init(jitImpl.jr.environment(), jitImpl.jr.cpuFeatures());
  code.setErrorHandler(errorHandler_.get());

#ifndef ASMJIT_NO_LOGGING
  if ((dumpJitCode_ & DumpJitCode::Code) || perfJitDump) {
    logger_ = std::unique_ptr<asmjit::Logger>(
        new OurLogger(a, perfJitDump, dumpJitCode_));
    logger_->setIndentation(asmjit::FormatIndentationGroup::kCode, 4);
    logger_->addFlags(asmjit::FormatFlags::kHexImms);
    code.setLogger(logger_.get());
  }
#endif

  code.attach(&a);

  roDataLabel_ = a.newNamedLabel("RO_DATA");
  returnLabel_ = a.newNamedLabel("leave");

  // Save read/write property cache addresses.
  roOfsReadPropertyCachePtr_ = uint64Const(
      (uint64_t)codeBlock->readPropertyCache(), "readPropertyCache");
  roOfsWritePropertyCachePtr_ = uint64Const(
      (uint64_t)codeBlock->writePropertyCache(), "writePropertyCache");
  roOfsPrivateNameCachePtr_ =
      uint64Const((uint64_t)codeBlock->privateNameCache(), "privateNameCache");
}

void Emitter::enter(uint32_t numCount, uint32_t npCount) {
  unsigned nextVec = kVecSaved.first;
  unsigned nextGp = kGPSaved.first;

  // Number registers: allocate in vector hw regs first.
  for (unsigned frIndex = 0; frIndex < numCount; ++frIndex) {
    HWReg hwReg;
    if (nextVec <= kVecSaved.second) {
      hwReg = HWReg::vecD(nextVec);
      comment("    ; alloc: d%u <= r%u", nextVec, frIndex);
      ++nextVec;
    } else if (nextGp <= kGPSaved.second) {
      hwReg = HWReg::gpX(nextGp);
      comment("    ; alloc: x%u <= r%u", nextGp, frIndex);
      ++nextGp;
    } else
      break;

    frameRegs_[frIndex].globalReg = hwReg;
    frameRegs_[frIndex].globalType = FRType::Number;
  }
  // Non-pointer regs: allocate in gp regs first.
  for (unsigned frIndex = numCount; frIndex < npCount + numCount; ++frIndex) {
    HWReg hwReg;
    if (nextGp <= kGPSaved.second) {
      hwReg = HWReg::gpX(nextGp);
      comment("    ; alloc: x%u <= r%u", nextGp, frIndex);
      ++nextGp;
    } else if (nextVec <= kVecSaved.second) {
      hwReg = HWReg::vecD(nextVec);
      comment("    ; alloc: d%u <= r%u", nextVec, frIndex);
      ++nextVec;
    } else
      break;

    frameRegs_[frIndex].globalReg = hwReg;
    frameRegs_[frIndex].globalType = FRType::UnknownNonPtr;
  }

  if (!codeBlock_->getRuntimeModule()
           ->getBytecode()
           ->getExceptionTable(codeBlock_->getFunctionID())
           .empty())
    catchTableLabel_ = a.newNamedLabel("CATCH_TABLE");

  frameSetup(
      frameRegs_.size(), nextGp - kGPSaved.first, nextVec - kVecSaved.first);
}

void Emitter::comment(const char *fmt, ...) {
  if (!hasLogger())
    return;
  va_list args;
  va_start(args, fmt);
  char buf[80];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  a.comment(buf);
}

JITCompiledFunctionPtr Emitter::addToRuntime(asmjit::JitRuntime &jr) {
  code.detach(&a);
  JITCompiledFunctionPtr fn;
  asmjit::Error err = jr.add(&fn, &code);
  if (err) {
    llvh::errs() << "AsmJit failed: " << asmjit::DebugUtils::errorAsString(err)
                 << "\n";
    hermes::hermes_fatal("AsmJit failed");
  }
  return fn;
}

#ifndef NDEBUG
void Emitter::assertPostInstructionInvariants() {
  for (const auto &frState : frameRegs_)
    assert(!frState.regIsDirty && "Frame register is dirty");

  // Check that any temps have an associated FR.
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(gpTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }

  for (unsigned i = kVecTemp1.first; i <= kVecTemp2.second; ++i) {
    if (i > kVecTemp1.second && i < kVecTemp2.first)
      continue;
    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(vecTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }
}
#endif

void Emitter::newBasicBlock(const asmjit::Label &label) {
  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  // Clear all local types and regs when starting a new basic block.
  // TODO: there must be a faster way to do this when there are many regs.
  for (FRState &frState : frameRegs_) {
    frState.localType = frState.globalType;
    assert(!frState.localGpX);
    assert(!frState.localVecD);
    if (frState.globalReg)
      frState.frameUpToDate = false;
  }

  a.bind(label);
}

int32_t Emitter::getDebugFunctionName() {
  if (roOfsDebugFunctionName_ < 0) {
    std::string str;
    llvh::raw_string_ostream ss(str);
    ss << codeBlock_->getFunctionID() << "(" << codeBlock_->getNameString()
       << ")";
    ss.flush();
    int32_t size = str.size() + 1;
    roOfsDebugFunctionName_ = reserveData(size, 1, asmjit::TypeId::kInt8, size);
    memcpy(roData_.data() + roOfsDebugFunctionName_, str.data(), size);
  }
  return roOfsDebugFunctionName_;
}

void Emitter::frameSetup(
    unsigned numFrameRegs,
    unsigned gpSaveCount,
    unsigned vecSaveCount) {
  assert(
      gpSaveCount <= kGPSaved.second - kGPSaved.first + 1 &&
      "Too many callee saved GP regs");
  assert(
      vecSaveCount <= kVecSaved.second - kVecSaved.first + 1 &&
      "Too many callee saved Vec regs");

  static_assert(
      kGPSaved.first == 21, "Callee saved GP regs must start from x21");
  // Always save x21 even if it is not needed for an FR because we use it for
  // the return value.
  if (gpSaveCount == 0)
    gpSaveCount = 1;
  // We always save x19 and x20 since they are used for xRuntime and xFrame.
  gpSaveCount += 2;

  gpSaveCount_ = gpSaveCount;
  vecSaveCount_ = vecSaveCount;

  // Higher addresses are at the top.
  // +-----------------------------+<---- old sp
  // |             x30             |
  // +-----------------------------+
  // |             x29             |
  // +-----------------------------+<---- new x29
  // |             ...             |
  // +-----------------------------+
  // |             x21             |
  // +-----------------------------+
  // |             x20             |
  // +-----------------------------+
  // |             x19             |
  // +-----------------------------+
  // |  Saved SHLocals* (optional) |
  // +-----------------------------+
  // |      SHJmpBuf (optional)    |
  // +-----------------------------+<--- new sp
  a.sub(a64::sp, a64::sp, getStackSize());

  unsigned stackOfs = getSavedRegsOffset();
  for (unsigned i = 0; i < gpSaveCount; i += 2, stackOfs += 16) {
    if (i + 1 < gpSaveCount)
      a.stp(a64::GpX(19 + i), a64::GpX(20 + i), a64::Mem(a64::sp, stackOfs));
    else
      a.str(a64::GpX(19 + i), a64::Mem(a64::sp, stackOfs));
  }
  for (unsigned i = 0; i < vecSaveCount; i += 2, stackOfs += 16) {
    if (i + 1 < vecSaveCount)
      a.stp(
          a64::VecD(kVecSaved.first + i),
          a64::VecD(kVecSaved.first + 1 + i),
          a64::Mem(a64::sp, stackOfs));
    else
      a.str(a64::VecD(kVecSaved.first + i), a64::Mem(a64::sp, stackOfs));
  }
  a.stp(a64::x29, a64::x30, a64::Mem(a64::sp, stackOfs));
  a.add(a64::x29, a64::sp, stackOfs);

  comment("// xRuntime");
  a.mov(xRuntime, a64::x0);

  // Save the SHLocals pointer because we don't allocate and push a new
  // SHLocals in the JIT.
  // Used in CatchInst to restore state.
  if (catchTableLabel_.isValid()) {
    comment("// saved SHLocals *");
    a.ldr(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::shLocals));
    a.str(a64::x0, a64::Mem(a64::sp, getSavedSHLocalsOffset()));
  }

#ifndef HERMES_CHECK_NATIVE_STACK
#error Only native stack checking is supported in the JIT
#endif

  comment("// _sh_check_native_stack_overflow");
  asmjit::Label nativeOverflowLab = newSlowPathLabel();
  asmjit::Label nativeOverflowContLab = newContLabel();
  // Get the stack bounds from the runtime.
  a.ldr(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::nativeStackHigh));
  a.ldr(a64::x1, a64::Mem(xRuntime, RuntimeOffsets::nativeStackSize));
  // Subtract the frame pointer from nativeStackHigh and compare it against the
  // size. If the difference is less than the stack size, then we are still
  // within the current stack bounds.
  a.sub(a64::x0, a64::x0, a64::x29);
  a.cmp(a64::x0, a64::x1);
  // If the frame pointer is within bounds, we are done. Otherwise, we need to
  // check if the bounds have changed.
  a.b_hi(nativeOverflowLab);
  a.bind(nativeOverflowContLab);
  slowPaths_.push_back(
      {.slowPathLab = nativeOverflowLab,
       .contLab = nativeOverflowContLab,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: _sh_check_native_stack_overflow");
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         // Do not save the IP because we have not yet set up the stack frame
         // for this function. If this throws, the exception should appear in
         // the caller.
         EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
             em, void (*)(SHRuntime *), _sh_check_native_stack_overflow);
         em.a.b(sl.contLab);
       }});

  comment("// xFrame");
  a.ldr(xFrame, a64::Mem(xRuntime, RuntimeOffsets::stackPointer));

  // If the function has a prohibitInvoke flag, we need to check if it has been
  // called correctly.
  auto prohibitInvoke = codeBlock_->getHeaderFlags().getProhibitInvoke();
  if (prohibitInvoke != ProhibitInvoke::None) {
    // Load new.target.
    a.ldur(
        a64::x0,
        a64::Mem(
            xFrame, StackFrameLayout::NewTarget * (int)sizeof(SHLegacyValue)));
    // Compare new.target against undefined.
    emit_sh_ljs_is_undefined(a, a64::x0, a64::x0);

    void (*slowCall)(SHRuntime *);
    const char *slowCallName;
    asmjit::Label throwInvalidInvokeLab;
    if (prohibitInvoke == ProhibitInvoke::Call) {
      // If regular calls are prohibited, then we jump to throwInvalidInvoke if
      // new.target is undefined.
      throwInvalidInvokeLab = a.newNamedLabel("throwInvalidCall");
      a.b_eq(throwInvalidInvokeLab);

      slowCall = _sh_throw_invalid_call;
      slowCallName = "_sh_throw_invalid_call";
    } else {
      assert(
          prohibitInvoke == ProhibitInvoke::Construct &&
          "Unknown prohibitInvoke");
      // If construct calls are prohibited, then we jump to throwInvalidInvoke
      // if new.target is not undefined.
      throwInvalidInvokeLab = a.newNamedLabel("throwInvalidConstruct");
      a.b_ne(throwInvalidInvokeLab);

      slowCall = _sh_throw_invalid_construct;
      slowCallName = "_sh_throw_invalid_construct";
    }

    slowPaths_.push_back(
        {.slowPathLab = throwInvalidInvokeLab,
         .slowCall = (void *)slowCall,
         .slowCallName = slowCallName,
         .emit = [](Emitter &em, SlowPath &sl) {
           em.comment("// Slow path: %s", sl.slowCallName);
           em.a.bind(sl.slowPathLab);
           em.a.mov(a64::x0, xRuntime);
           // We don't register a thunk since there will only be a single call
           // to this. Note that we also don't save the IP, because this is
           // being thrown in the caller's context.
           em.callWithoutThunk(sl.slowCall, sl.slowCallName);
           // Function does not return.
         }});
  }

  // NOTE: Unlike _sh_enter, we do not push an SHLocals object.
  //  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 13);
  comment("// _sh_enter");
  asmjit::Label registerOverflowLab = newSlowPathLabel();

  // Compute the remaining available stack space:
  // runtime.registerStackEnd_ - runtime.stackPointer_
  a.ldr(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::registerStackEnd));
  a.sub(a64::x0, a64::x0, xFrame);
  // Check if we need more registers than remain.
  size_t totalRegsToAlloc = numFrameRegs + hbc::StackFrameLayout::FirstLocal;
  size_t regAllocSize = totalRegsToAlloc * sizeof(SHLegacyValue);
  // NOTE: cmp has the same immediate field type as add/sub, so we can use the
  // same utility function.
  if (a64::Utils::isAddSubImm(regAllocSize)) {
    a.cmp(a64::x0, regAllocSize);
    a.b_lo(registerOverflowLab);
    a.add(a64::x0, xFrame, regAllocSize);
  } else {
    a.mov(a64::x1, regAllocSize);
    a.cmp(a64::x0, a64::x1);
    a.b_lo(registerOverflowLab);
    a.add(a64::x0, xFrame, a64::x1);
  }

  // Advance the register stack.
  a.str(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::stackPointer));
  a.str(xFrame, a64::Mem(xRuntime, RuntimeOffsets::currentFrame));

  static_assert(
      HERMESVALUE_VERSION == 2, "Raw zero value must be ignored by GC");
  // Initialize the pointer to the current set of registers.
  a.mov(a64::x0, xFrame);
  size_t regsToFill = totalRegsToAlloc;
  // Fill the registers with zero in groups of 4, then 2, then 1.
  // If there are more than 32 registers, start with a loop.
  if (regsToFill > 32) {
    a.movi(a64::v0.d2(), 0);
    // We will fill 4 registers on each iteration.
    unsigned loopBytes = llvh::alignDown(regsToFill, 4) * sizeof(SHLegacyValue);
    // Initialize the loop limit in x1.
    if (a64::Utils::isAddSubImm(loopBytes)) {
      a.add(a64::x1, a64::x0, loopBytes);
    } else {
      a.mov(a64::x1, loopBytes);
      a.add(a64::x1, a64::x0, a64::x1);
    }
    asmjit::Label loop = a.newLabel();
    a.bind(loop);
    // Loop until we reach the limit.
    a.stp(a64::v0, a64::v0, a64::Mem(a64::x0).post(32));
    a.cmp(a64::x0, a64::x1);
    a.b_lo(loop);

    regsToFill %= 4;
  } else if (regsToFill >= 4) {
    a.movi(a64::v0.d2(), 0);
    // If the number of registers is small, just fill them directly.
    while (regsToFill >= 4) {
      a.stp(a64::v0, a64::v0, a64::Mem(a64::x0).post(32));
      regsToFill -= 4;
    }
  }
  // Fill any excess registers.
  if (regsToFill >= 2) {
    a.stp(a64::xzr, a64::xzr, a64::Mem(a64::x0).post(16));
    regsToFill -= 2;
  }
  if (regsToFill > 0) {
    assert(regsToFill == 1 && "All regs must be filled");
    a.str(a64::xzr, a64::Mem(a64::x0));
  }

  // Create the slow path for throwing a register stack overflow.
  slowPaths_.push_back(
      {.slowPathLab = registerOverflowLab,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: _sh_throw_register_stack_overflow");
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         // Do not save the IP because we have not yet set up the stack frame
         // for this function. The exception should appear in the caller.
         EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
             em, void (*)(SHRuntime *), _sh_throw_register_stack_overflow);
       }});

  if (catchTableLabel_.isValid()) {
    comment("// _sh_try");
    uint32_t jmpBufOffset = getJmpBufOffset();
    // buf->prev = shr->shCurJmpBuf;
    a.ldr(a64::x0, a64::Mem(xRuntime, offsetof(SHRuntime, shCurJmpBuf)));
    a.str(a64::x0, a64::Mem(a64::sp, jmpBufOffset + offsetof(SHJmpBuf, prev)));

    // shr->shCurJmpBuf = buf;
    a.add(a64::x0, a64::sp, jmpBufOffset);
    a.str(a64::x0, a64::Mem(xRuntime, offsetof(SHRuntime, shCurJmpBuf)));

    // _setjmp(buf->buf);
    a.add(a64::x0, a64::sp, jmpBufOffset + offsetof(SHJmpBuf, buf));
    // setjmp can't throw and it'll be called once, so don't use a thunk.
    EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
        *this, int (*)(jmp_buf), _sh_setjmp);
    // If this a catch, go to the catch table to jump to either a handler BB or
    // rethrow.
    a.cbnz(a64::x0, catchTableLabel_);
  }

  if (dumpJitCode_ & DumpJitCode::EntryExit) {
    comment("// print entry");
    a.mov(a64::w0, 1);
    a.adr(a64::x1, roDataLabel_);
    a.add(a64::x1, a64::x1, getDebugFunctionName());
    EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
        *this, void (*)(bool, const char *), _sh_print_function_entry_exit);
  }
}

void Emitter::leave(llvh::ArrayRef<const asmjit::Label *> exceptionHandlers) {
  comment("// leaveFrame");
  a.bind(returnLabel_);
  if (dumpJitCode_ & DumpJitCode::EntryExit) {
    comment("// print exit");
    a.mov(a64::w0, 0);
    a.adr(a64::x1, roDataLabel_);
    a.add(a64::x1, a64::x1, getDebugFunctionName());
    EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
        *this, void (*)(bool, const char *), _sh_print_function_entry_exit);
  }

  if (catchTableLabel_.isValid()) {
    comment("// _sh_end_try");
    // shr->shCurJmpBuf = buf->prev
    uint32_t jmpBufOffset = getJmpBufOffset();
    a.ldr(a64::x0, a64::Mem(a64::sp, jmpBufOffset + offsetof(SHJmpBuf, prev)));
    a.str(a64::x0, a64::Mem(xRuntime, offsetof(SHRuntime, shCurJmpBuf)));
  }

  // _sh_leave(shr, &locals.head, frame);
  // Restore the previous stack frame.
  a.str(xFrame, a64::Mem(xRuntime, RuntimeOffsets::stackPointer));
  a.ldr(
      a64::x0,
      a64::Mem(
          xFrame,
          StackFrameLayout::PreviousFrame * (int)sizeof(SHLegacyValue)));
  a.str(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::currentFrame));

  // The return value has been stashed in x21 by ret(). Move it to the return
  // register.
  a.mov(a64::x0, a64::x21);

  unsigned stackOfs = getSavedRegsOffset();
  for (unsigned i = 0; i < gpSaveCount_; i += 2, stackOfs += 16) {
    if (i + 1 < gpSaveCount_)
      a.ldp(a64::GpX(19 + i), a64::GpX(20 + i), a64::Mem(a64::sp, stackOfs));
    else
      a.ldr(a64::GpX(19 + i), a64::Mem(a64::sp, stackOfs));
  }
  for (unsigned i = 0; i < vecSaveCount_; i += 2, stackOfs += 16) {
    if (i + 1 < vecSaveCount_)
      a.ldp(
          a64::VecD(kVecSaved.first + i),
          a64::VecD(kVecSaved.first + 1 + i),
          a64::Mem(a64::sp, stackOfs));
    else
      a.ldr(a64::VecD(kVecSaved.first + i), a64::Mem(a64::sp, stackOfs));
  }
  a.ldp(a64::x29, a64::x30, a64::Mem(a64::sp, stackOfs));

  a.add(a64::sp, a64::sp, getStackSize());

  a.ret(a64::x30);

  emitCatchTable(exceptionHandlers);
  emitSlowPaths();
  emitThunks();
  emitROData();
}

void Emitter::callThunk(void *fn, const char *name) {
  // Using thunks leads to 0.27% more branch mispredicts and 2.8% performance
  // regression on an important React benchmark. So, disable them for now.
  if constexpr (false) {
    comment("// call %s", name);
    a.bl(registerThunk(fn, name));
  } else {
    callWithoutThunk(fn, name);
  }
}

void Emitter::callThunkWithSavedIP(void *fn, const char *name) {
  // Save the current IP in the runtime.
  getBytecodeIP(a64::x16);
  a.str(a64::x16, a64::Mem(xRuntime, RuntimeOffsets::currentIP));

  // Call the passed function.
  callThunk(fn, name);

  if (emitAsserts_) {
    // Invalidate the current IP to make sure it is set before the next call.
    a.mov(a64::x16, Runtime::kInvalidCurrentIP);
    a.str(a64::x16, a64::Mem(xRuntime, RuntimeOffsets::currentIP));
  }
}

void Emitter::callWithoutThunk(void *fn, const char *name) {
  comment("// call %s", name);
  loadBits64InGp(a64::x16, (uint64_t)fn, name);
  a.blr(a64::x16);
}

void Emitter::emitIncrementCounter(JitCounter counter) {
  if (!emitCounters_)
    return;
  // Push some registers onto the stack so we can use them.
  a.stp(a64::x0, a64::x1, a64::Mem(a64::sp).pre(-16));

  // Increment the counter.
  a.ldr(a64::x0, a64::Mem(xRuntime, RuntimeOffsets::runtimeJitCounters));
  a.ldr(a64::x1, a64::Mem(a64::x0, (unsigned)counter * sizeof(uint64_t)));
  a.add(a64::x1, a64::x1, 1);
  a.str(a64::x1, a64::Mem(a64::x0, (unsigned)counter * sizeof(uint64_t)));

  // Pop the saved values back off the stack.
  a.ldp(a64::x0, a64::x1, a64::Mem(a64::sp).post(16));
}

uint16_t Emitter::initHCLazyIDMayAlloc(HiddenClass *hc) {
  uint16_t id = hc->getLazyJITId();
  // Assign a new ID to the HC if we have to.
  if (id != 0)
    return id;

  // Too many IDs. Fail.
  if (jitImpl_.prevHCId == JITContext::Impl::kHCIdOverflow)
    return 0;

  id = ++jitImpl_.prevHCId;
  hc->setLazyJITId(id);

  struct : Locals {
    PinnedValue<HiddenClass> hc;
  } lv;
  LocalsRAII lraii{runtime_, &lv};
  lv.hc = hc;

  if (jitImpl_.usedHCs.isUndefined()) {
    // We would like to use a long-lived object, but we can't, because
    // ArrayStorage cannot be long-lived: it can be allocated that way
    // initially, but when it grows, it is allocated "normally".
    jitImpl_.usedHCs = runtime_.ignoreAllocationFailure(
        ArrayStorageSmall::create(runtime_, 8));
  }

  auto mh = MutableHandle<ArrayStorageSmall>::vmcast(&jitImpl_.usedHCs);
  ArrayStorageSmall::push_back(mh, runtime_, lv.hc);

  return id;
}

void Emitter::loadFrameAddr(a64::GpX dst, FR frameReg) {
  auto ofs =
      (frameReg.index() + StackFrameLayout::FirstLocal) * sizeof(SHLegacyValue);
  // If the offset fits as an immediate, just emit an add.
  if (a64::Utils::isAddSubImm(ofs)) {
    a.add(dst, xFrame, ofs);
    return;
  }
  // We cannot add the offset as an immediate, so move it in first.
  a.mov(dst, ofs);
  a.add(dst, dst, xFrame);
}

template <bool use>
void Emitter::movHWFromHW(HWReg dst, HWReg src) {
  if (dst != src) {
    if (dst.isVecD() && src.isVecD())
      a.fmov(dst.a64VecD(), src.a64VecD());
    else if (dst.isVecD())
      a.fmov(dst.a64VecD(), src.a64GpX());
    else if (src.isVecD())
      a.fmov(dst.a64GpX(), src.a64VecD());
    else
      a.mov(dst.a64GpX(), src.a64GpX());
  }
  if constexpr (use) {
    useReg(src);
    useReg(dst);
  }
}

void Emitter::_storeHWToFrame(FR fr, HWReg src) {
  _storeFrame(src, fr);
  frameRegs_[fr.index()].frameUpToDate = true;
}

void Emitter::movHWFromFR(HWReg hwRes, FR src) {
  FRState &frState = frameRegs_[src.index()];
  assert(!frState.regIsDirty && "Any local should have a valid value");
  if (frState.localGpX)
    movHWFromHW<true>(hwRes, frState.localGpX);
  else if (frState.localVecD)
    movHWFromHW<true>(hwRes, frState.localVecD);
  else if (frState.globalReg && frState.globalRegUpToDate)
    movHWFromHW<true>(hwRes, frState.globalReg);
  else
    _loadFrame(useReg(hwRes), src);
}

void Emitter::movHWFromMem(HWReg hwRes, a64::Mem src) {
  if (hwRes.isVecD())
    a.ldr(hwRes.a64VecD(), src);
  else
    a.ldr(hwRes.a64GpX(), src);
}

void Emitter::movFRFromHW(FR dst, HWReg src, FRType type) {
  FRState &frState = frameRegs_[dst.index()];
  // If it is a local or global register, move the value into it and mark it as
  // updated.
  if (frState.localGpX) {
    movHWFromHW<false>(frState.localGpX, src);
    frUpdatedWithHW(dst, frState.localGpX, type);
  } else if (frState.localVecD) {
    movHWFromHW<false>(frState.localVecD, src);
    frUpdatedWithHW(dst, frState.localVecD, type);
  } else if (frState.globalReg) {
    movHWFromHW<false>(frState.globalReg, src);
    frUpdatedWithHW(dst, frState.globalReg, type);
  } else {
    // Otherwise store it directly to the frame.
    _storeHWToFrame(dst, src);
    frUpdateType(dst, type);
    frState.frameUpToDate = true;
  }
}

void Emitter::syncFrameOutParam(FR fr, FRType type) {
  auto &frState = frameRegs_[fr.index()];

  frState.frameUpToDate = true;

  // Since the frame is the source-of-truth here, there should not be any local
  // register.
  assert(!frState.localGpX && !frState.localVecD);

  if (frState.globalReg) {
    frState.globalRegUpToDate = true;
    _loadFrame(frState.globalReg, fr);
  }
  frUpdateType(fr, type);
}

template <class TAG>
HWReg Emitter::_allocTemp(TempRegAlloc &ra, llvh::Optional<HWReg> preferred) {
  llvh::Optional<unsigned> pr{};
  if (preferred)
    pr = preferred->indexInClass();
  if (auto optReg = ra.alloc(pr); optReg)
    return HWReg(*optReg, TAG{});
  // Spill one register.
  unsigned index = pr ? *pr : ra.leastRecentlyUsed();
  _spillTempForFR(HWReg(index, TAG{}));
  ra.free(index);
  // Allocate again. This must succeed.
  return HWReg(*ra.alloc(), TAG{});
}

void Emitter::freeReg(HWReg hwReg) {
  if (!hwReg.isValid())
    return;

  FR fr = hwRegs_[hwReg.combinedIndex()].contains;
  hwRegs_[hwReg.combinedIndex()].contains = {};

  if (hwReg.isGpX()) {
    if (fr.isValid()) {
      comment("    ; free x%u (r%u)", hwReg.indexInClass(), fr.index());
      assert(frameRegs_[fr.index()].localGpX == hwReg);
      frameRegs_[fr.index()].localGpX = {};
    } else {
      comment("    ; free x%u", hwReg.indexInClass());
    }
    if (isTempGpX(hwReg))
      gpTemp_.free(hwReg.indexInClass());
  } else {
    if (fr.isValid()) {
      comment("    ; free d%u (r%u)", hwReg.indexInClass(), fr.index());
      assert(frameRegs_[fr.index()].localVecD == hwReg);
      frameRegs_[fr.index()].localVecD = {};
    } else {
      comment("    ; free d%u", hwReg.indexInClass());
    }
    if (isTempVecD(hwReg))
      vecTemp_.free(hwReg.indexInClass());
  }
}
void Emitter::syncAndFreeTempReg(HWReg hwReg) {
  if (!hwReg.isValid() || !isTemp(hwReg) ||
      !hwRegs_[hwReg.combinedIndex()].contains.isValid()) {
    return;
  }
  _spillTempForFR(hwReg);
  freeReg(hwReg);
}

// TODO: check wherger we should make this call require a temp reg.
HWReg Emitter::useReg(HWReg hwReg) {
  if (!hwReg.isValid())
    return hwReg;
  // Check whether it is a temporary.
  if (hwReg.isGpX()) {
    if (isTempGpX(hwReg))
      gpTemp_.use(hwReg.indexInClass());
  } else {
    if (isTempVecD(hwReg))
      vecTemp_.use(hwReg.indexInClass());
  }
  return hwReg;
}

void Emitter::_spillTempForFR(HWReg toSpill) {
  assert(isTemp(toSpill));

  HWRegState &hwState = hwRegs_[toSpill.combinedIndex()];
  FR fr = hwState.contains;
  hwState.contains = {};
  assert(fr.isValid() && "Allocated tmp register is unused");

  FRState &frState = frameRegs_[fr.index()];

  assert(frState.globalReg != toSpill && "global regs can't be temporary");
  if (frState.globalReg) {
    if (!frState.globalRegUpToDate) {
      movHWFromHW<false>(frState.globalReg, toSpill);
      frState.globalRegUpToDate = true;
    }
  } else {
    if (!frState.frameUpToDate) {
      _storeHWToFrame(fr, toSpill);
      frState.frameUpToDate = true;
    }
  }

  if (frState.localGpX == toSpill)
    frState.localGpX = {};
  else if (frState.localVecD == toSpill)
    frState.localVecD = {};
  else
    assert(false && "local reg not used by FR");
}

void Emitter::syncToFrame(FR fr) {
  FRState &frState = frameRegs_[fr.index()];
  if (frState.frameUpToDate)
    return;

  HWReg hwReg = _isFRInRegister(fr);
  assert(
      hwReg.isValid() && "FR is not synced to frame and is not in a register");

  // We have an invariant that the global reg cannot have an old value if the
  // frame has a new one.
  if (frState.globalReg && !frState.globalRegUpToDate) {
    assert(hwReg != frState.globalReg && "FR is in a global reg");
    movHWFromHW<false>(frState.globalReg, hwReg);
    frState.globalRegUpToDate = true;
  }
  _storeHWToFrame(fr, hwReg);
}

void Emitter::syncAllFRTempExcept(FR exceptFR) {
  for (unsigned i = 0, e = frameRegs_.size(); i < e; ++i) {
    auto &state = frameRegs_[i];
    FR fr{i};
    if (fr == exceptFR)
      continue;

    // If there is a global reg, just sync to that if needed.
    if (state.globalReg) {
      if (state.globalRegUpToDate)
        continue;
      // Note that it is valid to have no local reg even if the global reg is
      // not up to date, because the FR may be uninitialized.
      if (state.localGpX) {
        comment("    ; sync: x%u (r%u)", state.localGpX.indexInClass(), i);
        movHWFromHW<false>(state.globalReg, HWReg{state.localGpX});
        state.globalRegUpToDate = true;
      } else if (state.localVecD) {
        comment("    ; sync: d%u (r%u)", state.localVecD.indexInClass(), i);
        movHWFromHW<false>(state.globalReg, HWReg{state.localVecD});
        state.globalRegUpToDate = true;
      }
      continue;
    }

    // There is no global reg, we must sync to the frame. If the frame is
    // already up to date, we can skip this FR.
    if (state.frameUpToDate)
      continue;

    // Note that it is valid to have no local reg even if the frame is not up to
    // date, because the FR may be uninitialized.
    if (state.localGpX) {
      // Check if the next register is also a local gpx that is not up to date.
      // If so, we can write them both together.
      if (i < e - 1) {
        auto &nextState = frameRegs_[i + 1];
        // Compute the offset so we can check if it is a valid stp immediate.
        auto ofs =
            (i + hbc::StackFrameLayout::FirstLocal) * sizeof(SHLegacyValue);
        // Check that the next register also needs to be stored.
        if (!nextState.globalReg && !nextState.frameUpToDate &&
            nextState.localGpX && isStpGpXImm(ofs)) {
          comment(
              "    ; sync: x%u, x%u (r%u, r%u)",
              state.localGpX.indexInClass(),
              nextState.localGpX.indexInClass(),
              i,
              i + 1);
          // Store both registers together with stp.
          a.stp(
              state.localGpX.a64GpX(),
              nextState.localGpX.a64GpX(),
              frA64Mem(FR{i}));
          state.frameUpToDate = true;
          nextState.frameUpToDate = true;
          ++i;
          continue;
        }
      }
      comment("    ; sync: x%u (r%u)", state.localGpX.indexInClass(), i);
      _storeHWToFrame(fr, state.localGpX);
    } else if (state.localVecD) {
      comment("    ; sync: d%u (r%u)", state.localVecD.indexInClass(), i);
      _storeHWToFrame(fr, state.localVecD);
    }
  }
}

void Emitter::freeAllFRTempExcept(FR exceptFR) {
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    freeFRTemp(fr);
  }

  for (unsigned i = kVecTemp1.first; i <= kVecTemp2.second; ++i) {
    if (i > kVecTemp1.second && i < kVecTemp2.first)
      continue;

    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    freeFRTemp(fr);
  }
}

void Emitter::freeFRTemp(FR fr) {
  auto &frState = frameRegs_[fr.index()];
  if (frState.localGpX) {
    assert(isTempGpX(frState.localGpX));
    comment(
        "    ; free x%u (r%u)", frState.localGpX.indexInClass(), fr.index());
    hwRegs_[frState.localGpX.combinedIndex()].contains = {};
    gpTemp_.free(frState.localGpX.indexInClass());
    frState.localGpX = {};
  }
  if (frState.localVecD) {
    assert(isTempVecD(frState.localVecD));
    comment(
        "    ; free d%u (r%u)", frState.localVecD.indexInClass(), fr.index());
    hwRegs_[frState.localVecD.combinedIndex()].contains = {};
    vecTemp_.free(frState.localVecD.indexInClass());
    frState.localVecD = {};
  }
}

void Emitter::_assignAllocatedLocalHWReg(FR fr, HWReg hwReg) {
  hwRegs_[hwReg.combinedIndex()].contains = fr;
  if (hwReg.isGpX()) {
    comment("    ; alloc: x%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localGpX = hwReg;
  } else {
    comment("    ; alloc: d%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localVecD = hwReg;
  }
}

HWReg Emitter::_isFRInRegister(FR fr) {
  auto &frState = frameRegs_[fr.index()];
  if (frState.localGpX)
    return useReg(frState.localGpX);
  if (frState.localVecD)
    return useReg(frState.localVecD);
  if (frState.globalReg)
    return frState.globalReg;
  return {};
}

HWReg Emitter::getOrAllocFRInVecD(
    FR fr,
    bool load,
    llvh::Optional<HWReg> preferred) {
  auto &frState = frameRegs_[fr.index()];

  assert(!(load && frState.regIsDirty) && "Local is dirty");
#ifndef NDEBUG
  if (!load)
    frState.regIsDirty = true;
#endif

  if (frState.localVecD) {
    return useReg(frState.localVecD);
  }

  // Do we have a global VecD allocated to this FR?
  if (frState.globalReg.isValidVecD()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localGpX &&
          "If globalReg is not up to date, there must be a localReg");
      movHWFromHW<true>(frState.globalReg, frState.localGpX);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local VecD, so we must allocate a new tmp reg.
  HWReg hwVecD = allocTempVecD(preferred);
  _assignAllocatedLocalHWReg(fr, hwVecD);

  if (load) {
    if (frState.localGpX) {
      movHWFromHW<false>(hwVecD, frState.localGpX);
    } else if (frState.globalReg.isValidGpX()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWFromHW<false>(hwVecD, frState.globalReg);
    } else {
      _loadFrame(hwVecD, fr);
      assert(frState.frameUpToDate && "frame not up-to-date");
    }
  }

  return hwVecD;
}

HWReg Emitter::getOrAllocFRInGpX(
    FR fr,
    bool load,
    llvh::Optional<HWReg> preferred) {
  auto &frState = frameRegs_[fr.index()];

  assert(!(load && frState.regIsDirty) && "Local is dirty");
#ifndef NDEBUG
  if (!load)
    frState.regIsDirty = true;
#endif

  if (frState.localGpX) {
    assert(!(load && frState.regIsDirty) && "Local is dirty");
    return useReg(frState.localGpX);
  }

  // Do we have a global GpX allocated to this FR?
  if (frState.globalReg.isValidGpX()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localVecD &&
          "If globalReg is not up to date, there must be a localReg");
      movHWFromHW<true>(frState.globalReg, frState.localVecD);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local GpX, so we must allocate a new tmp reg.
  HWReg hwGpX = allocTempGpX(preferred);
  _assignAllocatedLocalHWReg(fr, hwGpX);

  if (load) {
    if (frState.localVecD) {
      movHWFromHW<false>(hwGpX, frState.localVecD);
    } else if (frState.globalReg.isValidVecD()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWFromHW<false>(hwGpX, frState.globalReg);
    } else {
      assert(frState.frameUpToDate && "frame not up-to-date");
      _loadFrame(hwGpX, fr);
    }
  }

  return hwGpX;
}

HWReg Emitter::getOrAllocFRInAnyReg(
    FR fr,
    bool load,
    llvh::Optional<HWReg> preferred) {
  if (HWReg tmp = _isFRInRegister(fr))
    return tmp;

  // We have neither global nor local reg, so we must allocate a new tmp reg.
  HWReg hwReg{};
  if (preferred && preferred->isVecD()) {
    hwReg = allocTempVecD(preferred);
  } else {
    hwReg = allocTempGpX(preferred);
  }
  _assignAllocatedLocalHWReg(fr, hwReg);

  if (load) {
    assert(
        frameRegs_[fr.index()].frameUpToDate &&
        "Frame must be up to date when loading");
    _loadFrame(hwReg, fr);
  }

  return hwReg;
}

void Emitter::frUpdatedWithHW(FR fr, HWReg hwReg, FRType localType) {
  FRState &frState = frameRegs_[fr.index()];

  frState.frameUpToDate = false;
#ifndef NDEBUG
  frState.regIsDirty = false;
#endif

  if (frState.globalReg == hwReg) {
    frState.globalRegUpToDate = true;

    if (frState.localGpX)
      freeReg(frState.localGpX);
    if (frState.localVecD)
      freeReg(frState.localVecD);
  } else {
    frState.globalRegUpToDate = false;
    if (hwReg == frState.localGpX) {
      freeReg(frState.localVecD);
    } else {
      assert(
          hwReg == frState.localVecD &&
          "Updated reg doesn't match any FRState register");
      freeReg(frState.localGpX);
    }
  }
  frUpdateType(fr, localType);
}

void Emitter::frUpdateType(FR fr, FRType type) {
  frameRegs_[fr.index()].localType = type;
}

void Emitter::getBytecodeIP(const a64::GpX &xOut) {
  auto ofs = codeBlock_->getOffsetOf(emittingIP);
  loadBits64InGp(xOut, (uint64_t)codeBlock_->begin(), "Bytecode start");
  // The add instruction takes a 12 bit immediate optionally shifted by 12 bits.
  // So we do the add as up to two 12 bit steps. Note that this means that it
  // will currently fail on any function that is larger than 16MB.
  auto low12Bits = ofs & llvh::maskTrailingOnes<uint32_t>(12);
  assert(a64::Utils::isAddSubImm(low12Bits) && "immediate should be 12 bits");
  a.add(xOut, xOut, low12Bits);
  if (auto restBits = ofs - low12Bits)
    a.add(xOut, xOut, restBits);
}

void Emitter::unreachable() {
  EMIT_RUNTIME_CALL(*this, void (*)(), _sh_unreachable);
}

void Emitter::profilePoint(uint16_t pointIndex) {
  comment("// ProfilePoint %u", pointIndex);
#ifdef HERMESVM_PROFILER_BB
  syncAllFRTempExcept({});
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, pointIndex);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, uint16_t),
      _interpreter_register_bb_execution);
#else
  // No-op if profiling is not enabled.
#endif
}

void Emitter::directEval(FR frRes, FR frText, bool strictCaller) {
  comment("// DirectEval r%u, r%u", frRes.index(), frText.index());
  syncAllFRTempExcept({});
  syncToFrame(frText);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frText);
  a.mov(a64::w2, strictCaller);
  EMIT_RUNTIME_CALL(
      *this,
      HermesValue(*)(Runtime &, PinnedHermesValue *, bool),
      _jit_direct_eval);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<true>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::catchInst(FR frRes) {
  comment("// Catch r%u", frRes.index());

  HWReg hwTemp = allocTempGpX();
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes);
  freeReg(hwTemp);

  // Catch simply returns the thrown value and clears it.

  // Read thrown value.
  a.ldr(hwRes.a64GpX(), a64::Mem(xRuntime, RuntimeOffsets::thrownValue));
  // Clear thrown value.
  loadBits64InGp(hwTemp.a64GpX(), _sh_ljs_empty().raw, "empty");
  a.str(hwTemp.a64GpX(), a64::Mem(xRuntime, RuntimeOffsets::thrownValue));
}

void Emitter::ret(FR frValue) {
  movHWFromFR(HWReg::gpX(21), frValue);
  a.b(returnLabel_);
}

void Emitter::mov(FR frRes, FR frInput, bool logComment) {
  // Sometimes mov() is used by other instructions, so logging is optional.
  if (logComment)
    comment("// %s r%u, r%u", "mov", frRes.index(), frInput.index());
  if (frRes == frInput)
    return;

  HWReg hwInput = getOrAllocFRInAnyReg(frInput, true);
  HWReg hwDest = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwDest, hwInput);
  frUpdatedWithHW(frRes, hwDest, frameRegs_[frInput.index()].localType);
}

void Emitter::loadParam(FR frRes, uint32_t paramIndex) {
  comment("// LoadParam r%u, %u", frRes.index(), paramIndex);

  asmjit::Error err;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwTmp = allocAndLogTempGpX();
  a64::GpW wTmp(hwTmp.indexInClass());

  a.ldur(
      wTmp,
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));

  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTmp, paramIndex));
  // Does paramIndex fit in the 12-bit unsigned immediate?
  if (err) {
    HWReg hwTmp2 = allocAndLogTempGpX();
    a64::GpW wTmp2(hwTmp2.indexInClass());
    loadBits64InGp(wTmp2, paramIndex, "paramIndex");
    a.cmp(wTmp, wTmp2);
    freeReg(hwTmp2);
  }
  a.b_lo(slowPathLab);

  freeReg(hwTmp);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);

  // FIXME: handle integer overflow better?
  int32_t ofs = ((int)StackFrameLayout::ThisArg - (int32_t)paramIndex) *
      (int)sizeof(SHLegacyValue);
  if (ofs >= 0)
    hermes_fatal("JIT integer overflow");
  EXPECT_ERROR(
      asmjit::kErrorInvalidDisplacement,
      err = a.ldur(hwRes.a64GpX(), a64::Mem(xFrame, ofs)));
  // Does the offset fit in the 9-bit signed offset?
  if (err) {
    ofs = -ofs;
    a64::GpX xRes = hwRes.a64GpX();
    if (ofs <= 4095) {
      a.sub(xRes, xFrame, ofs);
    } else {
      loadBits64InGp(xRes, ofs, nullptr);
      a.sub(xRes, xFrame, xRes);
    }
    a.ldr(xRes, a64::Mem(xRes));
  }

  a.bind(contLab);
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: LoadParam r%u", sl.frRes.index());
         em.a.bind(sl.slowPathLab);
         em.loadBits64InGp(
             sl.hwRes.a64GpX(), _sh_ljs_undefined().raw, "undefined");
         em.a.b(sl.contLab);
       }});
}

void Emitter::loadConstDouble(FR frRes, double val, const char *name) {
  comment("// LoadConst%s r%u, %f", name, frRes.index(), val);
  HWReg hwRes{};

  // Check bitwise for zero because it may be -0.
  if (llvh::DoubleToBits(val) == llvh::DoubleToBits(0)) {
    // TODO: this check should be wider.
    hwRes = getOrAllocFRInVecD(frRes, false);
    a.movi(hwRes.a64VecD(), 0);
  } else if (a64::Utils::isFP64Imm8((double)val)) {
    hwRes = getOrAllocFRInVecD(frRes, false);
    a.fmov(hwRes.a64VecD(), (double)val);
  } else {
    uint64_t bits = llvh::DoubleToBits(val);
    if (isCheapConst(bits)) {
      hwRes = getOrAllocFRInGpX(frRes, false);
      a.mov(hwRes.a64GpX(), bits);
    } else {
      hwRes = getOrAllocFRInVecD(frRes, false);
      a.ldr(
          hwRes.a64VecD(),
          a64::Mem(roDataLabel_, uint64Const(bits, "fp64 const")));
    }
  }
  frUpdatedWithHW(frRes, hwRes, FRType::Number);
}

template <typename REG>
void Emitter::loadBits64InGp(
    const REG &dest,
    uint64_t bits,
    const char *constName) {
  if (isCheapConst(bits)) {
    a.mov(dest, bits);
  } else {
    a.ldr(dest, a64::Mem(roDataLabel_, uint64Const(bits, constName)));
  }
}

void Emitter::loadSmallHermesValueInGpX(
    a64::GpX &dest,
    SmallHermesValue shv,
    const char *constName) {
  if constexpr (sizeof(SmallHermesValue) == 4) {
    a.mov(dest.w(), shv.getRaw());
  } else {
    loadBits64InGp(dest, shv.getRaw(), constName);
  }
}

void Emitter::loadConstStringInGpX(
    SymbolID id,
    const a64::GpX &xOut,
    const a64::GpX &xTemp) {
  assert(xOut != xTemp);
  static_assert(
      std::is_same_v<
          RuntimeOffsets::IdentifierTableLookupVectorType,
          TransparentConservativeVector<
              RuntimeOffsets::IdentifierTableLookupEntryType>>,
      "lookupVector_ must be transparent");
  a.ldr(
      xOut,
      a64::Mem(
          xRuntime,
          RuntimeOffsets::identifierTable +
              RuntimeOffsets::identifierTableLookupVector +
              TransparentConservativeVector<
                  RuntimeOffsets::IdentifierTableLookupEntryType>::
                  dataPointerOffset()));
  // xOut = xOut[symID.index].strPrim_
  size_t offset =
      (id.unsafeGetIndex() * RuntimeOffsets::identifierTableLookupEntrySize) +
      RuntimeOffsets::identifierTableLookupEntryStrPrim;
  asmjit::Error err;
  EXPECT_ERROR(
      asmjit::kErrorInvalidDisplacement,
      err = a.ldr(xOut, a64::Mem(xOut, offset)));
  if (err) {
    a.mov(xTemp, offset);
    a.ldr(xOut, a64::Mem(xOut, xTemp));
  }
}

void Emitter::loadConstBits64(
    FR frRes,
    uint64_t bits,
    FRType type,
    const char *name) {
  comment(
      "// LoadConst%s r%u, %llu",
      name,
      frRes.index(),
      (unsigned long long)bits);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);

  loadBits64InGp(hwRes.a64GpX(), bits, name);
  frUpdatedWithHW(frRes, hwRes, type);
}

void Emitter::loadConstString(
    FR frRes,
    RuntimeModule *runtimeModule,
    uint32_t stringID) {
  comment("// LoadConstString r%u, stringID %u", frRes.index(), stringID);

  Runtime &runtime = runtimeModule->getRuntime();
  SymbolID symID = runtimeModule->getSymbolIDFromStringIDMayAllocate(stringID);
  [[maybe_unused]] StringPrimitive *strPrim =
      runtime.getStringPrimFromSymbolID(symID);
  assert(strPrim && "must be allocated");

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  HWReg hwTmp = allocTempGpX();
  freeReg(hwTmp);

  loadConstStringInGpX(symID, hwRes.a64GpX(), hwTmp.a64GpX());
  emit_sh_ljs_string(a, hwRes.a64GpX());
}

void Emitter::loadConstBigInt(
    FR frRes,
    RuntimeModule *runtimeModule,
    uint32_t bigIntID) {
  comment("// LoadConstBigInt r%u, bigIntID %u", frRes.index(), bigIntID);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(a64::w2, bigIntID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHRuntimeModule *, uint32_t),
      _sh_ljs_get_bytecode_bigint);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<true>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::toNumber(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumber", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput))
    return mov(frRes, frInput, false);

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  syncToFrame(frInput);

  hwInput = getOrAllocFRInVecD(frInput, true);

  // We don't free frRes so that if it is the same as frThis, the register is
  // simply persisted and we do not need to perform a move in the fast path.
  freeAllFRTempExcept(frRes);
  hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Number);

  // Since HermesValue is NaN-boxed we know that all non-number values will be
  // NaN. So we can conveniently test for non-number values by checking for NaN
  // (which does not compare equal to itself).
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.fcmp(hwInput.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);
  movHWFromHW<false>(hwRes, hwInput);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: toNumber r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             double (*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_double_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::vecD(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::toNumeric(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumeric", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput))
    return mov(frRes, frInput, false);

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  syncToFrame(frInput);

  hwInput = getOrAllocFRInVecD(frInput, true);

  // We don't free frRes so that if it is the same as frThis, the register is
  // simply persisted and we do not need to perform a move in the fast path.
  freeAllFRTempExcept(frRes);
  hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::UnknownPtr);

  // Since HermesValue is NaN-boxed we know that all non-number values will be
  // NaN. So we can conveniently test for non-number values by checking for NaN
  // (which does not compare equal to itself).
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.fcmp(hwInput.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);
  movHWFromHW<false>(hwRes, hwInput);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: toNumeric r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_numeric_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::toInt32(FR frRes, FR frInput) {
  comment("// ToInt32 r%u, r%u", frRes.index(), frInput.index());

  HWReg hwTempGpX = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInVecD(frInput, true);
  emit_double_is_int(
      a, hwTempGpX.a64GpX(), hwTempVecD.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempGpX);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Number);

  // Truncate to an int32 in the fast path.
  a.scvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// to_int32 r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             double (*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_int32_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::vecD(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::addEmptyString(FR frRes, FR frInput) {
  comment("// AddEmptyString r%u, r%u", frRes.index(), frInput.index());

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);
  freeAllFRTempExcept(frRes);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);

  // Check if the input is already a string and don't do anything.
  emit_sh_ljs_is_string(a, hwTemp.a64GpX(), hwInput.a64GpX());
  a.b_ne(slowPathLab);

  // Fast path.
  movHWFromHW<false>(hwRes, hwInput);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: AddEmptyString r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_add_empty_string_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::bumpAllocAndUnpoison(
    uint32_t sz,
    const a64::GpX &xOut,
    const a64::GpX &xTemp1,
    const a64::GpX &xTemp2,
    const asmjit::Label &slowPathLab) {
#ifdef HERMESVM_GC_HADES
  // Load the current YG level and end address.
  a.ldr(xOut, a64::Mem(xRuntime, RuntimeOffsets::runtimeHadesYGLevel));
  a.ldr(xTemp1, a64::Mem(xRuntime, RuntimeOffsets::runtimeHadesYGEnd));

  // Try to increment the heap level by the size of the object.
  if (a64::Utils::isAddSubImm(sz)) {
    a.add(xTemp2, xOut, sz);
  } else {
    a.mov(xTemp2, sz);
    a.add(xTemp2, xOut, xTemp2);
  }
  a.cmp(xTemp2, xTemp1);
  a.b_hi(slowPathLab);

#if LLVM_ADDRESS_SANITIZER_BUILD
  // Save all the temporary registers in pairs.
  static_assert((kGPTemp.second - kGPTemp.first + 1) % 2 == 0);
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; i += 2)
    a.stp(a64::x(i + 1), a64::x(i), a64::Mem(a64::sp).pre(-16));

  a.mov(a64::x0, xOut);
  a.mov(a64::x1, sz);
  // Unpoison the newly allocated memory.
  EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
      *this,
      void (*)(void const volatile *, size_t),
      __asan_unpoison_memory_region);

  // Restore the temporary registers.
  for (int i = kGPTemp.second; i >= kGPTemp.first; i -= 2)
    a.ldp(a64::x(i), a64::x(i - 1), a64::Mem(a64::sp).post(16));
#endif

  // Allocating succeeded, update the level.
  a.str(xTemp2, a64::Mem(xRuntime, RuntimeOffsets::runtimeHadesYGLevel));
#else
  // MallocGC does not support inline allocation.
  a.b(slowPathLab);
#endif
}

void Emitter::initGCCell(
    CellKind kind,
    uint32_t sz,
    const a64::GpX &xCell,
    const a64::GpX &xTemp1) {
  // Initialize the fields on GCCell.
#ifndef NDEBUG
  // cell->magic = GCCell::kMagic
  a.mov(xTemp1, RuntimeOffsets::gcCellMagicValue);
  static_assert(sizeof(SHGCCell::magic) == 2);
  a.strh(xTemp1.w(), a64::Mem(xCell, offsetof(SHGCCell, magic)));

  // cell->debugAllocationId = heap->debugAllocationCounter_++;
  a.ldr(xTemp1, a64::Mem(xRuntime, RuntimeOffsets::runtimeDebugAllocCounter));
  static_assert(sizeof(SHGCCell::debugAllocationId) == 4);
  a.str(xTemp1.w(), a64::Mem(xCell, offsetof(SHGCCell, debugAllocationId)));
  a.add(xTemp1, xTemp1, 1);
  a.str(xTemp1, a64::Mem(xRuntime, RuntimeOffsets::runtimeDebugAllocCounter));
#endif

  // Load the KindAndSize into a register.
  KindAndSize ks{kind, sz};
  CompressedPointer::RawType rawKS;
  static_assert(sizeof(ks) == sizeof(rawKS));
  memcpy(&rawKS, &ks, sizeof(ks));
  // Note that this is almost always a "cheap constant".
  loadBits64InGp(xTemp1, rawKS, "KindAndSize");

  // KindAndSize has the same size as a compressed pointer, so store it as one.
  emit_store_cp(a, xTemp1, a64::Mem(xCell, offsetof(SHGCCell, kindAndSize)));
}

void Emitter::allocInYoung(
    CellKind kind,
    uint32_t sz,
    const a64::GpX &xOut,
    const a64::GpX &xTemp1,
    const a64::GpX &xTemp2,
    const asmjit::Label &slowPathLab) {
  // Ensure the size is aligned as required.
  sz = heapAlignSize(sz);
  bumpAllocAndUnpoison(sz, xOut, xTemp1, xTemp2, slowPathLab);
  initGCCell(kind, sz, xOut, xTemp1);
}

void Emitter::alloc2InYoung(
    CellKind kind1,
    uint32_t sz1,
    CellKind kind2,
    uint32_t sz2,
    const a64::GpX &xOut1,
    const a64::GpX &xOut2,
    const a64::GpX &xTemp,
    const asmjit::Label &slowPathLab) {
  // Ensure the size is aligned as required.
  sz1 = heapAlignSize(sz1);
  sz2 = heapAlignSize(sz2);
  bumpAllocAndUnpoison(
      sz1 + sz2, xOut1, /* xTemp1 */ xOut2, /*xTemp2 */ xTemp, slowPathLab);

  // Initialize first cell.
  initGCCell(kind1, sz1, xOut1, xTemp);

  // Place the pointer to the second cell in xTemp1 and initialize the GCCell.
  if (a64::Utils::isAddSubImm(sz1)) {
    a.add(xOut2, xOut1, sz1);
  } else {
    a.mov(xTemp, sz1);
    a.add(xOut2, xOut1, xTemp);
  }
  initGCCell(kind2, sz2, xOut2, xTemp);
}

void Emitter::newObject(FR frRes) {
  comment("// NewObject r%u", frRes.index());
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  // Allocate the result register.
  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  auto xRes = hwRes.a64GpX();

  // Allocate temporary registers, note that these must be different from the
  // result as we will use all of them together.
  HWReg hwTemp1 = allocTempGpX();
  HWReg hwTemp2 = allocTempGpX();
  auto xTemp1 = hwTemp1.a64GpX();
  auto xTemp2 = hwTemp2.a64GpX();
  freeReg(hwTemp1);
  freeReg(hwTemp2);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  // Allocate the object.
  allocInYoung(
      CellKind::JSObjectKind,
      cellSize<JSObject>(),
      xRes,
      xTemp1,
      xTemp2,
      slowPathLab);

  // Get the parent.
  a.ldr(xTemp1, a64::Mem(xRuntime, offsetof(Runtime, objectPrototype)));
  emit_sh_ljs_get_pointer(a, xTemp1, xTemp1);
  emit_sh_cp_encode_non_null(a, xTemp1);

  emit_jsobject_init(
      a, xRes, /* xParent */ xTemp1, /* xTempOrPropStorageOpt */ xTemp2, false);

  // Add the object tag to the result.
  emit_sh_ljs_object(a, xRes);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: NewObject r%u", sl.frRes.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(
             em, SHLegacyValue(*)(SHRuntime *), _sh_ljs_new_object);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::newObjectWithParent(FR frRes, FR frParent) {
  comment("// NewObjectWithParent r%u, r%u", frRes.index(), frParent.index());
  syncAllFRTempExcept(frRes != frParent ? frRes : FR());
  syncToFrame(frParent);
  auto hwParent = getOrAllocFRInGpX(frParent, true);
  auto hwNewObjPtr = allocTempGpX();
  auto hwTemp1 = allocTempGpX();
  auto hwTemp2 = allocTempGpX();
  auto xParent = hwParent.a64GpX();
  auto xNewObjPtr = hwNewObjPtr.a64GpX();
  auto xTemp1 = hwTemp1.a64GpX();
  auto xTemp2 = hwTemp2.a64GpX();
  freeAllFRTempExcept({});
  freeReg(hwNewObjPtr);
  freeReg(hwTemp1);
  freeReg(hwTemp2);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  allocInYoung(
      CellKind::JSObjectKind,
      cellSize<JSObject>(),
      xNewObjPtr,
      xTemp1,
      xTemp2,
      slowPathLab);

  // Compute what the parent should be.
  auto decodeObjParentLab = a.newLabel();
  auto parentDoneLab = a.newLabel();

  // Check if the parent is an object.
  emit_sh_ljs_is_object(a, xTemp1, xParent);
  a.b_eq(decodeObjParentLab);

  // Check if the parent is null.
  emit_sh_ljs_is_null(a, xTemp1, xParent);
  // At this point, the parent is no longer useful, so we are free to overwrite
  // it. Set it to zero so we store nullptr if the parent is JS null.
  a.mov(xParent, 0);
  a.b_eq(parentDoneLab);

  // The parent is not an object or null, so get Object.prototype.
  a.ldr(xParent, a64::Mem(xRuntime, offsetof(Runtime, objectPrototype)));

  // Extract the parent object pointer from the HermesValue in xParent.
  a.bind(decodeObjParentLab);
  emit_sh_ljs_get_pointer(a, xParent, xParent);
  emit_sh_cp_encode_non_null(a, xParent);

  a.bind(parentDoneLab);

  // Initialize the object.
  emit_jsobject_init(a, xNewObjPtr, xParent, xTemp1, false);

  auto hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);

  // Move the object into the result register as a HermesValue.
  emit_sh_ljs_object2(a, hwRes.a64GpX(), xNewObjPtr);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frParent,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: NewObjectWithParent r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);

         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_new_object_with_parent);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::newObjectWithBuffer(
    FR frRes,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  ShapeTableEntry shapeInfo = codeBlock_->getRuntimeModule()
                                  ->getBytecode()
                                  ->getObjectShapeTable()[shapeTableIndex];

  // If the object and the property storage can't be guaranteed to be in the
  // young gen, bail.
  // TODO: Fix this once we can inline write barriers.
  if (shapeInfo.numProps > JSObject::maxYoungGenAllocationPropCount()) {
    newObjectWithBufferSlow(frRes, shapeTableIndex, valBufferOffset);
    return;
  }

  // Simple visitor to check if the fast path is possible.
  struct {
    void visitStringID(StringID) {}
    void visitNumber(double d) {
      // TODO: Implement fast loading for boxed double values.
      if (!SmallHermesValue::canInlineDouble(d))
        fast = false;
    }
    void visitNull() {}
    void visitUndefined() {}
    void visitBool(bool) {}
    bool fast = true;
  } fastPathCheckVisitor{};

  SerializedLiteralParser::parse(
      codeBlock_->getRuntimeModule()
          ->getBytecode()
          ->getLiteralValueBuffer()
          .slice(valBufferOffset),
      shapeInfo.numProps,
      fastPathCheckVisitor);

  // If we can't use the fast path, fall back to the slow path.
  if (!fastPathCheckVisitor.fast) {
    newObjectWithBufferSlow(frRes, shapeTableIndex, valBufferOffset);
    return;
  }

  // Fast path: we can create the object directly.
  comment(
      "// NewObjectWithBuffer r%u, %u, %u",
      frRes.index(),
      shapeTableIndex,
      valBufferOffset);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);

  HWReg hwObj = allocTempGpX();
  HWReg hwClazz = allocTempGpX();
  HWReg hwTmp = allocTempGpX();
  HWReg hwTmp2 = allocTempGpX();
  a64::GpX xObj = hwObj.a64GpX();
  a64::GpX xClazz = hwClazz.a64GpX();
  a64::GpX xTmp = hwTmp.a64GpX();
  a64::GpX xTmp2 = hwTmp2.a64GpX();
  freeReg(hwObj);
  freeReg(hwClazz);
  freeReg(hwTmp);
  freeReg(hwTmp2);

  // Load the HiddenClass from the cache.
#ifdef HERMESVM_GC_HADES
  // First check the read barrier.
  a.ldrb(
      xTmp.w(),
      a64::Mem(xRuntime, RuntimeOffsets::runtimeHadesOGMarkingBarriers));
  a.cbnz(xTmp.w(), slowPathLab);
#endif
  // Load the HC from the cache.
  static_assert(
      std::is_same_v<
          TransparentConservativeVector<WeakRoot<HiddenClass>>,
          RuntimeOffsets::RuntimeModuleObjectLiteralHiddenClassesType>,
      "objectLiteralHiddenClasses_ must be transparent");
  loadBits64InGp(
      xClazz, (uint64_t)codeBlock_->getRuntimeModule(), "RuntimeModule");
  a.ldr(
      xClazz,
      a64::Mem(
          xClazz, RuntimeOffsets::runtimeModuleObjectLiteralHiddenClasses));
  emit_load_cp(
      a,
      xClazz,
      a64::Mem(xClazz, shapeTableIndex * sizeof(WeakRoot<HiddenClass>)));
  // If the HC isn't cached, slow path.
  a.cbz(xClazz, slowPathLab);

  // Create the object.
  const unsigned numIndirectSlots =
      shapeInfo.numProps <= JSObject::DIRECT_PROPERTY_SLOTS
      ? 0
      : shapeInfo.numProps - JSObject::DIRECT_PROPERTY_SLOTS;
  if (numIndirectSlots > 0) {
    // Need indirect property storage, so allocate 2 cells.
    // We know that both together should be able to fit in young gen if there's
    // space due to the above checks.
    // PropStorage will be in xTmp2.
    alloc2InYoung(
        CellKind::JSObjectKind,
        cellSize<JSObject>(),
        PropStorage::getCellKind(),
        PropStorage::allocationSize(numIndirectSlots),
        /* xOut1 */ xObj,
        /* xOut2 */ xTmp2,
        xTmp,
        slowPathLab);
  } else {
    allocInYoung(
        CellKind::JSObjectKind,
        cellSize<JSObject>(),
        xObj,
        xTmp,
        xTmp2,
        slowPathLab);
  }

  // Get the parent.
  a.ldr(xTmp, a64::Mem(xRuntime, offsetof(Runtime, objectPrototype)));
  emit_sh_ljs_get_pointer(a, xTmp, xTmp);
  emit_sh_cp_encode_non_null(a, xTmp);

  // Initialize the JSObject to have the correct parent/HC.
  emit_jsobject_init(
      a,
      xObj,
      /* xParent */ xTmp,
      /* xTempOrPropStorageOpt */ xTmp2,
      numIndirectSlots > 0,
      /* xClazz */ xClazz);

  if (numIndirectSlots > 0) {
    // The parent in xTmp is no longer needed.
    // Populate the size.
    a.mov(xTmp, numIndirectSlots);
    a.str(
        xTmp,
        a64::Mem(
            xObj,
            heapAlignSize(cellSize<JSObject>()) +
                offsetof(SHArrayStorageSmall, size)));
  }

  // Come back from the slow path with xObj having the JSObject pointer.
  a.bind(contLab);

  // Store the HermesValue encoded result and never update it again.
  // From here on we'll just populate the values via a raw pointer to the
  // JSObject in xObj.
  emit_sh_ljs_object2(a, hwRes.a64GpX(), xObj);

  // Store each of the values to the object at xObj.
  // No write barrier required because the object and property storage were
  // allocated in the young gen.
  struct {
    Emitter &em;
    a64::GpX &xObj;
    a64::GpX &xTmp;
    a64::GpX &xTmp2;
    /// Needed by the convenient EXPECT_ERROR macro, which expects to be able to
    /// use the Emitter::expectedError_ field (using implicit 'this').
    /// Reference it directly here so the macro keeps working.
    asmjit::Error &expectedError_ = em.expectedError_;

    /// Iteration counter.
    /// Index of the next value to be inserted into the object.
    size_t i = 0;

    // \return the offset from xObj where we'll place the property.
    //   xObj either points to the JSObject or to indirect property storage
    //   depending on the value of \c i.
    size_t currentOffset() const {
      if (i < JSObject::DIRECT_PROPERTY_SLOTS) {
        return offsetof(SHJSObjectAndDirectProps, directProps) +
            i * sizeof(SHGCSmallHermesValue);
      } else {
        // If we're reading from indirect storage, offset by the number of
        // direct property slots.
        auto storageSlot = i - JSObject::DIRECT_PROPERTY_SLOTS;
        return offsetof(SHArrayStorageSmall, storage) +
            storageSlot * sizeof(SHGCSmallHermesValue);
      }
    }

    /// Store a simple SmallHermesValue into the current offset in the object.
    void storeVal(SmallHermesValue val) {
      em.loadSmallHermesValueInGpX(xTmp, val, "object literal buffer val");
      asmjit::Error err;
      EXPECT_ERROR(
          asmjit::kErrorInvalidDisplacement,
          err = emit_store_shv(em.a, xTmp, a64::Mem(xObj, currentOffset())));
      if (err) {
        em.a.mov(xTmp2, currentOffset());
        emit_store_shv(em.a, xTmp, a64::Mem(xObj, xTmp2));
      }
    }

    /// Advance internal state to the next slot, updating the counter and any
    /// registers so that they have the correct state for the next element.
    void advance() {
      ++i;

      // xObj was set to the JSObject itself initially.
      // If we are transitioning into indirect storage, switch xObj to point to
      // the indirect property storage.
      if (i == JSObject::DIRECT_PROPERTY_SLOTS) {
        // We know alloc2InYoung succeded so we can just advance the pointer.
#ifdef HERMESVM_GC_HADES
        em.a.add(xObj, xObj, heapAlignSize(cellSize<JSObject>()));
#else
        emit_load_cp(
            em.a, xObj, a64::Mem(xObj, offsetof(SHJSObject, propStorage)));
        emit_sh_cp_decode_non_null(em.a, xObj);
#endif
      }
    }

    void visitStringID(StringID id) {
      em.comment("    ; string");
      RuntimeModule *runtimeModule = em.codeBlock_->getRuntimeModule();
      Runtime &runtime = runtimeModule->getRuntime();
      SymbolID symID = runtimeModule->getSymbolIDFromStringIDMayAllocate(id);

      // Force allocation of the StringPrimitive at JIT time.
      // StringPrimitive won't be freed because RuntimeModule keeps the symbols
      // in stringIDMap_ alive and StringPrimitives with live symbols aren't
      // freed.
      [[maybe_unused]] StringPrimitive *strPrim =
          runtime.getStringPrimFromSymbolID(symID);
      assert(strPrim && "must be allocated");

      // xTmp = identifierTable_.lookupVector_.ptr
      em.loadConstStringInGpX(symID, xTmp, xTmp2);
      // Encode compressed pointer and wrap with StringTag.
      // We know it's not null because we allocated it at JIT compile time.
      emit_sh_cp_encode_non_null(em.a, xTmp);
      emit_shv_string(em.a, xTmp);
      // Store to storage.
      emit_store_shv(em.a, xTmp, a64::Mem(xObj, currentOffset()));

      advance();
    }
    void visitNumber(double d) {
      em.comment("    ; number: %lf", d);
      assert(
          SmallHermesValue::canInlineDouble(d) &&
          "boxed doubles not supported in fast path");
      storeVal(SmallHermesValue::encodeInlineDoubleValueUnsafe(d));
      advance();
    }
    void visitNull() {
      em.comment("    ; null");
      storeVal(SmallHermesValue::encodeNullValue());
      advance();
    }
    void visitUndefined() {
      em.comment("    ; undefined");
      storeVal(SmallHermesValue::encodeUndefinedValue());
      advance();
    }
    void visitBool(bool b) {
      em.comment("    ; bool: %d", b);
      storeVal(SmallHermesValue::encodeBoolValue(b));
      advance();
    }
  } emittingVisitor{*this, xObj, xTmp, xTmp2};

  SerializedLiteralParser::parse(
      codeBlock_->getRuntimeModule()
          ->getBytecode()
          ->getLiteralValueBuffer()
          .slice(valBufferOffset),
      shapeInfo.numProps,
      emittingVisitor);

  // This slow path only allocates the new object.
  // Property population is always done in JIT-emitted code specialized for this
  // shape table entry.
  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwObj,
       .sizeOrIdx = shapeTableIndex,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: NewObjectWithBuffer r%u, %u, %u",
             sl.frRes.index(),
             sl.sizeOrIdx,
             sl.sizeOrIdx2);
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadBits64InGp(a64::x1, (uint64_t)em.codeBlock_, "CodeBlock");
         em.a.mov(a64::w2, sl.sizeOrIdx);
         em.loadFrameAddr(a64::x3, sl.frRes);
         EMIT_RUNTIME_CALL(
             em,
             JSObject *
                 (*)(Runtime &, CodeBlock *, uint32_t, PinnedHermesValue *),
             _jit_new_empty_object_for_buffer);
         // Move into xObj.
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::newObjectWithBufferSlow(
    FR frRes,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  comment(
      "// NewObjectWithBuffer r%u, %u, %u",
      frRes.index(),
      shapeTableIndex,
      valBufferOffset);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, shapeTableIndex);
  a.mov(a64::w3, valBufferOffset);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHCodeBlock *, uint32_t, uint32_t),
      _interpreter_create_object_from_buffer);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newObjectWithBufferAndParent(
    FR frRes,
    FR frParent,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  comment(
      "// NewObjectWithBufferAndParent r%u, r%u, %u, %u",
      frRes.index(),
      frParent.index(),
      shapeTableIndex,
      valBufferOffset);

  // We unconditionally skip frRes here because we handle frParent with the
  // syncToFrame below.
  syncAllFRTempExcept(frRes);
  syncToFrame(frParent);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  loadFrameAddr(a64::x2, frParent);
  a.mov(a64::w3, shapeTableIndex);
  a.mov(a64::w4, valBufferOffset);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHCodeBlock *, SHLegacyValue *, uint32_t, uint32_t),
      _interpreter_create_object_from_buffer_with_parent);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newArray(FR frRes, uint32_t size) {
  comment("// NewArray r%u, %u", frRes.index(), size);
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, size);
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, uint32_t), _sh_ljs_new_array);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newArrayWithBuffer(
    FR frRes,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex) {
  comment(
      "// NewArrayWithBuffer r%u, %u, %u, %u",
      frRes.index(),
      numElements,
      numLiterals,
      bufferIndex);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, numElements);
  a.mov(a64::w3, numLiterals);
  a.mov(a64::w4, bufferIndex);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHCodeBlock *, uint32_t, uint32_t, uint32_t),
      _interpreter_create_array_from_buffer);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newFastArray(FR frRes, uint32_t size) {
  comment("// NewFastArray r%u, %u", frRes.index(), size);
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, size);
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, uint32_t), _sh_new_fastarray);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::fastArrayLength(FR frRes, FR frArr) {
  comment("// FastArrayLength r%u, r%u", frRes.index(), frArr.index());
  // We allocate a temporary register to compute the address instead of using
  // the result register in case the result has a VecD allocated for it.
  HWReg temp = allocTempGpX();
  HWReg hwArr = getOrAllocFRInGpX(frArr, true);
  // Done allocating, free the temp so it can be reused for the result.
  freeReg(temp);
  emit_sh_ljs_get_pointer(a, temp.a64GpX(), hwArr.a64GpX());

#ifdef HERMESVM_BOXED_DOUBLES
  // If boxed doubles are enabled, load the size from the ArrayStorage, where it
  // is stored as an integer.
  emit_load_cp(
      a,
      temp.a64GpX(),
      a64::Mem(temp.a64GpX(), offsetof(SHFastArray, indexedStorage)));
  emit_sh_cp_decode_non_null(a, temp.a64GpX());
  a.ldr(
      temp.a64GpX().w(),
      a64::Mem(temp.a64GpX(), offsetof(SHArrayStorage, size)));
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  a.ucvtf(hwRes.a64VecD(), temp.a64GpX().w());
#else
  // If boxed doubles are disabled, we can just load the size from the length
  // property of the FastArray.
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(hwRes, a64::Mem(temp.a64GpX(), offsetof(SHFastArray, length)));
#endif

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::fastArrayLoad(FR frRes, FR frArr, FR frIdx) {
  comment(
      "// FastArrayLoad r%u, r%u, r%u",
      frRes.index(),
      frArr.index(),
      frIdx.index());
#if defined(HERMESVM_COMPRESSED_POINTERS) || defined(HERMESVM_BOXED_DOUBLES)
  syncAllFRTempExcept(frRes != frArr && frRes != frIdx ? frRes : FR());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frArr);
  movHWFromFR(HWReg::vecD(0), frIdx);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, double idx),
      _sh_fastarray_load);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
#else
  asmjit::Label slowPathLab = newSlowPathLabel();
  // We allocate a temporary register to compute the address instead of using
  // the result register in case the result has a VecD allocated for it.
  HWReg hwTmpStorage = allocTempGpX();
  HWReg hwTmpSize = allocTempGpX();
  HWReg hwTmpIdxGpX = allocTempGpX();
  HWReg hwTmpIdxVecD = allocTempVecD();
  HWReg hwArr = getOrAllocFRInGpX(frArr, true);
  HWReg hwIdx = getOrAllocFRInVecD(frIdx, true);
  // Done allocating, free the temps so they can be reused for the result.
  freeReg(hwTmpStorage);
  freeReg(hwTmpSize);
  freeReg(hwTmpIdxGpX);
  freeReg(hwTmpIdxVecD);

  // Retrieve the FastArray pointer and use it to load the indexed storage
  // pointer.
  emit_sh_ljs_get_pointer(a, hwTmpStorage.a64GpX(), hwArr.a64GpX());
  movHWFromMem(
      hwTmpStorage,
      a64::Mem(hwTmpStorage.a64GpX(), offsetof(SHFastArray, indexedStorage)));

  // Load the size from the indexed storage.
  a.ldr(
      hwTmpSize.a64GpX().w(),
      a64::Mem(hwTmpStorage.a64GpX(), offsetof(SHArrayStorageSmall, size)));

  // Check if the index is a uint32.
  emit_double_is_uint32(
      a, hwTmpIdxGpX.a64GpX().w(), hwTmpIdxVecD.a64VecD(), hwIdx.a64VecD());
  // If the conversion was successful, compare the size against the index.
  // Otherwise, set the flags to zero to force the subsequent b_ls to be taken.
  a.ccmp(
      hwTmpSize.a64GpX().w(), hwTmpIdxGpX.a64GpX().w(), 0, a64::CondCode::kEQ);
  // If the index is out-of-bounds jump to the failure path.
  // We will have to sync registers when the access is inside a try region
  // because we could read from the FRs again in this function.
  if (isInTry())
    syncAllFRTempExcept(frRes != frArr && frRes != frIdx ? frRes : FR());
  a.b_ls(slowPathLab);

  // Add the offset of the actual data in the ArrayStorage.
  a.add(
      hwTmpStorage.a64GpX(),
      hwTmpStorage.a64GpX(),
      offsetof(SHArrayStorageSmall, storage));

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(
      hwRes,
      a64::Mem(hwTmpStorage.a64GpX(), hwTmpIdxGpX.a64GpX(), a64::lsl(3)));
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .frRes = frRes,
       .frInput1 = frArr,
       .frInput2 = frIdx,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: FastArrayLoad r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(em, void (*)(SHRuntime *), _sh_throw_array_oob);
         // Call does not return.
       }});
#endif
}

void Emitter::fastArrayStore(FR frArr, FR frIdx, FR frVal) {
  comment(
      "// FastArrayStore r%u, r%u, r%u",
      frArr.index(),
      frIdx.index(),
      frVal.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frVal);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frVal);
  loadFrameAddr(a64::x2, frArr);
  movHWFromFR(HWReg{a64::d0}, frIdx);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, const SHLegacyValue *, SHLegacyValue *, double idx),
      _sh_fastarray_store);
}

void Emitter::fastArrayPush(FR frArr, FR frVal) {
  comment("// FastArrayPush r%u, r%u", frArr.index(), frVal.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frVal);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frVal);
  loadFrameAddr(a64::x2, frArr);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_fastarray_push);
}

void Emitter::fastArrayAppend(FR frArr, FR frOther) {
  comment("// FastArrayAppend r%u, r%u", frArr.index(), frOther.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frOther);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frOther);
  loadFrameAddr(a64::x2, frArr);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_fastarray_append);
}

void Emitter::getGlobalObject(FR frRes) {
  comment("// GetGlobalObject r%u", frRes.index());
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(hwRes, a64::Mem(xRuntime, RuntimeOffsets::globalObject));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::declareGlobalVar(SHSymbolID symID) {
  comment("// DeclareGlobalVar %u", symID);

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, symID);
  EMIT_RUNTIME_CALL(
      *this, void (*)(SHRuntime *, SHSymbolID), _sh_ljs_declare_global_var);
}

void Emitter::createTopLevelEnvironment(FR frRes, uint32_t size) {
  comment("// CreateTopLevelEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, 0);
  a.mov(a64::w2, size);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *, uint32_t),
      _sh_ljs_create_environment);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::createFunctionEnvironment(FR frRes, uint32_t size) {
  comment("// CreateFunctionEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  // Allocate the result register.
  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  auto xRes = hwRes.a64GpX();

  // Allocate some temporaries.
  HWReg hwTemp1 = allocTempGpX();
  HWReg hwTemp2 = allocTempGpX();
  auto hwTempV = allocTempVecD();
  auto xTemp1 = hwTemp1.a64GpX();
  auto xTemp2 = hwTemp2.a64GpX();
  auto vTemp = hwTempV.a64VecD().v();
  freeReg(hwTemp1);
  freeReg(hwTemp2);
  freeReg(hwTempV);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  // Try to allocate the new environment cell.
  allocInYoung(
      CellKind::EnvironmentKind,
      Environment::allocationSize(size),
      xRes,
      xTemp1,
      xTemp2,
      slowPathLab);

  // Get the current closure pointer.
  a.ldur(
      xTemp1,
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::CalleeClosureOrCB *
              (int)sizeof(SHLegacyValue)));
  emit_sh_ljs_get_pointer(a, xTemp1, xTemp1);

  // xTemp1 = closure->environment
  emit_load_cp(a, xTemp1, a64::Mem(xTemp1, offsetof(SHCallable, environment)));

  // Initialize the environment cell.
  emit_environment_init(a, xRes, /* xParentEnv */ xTemp1, xTemp2, vTemp, size);

  // Encode the cell as a HermesValue.
  emit_sh_ljs_object(a, xRes);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwRes,
       .sizeOrIdx = size,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: CreateFunctionEnvironment r%u", sl.frRes.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.a.mov(a64::w2, sl.sizeOrIdx);

         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
             _sh_ljs_create_function_environment);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::createEnvironment(FR frRes, FR frParent, uint32_t size) {
  comment(
      "// CreateEnvironment r%u, r%u, %u",
      frRes.index(),
      frParent.index(),
      size);

  syncAllFRTempExcept(frRes != frParent ? frRes : FR{});
  syncToFrame(frParent);
  auto hwParent = getOrAllocFRInGpX(frParent, true);
  auto hwTemp1 = allocTempGpX();
  auto hwTemp2 = allocTempGpX();
  auto hwNewEnvPtr = allocTempGpX();
  auto hwTempV = allocTempVecD();

  auto xTemp1 = hwTemp1.a64GpX();
  auto xTemp2 = hwTemp2.a64GpX();
  auto xNewEnvPtr = hwNewEnvPtr.a64GpX();
  auto vTemp = hwTempV.a64VecD().v();

  freeReg(hwTemp1);
  freeReg(hwTemp2);
  freeReg(hwNewEnvPtr);
  freeReg(hwTempV);

  freeAllFRTempExcept({});

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  allocInYoung(
      CellKind::EnvironmentKind,
      Environment::allocationSize(size),
      xNewEnvPtr,
      xTemp1,
      xTemp2,
      slowPathLab);

  // Load a compressed pointer to the parent environment in xTemp1.
  emit_sh_ljs_get_pointer(a, xTemp1, hwParent.a64GpX());
  emit_sh_cp_encode_non_null(a, xTemp1);

  emit_environment_init(
      a, xNewEnvPtr, /* xParentEnv */ xTemp1, xTemp2, vTemp, size);

  // Finally, allocate the result register.
  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);

  emit_sh_ljs_object2(a, hwRes.a64GpX(), xNewEnvPtr);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frParent,
       .hwRes = hwRes,
       .sizeOrIdx = size,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: CreateEnvironment r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);

         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.a.mov(a64::w2, sl.sizeOrIdx);

         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *, uint32_t),
             _sh_ljs_create_environment);

         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::getParentEnvironment(FR frRes, uint32_t level) {
  comment("// GetParentEnvironment r%u, %u", frRes.index(), level);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  a64::GpX xRes = hwRes.a64GpX();
  frUpdatedWithHW(frRes, hwRes);

  // Get current closure.
  a.ldur(
      xRes,
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::CalleeClosureOrCB *
              (int)sizeof(SHLegacyValue)));
  // get pointer.
  emit_sh_ljs_get_pointer(a, xRes, xRes);
  // xRes = closure->environment
  emit_load_cp(a, xRes, a64::Mem(xRes, offsetof(SHCallable, environment)));
  emit_sh_cp_decode_non_null(a, xRes);
  for (; level; --level) {
    // xRes = env->parent.
    emit_load_cp(
        a, xRes, a64::Mem(xRes, offsetof(SHEnvironment, parentEnvironment)));
    emit_sh_cp_decode_non_null(a, xRes);
  }
  // encode object.
  emit_sh_ljs_object(a, xRes);
}

void Emitter::getEnvironment(FR frRes, FR frSource, uint32_t level) {
  comment(
      "// GetEnvironment r%u, r%u, %u", frRes.index(), frSource.index(), level);

  HWReg hwSource = getOrAllocFRInGpX(frSource, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes);
  a64::GpX xRes = hwRes.a64GpX();

  emit_sh_ljs_get_pointer(a, xRes, hwSource.a64GpX());
  for (; level; --level) {
    // xRes = env->parent.
    emit_load_cp(
        a, xRes, a64::Mem(xRes, offsetof(SHEnvironment, parentEnvironment)));
    emit_sh_cp_decode_non_null(a, xRes);
  }
  // encode object.
  emit_sh_ljs_object(a, xRes);
}

void Emitter::getClosureEnvironment(FR frRes, FR frClosure) {
  comment(
      "// GetClosureEnvironment r%u, r%u", frRes.index(), frClosure.index());
  // We know the layout of the closure, so we can load directly.
  auto ofs = offsetof(SHCallable, environment);
  auto hwClosure = getOrAllocFRInGpX(frClosure, true);
  auto hwRes = getOrAllocFRInGpX(frRes, false);
  // Use the result register as a scratch register for computing the address.
  emit_sh_ljs_get_pointer(a, hwRes.a64GpX(), hwClosure.a64GpX());
  emit_load_cp(a, hwRes.a64GpX(), a64::Mem(hwRes.a64GpX(), ofs));
  emit_sh_cp_decode_non_null(a, hwRes.a64GpX());
  // The result is a pointer, so add the object tag.
  emit_sh_ljs_object(a, hwRes.a64GpX());
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::loadFromEnvironment(FR frRes, FR frEnv, uint32_t slot) {
  comment(
      "// LoadFromEnvironment r%u, r%u, %u",
      frRes.index(),
      frEnv.index(),
      slot);

  // TODO: register allocation could be smarter if frRes !=- frEnv.

  HWReg hwTmp1 = allocTempGpX();
  a64::GpX xTmp1 = hwTmp1.a64GpX();

  movHWFromFR(hwTmp1, frEnv);
  // get pointer.
  emit_sh_ljs_get_pointer(a, xTmp1, xTmp1);

  a.ldr(
      xTmp1,
      a64::Mem(
          xTmp1,
          offsetof(SHEnvironment, slots) + sizeof(SHLegacyValue) * slot));

  freeReg(hwTmp1);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, hwTmp1);
  movHWFromHW<false>(hwRes, hwTmp1);
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::storeToEnvironment(bool np, FR frEnv, uint32_t slot, FR frValue) {
  // TODO: this should really be inlined!
  comment(
      "// StoreNPToEnvironment r%u, %u, r%u",
      frEnv.index(),
      slot,
      frValue.index());

  // Here we apply a technique that may be subtle. We have various FRs that we
  // want to load into parameter registers (x0, x1, etc) by value. Some of these
  // FRs may live in the parameter registers we want to use, but some may not.
  // So, first we make sure that the FRs that live in x0, x1, etc., are synced
  // to their primary location and the temps x0, x1, etc., are freed.
  // As we do this, we immediately move the corresponding parameter from its
  // corresponding FR, to maximize the chance that it can be moved from a
  // register.

  // Make sure x0, x1, x2, x3 are unused.
  syncAndFreeTempReg(HWReg::gpX(0));
  a.mov(a64::x0, xRuntime);

  syncAndFreeTempReg(HWReg::gpX(1));
  movHWFromFR(HWReg::gpX(1), frEnv);

  syncAndFreeTempReg(HWReg::gpX(2));
  movHWFromFR(HWReg::gpX(2), frValue);

  syncAndFreeTempReg(HWReg::gpX(3));
  a.mov(a64::w3, slot);

  // Make sure all FRs can be accessed. Some of them might be in temp regs.
  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  if (np) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue, SHLegacyValue, uint32_t),
        _sh_ljs_store_np_to_env);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue, SHLegacyValue, uint32_t),
        _sh_ljs_store_to_env);
  }
}

void Emitter::createClosure(
    FR frRes,
    FR frEnv,
    RuntimeModule *runtimeModule,
    uint32_t functionID) {
  comment(
      "// CreateClosure r%u, r%u, %u",
      frRes.index(),
      frEnv.index(),
      functionID);
  syncAllFRTempExcept(frRes != frEnv ? frRes : FR());
  syncToFrame(frEnv);

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frEnv);
  loadBits64InGp(a64::x2, (uint64_t)runtimeModule, "RuntimeModule");
  loadBits64InGp(a64::w3, functionID, nullptr);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, const SHLegacyValue *, SHRuntimeModule *, uint32_t),
      _sh_ljs_create_bytecode_closure);

  freeAllFRTempExcept({});
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::createBaseClass(FR frRes, FR frPrototypeOut, FR frEnv) {
  comment(
      "// CreateBaseClass r%u, r%u, r%u",
      frRes.index(),
      frPrototypeOut.index(),
      frEnv.index());
  // TODO: we should also not be syncing frPrototypeOut when possible.
  syncAllFRTempExcept(frRes != frEnv ? frRes : FR());
  syncToFrame(frEnv);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  // The interpreter expects that the frameRegs it receives starts on the first
  // local register.
  auto ofs = hbc::StackFrameLayout::FirstLocal * sizeof(SHLegacyValue);
  a.add(a64::x1, xFrame, ofs);
  EMIT_RUNTIME_CALL(
      *this, void (*)(SHRuntime *, SHLegacyValue *), _interpreter_create_class);

  // Ensure that the out params have their frame location marked as up-to-date,
  // and any global register is updated.
  syncFrameOutParam(frRes);
  syncFrameOutParam(frPrototypeOut);
}

void Emitter::createDerivedClass(
    FR frRes,
    FR frPrototypeOut,
    FR frEnv,
    FR frSuperClass) {
  comment(
      "// CreateDerivedClass r%u, r%u, r%u r%u",
      frRes.index(),
      frPrototypeOut.index(),
      frEnv.index(),
      frSuperClass.index());
  // TODO: we should also not be syncing frPrototypeOut when possible.
  syncAllFRTempExcept(frRes != frEnv && frRes != frSuperClass ? frRes : FR());
  syncToFrame(frEnv);
  syncToFrame(frSuperClass);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  // The interpreter expects that the frameRegs it receives starts on the first
  // local register.
  auto ofs = hbc::StackFrameLayout::FirstLocal * sizeof(SHLegacyValue);
  a.add(a64::x1, xFrame, ofs);
  EMIT_RUNTIME_CALL(
      *this, void (*)(SHRuntime *, SHLegacyValue *), _interpreter_create_class);

  // Ensure that the updated frame location is sync'd back.
  syncFrameOutParam(frRes);
  syncFrameOutParam(frPrototypeOut);
}

void Emitter::createGenerator(
    FR frRes,
    FR frEnv,
    RuntimeModule *runtimeModule,
    uint32_t functionID) {
  comment(
      "// CreateGenerator r%u, r%u, %u",
      frRes.index(),
      frEnv.index(),
      functionID);
  syncAllFRTempExcept(frRes != frEnv ? frRes : FR());
  syncToFrame(frEnv);

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  loadFrameAddr(a64::x2, frEnv);
  loadBits64InGp(a64::x3, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(a64::w4, functionID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          const SHLegacyValue *,
          SHRuntimeModule *,
          uint32_t),
      _interpreter_create_generator);

  freeAllFRTempExcept({});
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getArgumentsLength(FR frRes, FR frLazyReg) {
  comment("// GetArgumentsLength r%u, r%u", frRes.index(), frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept(frRes != frLazyReg ? frRes : FR());
  syncToFrame(frLazyReg);

  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwTemp = allocTempGpX();
  freeAllFRTempExcept({});
  freeReg(hwTemp);
  // Avoid an extra mov by using the temp register for the result if possible.
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes);

  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwLazyReg.a64GpX());
  a.b_eq(slowPathLab);

  // Fast path: if it's not an object, read from the frame.
  static_assert(
      HERMESVALUE_VERSION == 2,
      "NativeUint32 is stored as the lower 32 bits of the raw HermesValue");
  a.ldur(
      hwTemp.a64GpX().w(),
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));

  // Encode the uint32_t as a double (making it a HermesValue).
  a.ucvtf(hwRes.a64VecD(), hwTemp.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frLazyReg,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: GetArgumentsLength r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
             _sh_ljs_get_arguments_length);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::createThis(
    FR frRes,
    FR frCallee,
    FR frNewTarget,
    uint8_t cacheIdx) {
  comment(
      "// CreateThis r%u, r%u, r%u, cache %u",
      frRes.index(),
      frCallee.index(),
      frNewTarget.index(),
      cacheIdx);

  syncAllFRTempExcept(frRes != frCallee && frRes != frNewTarget ? frRes : FR());
  syncToFrame(frCallee);
  syncToFrame(frNewTarget);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frCallee);
  loadFrameAddr(a64::x2, frNewTarget);
  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x3, 0);
  } else {
    a.ldr(a64::x3, a64::Mem(roDataLabel_, roOfsReadPropertyCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x3, a64::x3, sizeof(SHReadPropertyCacheEntry) * cacheIdx);
  }
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHReadPropertyCacheEntry *),
      _sh_ljs_create_this);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::selectObject(FR frRes, FR frThis, FR frConstructed) {
  comment(
      "// SelectObject r%u, r%u, r%u",
      frRes.index(),
      frThis.index(),
      frConstructed.index());

  HWReg hwConstructed = getOrAllocFRInGpX(frConstructed, true);
  HWReg hwThis = getOrAllocFRInGpX(frThis, true);

  // Check if frConstructed is an object.
  // Get the tag bits in xTmpConstructedTag by right shifting.
  HWReg hwTmpConstructedTag = allocTempGpX();
  emit_sh_ljs_is_object(
      a, hwTmpConstructedTag.a64GpX(), hwConstructed.a64GpX());
  freeReg(hwTmpConstructedTag);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  // If it is an object, use Constructed, otherwise use This.
  // Store result in hwRes.
  a.csel(
      hwRes.a64GpX(),
      hwConstructed.a64GpX(),
      hwThis.a64GpX(),
      asmjit::arm::CondCode::kEQ);

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::loadThisNS(FR frRes) {
  comment("// LoadThisNS r%u", frRes.index());
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);

  // Load the ThisArg from the stack.
  a.ldur(
      hwRes.a64GpX(),
      a64::Mem(xFrame, StackFrameLayout::ThisArg * (int)sizeof(SHLegacyValue)));
  // If it is an object, we are done.
  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwRes.a64GpX());
  a.b_ne(slowPathLab);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: LoadThisNS r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.ldur(
             a64::x1,
             a64::Mem(
                 xFrame,
                 StackFrameLayout::ThisArg * (int)sizeof(SHLegacyValue)));
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, SHLegacyValue),
             _sh_ljs_coerce_this_ns);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::coerceThisNS(FR frRes, FR frThis) {
  comment("// CoerceThisNS r%u, r%u", frRes.index(), frThis.index());
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  HWReg hwThis = getOrAllocFRInGpX(frThis, true);

  // Sync all registers. Note that we don't need to check for frRes == frThis
  // here, because frThis is sync'd unconditionally below.
  syncAllFRTempExcept(frRes);
  syncToFrame(frThis);

  // Allocate a temporary register. This must not be the same as hwThis, but may
  // be the same as hwRes.
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);

  // We don't free frRes so that if it is the same as frThis, the register is
  // simply persisted and we do not need to perform a move in the fast path.
  freeAllFRTempExcept(frRes);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  // If the operand is an object, we are done, otherwise, go to the slow path.
  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwThis.a64GpX());
  a.b_ne(slowPathLab);

  movHWFromHW<false>(hwRes, hwThis);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frThis,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: CoerceThis r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em._loadFrame(HWReg(a64::x1), sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, SHLegacyValue),
             _sh_ljs_coerce_this_ns);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::debugger() {
  comment("// Debugger");
  if (dumpJitCode_ & DumpJitCode::BRK)
    a.brk(0);
}

void Emitter::getNewTarget(FR frRes) {
  comment("// GetNewTarget r%u", frRes.index());
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  a.ldur(
      hwRes.a64GpX(),
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::NewTarget * (int)sizeof(SHLegacyValue)));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::iteratorBegin(FR frRes, FR frSource) {
  comment("// IteratorBegin r%u, r%u", frRes.index(), frSource.index());

  syncAllFRTempExcept(frRes != frSource ? frRes : FR());
  syncToFrame(frSource);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *),
      _sh_ljs_iterator_begin_rjs);

  syncFrameOutParam(frSource);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::iteratorNext(FR frRes, FR frIteratorOrIdx, FR frSourceOrNext) {
  comment(
      "// IteratorNext r%u, r%u, r%u",
      frRes.index(),
      frIteratorOrIdx.index(),
      frSourceOrNext.index());

  syncAllFRTempExcept(
      frRes != frIteratorOrIdx && frRes != frSourceOrNext ? frRes : FR());
  syncToFrame(frIteratorOrIdx);
  syncToFrame(frSourceOrNext);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frIteratorOrIdx);
  loadFrameAddr(a64::x2, frSourceOrNext);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, const SHLegacyValue *),
      _sh_ljs_iterator_next_rjs);

  syncFrameOutParam(frIteratorOrIdx);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::iteratorClose(FR frIteratorOrIdx, bool ignoreExceptions) {
  comment(
      "// IteratorClose r%u, %u", frIteratorOrIdx.index(), ignoreExceptions);

  syncAllFRTempExcept({});
  syncToFrame(frIteratorOrIdx);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frIteratorOrIdx);
  a.mov(a64::w2, ignoreExceptions);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, const SHLegacyValue *, bool),
      _sh_ljs_iterator_close_rjs);
}

void Emitter::throwInst(FR frInput) {
  comment("// Throw r%u", frInput.index());

  // We have to sync registers when the throw is inside a try region
  // because we could read from the FRs again in this function.
  if (isInTry())
    syncAllFRTempExcept({});
  movHWFromFR(HWReg::gpX(1), frInput);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  EMIT_RUNTIME_CALL(*this, void (*)(SHRuntime *, SHLegacyValue), _sh_throw);
}

void Emitter::throwIfEmptyUndefinedImpl(FR frRes, FR frInput, bool empty) {
  comment(
      "// %s r%u, r%u",
      empty ? "ThrowIfEmpty" : "ThrowIfUndefined",
      frRes.index(),
      frInput.index());

  asmjit::Label slowPathLab = newSlowPathLabel();

  // We have to sync registers when the throw is inside a try region
  // because we could read from the FRs again in this function.
  if (isInTry())
    syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  if (isInTry())
    freeAllFRTempExcept({});
  freeReg(hwTemp);

  if (empty)
    emit_sh_ljs_is_empty(a, hwTemp.a64GpX(), hwInput.a64GpX());
  else
    emit_sh_ljs_is_undefined(a, hwTemp.a64GpX(), hwInput.a64GpX());
  a.b_eq(slowPathLab);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  movHWFromHW<false>(hwRes, hwInput);
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .name = empty ? "ThrowIfEmpty" : "ThrowIfUndefined",
       .frRes = frRes,
       .frInput1 = frInput,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(em, void (*)(SHRuntime *), _sh_throw_empty);
         // Call does not return.
       }});
}

void Emitter::throwIfThisInitialized(FR frInput) {
  comment("// ThrowIfThisInitialized r%u", frInput.index());

  asmjit::Label slowPathLab = newSlowPathLabel();

  // TODO: Add back the sync/free calls inside try.
  // Outside a try it's not observable behavior.
  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);

  emit_sh_ljs_is_empty(a, hwTemp.a64GpX(), hwInput.a64GpX());
  a.b_ne(slowPathLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .frInput1 = frInput,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: ThrowIfThisInitialized r%u", sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(
             em, void (*)(SHRuntime *), _sh_throw_this_already_initialized);
         // Call does not return.
       }});
}

void Emitter::createRegExp(
    FR frRes,
    SHSymbolID patternID,
    SHSymbolID flagsID,
    uint32_t regexpID) {
  comment("// CreateRegExp r%u, %u, %u", frRes.index(), patternID, flagsID);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, patternID);
  a.mov(a64::w3, flagsID);
  a.mov(a64::w4, regexpID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHCodeBlock *, uint32_t, uint32_t, uint32_t),
      _interpreter_create_regexp);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::loadParentNoTraps(FR frRes, FR frObj) {
  comment("// LoadParentNoTraps r%u, r%u", frRes.index(), frObj.index());

  HWReg hwObj = getOrAllocFRInGpX(frObj, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  HWReg hwTmp = allocTempGpX();
  freeReg(hwTmp);
  a64::GpX xTmp = hwTmp.a64GpX();
  a64::GpX xRes = hwRes.a64GpX();
  emit_sh_ljs_get_pointer(a, xTmp, hwObj.a64GpX());
  // xTmp contains the unencoded pointer value.
  emit_load_cp(a, xTmp, a64::Mem(xTmp, offsetof(SHJSObject, parent)));
  emit_sh_cp_decode(a, xTmp);
  // Check whether it is nullptr and set flags.
  // TODO: Combine this null check with the one in emit_load_and_sh_cp_decode.
  a.cmp(xTmp, 0);
  // xRes contains the encoded pointer.
  emit_sh_ljs_object2(a, xRes, xTmp);
  // xTmp contains encoded null.
  loadBits64InGp(xTmp, _sh_ljs_null().raw, "null");
  // If the pointer was nullptr, use encoded null, otherwise encoded ptr.
  a.csel(xRes, xTmp, xRes, asmjit::arm::CondCode::kEQ);

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::typedLoadParent(FR frRes, FR frObj) {
  comment("// TypedLoadParent r%u, r%u", frRes.index(), frObj.index());

  HWReg hwObj = getOrAllocFRInGpX(frObj, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  a64::GpX xRes = hwRes.a64GpX();
  emit_sh_ljs_get_pointer(a, xRes, hwObj.a64GpX());
  emit_load_cp(a, xRes, a64::Mem(xRes, offsetof(SHJSObject, parent)));
  emit_sh_cp_decode_non_null(a, xRes);
  emit_sh_ljs_object(a, xRes);

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::putByValImpl(
    FR frTarget,
    FR frKey,
    FR frValue,
    const char *name,
    void (*shImpl)(
        SHRuntime *shr,
        SHLegacyValue *target,
        SHLegacyValue *key,
        SHLegacyValue *value),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frTarget.index(),
      frKey.index(),
      frValue.index());

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  callThunkWithSavedIP((void *)shImpl, shImplName);
}

void Emitter::putByValWithReceiver(
    FR frTarget,
    FR frKey,
    FR frValue,
    FR frReceiver,
    bool isStrict) {
  comment(
      "// PutByValWithReceiver r%u, r%u, r%u, r%u, %d",
      frTarget.index(),
      frKey.index(),
      frValue.index(),
      frReceiver.index(),
      isStrict);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frValue);
  syncToFrame(frReceiver);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  loadFrameAddr(a64::x4, frReceiver);
  a.mov(a64::w5, isStrict);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *shr,
          SHLegacyValue *target,
          SHLegacyValue *key,
          SHLegacyValue *value,
          SHLegacyValue *receiver,
          bool isStrict),
      _sh_ljs_put_by_val_with_receiver_rjs);
}

void Emitter::delByVal(FR frRes, FR frTarget, FR frKey, bool strict) {
  comment(
      "// DelByVal r%u, r%u, r%u, %d",
      frRes.index(),
      frTarget.index(),
      frKey.index(),
      strict);

  syncAllFRTempExcept(frRes != frTarget && frRes != frKey ? frRes : FR{});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  if (strict) {
    EMIT_RUNTIME_CALL(
        *this,
        SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_del_by_val_strict);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_del_by_val_loose);
  }

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::addOwnPrivateBySym(FR frTarget, FR frKey, FR frValue) {
  comment(
      "// AddOwnPrivateBySym r%u, r%u, r%u",
      frTarget.index(),
      frKey.index(),
      frValue.index());

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_add_own_private_by_sym);
}

void Emitter::getOwnPrivateBySym(
    FR frRes,
    FR frTarget,
    FR frKey,
    uint8_t cacheIdx) {
  comment(
      "// GetOwnPrivateBySym r%u, r%u, r%u, cache %u",
      frRes.index(),
      frTarget.index(),
      frKey.index(),
      cacheIdx);

  syncAllFRTempExcept(frRes != frTarget && frRes != frKey ? frRes : FR());
  syncToFrame(frTarget);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);

  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x3, 0);
  } else {
    a.ldr(a64::x3, a64::Mem(roDataLabel_, roOfsPrivateNameCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x3, a64::x3, sizeof(SHPrivateNameCacheEntry) * cacheIdx);
  }

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          const SHLegacyValue *,
          const SHLegacyValue *,
          SHPrivateNameCacheEntry *),
      _sh_ljs_get_own_private_by_sym);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}
void Emitter::putOwnPrivateBySym(
    FR frTarget,
    FR frKey,
    FR frValue,
    uint8_t cacheIdx) {
  comment(
      "// PutOwnPrivateBySym r%u, r%u, r%u, cache %u",
      frTarget.index(),
      frKey.index(),
      frValue.index(),
      cacheIdx);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);

  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x4, 0);
  } else {
    a.ldr(a64::x4, a64::Mem(roDataLabel_, roOfsPrivateNameCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x4, a64::x4, sizeof(SHPrivateNameCacheEntry) * cacheIdx);
  }

  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHPrivateNameCacheEntry *),
      _sh_ljs_put_own_private_by_sym);
}

class HERMES_ATTRIBUTE_INTERNAL_LINKAGE Emitter::GetByIdImpl {
  Emitter &_;
  a64::Assembler &a;

  FR frRes;
  SHSymbolID symID;
  FR frSource;
  uint8_t cacheIdx;
  const char *name;
  SHLegacyValue (*shImpl)(
      SHRuntime *shr,
      const SHLegacyValue *source,
      SHSymbolID symID,
      SHReadPropertyCacheEntry *propCacheEntry);
  const char *shImplName;

  asmjit::Label contLab;
  asmjit::Label slowPathLab;
  HWReg hwRes;
  a64::GpX xTemp1;
  a64::GpX xTemp2;
  a64::GpX xTemp3;
  a64::GpX xTemp4;

 public:
  GetByIdImpl(
      Emitter &emitter,
      FR frRes,
      SHSymbolID symID,
      FR frSource,
      uint8_t cacheIdx,
      const char *name,
      SHLegacyValue (*shImpl)(
          SHRuntime *shr,
          const SHLegacyValue *source,
          SHSymbolID symID,
          SHReadPropertyCacheEntry *propCacheEntry),
      const char *shImplName)
      : _(emitter),
        a(emitter.a),
        frRes(frRes),
        symID(symID),
        frSource(frSource),
        cacheIdx(cacheIdx),
        name(name),
        shImpl(shImpl),
        shImplName(shImplName) {}

  void run() {
    _.comment(
        "// %s r%u, r%u, cache %u, symID %u",
        name,
        frRes.index(),
        frSource.index(),
        cacheIdx,
        symID);

    // All temporaries will potentially be clobbered by the slow path.
    _.syncAllFRTempExcept(frRes != frSource ? frRes : FR{});
    // Ensure the source register is in memory for the slow path.
    _.syncToFrame(frSource);

    if (cacheIdx != hbc::PROPERTY_CACHING_DISABLED) {
      slowPathLab = a.newLabel();
      contLab = a.newLabel();
      emitFastPath();
      a.bind(slowPathLab);
    } else {
      // All temporaries will be clobbered.
      _.freeAllFRTempExcept({});

      // Remember the result register.
      hwRes = _.getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
    }

    a.mov(a64::x0, xRuntime);
    _.loadFrameAddr(a64::x1, frSource);
    a.mov(a64::w2, symID);
    if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
      a.mov(a64::x3, 0);
    } else {
      a.ldr(a64::x3, a64::Mem(_.roDataLabel_, _.roOfsReadPropertyCachePtr_));
      if (cacheIdx != 0)
        a.add(a64::x3, a64::x3, sizeof(SHReadPropertyCacheEntry) * cacheIdx);
    }
    _.callThunkWithSavedIP((void *)shImpl, shImplName);

    _.movHWFromHW<false>(hwRes, HWReg::gpX(0));
    _.frUpdatedWithHW(frRes, hwRes);

    if (contLab.isValid())
      a.bind(contLab);
  }

 private:
  /// Load a SHV from a given slot in an object.
  ///
  /// \param resReg The register to load the SHV into.
  /// \param objReg The register containing the object pointer.
  /// \param tmpReg A temporary register which might be needed if the slot is
  ///   too large to fit in an immediate load. It must not be the same as
  ///   \p objReg, but can be the same as \p resReg.
  /// \param slot The slot index to load from the object.
  void emitLoadFromSlot(
      const a64::GpX &resReg,
      const a64::GpX &objReg,
      const a64::GpX &tmpReg,
      SlotIndex slot) {
    assert(tmpReg != objReg && "tmpReg and objReg must be different");
    if (slot < HERMESVM_DIRECT_PROPERTY_SLOTS) {
      emit_load_from_base_offset<sizeof(SHGCSmallHermesValue), true>(
          a,
          resReg,
          objReg,
          {},
          offsetof(SHJSObjectAndDirectProps, directProps) +
              slot * sizeof(SHGCSmallHermesValue));
    } else {
      emit_load_cp(
          a, objReg, a64::Mem(objReg, offsetof(SHJSObject, propStorage)));
      emit_sh_cp_decode_non_null(a, objReg);
      emit_load_from_base_offset<sizeof(SmallHermesValue), false>(
          a,
          resReg,
          objReg,
          tmpReg,
          offsetof(SHArrayStorageSmall, storage) +
              (slot - HERMESVM_DIRECT_PROPERTY_SLOTS) *
                  sizeof(SHGCSmallHermesValue));
    }
  }

  void emitFastPath() {
    // Label for indirect property access.
    asmjit::Label indirectLab = a.newLabel();

    // We don't need the other temporaries.
    _.freeAllFRTempExcept(frSource);

    // We need the source in a GPx register.
    HWReg hwSourceGpx = _.getOrAllocFRInGpX(frSource, true);

    // Here we start allocating and freeing registers. It is important to
    // realize that this doesn't generate any code, it only updates
    // metadata, marking registers as used or free. So, we have to perform a
    // series of register allocs and frees, ahead of time, based on our
    // understanding of how the live ranges of these registers overlap. Then
    // we just use the recorded registers at the right time.

    // xTemp1 will contain the input object.
    HWReg hwTemp1Gpx = _.allocTempGpX();
    xTemp1 = hwTemp1Gpx.a64GpX();
    // Free frSource before allocating more temporaries, because we won't
    // need it at the same time as them.
    _.freeFRTemp(frSource);

    // Get register assignments for the rest of the temporaries.
    HWReg hwTemp2Gpx = _.allocTempGpX();
    HWReg hwTemp3Gpx = _.allocTempGpX();
    HWReg hwTemp4Gpx = _.allocTempGpX();
    xTemp2 = hwTemp2Gpx.a64GpX();
    xTemp3 = hwTemp3Gpx.a64GpX();
    xTemp4 = hwTemp4Gpx.a64GpX();

    // Now that we have recorded their registers, mark all temp registers as
    // free.
    _.freeReg(hwTemp1Gpx);
    _.freeReg(hwTemp2Gpx);
    _.freeReg(hwTemp3Gpx);
    _.freeReg(hwTemp4Gpx);

    // Allocate the result register. Note that it can overlap the temps we
    // just freed.
    hwRes = _.getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));

    // Finally we begin code generation for the fast path.

    // Is the input an object.
    emit_sh_ljs_is_object(a, xTemp1, hwSourceGpx.a64GpX());
    a.b_ne(slowPathLab);
    // xTemp1 is the pointer to the object.
    emit_sh_ljs_get_pointer(a, xTemp1, hwSourceGpx.a64GpX());

    // xTemp2 is the hidden class.
    emit_load_cp(a, xTemp2, a64::Mem(xTemp1, offsetof(SHJSObject, clazz)));

    Emit_sh_shv_decode shvDecode(a, hwRes.a64GpX(), contLab);

    // Optionally emit a very fast path specialized for the cache entry, if the
    // entry had exactly one successful match.
    if (ReadPropertyCacheEntry *cacheEntry =
            _.codeBlock_->getReadCacheEntry(cacheIdx);
        cacheEntry->numGoodChanges == 1) {
      if (cacheEntry->clazz.getNoBarrierUnsafe() &&
          !cacheEntry->negMatchClazz.getNoBarrierUnsafe()) {
        ++JITNumGetByIdSpec;
        emitObjectSpecialization(shvDecode, cacheEntry);
      } else if (
          cacheEntry->clazz.getNoBarrierUnsafe() &&
          cacheEntry->negMatchClazz.getNoBarrierUnsafe()) {
        ++JITNumGetByIdSpec;
        emitParentSpecialization(shvDecode, cacheEntry);
        // We don't try other things after parent specialization.
        return;
      }
    }

    _.comment("// Read property cache");

    // xTemp3 points to the start of read property cache.
    a.ldr(xTemp3, a64::Mem(_.roDataLabel_, _.roOfsReadPropertyCachePtr_));
    // xTemp4 = cacheEntry->clazz.
    emit_load_cp(
        a,
        xTemp4,
        a64::Mem(
            xTemp3,
            sizeof(SHReadPropertyCacheEntry) * cacheIdx +
                offsetof(SHReadPropertyCacheEntry, clazz)));

    // Compare hidden classes.
    a.cmp(xTemp2, xTemp4);
    a.b_ne(slowPathLab);

    // Hidden class matches. Fetch the slot in xTemp4
    emit_load_slot16(a, xTemp4, xTemp3, cacheIdx);

    // Is it an indirect slot?
    a.cmp(xTemp4.w(), HERMESVM_DIRECT_PROPERTY_SLOTS);
    a.b_hs(indirectLab);

    // Shift by 2 or 3 bits depending on whether properties are 4 or 8
    // bytes.
    constexpr size_t kPropShiftAmt = sizeof(SHGCSmallHermesValue) == 4 ? 2 : 3;
    // Load from a direct slot.
    a.add(xTemp3, xTemp1, offsetof(SHJSObjectAndDirectProps, directProps));
    emit_load_shv(
        a,
        hwRes.a64GpX(),
        a64::Mem(
            xTemp3, xTemp4, a64::Shift(a64::ShiftOp::kLSL, kPropShiftAmt)));
    shvDecode.emitFirstCase(a);
    a.b(contLab);

    a.bind(indirectLab);
    // Load from an in-direct slot.
    // xTemp1 is the object
    // xTemp4 is the slot

    // xTemp1 = xTemp1->propStorage
    emit_load_cp(
        a, xTemp1, a64::Mem(xTemp1, offsetof(SHJSObject, propStorage)));
    emit_sh_cp_decode_non_null(a, xTemp1);
    constexpr ssize_t ofs = offsetof(SHArrayStorageSmall, storage) -
        HERMESVM_DIRECT_PROPERTY_SLOTS * sizeof(SHGCSmallHermesValue);
    if constexpr (ofs < 0)
      a.sub(xTemp1, xTemp1, -ofs);
    else
      a.add(xTemp1, xTemp1, ofs);
    emit_load_shv(
        a,
        hwRes.a64GpX(),
        a64::Mem(
            xTemp1, xTemp4, a64::Shift(a64::ShiftOp::kLSL, kPropShiftAmt)));
    shvDecode.emitAll(a);
    a.b(contLab);
  }

  /// Emit a specialization for accessing a property on the object.
  void emitObjectSpecialization(
      Emit_sh_shv_decode &shvDecode,
      ReadPropertyCacheEntry *cacheEntry) {
    // Obtain the HC ID.
    auto clazzID = _.initHCLazyIDMayAlloc(
        cacheEntry->clazz.getNoBarrierUnsafe(_.runtime_));
    if (!clazzID)
      return;

    _.comment("// Get from object specialization");

    asmjit::Label failSpecLab = a.newLabel();

    // Decode the HC compressed pointer.
    const auto &xReg =
        emit_sh_cp_decode_non_null_preserve_input(a, xTemp3, xTemp2);
    // xTemp3 = hc->lazyJITId
    a.ldrh(xTemp3.w(), a64::Mem(xReg, RuntimeOffsets::hiddenClassLazyJITId));

    emit_cmp_imm32(a, xTemp3.w(), clazzID, xTemp4.w());
    a.b_ne(failSpecLab);
    // A match. Just load the property directly.
    emitLoadFromSlot(hwRes.a64GpX(), xTemp1, xTemp4, cacheEntry->getSlot());
    shvDecode.emitFirstCase(a);
    a.b(contLab);
    a.bind(failSpecLab);
  }

  /// Emit a specialization for accessing a property on the parent.
  /// Does not preserve temps. Assumes no fast path runs after it.
  void emitParentSpecialization(
      Emit_sh_shv_decode &shvDecode,
      ReadPropertyCacheEntry *cacheEntry) {
    // Obtain the HC IDs for the object's class and the parent class.
    auto clazzID = _.initHCLazyIDMayAlloc(
        cacheEntry->negMatchClazz.getNoBarrierUnsafe(_.runtime_));
    if (!clazzID)
      return;
    auto parentClsID = _.initHCLazyIDMayAlloc(
        cacheEntry->clazz.getNoBarrierUnsafe(_.runtime_));
    if (!parentClsID)
      return;

    _.comment("// Get from parent specialization");

    asmjit::Label failSpecLab = a.newLabel();

    emit_sh_cp_decode_non_null(a, xTemp2);
    // xTemp2 = hc->lazyJITId
    a.ldrh(xTemp2.w(), a64::Mem(xTemp2, RuntimeOffsets::hiddenClassLazyJITId));
    // if object class mismatch, fail.
    emit_cmp_imm32(a, xTemp2.w(), clazzID, xTemp4.w());
    a.b_ne(failSpecLab);

    // Get the parent.
    emit_load_cp(a, xTemp1, a64::Mem(xTemp1, offsetof(SHJSObject, parent)));
    // If no parent, fail.
    a.cbz(xTemp1, failSpecLab);
    emit_sh_cp_decode_non_null(a, xTemp1);
    // Get the parent's hidden class.
    emit_load_cp(a, xTemp2, a64::Mem(xTemp1, offsetof(SHJSObject, clazz)));
    emit_sh_cp_decode_non_null(a, xTemp2);
    // xTemp2 = hc->lazyJITId
    a.ldrh(xTemp2.w(), a64::Mem(xTemp2, RuntimeOffsets::hiddenClassLazyJITId));
    // if parent class mismatch, fail.
    emit_cmp_imm32(a, xTemp2.w(), parentClsID, xTemp4.w());
    a.b_ne(failSpecLab);
    // A match. Just load the property directly.
    emitLoadFromSlot(hwRes.a64GpX(), xTemp1, xTemp4, cacheEntry->getSlot());
    shvDecode.emitAll(a);
    a.b(contLab);
    a.bind(failSpecLab);
  }
};

void Emitter::getByIdImpl(
    FR frRes,
    SHSymbolID symID,
    FR frSource,
    uint8_t cacheIdx,
    const char *name,
    SHLegacyValue (*shImpl)(
        SHRuntime *shr,
        const SHLegacyValue *source,
        SHSymbolID symID,
        SHReadPropertyCacheEntry *propCacheEntry),
    const char *shImplName) {
  GetByIdImpl(*this, frRes, symID, frSource, cacheIdx, name, shImpl, shImplName)
      .run();
}

void Emitter::getByIdWithReceiver(
    FR frRes,
    SHSymbolID symID,
    FR frSource,
    FR frReceiver,
    uint8_t cacheIdx) {
  comment(
      "// GetByIdWithReceiver r%u, r%u, r%u, cache %u, symID %u",
      frRes.index(),
      frSource.index(),
      frReceiver.index(),
      cacheIdx,
      symID);

  // TODO: Add a fast path, probably by sharing code with getByIdImpl.

  syncAllFRTempExcept(frRes != frSource && frRes != frReceiver ? frRes : FR());
  syncToFrame(frSource);
  syncToFrame(frReceiver);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  loadFrameAddr(a64::x2, frReceiver);
  a.mov(a64::w3, symID);
  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x4, 0);
  } else {
    a.ldr(a64::x4, a64::Mem(roDataLabel_, roOfsReadPropertyCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x4, a64::x4, sizeof(SHReadPropertyCacheEntry) * cacheIdx);
  }
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime * shr,
          const SHLegacyValue *source,
          const SHLegacyValue *receiver,
          SHSymbolID symID,
          SHReadPropertyCacheEntry *propCacheEntry),
      _sh_ljs_get_by_id_with_receiver_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getByValWithReceiver(
    FR frRes,
    FR frSource,
    FR frKey,
    FR frReceiver) {
  comment(
      "// GetByValWithReceiver r%u, r%u, r%u, r%u",
      frRes.index(),
      frSource.index(),
      frReceiver.index(),
      frKey.index());

  syncAllFRTempExcept(
      frRes != frSource && frRes != frReceiver && frRes != frKey ? frRes
                                                                 : FR());
  syncToFrame(frSource);
  syncToFrame(frKey);
  syncToFrame(frReceiver);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frReceiver);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime * shr,
          SHLegacyValue * source,
          SHLegacyValue * key,
          SHLegacyValue * receiver),
      _sh_ljs_get_by_val_with_receiver_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::jmpTypeOfIs(
    const asmjit::Label &target,
    FR frInput,
    TypeOfIsTypes origTypes) {
  comment("// jTypeOfIs r%u, %u", frInput.index(), origTypes.getRaw());

  TypeOfIsTypes invertedTypes = origTypes.invert();

  // Do this always because it's the end of a basic block.
  // The freeAllFRTempExcept calls are within fast paths because we may want to
  // use FR temps to syncToFrame(frInput) in the call path, and we know at JIT
  // time whether we'll emit the call path.
  syncAllFRTempExcept({});

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);
  freeAllFRTempExcept({});

  auto xInput = hwInput.a64GpX();
  auto xTemp = hwTemp.a64GpX();
  auto wTemp = xTemp.w();

  // Try and see if inverting will result in fewer checks.
  // If so, flip it and set invert=true.
  bool invert = false;
  TypeOfIsTypes typesToCheck = origTypes;
  if (invertedTypes.count() < origTypes.count()) {
    invert = true;
    typesToCheck = invertedTypes;
  }

  // doneLab goes at the end of the instruction if there's multiple bits to
  // check, allowing short-circuiting the remaining checks if one of the
  // TypeOfIsTypes bits matches the kind of the input.
  // Use numRemainingTypes to track how many bits are left to check.
  asmjit::Label doneLab = a.newLabel();
  size_t numRemainingTypes = typesToCheck.count();

  // Checks are done as follows:
  // * If not inverted, just go to the target if the tag matches the bit,
  //   else fallthrough to the next case (if any).
  // * If inverted and there's multiple bits remaining,
  //   if the tag matches the bit, short circuit to doneLab and we've
  //   finished executing the instruction (no need to check the other bits).
  // * If inverted and there's only one bit remaining,
  //   then if the tag does NOT match the bit, go to the target
  //   immediately.
  //
  // In this way, single-bit checks (both inverted and not) are fast,
  // and multiple-bit checks are correct.
  // It's possible more complexity can optimize this further if needed, but this
  // is not a bad start.

  /// Emit the simple check for a match.
  /// If we're not inverted, branch to the target based on cond.
  /// If we're inverted:
  ///   If there's bits remaining to check, branch to doneLab if the tag matches
  ///   because we can short circuit the rest of the checks.
  ///   If there's no bits remaining to check, branch to the target if the tag
  ///   does NOT match the bit.
  /// \param cond the condition code, which if true, indicates a tag match.
  auto emitCondCheck = [this, invert, &numRemainingTypes, &target, &doneLab](
                           a64::CondCode cond) {
    if (!invert)
      a.b(cond, target);
    else if (numRemainingTypes > 0)
      a.b(cond, doneLab);
    else
      a.b(a64::negateCond(cond), target);
  };

  if (typesToCheck.hasUndefined()) {
    --numRemainingTypes;
    emit_sh_ljs_is_undefined(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasSymbol()) {
    --numRemainingTypes;
    emit_sh_ljs_is_symbol(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasString()) {
    --numRemainingTypes;
    emit_sh_ljs_is_string(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasBoolean()) {
    --numRemainingTypes;
    emit_sh_ljs_is_bool(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasNull()) {
    --numRemainingTypes;
    emit_sh_ljs_is_null(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasBigint()) {
    --numRemainingTypes;
    emit_sh_ljs_is_bigint(a, xTemp, xInput);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasNumber()) {
    --numRemainingTypes;
    static_assert(
        HERMESVALUE_VERSION == 2,
        "HVTag_First must be the first after double limit");
    loadBits64InGp(
        xTemp, ((uint64_t)HVTag_First << kHV_NumDataBits), "doubleLim");
    a.cmp(xInput, xTemp);
    emitCondCheck(a64::CondCode::kLO);
  }
  // TODO: Special-case if both hasObject() and hasFunction() are set,
  // because we no longer would need to check the CellKind.
  if (typesToCheck.hasObject()) {
    --numRemainingTypes;
    asmjit::Label objectDoneLab = a.newLabel();
    emit_sh_ljs_is_object(a, xTemp, xInput);
    if (!invert)
      a.b_ne(objectDoneLab);
    else if (numRemainingTypes > 0)
      a.b_ne(objectDoneLab);
    else
      a.b_ne(target);
    emit_sh_ljs_get_pointer(a, hwTemp.a64GpX(), hwInput.a64GpX());
    emit_gccell_get_kind(a, xTemp, xTemp);
    emit_cellkind_in_range(
        a,
        wTemp,
        wTemp,
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
    emitCondCheck(a64::CondCode::kHI);
    a.bind(objectDoneLab);
  }
  if (typesToCheck.hasFunction()) {
    --numRemainingTypes;
    asmjit::Label functionDoneLab = a.newLabel();
    emit_sh_ljs_is_object(a, xTemp, xInput);
    if (!invert)
      a.b_ne(functionDoneLab);
    else if (numRemainingTypes > 0)
      a.b_ne(functionDoneLab);
    else
      a.b_ne(target);
    emit_sh_ljs_get_pointer(a, xTemp, xInput);
    emit_gccell_get_kind(a, xTemp, xTemp);
    emit_cellkind_in_range(
        a,
        wTemp,
        wTemp,
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
    emitCondCheck(a64::CondCode::kLS);
    a.bind(functionDoneLab);
  }

  assert(numRemainingTypes == 0 && "missed a type");

  // Put doneLab after, so we skip the branch if we directly branch to doneLab
  // from above.
  a.bind(doneLab);
}

void Emitter::typeOfIs(FR frRes, FR frInput, TypeOfIsTypes origTypes) {
  comment(
      "// typeOfIs r%u, r%u, %u",
      frRes.index(),
      frInput.index(),
      origTypes.getRaw());

  // Store the input in hwInputTemp for the duration of the instruction.
  // Needed because it's possible frRes == frInput, and we want to write to
  // frRes at the top of the instruction.
  HWReg hwInputTemp;
  if (frRes == frInput) {
    hwInputTemp = allocTempGpX();
    movHWFromFR(hwInputTemp, frInput);
  } else {
    hwInputTemp = getOrAllocFRInGpX(frInput, true);
  }
  HWReg hwTemp = allocTempGpX();
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes);
  freeReg(hwTemp);
  if (frRes == frInput) {
    freeReg(hwInputTemp);
  }

  auto xInputTemp = hwInputTemp.a64GpX();
  auto xTemp = hwTemp.a64GpX();
  auto wTemp = xTemp.w();
  auto xRes = hwRes.a64GpX();

  TypeOfIsTypes invertedTypes = origTypes.invert();

  // Try and see if inverting will result in fewer checks.
  // If so, flip it and set invert=true.
  bool invert = false;
  TypeOfIsTypes typesToCheck = origTypes;
  if (invertedTypes.count() < origTypes.count()) {
    invert = true;
    typesToCheck = invertedTypes;
  }

  // matchLab goes directly to the end of the instruction if there are multiple
  // bits to check, allowing short-circuiting the remaining checks if one of the
  // TypeOfIsTypes bits matches the kind of the input.
  // If there's only one bit to check, we don't put extra code the end - none of
  // the other cases will be emitted.
  asmjit::Label matchLab{};
  if (typesToCheck.count() > 1)
    matchLab = a.newLabel();

  // First, initialize xRes if necessary:
  // * If there are multiple bits set, initialize it to the value we would
  //   produce on a match. This is false if inverted and true otherwise.
  // * If there's only one bit set, leave it uninitialized, since we will
  //   overwrite the value in the individual cases with cset.
  //
  // Checks are done as follows:
  // * If there are multiple bits set, then matchLab is valid,
  //   so if the tag matches the bit, branch to matchLab.
  //   If the tag doesn't match, then fall through to the next check.
  // * If there's only one bit set, then matchLab is NOT valid,
  //   so emit cset with the appropriate condition code and we're done.
  //
  // In this way, single-bit checks (both inverted and not) are fast,
  // and multiple-bit checks are correct.

  /// Emit the simple check for a match.
  /// If there's multiple bits to check, this will branch based on \p cond
  /// to matchLab if the tag matches.
  /// If there's only one bit to check, this will emit a cinc with the
  /// appropriate condition code (and we're done).
  /// \param cond the condition code, which if true, indicates a tag match.
  auto emitCondCheck = [this, invert, &xRes, &matchLab](a64::CondCode cond) {
    if (matchLab.isValid())
      a.b(cond, matchLab);
    else
      a.cset(xRes, !invert ? cond : a64::negateCond(cond));
  };

  // As described above, if there are multiple cases, initialize it to the value
  // it should have on a successful match.
  if (matchLab.isValid())
    a.mov(xRes, invert ? 0 : 1);

  if (typesToCheck.hasUndefined()) {
    emit_sh_ljs_is_undefined(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasSymbol()) {
    emit_sh_ljs_is_symbol(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasString()) {
    emit_sh_ljs_is_string(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasBoolean()) {
    emit_sh_ljs_is_bool(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasBigint()) {
    emit_sh_ljs_is_bigint(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasNull()) {
    emit_sh_ljs_is_null(a, xTemp, xInputTemp);
    emitCondCheck(a64::CondCode::kEQ);
  }
  if (typesToCheck.hasNumber()) {
    static_assert(
        HERMESVALUE_VERSION == 2,
        "HVTag_First must be the first after double limit");
    loadBits64InGp(
        xTemp, ((uint64_t)HVTag_First << kHV_NumDataBits), "doubleLim");
    a.cmp(xInputTemp, xTemp);
    emitCondCheck(a64::CondCode::kLO);
  }
  if (typesToCheck.hasObject()) {
    asmjit::Label objectDoneLab = a.newLabel();
    emit_sh_ljs_is_object(a, xTemp, xInputTemp);
    if (matchLab.isValid()) {
      // If the tag did NOT match, we can't run anything else in this case.
      // We must branch, b_ne and proceed to try matching any other cases.
      a.b_ne(objectDoneLab);
    } else {
      // No more tags to check. Decide the result here and go to the end.
      a.mov(xRes, invert ? 1 : 0);
      a.b_ne(objectDoneLab);
    }
    emit_sh_ljs_get_pointer(a, xTemp, xInputTemp);
    emit_gccell_get_kind(a, xTemp, xTemp);
    emit_cellkind_in_range(
        a,
        wTemp,
        wTemp,
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
    emitCondCheck(a64::CondCode::kHI);
    a.bind(objectDoneLab);
  }
  if (typesToCheck.hasFunction()) {
    asmjit::Label functionDoneLab = a.newLabel();
    emit_sh_ljs_is_object(a, xTemp, xInputTemp);
    if (matchLab.isValid()) {
      // If the tag did NOT match, we can't run anything else in this case.
      // We must branch, b_ne and proceed to try matching any other cases.
      a.b_ne(functionDoneLab);
    } else {
      // No more tags to check. Decide the result here and go to the end.
      a.mov(xRes, invert ? 1 : 0);
      a.b_ne(functionDoneLab);
    }
    emit_sh_ljs_get_pointer(a, xTemp, xInputTemp);
    emit_gccell_get_kind(a, xTemp, xTemp);
    emit_cellkind_in_range(
        a,
        wTemp,
        wTemp,
        CellKind::CallableKind_first,
        CellKind::CallableKind_last);
    emitCondCheck(a64::CondCode::kLS);
    a.bind(functionDoneLab);
  }

  if (matchLab.isValid()) {
    // We failed to match, so flip the result
    a.eor(xRes, xRes, 1);
    // We initialize xRes to the "match value", so there is nothing to do on a
    // match.
    a.bind(matchLab);
  }

  // xRes contains either 0 or 1 at this point, turn it into a bool HermesValue.
  emit_sh_ljs_bool(a, xRes);
}

void Emitter::uintSwitchImm(
    FR frInput,
    const asmjit::Label &defaultLabel,
    llvh::ArrayRef<const asmjit::Label *> labels,
    uint32_t minVal,
    uint32_t maxVal) {
  comment(
      "// uintSwitchImm r%u, min %u, max %u", frInput.index(), minVal, maxVal);

  asmjit::Error err;

  // End of the basic block.
  syncAllFRTempExcept({});

  // Load the input value into a double register to check if it's an int.
  HWReg hwInput = getOrAllocFRInVecD(frInput, true);

  HWReg hwTempInput = allocTempGpX();
  HWReg hwTempTarget = allocTempGpX();
  HWReg hwTempD = allocTempVecD();
  freeReg(hwTempInput);
  freeReg(hwTempTarget);
  freeReg(hwTempD);

  a64::VecD dInput = hwInput.a64VecD();
  a64::GpW wTempInput = hwTempInput.a64GpX().w();

  // Convert the input to an integer and back to double,
  // and check if the value remained the same.
  // If it didn't, jump to the default label.
  emit_double_is_uint32(a, wTempInput, hwTempD.a64VecD(), dInput);
  a.b_ne(defaultLabel);

  // Check if the integer value in xTemp is in range.
  // First check minVal.
  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTempInput, minVal));
  if (err) {
    a.mov(hwTempTarget.a64GpX().w(), minVal);
    a.cmp(wTempInput, hwTempTarget.a64GpX().w());
  }
  // If the value is lower than minVal, jump to the default label.
  a.b_lo(defaultLabel);

  // Now check maxVal.
  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTempInput, maxVal));
  if (err) {
    a.mov(hwTempTarget.a64GpX().w(), maxVal);
    a.cmp(wTempInput, hwTempTarget.a64GpX().w());
  }
  // If the value is higher than maxVal, jump to the default label.
  a.b_hi(defaultLabel);

  // Compute the offset into the jump table, dereference, and jump.
  // Offset by the minVal if necessary.
  if (minVal != 0) {
    EXPECT_ERROR(
        asmjit::kErrorInvalidImmediate,
        err = a.sub(wTempInput, wTempInput, minVal));
    if (err) {
      a.mov(hwTempTarget.a64GpX().w(), minVal);
      a.sub(wTempInput, wTempInput, hwTempTarget.a64GpX().w());
    }
  }

  // Label for the start of the jump table and the base of the br instruction
  // that actually executes the switch.
  // Used for both purposes due to placement of the jump table directly after
  // the br.
  asmjit::Label tableLab = a.newLabel();

  // wTempInput contains the index into the jump table.
  a64::GpX xTempTarget = hwTempTarget.a64GpX();
  // Load the jump offset into wTempInput by using adr to find the address of
  // the table and then reading 4 bytes from an offset of wTempInput bytes.
  a.adr(xTempTarget, tableLab);
  // Left shift 2 to get the byte offset into the table.
  a.ldr(
      wTempInput,
      a64::Mem(xTempTarget, wTempInput, a64::Shift(a64::ShiftOp::kLSL, 2)));
  // Add the jump offset to the base of the table to get the target address.
  a.add(xTempTarget, xTempTarget, wTempInput.x(), a64::sxtw(0));
  // Branch to the target address.
  a.br(xTempTarget);

  // Emit the jump table.
  // NOTE: The jump table is emitted immediately after the br instruction that
  // uses it.
  a.bind(tableLab);
  for (const asmjit::Label *label : labels) {
    a.embedLabelDelta(*label, tableLab, /* size */ 4);
  }

  // Do this always, since this could be the end of the BB.
  freeAllFRTempExcept({});
}

void Emitter::stringSwitchImm(
    FR frInput,
    const StringSwitchDenseMap &table,
    const asmjit::Label &defaultLabel,
    llvh::ArrayRef<StringSwitchCase> cases) {
  comment("// stringSwitchImm r%u, size %zu", frInput.index(), cases.size());

  // End of the basic block.
  syncAllFRTempExcept({});

  a.mov(a64::x0, &table);
  loadFrameAddr(a64::x1, frInput);

  EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
      *this,
      void *(*)(StringSwitchDenseMap *, SHLegacyValue *),
      _jit_string_switch_imm_table_lookup);

  a.cbz(a64::x0, defaultLabel);
  // Otherwise, branch to the address that was returned.
  a.br(a64::x0);

  // Do this always, since this could be the end of the BB.
  freeAllFRTempExcept({});
}

void Emitter::getByVal(FR frRes, FR frSource, FR frKey) {
  comment(
      "// getByVal r%u, r%u, r%u",
      frRes.index(),
      frSource.index(),
      frKey.index());

  syncAllFRTempExcept(frRes != frSource && frRes != frKey ? frRes : FR());
  syncToFrame(frSource);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  loadFrameAddr(a64::x2, frKey);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_get_by_val_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getByIndex(FR frRes, FR frSource, uint32_t key) {
  comment("// getByIdx r%u, r%u, %u", frRes.index(), frSource.index(), key);

  syncAllFRTempExcept(frRes != frSource ? frRes : FR());
  syncToFrame(frSource);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  a.mov(a64::w2, key);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _sh_ljs_get_by_index_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::putByIdImpl(
    FR frTarget,
    SHSymbolID symID,
    FR frValue,
    uint8_t cacheIdx,
    bool strictMode,
    bool tryProp) {
  comment(
      "// %sPutById%s r%u, r%u, cache %u, symID %u",
      tryProp ? "Try" : "",
      strictMode ? "Strict" : "Loose",
      frTarget.index(),
      frValue.index(),
      cacheIdx,
      symID);

  // New non-dictionary objects must not have their propStorage_ capacity
  // stored out of line, because we want to be able to quickly get property
  // storage capacity for JIT's cached add property fast path.
  // Ensure this by making sure that objects will be turned into dictionary
  // mode before leaving the single young gen segment.
  // TODO: Actually implement the fast path. This assert just keeps it possible.
  static_assert(
      HiddenClass::kDictionaryThreshold <
          JSObject::maxYoungGenAllocationPropCount(),
      "dictionary objects must be allocated in a single segment in young gen");

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  loadFrameAddr(a64::x2, frTarget);
  loadFrameAddr(a64::x3, frValue);
  a.mov(a64::w4, cacheIdx);
  a.mov(a64::w5, symID);
  a.mov(a64::w6, strictMode);
  a.mov(a64::w7, tryProp);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *shr,
          SHCodeBlock *codeBlock,
          SHLegacyValue *base,
          SHLegacyValue *value,
          uint8_t cacheIdx,
          SHSymbolID symID,
          bool strictMode,
          bool tryProp),
      _jit_put_by_id);
}

asmjit::Label Emitter::newPrefLabel(const char *pref, size_t index) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%s%lu", pref, index);
  return a.newNamedLabel(buf);
}

void Emitter::defineOwnById(
    FR frTarget,
    SHSymbolID symID,
    FR frValue,
    uint8_t cacheIdx) {
  comment(
      "// defineOwnById r%u, r%u, cache %u, symID %u",
      frTarget.index(),
      frValue.index(),
      cacheIdx,
      symID);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, symID);
  loadFrameAddr(a64::x3, frValue);
  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x4, 0);
  } else {
    a.ldr(a64::x4, a64::Mem(roDataLabel_, roOfsWritePropertyCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x4, a64::x4, sizeof(SHWritePropertyCacheEntry) * cacheIdx);
  }
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *shr,
          SHLegacyValue *target,
          SHSymbolID key,
          SHLegacyValue *value,
          SHWritePropertyCacheEntry *cacheEntrySHRuntime),
      _sh_ljs_define_own_by_id);
}

void Emitter::defineOwnByIndex(FR frTarget, FR frValue, uint32_t key) {
  comment(
      "// putOwnByIdx r%u, r%u, %u", frTarget.index(), frValue.index(), key);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, key);
  loadFrameAddr(a64::x3, frValue);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
      _sh_ljs_define_own_by_index);
}

void Emitter::defineOwnByVal(
    FR frTarget,
    FR frValue,
    FR frKey,
    bool enumerable) {
  comment(
      "// DefineOwnByVal r%u, r%u, r%u",
      frTarget.index(),
      frValue.index(),
      frKey.index());

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  if (enumerable) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_define_own_by_val);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_define_own_ne_by_val);
  }
}

void Emitter::defineOwnGetterSetterByVal(
    FR frTarget,
    FR frKey,
    FR frGetter,
    FR frSetter,
    bool enumerable) {
  comment(
      "// DefineOwnGetterSetterByVal r%u, r%u, r%u, r%u, %d",
      frTarget.index(),
      frKey.index(),
      frGetter.index(),
      frSetter.index(),
      enumerable);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frGetter);
  syncToFrame(frSetter);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frGetter);
  loadFrameAddr(a64::x4, frSetter);
  a.mov(a64::w5, enumerable);

  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *shr,
          SHLegacyValue *target,
          SHLegacyValue *key,
          SHLegacyValue *getter,
          SHLegacyValue *setter,
          bool enumerable),
      _sh_ljs_define_own_getter_setter_by_val);
}

void Emitter::getOwnBySlotIdx(FR frRes, FR frTarget, uint32_t slotIdx) {
  comment(
      "// GetOwnBySlotIdx r%u, r%u, %u",
      frRes.index(),
      frTarget.index(),
      slotIdx);

  HWReg hwTarget = getOrAllocFRInGpX(frTarget, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes);
  a64::GpX xRes = hwRes.a64GpX();

  size_t ofs;
  emit_sh_ljs_get_pointer(a, xRes, hwTarget.a64GpX());
  if (slotIdx < JSObject::DIRECT_PROPERTY_SLOTS) {
    // If the slot is in the direct property slots, load it directly.
    ofs = offsetof(SHJSObjectAndDirectProps, directProps) +
        slotIdx * sizeof(SHGCSmallHermesValue);
  } else {
    // If the slot is in indirect storage, retrieve the pointer to that storage.
    emit_load_cp(a, xRes, a64::Mem(xRes, offsetof(SHJSObject, propStorage)));
    emit_sh_cp_decode_non_null(a, xRes);
    auto storageSlot = slotIdx - JSObject::DIRECT_PROPERTY_SLOTS;
    ofs = offsetof(SHArrayStorageSmall, storage) +
        storageSlot * sizeof(SHGCSmallHermesValue);
  }
  emit_load_shv(a, xRes, a64::Mem(xRes, ofs));
  auto doneLab = a.newLabel();
  emit_sh_shv_decode(a, xRes, doneLab);
  a.bind(doneLab);
}

void Emitter::putOwnBySlotIdx(FR frTarget, FR frValue, uint32_t slotIdx) {
  comment(
      "// PutOwnBySlotIdx r%u, r%u, %u",
      frTarget.index(),
      frValue.index(),
      slotIdx);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  // For indirect stores, 0 is the first indirect index.
  a.mov(
      a64::w2,
      slotIdx < JSObject::DIRECT_PROPERTY_SLOTS
          ? slotIdx
          : slotIdx - JSObject::DIRECT_PROPERTY_SLOTS);
  loadFrameAddr(a64::x3, frValue);

  if (slotIdx < JSObject::DIRECT_PROPERTY_SLOTS) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
        _sh_prstore_direct);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
        _sh_prstore_indirect);
  }
}

void Emitter::instanceOf(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// InstanceOf r%u, r%u, r%u",
      frRes.index(),
      frLeft.index(),
      frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_instance_of_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::isIn(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// isIn r%u, r%u, r%u", frRes.index(), frLeft.index(), frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_is_in_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::privateIsIn(
    FR frRes,
    FR frPrivateName,
    FR frTarget,
    uint8_t cacheIdx) {
  comment(
      "// PrivateIsIn r%u, r%u, r%u, cache %u",
      frRes.index(),
      frPrivateName.index(),
      frTarget.index(),
      cacheIdx);

  syncAllFRTempExcept(
      frRes != frPrivateName && frRes != frTarget ? frRes : FR());
  syncToFrame(frPrivateName);
  syncToFrame(frTarget);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frPrivateName);
  loadFrameAddr(a64::x2, frTarget);

  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x3, 0);
  } else {
    a.ldr(a64::x3, a64::Mem(roDataLabel_, roOfsPrivateNameCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x3, a64::x3, sizeof(SHPrivateNameCacheEntry) * cacheIdx);
  }

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHPrivateNameCacheEntry *),
      _sh_ljs_private_is_in_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

int32_t Emitter::reserveData(
    int32_t dsize,
    size_t align,
    asmjit::TypeId typeId,
    int32_t itemCount,
    const char *comment) {
  // Align the new data.
  size_t oldSize = roData_.size();
  size_t dataOfs = (roData_.size() + align - 1) & ~(align - 1);
  if (dataOfs >= INT32_MAX)
    hermes::hermes_fatal("JIT RO data overflow");
  // Grow to include the data.
  roData_.resize(dataOfs + dsize);

  // If logging is enabled, generate data descriptors.
  if (hasLogger()) {
    // Optional padding descriptor.
    if (dataOfs != oldSize) {
      int32_t gap = (int32_t)(dataOfs - oldSize);
      roDataDesc_.push_back(
          {.size = gap, .typeId = asmjit::TypeId::kUInt8, .itemCount = gap});
    }

    roDataDesc_.push_back(
        {.size = dsize,
         .typeId = typeId,
         .itemCount = itemCount,
         .comment = comment});
  }

  return (int32_t)dataOfs;
}

/// Register a 64-bit constant in RO DATA and return its offset.
int32_t Emitter::uint64Const(uint64_t bits, const char *comment) {
  auto [it, inserted] = fp64ConstMap_.try_emplace(bits, 0);
  if (inserted) {
    int32_t dataOfs = reserveData(
        sizeof(double), sizeof(double), asmjit::TypeId::kFloat64, 1, comment);
    memcpy(roData_.data() + dataOfs, &bits, sizeof(double));
    it->second = dataOfs;
  }
  return it->second;
}

asmjit::Label Emitter::registerThunk(void *fn, const char *name) {
  auto [it, inserted] = thunkMap_.try_emplace(fn, 0);
  // Is this a new thunk?
  if (inserted) {
    it->second = thunks_.size();
    int32_t dataOfs =
        reserveData(sizeof(fn), sizeof(fn), asmjit::TypeId::kUInt64, 1, name);
    memcpy(roData_.data() + dataOfs, &fn, sizeof(fn));
    thunks_.emplace_back(name ? a.newNamedLabel(name) : a.newLabel(), dataOfs);
  }

  return thunks_[it->second].first;
}

void Emitter::emitCatchTable(
    llvh::ArrayRef<const asmjit::Label *> exceptionHandlers) {
  // No trys in the function, nothing to do here.
  if (!catchTableLabel_.isValid())
    return;

  a.bind(catchTableLabel_);

  asmjit::Label addressTableLab = a.newLabel();

  // Find the catch target for the exception.
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::x2, xFrame);
  a.add(a64::x3, a64::sp, getJmpBufOffset());
  a.ldr(a64::x4, a64::Mem(a64::sp, getSavedSHLocalsOffset()));
  a.adr(a64::x5, addressTableLab);
  EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
      *this,
      void *(*)(SHRuntime *,
                SHCodeBlock *,
                SHLegacyValue *,
                SHJmpBuf *,
                SHLocals *,
                int32_t *),
      _jit_find_catch_target);

  // The address to branch to was returned here.
  a.br(a64::x0);

  // Table of offsets from addressTableLab to jump to.
  a.bind(addressTableLab);
  for (const asmjit::Label *handler : exceptionHandlers) {
    a.embedLabelDelta(*handler, addressTableLab, /* size */ 4);
  }
}

void Emitter::emitSlowPaths() {
  while (!slowPaths_.empty()) {
    SlowPath &sp = slowPaths_.front();
    emittingIP = sp.emittingIP;
    sp.emit(*this, sp);
    slowPaths_.pop_front();
  }
  emittingIP = nullptr;
}

void Emitter::emitThunks() {
  comment("// Thunks");
  for (const auto &th : thunks_) {
    a.bind(th.first);
    a.ldr(a64::x16, a64::Mem(roDataLabel_, th.second));
    a.br(a64::x16);
  }
}

void Emitter::emitROData() {
  a.bind(roDataLabel_);
  if (!hasLogger()) {
    a.embed(roData_.data(), roData_.size());
  } else {
    int32_t ofs = 0;
    for (const auto &desc : roDataDesc_) {
      if (desc.comment)
        comment("// %s", desc.comment);
      a.embedDataArray(desc.typeId, roData_.data() + ofs, desc.itemCount);
      ofs += desc.size;
    }
  }
}

void Emitter::callImpl(FR frRes, FR frCallee) {
  uint32_t nRegs = frameRegs_.size();

  emitIncrementCounter(JitCounter::NumCall);
  FR calleeFrameArg{nRegs + hbc::StackFrameLayout::CalleeClosureOrCB};

  // Store the callee to the right location in the frame, if it isn't already
  // there.
  if (frCallee != calleeFrameArg) {
    // Free any temp register before we mov into it so movFRFromHW stores
    // directly to the frame.
    freeFRTemp(calleeFrameArg);
    auto calleeReg = getOrAllocFRInAnyReg(frCallee, true);
    movFRFromHW(
        calleeFrameArg, calleeReg, frameRegs_[frCallee.index()].localType);
  }

  static_assert(
      HERMESVALUE_VERSION == 2,
      "Native pointers must be encoded without modification");

  FR previousFrameArg{nRegs + hbc::StackFrameLayout::PreviousFrame};
  // Free any existing temp so we store directly.
  freeFRTemp(previousFrameArg);
  movFRFromHW(previousFrameArg, HWReg(xFrame), FRType::OtherNonPtr);

  FR savedIPArg{nRegs + hbc::StackFrameLayout::SavedIP};
  // Since we need a register to compute the IP in anyway, it is convenient to
  // just use any existing one for the SavedIP slot, and let the syncAllFRTemp
  // below write it to memory.
  auto savedIPReg = getOrAllocFRInGpX(savedIPArg, false);

  // Save the current IP in both the SavedIP slot and the runtime.
  getBytecodeIP(savedIPReg.a64GpX());
  frUpdatedWithHW(savedIPArg, savedIPReg, FRType::OtherNonPtr);
  a.str(savedIPReg.a64GpX(), a64::Mem(xRuntime, RuntimeOffsets::currentIP));

  FR savedCodeBlockArg = FR{nRegs + hbc::StackFrameLayout::SavedCodeBlock};
  // TODO: We should be able to directly store xzr.
  auto savedCodeBlockReg = getOrAllocFRInGpX(savedCodeBlockArg, false);
  frUpdatedWithHW(savedCodeBlockArg, savedCodeBlockReg, FRType::OtherNonPtr);
  a.mov(savedCodeBlockReg.a64GpX(), 0);

  FR shLocalsArg{nRegs + hbc::StackFrameLayout::SHLocals};
  // Free any existing temp so we store directly.
  freeFRTemp(shLocalsArg);
  movFRFromHW(shLocalsArg, savedCodeBlockReg, FRType::OtherNonPtr);

#ifndef NDEBUG
  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < StackFrameLayout::CallerExtraRegistersAtEnd; ++i) {
    assert(
        !frameRegs_[nRegs - i - 1].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }
#endif

  auto hwCallee = getOrAllocFRInGpX(frCallee, true);
  auto hwTemp = allocTempGpX();
  auto xTemp = hwTemp.a64GpX();
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept({});
  freeReg(hwTemp);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);

  // If we have not already created a slow path for non-object calls, do so now.
  if (!nonObjCallLabel_.isValid()) {
    nonObjCallLabel_ = newSlowPathLabel();
    slowPaths_.push_back(
        {.slowPathLab = nonObjCallLabel_,
         .emit = [](Emitter &em, SlowPath &sl) {
           em.comment("// Throw on non-object call");
           em.a.bind(sl.slowPathLab);
           em.a.mov(a64::x0, xRuntime);
           // The IP is already saved by the call setup, no need to save it
           // again.
           EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
               em, void (*)(SHRuntime *), _jit_throw_non_object_call);
           // The call does not return.
         }});
  }

  auto slowPathLab = newSlowPathLabel();
  auto contLab = newContLabel();

  // Check if the callee is a JSFunction we have already JITted.
  emit_sh_ljs_is_object(a, xTemp, hwCallee.a64GpX());
  a.b_ne(nonObjCallLabel_);

  // We can now use any temp registers we want, because everything has been
  // sync'd and we are done with hwCallee. We keep the callee in x1 so that we
  // don't have to move it in the slow path.
  emit_sh_ljs_get_pointer(a, a64::x1, hwCallee.a64GpX());
  emit_gccell_get_kind(a, a64::x0, a64::x1);

  // Check if it is a JSFunction.
  emit_cellkind_in_range(
      a,
      /* wTemp */ a64::w2,
      /* wInput */ a64::w0,
      CellKind::CodeBlockFunctionKind_first,
      CellKind::CodeBlockFunctionKind_last);
  a.b_hi(slowPathLab);

  // Check if the JSFunction has already been JIT compiled.
  a.ldr(a64::x2, a64::Mem(a64::x1, RuntimeOffsets::jsFunctionCodeBlock));
  a.ldr(a64::x2, a64::Mem(a64::x2, RuntimeOffsets::codeBlockJitPtr));
  a.cbz(a64::x2, slowPathLab);

  // We have a JIT compiled function, call it.
  a.bind(contLab);
  // Both the fast and slow paths have prepared any arguments except the first
  // one, which is Runtime, and placed the callee in x2. Call it.
  a.mov(a64::x0, xRuntime);
  a.blr(a64::x2);
  movHWFromHW<false>(hwRes, HWReg::gpX(0));

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frCallee,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: CallImpl r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.emitIncrementCounter(JitCounter::NumCallSlow);
         em.loadBits64InGp(
             a64::x2, (uint64_t)&VTable::jitCallArray, "VTable::jitCallArray");
         // Load the jitCall function. x0 still contains the callee CellKind
         // from the fast path.
         em.a.ldr(
             a64::x2,
             a64::Mem(a64::x2, a64::x0, a64::Shift(a64::ShiftOp::kLSL, 3)));
         // x1 already contains the Callable* from the fast path.
         em.a.b(sl.contLab);
       }});
}

void Emitter::call(FR frRes, FR frCallee, uint32_t argc) {
  comment("// Call r%u, r%u, %u", frRes.index(), frCallee.index(), argc);
  uint32_t nRegs = frameRegs_.size();

  // Store undefined as the new target.
  FR ntFrameArg{nRegs + hbc::StackFrameLayout::NewTarget};
  loadConstBits64(
      ntFrameArg, _sh_ljs_undefined().raw, FRType::UnknownNonPtr, "undefined");

  FR argcFrameArg{nRegs + hbc::StackFrameLayout::ArgCount};
  static_assert(HERMESVALUE_VERSION == 2, "Native u32 must not need encoding");
  // The bytecode arg count includes "this", but the frame one does not, so
  // subtract 1.
  loadConstBits64(argcFrameArg, argc - 1, FRType::OtherNonPtr, "argCount");

#ifndef NDEBUG
  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < argc; ++i) {
    assert(
        !frameRegs_[nRegs + hbc::StackFrameLayout::ThisArg - i].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }
#endif

  callImpl(frRes, frCallee);
}

void Emitter::callN(FR frRes, FR frCallee, llvh::ArrayRef<FR> args) {
  comment(
      "// Call%zu r%u, r%u, ...args",
      args.size(),
      frRes.index(),
      frCallee.index());
  uint32_t nRegs = frameRegs_.size();

  for (uint32_t i = 0; i < args.size(); ++i) {
    auto argLoc = FR{nRegs + hbc::StackFrameLayout::ThisArg - i};

    if (args[i] != argLoc) {
      // Free any temp register before we mov into it so movFRFromHW stores
      // directly to the frame.
      freeFRTemp(argLoc);
      auto argReg = getOrAllocFRInAnyReg(args[i], true);
      movFRFromHW(argLoc, argReg, frameRegs_[args[i].index()].localType);
    }
    assert(
        !frameRegs_[argLoc.index()].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }

  // Get a register for the new target.
  FR ntFrameArg{nRegs + hbc::StackFrameLayout::NewTarget};
  loadConstBits64(
      ntFrameArg, _sh_ljs_undefined().raw, FRType::UnknownNonPtr, "undefined");
  syncToFrame(ntFrameArg);

  FR argcFrameArg{nRegs + hbc::StackFrameLayout::ArgCount};
  static_assert(HERMESVALUE_VERSION == 2, "Native u32 must not need encoding");
  // The bytecode arg count includes "this", but the frame one does not, so
  // subtract 1.
  loadConstBits64(
      argcFrameArg, args.size() - 1, FRType::OtherNonPtr, "argCount");

  callImpl(frRes, frCallee);
}

void Emitter::callBuiltin(FR frRes, uint32_t builtinIndex, uint32_t argc) {
  comment(
      "// CallBuiltin r%u, %s, %u",
      frRes.index(),
      getBuiltinMethodName(builtinIndex),
      argc);
  // CallBuiltin internally sets "this", so we don't sync it to memory.
#ifndef NDEBUG
  uint32_t nRegs = frameRegs_.size();

  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < argc; ++i) {
    assert(
        !frameRegs_[nRegs + hbc::StackFrameLayout::ThisArg - i].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }
#endif

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  // The bytecode arg count includes "this", but the SH one does not, so
  // subtract 1.
  a.mov(a64::w2, argc - 1);
  a.mov(a64::w3, builtinIndex);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t, uint32_t),
      _jit_call_builtin);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::callWithNewTarget(
    FR frRes,
    FR frCallee,
    FR frNewTarget,
    uint32_t argc) {
  comment(
      "// CallWithNewTarget r%u, r%u, r%u, %u",
      frRes.index(),
      frCallee.index(),
      frNewTarget.index(),
      argc);
  uint32_t nRegs = frameRegs_.size();

  FR ntFrameArg{nRegs + hbc::StackFrameLayout::NewTarget};
  // Store the new target to the right location in the frame.
  if (ntFrameArg != frNewTarget) {
    // Free the register before we mov into it so we store directly to the
    // frame.
    freeFRTemp(ntFrameArg);
    auto newTargetReg = getOrAllocFRInAnyReg(frNewTarget, true);
    movFRFromHW(
        ntFrameArg, newTargetReg, frameRegs_[frNewTarget.index()].localType);
  }

  FR argcFrameArg{nRegs + hbc::StackFrameLayout::ArgCount};
  static_assert(HERMESVALUE_VERSION == 2, "Native u32 must not need encoding");
  // The bytecode arg count includes "this", but the frame one does not, so
  // subtract 1.
  loadConstBits64(argcFrameArg, argc - 1, FRType::OtherNonPtr, "argCount");

#ifndef NDEBUG
  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < argc; ++i) {
    assert(
        !frameRegs_[nRegs + hbc::StackFrameLayout::ThisArg - i].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }
#endif

  callImpl(frRes, frCallee);
}

void Emitter::callWithNewTargetLong(
    FR frRes,
    FR frCallee,
    FR frNewTarget,
    FR frArgc) {
  comment(
      "// CallWithNewTarget r%u, r%u, r%u, r%u",
      frRes.index(),
      frCallee.index(),
      frNewTarget.index(),
      frArgc.index());
  uint32_t nRegs = frameRegs_.size();

  FR ntFrameArg{nRegs + hbc::StackFrameLayout::NewTarget};
  // Store the new target to the right location in the frame.
  if (ntFrameArg != frNewTarget) {
    // Free the register before we mov into it so we store directly to the
    // frame.
    freeFRTemp(ntFrameArg);
    auto newTargetReg = getOrAllocFRInAnyReg(frNewTarget, true);
    movFRFromHW(
        ntFrameArg, newTargetReg, frameRegs_[frNewTarget.index()].localType);
  }

  HWReg hwArgc = getOrAllocFRInVecD(frArgc, true);
  FR argcFrameArg{nRegs + hbc::StackFrameLayout::ArgCount};
  HWReg hwArgcArg = getOrAllocFRInGpX(argcFrameArg, false);
  frUpdatedWithHW(argcFrameArg, hwArgcArg, FRType::OtherNonPtr);

  static_assert(HERMESVALUE_VERSION == 2, "Native u32 must not need encoding");
  a.fcvtzu(hwArgcArg.a64GpX(), hwArgc.a64VecD());
  // The bytecode arg count includes "this", but the frame one does not, so
  // subtract 1.
  a.sub(hwArgcArg.a64GpX(), hwArgcArg.a64GpX(), 1);

  callImpl(frRes, frCallee);
}

void Emitter::callRequire(FR frRes, FR frRequireFunc, uint32_t modIndex) {
  comment(
      "// CallRequire r%u, r%u, %u",
      frRes.index(),
      frRequireFunc.index(),
      modIndex);

  syncAllFRTempExcept(frRes != frRequireFunc ? frRes : FR());
  syncToFrame(frRequireFunc);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(
      a64::x1,
      (uint64_t)codeBlock_->getRuntimeModule() +
          RuntimeOffsets::runtimeModuleModuleCache,
      "cacheData");
  loadFrameAddr(a64::x2, frRequireFunc);
  a.mov(a64::w3, modIndex);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHArrayStorage **, SHLegacyValue *, uint32_t),
      _sh_ljs_callRequire);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getBuiltinClosure(FR frRes, uint32_t builtinIndex) {
  comment(
      "// GetBuiltinClosure r%u, %s",
      frRes.index(),
      getBuiltinMethodName(builtinIndex));
  auto hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);
  // Load the closure pointer and add the object tag.
  emit_load_builtin_closure(a, hwRes.a64GpX(), builtinIndex);
  emit_sh_ljs_object(a, hwRes.a64GpX());
}

void Emitter::arithUnop(
    bool forceNumber,
    FR frRes,
    FR frInput,
    const char *name,
    void (*fast)(
        a64::Assembler &a,
        const a64::VecD &dst,
        const a64::VecD &src,
        const a64::VecD &tmp),
    void *slowCall,
    const char *slowCallName) {
  comment("// %s r%u, r%u", name, frRes.index(), frInput.index());

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool inputIsNum;

  if (forceNumber) {
    frameRegs_[frInput.index()].localType = FRType::Number;
    inputIsNum = true;
  } else {
    inputIsNum = isFRKnownNumber(frInput);
  }

  hwInput = getOrAllocFRInVecD(frInput, true);
  if (!inputIsNum) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frInput ? frRes : FR());
    syncToFrame(frInput);

    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN (which does not compare equal to itself).
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.fcmp(hwInput.a64VecD(), hwInput.a64VecD());
    a.b_ne(slowPathLab);
  }

  hwRes = getOrAllocFRInVecD(frRes, false);
  HWReg hwTmp = hwRes != hwInput ? hwRes : allocTempVecD();
  fast(a, hwRes.a64VecD(), hwInput.a64VecD(), hwTmp.a64VecD());
  if (hwRes == hwInput)
    freeReg(hwTmp);

  frUpdatedWithHW(
      frRes, hwRes, inputIsNum ? FRType::Number : FRType::UnknownPtr);

  if (inputIsNum)
    return;

  freeAllFRTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::booleanNot(FR frRes, FR frInput) {
  comment("// Not r%u, r%u", frRes.index(), frInput.index());

  // TODO: Add a fast path, perhaps by sharing some code with JmpTrue.
  syncAndFreeTempReg(HWReg::gpX(0));
  movHWFromFR(HWReg::gpX(0), frInput);

  // Since we already loaded the input, no need to check for frRes == frInput.
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  EMIT_RUNTIME_CALL(*this, bool (*)(SHLegacyValue), _sh_ljs_to_boolean);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  // Negate the result.
  a.eor(hwRes.a64GpX(), a64::x0, 1);
  // Add the bool tag.
  emit_sh_ljs_bool(a, hwRes.a64GpX());
  frUpdatedWithHW(frRes, hwRes, FRType::Bool);
}

void Emitter::bitNot(FR frRes, FR frInput) {
  comment("// BitNot r%u, r%u", frRes.index(), frInput.index());

  HWReg hwTempGpX = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInVecD(frInput, true);
  emit_double_is_int(
      a, hwTempGpX.a64GpX(), hwTempVecD.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempGpX);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(
      frRes,
      hwRes,
      isFRKnownType(frInput, FRType::Number) ? FRType::Number
                                             : FRType::UnknownPtr);

  // Perform the negation and write it to the result.
  a.mvn(hwTempGpX.a64GpX().w(), hwTempGpX.a64GpX().w());
  a.scvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// bitNot r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_bit_not_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::typeOf(FR frRes, FR frInput) {
  comment("// TypeOf r%u, r%u", frRes.index(), frInput.index());
  syncAllFRTempExcept(frRes == frInput ? FR() : frRes);
  syncToFrame(frInput);
  freeAllFRTempExcept(FR());

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frInput);
  // TODO: Use a function that preserves temporary registers.
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, SHLegacyValue *), _sh_ljs_typeof);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getPNameList(FR frRes, FR frObj, FR frIdx, FR frSize) {
  comment(
      "// GetPNameList r%u, r%u, r%u, r%u",
      frRes.index(),
      frObj.index(),
      frIdx.index(),
      frSize.index());
  syncAllFRTempExcept({});
  // We have to sync frObj to the frame since it is an in/out parameter.
  syncToFrame(frObj);
  // No need to sync frIdx and frSize since they are just out parameters.
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frObj);
  loadFrameAddr(a64::x2, frIdx);
  loadFrameAddr(a64::x3, frSize);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_get_pname_list_rjs);

  // Ensure that the out params have their frame location marked as up-to-date,
  // and any global register is updated.
  syncFrameOutParam(frObj);
  syncFrameOutParam(frIdx);
  syncFrameOutParam(frSize);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getNextPName(
    FR frRes,
    FR frProps,
    FR frObj,
    FR frIdx,
    FR frSize) {
  comment(
      "// GetNextPName r%u, r%u, r%u, r%u, r%u",
      frRes.index(),
      frProps.index(),
      frObj.index(),
      frIdx.index(),
      frSize.index());

  syncAllFRTempExcept({});
  syncToFrame(frProps);
  syncToFrame(frObj);
  syncToFrame(frIdx);
  syncToFrame(frSize);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frProps);
  loadFrameAddr(a64::x2, frObj);
  loadFrameAddr(a64::x3, frIdx);
  loadFrameAddr(a64::x4, frSize);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHLegacyValue *),
      _sh_ljs_get_next_pname_rjs);

  // Ensure that the updated frame location is sync'd back to any global reg.
  syncFrameOutParam(frIdx);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::toPropertyKey(FR frRes, FR frVal) {
  comment("// ToPropertyKey r%u, r%u", frRes.index(), frVal.index());
  syncAllFRTempExcept(frRes != frVal ? frRes : FR());
  syncToFrame(frVal);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frVal);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
      _sh_ljs_to_property_key);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::createPrivateName(FR frRes, SHSymbolID symID) {
  comment("// CreatePrivateName r%u, %u", frRes.index(), symID);
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, symID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHSymbolID),
      _sh_ljs_create_private_name);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::addS(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// AddS r%u, r%u, r%u", frRes.index(), frLeft.index(), frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_string_add);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::mod(bool forceNumber, FR frRes, FR frLeft, FR frRight) {
  comment(
      "// %s%s r%u, r%u, r%u",
      "mod",
      forceNumber ? "N" : "",
      frRes.index(),
      frLeft.index(),
      frRight.index());
  HWReg hwRes, hwLeft, hwRight;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool leftIsNum, rightIsNum, slow;

  if (forceNumber) {
    frameRegs_[frLeft.index()].localType = FRType::Number;
    frameRegs_[frRight.index()].localType = FRType::Number;
    leftIsNum = rightIsNum = true;
    slow = false;
  } else {
    leftIsNum = isFRKnownNumber(frLeft);
    rightIsNum = isFRKnownNumber(frRight);
    slow = !(rightIsNum && leftIsNum);
  }

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());

  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncToFrame(frLeft);
    syncToFrame(frRight);
  }

  hwLeft = getOrAllocFRInVecD(frLeft, true);
  hwRight = getOrAllocFRInVecD(frRight, true);

  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN. We can do that with the VS condition code, which is set if either
    // operand to fcmp is NaN.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());
    a.b_vs(slowPathLab);
  }

  // Make sure d0, d1 are unused.
  syncAndFreeTempReg(HWReg::vecD(0));
  movHWFromFR(HWReg::vecD(0), frLeft);
  syncAndFreeTempReg(HWReg::vecD(1));
  movHWFromFR(HWReg::vecD(1), frRight);

  EMIT_RUNTIME_CALL(*this, double (*)(double, double), _sh_mod_double);
  freeAllFRTempExcept({});
  hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::vecD(0));
  movHWFromHW<false>(hwRes, HWReg::vecD(0));
  frUpdatedWithHW(frRes, hwRes);

  if (!slow)
    return;

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// mod r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(
                 SHRuntime *, const SHLegacyValue *, const SHLegacyValue *),
             _sh_ljs_mod_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::arithBinOp(
    bool forceNumber,
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    void (*fast)(
        a64::Assembler &a,
        const a64::VecD &res,
        const a64::VecD &dl,
        const a64::VecD &dr),
    void *slowCall,
    const char *slowCallName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frLeft.index(),
      frRight.index());
  HWReg hwRes, hwLeft, hwRight;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool leftIsNum, rightIsNum, slow;

  if (forceNumber) {
    frameRegs_[frLeft.index()].localType = FRType::Number;
    frameRegs_[frRight.index()].localType = FRType::Number;
    leftIsNum = rightIsNum = true;
    slow = false;
  } else {
    leftIsNum = isFRKnownNumber(frLeft);
    rightIsNum = isFRKnownNumber(frRight);
    slow = !(rightIsNum && leftIsNum);
  }

  hwLeft = getOrAllocFRInVecD(frLeft, true);
  hwRight = getOrAllocFRInVecD(frRight, true);

  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
    syncToFrame(frLeft);
    syncToFrame(frRight);
    freeAllFRTempExcept({});
  }

  hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, !slow ? FRType::Number : FRType::UnknownPtr);

  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN. We can do that with the VS condition code, which is set if either
    // operand to fcmp is NaN.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());
    a.b_vs(slowPathLab);
  }

  fast(a, hwRes.a64VecD(), hwLeft.a64VecD(), hwRight.a64VecD());

  if (!slow)
    return;

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// %s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::bitBinOp(
    FR frRes,
    FR frLeft,
    FR frRight,
    bool unsignedRes,
    const char *name,
    SHLegacyValue (*slowCall)(
        SHRuntime *shr,
        const SHLegacyValue *a,
        const SHLegacyValue *b),
    const char *slowCallName,
    void (*fast)(
        a64::Assembler &a,
        const a64::GpX &res,
        const a64::GpX &dl,
        const a64::GpX &dr)) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frLeft.index(),
      frRight.index());

  HWReg hwTempLGpX = allocTempGpX();
  HWReg hwTempRGpX = allocTempGpX();
  HWReg hwTempLVecD = allocTempVecD();
  HWReg hwTempRVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  // TODO: In principle, it should be possible to only sync these in the slow
  // path. If we do that, we have to ensure that the frameUpToDate bit is not
  // set, since subsequent instructions cannot rely on it. To do this, we would
  // need to preserve information for the slow path to know whether they were
  // already sync'd to memory.
  syncToFrame(frLeft);
  syncToFrame(frRight);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwLeft = getOrAllocFRInVecD(frLeft, true);
  emit_double_is_int(
      a, hwTempLGpX.a64GpX(), hwTempLVecD.a64VecD(), hwLeft.a64VecD());
  a.b_ne(slowPathLab);

  // Do the same for the RHS.
  HWReg hwRight = getOrAllocFRInVecD(frRight, true);
  emit_double_is_int(
      a, hwTempRGpX.a64GpX(), hwTempRVecD.a64VecD(), hwRight.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempLGpX);
  freeReg(hwTempRGpX);
  freeReg(hwTempLVecD);
  freeReg(hwTempRVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(
      frRes,
      hwRes,
      isFRKnownNumber(frLeft) && isFRKnownNumber(frRight) ? FRType::Number
                                                          : FRType::UnknownPtr);

  // Invoke the fast path, and move the result back as a 32 bit integer.
  fast(a, hwTempLGpX.a64GpX(), hwTempLGpX.a64GpX(), hwTempRGpX.a64GpX());
  if (unsignedRes)
    a.ucvtf(hwRes.a64VecD(), hwTempLGpX.a64GpX().w());
  else
    a.scvtf(hwRes.a64VecD(), hwTempLGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .slowCall = (void *)slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// %s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}
void Emitter::jmpTrueFalse(
    bool onTrue,
    const asmjit::Label &target,
    FR frInput) {
  comment("// Jmp%s r%u", onTrue ? "True" : "False", frInput.index());

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());

  if (isFRKnownType(frInput, FRType::Number)) {
    HWReg hwInput = getOrAllocFRInVecD(frInput, true);
    a.fcmp(hwInput.a64VecD(), 0.0);
    if (onTrue) {
      // Branch on < 0 and > 0. All that remains is 0 and NaN.
      a.b_mi(target);
      a.b_gt(target);
    } else {
      asmjit::Label label = a.newLabel();
      a.b_mi(label);
      a.b_gt(label);
      a.b(target);
      a.bind(label);
    }
  } else if (isFRKnownType(frInput, FRType::Bool)) {
    HWReg hwInput = getOrAllocFRInGpX(frInput, true);
    a64::GpX xInput = hwInput.a64GpX();

    static_assert(
        HERMESVALUE_VERSION == 2, "bool is encoded as a bit at kHV_BoolBitIdx");
    // We don't use tbz/tbnz here because they have a very restricted range.
    a.tst(xInput, 1ull << kHV_BoolBitIdx);
    a.b(onTrue ? a64::CondCode::kNotZero : a64::CondCode::kZero, target);
  } else {
    // TODO: we should inline all of it.
    syncAllFRTempExcept({});
    movHWFromFR(HWReg::gpX(0), frInput);
    EMIT_RUNTIME_CALL(*this, bool (*)(SHLegacyValue), _sh_ljs_to_boolean);
    if (onTrue)
      a.cbnz(a64::w0, target);
    else
      a.cbz(a64::w0, target);
    freeAllFRTempExcept(FR());
  }
}

void Emitter::jmp(const asmjit::Label &target) {
  comment("// Jmp Lx");
  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());
  a.b(target);
}

void Emitter::jmpUndefined(const asmjit::Label &target, FR frInput) {
  comment("// JmpUndefined r%u", frInput.index());

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());

  if (isFRKnownType(frInput, FRType::Number) ||
      isFRKnownType(frInput, FRType::Bool)) {
    return;
  }

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  a64::GpX xInput = hwInput.a64GpX();
  HWReg hwTmpTag = allocTempGpX();
  a64::GpX xTmpTag = hwTmpTag.a64GpX();

  emit_sh_ljs_is_undefined(a, xTmpTag, xInput);
  a.b_eq(target);

  freeReg(hwTmpTag);
}

void Emitter::jmpBuiltinIs(
    bool invert,
    const asmjit::Label &target,
    uint8_t builtinIndex,
    FR frInput) {
  comment(
      "// JmpBuiltinIs%s r%u, %u",
      invert ? "Not" : "",
      frInput.index(),
      builtinIndex);

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept({});
  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwBuiltin = allocTempGpX();
  freeReg(hwBuiltin);
  freeAllFRTempExcept({});

  // Load builtin pointer.
  emit_load_builtin_closure(a, hwBuiltin.a64GpX(), builtinIndex);

  // Encode an object HermesValue.
  emit_sh_ljs_object(a, hwBuiltin.a64GpX());

  // Compare the builtin pointer with the input, branch.
  a.cmp(hwBuiltin.a64GpX(), hwInput.a64GpX());
  if (!invert)
    a.b_eq(target);
  else
    a.b_ne(target);
}

void Emitter::jCond(
    bool forceNumber,
    bool invert,
    bool passArgsByVal,
    const asmjit::Label &target,
    FR frLeft,
    FR frRight,
    const char *name,
    a64::CondCode condCode,
    void *slowCall,
    const char *slowCallName) {
  comment(
      "// j_%s_%s Lx, r%u, r%u",
      invert ? "not" : "",
      name,
      frLeft.index(),
      frRight.index());
  HWReg hwLeft, hwRight;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool leftIsNum, rightIsNum, slow;

  if (forceNumber) {
    frameRegs_[frLeft.index()].localType = FRType::Number;
    frameRegs_[frRight.index()].localType = FRType::Number;
    leftIsNum = rightIsNum = true;
    slow = false;
  } else {
    leftIsNum = isFRKnownNumber(frLeft);
    rightIsNum = isFRKnownNumber(frRight);
    slow = !(rightIsNum && leftIsNum);
  }

  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncToFrame(frLeft);
    syncToFrame(frRight);
  }
  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());

  hwLeft = getOrAllocFRInVecD(frLeft, true);
  hwRight = getOrAllocFRInVecD(frRight, true);

  a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());

  // If the condition is not inverted, then it can only produce true if both
  // operands are numbers. Since we use NaN boxing, we know that all non-number
  // values will be NaN and therefore produce false. So if the result is true,
  // we can take the jump without checking for numbers.
  if (!invert)
    a.b(condCode, target);

  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN. We can do that with the VS condition code, which is set if either
    // operand to fcmp is NaN.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.b_vs(slowPathLab);
  }

  // If the condition is inverted, it will produce true if one of the operands
  // is a NaN, so we can only check it after the slow path check, since it would
  // incorrectly be taken for non-numbers.
  if (invert)
    a.b(a64::negateCond(condCode), target);

  if (!slow)
    return;

  a.bind(contLab);

  // Do this always, since this is the end of the BB.
  freeAllFRTempExcept(FR());

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .target = target,
       .name = name,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .invert = invert,
       .passArgsByVal = passArgsByVal,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: j_%s%s Lx, r%u, r%u",
             sl.invert ? "not_" : "",
             sl.name,
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         if (sl.passArgsByVal) {
           em._loadFrame(HWReg::gpX(0), sl.frInput1);
           em._loadFrame(HWReg::gpX(1), sl.frInput2);
         } else {
           em.a.mov(a64::x0, xRuntime);
           em.loadFrameAddr(a64::x1, sl.frInput1);
           em.loadFrameAddr(a64::x2, sl.frInput2);
         }
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         if (!sl.invert)
           em.a.cbnz(a64::w0, sl.target);
         else
           em.a.cbz(a64::w0, sl.target);
         em.a.b(sl.contLab);
       }});
}

void Emitter::jStrictEqual(
    bool invert,
    const asmjit::Label &target,
    FR frLeft,
    FR frRight) {
  comment(
      "// JStrict%sEq Lx, r%u, r%u",
      invert ? "Not" : "",
      frLeft.index(),
      frRight.index());

  // Fast path for raw-only comparison. One of the operands is a non-number
  // non-pointer value, meaning we can compare bits directly.
  if (isFRKnownBool(frLeft) || isFRKnownBool(frRight) ||
      isFRKnownOtherNonPtr(frLeft) || isFRKnownOtherNonPtr(frRight)) {
    // Do this always, since this could be the end of the BB.
    syncAllFRTempExcept({});
    HWReg hwLeft = getOrAllocFRInGpX(frLeft, true);
    HWReg hwRight = getOrAllocFRInGpX(frRight, true);
    freeAllFRTempExcept({});

    a.cmp(hwLeft.a64GpX(), hwRight.a64GpX());
    a.b(!invert ? a64::CondCode::kEQ : a64::CondCode::kNE, target);
    return;
  }

  // Fast path for number (double) comparison.
  // One of the operands is a number, so there's two cases:
  // * It's NaN: it'll fcmp false against everything which is what we want.
  // * It's not NaN: It won't have NaN tag bits so it'll compare false against
  //   all non-double HVs and correctly fcmp true against the same number.
  if (isFRKnownNumber(frLeft) || isFRKnownNumber(frRight)) {
    // Do this always, since this could be the end of the BB.
    syncAllFRTempExcept(FR());
    HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
    HWReg hwRightD = getOrAllocFRInVecD(frRight, true);
    freeAllFRTempExcept({});
    a.fcmp(hwLeftD.a64VecD(), hwRightD.a64VecD());
    a.b(!invert ? a64::CondCode::kEQ : a64::CondCode::kNE, target);
    return;
  }

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());

  HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
  HWReg hwRightD = getOrAllocFRInVecD(frRight, true);

  HWReg hwTmpLeft = allocTempGpX();
  a64::GpX xTmpLeft = hwTmpLeft.a64GpX();
  HWReg hwTmpRight = allocTempGpX();
  a64::GpX xTmpRight = hwTmpRight.a64GpX();
  HWReg hwLeft = getOrAllocFRInGpX(frLeft, true);
  HWReg hwRight = getOrAllocFRInGpX(frRight, true);
  freeReg(hwTmpLeft);
  freeReg(hwTmpRight);

  // Label for non-number comparisons.
  auto nonNumberLab = a.newLabel();

  // Set up slow path.
  auto slowPathLab = newSlowPathLabel();
  auto contLab = newContLabel();
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept(FR());

  a.fcmp(hwLeftD.a64VecD(), hwRightD.a64VecD());
  // If not inverted then equality here is real equality.
  // If the equality check fails, we don't know anything.
  if (!invert)
    a.b_eq(target);
  // Since HermesValue is NaN-boxed we know that all non-number values will be
  // NaN. So we can conveniently test for non-number values by checking for
  // NaN. We can do that with the VS condition code, which is set if either
  // operand to fcmp is NaN.
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.b_vs(nonNumberLab);
  // If neither number is NaN, then we can just jump to the correct endpoint.
  // We already checked equality above for the non-inverted case,
  // so just check the inverted case here to know if the values are not equal.
  // If all that fails, then the branch failed and we go to contLab.
  if (invert)
    a.b_ne(target);
  a.b(contLab);

  // Convenience labels so we don't have to think too hard about inverted logic
  // below.
  const asmjit::Label &equalLab = !invert ? target : contLab;
  const asmjit::Label &notEqualLab = !invert ? contLab : target;

  a.bind(nonNumberLab);
  emit_sh_ljs_is_double(a, hwLeft.a64GpX(), xTmpLeft);
  // Left is actually the JS NaN, which is never equal to anything.
  // No need to check RHS for JS NaN, because it won't cause false positive
  // on the raw bit check below.
  a.b_lo(notEqualLab);

  // Compare bits directly.
  // If they match exactly, the two values are equal.
  a.cmp(hwLeft.a64GpX(), hwRight.a64GpX());
  a.b_eq(equalLab);

  // First compare the tags. If they don't match, the two values are NOT equal.
  emit_sh_ljs_get_tag(a, xTmpLeft, hwLeft.a64GpX());
  emit_sh_ljs_get_tag(a, xTmpRight, hwRight.a64GpX());
  a.cmp(xTmpLeft, xTmpRight);
  a.b_ne(notEqualLab);

  // If the LHS is either a non-pointer or an object, we can compare raw values
  // only. We've already checked and we know that the raw values are not the
  // same, so if this is a non-pointer or an object, then the two values are NOT
  // strictly equal.
  emit_sh_ljs_tag_is_pointer(a, xTmpLeft);
  a.b_lo(notEqualLab);
  emit_sh_ljs_tag_is_object(a, xTmpLeft);
  a.b_eq(notEqualLab);

  // Now we know that the LHS is a non-object pointer.

  // Fast string path: string inequality can be easily determined by checking
  // the lengths. If the LHS isn't a string, go to slow path.
  emit_sh_ljs_tag_is_string(a, xTmpLeft);
  a.b_ne(slowPathLab);

  emit_stringprim_get_length_and_flags(a, xTmpLeft, hwLeft.a64GpX());
  emit_stringprim_get_length_and_flags(a, xTmpRight, hwRight.a64GpX());
  // XOR the lengths together and mask the result.
  // xTmpLeft will be nonzero if the lengths don't match.
  a.eor(xTmpLeft, xTmpLeft, xTmpRight);
  a.and_(xTmpLeft, xTmpLeft, RuntimeOffsets::stringPrimitiveLengthMask);
  // Length mismatch means we're done, the two values are NOT equal.
  a.cbnz(xTmpLeft, notEqualLab);
  a.b(slowPathLab);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .target = target,
       .name = invert ? "j_strict_not_eq" : "j_strict_eq",
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .invert = invert,
       .slowCall = (void *)_sh_ljs_strict_equal,
       .slowCallName = "_sh_ljs_strict_equal",
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s Lx, r%u, r%u",
             sl.name,
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em._loadFrame(HWReg::gpX(0), sl.frInput1);
         em._loadFrame(HWReg::gpX(1), sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         if (!sl.invert)
           em.a.cbnz(a64::w0, sl.target);
         else
           em.a.cbz(a64::w0, sl.target);
         em.a.b(sl.contLab);
       }});
}

void Emitter::strictEqualImpl(bool invert, FR frRes, FR frLeft, FR frRight) {
  comment(
      "// %s r%u, r%u, r%u",
      !invert ? "StrictEq" : "StrictNEq",
      frRes.index(),
      frLeft.index(),
      frRight.index());

  // Fast path for raw-only comparison. One of the operands is a non-number
  // non-pointer value, meaning we can compare bits directly.
  if (isFRKnownBool(frLeft) || isFRKnownBool(frRight) ||
      isFRKnownOtherNonPtr(frLeft) || isFRKnownOtherNonPtr(frRight)) {
    HWReg hwLeft = getOrAllocFRInGpX(frLeft, true);
    HWReg hwRight = getOrAllocFRInGpX(frRight, true);
    HWReg hwRes = getOrAllocFRInGpX(frRes, false);
    frUpdatedWithHW(frRes, hwRes, FRType::Bool);

    a.cmp(hwLeft.a64GpX(), hwRight.a64GpX());
    a.cset(hwRes.a64GpX(), !invert ? a64::CondCode::kEQ : a64::CondCode::kNE);
    emit_sh_ljs_bool(a, hwRes.a64GpX());
    return;
  }

  // Fast path for number (double) comparison.
  // One of the operands is a number, so there's two cases:
  // * It's NaN: it'll fcmp false against everything which is what we want.
  // * It's not NaN: It won't have NaN tag bits so it'll compare false against
  //   all non-double HVs and correctly fcmp true against the same number.
  if (isFRKnownNumber(frLeft) || isFRKnownNumber(frRight)) {
    // Do this always, since this could be the end of the BB.
    HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
    HWReg hwRightD = getOrAllocFRInVecD(frRight, true);
    HWReg hwRes = getOrAllocFRInGpX(frRes, false);
    frUpdatedWithHW(frRes, hwRes, FRType::Bool);

    a.fcmp(hwLeftD.a64VecD(), hwRightD.a64VecD());
    a.cset(hwRes.a64GpX(), !invert ? a64::CondCode::kEQ : a64::CondCode::kNE);
    emit_sh_ljs_bool(a, hwRes.a64GpX());
    return;
  }

  HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
  HWReg hwRightD = getOrAllocFRInVecD(frRight, true);

  // Allocate registers used for non-number comparisons.
  HWReg hwLeft = getOrAllocFRInGpX(frLeft, true);
  HWReg hwRight = getOrAllocFRInGpX(frRight, true);
  HWReg hwTmpLeft = allocTempGpX();
  HWReg hwTmpRight = allocTempGpX();
  a64::GpX xTmpLeft = hwTmpLeft.a64GpX();
  a64::GpX xTmpRight = hwTmpRight.a64GpX();
  freeReg(hwTmpLeft);
  freeReg(hwTmpRight);

  // Labels for non-number comparisons.
  auto nonNumberLab = a.newLabel();
  auto equalLab = a.newLabel();
  auto notEqualLab = a.newLabel();

  // Set up slow path.
  auto slowPathLab = newSlowPathLabel();
  auto contLab = newContLabel();
  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Bool);
  a64::GpX xRes = hwRes.a64GpX();

  // Start by comparing doubles with fcmp.
  a.fcmp(hwLeftD.a64VecD(), hwRightD.a64VecD());

  // Since HermesValue is NaN-boxed we know that all non-number values will be
  // NaN. So we can conveniently test for non-number values by checking for
  // NaN. We can do that with the VS condition code, which is set if either
  // operand to fcmp is NaN.
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.b_vs(nonNumberLab);

  // Store the result of the comparison in the lowest bit of tmpCmpRes.
  // asmjit will convert CondCode to the correct encoding for use in the opcode.
  a.cset(xRes, invert ? a64::CondCode::kNE : a64::CondCode::kEQ);
  emit_sh_ljs_bool(a, xRes);

  a.b(contLab);

  // May be JS NaN (not a number, but really it is a number).
  a.bind(nonNumberLab);
  emit_sh_ljs_is_double(a, hwLeft.a64GpX(), xTmpLeft);
  // Left is actually the JS NaN, which is never equal to anything.
  // No need to check RHS for JS NaN, because it won't cause false positive
  // on the raw bit check below.
  a.b_lo(notEqualLab);

  // Compare bits directly.
  // If they match exactly, the two values are equal.
  a.cmp(hwLeft.a64GpX(), hwRight.a64GpX());
  a.b_eq(equalLab);

  // First compare the tags. If they don't match, the two values are NOT equal.
  emit_sh_ljs_get_tag(a, xTmpLeft, hwLeft.a64GpX());
  emit_sh_ljs_get_tag(a, xTmpRight, hwRight.a64GpX());
  a.cmp(xTmpLeft, xTmpRight);
  a.b_ne(notEqualLab);

  // If the LHS is either a non-pointer or an object, we can compare raw values
  // only. We've already checked and we know that the raw values are not the
  // same, so if this is a non-pointer or an object, then the two values are NOT
  // strictly equal.
  emit_sh_ljs_tag_is_pointer(a, xTmpLeft);
  a.b_lo(notEqualLab);
  emit_sh_ljs_tag_is_object(a, xTmpLeft);
  a.b_eq(notEqualLab);

  // Now we know that the LHS is a non-object pointer.

  // Fast string path: string inequality can be easily determined by checking
  // the lengths. If the LHS isn't a string, go to slow path.
  emit_sh_ljs_tag_is_string(a, xTmpLeft);
  a.b_ne(slowPathLab);

  emit_stringprim_get_length_and_flags(a, xTmpLeft, hwLeft.a64GpX());
  emit_stringprim_get_length_and_flags(a, xTmpRight, hwRight.a64GpX());
  // XOR the lengths together and mask the result.
  // xTmpLeft will be nonzero if the lengths don't match.
  a.eor(xTmpLeft, xTmpLeft, xTmpRight);
  a.and_(xTmpLeft, xTmpLeft, RuntimeOffsets::stringPrimitiveLengthMask);
  // Length mismatch means we're done, the two values are NOT equal.
  a.cbnz(xTmpLeft, notEqualLab);
  a.b(slowPathLab);

  // Jump here if the result should be "equal".
  // Returns true if not inverted, false if inverted.
  a.bind(equalLab);
  emit_sh_ljs_bool_const(a, xRes, !invert);
  a.b(contLab);

  // Jump here if the result should be "not equal".
  // Returns false if not inverted, true if inverted.
  a.bind(notEqualLab);
  emit_sh_ljs_bool_const(a, xRes, invert);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = !invert ? "StrictEq" : "StrictNEq",
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .invert = invert,
       .passArgsByVal = true,
       .slowCall = (void *)_sh_ljs_strict_equal,
       .slowCallName = "_sh_ljs_strict_equal",
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         if (sl.passArgsByVal) {
           em._loadFrame(HWReg::gpX(0), sl.frInput1);
           em._loadFrame(HWReg::gpX(1), sl.frInput2);
         } else {
           em.a.mov(a64::x0, xRuntime);
           em.loadFrameAddr(a64::x1, sl.frInput1);
           em.loadFrameAddr(a64::x2, sl.frInput2);
         }
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);

         // Invert the slow path result if needed.
         if (sl.invert)
           em.a.eor(sl.hwRes.a64GpX(), a64::x0, 1);
         else
           em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));

         // Comparison functions return bool, so encode it.
         emit_sh_ljs_bool(em.a, sl.hwRes.a64GpX());
         em.a.b(sl.contLab);
       }});
}

void Emitter::compareImpl(
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    a64::CondCode condCode,
    void *slowCall,
    const char *slowCallName,
    bool invSlow,
    bool passArgsByVal) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frLeft.index(),
      frRight.index());
  HWReg hwLeft, hwRight;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool leftIsNum, rightIsNum, slow;

  leftIsNum = isFRKnownNumber(frLeft);
  rightIsNum = isFRKnownNumber(frRight);
  slow = !(rightIsNum && leftIsNum);

  hwLeft = getOrAllocFRInVecD(frLeft, true);
  hwRight = getOrAllocFRInVecD(frRight, true);
  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
    syncToFrame(frLeft);
    syncToFrame(frRight);
    freeAllFRTempExcept({});
  }

  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  a64::GpX xRes = hwRes.a64GpX();

  a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());

  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN. We can do that with the VS condition code, which is set if either
    // operand to fcmp is NaN.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.b_vs(slowPathLab);
  }

  // Store the result of the comparison in the lowest bit of tmpCmpRes.
  // asmjit will convert CondCode to the correct encoding for use in the opcode.
  a.cset(xRes, condCode);

  // Encode bool.
  emit_sh_ljs_bool(a, xRes);
  frUpdatedWithHW(frRes, hwRes, FRType::Bool);

  if (!slow)
    return;

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .invert = invSlow,
       .passArgsByVal = passArgsByVal,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: j_%s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         if (sl.passArgsByVal) {
           em._loadFrame(HWReg::gpX(0), sl.frInput1);
           em._loadFrame(HWReg::gpX(1), sl.frInput2);
         } else {
           em.a.mov(a64::x0, xRuntime);
           em.loadFrameAddr(a64::x1, sl.frInput1);
           em.loadFrameAddr(a64::x2, sl.frInput2);
         }
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);

         // Invert the slow path result if needed.
         if (sl.invert)
           em.a.eor(sl.hwRes.a64GpX(), a64::x0, 1);
         else
           em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));

         // Comparison functions return bool, so encode it.
         emit_sh_ljs_bool(em.a, sl.hwRes.a64GpX());
         em.a.b(sl.contLab);
       }});
}

void Emitter::getArgumentsPropByValImpl(
    FR frRes,
    FR frIndex,
    FR frLazyReg,
    const char *name,
    SHLegacyValue (*shImpl)(
        SHRuntime *shr,
        SHLegacyValue *frame,
        SHLegacyValue *idx,
        SHLegacyValue *lazyReg),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frIndex.index(),
      frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept(frRes != frIndex && frRes != frLazyReg ? frRes : FR());
  syncToFrame(frIndex);
  syncToFrame(frLazyReg);
  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwIndex = getOrAllocFRInVecD(frIndex, true);
  HWReg hwTempIndex = allocTempGpX();
  a64::GpW wTempIndex = hwTempIndex.a64GpX().w();
  HWReg hwTempArgCount = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();
  freeAllFRTempExcept({});
  freeReg(hwTempIndex);
  freeReg(hwTempArgCount);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);

  // If lazyReg is an object, go to slow path.
  emit_sh_ljs_is_object(a, hwTempIndex.a64GpX(), hwLazyReg.a64GpX());
  a.b_eq(slowPathLab);

  // If index is not an array index, go to slow path.
  emit_double_is_int(
      a, hwTempIndex.a64GpX(), hwTempVecD.a64VecD(), hwIndex.a64VecD());
  a.b_ne(slowPathLab);

  // If index >= arg count or index < 0, go to slow path.
  // Use an unsigned comparison to handle the negative index case.
  a.ldur(
      hwTempArgCount.a64GpX().w(),
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));
  a.cmp(hwTempIndex.a64GpX(), hwTempArgCount.a64GpX());
  a.b_hs(slowPathLab);

  // Load the argument from the stack.
  // We want framePtr[(firstArg - index) * 8].
  // Use shift SXTW to shift the signed w register by 3.
  a.mov(hwTempArgCount.a64GpX().w(), (int)StackFrameLayout::FirstArg);
  a.sub(wTempIndex, hwTempArgCount.a64GpX().w(), wTempIndex);
  a.ldr(
      hwRes.a64GpX(),
      a64::Mem(xFrame, wTempIndex, a64::Shift(a64::ShiftOp::kSXTW, 3)));

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frIndex,
       .frInput2 = frLazyReg,
       .hwRes = hwRes,
       .slowCall = (void *)shImpl,
       .slowCallName = shImplName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: %s r%u", sl.name, sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         em.loadFrameAddr(a64::x3, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::reifyArgumentsImpl(FR frLazyReg, bool strict, const char *name) {
  comment("// %s r%u", name, frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept({});
  syncToFrame(frLazyReg);

  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwTemp = allocTempGpX();
  freeAllFRTempExcept({});
  freeReg(hwTemp);

  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwLazyReg.a64GpX());
  // If the lazyReg is not an object, it needs to be reified, go to slow path.
  a.b_ne(slowPathLab);

  // Fast path: do nothing.
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frInput1 = frLazyReg,
       // Use hwRes field to pass the global reg for the in/out param.
       .hwRes = frameRegs_[frLazyReg.index()].globalReg,
       .slowCall = strict ? (void *)_sh_ljs_reify_arguments_strict
                          : (void *)_sh_ljs_reify_arguments_loose,
       .slowCallName = strict ? "_sh_ljs_reify_arguments_strict"
                              : "_sh_ljs_reify_arguments_loose",
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: %s r%u", sl.name, sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         // Slow path modifies the frame so we need to sync it if there's a
         // global reg.
         if (sl.hwRes.isValid()) {
           em._loadFrame(sl.hwRes, sl.frInput1);
         }
         em.a.b(sl.contLab);
       }});
}

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
