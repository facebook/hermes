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

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/VM/VTable.h"

namespace hermes::vm::arm64 {

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
      emit_add_imm_u24(a, a64::x3, sizeof(SHReadPropertyCacheEntry) * cacheIdx);
  }
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue (*)(
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, hwRes](Emitter &em, SlowPath &sp) {
        em.comment("// Slow path: LoadThisNS r%u", frRes.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.a.ldur(
            a64::x1,
            a64::Mem(
                xFrame,
                StackFrameLayout::ThisArg * (int)sizeof(SHLegacyValue)));
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, SHLegacyValue),
            _sh_ljs_coerce_this_ns);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frThis, hwRes](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: CoerceThis r%u, r%u", frRes.index(), frThis.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em._loadFrame(HWReg(a64::x1), frThis);
        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, SHLegacyValue),
            _sh_ljs_coerce_this_ns);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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
    slowPaths_.emplace_back(
        nonObjCallLabel_,
        /* emittingIP */ nullptr,
        [](Emitter &em, SlowPath &sp) {
          em.comment("// Throw on non-object call");
          em.a.bind(sp.slowPathLab);
          em.a.mov(a64::x0, xRuntime);
          // The IP is already saved by the call setup, no need to save it
          // again.
          EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
              em, void (*)(SHRuntime *), _jit_throw_non_object_call);
          // The call does not return.
        });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      /* emittingIP */ nullptr,
      [frRes, frCallee](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: CallImpl r%u, r%u", frRes.index(), frCallee.index());
        em.a.bind(sp.slowPathLab);
        em.emitIncrementCounter(JitCounter::NumCallSlow);
        em.loadBits64InGp(
            a64::x2, (uint64_t)&VTable::jitCallArray, "VTable::jitCallArray");
        // Load the jitCall function. x0 still contains the callee CellKind
        // from the fast path.
        em.a.ldr(
            a64::x2,
            a64::Mem(a64::x2, a64::x0, a64::Shift(a64::ShiftOp::kLSL, 3)));
        // x1 already contains the Callable* from the fast path.
        em.a.b(sp.contLab);
      });
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
  // The ThisArg slot is not populated by bytecode; _jit_call_builtin
  // initializes it. Note that the syncAllFRTempExcept({}) below does write
  // the ThisArg FR to memory along with everything else — it is simply that
  // whatever it writes there is dead, since the handler overwrites it.
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
      SHLegacyValue (*)(SHRuntime *, SHLegacyValue *, uint32_t, uint32_t),
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
      SHLegacyValue (*)(
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

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
