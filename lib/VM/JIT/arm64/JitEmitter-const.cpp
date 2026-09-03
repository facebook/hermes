/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_ARM64
#include "JitEmitter-internal.h"
#include "JitEmitter.h"
#include "../JitHandlers.h"

namespace hermes::vm::arm64 {

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
      SHLegacyValue (*)(SHRuntime *, SHRuntimeModule *, uint32_t),
      _sh_ljs_get_bytecode_bigint);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<true>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

} // namespace hermes::vm::arm64

#endif // HERMESVM_JIT_ARM64
