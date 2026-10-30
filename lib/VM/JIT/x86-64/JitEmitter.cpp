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
#include "JitImpl.h"

#include "hermes/Support/ErrorHandling.h"

#include <cstdio>

#if defined(HERMESVM_COMPRESSED_POINTERS) && !defined(HERMESVM_CONTIGUOUS_HEAP)
#error JIT does not support non-contiguous heap with compressed pointers
#endif

namespace hermes::vm::x86_64 {

// Disable warnings about missing designated initializers, which the
// roDataDesc_ entries below rely on.
#ifdef __clang__
#if __has_warning("-Wmissing-designated-field-initializers")
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
#endif
#endif

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const hermes::vm::x86_64::HWReg &hwReg) {
  if (hwReg.isValidGpX()) {
    os << "r" << (int)hwReg.indexInClass();
  } else if (hwReg.isValidVecD()) {
    os << "xmm" << (int)hwReg.indexInClass();
  } else {
    assert(!hwReg.isValid());
    os << "<invalid>";
  }
  return os;
}

Emitter::Emitter(
    Runtime &runtime,
    JITContext::Impl &jitImpl,
    unsigned dumpJitCode,
    bool emitAsserts,
    bool emitTypeAsserts,
    bool emitCounters,
    PerfJitDump *perfJitDump,
    CodeBlock *codeBlock,
    const std::function<void(std::string &&message)> &longjmpError)
    : runtime_(runtime),
      jitImpl_(jitImpl),
      dumpJitCode_(dumpJitCode),
      emitAsserts_(emitAsserts),
      emitTypeAsserts_(emitTypeAsserts),
      emitCounters_(emitCounters),
      frameRegs_(codeBlock->getFrameSize()),
      codeBlock_(codeBlock) {
  errorHandler_ = std::unique_ptr<asmjit::ErrorHandler>(
      new OurErrorHandler(expectedError_, longjmpError));

  code.init(jitImpl.jr.environment(), jitImpl.jr.cpuFeatures());
  code.setErrorHandler(errorHandler_.get());

#ifndef ASMJIT_NO_LOGGING
  if ((dumpJitCode_ & DumpJitCode::Code) || perfJitDump) {
    logger_ = std::unique_ptr<asmjit::Logger>(
        new OurLogger(a, perfJitDump, dumpJitCode_));
    logger_->setIndentation(asmjit::FormatIndentationGroup::kCode, 4);
    logger_->addFlags(asmjit::FormatFlags::kHexImms);
    code.setLogger(logger_.get());
  }
#endif

  code.attach(&a);

  roDataLabel_ = a.newNamedLabel("RO_DATA");
  returnLabel_ = a.newNamedLabel("leave");

  // Save read/write property cache addresses.
  roOfsReadPropertyCachePtr_ = uint64Const(
      (uint64_t)codeBlock->readPropertyCache(), "readPropertyCache");
  roOfsWritePropertyCachePtr_ = uint64Const(
      (uint64_t)codeBlock->writePropertyCache(), "writePropertyCache");
  roOfsPrivateNameCachePtr_ =
      uint64Const((uint64_t)codeBlock->privateNameCache(), "privateNameCache");
}

void Emitter::commentV(const char *fmt, va_list args) {
  char buf[80];
  vsnprintf(buf, sizeof(buf), fmt, args);
  a.comment(buf);
}

JITCompiledFunctionPtr Emitter::addToRuntime(asmjit::JitRuntime &jr) {
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
  for (unsigned i = kGPTemp1.first; i <= kGPTemp2.second; ++i) {
    if (i > kGPTemp1.second && i < kGPTemp2.first)
      continue;
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(gpTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }

  for (unsigned i = kVecTemp.first; i <= kVecTemp.second; ++i) {
    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid()) {
      assert(vecTemp_.isAllocated(i) && "Temp register is not freed");
    }
  }
}
#endif

void Emitter::enter(uint32_t numCount, uint32_t npCount) {
  // Task 2: the x86-64 backend has no real substrate yet. Decline every
  // function unconditionally; JITContext::Compiler catches this and falls
  // back to the interpreter. Task 3 replaces this with the real prologue.
  unsupported("enter");
}

void Emitter::leave(llvh::ArrayRef<const asmjit::Label *> exceptionHandlers) {
  // Unreachable in Task 2: enter() always calls unsupported(), which aborts
  // compilation (via longjmp) before compileCodeBlockImpl() can reach the
  // point where leave() would be called. Task 3 ports the arm64 epilogue
  // logic here once enter() can succeed.
  hermes::hermes_fatal("x86-64 jit: leave() not yet implemented");
}

void Emitter::newBasicBlock(const asmjit::Label &label) {
  // Unreachable in Task 2, for the same reason as leave() above.
  hermes::hermes_fatal("x86-64 jit: newBasicBlock() not yet implemented");
}

asmjit::Label Emitter::newPrefLabel(const char *pref, size_t index) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%s%lu", pref, index);
  return a.newNamedLabel(buf);
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
  if (hasLogger()) {
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

void Emitter::emitROData() {
  a.bind(roDataLabel_);
  if (!hasLogger()) {
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

void Emitter::unsupported(const char *what) {
  char buf[128];
  snprintf(buf, sizeof(buf), "unsupported instruction: %s", what);
  // Route through the same expected-error / longjmp mechanism as a genuine
  // AsmJit error: reportError() calls OurErrorHandler::handleError(), which
  // (since this error code never matches expectedError_) formats the
  // message and invokes the longjmp callback passed to the constructor,
  // unwinding to JITContext::Compiler::compileCodeBlock() and abandoning
  // compilation of this function.
  a.reportError(asmjit::kErrorInvalidState, buf);
  hermes::hermes_fatal("jit: unsupported() unexpectedly returned");
}

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
