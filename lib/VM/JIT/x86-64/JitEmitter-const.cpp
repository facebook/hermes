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

void Emitter::loadConstDouble(FR frRes, double val, const char *name) {
  comment("// LoadConst%s r%u, %f", name, frRes.index(), val);
  HWReg hwRes{};

  // Check bitwise for zero because it may be -0.
  if (llvh::DoubleToBits(val) == llvh::DoubleToBits(0)) {
    // TODO: this check should be wider.
    hwRes = getOrAllocFRInVecD(frRes, false);
    a.vpxor(hwRes.xmm(), hwRes.xmm(), hwRes.xmm());
  } else {
    // x86-64: every other double is one `mov gpq, imm64`, so arm64's
    // fp-immediate and constant-pool cases both collapse into the Gp case.
    // TODO: a double that is only ever consumed by arithmetic would be
    // better materialized in a vector register, at the cost of a constant
    // pool entry. Revisit when the arithmetic emitters land.
    hwRes = getOrAllocFRInGpX(frRes, false);
    a.mov(hwRes.gpq(), asmjit::Imm(llvh::DoubleToBits(val)));
  }
  frUpdatedWithHW(frRes, hwRes, FRType::Number);
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

  loadBits64InGp(hwRes.gpq(), bits, name);
  frUpdatedWithHW(frRes, hwRes, type);
}

} // namespace hermes::vm::x86_64

#endif // HERMESVM_JIT_X86_64
