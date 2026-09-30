/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_ARM64

#include "JitEmitter-internal.h"

#include "hermes/VM/JSObject-inline.h"
#include "llvh/Support/Debug.h"

#define DEBUG_TYPE "jit"

namespace hermes::vm::arm64 {

void Emit_sh_shv_decode::emitFirstCase(a64::Assembler &a) {
#ifdef HERMESVM_BOXED_DOUBLES
  static_assert(HERMESVALUE_VERSION == 2, "Constructing HermesValue from bits");
  static_assert(HermesValue32::kVersion == 1, "Decoding HV32 bits");
  constexpr uint64_t kHV32TagMask = (1 << HermesValue32::kNumTagBits) - 1;

  // Start by testing the tag bits for 0, if so, this is a "compressed HV64".
  static_assert((int)HermesValue32::Tag::CompressedHV64 == 0);
  a.tst(xInOut, kHV32TagMask);
  // If the tag is non-zero, jump to the pointer check.
  a.b_ne(ptrLab);
  // CompressedHV64, just shift the bits if needed. Note the case where
  // HermesValue32 is actually 64 bits is only used for testing, so it is not
  // worth optimising here.
  if constexpr (sizeof(SmallHermesValue) < sizeof(HermesValue))
    a.lsl(xInOut, xInOut, 32);
#endif
}

void Emit_sh_shv_decode::emitRestCases(a64::Assembler &a) {
#ifndef NDEBUG
  assert(!restEmitted && "Rest cases already emitted");
  restEmitted = true;
#endif
#ifdef HERMESVM_BOXED_DOUBLES
  asmjit::Label symLab = a.newLabel();
  asmjit::Label bdLab = a.newLabel();

  static_assert(HERMESVALUE_VERSION == 2, "Constructing HermesValue from bits");
  static_assert(HermesValue32::kVersion == 1, "Decoding HV32 bits");
  constexpr uint64_t kHV32TagMask = (1 << HermesValue32::kNumTagBits) - 1;

  a.bind(ptrLab);

  // See the comments below for why the exact bits are important.
  static_assert((int)HermesValue32::Tag::String == 0b01);
  static_assert((int)HermesValue32::Tag::BigInt == 0b10);
  static_assert((int)HermesValue32::Tag::Object == 0b11);
  // We know that the tag is 3 bits, and we have already checked for 0, so we
  // can check for the 3 interesting pointer tags (which are in the range 1-3,
  // i.e., less than 2^2) just by testing the most significant bit of the tag.
  a.tbnz(xInOut, 2, symLab);
  // We can decompress the pointer right away, since the HV32 representation
  // just makes it look like an unaligned compressed pointer.
  emit_sh_cp_decode_non_null(a, xInOut);
  // Now we insert the new tag. First compute the "base" tag. The HV64 tag for a
  // pointer can be computed by adding the HV32 tag to this base.
  constexpr uint16_t baseTag =
      (uint16_t)HermesValue::Tag::Str - (uint16_t)HermesValue32::Tag::String;
  // Move in the base tag.
  a.movk(xInOut, baseTag, kHV_NumDataBits);

  // We know that the base tag has zeroes in its low 2 bits, and the HV32 tag is
  // known to be in the low 2 bits at this point. So we can use bfi to simply
  // copy those low bits over.
  static_assert((baseTag & 0b11) == 0);
  a.bfi(xInOut, xInOut, kHV_NumDataBits, 2);

  // Clear the HV32 tag in the low bits.
  a.and_(xInOut, xInOut, ~kHV32TagMask);
  a.b(doneLab);

  a.bind(symLab);
  // There are only two cases left, Symbol and BoxedDouble. We can distinguish
  // them with a single bit.
  static_assert((int)HermesValue32::Tag::BoxedDouble == 0b100);
  static_assert((int)HermesValue32::Tag::Symbol == 0b101);
  static_assert((int)HermesValue32::Tag::_Last == 0b110);
  // Test the low bit, to distinguish Symbol and BoxedDouble.
  a.tbz(xInOut, 0, bdLab);
  // SymbolID has a 17-bit tag in HV64, which means the tag cannot be inserted
  // in a single instruction. However, we can exploit the fact that HV64 tags
  // are particular double NaN's, with a sign bit of 1 and all-1 exponents, so
  // they start with 0xfff. This means that the most significant bits of the tag
  // are 1, and we can shift in those bits with an asr below as we shift out the
  // HV32 tag. First, insert the low 14 bits of the symbol tag as a 16 bit
  // immediate.
  constexpr uint16_t symTag =
      (uint16_t)((uint32_t)HVETag_Symbol << (HermesValue32::kNumTagBits - 1));
  // Assert that the top bits are preserved with an arithmetic right shift.
  static_assert(
      ((int16_t)symTag) >> (HermesValue32::kNumTagBits - 1) == HVETag_Symbol);
  a.movk(xInOut, symTag, kHV_NumDataBits);
  // Next, shift in the high 1's as we shift out the HV32 tag.
  a.asr(xInOut, xInOut, HermesValue32::kNumTagBits);
  a.b(doneLab);

  a.bind(bdLab);
  // It is a boxed double. As with pointers, we can decode it as a compressed
  // pointer first.
  emit_sh_cp_decode_non_null(a, xInOut);
  // Since the tag is still present, subtract it from the offset.
  constexpr size_t bdOffs = RuntimeOffsets::boxedDoubleValue -
      (size_t)HermesValue32::Tag::BoxedDouble;
  a.ldr(xInOut, a64::Mem(xInOut, bdOffs));
#endif
}

void emit_stringprim_get_length_and_flags(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  emit_sh_ljs_get_pointer(a, xOut, xIn);
  a.ldr(
      xOut.w(), a64::Mem(xOut, RuntimeOffsets::stringPrimitiveLengthAndFlags));
}

void emit_jsobject_init(
    a64::Assembler &a,
    const a64::GpX &xObj,
    const a64::GpX &xParent,
    const a64::GpX &xTempOrPropStorageOpt,
    bool hasPropStorage,
    const a64::GpX &xClazzOpt) {
  // obj->flags = 0
  a.str(a64::wzr, a64::Mem(xObj, offsetof(SHJSObject, flags)));

  if (hasPropStorage) {
    emit_sh_cp_encode_non_null(a, xTempOrPropStorageOpt);
    emit_store_cp(
        a,
        xTempOrPropStorageOpt,
        a64::Mem(xObj, offsetof(SHJSObject, propStorage)));
  }

  // No longer need the propStorage pointer. Alias for clarity.
  const a64::GpX &xTemp = xTempOrPropStorageOpt;

  if (!xClazzOpt.isValid()) {
    // Load the hidden class compressed pointer into xClazz.
    static_assert(
        JSObject::numOverlapSlots<JSObject>() == 0,
        "Cannot use 0 property root class.");
    a.ldr(xTemp, a64::Mem(xRuntime, RuntimeOffsets::runtimeRootClazzes));
    emit_sh_ljs_get_pointer(a, xTemp, xTemp);
    emit_sh_cp_encode_non_null(a, xTemp);
  }

  const a64::GpX &xClazz = xClazzOpt.isValid() ? xClazzOpt : xTemp;

  // Store the parent and hidden class.
  // obj->parent = xParent
  // obj->clazz = xClazz (may be the same as xTemp).
  assert(xClazz.isValid());
  static_assert(
      offsetof(SHJSObject, clazz) - offsetof(SHJSObject, parent) ==
          sizeof(CompressedPointer),
      "clazz and parent must be adjacent to use stp");
  if constexpr (sizeof(CompressedPointer) == 4)
    a.stp(
        xParent.w(), xClazz.w(), a64::Mem(xObj, offsetof(SHJSObject, parent)));
  else
    a.stp(xParent, xClazz, a64::Mem(xObj, offsetof(SHJSObject, parent)));

  // If !hasPropStorage, obj->propStorage = nullptr.

  // obj->directProps[N] = SmallHermesValue::encodeRawZeroValue()

  // We want to zero the rest of the object. To simplify things, we align the
  // size to the heap alignment of 8 bytes, which ensures that the end of the
  // fill region is aligned to 8 bytes.
  // Start the fill at propStorage if HasPropStorage is false,
  // otherwise start after propStorage in the directProps.
  size_t startZeroOffset = hasPropStorage
      ? offsetof(SHJSObjectAndDirectProps, directProps)
      : offsetof(SHJSObject, propStorage);
  size_t bytesToZero = heapAlignSize(cellSize<JSObject>()) - startZeroOffset;
  size_t zeroedBytes = 0;
  assert(bytesToZero % 4 == 0 && "Must be a multiple of 4");

  // If there is some amount that is not a multiple of 8, store that first.
  if (bytesToZero & 4) {
    a.str(a64::wzr, a64::Mem(xObj, startZeroOffset));
    zeroedBytes += 4;
  }

  // Now store any amount that is not a multiple of 16. Note that since the end
  // of the fill region is aligned to 8 bytes, we know all further stores will
  // be aligned to 8 bytes.
  if (bytesToZero & 8) {
    a.str(a64::xzr, a64::Mem(xObj, startZeroOffset + zeroedBytes));
    zeroedBytes += 8;
  }
  // Store the rest as multiples of 16.
  for (; zeroedBytes < bytesToZero; zeroedBytes += 16) {
    a.stp(a64::xzr, a64::xzr, a64::Mem(xObj, startZeroOffset + zeroedBytes));
  }
  assert(zeroedBytes == bytesToZero && "Did not zero the whole object");
}

/// Emit code to initialize the fields of a newly created environment.
/// \param xNewEnvPtr contains a pointer to the object to initialise.
/// \param xParentEnv contains a compressed pointer to the parent environment.
/// \param xTemp is a temporary register for use by the emitted code.
/// \param vTemp is a temporary vector register for use by the emitted code.
/// \param size is the number of slots in the new environment.
void emit_environment_init(
    a64::Assembler &a,
    const a64::GpX &xNewEnvPtr,
    const a64::GpX &xParentEnv,
    const a64::GpX &xTemp,
    const a64::VecV &vTemp,
    uint32_t size) {
  // Store the size and parent to the first two fields.
  a.mov(xTemp, size);
  if constexpr (sizeof(CompressedPointer) == 4) {
    a.stp(
        xParentEnv.w(),
        xTemp.w(),
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, parentEnvironment)));
  } else {
    a.str(
        xParentEnv,
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, parentEnvironment)));
    a.str(xTemp.w(), a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, size)));
  }

  // Load undefined.
  a.mov(xTemp, _sh_ljs_undefined().raw);

  auto slotsToFill = size;
  // Before we fill 4 at a time, make sure any excess slots are filled.
  if (slotsToFill % 2) {
    a.str(xTemp, a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, slots)));
    --slotsToFill;
  }
  if (slotsToFill % 4) {
    auto slotOffs = sizeof(SHLegacyValue) * (size - slotsToFill);
    a.stp(
        xTemp,
        xTemp,
        a64::Mem(xNewEnvPtr, offsetof(SHEnvironment, slots) + slotOffs));
    slotsToFill -= 2;
  }

  // The remaining number of slots must be a multiple of 4, which we can fill 4
  // at a time.
  assert((slotsToFill % 4) == 0 && "Remaining slots must be multiple of 4");
  if (slotsToFill) {
    // Copy the value into a vector register, so we can fill faster.
    a.dup(vTemp.d2(), xTemp);

    auto slotOffs = sizeof(SHLegacyValue) * (size - slotsToFill);
    // Initialize the pointer to the slots.
    a.add(xTemp, xNewEnvPtr, offsetof(SHEnvironment, slots) + slotOffs);

    // Fill the slots with undefined 4 at a time.
    for (; slotsToFill; slotsToFill -= 4)
      a.stp(vTemp, vTemp, a64::Mem(xTemp).post(32));
  }
  assert(slotsToFill == 0 && "All slots must be filled");
}

void OurErrorHandler::handleError(
    asmjit::Error err,
    const char *message,
    asmjit::BaseEmitter *origin) {
  if (err == expectedError_) {
    LLVM_DEBUG(
        llvh::outs() << "Expected AsmJit error: " << err << ": "
                     << asmjit::DebugUtils::errorAsString(err) << ": "
                     << message << "\n");
    return;
  }

  std::string formattedMsg{};
  {
    // Ensure we run any destructors for the ostream before longjmp.
    llvh::raw_string_ostream OS{formattedMsg};
    OS << "AsmJit error: " << err << ": "
       << asmjit::DebugUtils::errorAsString(err) << ": " << message;
    OS.flush();
  }

  // IMPORTANT: From here on, we MUST ensure that no destructors need to run.
  // One exception: formattedMsg will have its destructor skipped, but we're
  // moving out of it so in practice the std::string won't have anything to
  // free, avoiding leaks.
  LLVM_DEBUG(llvh::dbgs() << formattedMsg << "\n");
  longjmpError_(std::move(formattedMsg));
}

/// Helper function to load a pointer to the builtin closure with index
/// \p builtinIndex and place it in \p xRes.
void emit_load_builtin_closure(
    a64::Assembler &a,
    const a64::GpX &xRes,
    uint32_t builtinIndex) {
  static_assert(
      std::is_same_v<
          TransparentOwningPtr<Callable *, llvh::FreeDeleter>,
          RuntimeOffsets::BuiltinsType>,
      "builtins_ is a list of Callable *");
  static_assert(
      offsetof(TransparentOwningPtr<Callable *>, ptr) == 0,
      "TransparentOwningPtr must be transparent");

  a.ldr(xRes, a64::Mem(xRuntime, RuntimeOffsets::builtins));
  a.ldr(xRes, a64::Mem(xRes, builtinIndex * sizeof(Callable *)));
}

#ifndef ASMJIT_NO_LOGGING
asmjit::Error OurLogger::_log(const char *data, size_t size) noexcept {
  auto str =
      (size == SIZE_MAX ? llvh::StringRef(data) : llvh::StringRef(data, size));
  if (str.empty())
    return asmjit::kErrorOk;
  if (dumpJitCode_)
    llvh::outs() << str;
  if (!perfJitDump_)
    return asmjit::kErrorOk;

  // Comments by default do not have indentation, except some pseudocode
  // comments, which start with ';' after indentation.
  auto trimmed = str.ltrim();
  if (str.front() != ' ' || (!trimmed.empty() && trimmed.front() == ';')) {
    perfJitDump_->addCodeComment(str, a_.offset());
  }
  return asmjit::kErrorOk;
}
#endif

} // namespace hermes::vm::arm64

#endif // HERMESVM_JIT_ARM64
