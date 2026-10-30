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

namespace hermes::vm::x86_64 {

void Emitter::ret(FR frValue) {
  // kGPReturnStash (rbx) is the return value stash (the x21 analogue):
  // leave() moves it to rax, the SysV return register, after restoring the
  // frame. Clobbering rbx here without invalidating any FR's HWReg state is
  // sound only because every path through this mov terminates at leave():
  // there is no fall-through or branch back into the FR allocator that
  // could observe rbx's old, now-stale contents.
  movHWFromFR(HWReg::gpX(kGPReturnStash), frValue);
  a.jmp(returnLabel_);
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
    emitTypeAssert(frInput, hwInput, TypePred::IsNumber);
    // x86-64: arm64 needs two branches, on < 0 and on > 0, because its
    // fcmp reports "less" and "greater" separately and both are false on
    // unordered. vucomisd sets ZF for equal *and* for unordered, so a
    // truthy number is exactly ZF==0 -- one branch, with NaN falling out
    // for free.
    a.vucomisd(hwInput.xmm(), roConst64(0, "0.0"));
    a.j(onTrue ? x86::CondCode::kNE : x86::CondCode::kE, target);
  } else if (isFRKnownType(frInput, FRType::Bool)) {
    HWReg hwInput = getOrAllocFRInGpX(frInput, true);
    emitTypeAssert(frInput, hwInput, TypePred::IsBool);

    static_assert(
        HERMESVALUE_VERSION == 2, "bool is encoded as a bit at kHV_BoolBitIdx");
    // x86-64: kHV_BoolBitIdx is far above the 32 bits that a `test`
    // immediate can reach, so read the bit with `bt`, which takes an 8-bit
    // index and reports the bit in CF.
    a.bt(hwInput.gpq(), kHV_BoolBitIdx);
    a.j(onTrue ? x86::CondCode::kC : x86::CondCode::kNC, target);
  } else {
    // TODO: we should inline all of it.
    syncAllFRTempExcept({});
    // x86-64: the by-value SHLegacyValue argument goes in rdi.
    movHWFromFR(HWReg(x86::rdi), frInput);
    EMIT_RUNTIME_CALL(*this, bool (*)(SHLegacyValue), _sh_ljs_to_boolean);
    // x86-64: SysV defines only al for a bool return.
    a.test(x86::al, x86::al);
    a.j(onTrue ? x86::CondCode::kNZ : x86::CondCode::kZ, target);
    freeAllFRTempExcept(FR());
  }
}

void Emitter::jmp(const asmjit::Label &target) {
  comment("// Jmp Lx");
  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());
  a.jmp(target);
}

void Emitter::jmpUndefined(const asmjit::Label &target, FR frInput) {
  comment("// JmpUndefined r%u", frInput.index());

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());

  if (isFRKnownType(frInput, FRType::Number) ||
      isFRKnownType(frInput, FRType::Bool)) {
    emitTypeAssertFR(
        frInput,
        isFRKnownType(frInput, FRType::Number) ? TypePred::IsNumber
                                               : TypePred::IsBool);
    return;
  }

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTmpTag = allocTempGpX();

  emit_sh_ljs_is_undefined(a, hwTmpTag.gpq(), hwInput.gpq());
  a.je(target);

  freeReg(hwTmpTag);
}

void Emitter::jCond(
    bool forceNumber,
    bool invert,
    bool passArgsByVal,
    const asmjit::Label &target,
    FR frLeft,
    FR frRight,
    const char *name,
    x86::CondCode condCode,
    bool swapOperands,
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

  if (leftIsNum)
    emitTypeAssert(frLeft, hwLeft, TypePred::IsNumber);
  if (rightIsNum)
    emitTypeAssert(frRight, hwRight, TypePred::IsNumber);

  // x86-64: "less" and "less or equal" compare the reversed operands, so
  // that the condition stays in the above family. See the condition mapping
  // in the design.
  a.vucomisd(
      swapOperands ? hwRight.xmm() : hwLeft.xmm(),
      swapOperands ? hwLeft.xmm() : hwRight.xmm());

  // x86-64: kA and kAE are false on unordered, exactly like the arm64 codes
  // this was ported from, so their branches may precede the unordered
  // routing. kE is not -- vucomisd sets ZF on unordered -- so its branch
  // must always come after unordered has been dealt with.
  bool const unorderedSafe = condCode != x86::CondCode::kE;

  // If the condition is not inverted, then it can only produce true if both
  // operands are numbers. Since we use NaN boxing, we know that all non-number
  // values will be NaN and therefore produce false. So if the result is true,
  // we can take the jump without checking for numbers.
  if (!invert && unorderedSafe)
    a.j(condCode, target);

  // Where there is a slow path, unordered means "not both numbers" and is
  // routed there. Where there is not, both operands are statically numbers,
  // but either can still be the JS NaN; only the equal family, whose
  // condition codes read ZF, has to do anything about that.
  asmjit::Label unorderedSkipLab;
  if (slow) {
    // Since HermesValue is NaN-boxed we know that all non-number values will be
    // NaN. So we can conveniently test for non-number values by checking for
    // NaN. x86-64: unordered is reported in PF.
    static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
    a.jp(slowPathLab);
  } else if (!unorderedSafe) {
    if (invert) {
      // !(NaN == x) is true.
      a.jp(target);
    } else {
      // NaN == x is false: skip the branch below.
      unorderedSkipLab = a.newLabel();
      a.jp(unorderedSkipLab);
    }
  }

  // If the condition is inverted, it will produce true if one of the operands
  // is a NaN, so we can only check it after the slow path check, since it would
  // incorrectly be taken for non-numbers.
  if (invert)
    a.j(x86::negateCond(condCode), target);
  else if (!unorderedSafe)
    a.j(condCode, target);

  if (unorderedSkipLab.isValid())
    a.bind(unorderedSkipLab);

  if (!slow)
    return;

  a.bind(contLab);

  // Do this always, since this is the end of the BB.
  freeAllFRTempExcept(FR());

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [name,
       slowCall,
       slowCallName,
       target,
       frLeft,
       frRight,
       invert,
       passArgsByVal](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: j_%s%s Lx, r%u, r%u",
            invert ? "not_" : "",
            name,
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        if (passArgsByVal) {
          em._loadFrame(HWReg(x86::rdi), frLeft);
          em._loadFrame(HWReg(x86::rsi), frRight);
        } else {
          em.a.mov(x86::rdi, xRuntime);
          em.loadFrameAddr(x86::rsi, frLeft);
          em.loadFrameAddr(x86::rdx, frRight);
        }
        em.callRuntimeWithSavedIP(slowCall, slowCallName);
        // x86-64: SysV defines only al for a bool return.
        em.a.test(x86::al, x86::al);
        em.a.j(!invert ? x86::CondCode::kNZ : x86::CondCode::kZ, target);
        em.a.jmp(sp.contLab);
      });
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

    if (isFRKnownBool(frLeft) || isFRKnownOtherNonPtr(frLeft))
      emitTypeAssert(frLeft, hwLeft, TypePred::BitComparable);
    if (isFRKnownBool(frRight) || isFRKnownOtherNonPtr(frRight))
      emitTypeAssert(frRight, hwRight, TypePred::BitComparable);

    // An integer compare is never unordered, so kE/kNE are exact.
    a.cmp(hwLeft.gpq(), hwRight.gpq());
    a.j(!invert ? x86::CondCode::kE : x86::CondCode::kNE, target);
    return;
  }

  // Fast path for number (double) comparison.
  // One of the operands is a number, so there's two cases:
  // * It's NaN: it'll compare false against everything which is what we want.
  // * It's not NaN: It won't have NaN tag bits so it'll compare false against
  //   all non-double HVs and correctly compare true against the same number.
  if (isFRKnownNumber(frLeft) || isFRKnownNumber(frRight)) {
    // Do this always, since this could be the end of the BB.
    syncAllFRTempExcept(FR());
    HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
    HWReg hwRightD = getOrAllocFRInVecD(frRight, true);
    freeAllFRTempExcept({});

    if (isFRKnownNumber(frLeft))
      emitTypeAssert(frLeft, hwLeftD, TypePred::IsNumber);
    if (isFRKnownNumber(frRight))
      emitTypeAssert(frRight, hwRightD, TypePred::IsNumber);

    a.vucomisd(hwLeftD.xmm(), hwRightD.xmm());
    // x86-64: unordered -- a JS NaN, or a NaN-boxed non-number against the
    // operand known to be a number -- means "not strictly equal", but ZF
    // says the opposite, so PF has to be consulted first.
    if (!invert) {
      asmjit::Label skipLab = a.newLabel();
      a.jp(skipLab);
      a.je(target);
      a.bind(skipLab);
    } else {
      a.jp(target);
      a.jne(target);
    }
    return;
  }

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());

  HWReg hwLeftD = getOrAllocFRInVecD(frLeft, true);
  HWReg hwRightD = getOrAllocFRInVecD(frRight, true);

  HWReg hwTmpLeft = allocTempGpX();
  x86::Gp xTmpLeft = hwTmpLeft.gpq();
  HWReg hwTmpRight = allocTempGpX();
  x86::Gp xTmpRight = hwTmpRight.gpq();
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

  a.vucomisd(hwLeftD.xmm(), hwRightD.xmm());
  // Since HermesValue is NaN-boxed we know that all non-number values will be
  // NaN. So we can conveniently test for non-number values by checking for
  // NaN. x86-64: unordered is reported in PF -- and, unlike arm64's kEQ,
  // x86's kE is *true* on unordered, so the equality branch cannot precede
  // this routing; it follows it instead.
  static_assert(HERMESVALUE_VERSION == 2, "Non-numbers must be NaN");
  a.jp(nonNumberLab);
  // Neither operand is NaN, so we can jump to the correct endpoint: equality
  // here is real equality, and inequality is real inequality.
  // If the branch fails, we go to contLab.
  a.j(!invert ? x86::CondCode::kE : x86::CondCode::kNE, target);
  a.jmp(contLab);

  // Convenience labels so we don't have to think too hard about inverted logic
  // below.
  const asmjit::Label &equalLab = !invert ? target : contLab;
  const asmjit::Label &notEqualLab = !invert ? contLab : target;

  a.bind(nonNumberLab);
  emit_sh_ljs_is_double(a, hwLeft.gpq(), xTmpLeft);
  // Left is actually the JS NaN, which is never equal to anything.
  // No need to check RHS for JS NaN, because it won't cause false positive
  // on the raw bit check below.
  a.jb(notEqualLab);

  // Compare bits directly.
  // If they match exactly, the two values are equal.
  a.cmp(hwLeft.gpq(), hwRight.gpq());
  a.je(equalLab);

  // First compare the tags. If they don't match, the two values are NOT equal.
  emit_sh_ljs_get_tag(a, xTmpLeft, hwLeft.gpq());
  emit_sh_ljs_get_tag(a, xTmpRight, hwRight.gpq());
  a.cmp(xTmpLeft, xTmpRight);
  a.jne(notEqualLab);

  // If the LHS is either a non-pointer or an object, we can compare raw values
  // only. We've already checked and we know that the raw values are not the
  // same, so if this is a non-pointer or an object, then the two values are NOT
  // strictly equal.
  emit_sh_ljs_tag_is_pointer(a, xTmpLeft);
  a.jb(notEqualLab);
  emit_sh_ljs_tag_is_object(a, xTmpLeft);
  a.je(notEqualLab);

  // Now we know that the LHS is a non-object pointer.

  // Fast string path: string inequality can be easily determined by checking
  // the lengths. If the LHS isn't a string, go to slow path.
  emit_sh_ljs_tag_is_string(a, xTmpLeft);
  a.jne(slowPathLab);

  emit_stringprim_get_length_and_flags(a, xTmpLeft, hwLeft.gpq());
  emit_stringprim_get_length_and_flags(a, xTmpRight, hwRight.gpq());
  // XOR the lengths together and mask the result.
  // xTmpLeft will be nonzero if the lengths don't match.
  // x86-64: the loads zero-extend, so 32-bit operations are equivalent to
  // arm64's 64-bit ones here, and the mask fits in an imm32.
  a.xor_(xTmpLeft.r32(), xTmpRight.r32());
  a.and_(
      xTmpLeft.r32(), asmjit::Imm(RuntimeOffsets::stringPrimitiveLengthMask));
  // Length mismatch means we're done, the two values are NOT equal.
  a.jnz(notEqualLab);
  a.jmp(slowPathLab);

  a.bind(contLab);

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [target, frLeft, frRight, invert](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: %s Lx, r%u, r%u",
            invert ? "j_strict_not_eq" : "j_strict_eq",
            frLeft.index(),
            frRight.index());
        em.a.bind(sp.slowPathLab);
        em._loadFrame(HWReg(x86::rdi), frLeft);
        em._loadFrame(HWReg(x86::rsi), frRight);
        em.callRuntimeWithSavedIP(
            (void *)_sh_ljs_strict_equal, "_sh_ljs_strict_equal");
        // x86-64: SysV defines only al for a bool return.
        em.a.test(x86::al, x86::al);
        em.a.j(!invert ? x86::CondCode::kNZ : x86::CondCode::kZ, target);
        em.a.jmp(sp.contLab);
      });
}

} // namespace hermes::vm::x86_64

#endif // HERMESVM_JIT_X86_64
