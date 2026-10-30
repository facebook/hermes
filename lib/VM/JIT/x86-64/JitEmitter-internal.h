/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT

#include "JitEmitter.h"

#include "../RuntimeOffsets.h"

namespace hermes::vm::x86_64 {

// Ensure that HermesValue tags are handled correctly by updating this every
// time the HERMESVALUE_VERSION changes, and going through the JIT and updating
// any relevant code.
static_assert(
    HERMESVALUE_VERSION == 2,
    "HermesValue version mismatch, JIT may need to be updated");

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

/// Save the current IP and emit a call to a runtime function. This should be
/// used in most cases when invoking slow paths and handlers for complex
/// functionality.
#define EMIT_RUNTIME_CALL(em, type, func)             \
  do {                                                \
    using _FnT = type;                                \
    _FnT _fn = func;                                  \
    (void)_fn;                                        \
    (em).callRuntimeWithSavedIP((void *)func, #func); \
  } while (0)

/// Call a runtime function without saving the IP. This is intended for special
/// cases where we want to preserve the currently saved IP or if the IP is not
/// needed.
#define EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(em, type, func) \
  do {                                                     \
    using _FnT = type;                                     \
    _FnT _fn = func;                                       \
    (void)_fn;                                             \
    (em).callRuntime((void *)func, #func);                 \
  } while (0)

/// Get the tag for the HermesValue in \p in and place it in \p out.
///
/// x86-64: the shift is two-operand, so unlike arm64 \p out is always
/// written, and \p in is preserved only when the two differ.
inline void
emit_sh_ljs_get_tag(x86::Assembler &a, const x86::Gp &out, const x86::Gp &in) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "kHV_NumDataBits is 48 and can be easily shifted");
  if (out != in)
    a.mov(out, in);
  a.sar(out, kHV_NumDataBits);
}

/// Check whether \p tagReg is the tag for a pointer value.
/// CPU flags are updated as result. jae on success.
///
/// x86-64: arm64 has to phrase the comparison as `cmn tag, -tag`, since its
/// immediates are unsigned; x86's imm32 is sign-extended, so the negative
/// tag is a plain `cmp` operand.
inline void emit_sh_ljs_tag_is_pointer(
    x86::Assembler &a,
    const x86::Gp &tagReg) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "All tags above HVTag_FirstPointer are pointers");
  // The valid pointer tags are: 0xfd (HVTag_FirstPointer) to 0xff
  // (HVTag_Last), but all sign extended to 64 bits.
  // We need an unsigned comparison (tag >= 0xfd) to catch all pointers.
  // Doubles may have 0 in the tag bits, e.g. so we have to use
  // unsigned condition codes to make sure they don't get detected as pointers.
  a.cmp(tagReg, asmjit::Imm(HVTag_FirstPointer));
}

/// Emit code to check whether \p tagReg is an object tag.
/// The input reg is not modified.
/// CPU flags are updated as result. je on success.
inline void emit_sh_ljs_tag_is_object(
    x86::Assembler &a,
    const x86::Gp &tagReg) {
  static_assert(
      (int16_t)HVTag_Object == (int16_t)(-1) && "HV_TagObject must be -1");
  a.cmp(tagReg, asmjit::Imm(HVTag_Object));
}

/// Emit code to check whether \p tagReg is a string tag.
/// The input reg is not modified.
/// CPU flags are updated as result. je on success.
inline void emit_sh_ljs_tag_is_string(
    x86::Assembler &a,
    const x86::Gp &tagReg) {
  // x86-64: the message is deliberately not arm64's. Its copy of this
  // assert says "HV_TagObject must be -1", copied from the object helper
  // above and never updated; the condition here is about HVTag_Str.
  static_assert((int16_t)HVTag_Str == (int16_t)(-3) && "HVTag_Str must be -3");
  a.cmp(tagReg, asmjit::Imm(HVTag_Str));
}

/// Emit code to check whether the input reg is an object, using the
/// specified temp register. The input reg is not modified unless it is
/// the same as the temp, which is allowed.
/// CPU flags are updated as result. je on success.
inline void emit_sh_ljs_is_object(
    x86::Assembler &a,
    const x86::Gp &tempReg,
    const x86::Gp &inputReg) {
  emit_sh_ljs_get_tag(a, tempReg, inputReg);
  emit_sh_ljs_tag_is_object(a, tempReg);
}

/// Extract the pointer out of the HermesValue in \p in into \p out.
///
/// x86-64: kHV_DataMask does not fit in a sign-extended imm32 and this
/// helper has no scratch register, so the tag is shifted out and back in
/// instead of masked off with arm64's single `and`.
inline void emit_sh_ljs_get_pointer(
    x86::Assembler &a,
    const x86::Gp &out,
    const x86::Gp &in) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "kHV_DataMask is 0x000...1111... and is exactly the low kHV_NumDataBits");
  if (out != in)
    a.mov(out, in);
  a.shl(out, 64 - kHV_NumDataBits);
  a.shr(out, 64 - kHV_NumDataBits);
}

/// Emit code to check whether the input HermesValue is a double.
/// The input reg is not modified.
/// The temp reg is modified.
/// CPU flags are updated as result. jb on success.
inline void emit_sh_ljs_is_double(
    x86::Assembler &a,
    const x86::Gp &input,
    const x86::Gp &tmp) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "numbers must be lower than HVTag_First << kHV_NumDataBits");
  a.mov(tmp, asmjit::Imm((uint64_t)HVTag_First << kHV_NumDataBits));
  a.cmp(input, tmp);
}

/// Emit code to check whether the input reg is bool, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. je on success.
///
/// x86-64: the two-operand shift needs a copy when tempReg and inputReg
/// differ, unlike arm64's three-operand asr; when they are the same
/// register the shift simply runs in place, exactly as arm64 allows.
inline void emit_sh_ljs_is_bool(
    x86::Assembler &a,
    const x86::Gp &tempReg,
    const x86::Gp &inputReg) {
  // Get the ETag bits by right shifting one bit further than the tag, and
  // compare against the sign-extended ETag constant directly (imm32 is
  // sign-extended on x86, so there is no arm64-style cmn-with-negation).
  static_assert(
      (int16_t)HVETag_Bool == (int16_t)(-10) && "HVETag_Bool must be -10");
  if (tempReg != inputReg)
    a.mov(tempReg, inputReg);
  a.sar(tempReg, kHV_NumDataBits - 1);
  a.cmp(tempReg, asmjit::Imm(HVETag_Bool));
}

/// For a register \p dInput, which contains a double, check whether it is
/// exactly an integer, i.e. whether truncating it and converting back gives
/// the same value.
/// CPU flags are updated: the value is an integer only if both \c jne and
/// \c jp fall through (see below).
/// If successful, \p xTemp will contain the number converted to a signed 64
/// bit integer; \p dTemp is clobbered either way.
/// \pre dTemp != dInput, because both are used in the comparison.
///
/// x86-64: arm64's fcvtzs saturates, so it has to defeat the saturation with
/// an sbfx before the round trip, or the doubles 2^63 and -2^63 would both
/// convert back to themselves and be accepted with the wrong low 32 bits.
/// vcvttsd2si does not saturate: every out-of-range input, and every NaN,
/// produces the "integer indefinite" value INT64_MIN, which converts back to
/// exactly -2^63. The only input that value can compare equal to is -2^63
/// itself, whose conversion to INT64_MIN is the correct one -- its low 32
/// bits are 0, which is ToInt32(-2^63). So no sbfx analogue is needed here
/// and the round-trip compare alone is exact.
///
/// The other divergence is the branch. arm64's fcmp leaves NE set on
/// unordered, so a single b.ne covers "different" and "input was a NaN".
/// vucomisd instead reports unordered as ZF=PF=CF=1, i.e. as *equal*, so the
/// caller must branch on both: jne for an ordered mismatch and jp for the
/// unordered case.
inline void emit_double_is_int(
    x86::Assembler &a,
    const x86::Gp &xTemp,
    const x86::Xmm &dTemp,
    const x86::Xmm &dInput) {
  assert(dTemp != dInput && "must use a different temp");
  assert(xTemp.isGpq() && "the round trip must go through 64 bits");

  // Convert the operand to a signed 64 bit integer.
  a.vcvttsd2si(xTemp, dInput);
  // Convert back to a double and see if they compare equal. dTemp is also
  // the source of the untouched upper half, which is dead here.
  a.vcvtsi2sd(dTemp, dTemp, xTemp);
  a.vucomisd(dTemp, dInput);
}

/// Convert the low 32 bits of \p xInt to a double in \p dRes, interpreting
/// them as unsigned if \p isUnsigned and as signed otherwise.
/// \p xInt is clobbered when \p isUnsigned.
///
/// x86-64: this stands in for arm64's scvtf/ucvtf of a W register. x86 has
/// no unsigned integer-to-double conversion at all, so the unsigned case
/// zero-extends into 64 bits -- a 32-bit mov does that for free -- and
/// converts that as a signed 64-bit value, which is exact for every uint32.
inline void emit_int32_to_double(
    x86::Assembler &a,
    const x86::Xmm &dRes,
    const x86::Gp &xInt,
    bool isUnsigned) {
  assert(xInt.isGpq() && "the unsigned conversion needs the full register");
  if (isUnsigned) {
    a.mov(xInt.r32(), xInt.r32());
    a.vcvtsi2sd(dRes, dRes, xInt);
  } else {
    a.vcvtsi2sd(dRes, dRes, xInt.r32());
  }
}

/// Encode the boolean in the low bit of \p inOut as a HermesValue in place.
/// The high bits of \p inOut must be zero.
///
/// x86-64: arm64 folds the tag in with `movk`, which x86 has no equivalent
/// of, and the tag constant is too large for a sign-extended imm32, so the
/// caller supplies \p tempReg to materialize it.
inline void emit_sh_ljs_bool(
    x86::Assembler &a,
    const x86::Gp &inOut,
    const x86::Gp &tempReg) {
  static constexpr SHLegacyValue baseBool = HermesValue::encodeBoolValue(false);
  static_assert(HERMESVALUE_VERSION == 2);
  static_assert(
      (llvh::isShiftedUInt<16, kHV_NumDataBits>(baseBool.raw)) &&
      "Boolean tag must be 16 bits.");
  assert(tempReg != inOut && "temp register must differ from the input");
  a.shl(inOut, kHV_BoolBitIdx);
  // Add the bool tag.
  a.mov(tempReg, asmjit::Imm(baseBool.raw));
  a.or_(inOut, tempReg);
}

/// Store the HermesValue for the bool \p val into \p out.
///
/// x86-64: any 64-bit constant is a single `mov`, so unlike arm64 there is
/// no mov/movk pair for the true case.
inline void
emit_sh_ljs_bool_const(x86::Assembler &a, const x86::Gp &out, bool val) {
  static constexpr SHLegacyValue baseBool = HermesValue::encodeBoolValue(false);
  static_assert(HERMESVALUE_VERSION == 2);
  a.mov(
      out,
      asmjit::Imm(
          baseBool.raw | (val ? (uint64_t)1 << kHV_BoolBitIdx : (uint64_t)0)));
}

/// Load the lengthAndFlags of a HermesValue that contains a StringPrimitive
/// into \p out.
/// out and in may be the same, but in will be clobbered.
///
/// x86-64: inline here rather than out of line as on arm64; the body is two
/// instructions. The 32-bit load zeroes the upper half of \p out.
inline void emit_stringprim_get_length_and_flags(
    x86::Assembler &a,
    const x86::Gp &out,
    const x86::Gp &in) {
  emit_sh_ljs_get_pointer(a, out, in);
  a.mov(
      out.r32(),
      x86::dword_ptr(out, RuntimeOffsets::stringPrimitiveLengthAndFlags));
}

/// Emit code to check whether the input reg holds undefined, using the
/// specified temp register, which must differ from the input.
/// CPU flags are updated as a result: je on success.
///
/// x86-64: arm64 extracts the tag with an arithmetic shift and compares that.
/// Here the whole undefined bit pattern is materialized into the temp
/// instead, because x86 has a one-instruction 64-bit immediate load and no
/// three-operand shift. Undefined carries no payload -- it is a single bit
/// pattern -- so comparing the full value is exactly the tag check.
inline void emit_sh_ljs_is_undefined(
    x86::Assembler &a,
    const x86::Gp &tempReg,
    const x86::Gp &inputReg) {
  static_assert(
      HERMESVALUE_VERSION == 2,
      "undefined must be a single bit pattern with no payload");
  assert(tempReg != inputReg && "temp register must differ from the input");
  a.mov(tempReg, asmjit::Imm(_sh_ljs_undefined().raw));
  a.cmp(inputReg, tempReg);
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
      asmjit::BaseEmitter *origin) override;
};

#ifndef ASMJIT_NO_LOGGING
class OurLogger : public asmjit::Logger {
 private:
  x86::Assembler &a_;
  PerfJitDump *perfJitDump_{nullptr};
  bool dumpJitCode_{false};

 public:
  OurLogger(x86::Assembler &a, PerfJitDump *perfJitDump, bool dumpJitCode)
      : a_(a), perfJitDump_(perfJitDump), dumpJitCode_(dumpJitCode) {}

  ASMJIT_API asmjit::Error _log(const char *data, size_t size) noexcept
      override;
};
#endif

} // namespace hermes::vm::x86_64

#endif // HERMESVM_JIT
