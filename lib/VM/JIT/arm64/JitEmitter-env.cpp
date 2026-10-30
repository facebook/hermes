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

namespace hermes::vm::arm64 {

void Emitter::createTopLevelEnvironment(FR frRes, uint32_t size) {
  comment("// CreateTopLevelEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, 0);
  a.mov(a64::w2, size);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *, uint32_t),
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, hwRes, size](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: CreateFunctionEnvironment r%u", frRes.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::x0, xRuntime);
        em.a.mov(a64::x1, xFrame);
        em.a.mov(a64::w2, size);

        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, SHLegacyValue *, uint32_t),
            _sh_ljs_create_function_environment);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, frParent, hwRes, size](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: CreateEnvironment r%u, r%u",
            frRes.index(),
            frParent.index());
        em.a.bind(sp.slowPathLab);

        em.a.mov(a64::x0, xRuntime);
        em.loadFrameAddr(a64::x1, frParent);
        em.a.mov(a64::w2, size);

        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *, uint32_t),
            _sh_ljs_create_environment);

        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.b(sp.contLab);
      });
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

  // xScratch is touched only if the offset is too large for the scaled
  // immediate: LoadFromEnvironmentL carries a UInt16 slot, and past slot
  // ~4092 the displacement no longer encodes.
  emit_load_from_base_offset<sizeof(SHLegacyValue)>(
      a,
      xTmp1,
      xTmp1,
      xScratch,
      offsetof(SHEnvironment, slots) + sizeof(SHLegacyValue) * slot);

  freeReg(hwTmp1);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, hwTmp1);
  movHWFromHW<false>(hwRes, hwTmp1);
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::storeToEnvironment(bool np, FR frEnv, uint32_t slot, FR frValue) {
  // TODO: this should really be inlined!
  comment(
      np ? "// StoreNPToEnvironment r%u, %u, r%u"
         : "// StoreToEnvironment r%u, %u, r%u",
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
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frEnv);
  loadBits64InGp(a64::x2, (uint64_t)runtimeModule, "RuntimeModule");
  loadBits64InGp(a64::w3, functionID, nullptr);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue (*)(
          SHRuntime *, const SHLegacyValue *, SHRuntimeModule *, uint32_t),
      _sh_ljs_create_bytecode_closure);

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
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  loadFrameAddr(a64::x2, frEnv);
  loadBits64InGp(a64::x3, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(a64::w4, functionID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue (*)(
          SHRuntime *,
          SHLegacyValue *,
          const SHLegacyValue *,
          SHRuntimeModule *,
          uint32_t),
      _interpreter_create_generator);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
