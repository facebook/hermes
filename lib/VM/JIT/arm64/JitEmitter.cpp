/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitEmitter.h"

#include "hermes/Support/ErrorHandling.h"

namespace hermes::vm::arm64 {

namespace {

class OurErrorHandler : public asmjit::ErrorHandler {
  void handleError(
      asmjit::Error err,
      const char *message,
      asmjit::BaseEmitter *origin) override {
    llvh::errs() << "AsmJit error: " << err << ": "
                 << asmjit::DebugUtils::errorAsString(err) << ": " << message
                 << "\n";
    hermes_fatal("AsmJit error");
  }
};

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

#define EMIT_RUNTIME_CALL(em, func) (em).call((void *)func, #func)

Emitter::Emitter(
    asmjit::JitRuntime &jitRT,
    bool dumpJitCode,
    uint32_t numFrameRegs,
    unsigned numCount,
    unsigned npCount)
    : frameRegs_(numFrameRegs) {
  if (dumpJitCode)
    logger_ = std::unique_ptr<asmjit::Logger>(new OurLogger());
  if (logger_)
    logger_->setIndentation(asmjit::FormatIndentationGroup::kCode, 4);

  errorHandler_ = std::unique_ptr<asmjit::ErrorHandler>(new OurErrorHandler());

  code.init(jitRT.environment(), jitRT.cpuFeatures());
  code.setErrorHandler(errorHandler_.get());
  if (logger_)
    code.setLogger(logger_.get());
  code.attach(&a);

  roDataLabel_ = a.newNamedLabel("RO_DATA");
  returnLabel_ = a.newNamedLabel("leave");

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
    frameRegs_[frIndex].globalType = FRType::Unknown;
  }

  frameSetup(numFrameRegs, nextGp - kGPSaved.first, nextVec - kVecSaved.first);
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

void Emitter::newBasicBlock(const asmjit::Label &label) {
  syncAllTempExcept({});
  freeAllTempExcept({});

  // Clear all local types and regs when starting a new basic block.
  // TODO: there must be a faster way to do this when there are many regs.
  for (FRState &frState : frameRegs_) {
    frState.localType = frState.globalType;
    assert(!frState.localGpX);
    assert(!frState.localVecD);
  }

  a.bind(label);
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
      a.stp(a64::VecD(16 + i), a64::VecD(17 + i), a64::Mem(a64::sp, stackOfs));
    else
      a.str(a64::VecD(16 + i), a64::Mem(a64::sp, stackOfs));
  }
  a.stp(a64::x29, a64::x30, a64::Mem(a64::sp, stackOfs));
  a.add(a64::x29, a64::sp, stackOfs);

  // ((uint64_t)HVTag_First << kHV_NumDataBits)
  comment("// xDoubleLim");
  a.mov(xDoubleLim, ((uint64_t)HVTag_First << kHV_NumDataBits));

  comment("// xRuntime");
  a.mov(xRuntime, a64::x0);

  //  _sh_check_native_stack_overflow(shr);
  EMIT_RUNTIME_CALL(*this, _sh_check_native_stack_overflow);

  // Function<bench>(3 params, 13 registers):
  //  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 13);
  comment("// _sh_enter");
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  a.mov(a64::w2, numFrameRegs);
  EMIT_RUNTIME_CALL(*this, _sh_enter);
  comment("// xFrame");
  a.mov(xFrame, a64::x0);

  //  locals.head.count = 0;
  comment("// locals.head.count = 0");
  a.mov(a64::w1, 0);
  a.str(a64::w1, a64::Mem(a64::sp, offsetof(SHLocals, count)));
}

void Emitter::leave() {
  comment("// leaveFrame");
  a.bind(returnLabel_);
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  a.mov(a64::x2, xFrame);
  EMIT_RUNTIME_CALL(*this, _sh_leave);

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

void Emitter::call(void *fn, const char *name) {
  //    comment("// call %s", name);
  a.bl(registerCall(fn, name));
}

void Emitter::loadFrameAddr(a64::GpX dst, FR frameReg) {
  // FIXME: check range of frameReg * 8
  if (frameReg == FR(0))
    a.mov(dst, xFrame);
  else
    a.add(dst, xFrame, frameReg.index() * sizeof(SHLegacyValue));
}

template <bool use>
void Emitter::movHWReg(HWReg dst, HWReg src) {
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

void Emitter::storeHWRegToFrame(FR fr, HWReg src) {
  if (src.isVecD())
    storeFrame(src.a64VecD(), fr);
  else
    storeFrame(src.a64GpX(), fr);
  frameRegs_[fr.index()].frameUpToDate = true;
}

void Emitter::movHWFromFR(HWReg hwRes, FR src) {
  FRState &frState = frameRegs_[src.index()];
  if (frState.localGpX)
    movHWReg<true>(hwRes, frState.localGpX);
  else if (frState.localVecD)
    movHWReg<true>(hwRes, frState.localVecD);
  else if (frState.globalReg && frState.globalRegUpToDate)
    movHWReg<true>(hwRes, frState.globalReg);
  else
    loadFrame(a64::GpX(useReg(hwRes).indexInClass()), src);
}

template <class TAG>
HWReg Emitter::_allocTemp(TempRegAlloc &ra) {
  if (auto optReg = ra.alloc(); optReg)
    return HWReg(*optReg, TAG{});
  // Spill one register.
  unsigned index = ra.leastRecentlyUsed();
  spillTempReg(HWReg(index, TAG{}));
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
    comment("    ; free x%u (r%u)", hwReg.indexInClass(), fr.index());
    if (fr.isValid()) {
      assert(frameRegs_[fr.index()].localGpX == hwReg);
      frameRegs_[fr.index()].localGpX = {};
    }
    if (isTempGpX(hwReg))
      gpTemp_.free(hwReg.indexInClass());
  } else {
    comment("    ; free d%u (r%u)", hwReg.indexInClass(), fr.index());
    if (fr.isValid()) {
      assert(frameRegs_[fr.index()].localVecD == hwReg);
      frameRegs_[fr.index()].localVecD = {};
    }
    if (isTempVecD(hwReg))
      vecTemp_.free(hwReg.indexInClass());
  }
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

void Emitter::spillTempReg(HWReg toSpill) {
  assert(isTemp(toSpill));

  HWRegState &hwState = hwRegs_[toSpill.combinedIndex()];
  FR fr = hwState.contains;
  hwState.contains = {};
  assert(fr.isValid() && "Allocated tmp register is unused");

  FRState &frState = frameRegs_[fr.index()];

  assert(frState.globalReg != toSpill && "global regs can't be temporary");
  if (frState.globalReg) {
    if (!frState.globalRegUpToDate) {
      movHWReg<false>(frState.globalReg, toSpill);
      frState.globalRegUpToDate = true;
    }
  } else {
    if (!frState.frameUpToDate) {
      storeHWRegToFrame(fr, toSpill);
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

void Emitter::syncToMem(FR fr) {
  FRState &frState = frameRegs_[fr.index()];
  if (frState.frameUpToDate)
    return;

  HWReg hwReg = isFRInRegister(fr);
  assert(
      hwReg.isValid() && "FR is not synced to frame and is not in a register");
  storeHWRegToFrame(fr, hwReg);
}

void Emitter::syncAllTempExcept(FR exceptFR) {
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
        movHWReg<false>(frState.globalReg, hwReg);
        frState.globalRegUpToDate = true;
      }
    } else {
      if (!frState.frameUpToDate) {
        comment("    ; sync: x%u (r%u)", i, fr.index());
        storeHWRegToFrame(fr, hwReg);
      }
    }
  }

  for (unsigned i = kVecTemp.first; i <= kVecTemp.second; ++i) {
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
        movHWReg<false>(frState.globalReg, hwReg);
        frState.globalRegUpToDate = true;
      }
    } else {
      if (!frState.frameUpToDate) {
        comment("    ; sync d%u (r%u)", i, fr.index());
        storeHWRegToFrame(fr, hwReg);
      }
    }
  }
}

void Emitter::freeAllTempExcept(FR exceptFR) {
  for (unsigned i = kGPTemp.first; i <= kGPTemp.second; ++i) {
    HWReg hwReg(i, HWReg::GpX{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    comment("    ; free: x%u (r%u)", i, fr.index());
    hwRegs_[hwReg.combinedIndex()].contains = {};

    FRState &frState = frameRegs_[fr.index()];
    assert(frState.localGpX == hwReg && "tmpreg not bound to FR localreg");
    frState.localGpX = {};
    gpTemp_.free(hwReg.indexInClass());
  }

  for (unsigned i = kVecTemp.first; i <= kVecTemp.second; ++i) {
    HWReg hwReg(i, HWReg::VecD{});
    FR fr = hwRegs_[hwReg.combinedIndex()].contains;
    if (!fr.isValid() || fr == exceptFR)
      continue;
    comment("    ; free: d%u (r%u)", i, fr.index());
    hwRegs_[hwReg.combinedIndex()].contains = {};

    FRState &frState = frameRegs_[fr.index()];
    assert(frState.localVecD == hwReg && "tmpreg not bound to FR localreg");
    frState.localVecD = {};
    vecTemp_.free(hwReg.indexInClass());
    assert(!frState.localGpX && "We already spilled all GpX temps");
  }
}

void Emitter::assignAllocatedLocalHWReg(FR fr, HWReg hwReg) {
  hwRegs_[hwReg.combinedIndex()].contains = fr;
  if (hwReg.isGpX()) {
    comment("    ; alloc: x%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localGpX = hwReg;
  } else {
    comment("    ; alloc: d%u <- r%u", hwReg.indexInClass(), fr.index());
    frameRegs_[fr.index()].localVecD = hwReg;
  }
}

HWReg Emitter::isFRInRegister(FR fr) {
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

  if (frState.localVecD)
    return useReg(frState.localVecD);

  // Do we have a global VecD allocated to this FR?
  if (frState.globalReg.isValidVecD()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localGpX &&
          "If globalReg is not up to date, there must be a localReg");
      movHWReg<true>(frState.globalReg, frState.localGpX);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local VecD, so we must allocate a new tmp reg.
  HWReg hwVecD = allocTempVecD();
  assignAllocatedLocalHWReg(fr, hwVecD);

  if (load) {
    if (frState.localGpX) {
      movHWReg<false>(hwVecD, frState.localGpX);
    } else if (frState.globalReg.isValidGpX()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWReg<false>(hwVecD, frState.globalReg);
    } else {
      loadFrame(hwVecD.a64VecD(), fr);
      frState.frameUpToDate = true;
    }
  }

  return hwVecD;
}

HWReg Emitter::getOrAllocFRInGpX(FR fr, bool load) {
  auto &frState = frameRegs_[fr.index()];

  if (frState.localGpX)
    return useReg(frState.localGpX);

  // Do we have a global GpX allocated to this FR?
  if (frState.globalReg.isValidGpX()) {
    // If the caller requires that the latest value is present, but it isn't,
    // we need to put it there.
    if (load && !frState.globalRegUpToDate) {
      assert(
          frState.localVecD &&
          "If globalReg is not up to date, there must be a localReg");
      movHWReg<true>(frState.globalReg, frState.localVecD);
      frState.globalRegUpToDate = true;
    }

    return frState.globalReg;
  }

  // We have neither global nor local GpX, so we must allocate a new tmp reg.
  HWReg hwGpX = allocTempGpX();
  assignAllocatedLocalHWReg(fr, hwGpX);

  if (load) {
    if (frState.localVecD) {
      movHWReg<false>(hwGpX, frState.localVecD);
    } else if (frState.globalReg.isValidVecD()) {
      assert(
          frState.globalRegUpToDate &&
          "globalReg must be up to date if no local regs");
      movHWReg<false>(hwGpX, frState.globalReg);
    } else {
      loadFrame(hwGpX.a64GpX(), fr);
      frState.frameUpToDate = true;
    }
  }

  return hwGpX;
}

HWReg Emitter::getOrAllocFRInAnyReg(FR fr, bool load) {
  if (HWReg tmp = isFRInRegister(fr))
    return tmp;

  // We have neither global nor local reg, so we must allocate a new tmp reg.
  HWReg hwGpX = allocTempGpX();
  assignAllocatedLocalHWReg(fr, hwGpX);

  if (load) {
    loadFrame(hwGpX.a64GpX(), fr);
    frameRegs_[fr.index()].frameUpToDate = true;
  }

  return hwGpX;
}

void Emitter::frUpdatedWithHWReg(
    FR fr,
    HWReg hwReg,
    hermes::OptValue<FRType> localType) {
  FRState &frState = frameRegs_[fr.index()];

  frState.frameUpToDate = false;

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
  if (localType)
    frState.localType = *localType;
}

void Emitter::ret(FR frValue) {
  if (HWReg hwReg = isFRInRegister(frValue))
    movHWReg<false>(HWReg::gpX(22), hwReg);
  else
    loadFrame(a64::x22, frValue);
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
  movHWReg<false>(hwDest, hwInput);
  frUpdatedWithHWReg(frRes, hwDest, frameRegs_[frInput.index()].localType);
}

void Emitter::loadParam(FR frRes, uint32_t paramIndex) {
  comment("// LoadParam r%u, %u", frRes.index(), paramIndex);
  syncAllTempExcept(frRes);
  freeAllTempExcept(frRes);
  a.mov(a64::x0, xFrame);
  a.mov(a64::w1, paramIndex);
  EMIT_RUNTIME_CALL(*this, _sh_ljs_param);

  HWReg hwRes = getOrAllocFRInAnyReg(frRes, false);
  movHWReg<false>(hwRes, HWReg::gpX(0));
  frUpdatedWithHWReg(frRes, hwRes);
}

void Emitter::loadConstUInt8(FR frRes, uint8_t val) {
  comment("// LoadConstUInt8 r%u, %u", frRes.index(), val);
  HWReg hwRes{};

  if (val == 0) {
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
  frUpdatedWithHWReg(frRes, hwRes, FRType::Number);
}

void Emitter::toNumber(FR frRes, FR frInput) {
  comment("// %s r%u, r%u", "toNumber", frRes.index(), frInput.index());
  if (isFRKnownNumber(frInput))
    return mov(frRes, frInput, false);

  HWReg hwRes, hwInput;
  asmjit::Label slowPathLab = newSlowPathLabel();
  asmjit::Label contLab = newContLabel();
  syncAllTempExcept(frRes != frInput ? frRes : FR());
  syncToMem(frInput);

  hwInput = getOrAllocFRInGpX(frInput, true);
  a.cmp(hwInput.a64GpX(), xDoubleLim);
  a.b_hs(slowPathLab);

  if (frRes != frInput) {
    hwRes = getOrAllocFRInVecD(frRes, false);
    movHWReg<false>(hwRes, hwInput);
  } else {
    hwRes = hwInput;
  }
  frUpdatedWithHWReg(frRes, hwRes, FRType::Number);

  freeAllTempExcept(frRes);
  a.bind(contLab);

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .name = "toNumber",
       .frRes = frRes,
       .frInput1 = frInput,
       .hwRes = hwRes,
       .slowCall = (void *)_sh_ljs_to_double_rjs,
       .slowCallName = "_sh_ljs_to_double_rjs",
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.call(sl.slowCall, sl.slowCallName);
         em.movHWReg<false>(sl.hwRes, HWReg::vecD(0));
         em.a.b(sl.contLab);
       }});
}

asmjit::Label Emitter::newPrefLabel(const char *pref, size_t index) {
  char buf[8];
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

asmjit::Label Emitter::registerCall(void *fn, const char *name) {
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
    sp.emit(*this, sp);
    slowPaths_.pop_front();
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
    syncAllTempExcept(frRes != frInput ? frRes : FR());
    syncToMem(frInput);
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

  frUpdatedWithHWReg(
      frRes, hwRes, inputIsNum ? OptValue(FRType::Number) : OptValue<FRType>());

  if (inputIsNum)
    return;

  freeAllTempExcept(frRes);
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
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: %s r%u, r%u",
             sl.name,
             sl.frRes.index(),
             sl.frInput1.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.call(sl.slowCall, sl.slowCallName);
         em.movHWReg<false>(sl.hwRes, HWReg::gpX(0));
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
    syncAllTempExcept(frRes != frLeft && frRes != frRight ? frRes : FR());
    syncToMem(frLeft);
    syncToMem(frRight);
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

  frUpdatedWithHWReg(
      frRes, hwRes, !slow ? OptValue(FRType::Number) : OptValue<FRType>());

  if (!slow)
    return;

  freeAllTempExcept(frRes);
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
         em.call(sl.slowCall, sl.slowCallName);
         em.movHWReg<false>(sl.hwRes, HWReg::gpX(0));
         em.a.b(sl.contLab);
       }});
}

void Emitter::jCond(
    bool forceNumber,
    bool invert,
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
  }
  // Do this always, since this could be the end of the BB.
  syncAllTempExcept(FR());

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
  freeAllTempExcept(FR());

  slowPaths_.push_back(
      {.slowPathLab = slowPathLab,
       .contLab = contLab,
       .target = target,
       .name = name,
       .frInput1 = frLeft,
       .frInput2 = frRight,
       .invert = invert,
       .slowCall = slowCall,
       .slowCallName = slowCallName,
       .emit = [](Emitter &em, SlowPath &sl) {
         em.comment(
             "// Slow path: j_%s%s Lx, r%u, r%u",
             sl.invert ? "not_" : "",
             sl.name,
             sl.frInput1.index(),
             sl.frInput2.index());
         em.a.bind(sl.slowPathLab);
         em.a.mov(a64::x0, xRuntime);
         em.loadFrameAddr(a64::x1, sl.frInput1);
         em.loadFrameAddr(a64::x2, sl.frInput2);
         em.call(sl.slowCall, sl.slowCallName);
         if (!sl.invert)
           em.a.cbnz(a64::w0, sl.target);
         else
           em.a.cbz(a64::w0, sl.target);
         em.a.b(sl.contLab);
       }});
}

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
