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

#include "llvh/Support/Compiler.h"

namespace hermes::vm::arm64 {

void Emitter::bumpAllocAndUnpoison(
    uint32_t sz,
    const a64::GpX &xOut,
    const a64::GpX &xTemp1,
    const a64::GpX &xTemp2,
    const asmjit::Label &slowPathLab) {
#if HERMESVM_GCKIND == _HERMESVM_GCVALUE_HADES
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
  EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
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

} // namespace hermes::vm::arm64

#endif // HERMESVM_JIT_ARM64
