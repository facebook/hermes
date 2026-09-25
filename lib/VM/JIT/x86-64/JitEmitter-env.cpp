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

namespace hermes::vm::x86_64 {

void Emitter::createTopLevelEnvironment(FR frRes, uint32_t size) {
  comment("// CreateTopLevelEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(x86::rdi, xRuntime);
  // The parent environment is nullptr.
  // x86-64: `xor` also writes EFLAGS, where arm64's `mov x1, 0` does not.
  a.xor_(x86::esi, x86::esi);
  a.mov(x86::edx, asmjit::Imm(size));

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
  auto res = hwRes.gpq();

  // Allocate some temporaries.
  HWReg hwTemp1 = allocTempGpX();
  HWReg hwTemp2 = allocTempGpX();
  auto hwTempV = allocTempVecD();
  auto temp1 = hwTemp1.gpq();
  auto temp2 = hwTemp2.gpq();
  auto vTemp = hwTempV.xmm();
  freeReg(hwTemp1);
  freeReg(hwTemp2);
  freeReg(hwTempV);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  // Try to allocate the new environment cell.
  allocInYoung(
      CellKind::EnvironmentKind,
      Environment::allocationSize(size),
      res,
      temp1,
      temp2,
      slowPathLab);

  // Get the current closure pointer.
  // x86-64: arm64 needs ldur for the negative offset; every x86 memory
  // operand carries a signed 32-bit displacement, so this is a plain mov.
  a.mov(
      temp1,
      x86::qword_ptr(
          xFrame,
          (int32_t)StackFrameLayout::CalleeClosureOrCB *
              (int32_t)sizeof(SHLegacyValue)));
  emit_sh_ljs_get_pointer(a, temp1, temp1);

  // temp1 = closure->environment
  emit_load_cp(a, temp1, x86::ptr(temp1, offsetof(SHCallable, environment)));

  // Initialize the environment cell.
  emit_environment_init(a, res, /* parentEnv */ temp1, temp2, vTemp, size);

  // Encode the cell as a HermesValue.
  emit_sh_ljs_object(a, res);

  a.bind(contLab);

  slowPaths_.emplace_back(
      slowPathLab,
      contLab,
      emittingIP,
      [frRes, hwRes, size](Emitter &em, SlowPath &sp) {
        em.comment(
            "// Slow path: CreateFunctionEnvironment r%u", frRes.index());
        em.a.bind(sp.slowPathLab);
        em.a.mov(x86::rdi, xRuntime);
        em.a.mov(x86::rsi, xFrame);
        em.a.mov(x86::edx, asmjit::Imm(size));

        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, SHLegacyValue *, uint32_t),
            _sh_ljs_create_function_environment);
        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
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

  auto temp1 = hwTemp1.gpq();
  auto temp2 = hwTemp2.gpq();
  auto newEnvPtr = hwNewEnvPtr.gpq();
  auto vTemp = hwTempV.xmm();

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
      newEnvPtr,
      temp1,
      temp2,
      slowPathLab);

  // Load a compressed pointer to the parent environment in temp1.
  emit_sh_ljs_get_pointer(a, temp1, hwParent.gpq());
  emit_sh_cp_encode_non_null(a, temp1);

  emit_environment_init(
      a, newEnvPtr, /* parentEnv */ temp1, temp2, vTemp, size);

  // Finally, allocate the result register.
  HWReg hwRes = getOrAllocFRInGpX(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);

  emit_sh_ljs_object2(a, hwRes.gpq(), newEnvPtr);

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

        em.a.mov(x86::rdi, xRuntime);
        em.loadFrameAddr(x86::rsi, frParent);
        em.a.mov(x86::edx, asmjit::Imm(size));

        EMIT_RUNTIME_CALL(
            em,
            SHLegacyValue (*)(SHRuntime *, const SHLegacyValue *, uint32_t),
            _sh_ljs_create_environment);

        em.movHWFromHW<false>(hwRes, HWReg::gpX(0));
        em.a.jmp(sp.contLab);
      });
}

void Emitter::getParentEnvironment(FR frRes, uint32_t level) {
  comment("// GetParentEnvironment r%u, %u", frRes.index(), level);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  x86::Gp res = hwRes.gpq();
  frUpdatedWithHW(frRes, hwRes);

  // Get current closure.
  // x86-64: see createFunctionEnvironment -- the negative frame offset is
  // just a displacement here, so arm64's ldur is a plain mov.
  a.mov(
      res,
      x86::qword_ptr(
          xFrame,
          (int32_t)StackFrameLayout::CalleeClosureOrCB *
              (int32_t)sizeof(SHLegacyValue)));
  // get pointer.
  emit_sh_ljs_get_pointer(a, res, res);
  // res = closure->environment
  emit_load_cp(a, res, x86::ptr(res, offsetof(SHCallable, environment)));
  emit_sh_cp_decode_non_null(a, res);
  for (; level; --level) {
    // res = env->parent.
    emit_load_cp(
        a, res, x86::ptr(res, offsetof(SHEnvironment, parentEnvironment)));
    emit_sh_cp_decode_non_null(a, res);
  }
  // encode object.
  emit_sh_ljs_object(a, res);
}

void Emitter::getEnvironment(FR frRes, FR frSource, uint32_t level) {
  comment(
      "// GetEnvironment r%u, r%u, %u", frRes.index(), frSource.index(), level);

  HWReg hwSource = getOrAllocFRInGpX(frSource, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  frUpdatedWithHW(frRes, hwRes);
  x86::Gp res = hwRes.gpq();

  emit_sh_ljs_get_pointer(a, res, hwSource.gpq());
  for (; level; --level) {
    // res = env->parent.
    emit_load_cp(
        a, res, x86::ptr(res, offsetof(SHEnvironment, parentEnvironment)));
    emit_sh_cp_decode_non_null(a, res);
  }
  // encode object.
  emit_sh_ljs_object(a, res);
}

void Emitter::getClosureEnvironment(FR frRes, FR frClosure) {
  comment(
      "// GetClosureEnvironment r%u, r%u", frRes.index(), frClosure.index());
  // We know the layout of the closure, so we can load directly.
  auto ofs = offsetof(SHCallable, environment);
  auto hwClosure = getOrAllocFRInGpX(frClosure, true);
  auto hwRes = getOrAllocFRInGpX(frRes, false);
  // Use the result register as a scratch register for computing the address.
  emit_sh_ljs_get_pointer(a, hwRes.gpq(), hwClosure.gpq());
  emit_load_cp(a, hwRes.gpq(), x86::ptr(hwRes.gpq(), ofs));
  emit_sh_cp_decode_non_null(a, hwRes.gpq());
  // The result is a pointer, so add the object tag.
  emit_sh_ljs_object(a, hwRes.gpq());
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
  x86::Gp tmp1 = hwTmp1.gpq();

  movHWFromFR(hwTmp1, frEnv);
  // get pointer.
  emit_sh_ljs_get_pointer(a, tmp1, tmp1);

  // x86-64: arm64 needs a scratch register once the slot index pushes the
  // offset past its scaled immediate (around slot 4092); a signed 32-bit
  // displacement covers every slot a UInt16 index can name, so the load is
  // unconditional here and no scratch is involved.
  size_t ofs =
      offsetof(SHEnvironment, slots) + sizeof(SHLegacyValue) * (size_t)slot;
  assert(ofs <= (size_t)INT32_MAX && "slot offset must fit a disp32");
  a.mov(tmp1, x86::qword_ptr(tmp1, (int32_t)ofs));

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
  // want to load into parameter registers (rdi, rsi, etc) by value. Some of
  // these FRs may live in the parameter registers we want to use, but some may
  // not. So, first we make sure that the FRs that live in rdi, rsi, etc., are
  // synced to their primary location and the temps rdi, rsi, etc., are freed.
  // As we do this, we immediately move the corresponding parameter from its
  // corresponding FR, to maximize the chance that it can be moved from a
  // register.
  //
  // x86-64: the argument registers are the SysV ones, not the first four
  // allocatable Gps -- on arm64 they happen to be the same registers.

  // Make sure rdi, rsi, rdx, rcx are unused.
  syncAndFreeTempReg(HWReg(x86::rdi));
  a.mov(x86::rdi, xRuntime);

  syncAndFreeTempReg(HWReg(x86::rsi));
  movHWFromFR(HWReg(x86::rsi), frEnv);

  syncAndFreeTempReg(HWReg(x86::rdx));
  movHWFromFR(HWReg(x86::rdx), frValue);

  syncAndFreeTempReg(HWReg(x86::rcx));
  a.mov(x86::ecx, asmjit::Imm(slot));

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

  a.mov(x86::rdi, xRuntime);
  loadFrameAddr(x86::rsi, frEnv);
  loadBits64InGp(x86::rdx, (uint64_t)runtimeModule, "RuntimeModule");
  loadBits64InGp(x86::ecx, functionID, nullptr);
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

  a.mov(x86::rdi, xRuntime);
  // The interpreter expects that the frameRegs it receives starts on the first
  // local register.
  auto ofs = hbc::StackFrameLayout::FirstLocal * sizeof(SHLegacyValue);
  a.lea(x86::rsi, x86::ptr(xFrame, (int32_t)ofs));
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

  a.mov(x86::rdi, xRuntime);
  // The interpreter expects that the frameRegs it receives starts on the first
  // local register.
  auto ofs = hbc::StackFrameLayout::FirstLocal * sizeof(SHLegacyValue);
  a.lea(x86::rsi, x86::ptr(xFrame, (int32_t)ofs));
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

  a.mov(x86::rdi, xRuntime);
  a.mov(x86::rsi, xFrame);
  loadFrameAddr(x86::rdx, frEnv);
  loadBits64InGp(x86::rcx, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(x86::r8d, asmjit::Imm(functionID));
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

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
