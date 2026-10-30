/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter-internal.h"

#include "hermes/VM/JSObject-inline.h"

#define DEBUG_TYPE "jit"

namespace hermes::vm::x86_64 {

void emit_jsobject_init(
    x86::Assembler &a,
    const x86::Gp &obj,
    const x86::Gp &tempOrPropStorageOpt,
    bool hasPropStorage,
    const x86::Gp &clazzOpt) {
  // obj->flags = 0
  // x86-64: a 32-bit store of an immediate needs no register, so arm64's
  // wzr has no counterpart to give up here.
  a.mov(x86::dword_ptr(obj, offsetof(SHJSObject, flags)), asmjit::Imm(0));

  if (hasPropStorage) {
    emit_sh_cp_encode_non_null(a, tempOrPropStorageOpt);
    emit_store_cp(
        a,
        tempOrPropStorageOpt,
        x86::ptr(obj, offsetof(SHJSObject, propStorage)));
  }

  // No longer need the propStorage pointer. Alias for clarity.
  const x86::Gp &temp = tempOrPropStorageOpt;

  if (!clazzOpt.isValid()) {
    // Load the hidden class compressed pointer into clazz.
    a.mov(temp, x86::qword_ptr(xRuntime, offsetof(Runtime, classJSObject)));
    emit_sh_ljs_get_pointer(a, temp, temp);
    emit_sh_cp_encode_non_null(a, temp);
  }

  const x86::Gp &clazz = clazzOpt.isValid() ? clazzOpt : temp;

  // Store the hidden class, which also determines the parent.
  // obj->clazz = clazz (may be the same register as temp).
  assert(clazz.isValid());
  emit_store_cp(a, clazz, x86::ptr(obj, offsetof(SHJSObject, clazz)));

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

  // x86-64: `mov mem, imm` needs no source register, so unlike arm64 there is
  // nothing to gain from pairing the stores and the 16-byte step disappears.
  // If there is some amount that is not a multiple of 8, store that first.
  if (bytesToZero & 4) {
    a.mov(x86::dword_ptr(obj, (int32_t)startZeroOffset), asmjit::Imm(0));
    zeroedBytes += 4;
  }

  // The rest of the fill region is a multiple of 8 and, since the end of the
  // region is 8-byte aligned, so is every store below.
  for (; zeroedBytes < bytesToZero; zeroedBytes += 8) {
    a.mov(
        x86::qword_ptr(obj, (int32_t)(startZeroOffset + zeroedBytes)),
        asmjit::Imm(0));
  }
  assert(zeroedBytes == bytesToZero && "Did not zero the whole object");
}

/// Emit code to initialize the fields of a newly created environment.
/// \param newEnvPtr contains a pointer to the object to initialise.
/// \param parentEnv contains a compressed pointer to the parent environment.
/// \param temp is a temporary register for use by the emitted code.
/// \param vTemp is a temporary vector register for use by the emitted code.
/// \param size is the number of slots in the new environment.
///
/// No write barrier is emitted for the parent pointer, and none is needed:
/// the cell was just bump-allocated in the young generation, so it cannot be
/// an old-gen object pointing at a young one, and it is not yet reachable
/// from anything the GC scans. The same reasoning covers the slots, which are
/// filled with undefined. This mirrors arm64 exactly.
void emit_environment_init(
    x86::Assembler &a,
    const x86::Gp &newEnvPtr,
    const x86::Gp &parentEnv,
    const x86::Gp &temp,
    const x86::Xmm &vTemp,
    uint32_t size) {
  // Store the size and parent to the first two fields.
  // x86-64: arm64 pairs the two narrow stores with stp, which forces the size
  // into a register first. Here a store of an immediate is a single
  // instruction in both compressed-pointer configurations, so the two cases
  // collapse into one and differ only in the width of the parent store, which
  // emit_store_cp() picks.
  emit_store_cp(
      a,
      parentEnv,
      x86::ptr(newEnvPtr, offsetof(SHEnvironment, parentEnvironment)));
  a.mov(
      x86::dword_ptr(newEnvPtr, offsetof(SHEnvironment, size)),
      asmjit::Imm(size));

  // Load undefined.
  a.mov(temp, asmjit::Imm(_sh_ljs_undefined().raw));

  // x86-64: every slot is reachable through the store's signed 32-bit
  // displacement, so arm64's post-indexed walk over a slot pointer (and the
  // register it consumes) is replaced by folded displacements. The vector
  // stores are unaligned: a cell is only guaranteed 8-byte aligned, so the
  // 16-byte stores below must not require more.
  auto slotsToFill = size;
  /// \return the displacement of the first unfilled slot, plus \p extra.
  const auto slotOfs = [size](uint32_t remaining, uint32_t extra) {
    size_t ofs = offsetof(SHEnvironment, slots) +
        sizeof(SHLegacyValue) * (size_t)(size - remaining) + extra;
    assert(ofs <= (size_t)INT32_MAX && "slot offset must fit a disp32");
    return (int32_t)ofs;
  };

  // Before we fill 4 at a time, make sure any excess slots are filled.
  if (slotsToFill % 2) {
    a.mov(x86::qword_ptr(newEnvPtr, offsetof(SHEnvironment, slots)), temp);
    --slotsToFill;
  }
  if (slotsToFill % 4) {
    // x86-64: arm64's stp of the same register twice is a pair of stores.
    a.mov(x86::qword_ptr(newEnvPtr, slotOfs(slotsToFill, 0)), temp);
    a.mov(x86::qword_ptr(newEnvPtr, slotOfs(slotsToFill, 8)), temp);
    slotsToFill -= 2;
  }

  // The remaining number of slots must be a multiple of 4, which we can fill 4
  // at a time.
  assert((slotsToFill % 4) == 0 && "Remaining slots must be multiple of 4");
  if (slotsToFill) {
    // Copy the value into a vector register, so we can fill faster. vmovq
    // zeroes the upper half, which vpunpcklqdq then overwrites with a second
    // copy of the low half -- arm64 does both in one `dup`.
    a.vmovq(vTemp, temp);
    a.vpunpcklqdq(vTemp, vTemp, vTemp);

    // Fill the slots with undefined 4 at a time. Each 16-byte store covers
    // two slots, where arm64's stp of two vector registers covers four.
    for (; slotsToFill; slotsToFill -= 4) {
      a.vmovups(x86::xmmword_ptr(newEnvPtr, slotOfs(slotsToFill, 0)), vTemp);
      a.vmovups(x86::xmmword_ptr(newEnvPtr, slotOfs(slotsToFill, 16)), vTemp);
    }
  }
  assert(slotsToFill == 0 && "All slots must be filled");
}

/// Helper function to load a pointer to the builtin closure with index
/// \p builtinIndex and place it in \p res.
void emit_load_builtin_closure(
    x86::Assembler &a,
    const x86::Gp &res,
    uint32_t builtinIndex) {
  static_assert(
      std::is_same_v<
          TransparentOwningPtr<Callable *, llvh::FreeDeleter>,
          RuntimeOffsets::BuiltinsType>,
      "builtins_ is a list of Callable *");
  static_assert(
      offsetof(TransparentOwningPtr<Callable *>, ptr) == 0,
      "TransparentOwningPtr must be transparent");

  a.mov(res, x86::qword_ptr(xRuntime, RuntimeOffsets::builtins));
  a.mov(res, x86::qword_ptr(res, (int32_t)(builtinIndex * sizeof(Callable *))));
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

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
