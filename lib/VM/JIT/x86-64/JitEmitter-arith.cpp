/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter-internal.h"
#include "JitEmitter.h"

#include "../JitHandlers.h"

#include "../RuntimeOffsets.h"

namespace hermes::vm::x86_64 {

void Emitter::toNumber(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumber", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput)) {
    emitTypeAssertFR(frInput, TypePred::IsNumber);
    return mov(frRes, frInput, false);
  }

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
  // x86-64: vucomisd reports unordered in PF, and sets ZF as well, so
  // arm64's `b.ne` has no equivalent here -- the NaN test is `jp`.
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.vucomisd(hwInput.xmm(), hwInput.xmm());
  a.jp(slowPathLab);
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
        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frInput);
        EMIT_RUNTIME_CALL(
            em,
            double (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_to_double_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::vecD(0));
        em.a.jmp(sp.contLab);
      });
}

void Emitter::toNumeric(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumeric", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput)) {
    emitTypeAssertFR(frInput, TypePred::IsNumber);
    return mov(frRes, frInput, false);
  }

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
  // x86-64: see toNumber -- the unordered result lives in PF, so `jp`.
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.vucomisd(hwInput.xmm(), hwInput.xmm());
  a.jp(slowPathLab);
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
        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frInput);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *),
            _sh_ljs_to_numeric_rjs);
        // x86-64: a one-eightbyte struct whose union mixes an integer and a
        // double classifies as INTEGER under SysV, so SHLegacyValue comes
        // back in rax, exactly as it comes back in x0 on arm64.
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
      });
}

void Emitter::arithUnop(
    bool forceNumber,
    FR frRes,
    FR frInput,
    const char *name,
    void (*fast)(
        Emitter &em,
        const x86::Xmm &d,
        const x86::Xmm &s,
        const x86::Xmm &tmp),
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
  if (inputIsNum)
    emitTypeAssert(frInput, hwInput, TypePred::IsNumber);
  if (!inputIsNum) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frInput ? frRes : FR());
    syncToFrame(frInput);

    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN (which does not compare equal to itself).
    // x86-64: unordered lands in PF, so the NaN test is `jp`.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.vucomisd(hwInput.xmm(), hwInput.xmm());
    a.jp(slowPathLab);
  }

  hwRes = getOrAllocFRInVecD(frRes, false);
  HWReg hwTmp = hwRes != hwInput ? hwRes : allocTempVecD();
  fast(*this, hwRes.xmm(), hwInput.xmm(), hwTmp.xmm());
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
        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frInput);
        em.callRuntimeWithSavedIP(slowCall, slowCallName);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
      });
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

  if (leftIsNum)
    emitTypeAssert(frLeft, hwLeft, TypePred::IsNumber);
  if (rightIsNum)
    emitTypeAssert(frRight, hwRight, TypePred::IsNumber);

  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN.
    // x86-64: vucomisd sets PF when either operand is NaN, which is what
    // arm64 reads out of the VS condition code, so this is `jp`.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.vucomisd(hwLeft.xmm(), hwRight.xmm());
    a.jp(slowPathLab);
  }

  // Make sure xmm0, xmm1 are unused.
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
        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frLeft);
        em.loadFrameAddr(x86::rdx, frRight);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(
                SHRuntime *, const SHLegacyValue *, const SHLegacyValue *),
            _sh_ljs_mod_rjs);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
      });
}

void Emitter::arithBinOp(
    bool forceNumber,
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    void (*fast)(
        x86::Assembler &a,
        const x86::Xmm &res,
        const x86::Xmm &dl,
        const x86::Xmm &dr),
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

  if (leftIsNum)
    emitTypeAssert(frLeft, hwLeft, TypePred::IsNumber);
  if (rightIsNum)
    emitTypeAssert(frRight, hwRight, TypePred::IsNumber);

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
    // NaN.
    // x86-64: see mod() -- PF is set when either operand is NaN, so `jp`.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.vucomisd(hwLeft.xmm(), hwRight.xmm());
    a.jp(slowPathLab);
  }

  fast(a, hwRes.xmm(), hwLeft.xmm(), hwRight.xmm());

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
        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frLeft);
        em.loadFrameAddr(x86::rdx, frRight);
        em.callRuntimeWithSavedIP(slowCall, slowCallName);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
      });
}

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
