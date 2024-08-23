/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "loop1-jit.h"

#include "asmjit/a64.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/static_h.h"

#include "llvh/Support/raw_ostream.h"

#include <deque>
#include <functional>

namespace a64 = asmjit::a64;

static asmjit::JitRuntime s_jitRT;

// x19 is runtime
static constexpr auto xRuntime = a64::x19;
// x20 is frame
static constexpr auto xFrame = a64::x20;
// x0 < xDoubleLim means that it is a double.
//    cmp   x0, xDoubleLim
//    b.hs  slowPath
static constexpr auto xDoubleLim = a64::x21;

class ErrorHandler : public asmjit::ErrorHandler {
  virtual void handleError(
      asmjit::Error err,
      const char *message,
      asmjit::BaseEmitter *origin) override {
    llvh::errs() << "AsmJit error: " << err << ": "
                 << asmjit::DebugUtils::errorAsString(err) << ": " << message
                 << "\n";
    hermes::hermes_fatal("AsmJit error");
  }
};

#define EMIT_RUNTIME_CALL(em, func) (em).call(offsetof(SHRuntime, func), #func)

class Emitter {
  std::unique_ptr<asmjit::FileLogger> fileLogger_{};
  asmjit::FileLogger *logger_ = nullptr;
  ErrorHandler errorHandler_;
  std::deque<std::function<void()>> slowPaths_{};

 public:
  asmjit::CodeHolder code{};
  a64::Assembler a{};

  explicit Emitter(asmjit::JitRuntime &jitRT) {
#ifndef NDEBUG
    fileLogger_ = std::make_unique<asmjit::FileLogger>(stdout);
#endif
    if (fileLogger_)
      logger_ = fileLogger_.get();

    if (logger_)
      logger_->setIndentation(asmjit::FormatIndentationGroup::kCode, 4);

    code.init(jitRT.environment(), jitRT.cpuFeatures());
    code.setErrorHandler(&errorHandler_);
    if (logger_)
      code.setLogger(logger_);
    code.attach(&a);
  }

  /// Log a comment.
  /// Annotated with printf-style format.
  void comment(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
    if (!logger_)
      return;
    va_list args;
    va_start(args, fmt);
    char buf[80];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    a.comment(buf);
  }

  JitFn addToRuntime() {
    emitSlowPaths();

    code.detach(&a);
    JitFn fn;
    asmjit::Error err = s_jitRT.add(&fn, &code);
    if (err) {
      llvh::errs() << "AsmJit failed: "
                   << asmjit::DebugUtils::errorAsString(err) << "\n";
      hermes::hermes_fatal("AsmJit failed");
    }
    return fn;
  }

  void frameSetup() {
    a.sub(a64::sp, a64::sp, 10 * 8);
    //  0-3: SHLocals
    //  4: x22
    //  5: x21
    //  6: x20
    //  7: x19
    //  8: x29 <- new x29 points here
    //  9: x30

    a.stp(a64::x22, a64::x21, a64::Mem(a64::sp, 4 * 8));
    a.stp(a64::x20, a64::x19, a64::Mem(a64::sp, 6 * 8));
    a.stp(a64::x29, a64::x30, a64::Mem(a64::sp, 8 * 8));
    a.add(a64::x29, a64::sp, 8 * 8);

    // ((uint64_t)HVTag_First << kHV_NumDataBits)
    comment("// xDoubleLim");
    a.mov(xDoubleLim, ((uint64_t)HVTag_First << kHV_NumDataBits));
  }

  void leaveFrame() {
    comment("// leaveFrame");
    a.ldp(a64::x29, a64::x30, a64::Mem(a64::sp, 8 * 8));
    a.ldp(a64::x20, a64::x19, a64::Mem(a64::sp, 6 * 8));
    a.ldp(a64::x22, a64::x21, a64::Mem(a64::sp, 4 * 8));
    a.add(a64::sp, a64::sp, 10 * 8);
    a.ret(a64::x30);
  }

  void call(int32_t ofs, const char *name) {
    comment("// call %s", name);
    a.ldr(a64::x8, a64::Mem(xRuntime, ofs));
    a.blr(a64::x8);
  }

  /// Create an a64::Mem to a specifc frame register.
  static constexpr inline a64::Mem frameReg(uint32_t index) {
    // FIXME: check if the offset fits
    return a64::Mem(xFrame, index * sizeof(SHLegacyValue));
  }

  void loadFrame(a64::Gp dest, uint32_t rFrom) {
    // FIXME: check if the offset fits
    a.fmov(dest, a64::VecD(16 + rFrom));
  }
  template <typename R>
  void storeFrame(R src, uint32_t dest) {
    // FIXME: check if the offset fits
    a.fmov(a64::VecD(16 + dest), src);
  }

  void loadParam(uint32_t rRes, uint32_t paramIndex) {
    comment("// LoadParam r%u, %u", rRes, paramIndex);
    a.mov(a64::x0, xFrame);
    a.mov(a64::w1, paramIndex);
    EMIT_RUNTIME_CALL(*this, _sh_ljs_param);
    storeFrame(a64::x0, rRes);
  }

  void mov(uint32_t rRes, uint32_t rInput) {
    comment("// %s r%u, r%u", "mov", rRes, rInput);
    a.fmov(a64::VecD(16 + rRes), a64::VecD(16 + rInput));
  }

  void mul(uint32_t rRes, uint32_t rLeft, uint32_t rRight) {
    binOp(
        rRes,
        rLeft,
        rRight,
        "mul",
        [this](a64::VecD res, a64::VecD dl, a64::VecD dr) {
          a.fmul(res, dl, dr);
        },
        offsetof(SHRuntime, _sh_ljs_mul_rjs),
        "_sh_ljs_mul_rjs");
  }
  void add(uint32_t rRes, uint32_t rLeft, uint32_t rRight) {
    binOp(
        rRes,
        rLeft,
        rRight,
        "add",
        [this](a64::VecD res, a64::VecD dl, a64::VecD dr) {
          a.fadd(res, dl, dr);
        },
        offsetof(SHRuntime, _sh_ljs_add_rjs),
        "_sh_ljs_add_rjs");
  }

  void dec(uint32_t rRes, uint32_t rInput) {
    unop(
        rRes,
        rInput,
        "dec",
        [this](a64::VecD d, a64::VecD src, a64::VecD tmp) {
          a.fmov(tmp, -1.0);
          a.fadd(d, src, tmp);
        },
        offsetof(SHRuntime, _sh_ljs_dec_rjs),
        "_sh_ljs_dec_rjs");
  }
  void inc(uint32_t rRes, uint32_t rInput) {
    unop(
        rRes,
        rInput,
        "inc",
        [this](a64::VecD d, a64::VecD src, a64::VecD tmp) {
          a.fmov(tmp, 1.0);
          a.fadd(d, src, tmp);
        },
        offsetof(SHRuntime, _sh_ljs_inc_rjs),
        "_sh_ljs_inc_rjs");
  }

  void jGreater(
      bool invert,
      const asmjit::Label &target,
      uint32_t rLeft,
      uint32_t rRight) {
    jCond(
        invert,
        target,
        rLeft,
        rRight,
        "greater",
        [this](const asmjit::Label &target) { a.b_gt(target); },
        offsetof(SHRuntime, _sh_ljs_greater_rjs),
        "_sh_ljs_greater_rjs");
  }
  void jGreaterEqual(
      bool invert,
      const asmjit::Label &target,
      uint32_t rLeft,
      uint32_t rRight) {
    jCond(
        invert,
        target,
        rLeft,
        rRight,
        "greater_equal",
        [this](const asmjit::Label &target) { a.b_ge(target); },
        offsetof(SHRuntime, _sh_ljs_greater_equal_rjs),
        "_sh_ljs_greater_equal_rjs");
  }

  void loadConstUInt8(uint32_t rRes, uint8_t val) {
    comment("// LoadConstUInt8 r%u, %u", rRes, val);

    if (a64::Utils::isFP64Imm8((double)val)) {
      a.fmov(a64::VecD(16 + rRes), val);
    } else {
      // TODO: use a constant pool.
      a.mov(a64::w0, val);
      a.ucvtf(a64::VecD(16 + rRes), a64::w0);
    }
  }

 private:
  asmjit::Label newPrefLabel(const char *pref, size_t index) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%s%lu", pref, index);
    return a.newNamedLabel(buf);
  }
  inline asmjit::Label newSlowPathLabel() {
    return newPrefLabel("SLOW_", slowPaths_.size());
  }
  inline asmjit::Label newContLabel() {
    return newPrefLabel("CONT_", slowPaths_.size());
  }

  void emitSlowPaths() {
    while (!slowPaths_.empty()) {
      slowPaths_.front()();
      slowPaths_.pop_front();
    }
  }

  template <typename FastPath>
  void unop(
      uint32_t rRes,
      uint32_t rInput,
      const char *name,
      FastPath fast,
      int32_t slowCallOfs,
      const char *slowCallName) {
    comment("// %s r%u, r%u", name, rRes, rInput);
    fast(a64::VecD(16 + rRes), a64::VecD(16 + rInput), a64::d1);
  }

  template <typename FastPath>
  void binOp(
      uint32_t rRes,
      uint32_t rLeft,
      uint32_t rRight,
      const char *name,
      FastPath fast,
      int32_t slowCallOfs,
      const char *slowCallName) {
    comment("// %s r%u, r%u, r%u", name, rRes, rLeft, rRight);
    fast(a64::VecD(16 + rRes), a64::VecD(16 + rLeft), a64::VecD(16 + rRight));
  }

  template <typename FastPath>
  void jCond(
      bool invert,
      const asmjit::Label &target,
      uint32_t rLeft,
      uint32_t rRight,
      const char *name,
      FastPath fast,
      int32_t slowCallOfs,
      const char *slowCallName) {
    comment(
        "// j_%s_%s Lx, r%u, r%u", invert ? "not" : "", name, rLeft, rRight);
    a.fcmp(a64::VecD(16 + rLeft), a64::VecD(16 + rRight));
    if (!invert) {
      fast(target);
    } else {
      auto contLab = a.newLabel();
      fast(contLab);
      a.b(target);
      a.bind(contLab);
    }
  }
};

JitFn compile_loop1(void) {
  Emitter em(s_jitRT);
  a64::Assembler &a = em.a;

  em.frameSetup();

  em.comment("// xRuntime");
  a.mov(xRuntime, a64::x0);

  //  _sh_check_native_stack_overflow(shr);
  EMIT_RUNTIME_CALL(em, _sh_check_native_stack_overflow);

  // Function<bench>(3 params, 13 registers):
  //  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 13);
  em.comment("// _sh_enter");
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  a.mov(a64::w2, 13);
  EMIT_RUNTIME_CALL(em, _sh_enter);
  em.comment("// xFrame");
  a.mov(xFrame, a64::x0);

  //  locals.head.count = 0;
  em.comment("// locals.head.count = 0");
  a.mov(a64::w1, 0);
  a.str(a64::w1, a64::Mem(a64::sp, offsetof(SHLocals, count)));

  //     LoadParam         r5, 2
  em.loadParam(5, 2);
  //     LoadParam         r0, 1
  em.loadParam(0, 1);
  //     Dec               r4, r0
  em.dec(4, 0);
  //     LoadConstZero     r3
  em.loadConstUInt8(3, 0);
  //     LoadConstUInt8    r2, 1
  em.loadConstUInt8(2, 1);
  //     LoadConstZero     r1
  em.loadConstUInt8(1, 0);
  //     LoadConstZero     r0
  em.loadConstUInt8(0, 0);
  //     JNotGreaterEqual  L1, r4, r0
  auto L1 = a.newLabel();
  em.jGreaterEqual(true, L1, 4, 0);
  // L4:
  auto L4 = a.newLabel();
  a.bind(L4);
  //     Dec               r10, r5
  em.dec(10, 5);
  //     Mov               r8, r1
  em.mov(8, 1);
  //     Mov               r6, r4
  em.mov(6, 4);
  //     Mov               r9, r5
  em.mov(9, 5);
  //     Mov               r7, r9
  em.mov(7, 9);
  //     JNotGreater       L2, r10, r2
  auto L2 = a.newLabel();
  em.jGreater(true, L2, 10, 2);
  // L3:
  auto L3 = a.newLabel();
  a.bind(L3);
  //     Mul               r9, r9, r10
  em.mul(9, 9, 10);
  //     Dec               r10, r10
  em.dec(10, 10);
  //     Mov               r7, r9
  em.mov(7, 9);
  //     JGreater          L3, r10, r2
  em.jGreater(false, L3, 10, 2);
  // L2:
  a.bind(L2);
  //     Add               r1, r8, r7
  em.add(1, 8, 7);
  //     Dec               r4, r6
  em.dec(4, 6);
  //     Mov               r0, r1
  em.mov(0, 1);
  //     JGreaterEqual     L4, r4, r3
  em.jGreaterEqual(false, L4, 4, 3);
  // L1:
  a.bind(L1);
  //     Ret               r0
  em.loadFrame(a64::x22, 0);

  // _sh_leave(shr, &locals.head, frame);
  a.mov(a64::x0, xRuntime);
  a.mov(a64::x1, a64::sp);
  a.mov(a64::x2, xFrame);
  EMIT_RUNTIME_CALL(em, _sh_leave);

  a.mov(a64::x0, a64::x22);
  em.leaveFrame();

  return em.addToRuntime();
}
