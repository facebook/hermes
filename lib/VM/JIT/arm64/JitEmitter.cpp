/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitEmitter.h"

#include "JitHandlers.h"

#include "../RuntimeOffsets.h"
#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/StaticHUtils.h"

#define DEBUG_TYPE "jit"

#if defined(HERMESVM_COMPRESSED_POINTERS) || defined(HERMESVM_BOXED_DOUBLES)
#error JIT does not support compressed pointers or boxed doubles yet
#endif

namespace hermes::vm::arm64 {
namespace {

// Ensure that HermesValue tags are handled correctly by updating this every
// time the HERMESVALUE_VERSION changes, and going through the JIT and updating
// any relevant code.
static_assert(
    HERMESVALUE_VERSION == 1,
    "HermesValue version mismatch, JIT may need to be updated");

void emit_sh_ljs_get_pointer(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  // See:
  // https://dinfuehr.github.io/blog/encoding-of-immediate-values-on-aarch64/
  static_assert(
      HERMESVALUE_VERSION == 1,
      "kHV_DataMask is 0x000...1111... and can be encoded as a logical immediate");
  a.and_(xOut, xIn, kHV_DataMask);
}

/// Encode a native pointer into a tagged object pointer (SHLegacyValue).
/// The same register is used for input and output.
void emit_sh_ljs_object(a64::Assembler &a, const a64::GpX &inOut) {
  static_assert(
      HERMESVALUE_VERSION == 1,
      "HVTag_Object << kHV_NumDataBits is 0x1111...0000... and can be encoded as a logical immediate");
  a.movk(inOut, (uint16_t)HVTag_Object, kHV_NumDataBits);
}

/// Encode a native pointer into a tagged object pointer (SHLegacyValue).
/// Takes an input and output register, which can be the same.
/// In some sense this supersedes
void emit_sh_ljs_object2(
    a64::Assembler &a,
    const a64::GpX &xOut,
    const a64::GpX &xIn) {
  static_assert(
      HERMESVALUE_VERSION == 1,
      "HVTag_Object << kHV_NumDataBits is 0x1111...0000... and can be encoded as a logical immediate");
  a.orr(xOut, xIn, (uint64_t)HVTag_Object << kHV_NumDataBits);
}

/// Emit code to check whether the input reg is an object, using the specified
/// temp register. The input reg is not
/// modified unless it is the same as the temp, which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_object(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Check if frConstructed is an object.
  // Get the tag bits in xTmpConstructedTag by right shifting.
  static_assert(
      (int16_t)HVTag_Object == (int16_t)(-1) && "HV_TagObject must be -1");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits);
  a.cmn(xTempReg, -HVTag_Object);
}

/// Emit code to check whether the input reg is a string, using the specified
/// temp register. The input reg is not
/// modified unless it is the same as the temp, which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_string(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert((int16_t)HVTag_Str == (int16_t)(-3) && "HV_TagStr must be -3");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits);
  a.cmn(xTempReg, -HVTag_Str);
}

/// Emit code to check whether the input reg is empty, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_empty(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      (int16_t)HVETag_Empty == (int16_t)(-14) && "HVETag_Empty must be -14");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Empty);
}

/// Emit code to check whether the input reg is undefined, using the specified
/// temp register.
/// The input reg is not modified unless it is the same as the temp,
/// which is allowed.
/// CPU flags are updated as result. b.eq on success.
void emit_sh_ljs_is_undefined(
    a64::Assembler &a,
    const a64::GpX &xTempReg,
    const a64::GpX &xInputReg) {
  // Get the tag bits by right shifting.
  static_assert(
      HERMESVALUE_VERSION == 1,
      "HVETag_Undefined must be at kHV_NumDataBits - 1");
  static_assert(
      (int16_t)HVETag_Undefined == (int16_t)(-12) &&
      "HVETag_Undefined must be -12");
  a.asr(xTempReg, xInputReg, kHV_NumDataBits - 1);
  a.cmn(xTempReg, -HVETag_Undefined);
}

/// For a register \p inOut that contains a bool (i.e. either 0 or 1), turn it
/// into a HermesValue boolean by adding the corresponding tag.
void emit_sh_ljs_bool(a64::Assembler &a, const a64::GpX inOut) {
  static constexpr SHLegacyValue baseBool = HermesValue::encodeBoolValue(false);
  // We know that the ETag for bool as a 0 in its lowest bit, and is therefore a
  // shifted 16 bit value. We can exploit this to use movk to set the tag.
  static_assert(HERMESVALUE_VERSION == 1);
  static_assert(
      (llvh::isShiftedUInt<16, kHV_NumDataBits>(baseBool.raw)) &&
      "Boolean tag must be 16 bits.");
  // Add the bool tag.
  a.movk(inOut, baseBool.raw >> kHV_NumDataBits, kHV_NumDataBits);
}

/// For a register \p dInput, which contains a double, check whether it is a
/// valid signed 64-bit integer.
/// CPU flags are updated. b_eq on success.
/// If successful, \p xTemp will contain the number converted to int,
/// and \p dTemp will contain the same number as \p dInput.
/// \pre dTemp != dInput, because both are used in the comparison.
void emit_double_is_int(
    a64::Assembler &a,
    const a64::GpX &xTemp,
    const a64::VecD &dTemp,
    const a64::VecD &dInput) {
  assert(dTemp != dInput && "must use a different temp");

  // Convert the operand to a signed 64 bit integer.
  a.fcvtzs(xTemp, dInput);
  // Sign extend it from the second-to-last bit. This is necessary because
  // fcvtzs is saturating and will convert the double 2^63 to 2^63 - 1, which
  // will get converted back to 2^63 by scvtf. They will therefore incorrectly
  // compare equal after truncation.
  a.sbfx(xTemp, xTemp, 0, 63);
  // Convert back to a double and see if they compare equal.
  a.scvtf(dTemp, xTemp);
  a.fcmp(dTemp, dInput);
}

/// For a register \p dInput, which contains a double, check whether it is a
/// valid unsigned 32-bit integer.
/// CPU flags are updated. b_eq on success.
/// If successful, \p wTemp will contain the number converted to int,
/// and \p dTemp will contain the same number as \p dInput.
/// \pre dTemp != dInput, because both are used in the comparison.
void emit_double_is_uint32(
    a64::Assembler &a,
    const a64::GpW &wTemp,
    const a64::VecD &dTemp,
    const a64::VecD &dInput) {
  assert(dTemp != dInput && "must use a different temp");
  a.fcvtzu(wTemp, dInput);
  a.ucvtf(dTemp, wTemp);
  a.fcmp(dTemp, dInput);
}

class OurErrorHandler : public asmjit::ErrorHandler {
  asmjit::Error &expectedError_;
  std::function<void(std::string &&message)> const longjmpError_;

 public:
  /// \param expectedError if we get an error matching this value, we ignore it.
  explicit OurErrorHandler(
      asmjit::Error &expectedError,
      const std::function<void(std::string &&message)> &longjmpError)
      : expectedError_(expectedError), longjmpError_(longjmpError) {}

  void handleError(
      asmjit::Error err,
      const char *message,
      asmjit::BaseEmitter *origin) override {
    if (err == expectedError_) {
      LLVM_DEBUG(
          llvh::outs() << "Expected AsmJit error: " << err << ": "
                       << asmjit::DebugUtils::errorAsString(err) << ": "
                       << message << "\n");
      return;
    }

    std::string formattedMsg{};
    llvh::raw_string_ostream OS{formattedMsg};
    OS << "AsmJit error: " << err << ": "
       << asmjit::DebugUtils::errorAsString(err) << ": " << message;
    OS.flush();

    LLVM_DEBUG(llvh::dbgs() << formattedMsg << "\n");
    longjmpError_(std::move(formattedMsg));
  }
};

/// This macro is used to catch and handle low probability instructing encoding
/// errors - i.e. when an immediate operand doesn't fit in the instruction
/// encoding. It causes Asmjit to just return an error code instead of
/// terminating the entire compilation.
///
/// \param expValue the error value that we want to handle.
/// \param code  C++ code to invoke asmjit and store the result in a variable.
#define EXPECT_ERROR(expValue, code)          \
  do {                                        \
    assert(                                   \
        expectedError_ == asmjit::kErrorOk && \
        "expectedError_ is not cleared");     \
    expectedError_ = (expValue);              \
    code;                                     \
    expectedError_ = asmjit::kErrorOk;        \
  } while (0)

class OurLogger : public asmjit::Logger {
  ASMJIT_API asmjit::Error _log(const char *data, size_t size) noexcept
      override {
    llvh::outs()
        << (size == SIZE_MAX ? llvh::StringRef(data)
                             : llvh::StringRef(data, size));
    return asmjit::kErrorOk;
  }
};

} // unnamed namespace

/// Return true if the specified 64-bit value can be efficiently loaded on
/// Arm64 with up to two integer instructions. In other words, it has at most
/// two non-zero 16-bit words.
static bool isCheapConst(uint64_t k) {
  unsigned count = 0;
  for (uint64_t mask = 0xFFFF; mask != 0; mask <<= 16) {
    if (k & mask)
      ++count;
  }
  return count <= 2;
}

/// Save the current IP and emit a call to a runtime function. This should be
/// used in most cases when invoking slow paths and handlers for complex
/// functionality.
#define EMIT_RUNTIME_CALL(em, type, func)           \
  do {                                              \
    using _FnT = type;                              \
    _FnT _fn = func;                                \
    (void)_fn;                                      \
    (em).callThunkWithSavedIP((void *)func, #func); \
  } while (0)

/// Call a runtime function without saving the IP. This is intended for special
/// cases where we want to preserve the currently saved IP or if the IP is not
/// needed.
#define EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(em, type, func) \
  do {                                                     \
    using _FnT = type;                                     \
    _FnT _fn = func;                                       \
    (void)_fn;                                             \
    (em).callThunk((void *)func, #func);                   \
  } while (0)

/// Make call without creating a thunk for it or saving the IP. This is useful
/// for specific functionality where saving the IP is either unnecessary or
/// incorrect, and where the call target is only going to be invoked at most
/// once in the function.
#define EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(em, type, func) \
  do {                                                               \
    using _FnT = type;                                               \
    _FnT _fn = func;                                                 \
    (void)_fn;                                                       \
    (em).callWithoutThunk((void *)func, #func);                      \
  } while (0)

Emitter::Emitter(
    asmjit::JitRuntime &jitRT,
    unsigned dumpJitCode,
    CodeBlock *codeBlock,
    PropertyCacheEntry *readPropertyCache,
    PropertyCacheEntry *writePropertyCache,
    uint32_t numFrameRegs,
    const std::function<void(std::string &&message)> &longjmpError)
    : dumpJitCode_(dumpJitCode),
      frameRegs_(numFrameRegs),
      codeBlock_(codeBlock) {
  if (dumpJitCode_ & DumpJitCode::Code)
    logger_ = std::unique_ptr<asmjit::Logger>(new OurLogger());
  if (logger_)
    logger_->setIndentation(asmjit::FormatIndentationGroup::kCode, 4);

  errorHandler_ = std::unique_ptr<asmjit::ErrorHandler>(
      new OurErrorHandler(expectedError_, longjmpError));

  code.init(jitRT.environment(), jitRT.cpuFeatures());
  code.setErrorHandler(errorHandler_.get());
  if (logger_)
    code.setLogger(logger_.get());
  code.attach(&a);

  roDataLabel_ = a.newNamedLabel("RO_DATA");
  returnLabel_ = a.newNamedLabel("leave");

  // Save read/write property cache addresses.
  roOfsReadPropertyCachePtr_ =
      uint64Const((uint64_t)readPropertyCache, "readPropertyCache");
  roOfsWritePropertyCachePtr_ =
      uint64Const((uint64_t)writePropertyCache, "writePropertyCache");
}

void Emitter::enter(uint32_t numCount, uint32_t npCount) {
  unsigned nextVec = kVecSaved.first;
  unsigned nextGp = kGPSaved.first;

  // Number registers: allocate in vector hw regs first.
  for (unsigned frIndex = 0; frIndex < numCount; ++frIndex) {
    HWReg hwReg;
    if (nextVec <= kVecSaved.second) {
      hwReg = HWReg::vecD(nextVec);
      comment("    ; alloc: d%u <= r%u", nextVec, frIndex);
      ++nextVec;
    } else if (nextGp <= kGPSaved.second) {
      hwReg = HWReg::gpX(nextGp);
      comment("    ; alloc: x%u <= r%u", nextGp, frIndex);
      ++nextGp;
    } else
      break;

    frameRegs_[frIndex].globalReg = hwReg;
    frameRegs_[frIndex].globalType = FRType::Number;
  }
  // Non-pointer regs: allocate in gp regs first.
  for (unsigned frIndex = 0; frIndex < npCount; ++frIndex) {
    HWReg hwReg;
    if (nextGp <= kGPSaved.second) {
      hwReg = HWReg::gpX(nextGp);
      comment("    ; alloc: x%u <= r%u", nextGp, frIndex);
      ++nextGp;
    } else if (nextVec <= kVecSaved.second) {
      hwReg = HWReg::vecD(nextVec);
      comment("    ; alloc: d%u <= r%u", nextVec, frIndex);
      ++nextVec;
    } else
      break;

    frameRegs_[frIndex].globalReg = hwReg;
    frameRegs_[frIndex].globalType = FRType::UnknownNonPtr;
  }

  frameSetup(
      frameRegs_.size(), nextGp - kGPSaved.first, nextVec - kVecSaved.first);
}

void Emitter::comment(const char *fmt, ...) {
  if (!logger_)
    return;
  va_list args;
  va_start(args, fmt);
  char buf[80];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  a.comment(buf);
}

JITCompiledFunctionPtr Emitter::addToRuntime(asmjit::JitRuntime &jr) {
  emitSlowPaths();
  emitThunks();
  emitROData();

  code.detach(&a);
  JITCompiledFunctionPtr fn;
  asmjit::Error err = jr.add(&fn, &code);
  if (err) {
    llvh::errs() << "AsmJit failed: " << asmjit::DebugUtils::errorAsString(err)
                 << "\n";
    hermes::hermes_fatal("AsmJit failed");
  }
  return fn;
}

#ifndef NDEBUG
void Emitter::assertPostInstructionInvariants() {
  for (const auto &frState : frameRegs_)
    assert(!frState.regIsDirty && "Frame register is dirty");

  // Check that any temps have an associated FR.
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(gpTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }

  for (unsigned i = kVecTemp1.first; i <= kVecTemp2.second; ++i) {
    if (i > kVecTemp1.second && i < kVecTemp2.first)
      continue;
    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(vecTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }
}
#endif

void Emitter::newBasicBlock(const asmjit::Label &label) {
  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  // Clear all local types and regs when starting a new basic block.
  // TODO: there must be a faster way to do this when there are many regs.
  for (FRState &frState : frameRegs_) {
    frState.localType = frState.globalType;
    assert(!frState.localGpX);
    assert(!frState.localVecD);
    if (frState.globalReg)
      frState.frameUpToDate = false;
  }

  a.bind(label);
}

int32_t Emitter::getDebugFunctionName() {
  if (roOfsDebugFunctionName_ < 0) {
    std::string str;
    llvh::raw_string_ostream ss(str);
    ss << codeBlock_->getFunctionID() << "(" << codeBlock_->getNameString()
       << ")";
    ss.flush();
    int32_t size = str.size() + 1;
    roOfsDebugFunctionName_ = reserveData(size, 1, asmjit::TypeId::kInt8, size);
    memcpy(roData_.data() + roOfsDebugFunctionName_, str.data(), size);
  }
  return roOfsDebugFunctionName_;
}

void Emitter::frameSetup(
    unsigned numFrameRegs,
    unsigned gpSaveCount,
    unsigned vecSaveCount) {
  assert(
      gpSaveCount <= kGPSaved.second - kGPSaved.first + 1 &&
      "Too many callee saved GP regs");
  assert(
      vecSaveCount <= kVecSaved.second - kVecSaved.first + 1 &&
      "Too many callee saved Vec regs");

  auto prohibitInvoke = codeBlock_->getHeaderFlags().prohibitInvoke;
  if (prohibitInvoke != ProhibitInvoke::None) {
    // Load the new frame pointer. We leave x0 untouched since it contains the
    // runtime parameter, which is used below and in throwInvalidInvoke_.
    a.ldr(a64::x1, a64::Mem(a64::x0, offsetof(Runtime, stackPointer_)));
    // Load new.target.
    a.ldur(
        a64::x1,
        a64::Mem(
            a64::x1, StackFrameLayout::NewTarget * (int)sizeof(SHLegacyValue)));
    // Compare new.target against undefined.
    emit_sh_ljs_is_undefined(a, a64::x1, a64::x1);

    if (prohibitInvoke == ProhibitInvoke::Call) {
      // If regular calls are prohibited, then we jump to throwInvalidInvoke if
      // new.target is undefined.
      throwInvalidInvoke_ = a.newNamedLabel("throwInvalidCall");
      a.b_eq(throwInvalidInvoke_);
    } else if (prohibitInvoke == ProhibitInvoke::Construct) {
      // If construct calls are prohibited, then we jump to throwInvalidInvoke
      // if new.target is not undefined.
      throwInvalidInvoke_ = a.newNamedLabel("throwInvalidConstruct");
      a.b_ne(throwInvalidInvoke_);
    }
  }

  static_assert(
      kGPSaved.first == 22, "Callee saved GP regs must start from x22");
  // Always save x22.
  if (gpSaveCount == 0)
    gpSaveCount = 1;
  // We always save x19, x20, x21.
  gpSaveCount += 3;

  gpSaveCount_ = gpSaveCount;
  vecSaveCount_ = vecSaveCount;

  //  0-3: SHLocals
  //  4: x22
  //  5: x21
  //  6: x20
  //  7: x19
  //  8: x29 <- new x29 points here
  //  9: x30
  a.sub(
      a64::sp,
      a64::sp,
      (4 + ((gpSaveCount + 1) & ~1) + ((vecSaveCount + 1) & ~1) + 2) * 8);

  unsigned stackOfs = 4 * 8;
  for (unsigned i = 0; i < gpSaveCount; i += 2, stackOfs += 16) {
    if (i + 1 < gpSaveCount)
      a.stp(a64::GpX(19 + i), a64::GpX(20 + i), a64::Mem(a64::sp, stackOfs));
    else
      a.str(a64::GpX(19 + i), a64::Mem(a64::sp, stackOfs));
  }
  for (unsigned i = 0; i < vecSaveCount; i += 2, stackOfs += 16) {
    if (i + 1 < vecSaveCount)
      a.stp(
          a64::VecD(kVecSaved.first + i),
          a64::VecD(kVecSaved.first + 1 + i),
          a64::Mem(a64::sp, stackOfs));
    else
      a.str(a64::VecD(kVecSaved.first + i), a64::Mem(a64::sp, stackOfs));
  }
  a.stp(a64::x29, a64::x30, a64::Mem(a64::sp, stackOfs));
  a.add(a64::x29, a64::sp, stackOfs);

  // ((uint64_t)HVTag_First << kHV_NumDataBits)
  static_assert(
      HERMESVALUE_VERSION == 1,
      "HVTag_First must be the first after double limit");
  comment("// xDoubleLim");
  a.mov(xDoubleLim, ((uint64_t)HVTag_First << kHV_NumDataBits));

  comment("// xRuntime");
  a.mov(xRuntime, a64::x0);

  //  _sh_check_native_stack_overflow(shr);
  // Do not save the IP because we have not yet set up the stack frame for this
  // function. If this throws, the exception should appear in the caller.
  EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
      *this, void (*)(SHRuntime *), _sh_check_native_stack_overflow);

  // Function<bench>(3 params, 13 registers):
  //  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 13);
  comment("// _sh_enter");
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  // _sh_enter expects the number of registers to include any extra registers at
  // the start of the frame.
  a.mov(a64::w2, numFrameRegs + hbc::StackFrameLayout::FirstLocal);
  // Like _sh_check_native_stack_overflow, we do not save the IP here so that
  // thrown exceptions appear in the caller.
  EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
      *this, SHLegacyValue * (*)(SHRuntime *, SHLocals *, uint32_t), _sh_enter);
  comment("// xFrame");
  a.mov(xFrame, a64::x0);

  //  locals.head.count = 0;
  comment("// locals.head.count = 0");
  a.mov(a64::w1, 0);
  a.str(a64::w1, a64::Mem(a64::sp, offsetof(SHLocals, count)));

  if (dumpJitCode_ & DumpJitCode::EntryExit) {
    comment("// print entry");
    a.mov(a64::w0, 1);
    a.adr(a64::x1, roDataLabel_);
    a.add(a64::x1, a64::x1, getDebugFunctionName());
    EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
        *this, void (*)(bool, const char *), _sh_print_function_entry_exit);
  }
}

void Emitter::leave() {
  comment("// leaveFrame");
  a.bind(returnLabel_);
  if (dumpJitCode_ & DumpJitCode::EntryExit) {
    comment("// print exit");
    a.mov(a64::w0, 0);
    a.adr(a64::x1, roDataLabel_);
    a.add(a64::x1, a64::x1, getDebugFunctionName());
    EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
        *this, void (*)(bool, const char *), _sh_print_function_entry_exit);
  }
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  a.mov(a64::x2, xFrame);
  // _sh_leave cannot throw or observe the IP.
  EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
      *this, void (*)(SHRuntime *, SHLocals *, SHLegacyValue *), _sh_leave);

  // The return value has been stashed in x22 by ret(). Move it to the return
  // register.
  a.mov(a64::x0, a64::x22);

  unsigned stackOfs = 4 * 8;
  for (unsigned i = 0; i < gpSaveCount_; i += 2, stackOfs += 16) {
    if (i + 1 < gpSaveCount_)
      a.ldp(a64::GpX(19 + i), a64::GpX(20 + i), a64::Mem(a64::sp, stackOfs));
    else
      a.ldr(a64::GpX(19 + i), a64::Mem(a64::sp, stackOfs));
  }
  for (unsigned i = 0; i < vecSaveCount_; i += 2, stackOfs += 16) {
    if (i + 1 < vecSaveCount_)
      a.ldp(
          a64::VecD(kVecSaved.first + i),
          a64::VecD(kVecSaved.first + 1 + i),
          a64::Mem(a64::sp, stackOfs));
    else
      a.ldr(a64::VecD(kVecSaved.first + i), a64::Mem(a64::sp, stackOfs));
  }
  a.ldp(a64::x29, a64::x30, a64::Mem(a64::sp, stackOfs));

  a.add(
      a64::sp,
      a64::sp,
      (4 + ((gpSaveCount_ + 1) & ~1) + ((vecSaveCount_ + 1) & ~1) + 2) * 8);

  a.ret(a64::x30);
}

void Emitter::callThunk(void *fn, const char *name) {
  //    comment("// call %s", name);
  a.bl(registerThunk(fn, name));
}

void Emitter::callThunkWithSavedIP(void *fn, const char *name) {
  // Save the current IP in the runtime.
  auto ofs = codeBlock_->getOffsetOf(emittingIP);
  loadBits64InGp(a64::x16, (uint64_t)codeBlock_->begin(), "Bytecode start");
  if (a64::Utils::isAddSubImm(ofs)) {
    a.add(a64::x16, a64::x16, ofs);
  } else {
    a.mov(a64::x17, ofs);
    a.add(a64::x16, a64::x16, a64::x17);
  }
  a.str(a64::x16, a64::Mem(xRuntime, offsetof(Runtime, currentIP_)));

  // Call the passed function.
  callThunk(fn, name);

// Invalidate the current IP to make sure it is set before the next call.
#ifndef NDEBUG
  a.mov(a64::x16, Runtime::kInvalidCurrentIP);
  a.str(a64::x16, a64::Mem(xRuntime, offsetof(Runtime, currentIP_)));
#endif
}

void Emitter::callWithoutThunk(void *fn, const char *name) {
  comment("// call %s", name);
  loadBits64InGp(a64::x16, (uint64_t)fn, name);
  a.blr(a64::x16);
}

void Emitter::loadFrameAddr(a64::GpX dst, FR frameReg) {
  auto ofs =
      (frameReg.index() + StackFrameLayout::FirstLocal) * sizeof(SHLegacyValue);
  // If the offset fits as an immediate, just emit an add.
  if (a64::Utils::isAddSubImm(ofs)) {
    a.add(dst, xFrame, ofs);
    return;
  }
  // We cannot add the offset as an immediate, so move it in first.
  a.mov(dst, ofs);
  a.add(dst, dst, xFrame);
}

template <bool use>
void Emitter::movHWFromHW(HWReg dst, HWReg src) {
  if (dst != src) {
    if (dst.isVecD() && src.isVecD())
      a.fmov(dst.a64VecD(), src.a64VecD());
    else if (dst.isVecD())
      a.fmov(dst.a64VecD(), src.a64GpX());
    else if (src.isVecD())
      a.fmov(dst.a64GpX(), src.a64VecD());
    else
      a.mov(dst.a64GpX(), src.a64GpX());
  }
  if constexpr (use) {
    useReg(src);
    useReg(dst);
  }
}

void Emitter::_storeHWToFrame(FR fr, HWReg src) {
  _storeFrame(src, fr);
  frameRegs_[fr.index()].frameUpToDate = true;
}

void Emitter::movHWFromFR(HWReg hwRes, FR src) {
  FRState &frState = frameRegs_[src.index()];
  assert(!frState.regIsDirty && "Any local should have a valid value");
  if (frState.localGpX)
    movHWFromHW<true>(hwRes, frState.localGpX);
  else if (frState.localVecD)
    movHWFromHW<true>(hwRes, frState.localVecD);
  else if (frState.globalReg && frState.globalRegUpToDate)
    movHWFromHW<true>(hwRes, frState.globalReg);
  else
    _loadFrame(useReg(hwRes), src);
}

void Emitter::movHWFromMem(HWReg hwRes, a64::Mem src) {
  if (hwRes.isVecD())
    a.ldr(hwRes.a64VecD(), src);
  else
    a.ldr(hwRes.a64GpX(), src);
}

void Emitter::movFRFromHW(FR dst, HWReg src, FRType type) {
  FRState &frState = frameRegs_[dst.index()];
  // If it is a local or global register, move the value into it and mark it as
  // updated.
  if (frState.localGpX) {
    movHWFromHW<false>(frState.localGpX, src);
    frUpdatedWithHW(dst, frState.localGpX, type);
  } else if (frState.localVecD) {
    movHWFromHW<false>(frState.localVecD, src);
    frUpdatedWithHW(dst, frState.localVecD, type);
  } else if (frState.globalReg) {
    movHWFromHW<false>(frState.globalReg, src);
    frUpdatedWithHW(dst, frState.globalReg, type);
  } else {
    // Otherwise store it directly to the frame.
    _storeHWToFrame(dst, src);
    frUpdateType(dst, type);
    frState.frameUpToDate = true;
  }
}

void Emitter::syncFrameOutParam(FR fr, FRType type) {
  auto &frState = frameRegs_[fr.index()];

  frState.frameUpToDate = true;

  // Since the frame is the source-of-truth here, there should not be any local
  // register.
  assert(!frState.localGpX && !frState.localVecD);

  if (frState.globalReg) {
    frState.globalRegUpToDate = true;
    _loadFrame(frState.globalReg, fr);
  }
  frUpdateType(fr, type);
}

template <class TAG>
HWReg Emitter::_allocTemp(TempRegAlloc &ra, llvh::Optional<HWReg> preferred) {
  llvh::Optional<unsigned> pr{};
  if (preferred)
    pr = preferred->indexInClass();
  if (auto optReg = ra.alloc(pr); optReg)
    return HWReg(*optReg, TAG{});
  // Spill one register.
  unsigned index = pr ? *pr : ra.leastRecentlyUsed();
  _spillTempForFR(HWReg(index, TAG{}));
  ra.free(index);
  // Allocate again. This must succeed.
  return HWReg(*ra.alloc(), TAG{});
}

void Emitter::freeReg(HWReg hwReg) {
  if (!hwReg.isValid())
    return;

  FR fr = hwRegs_[hwReg.combinedIndex()].contains;
  hwRegs_[hwReg.combinedIndex()].contains = {};

  if (hwReg.isGpX()) {
    if (fr.isValid()) {
      comment("    ; free x%u (r%u)", hwReg.indexInClass(), fr.index());
      assert(frameRegs_[fr.index()].localGpX == hwReg);
      frameRegs_[fr.index()].localGpX = {};
    } else {
      comment("    ; free x%u", hwReg.indexInClass());
    }
    if (isTempGpX(hwReg))
      gpTemp_.free(hwReg.indexInClass());
  } else {
    if (fr.isValid()) {
      comment("    ; free d%u (r%u)", hwReg.indexInClass(), fr.index());
      assert(frameRegs_[fr.index()].localVecD == hwReg);
      frameRegs_[fr.index()].localVecD = {};
    } else {
      comment("    ; free d%u", hwReg.indexInClass());
    }
    if (isTempVecD(hwReg))
      vecTemp_.free(hwReg.indexInClass());
  }
}
void Emitter::syncAndFreeTempReg(HWReg hwReg) {
  if (!hwReg.isValid() || !isTemp(hwReg) ||
      !hwRegs_[hwReg.combinedIndex()].contains.isValid()) {
    return;
  }
  _spillTempForFR(hwReg);
  freeReg(hwReg);
}

// TODO: check wherger we should make this call require a temp reg.
HWReg Emitter::useReg(HWReg hwReg) {
  if (!hwReg.isValid())
    return hwReg;
  // Check whether it is a temporary.
  if (hwReg.isGpX()) {
    if (isTempGpX(hwReg))
      gpTemp_.use(hwReg.indexInClass());
  } else {
    if (isTempVecD(hwReg))
      vecTemp_.use(hwReg.indexInClass());
  }
  return hwReg;
}

void Emitter::_spillTempForFR(HWReg toSpill) {
  assert(isTemp(toSpill));

  HWRegState &hwState = hwRegs_[toSpill.combinedIndex()];
  FR fr = hwState.contains;
  hwState.contains = {};
  assert(fr.isValid() && "Allocated tmp register is unused");

  FRState &frState = frameRegs_[fr.index()];

  assert(frState.globalReg != toSpill && "global regs can't be temporary");
  if (frState.globalReg) {
    if (!frState.globalRegUpToDate) {
      movHWFromHW<false>(frState.globalReg, toSpill);
      frState.globalRegUpToDate = true;
    }
  } else {
    if (!frState.frameUpToDate) {
      _storeHWToFrame(fr, toSpill);
      frState.frameUpToDate = true;
    }
  }

  if (frState.localGpX == toSpill)
    frState.localGpX = {};
  else if (frState.localVecD == toSpill)
    frState.localVecD = {};
  else
    assert(false && "local reg not used by FR");
}

void Emitter::syncToFrame(FR fr) {
  FRState &frState = frameRegs_[fr.index()];
  if (frState.frameUpToDate)
    return;

  HWReg hwReg = _isFRInRegister(fr);
  assert(
      hwReg.isValid() && "FR is not synced to frame and is not in a register");

  // We have an invariant that the global reg cannot have an old value if the
  // frame has a new one.
  if (frState.globalReg && !frState.globalRegUpToDate) {
    assert(hwReg != frState.globalReg && "FR is in a global reg");
    movHWFromHW<false>(frState.globalReg, hwReg);
    frState.globalRegUpToDate = true;
  }
  _storeHWToFrame(fr, hwReg);
}

void Emitter::syncAllFRTempExcept(FR exceptFR) {
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;

    FRState &frState = frameRegs_[fr.index()];
    assert(frState.localGpX == hwReg && "tmpreg not bound to FR localreg");
    if (frState.globalReg) {
      if (!frState.globalRegUpToDate) {
        comment("    ; sync: x%u (r%u)", i, fr.index());
        movHWFromHW<false>(frState.globalReg, hwReg);
        frState.globalRegUpToDate = true;
      }
    } else {
      if (!frState.frameUpToDate) {
        comment("    ; sync: x%u (r%u)", i, fr.index());
        _storeHWToFrame(fr, hwReg);
      }
    }
  }

  for (unsigned i = kVecTemp1.first; i <= kVecTemp2.second; ++i) {
    if (i > kVecTemp1.second && i < kVecTemp2.first)
      continue;

    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;

    FRState &frState = frameRegs_[fr.index()];
    assert(frState.localVecD == hwReg && "tmpreg not bound to FR localreg");
    // If there is a local GpX, it already synced the value.
    if (frState.localGpX)
      continue;
    if (frState.globalReg) {
      if (!frState.globalRegUpToDate) {
        comment("    ; sync d%u (r%u)", i, fr.index());
        movHWFromHW<false>(frState.globalReg, hwReg);
        frState.globalRegUpToDate = true;
      }
    } else {
      if (!frState.frameUpToDate) {
        comment("    ; sync d%u (r%u)", i, fr.index());
        _storeHWToFrame(fr, hwReg);
      }
    }
  }
}

void Emitter::freeAllFRTempExcept(FR exceptFR) {
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    freeFRTemp(fr);
  }

  for (unsigned i = kVecTemp1.first; i <= kVecTemp2.second; ++i) {
    if (i > kVecTemp1.second && i < kVecTemp2.first)
      continue;

    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    freeFRTemp(fr);
  }
}

void Emitter::freeFRTemp(FR fr) {
  auto &frState = frameRegs_[fr.index()];
  if (frState.localGpX) {
    assert(isTempGpX(frState.localGpX));
    comment(
        "    ; free x%u (r%u)", frState.localGpX.indexInClass(), fr.index());
    hwRegs_[frState.localGpX.combinedIndex()].contains = {};
    gpTemp_.free(frState.localGpX.indexInClass());
    frState.localGpX = {};
  }
  if (frState.localVecD) {
    assert(isTempVecD(frState.localVecD));
    comment(
        "    ; free d%u (r%u)", frState.localVecD.indexInClass(), fr.index());
    hwRegs_[frState.localVecD.combinedIndex()].contains = {};
    vecTemp_.free(frState.localVecD.indexInClass());
    frState.localVecD = {};
  }
}

void Emitter::_assignAllocatedLocalHWReg(FR fr, HWReg hwReg) {
  hwRegs_[hwReg.combinedIndex()].contains = fr;
  if (hwReg.isGpX()) {
    comment("    ; alloc: x%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localGpX = hwReg;
  } else {
    comment("    ; alloc: d%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localVecD = hwReg;
  }
}

HWReg Emitter::_isFRInRegister(FR fr) {
  auto &frState = frameRegs_[fr.index()];
  if (frState.localGpX)
    return useReg(frState.localGpX);
  if (frState.localVecD)
    return useReg(frState.localVecD);
  if (frState.globalReg)
    return frState.globalReg;
  return {};
}

HWReg Emitter::getOrAllocFRInVecD(FR fr, bool load) {
  auto &frState = frameRegs_[fr.index()];

  assert(!(load && frState.regIsDirty) && "Local is dirty");
#ifndef NDEBUG
  if (!load)
    frState.regIsDirty = true;
#endif

  if (frState.localVecD) {
    return useReg(frState.localVecD);
  }

  // Do we have a global VecD allocated to this FR?
  if (frState.globalReg.isValidVecD()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localGpX &&
          "If globalReg is not up to date, there must be a localReg");
      movHWFromHW<true>(frState.globalReg, frState.localGpX);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local VecD, so we must allocate a new tmp reg.
  HWReg hwVecD = allocTempVecD();
  _assignAllocatedLocalHWReg(fr, hwVecD);

  if (load) {
    if (frState.localGpX) {
      movHWFromHW<false>(hwVecD, frState.localGpX);
    } else if (frState.globalReg.isValidGpX()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWFromHW<false>(hwVecD, frState.globalReg);
    } else {
      _loadFrame(hwVecD, fr);
      assert(frState.frameUpToDate && "frame not up-to-date");
    }
  }

  return hwVecD;
}

HWReg Emitter::getOrAllocFRInGpX(FR fr, bool load) {
  auto &frState = frameRegs_[fr.index()];

  assert(!(load && frState.regIsDirty) && "Local is dirty");
#ifndef NDEBUG
  if (!load)
    frState.regIsDirty = true;
#endif

  if (frState.localGpX) {
    assert(!(load && frState.regIsDirty) && "Local is dirty");
    return useReg(frState.localGpX);
  }

  // Do we have a global GpX allocated to this FR?
  if (frState.globalReg.isValidGpX()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localVecD &&
          "If globalReg is not up to date, there must be a localReg");
      movHWFromHW<true>(frState.globalReg, frState.localVecD);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local GpX, so we must allocate a new tmp reg.
  HWReg hwGpX = allocTempGpX();
  _assignAllocatedLocalHWReg(fr, hwGpX);

  if (load) {
    if (frState.localVecD) {
      movHWFromHW<false>(hwGpX, frState.localVecD);
    } else if (frState.globalReg.isValidVecD()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWFromHW<false>(hwGpX, frState.globalReg);
    } else {
      assert(frState.frameUpToDate && "frame not up-to-date");
      _loadFrame(hwGpX, fr);
    }
  }

  return hwGpX;
}

HWReg Emitter::getOrAllocFRInAnyReg(
    FR fr,
    bool load,
    llvh::Optional<HWReg> preferred) {
  if (HWReg tmp = _isFRInRegister(fr))
    return tmp;

  // We have neither global nor local reg, so we must allocate a new tmp reg.
  HWReg hwReg{};
  if (preferred && preferred->isVecD()) {
    hwReg = allocTempVecD(preferred);
  } else {
    hwReg = allocTempGpX(preferred);
  }
  _assignAllocatedLocalHWReg(fr, hwReg);

  if (load) {
    assert(
        frameRegs_[fr.index()].frameUpToDate &&
        "Frame must be up to date when loading");
    _loadFrame(hwReg, fr);
  }

  return hwReg;
}

void Emitter::frUpdatedWithHW(FR fr, HWReg hwReg, FRType localType) {
  FRState &frState = frameRegs_[fr.index()];

  frState.frameUpToDate = false;
#ifndef NDEBUG
  frState.regIsDirty = false;
#endif

  if (frState.globalReg == hwReg) {
    frState.globalRegUpToDate = true;

    if (frState.localGpX)
      freeReg(frState.localGpX);
    if (frState.localVecD)
      freeReg(frState.localVecD);
  } else {
    frState.globalRegUpToDate = false;
    if (hwReg == frState.localGpX) {
      freeReg(frState.localVecD);
    } else {
      assert(
          hwReg == frState.localVecD &&
          "Updated reg doesn't match any FRState register");
      freeReg(frState.localGpX);
    }
  }
  frUpdateType(fr, localType);
}

void Emitter::frUpdateType(FR fr, FRType type) {
  frameRegs_[fr.index()].localType = type;
}

void Emitter::unreachable() {
  EMIT_RUNTIME_CALL(*this, void (*)(), _sh_unreachable);
}

void Emitter::profilePoint(uint16_t pointIndex) {
  comment("// ProfilePoint %u", pointIndex);
#ifdef HERMESVM_PROFILER_BB
  syncAllFRTempExcept({});
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, pointIndex);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, uint16_t),
      _interpreter_register_bb_execution);
#else
  // No-op if profiling is not enabled.
#endif
}

void Emitter::ret(FR frValue) {
  movHWFromFR(HWReg::gpX(22), frValue);
  a.b(returnLabel_);
}

void Emitter::mov(FR frRes, FR frInput, bool logComment) {
  // Sometimes mov() is used by other instructions, so logging is optional.
  if (logComment)
    comment("// %s r%u, r%u", "mov", frRes.index(), frInput.index());
  if (frRes == frInput)
    return;

  HWReg hwInput = getOrAllocFRInAnyReg(frInput, true);
  HWReg hwDest = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwDest, hwInput);
  frUpdatedWithHW(frRes, hwDest, frameRegs_[frInput.index()].localType);
}

void Emitter::loadParam(FR frRes, uint32_t paramIndex) {
  comment("// LoadParam r%u, %u", frRes.index(), paramIndex);

  asmjit::Error err;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwTmp = allocAndLogTempGpX();
  a64::GpW wTmp(hwTmp.indexInClass());

  a.ldur(
      wTmp,
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));

  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTmp, paramIndex));
  // Does paramIndex fit in the 12-bit unsigned immediate?
  if (err) {
    HWReg hwTmp2 = allocAndLogTempGpX();
    a64::GpW wTmp2(hwTmp2.indexInClass());
    loadBits64InGp(wTmp2, paramIndex, "paramIndex");
    a.cmp(wTmp, wTmp2);
    freeReg(hwTmp2);
  }
  a.b_lo(slowPathLab);

  freeReg(hwTmp);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);

  // FIXME: handle integer overflow better?
  int32_t ofs = ((int)StackFrameLayout::ThisArg - (int32_t)paramIndex) *
      (int)sizeof(SHLegacyValue);
  if (ofs >= 0)
    hermes_fatal("JIT integer overflow");
  EXPECT_ERROR(
      asmjit::kErrorInvalidDisplacement,
      err = a.ldur(hwRes.a64GpX(), a64::Mem(xFrame, ofs)));
  // Does the offset fit in the 9-bit signed offset?
  if (err) {
    ofs = -ofs;
    a64::GpX xRes = hwRes.a64GpX();
    if (ofs <= 4095) {
      a.sub(xRes, xFrame, ofs);
    } else {
      loadBits64InGp(xRes, ofs, nullptr);
      a.sub(xRes, xFrame, xRes);
    }
    a.ldr(xRes, a64::Mem(xRes));
  }

  a.bind(contLab);
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: LoadParam r%u", sl.frRes.index());
         em.a.bind(sl.slowPathLab);
         em.loadBits64InGp(
             sl.hwRes.a64GpX(), _sh_ljs_undefined().raw, "undefined");
         em.a.b(sl.contLab);
       }});
}

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

template <typename REG>
void Emitter::loadBits64InGp(
    const REG &dest,
    uint64_t bits,
    const char *constName) {
  if (isCheapConst(bits)) {
    a.mov(dest, bits);
  } else {
    a.ldr(dest, a64::Mem(roDataLabel_, uint64Const(bits, constName)));
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

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(a64::w2, stringID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHRuntimeModule *, uint32_t),
      _sh_ljs_get_bytecode_string);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<true>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
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
      SHLegacyValue(*)(SHRuntime *, SHRuntimeModule *, uint32_t),
      _sh_ljs_get_bytecode_bigint);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<true>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::toNumber(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumber", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput))
    return mov(frRes, frInput, false);

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  syncToFrame(frInput);

  hwInput = getOrAllocFRInGpX(frInput, true);
  a.cmp(hwInput.a64GpX(), xDoubleLim);
  a.b_hs(slowPathLab);

  if (frRes != frInput) {
    hwRes = getOrAllocFRInVecD(frRes, false);
    movHWFromHW<false>(hwRes, hwInput);
  } else {
    hwRes = hwInput;
  }
  frUpdatedWithHW(frRes, hwRes, FRType::Number);

  freeAllFRTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: toNumber r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             double (*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_double_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::vecD(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::toNumeric(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumeric", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput))
    return mov(frRes, frInput, false);

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  syncToFrame(frInput);

  hwInput = getOrAllocFRInGpX(frInput, true);
  a.cmp(hwInput.a64GpX(), xDoubleLim);
  a.b_hs(slowPathLab);

  if (frRes != frInput) {
    hwRes = getOrAllocFRInVecD(frRes, false);
    movHWFromHW<false>(hwRes, hwInput);
  } else {
    hwRes = hwInput;
  }
  frUpdatedWithHW(frRes, hwRes, FRType::UnknownPtr);

  freeAllFRTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: toNumeric r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_numeric_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::toInt32(FR frRes, FR frInput) {
  comment("// ToInt32 r%u, r%u", frRes.index(), frInput.index());

  HWReg hwTempGpX = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInVecD(frInput, true);
  emit_double_is_int(
      a, hwTempGpX.a64GpX(), hwTempVecD.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempGpX);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Number);

  // Truncate to an int32 in the fast path.
  a.scvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// to_int32 r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             double (*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_to_int32_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::vecD(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::addEmptyString(FR frRes, FR frInput) {
  comment("// AddEmptyString r%u, r%u", frRes.index(), frInput.index());

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  freeReg(hwTemp);
  freeAllFRTempExcept(frRes);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);

  // Check if the input is already a string and don't do anything.
  emit_sh_ljs_is_string(a, hwTemp.a64GpX(), hwInput.a64GpX());
  a.b_ne(slowPathLab);

  // Fast path.
  movHWFromHW<false>(hwRes, hwInput);
  frUpdatedWithHW(frRes, hwRes, FRType::Pointer);

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: AddEmptyString r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_add_empty_string_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::newObject(FR frRes) {
  comment("// NewObject r%u", frRes.index());
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  EMIT_RUNTIME_CALL(*this, SHLegacyValue(*)(SHRuntime *), _sh_ljs_new_object);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newObjectWithParent(FR frRes, FR frParent) {
  comment("// NewObjectWithParent r%u, r%u", frRes.index(), frParent.index());
  syncAllFRTempExcept(frRes != frParent ? frRes : FR());
  syncToFrame(frParent);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frParent);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
      _sh_ljs_new_object_with_parent);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newObjectWithBuffer(
    FR frRes,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  comment(
      "// NewObjectWithBuffer r%u, %u, %u",
      frRes.index(),
      shapeTableIndex,
      valBufferOffset);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, shapeTableIndex);
  a.mov(a64::w3, valBufferOffset);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHCodeBlock *, uint32_t, uint32_t),
      _interpreter_create_object_from_buffer);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newArray(FR frRes, uint32_t size) {
  comment("// NewArray r%u, %u", frRes.index(), size);
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, size);
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, uint32_t), _sh_ljs_new_array);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newArrayWithBuffer(
    FR frRes,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex) {
  comment(
      "// NewArrayWithBuffer r%u, %u, %u, %u",
      frRes.index(),
      numElements,
      numLiterals,
      bufferIndex);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, numElements);
  a.mov(a64::w3, numLiterals);
  a.mov(a64::w4, bufferIndex);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHCodeBlock *, uint32_t, uint32_t, uint32_t),
      _interpreter_create_array_from_buffer);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::newFastArray(FR frRes, uint32_t size) {
  comment("// NewFastArray r%u, %u", frRes.index(), size);
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, size);
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, uint32_t), _sh_new_fastarray);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::fastArrayLength(FR frRes, FR frArr) {
  comment("// FastArrayLength r%u, r%u", frRes.index(), frArr.index());
  // We allocate a temporary register to compute the address instead of using
  // the result register in case the result has a VecD allocated for it.
  HWReg temp = allocTempGpX();
  HWReg hwArr = getOrAllocFRInGpX(frArr, true);
  // Done allocating, free the temp so it can be reused for the result.
  freeReg(temp);
  emit_sh_ljs_get_pointer(a, temp.a64GpX(), hwArr.a64GpX());

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(hwRes, a64::Mem(temp.a64GpX(), offsetof(SHFastArray, length)));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::fastArrayLoad(FR frRes, FR frArr, FR frIdx) {
  comment(
      "// FastArrayLoad r%u, r%u, r%u",
      frRes.index(),
      frArr.index(),
      frIdx.index());
  asmjit::Label slowPathLab = newSlowPathLabel();
  // We allocate a temporary register to compute the address instead of using
  // the result register in case the result has a VecD allocated for it.
  HWReg hwTmpStorage = allocTempGpX();
  HWReg hwTmpSize = allocTempGpX();
  HWReg hwTmpIdxGpX = allocTempGpX();
  HWReg hwTmpIdxVecD = allocTempVecD();
  HWReg hwArr = getOrAllocFRInGpX(frArr, true);
  HWReg hwIdx = getOrAllocFRInVecD(frIdx, true);
  // Done allocating, free the temps so they can be reused for the result.
  freeReg(hwTmpStorage);
  freeReg(hwTmpSize);
  freeReg(hwTmpIdxGpX);
  freeReg(hwTmpIdxVecD);

  // Retrieve the FastArray pointer and use it to load the indexed storage
  // pointer.
  emit_sh_ljs_get_pointer(a, hwTmpStorage.a64GpX(), hwArr.a64GpX());
  movHWFromMem(
      hwTmpStorage,
      a64::Mem(hwTmpStorage.a64GpX(), offsetof(SHFastArray, indexedStorage)));

  // Load the size from the indexed storage.
  a.ldr(
      hwTmpSize.a64GpX().w(),
      a64::Mem(hwTmpStorage.a64GpX(), offsetof(SHArrayStorageSmall, size)));

  // Check if the index is a uint32.
  emit_double_is_uint32(
      a, hwTmpIdxGpX.a64GpX().w(), hwTmpIdxVecD.a64VecD(), hwIdx.a64VecD());
  // If the conversion was successful, compare the size against the index.
  // Otherwise, set the flags to zero to force the subsequent b_ls to be taken.
  a.ccmp(
      hwTmpSize.a64GpX().w(), hwTmpIdxGpX.a64GpX().w(), 0, a64::CondCode::kEQ);
  // If the index is out-of-bounds jump to the failure path.
  // TODO: We currently disregard the state of the registers on an OOB access
  // because we cannot JIT try-catch, so we are guaranteed to be leaving the
  // current function. If we add support for JIT of try-catch, we will have to
  // sync registers when the access is inside a try region.
  a.b_ls(slowPathLab);

  // Add the offset of the actual data in the ArrayStorage.
  a.add(
      hwTmpStorage.a64GpX(),
      hwTmpStorage.a64GpX(),
      offsetof(SHArrayStorageSmall, storage));

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(
      hwRes,
      a64::Mem(hwTmpStorage.a64GpX(), hwTmpIdxGpX.a64GpX(), a64::lsl(3)));
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .frRes = frRes,
       .frInput1 = frArr,
       .frInput2 = frIdx,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: FastArrayLoad r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(em, void (*)(SHRuntime *), _sh_throw_array_oob);
         // Call does not return.
       }});
}

void Emitter::fastArrayStore(FR frArr, FR frIdx, FR frVal) {
  comment(
      "// FastArrayStore r%u, r%u, r%u",
      frArr.index(),
      frIdx.index(),
      frVal.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frVal);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frVal);
  loadFrameAddr(a64::x2, frArr);
  movHWFromFR(HWReg{a64::d0}, frIdx);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, const SHLegacyValue *, SHLegacyValue *, double idx),
      _sh_fastarray_store);
}

void Emitter::fastArrayPush(FR frArr, FR frVal) {
  comment("// FastArrayPush r%u, r%u", frArr.index(), frVal.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frVal);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frVal);
  loadFrameAddr(a64::x2, frArr);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_fastarray_push);
}

void Emitter::fastArrayAppend(FR frArr, FR frOther) {
  comment("// FastArrayAppend r%u, r%u", frArr.index(), frOther.index());
  syncAllFRTempExcept({});
  syncToFrame(frArr);
  syncToFrame(frOther);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frOther);
  loadFrameAddr(a64::x2, frArr);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_fastarray_append);
}

void Emitter::getGlobalObject(FR frRes) {
  comment("// GetGlobalObject r%u", frRes.index());
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromMem(hwRes, a64::Mem(xRuntime, RuntimeOffsets::globalObject));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::declareGlobalVar(SHSymbolID symID) {
  comment("// DeclareGlobalVar %u", symID);

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, symID);
  EMIT_RUNTIME_CALL(
      *this, void (*)(SHRuntime *, SHSymbolID), _sh_ljs_declare_global_var);
}

void Emitter::createTopLevelEnvironment(FR frRes, uint32_t size) {
  comment("// CreateTopLevelEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, 0);
  a.mov(a64::w2, size);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *, uint32_t),
      _sh_ljs_create_environment);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::createFunctionEnvironment(FR frRes, uint32_t size) {
  comment("// CreateFunctionEnvironment r%u, %u", frRes.index(), size);

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  a.mov(a64::w2, size);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _sh_ljs_create_function_environment);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::createEnvironment(FR frRes, FR frParent, uint32_t size) {
  comment(
      "// CreateEnvironment r%u, r%u, %u",
      frRes.index(),
      frParent.index(),
      size);

  syncAllFRTempExcept(frRes != frParent ? frRes : FR{});
  syncToFrame(frParent);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frParent);
  a.mov(a64::w2, size);

  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *, uint32_t),
      _sh_ljs_create_environment);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getParentEnvironment(FR frRes, uint32_t level) {
  comment("// GetParentEnvironment r%u, %u", frRes.index(), level);

  HWReg hwTmp1 = allocTempGpX();
  a64::GpX xTmp1 = hwTmp1.a64GpX();

  // Get current closure.
  a.ldur(
      xTmp1,
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::CalleeClosureOrCB *
              (int)sizeof(SHLegacyValue)));
  // get pointer.
  emit_sh_ljs_get_pointer(a, xTmp1, xTmp1);
  // xTmp1 = closure->environment
  a.ldr(xTmp1, a64::Mem(xTmp1, offsetof(SHCallable, environment)));
  for (; level; --level) {
    // xTmp1 = env->parent.
    a.ldr(xTmp1, a64::Mem(xTmp1, offsetof(SHEnvironment, parentEnvironment)));
  }
  // encode object.
  emit_sh_ljs_object(a, xTmp1);

  freeReg(hwTmp1);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, hwTmp1);
  movHWFromHW<false>(hwRes, hwTmp1);
  frUpdatedWithHW(frRes, hwRes);
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
  movHWFromMem(hwRes, a64::Mem(hwRes.a64GpX(), ofs));
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

  a.ldr(
      xTmp1,
      a64::Mem(
          xTmp1,
          offsetof(SHEnvironment, slots) + sizeof(SHLegacyValue) * slot));

  freeReg(hwTmp1);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, hwTmp1);
  movHWFromHW<false>(hwRes, hwTmp1);
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::storeToEnvironment(bool np, FR frEnv, uint32_t slot, FR frValue) {
  // TODO: this should really be inlined!
  comment(
      "// StoreNPToEnvironment r%u, %u, r%u",
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

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frEnv);
  loadBits64InGp(a64::x2, (uint64_t)runtimeModule, "RuntimeModule");
  loadBits64InGp(a64::w3, functionID, nullptr);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, const SHLegacyValue *, SHRuntimeModule *, uint32_t),
      _sh_ljs_create_bytecode_closure);

  freeAllFRTempExcept({});
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
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

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  loadFrameAddr(a64::x2, frEnv);
  loadBits64InGp(a64::x3, (uint64_t)runtimeModule, "RuntimeModule");
  a.mov(a64::w4, functionID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          const SHLegacyValue *,
          SHRuntimeModule *,
          uint32_t),
      _interpreter_create_generator);

  freeAllFRTempExcept({});
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getArgumentsLength(FR frRes, FR frLazyReg) {
  comment("// GetArgumentsLength r%u, r%u", frRes.index(), frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept(frRes != frLazyReg ? frRes : FR());
  syncToFrame(frLazyReg);

  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwTemp = allocTempGpX();
  freeAllFRTempExcept({});
  freeReg(hwTemp);
  // Avoid an extra mov by using the temp register for the result if possible.
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(frRes, hwRes, FRType::Number);

  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwLazyReg.a64GpX());
  a.b_eq(slowPathLab);

  // Fast path: if it's not an object, read from the frame.
  static_assert(
      HERMESVALUE_VERSION == 1,
      "NativeUint32 is stored as the lower 32 bits of the raw HermesValue");
  a.ldur(
      hwTemp.a64GpX().w(),
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));

  // Encode the uint32_t as a double (making it a HermesValue).
  a.ucvtf(hwRes.a64VecD(), hwTemp.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frLazyReg,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: GetArgumentsLength r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
             _sh_ljs_get_arguments_length);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

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
      a.add(a64::x3, a64::x3, sizeof(SHPropertyCacheEntry) * cacheIdx);
  }
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHPropertyCacheEntry *),
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

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.ldur(
      a64::x1,
      a64::Mem(
          xFrame, (int)StackFrameLayout::ThisArg * (int)sizeof(SHLegacyValue)));
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue),
      _sh_ljs_coerce_this_ns);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::coerceThisNS(FR frRes, FR frThis) {
  comment("// CoerceThisNS r%u, r%u", frRes.index(), frThis.index());

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  movHWFromFR(HWReg::gpX(1), frThis);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue),
      _sh_ljs_coerce_this_ns);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::debugger() {
  comment("// Debugger");
  if (dumpJitCode_ & DumpJitCode::BRK)
    a.brk(0);
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

void Emitter::iteratorBegin(FR frRes, FR frSource) {
  comment("// IteratorBegin r%u, r%u", frRes.index(), frSource.index());

  syncAllFRTempExcept(frRes != frSource ? frRes : FR());
  syncToFrame(frSource);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *),
      _sh_ljs_iterator_begin_rjs);

  syncFrameOutParam(frSource);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::iteratorNext(FR frRes, FR frIteratorOrIdx, FR frSourceOrNext) {
  comment(
      "// IteratorNext r%u, r%u, r%u",
      frRes.index(),
      frIteratorOrIdx.index(),
      frSourceOrNext.index());

  syncAllFRTempExcept(
      frRes != frIteratorOrIdx && frRes != frSourceOrNext ? frRes : FR());
  syncToFrame(frIteratorOrIdx);
  syncToFrame(frSourceOrNext);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frIteratorOrIdx);
  loadFrameAddr(a64::x2, frSourceOrNext);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, const SHLegacyValue *),
      _sh_ljs_iterator_next_rjs);

  syncFrameOutParam(frIteratorOrIdx);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::iteratorClose(FR frIteratorOrIdx, bool ignoreExceptions) {
  comment(
      "// IteratorClose r%u, %u", frIteratorOrIdx.index(), ignoreExceptions);

  syncAllFRTempExcept({});
  syncToFrame(frIteratorOrIdx);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frIteratorOrIdx);
  a.mov(a64::w2, ignoreExceptions);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, const SHLegacyValue *, bool),
      _sh_ljs_iterator_close_rjs);
}

void Emitter::throwInst(FR frInput) {
  comment("// Throw r%u", frInput.index());

  syncAllFRTempExcept({});
  movHWFromFR(HWReg::gpX(1), frInput);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  EMIT_RUNTIME_CALL(*this, void (*)(SHRuntime *, SHLegacyValue), _sh_throw);
}

void Emitter::throwIfEmpty(FR frRes, FR frInput) {
  comment("// ThrowIfEmpty r%u, r%u", frRes.index(), frInput.index());

  asmjit::Label slowPathLab = newSlowPathLabel();

  // TODO: Add back the sync/free calls inside try.
  // Outside a try it's not observable behavior.
  // syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  HWReg hwTemp = allocTempGpX();
  // freeAllFRTempExcept({});
  freeReg(hwTemp);

  emit_sh_ljs_is_empty(a, hwTemp.a64GpX(), hwInput.a64GpX());
  a.b_eq(slowPathLab);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  movHWFromHW<false>(hwRes, hwInput);
  frUpdatedWithHW(frRes, hwRes);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: ThrowIfEmpty r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         EMIT_RUNTIME_CALL(em, void (*)(SHRuntime *), _sh_throw_empty);
         // Call does not return.
       }});
}

void Emitter::createRegExp(
    FR frRes,
    SHSymbolID patternID,
    SHSymbolID flagsID,
    uint32_t regexpID) {
  comment("// CreateRegExp r%u, %u, %u", frRes.index(), patternID, flagsID);

  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadBits64InGp(a64::x1, (uint64_t)codeBlock_, "CodeBlock");
  a.mov(a64::w2, patternID);
  a.mov(a64::w3, flagsID);
  a.mov(a64::w4, regexpID);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHCodeBlock *, uint32_t, uint32_t, uint32_t),
      _interpreter_create_regexp);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::loadParentNoTraps(FR frRes, FR frObj) {
  comment("// LoadParentNoTraps r%u, r%u", frRes.index(), frObj.index());

  HWReg hwObj = getOrAllocFRInGpX(frObj, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  HWReg hwTmp = allocTempGpX();
  freeReg(hwTmp);
  a64::GpX xTmp = hwTmp.a64GpX();
  a64::GpX xRes = hwRes.a64GpX();
  emit_sh_ljs_get_pointer(a, xTmp, hwObj.a64GpX());
  // xTmp contains the unencoded pointer value.
  a.ldr(xTmp, a64::Mem(xTmp, offsetof(SHJSObject, parent)));
  // Check whether it is nullptr and set flags.
  a.cmp(xTmp, 0);
  // xRes contains the encoded pointer.
  emit_sh_ljs_object2(a, xRes, xTmp);
  // xTmp contains encoded null.
  loadBits64InGp(xTmp, _sh_ljs_null().raw, "null");
  // If the pointer was nullptr, use encoded null, otherwise encoded ptr.
  a.csel(xRes, xTmp, xRes, asmjit::arm::CondCode::kEQ);

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::typedLoadParent(FR frRes, FR frObj) {
  comment("// TypedLoadParent r%u, r%u", frRes.index(), frObj.index());

  HWReg hwObj = getOrAllocFRInGpX(frObj, true);
  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  a64::GpX xRes = hwRes.a64GpX();
  emit_sh_ljs_get_pointer(a, xRes, hwObj.a64GpX());
  a.ldr(xRes, a64::Mem(xRes, offsetof(SHJSObject, parent)));
  emit_sh_ljs_object(a, xRes);

  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::typedStoreParent(FR frStoredValue, FR frObj) {
  comment("// TypedStoreParent r%u, r%u", frStoredValue.index(), frObj.index());

  syncAllFRTempExcept({});
  syncToFrame(frStoredValue);
  syncToFrame(frObj);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frStoredValue);
  loadFrameAddr(a64::x2, frObj);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, const SHLegacyValue *, const SHLegacyValue *),
      _sh_typed_store_parent);
}

void Emitter::putByValImpl(
    FR frTarget,
    FR frKey,
    FR frValue,
    const char *name,
    void (*shImpl)(
        SHRuntime *shr,
        SHLegacyValue *target,
        SHLegacyValue *key,
        SHLegacyValue *value),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frTarget.index(),
      frKey.index(),
      frValue.index());

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  callThunkWithSavedIP((void *)shImpl, shImplName);
}

void Emitter::delByIdImpl(
    FR frRes,
    FR frTarget,
    SHSymbolID key,
    const char *name,
    SHLegacyValue (
        *shImpl)(SHRuntime *shr, SHLegacyValue *target, SHSymbolID key),
    const char *shImplName) {
  comment("// %s r%u, r%u, %u", name, frRes.index(), frTarget.index(), key);

  syncAllFRTempExcept(frRes != frTarget ? frRes : FR{});
  syncToFrame(frTarget);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, key);
  callThunkWithSavedIP((void *)shImpl, shImplName);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::delByValImpl(
    FR frRes,
    FR frTarget,
    FR frKey,
    const char *name,
    SHLegacyValue (
        *shImpl)(SHRuntime *shr, SHLegacyValue *target, SHLegacyValue *key),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frTarget.index(),
      frKey.index());

  syncAllFRTempExcept(frRes != frTarget && frRes != frKey ? frRes : FR{});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  callThunkWithSavedIP((void *)shImpl, shImplName);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getByIdImpl(
    FR frRes,
    SHSymbolID symID,
    FR frSource,
    uint8_t cacheIdx,
    const char *name,
    SHLegacyValue (*shImpl)(
        SHRuntime *shr,
        const SHLegacyValue *source,
        SHSymbolID symID,
        SHPropertyCacheEntry *propCacheEntry),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, cache %u, symID %u",
      name,
      frRes.index(),
      frSource.index(),
      cacheIdx,
      symID);

  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  HWReg hwRes;

  // All temporaries will potentially be clobbered by the slow path.
  syncAllFRTempExcept(frRes != frSource ? frRes : FR{});

  if (cacheIdx != hbc::PROPERTY_CACHING_DISABLED) {
    // Label for indirect property access.
    asmjit::Label indirectLab = a.newLabel();
    slowPathLab = a.newLabel();
    contLab = a.newLabel();

    // We don't need the other temporaries.
    freeAllFRTempExcept(frSource);

    // We need the source in a GPx register.
    HWReg hwSourceGpx = getOrAllocFRInGpX(frSource, true);

    // Here we start allocating and freeing registers. It is important to
    // realize that this doesn't generate any code, it only updates metadata,
    // marking registers as used or free. So, we have to perform a series of
    // register allocs and frees, ahead of time, based on our understanding of
    // how the live ranges of these registers overlap. Then we just use the
    // recorded registers at the right time.

    // xTemp1 will contain the input object.
    HWReg hwTemp1Gpx = allocTempGpX();
    auto xTemp1 = hwTemp1Gpx.a64GpX();
    // Free frSource before allocating more temporaries, because we won't need
    // it at the same time as them.
    freeFRTemp(frSource);

    // Get register assignments for the rest of the temporaries.
    HWReg hwTemp2Gpx = allocTempGpX();
    HWReg hwTemp3Gpx = allocTempGpX();
    HWReg hwTemp4Gpx = allocTempGpX();
    auto xTemp2 = hwTemp2Gpx.a64GpX();
    auto xTemp3 = hwTemp3Gpx.a64GpX();
    auto xTemp4 = hwTemp4Gpx.a64GpX();

    // Now that we have recorded their registers, mark all temp registers as
    // free.
    freeReg(hwTemp1Gpx);
    freeReg(hwTemp2Gpx);
    freeReg(hwTemp3Gpx);
    freeReg(hwTemp4Gpx);

    // Allocate the result register. Note that it can overlap the temps we just
    // freed.
    hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));

    // Finally we begin code generation for the fast path.

    // Is the input an object.
    emit_sh_ljs_is_object(a, xTemp1, hwSourceGpx.a64GpX());
    a.b_ne(slowPathLab);
    // xTemp1 is the pointer to the object.
    emit_sh_ljs_get_pointer(a, xTemp1, hwSourceGpx.a64GpX());

#ifdef HERMESVM_COMPRESSED_POINTERS
#error Compressed pointers not implemented
#endif
    // xTemp2 is the hidden class.
    a.ldr(xTemp2, a64::Mem(xTemp1, offsetof(SHJSObject, clazz)));

    // xTemp3 points to the start of read property cache.
    a.ldr(xTemp3, a64::Mem(roDataLabel_, roOfsReadPropertyCachePtr_));
    // xTemp4 = cacheEntry->clazz.
    a.ldr(
        xTemp4,
        a64::Mem(
            xTemp3,
            sizeof(SHPropertyCacheEntry) * cacheIdx +
                offsetof(SHPropertyCacheEntry, clazz)));

    // Compare hidden classes.
    a.cmp(xTemp2, xTemp4);
    a.b_ne(slowPathLab);

    // Hidden class matches. Fetch the slot in xTemp4
    a.ldr(
        xTemp4.w(),
        a64::Mem(
            xTemp3,
            sizeof(SHPropertyCacheEntry) * cacheIdx +
                offsetof(SHPropertyCacheEntry, slot)));

    // Is it an indirect slot?
    a.cmp(xTemp4.w(), HERMESVM_DIRECT_PROPERTY_SLOTS);
    a.b_hs(indirectLab);

    // Load from a direct slot.
    a.add(xTemp3, xTemp1, offsetof(SHJSObjectAndDirectProps, directProps));
    a.ldr(
        hwRes.a64GpX(),
        a64::Mem(xTemp3, xTemp4, a64::Shift(a64::ShiftOp::kLSL, 3)));

    a.b(contLab);

    a.bind(indirectLab);
    // Load from an in-direct slot.
    // xTemp1 is the object
    // xTemp4 is the slot

    // xTemp1 = xTemp1->propStorage
    a.ldr(xTemp1, a64::Mem(xTemp1, offsetof(SHJSObject, propStorage)));
    constexpr ssize_t ofs = offsetof(SHArrayStorageSmall, storage) -
        HERMESVM_DIRECT_PROPERTY_SLOTS * sizeof(SHGCSmallHermesValue);
    if constexpr (ofs < 0)
      a.sub(xTemp1, xTemp1, -ofs);
    else
      a.add(xTemp1, xTemp1, ofs);
    a.ldr(
        hwRes.a64GpX(),
        a64::Mem(xTemp1, xTemp4, a64::Shift(a64::ShiftOp::kLSL, 3)));
    a.b(contLab);

    a.bind(slowPathLab);

    // Ensure the frSource is in memory for the fast path. Note that we haven't
    // done it before.
    // Note that this is the reason we can't use a regular slow path at the
    // end of the function. We need syncToFrame() to execute in the slow path,
    // but by then the state is gone.
    syncToFrame(frSource);
  } else {
    // We arrive here if there is no fast path. Ensure that frSource is in
    // memory.
    syncToFrame(frSource);
    // All temporaries will be clobbered.
    freeAllFRTempExcept({});

    // Remember the result register.
    hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  }

  comment(
      "// %s r%u, r%u, cache %u, symID %u",
      name,
      frRes.index(),
      frSource.index(),
      cacheIdx,
      symID);

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  a.mov(a64::w2, symID);
  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x3, 0);
  } else {
    a.ldr(a64::x3, a64::Mem(roDataLabel_, roOfsReadPropertyCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x3, a64::x3, sizeof(SHPropertyCacheEntry) * cacheIdx);
  }
  callThunkWithSavedIP((void *)shImpl, shImplName);

  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);

  if (contLab.isValid())
    a.bind(contLab);
}

void Emitter::switchImm(
    FR frInput,
    const asmjit::Label &defaultLabel,
    llvh::ArrayRef<const asmjit::Label *> labels,
    uint32_t minVal,
    uint32_t maxVal) {
  comment("// switchImm r%u, min %u, max %u", frInput.index(), minVal, maxVal);

  asmjit::Error err;

  // End of the basic block.
  syncAllFRTempExcept({});

  // Load the input value into a double register to check if it's an int.
  HWReg hwInput = getOrAllocFRInVecD(frInput, true);

  HWReg hwTempInput = allocTempGpX();
  HWReg hwTempTarget = allocTempGpX();
  HWReg hwTempD = allocTempVecD();
  freeReg(hwTempInput);
  freeReg(hwTempTarget);
  freeReg(hwTempD);

  a64::VecD dInput = hwInput.a64VecD();
  a64::GpW wTempInput = hwTempInput.a64GpX().w();

  // Convert the input to an integer and back to double,
  // and check if the value remained the same.
  // If it didn't, jump to the default label.
  emit_double_is_uint32(a, wTempInput, hwTempD.a64VecD(), dInput);
  a.b_ne(defaultLabel);

  // Check if the integer value in xTemp is in range.
  // First check minVal.
  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTempInput, minVal));
  if (err) {
    a.mov(hwTempTarget.a64GpX().w(), minVal);
    a.cmp(wTempInput, hwTempTarget.a64GpX().w());
  }
  // If the value is lower than minVal, jump to the default label.
  a.b_lo(defaultLabel);

  // Now check maxVal.
  EXPECT_ERROR(asmjit::kErrorInvalidImmediate, err = a.cmp(wTempInput, maxVal));
  if (err) {
    a.mov(hwTempTarget.a64GpX().w(), maxVal);
    a.cmp(wTempInput, hwTempTarget.a64GpX().w());
  }
  // If the value is higher than maxVal, jump to the default label.
  a.b_hi(defaultLabel);

  // Compute the offset into the jump table, dereference, and jump.
  // Offset by the minVal if necessary.
  if (minVal != 0) {
    EXPECT_ERROR(
        asmjit::kErrorInvalidImmediate,
        err = a.sub(wTempInput, wTempInput, minVal));
    if (err) {
      a.mov(hwTempTarget.a64GpX().w(), minVal);
      a.sub(wTempInput, wTempInput, hwTempTarget.a64GpX().w());
    }
  }

  // Label for the start of the jump table and the base of the br instruction
  // that actually executes the switch.
  // Used for both purposes due to placement of the jump table directly after
  // the br.
  asmjit::Label tableLab = a.newLabel();

  // wTempInput contains the index into the jump table.
  a64::GpX xTempTarget = hwTempTarget.a64GpX();
  // Load the jump offset into wTempInput by using adr to find the address of
  // the table and then reading 4 bytes from an offset of wTempInput bytes.
  a.adr(xTempTarget, tableLab);
  // Left shift 2 to get the byte offset into the table.
  a.ldr(
      wTempInput,
      a64::Mem(xTempTarget, wTempInput, a64::Shift(a64::ShiftOp::kLSL, 2)));
  // Add the jump offset to the base of the table to get the target address.
  a.add(xTempTarget, xTempTarget, wTempInput.x(), a64::sxtw(0));
  // Branch to the target address.
  a.br(xTempTarget);

  // Emit the jump table.
  // NOTE: The jump table is emitted immediately after the br instruction that
  // uses it.
  a.bind(tableLab);
  for (const asmjit::Label *label : labels) {
    a.embedLabelDelta(*label, tableLab, /* size */ 4);
  }

  // Do this always, since this could be the end of the BB.
  freeAllFRTempExcept({});
}

void Emitter::getByVal(FR frRes, FR frSource, FR frKey) {
  comment(
      "// getByVal r%u, r%u, r%u",
      frRes.index(),
      frSource.index(),
      frKey.index());

  syncAllFRTempExcept(frRes != frSource && frRes != frKey ? frRes : FR());
  syncToFrame(frSource);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  loadFrameAddr(a64::x2, frKey);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_get_by_val_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getByIndex(FR frRes, FR frSource, uint32_t key) {
  comment("// getByIdx r%u, r%u, %u", frRes.index(), frSource.index(), key);

  syncAllFRTempExcept(frRes != frSource ? frRes : FR());
  syncToFrame(frSource);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frSource);
  a.mov(a64::w2, key);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _sh_ljs_get_by_index_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::putByIdImpl(
    FR frTarget,
    SHSymbolID symID,
    FR frValue,
    uint8_t cacheIdx,
    const char *name,
    void (*shImpl)(
        SHRuntime *shr,
        SHLegacyValue *target,
        SHSymbolID symID,
        SHLegacyValue *value,
        SHPropertyCacheEntry *propCacheEntry),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, cache %u, symID %u",
      name,
      frTarget.index(),
      frValue.index(),
      cacheIdx,
      symID);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, symID);
  loadFrameAddr(a64::x3, frValue);
  if (cacheIdx == hbc::PROPERTY_CACHING_DISABLED) {
    a.mov(a64::x4, 0);
  } else {
    a.ldr(a64::x4, a64::Mem(roDataLabel_, roOfsWritePropertyCachePtr_));
    if (cacheIdx != 0)
      a.add(a64::x4, a64::x4, sizeof(SHPropertyCacheEntry) * cacheIdx);
  }
  callThunkWithSavedIP((void *)shImpl, shImplName);
}

asmjit::Label Emitter::newPrefLabel(const char *pref, size_t index) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%s%lu", pref, index);
  return a.newNamedLabel(buf);
}

void Emitter::putOwnByIndex(FR frTarget, FR frValue, uint32_t key) {
  comment(
      "// putOwnByIdx r%u, r%u, %u", frTarget.index(), frValue.index(), key);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, key);
  loadFrameAddr(a64::x3, frValue);
  EMIT_RUNTIME_CALL(
      *this,
      void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
      _sh_ljs_put_own_by_index);
}

void Emitter::putOwnByVal(FR frTarget, FR frValue, FR frKey, bool enumerable) {
  comment(
      "// PutOwnByVal r%u, r%u, r%u",
      frTarget.index(),
      frValue.index(),
      frKey.index());

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  syncToFrame(frKey);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frValue);
  if (enumerable) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_put_own_by_val);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
        _sh_ljs_put_own_ne_by_val);
  }
}

void Emitter::putOwnGetterSetterByVal(
    FR frTarget,
    FR frKey,
    FR frGetter,
    FR frSetter,
    bool enumerable) {
  comment(
      "// PutOwnGetterSetterByVal r%u, r%u, r%u, r%u, %d",
      frTarget.index(),
      frKey.index(),
      frGetter.index(),
      frSetter.index(),
      enumerable);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frKey);
  syncToFrame(frGetter);
  syncToFrame(frSetter);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  loadFrameAddr(a64::x2, frKey);
  loadFrameAddr(a64::x3, frGetter);
  loadFrameAddr(a64::x4, frSetter);
  a.mov(a64::w5, enumerable);

  EMIT_RUNTIME_CALL(
      *this,
      void (*)(
          SHRuntime *shr,
          SHLegacyValue *target,
          SHLegacyValue *key,
          SHLegacyValue *getter,
          SHLegacyValue *setter,
          bool enumerable),
      _sh_ljs_put_own_getter_setter_by_val);
}

void Emitter::putNewOwnById(
    FR frTarget,
    FR frValue,
    SHSymbolID key,
    bool enumerable) {
  comment(
      "// PutNewOwn%sById r%u, r%u, %u",
      enumerable ? "NE" : "",
      frTarget.index(),
      frValue.index(),
      key);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  a.mov(a64::w2, key);
  loadFrameAddr(a64::x3, frValue);
  if (enumerable) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *shr,
            SHLegacyValue *target,
            SHSymbolID key,
            SHLegacyValue *value),
        _sh_ljs_put_new_own_by_id);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(
            SHRuntime *shr,
            SHLegacyValue *target,
            SHSymbolID key,
            SHLegacyValue *value),
        _sh_ljs_put_new_own_ne_by_id);
  }
}

void Emitter::getOwnBySlotIdx(FR frRes, FR frTarget, uint32_t slotIdx) {
  comment(
      "// GetOwnBySlotIdx r%u, r%u, %u",
      frRes.index(),
      frTarget.index(),
      slotIdx);

#ifndef HERMESVM_BOXED_DOUBLES
  if (slotIdx < JSObject::DIRECT_PROPERTY_SLOTS) {
    // If the slot is in the direct property slots, we can just load it
    // directly.
    auto ofs = offsetof(SHJSObjectAndDirectProps, directProps) +
        slotIdx * sizeof(SHLegacyValue);
    // We allocate a temporary register to compute the address instead of using
    // the result register in case the result has a VecD allocated for it.
    HWReg temp = allocTempGpX();
    movHWFromFR(temp, frTarget);
    emit_sh_ljs_get_pointer(a, temp.a64GpX(), temp.a64GpX());
    // Free the temporary register before allocating the result so it can be
    // reused.
    freeReg(temp);
    HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
    movHWFromMem(hwRes, a64::Mem(temp.a64GpX(), ofs));
    frUpdatedWithHW(frRes, hwRes);
    return;
  }
#endif

  // Free x1 first, such that if frTarget is in any register (except from x1),
  // we can mov it in before we free all the registers.
  syncAndFreeTempReg(HWReg::gpX(1));
  movHWFromFR(HWReg::gpX(1), frTarget);

  syncAllFRTempExcept({});
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  // For indirect loads, 0 is the first indirect index.
  a.mov(
      a64::w2,
      slotIdx < JSObject::DIRECT_PROPERTY_SLOTS
          ? slotIdx
          : slotIdx - JSObject::DIRECT_PROPERTY_SLOTS);

  if (slotIdx < JSObject::DIRECT_PROPERTY_SLOTS) {
    EMIT_RUNTIME_CALL(
        *this,
        SHLegacyValue(*)(SHRuntime *, SHLegacyValue, uint32_t),
        _sh_prload_direct);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        SHLegacyValue(*)(SHRuntime *, SHLegacyValue, uint32_t),
        _sh_prload_indirect);
  }

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::putOwnBySlotIdx(FR frTarget, FR frValue, uint32_t slotIdx) {
  comment(
      "// PutOwnBySlotIdx r%u, r%u, %u",
      frTarget.index(),
      frValue.index(),
      slotIdx);

  syncAllFRTempExcept({});
  syncToFrame(frTarget);
  syncToFrame(frValue);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frTarget);
  // For indirect stores, 0 is the first indirect index.
  a.mov(
      a64::w2,
      slotIdx < JSObject::DIRECT_PROPERTY_SLOTS
          ? slotIdx
          : slotIdx - JSObject::DIRECT_PROPERTY_SLOTS);
  loadFrameAddr(a64::x3, frValue);

  if (slotIdx < JSObject::DIRECT_PROPERTY_SLOTS) {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
        _sh_prstore_direct);
  } else {
    EMIT_RUNTIME_CALL(
        *this,
        void (*)(SHRuntime *, SHLegacyValue *, uint32_t, SHLegacyValue *),
        _sh_prstore_indirect);
  }
}

void Emitter::instanceOf(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// InstanceOf r%u, r%u, r%u",
      frRes.index(),
      frLeft.index(),
      frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_instance_of_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::isIn(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// isIn r%u, r%u, r%u", frRes.index(), frLeft.index(), frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_is_in_rjs);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

int32_t Emitter::reserveData(
    int32_t dsize,
    size_t align,
    asmjit::TypeId typeId,
    int32_t itemCount,
    const char *comment) {
  // Align the new data.
  size_t oldSize = roData_.size();
  size_t dataOfs = (roData_.size() + align - 1) & ~(align - 1);
  if (dataOfs >= INT32_MAX)
    hermes::hermes_fatal("JIT RO data overflow");
  // Grow to include the data.
  roData_.resize(dataOfs + dsize);

  // If logging is enabled, generate data descriptors.
  if (logger_) {
    // Optional padding descriptor.
    if (dataOfs != oldSize) {
      int32_t gap = (int32_t)(dataOfs - oldSize);
      roDataDesc_.push_back(
          {.size = gap, .typeId = asmjit::TypeId::kUInt8, .itemCount = gap});
    }

    roDataDesc_.push_back(
        {.size = dsize,
         .typeId = typeId,
         .itemCount = itemCount,
         .comment = comment});
  }

  return (int32_t)dataOfs;
}

/// Register a 64-bit constant in RO DATA and return its offset.
int32_t Emitter::uint64Const(uint64_t bits, const char *comment) {
  auto [it, inserted] = fp64ConstMap_.try_emplace(bits, 0);
  if (inserted) {
    int32_t dataOfs = reserveData(
        sizeof(double), sizeof(double), asmjit::TypeId::kFloat64, 1, comment);
    memcpy(roData_.data() + dataOfs, &bits, sizeof(double));
    it->second = dataOfs;
  }
  return it->second;
}

asmjit::Label Emitter::registerThunk(void *fn, const char *name) {
  auto [it, inserted] = thunkMap_.try_emplace(fn, 0);
  // Is this a new thunk?
  if (inserted) {
    it->second = thunks_.size();
    int32_t dataOfs =
        reserveData(sizeof(fn), sizeof(fn), asmjit::TypeId::kUInt64, 1, name);
    memcpy(roData_.data() + dataOfs, &fn, sizeof(fn));
    thunks_.emplace_back(name ? a.newNamedLabel(name) : a.newLabel(), dataOfs);
  }

  return thunks_[it->second].first;
}

void Emitter::emitSlowPaths() {
  while (!slowPaths_.empty()) {
    SlowPath &sp = slowPaths_.front();
    emittingIP = sp.emittingIP;
    sp.emit(*this, sp);
    slowPaths_.pop_front();
  }
  emittingIP = nullptr;

  auto prohibitInvoke = codeBlock_->getHeaderFlags().prohibitInvoke;
  if (prohibitInvoke != ProhibitInvoke::None) {
    a.bind(throwInvalidInvoke_);
    // We don't register a thunk since there will only be a single call to this.
    // Note that we also don't save the IP, because this is being thrown in the
    // caller's context.
    // This is thrown at the start of the function, so runtime is already in x0.
    if (prohibitInvoke == ProhibitInvoke::Call)
      EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
          *this, void (*)(SHRuntime *), _sh_throw_invalid_call);
    else
      EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP(
          *this, void (*)(SHRuntime *), _sh_throw_invalid_construct);
  }
}

void Emitter::emitThunks() {
  comment("// Thunks");
  for (const auto &th : thunks_) {
    a.bind(th.first);
    a.ldr(a64::x16, a64::Mem(roDataLabel_, th.second));
    a.br(a64::x16);
  }
}

void Emitter::emitROData() {
  a.bind(roDataLabel_);
  if (!logger_) {
    a.embed(roData_.data(), roData_.size());
  } else {
    int32_t ofs = 0;
    for (const auto &desc : roDataDesc_) {
      if (desc.comment)
        comment("// %s", desc.comment);
      a.embedDataArray(desc.typeId, roData_.data() + ofs, desc.itemCount);
      ofs += desc.size;
    }
  }
}

void Emitter::call(FR frRes, FR frCallee, uint32_t argc) {
  comment("// Call r%u, r%u, %u", frRes.index(), frCallee.index(), argc);
  uint32_t nRegs = frameRegs_.size();

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

  // Store undefined as the new target.
  FR ntFrameArg{nRegs + hbc::StackFrameLayout::NewTarget};
  loadConstBits64(
      ntFrameArg, _sh_ljs_undefined().raw, FRType::UnknownNonPtr, "undefined");

#ifndef NDEBUG
  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  assert(
      !frameRegs_[calleeFrameArg.index()].globalReg &&
      "frame regs are not number/non-pointer so can't have global reg");
  assert(
      !frameRegs_[ntFrameArg.index()].globalReg &&
      "frame regs are not number/non-pointer so can't have global reg");

  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < argc; ++i) {
    assert(
        !frameRegs_[nRegs + hbc::StackFrameLayout::ThisArg - i].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }
#endif

  syncAllFRTempExcept(FR());
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  a.mov(a64::w2, argc - 1);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _jit_dispatch_call);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::callN(FR frRes, FR frCallee, llvh::ArrayRef<FR> args) {
  comment(
      "// Call%zu r%u, r%u, ...args",
      args.size(),
      frRes.index(),
      frCallee.index());
  uint32_t nRegs = frameRegs_.size();

  FR calleeFrameArg{nRegs + hbc::StackFrameLayout::CalleeClosureOrCB};
  // Store the callee to the right location in the frame.
  if (frCallee != calleeFrameArg) {
    // Free any temp register before we mov into it so movFRFromHW stores
    // directly to the frame.
    freeFRTemp(calleeFrameArg);
    auto calleeReg = getOrAllocFRInAnyReg(frCallee, true);
    movFRFromHW(
        calleeFrameArg, calleeReg, frameRegs_[frCallee.index()].localType);
  }
  syncToFrame(calleeFrameArg);

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

  // For now we sync all registers, since we skip writing to the frame in some
  // cases above, but in principle, we could track frRes specially.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  a.mov(a64::w2, args.size() - 1);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _jit_dispatch_call);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::callBuiltin(FR frRes, uint32_t builtinIndex, uint32_t argc) {
  comment(
      "// CallBuiltin r%u, %s, %u",
      frRes.index(),
      getBuiltinMethodName(builtinIndex),
      argc);
  // CallBuiltin internally sets "this", so we don't sync it to memory.
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
  // NOTE: _sh_ljs_call_builtin does not itself populate the SavedIP field, but
  // it will be populated by NativeFunction::_nativeCall.
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t, uint32_t),
      _sh_ljs_call_builtin);
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

  auto calleeFrameArg = FR{nRegs + hbc::StackFrameLayout::CalleeClosureOrCB};
  // Store the callee to the right location in the frame.
  if (calleeFrameArg != frCallee) {
    // Free any temp register before we mov into it so movFRFromHW stores
    // directly to the frame.
    freeFRTemp(calleeFrameArg);
    auto calleeReg = getOrAllocFRInAnyReg(frCallee, true);
    movFRFromHW(
        calleeFrameArg, calleeReg, frameRegs_[frCallee.index()].localType);
  }

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

#ifndef NDEBUG
  // No need to sync the set up call stack to the frame memory,
  // because it these registers can't have global registers.
  for (uint32_t i = 0; i < argc; ++i) {
    assert(
        !frameRegs_[nRegs + hbc::StackFrameLayout::ThisArg - i].globalReg &&
        "frame regs are not number/non-pointer so can't have global reg");
  }

  assert(
      !frameRegs_[calleeFrameArg.index()].globalReg &&
      "frame regs are not number/non-pointer so can't have global reg");
  assert(
      !frameRegs_[ntFrameArg.index()].globalReg &&
      "frame regs are not number/non-pointer so can't have global reg");
#endif

  syncAllFRTempExcept({});
  freeAllFRTempExcept(frRes);

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  a.mov(a64::w2, argc - 1);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _jit_dispatch_call);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
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

  auto calleeFrameArg = FR{nRegs + hbc::StackFrameLayout::CalleeClosureOrCB};
  // Store the callee to the right location in the frame.
  if (calleeFrameArg != frCallee) {
    // Free any temp register before we mov into it so movFRFromHW stores
    // directly to the frame.
    freeFRTemp(calleeFrameArg);
    auto calleeReg = getOrAllocFRInAnyReg(frCallee, true);
    movFRFromHW(
        calleeFrameArg, calleeReg, frameRegs_[frCallee.index()].localType);
  }

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

  syncToFrame(calleeFrameArg);
  syncToFrame(ntFrameArg);

  HWReg hwArgc = getOrAllocFRInVecD(frArgc, true);
  HWReg hwTemp = allocTempGpX(HWReg::gpX(2));
  freeReg(hwTemp);
  a.fcvtzu(hwTemp.a64GpX(), hwArgc.a64VecD());

  syncAllFRTempExcept({});
  freeAllFRTempExcept(frRes);

  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, xFrame);
  // The bytecode arg count includes "this", but the SH one does not, so
  // subtract 1.
  a.sub(a64::w2, hwTemp.a64GpX().w(), 1);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, uint32_t),
      _jit_dispatch_call);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getBuiltinClosure(FR frRes, uint32_t builtinIndex) {
  comment(
      "// GetBuiltinClosure r%u, %s",
      frRes.index(),
      getBuiltinMethodName(builtinIndex));
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept(frRes);

  a.mov(a64::x0, xRuntime);
  a.mov(a64::w1, builtinIndex);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, uint32_t),
      _sh_ljs_get_builtin_closure);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::arithUnop(
    bool forceNumber,
    FR frRes,
    FR frInput,
    const char *name,
    void (*fast)(
        a64::Assembler &a,
        const a64::VecD &dst,
        const a64::VecD &src,
        const a64::VecD &tmp),
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

  if (!inputIsNum) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frInput ? frRes : FR());
    syncToFrame(frInput);
  }

  if (inputIsNum) {
    hwInput = getOrAllocFRInVecD(frInput, true);
  } else {
    hwInput = getOrAllocFRInGpX(frInput, true);
    a.cmp(hwInput.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
    hwInput = getOrAllocFRInVecD(frInput, true);
  }

  hwRes = getOrAllocFRInVecD(frRes, false);
  HWReg hwTmp = hwRes != hwInput ? hwRes : allocTempVecD();
  fast(a, hwRes.a64VecD(), hwInput.a64VecD(), hwTmp.a64VecD());
  if (hwRes == hwInput)
    freeReg(hwTmp);

  frUpdatedWithHW(
      frRes, hwRes, inputIsNum ? FRType::Number : FRType::UnknownPtr);

  if (inputIsNum)
    return;

  freeAllFRTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::booleanNot(FR frRes, FR frInput) {
  comment("// Not r%u, r%u", frRes.index(), frInput.index());

  // TODO: Add a fast path, perhaps by sharing some code with JmpTrue.
  syncAndFreeTempReg(HWReg::gpX(0));
  movHWFromFR(HWReg::gpX(0), frInput);

  // Since we already loaded the input, no need to check for frRes == frInput.
  syncAllFRTempExcept(frRes);
  freeAllFRTempExcept({});
  EMIT_RUNTIME_CALL(*this, bool (*)(SHLegacyValue), _sh_ljs_to_boolean);

  HWReg hwRes = getOrAllocFRInGpX(frRes, false);
  // Negate the result.
  a.eor(hwRes.a64GpX(), a64::x0, 1);
  // Add the bool tag.
  emit_sh_ljs_bool(a, hwRes.a64GpX());
  frUpdatedWithHW(frRes, hwRes, FRType::Bool);
}

void Emitter::bitNot(FR frRes, FR frInput) {
  comment("// BitNot r%u, r%u", frRes.index(), frInput.index());

  HWReg hwTempGpX = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frInput ? frRes : FR());
  // TODO: As with binary bit ops, it should be possible to only do this in the
  // slow path.
  syncToFrame(frInput);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwInput = getOrAllocFRInVecD(frInput, true);
  emit_double_is_int(
      a, hwTempGpX.a64GpX(), hwTempVecD.a64VecD(), hwInput.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempGpX);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(
      frRes,
      hwRes,
      isFRKnownType(frInput, FRType::Number) ? FRType::Number
                                             : FRType::UnknownPtr);

  // Perform the negation and write it to the result.
  a.mvn(hwTempGpX.a64GpX().w(), hwTempGpX.a64GpX().w());
  a.scvtf(hwRes.a64VecD(), hwTempGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// bitNot r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(SHRuntime *, const SHLegacyValue *),
             _sh_ljs_bit_not_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::typeOf(FR frRes, FR frInput) {
  comment("// TypeOf r%u, r%u", frRes.index(), frInput.index());
  syncAllFRTempExcept(frRes == frInput ? FR() : frRes);
  syncToFrame(frInput);
  freeAllFRTempExcept(FR());

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frInput);
  // TODO: Use a function that preserves temporary registers.
  EMIT_RUNTIME_CALL(
      *this, SHLegacyValue(*)(SHRuntime *, SHLegacyValue *), _sh_ljs_typeof);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getPNameList(FR frRes, FR frObj, FR frIdx, FR frSize) {
  comment(
      "// GetPNameList r%u, r%u, r%u, r%u",
      frRes.index(),
      frObj.index(),
      frIdx.index(),
      frSize.index());
  syncAllFRTempExcept({});
  // We have to sync frObj to the frame since it is an in/out parameter.
  syncToFrame(frObj);
  // No need to sync frIdx and frSize since they are just out parameters.
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frObj);
  loadFrameAddr(a64::x2, frIdx);
  loadFrameAddr(a64::x3, frSize);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *, SHLegacyValue *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_get_pname_list_rjs);

  // Ensure that the out params have their frame location marked as up-to-date,
  // and any global register is updated.
  syncFrameOutParam(frObj);
  syncFrameOutParam(frIdx);
  syncFrameOutParam(frSize);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::getNextPName(
    FR frRes,
    FR frProps,
    FR frObj,
    FR frIdx,
    FR frSize) {
  comment(
      "// GetNextPName r%u, r%u, r%u, r%u, r%u",
      frRes.index(),
      frProps.index(),
      frObj.index(),
      frIdx.index(),
      frSize.index());

  syncAllFRTempExcept({});
  syncToFrame(frProps);
  syncToFrame(frObj);
  syncToFrame(frIdx);
  syncToFrame(frSize);
  freeAllFRTempExcept({});
  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frProps);
  loadFrameAddr(a64::x2, frObj);
  loadFrameAddr(a64::x3, frIdx);
  loadFrameAddr(a64::x4, frSize);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(
          SHRuntime *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHLegacyValue *,
          SHLegacyValue *),
      _sh_ljs_get_next_pname_rjs);

  // Ensure that the updated frame location is sync'd back to any global reg.
  syncFrameOutParam(frIdx);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
}

void Emitter::addS(FR frRes, FR frLeft, FR frRight) {
  comment(
      "// AddS r%u, r%u, r%u", frRes.index(), frLeft.index(), frRight.index());

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  syncToFrame(frLeft);
  syncToFrame(frRight);
  freeAllFRTempExcept({});

  a.mov(a64::x0, xRuntime);
  loadFrameAddr(a64::x1, frLeft);
  loadFrameAddr(a64::x2, frRight);
  EMIT_RUNTIME_CALL(
      *this,
      SHLegacyValue(*)(SHRuntime *, SHLegacyValue *, SHLegacyValue *),
      _sh_ljs_string_add);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  movHWFromHW<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);
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

  if (!leftIsNum) {
    hwLeft = getOrAllocFRInGpX(frLeft, true);
    a.cmp(hwLeft.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }
  if (!rightIsNum) {
    hwRight = getOrAllocFRInGpX(frRight, true);
    a.cmp(hwRight.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }

  // Make sure d0, d1 are unused.
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

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// mod r%u, r%u, r%u",
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         EMIT_RUNTIME_CALL(
             em,
             SHLegacyValue(*)(
                 SHRuntime *, const SHLegacyValue *, const SHLegacyValue *),
             _sh_ljs_mod_rjs);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::arithBinOp(
    bool forceNumber,
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    void (*fast)(
        a64::Assembler &a,
        const a64::VecD &res,
        const a64::VecD &dl,
        const a64::VecD &dr),
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

  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
    syncToFrame(frLeft);
    syncToFrame(frRight);
  }

  if (leftIsNum) {
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  } else {
    hwLeft = getOrAllocFRInGpX(frLeft, true);
    a.cmp(hwLeft.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }
  if (rightIsNum) {
    hwRight = getOrAllocFRInVecD(frRight, true);
  } else {
    hwRight = getOrAllocFRInGpX(frRight, true);
    a.cmp(hwRight.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }

  if (!leftIsNum)
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  if (!rightIsNum)
    hwRight = getOrAllocFRInVecD(frRight, true);

  hwRes = getOrAllocFRInVecD(frRes, false);
  fast(a, hwRes.a64VecD(), hwLeft.a64VecD(), hwRight.a64VecD());

  frUpdatedWithHW(frRes, hwRes, !slow ? FRType::Number : FRType::UnknownPtr);

  if (!slow)
    return;

  freeAllFRTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// %s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::bitBinOp(
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    SHLegacyValue (*slowCall)(
        SHRuntime *shr,
        const SHLegacyValue *a,
        const SHLegacyValue *b),
    const char *slowCallName,
    void (*fast)(
        a64::Assembler &a,
        const a64::GpX &res,
        const a64::GpX &dl,
        const a64::GpX &dr)) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frLeft.index(),
      frRight.index());

  HWReg hwTempLGpX = allocTempGpX();
  HWReg hwTempRGpX = allocTempGpX();
  HWReg hwTempLVecD = allocTempVecD();
  HWReg hwTempRVecD = allocTempVecD();

  syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
  // TODO: In principle, it should be possible to only sync these in the slow
  // path. If we do that, we have to ensure that the frameUpToDate bit is not
  // set, since subsequent instructions cannot rely on it. To do this, we would
  // need to preserve information for the slow path to know whether they were
  // already sync'd to memory.
  syncToFrame(frLeft);
  syncToFrame(frRight);

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  HWReg hwLeft = getOrAllocFRInVecD(frLeft, true);
  emit_double_is_int(
      a, hwTempLGpX.a64GpX(), hwTempLVecD.a64VecD(), hwLeft.a64VecD());
  a.b_ne(slowPathLab);

  // Do the same for the RHS.
  HWReg hwRight = getOrAllocFRInVecD(frRight, true);
  emit_double_is_int(
      a, hwTempRGpX.a64GpX(), hwTempRVecD.a64VecD(), hwRight.a64VecD());
  a.b_ne(slowPathLab);

  // Done allocating registers. Free them all and allocate the result.
  freeAllFRTempExcept({});
  freeReg(hwTempLGpX);
  freeReg(hwTempRGpX);
  freeReg(hwTempLVecD);
  freeReg(hwTempRVecD);
  HWReg hwRes = getOrAllocFRInVecD(frRes, false);
  frUpdatedWithHW(
      frRes,
      hwRes,
      isFRKnownNumber(frLeft) && isFRKnownNumber(frRight) ? FRType::Number
                                                          : FRType::UnknownPtr);

  // Invoke the fast path, and move the result back as a 32 bit integer.
  fast(a, hwTempLGpX.a64GpX(), hwTempLGpX.a64GpX(), hwTempRGpX.a64GpX());
  a.scvtf(hwRes.a64VecD(), hwTempLGpX.a64GpX().w());

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .slowCall = (void *)slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// %s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
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
    a.fcmp(hwInput.a64VecD(), 0.0);
    if (onTrue) {
      // Branch on < 0 and > 0. All that remains is 0 and NaN.
      a.b_mi(target);
      a.b_gt(target);
    } else {
      asmjit::Label label = a.newLabel();
      a.b_mi(label);
      a.b_gt(label);
      a.b(target);
      a.bind(label);
    }
  } else if (isFRKnownType(frInput, FRType::Bool)) {
    HWReg hwInput = getOrAllocFRInGpX(frInput, true);
    a64::GpX xInput = hwInput.a64GpX();

    static_assert(
        HERMESVALUE_VERSION == 1,
        "bool is encoded as 32-bit value in the low bits");
    if (onTrue)
      a.cbnz(xInput.w(), target);
    else
      a.cbz(xInput.w(), target);
  } else {
    // TODO: we should inline all of it.
    syncAllFRTempExcept({});
    movHWFromFR(HWReg::gpX(0), frInput);
    EMIT_RUNTIME_CALL(*this, bool (*)(SHLegacyValue), _sh_ljs_to_boolean);
    if (onTrue)
      a.cbnz(a64::w0, target);
    else
      a.cbz(a64::w0, target);
    freeAllFRTempExcept(FR());
  }
}

void Emitter::jmp(const asmjit::Label &target) {
  comment("// Jmp Lx");
  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());
  a.b(target);
}

void Emitter::jmpUndefined(const asmjit::Label &target, FR frInput) {
  comment("// JmpUndefined r%u", frInput.index());

  // Do this always, since this could be the end of the BB.
  syncAllFRTempExcept(FR());
  freeAllFRTempExcept(FR());

  if (isFRKnownType(frInput, FRType::Number) ||
      isFRKnownType(frInput, FRType::Bool)) {
    return;
  }

  HWReg hwInput = getOrAllocFRInGpX(frInput, true);
  a64::GpX xInput = hwInput.a64GpX();
  HWReg hwTmpTag = allocTempGpX();
  a64::GpX xTmpTag = hwTmpTag.a64GpX();

  emit_sh_ljs_is_undefined(a, xTmpTag, xInput);
  a.b_eq(target);

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
    void(fast)(a64::Assembler &a, const asmjit::Label &target),
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

  if (leftIsNum) {
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  } else {
    hwLeft = getOrAllocFRInGpX(frLeft, true);
    a.cmp(hwLeft.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }
  if (rightIsNum) {
    hwRight = getOrAllocFRInVecD(frRight, true);
  } else {
    hwRight = getOrAllocFRInGpX(frRight, true);
    a.cmp(hwRight.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }

  if (!leftIsNum)
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  if (!rightIsNum)
    hwRight = getOrAllocFRInVecD(frRight, true);

  a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());
  if (!invert) {
    fast(a, target);
  } else {
    if (!contLab.isValid())
      contLab = a.newLabel();
    fast(a, contLab);
    a.b(target);
  }
  if (contLab.isValid())
    a.bind(contLab);

  if (!slow)
    return;

  // Do this always, since this is the end of the BB.
  freeAllFRTempExcept(FR());

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .target = target,
       .name = name,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .invert = invert,
       .passArgsByVal = passArgsByVal,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: j_%s%s Lx, r%u, r%u",
             sl.invert ? "not_" : "",
             sl.name,
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         if (sl.passArgsByVal) {
           em._loadFrame(HWReg::gpX(0), sl.frInput1);
           em._loadFrame(HWReg::gpX(1), sl.frInput2);
         } else {
           em.a.mov(a64::x0, xRuntime);
           em.loadFrameAddr(a64::x1, sl.frInput1);
           em.loadFrameAddr(a64::x2, sl.frInput2);
         }
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         if (!sl.invert)
           em.a.cbnz(a64::w0, sl.target);
         else
           em.a.cbz(a64::w0, sl.target);
         em.a.b(sl.contLab);
       }});
}

void Emitter::compareImpl(
    FR frRes,
    FR frLeft,
    FR frRight,
    const char *name,
    a64::CondCode condCode,
    void *slowCall,
    const char *slowCallName,
    bool invSlow,
    bool passArgsByVal) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frLeft.index(),
      frRight.index());
  HWReg hwLeft, hwRight;
  asmjit::Label slowPathLab;
  asmjit::Label contLab;
  bool leftIsNum, rightIsNum, slow;

  leftIsNum = isFRKnownNumber(frLeft);
  rightIsNum = isFRKnownNumber(frRight);
  slow = !(rightIsNum && leftIsNum);

  if (slow) {
    slowPathLab = newSlowPathLabel();
    contLab = newContLabel();
    syncAllFRTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
    syncToFrame(frLeft);
    syncToFrame(frRight);
  }

  if (leftIsNum) {
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  } else {
    hwLeft = getOrAllocFRInGpX(frLeft, true);
    a.cmp(hwLeft.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }
  if (rightIsNum) {
    hwRight = getOrAllocFRInVecD(frRight, true);
  } else {
    hwRight = getOrAllocFRInGpX(frRight, true);
    a.cmp(hwRight.a64GpX(), xDoubleLim);
    a.b_hs(slowPathLab);
  }

  if (!leftIsNum)
    hwLeft = getOrAllocFRInVecD(frLeft, true);
  if (!rightIsNum)
    hwRight = getOrAllocFRInVecD(frRight, true);
  if (slow)
    freeAllFRTempExcept({});

  HWReg hwRes = slow ? getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0))
                     : getOrAllocFRInGpX(frRes, false);
  a64::GpX xRes = hwRes.a64GpX();

  a.fcmp(hwLeft.a64VecD(), hwRight.a64VecD());
  // Store the result of the comparison in the lowest bit of tmpCmpRes.
  // asmjit will convert CondCode to the correct encoding for use in the opcode.
  a.cset(xRes, condCode);

  // Encode bool.
  emit_sh_ljs_bool(a, xRes);
  frUpdatedWithHW(frRes, hwRes, FRType::Bool);

  if (!slow)
    return;

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .hwRes = hwRes,
       .invert = invSlow,
       .passArgsByVal = passArgsByVal,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: j_%s r%u, r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         if (sl.passArgsByVal) {
           em._loadFrame(HWReg::gpX(0), sl.frInput1);
           em._loadFrame(HWReg::gpX(1), sl.frInput2);
         } else {
           em.a.mov(a64::x0, xRuntime);
           em.loadFrameAddr(a64::x1, sl.frInput1);
           em.loadFrameAddr(a64::x2, sl.frInput2);
         }
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);

         // Invert the slow path result if needed.
         if (sl.invert)
           em.a.eor(sl.hwRes.a64GpX(), a64::x0, 1);
         else
           em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));

         // Comparison functions return bool, so encode it.
         emit_sh_ljs_bool(em.a, sl.hwRes.a64GpX());
         em.a.b(sl.contLab);
       }});
}

void Emitter::getArgumentsPropByValImpl(
    FR frRes,
    FR frIndex,
    FR frLazyReg,
    const char *name,
    SHLegacyValue (*shImpl)(
        SHRuntime *shr,
        SHLegacyValue *frame,
        SHLegacyValue *idx,
        SHLegacyValue *lazyReg),
    const char *shImplName) {
  comment(
      "// %s r%u, r%u, r%u",
      name,
      frRes.index(),
      frIndex.index(),
      frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept(frRes != frIndex && frRes != frLazyReg ? frRes : FR());
  syncToFrame(frIndex);
  syncToFrame(frLazyReg);
  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwIndex = getOrAllocFRInVecD(frIndex, true);
  HWReg hwTempIndex = allocTempGpX();
  a64::GpW wTempIndex = hwTempIndex.a64GpX().w();
  HWReg hwTempArgCount = allocTempGpX();
  HWReg hwTempVecD = allocTempVecD();
  freeAllFRTempExcept({});
  freeReg(hwTempIndex);
  freeReg(hwTempArgCount);
  freeReg(hwTempVecD);
  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false, HWReg::gpX(0));
  frUpdatedWithHW(frRes, hwRes);

  // If lazyReg is an object, go to slow path.
  emit_sh_ljs_is_object(a, hwTempIndex.a64GpX(), hwLazyReg.a64GpX());
  a.b_eq(slowPathLab);

  // If index is not an array index, go to slow path.
  emit_double_is_int(
      a, hwTempIndex.a64GpX(), hwTempVecD.a64VecD(), hwIndex.a64VecD());
  a.b_ne(slowPathLab);

  // If index >= arg count or index < 0, go to slow path.
  // Use an unsigned comparison to handle the negative index case.
  a.ldur(
      hwTempArgCount.a64GpX().w(),
      a64::Mem(
          xFrame,
          (int)StackFrameLayout::ArgCount * (int)sizeof(SHLegacyValue)));
  a.cmp(hwTempIndex.a64GpX(), hwTempArgCount.a64GpX());
  a.b_hs(slowPathLab);

  // Load the argument from the stack.
  // We want framePtr[(firstArg - index) * 8].
  // Use shift SXTW to shift the signed w register by 3.
  a.mov(hwTempArgCount.a64GpX().w(), (int)StackFrameLayout::FirstArg);
  a.sub(wTempIndex, hwTempArgCount.a64GpX().w(), wTempIndex);
  a.ldr(
      hwRes.a64GpX(),
      a64::Mem(xFrame, wTempIndex, a64::Shift(a64::ShiftOp::kSXTW, 3)));

  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frRes = frRes,
       .frInput1 = frIndex,
       .frInput2 = frLazyReg,
       .hwRes = hwRes,
       .slowCall = (void *)shImpl,
       .slowCallName = shImplName,
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: %s r%u", sl.name, sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         em.loadFrameAddr(a64::x3, sl.frInput2);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         em.movHWFromHW<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::reifyArgumentsImpl(FR frLazyReg, bool strict, const char *name) {
  comment("// %s r%u", name, frLazyReg.index());

  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();

  syncAllFRTempExcept({});
  syncToFrame(frLazyReg);

  HWReg hwLazyReg = getOrAllocFRInGpX(frLazyReg, true);
  HWReg hwTemp = allocTempGpX();
  freeAllFRTempExcept({});
  freeReg(hwTemp);

  emit_sh_ljs_is_object(a, hwTemp.a64GpX(), hwLazyReg.a64GpX());
  // If the lazyReg is not an object, it needs to be reified, go to slow path.
  a.b_ne(slowPathLab);

  // Fast path: do nothing.
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = name,
       .frInput1 = frLazyReg,
       // Use hwRes field to pass the global reg for the in/out param.
       .hwRes = frameRegs_[frLazyReg.index()].globalReg,
       .slowCall = strict ? (void *)_sh_ljs_reify_arguments_strict
                          : (void *)_sh_ljs_reify_arguments_loose,
       .slowCallName = strict ? "_sh_ljs_reify_arguments_strict"
                              : "_sh_ljs_reify_arguments_loose",
       .emittingIP = emittingIP,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment("// Slow path: %s r%u", sl.name, sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.a.mov(a64::x1, xFrame);
         em.loadFrameAddr(a64::x2, sl.frInput1);
         em.callThunkWithSavedIP(sl.slowCall, sl.slowCallName);
         // Slow path modifies the frame so we need to sync it if there's a
         // global reg.
         if (sl.hwRes.isValid()) {
           em._loadFrame(sl.hwRes, sl.frInput1);
         }
         em.a.b(sl.contLab);
       }});
}

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
