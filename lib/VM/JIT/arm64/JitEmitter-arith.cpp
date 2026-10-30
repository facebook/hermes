/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitEmitter-internal.h"
#include "JitEmitter.h"
#include "JitHandlers.h"

#include "../RuntimeOffsets.h"

namespace hermes::vm::arm64 {

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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frInput, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: toNumber r%u, r%u", frRes.index(), frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        EMIT_RUNTIME_CALL(
            em,
            double (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_to_double_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::vecD(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frInput, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: toNumeric r%u, r%u", frRes.index(), frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_to_numeric_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
}

void Emitter::toInt32(FR frRes, FR frInput, bool isSigned) {
  comment(
      "// %s r%u, r%u",
      isSigned ? "ToInt32" : "ToUint32",
      frRes.index(),
      frInput.index());

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

  if (isSigned) {
    // Convert int32 back to double.
    a.scvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());
  } else {
    // Convert uint32 back to double.
    a.ucvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());
  }

  a.bind(contLab);

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [isSigned, frRes, frInput, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: %s, r%u, r%u",
            isSigned ? "ToInt32" : "ToUint32",
            frRes.index(),
            frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        em.callThunkWithSavedIP(
            isSigned ? (void *)_sh_ljs_to_int32_rjs
                     : (void *)_sh_ljs_to_uint32_rjs,
            isSigned ? "_sh_ljs_to_int32_rjs" : "_sh_ljs_to_uint32_rjs");
        em.movHWFromHW<false>(hwRes, HWReg::vecD(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frInput, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: AddEmptyString r%u, r%u",
            frRes.index(),
            frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_add_empty_string_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [name, frRes, frInput, hwRes, slowCall, slowCallName](
          Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: %s r%u, r%u", name, frRes.index(), frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        em.callThunkWithSavedIP(slowCall, slowCallName);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frInput, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: bitNot r%u, r%u", frRes.index(), frInput.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frInput);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_bit_not_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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
      *this, SHLegacyValue (*)(SHRuntime *, SHLegacyValue *), _sh_ljs_typeof);

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
      SHLegacyValue (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frLeft, frRight, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// mod r%u, r%u, r%u",
            frRes.index(),
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frLeft);
        em.loadFrameAddr(a64::x2, frRight);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(
                SHRuntime *, const SHLegacyValue *, const SHLegacyValue *),
            _sh_ljs_mod_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [name, frRes, frLeft, frRight, hwRes, slowCall, slowCallName](
          Emitter &em, SlowPath &sp) {
        em.comment(
            "// %s r%u, r%u, r%u",
            name,
            frRes.index(),
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frLeft);
        em.loadFrameAddr(a64::x2, frRight);
        em.callThunkWithSavedIP(slowCall, slowCallName);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [name, frRes, frLeft, frRight, hwRes, slowCall, slowCallName](
          Emitter &em, SlowPath &sp) {
        em.comment(
            "// %s r%u, r%u, r%u",
            name,
            frRes.index(),
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frLeft);
        em.loadFrameAddr(a64::x2, frRight);
        em.callThunkWithSavedIP((void *)slowCall, slowCallName);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [invert, frRes, frLeft, frRight, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: %s r%u, r%u, r%u",
            !invert ? "StrictEq" : "StrictNEq",
            frRes.index(),
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        // _sh_ljs_strict_equal takes its arguments by value.
        em._loadFrame(HWReg::gpX(0), frLeft);
        em._loadFrame(HWReg::gpX(1), frRight);
        em.callThunkWithSavedIP(
            (void *)_sh_ljs_strict_equal, "_sh_ljs_strict_equal");

        // Invert the slow path result if needed.
        if (invert)
          em.a.eor(hwRes.a64GpX(), a64::x0, 1);
        else
          em.movHWFromHW<false>(hwRes, HWReg::gpX(0));

        // Comparison functions return bool, so encode it.
        emit_sh_ljs_bool(em.a, hwRes.a64GpX());
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [name,
       frRes,
       frLeft,
       frRight,
       hwRes,
       invSlow,
       passArgsByVal,
       slowCall,
       slowCallName](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: j_%s r%u, r%u, r%u",
            name,
            frRes.index(),
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        if (passArgsByVal) {
          em._loadFrame(HWReg::gpX(0), frLeft);
          em._loadFrame(HWReg::gpX(1), frRight);
        } else {
          em.a.mov(a64::x0, xRuntime);
          em.loadFrameAddr(a64::x1, frLeft);
          em.loadFrameAddr(a64::x2, frRight);
        }
        em.callThunkWithSavedIP(slowCall, slowCallName);

        // Invert the slow path result if needed.
        if (invSlow)
          em.a.eor(hwRes.a64GpX(), a64::x0, 1);
        else
          em.movHWFromHW<false>(hwRes, HWReg::gpX(0));

        // Comparison functions return bool, so encode it.
        emit_sh_ljs_bool(em.a, hwRes.a64GpX());
        em.a.b(sp.contLab);
      });
}

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
