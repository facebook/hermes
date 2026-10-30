/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "asmjit/x86.h"

#include "../JitHandlers.h"

#include "hermes/ADT/DenseUInt64.h"
#include "hermes/ADT/SimpleLRU.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JIT/JIT.h"
#include "hermes/VM/JIT/PerfJitDump.h"
#include "hermes/VM/JIT/x86-64/JIT.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/static_h.h"
#include "hermes/VMLayouts/StackFrameLayout.h"

#include <cstdarg>
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallVector.h"

#include <deque>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace hermes::vm::x86_64 {

namespace x86 = asmjit::x86;

/// A HermesVM frame register
class FR {
  uint32_t index_;

 public:
  static constexpr uint32_t kInvalid = UINT32_MAX;

  FR() : index_(kInvalid) {}
  constexpr explicit FR(uint32_t index) : index_(index) {}

  constexpr bool isValid() const {
    return index_ != kInvalid;
  }

  constexpr uint32_t index() const {
    return index_;
  }
  bool operator==(const FR &fr) const {
    return fr.index_ == index_;
  }
  bool operator!=(const FR &fr) const {
    return fr.index_ != index_;
  }
};

enum class FRType : uint8_t {
  Number = 1,
  Bool = 2,
  /// Any other non-pointer type.
  OtherNonPtr = 4,
  Pointer = 8,
  UnknownNonPtr = Number | Bool | OtherNonPtr,
  UnknownPtr = UnknownNonPtr | Pointer,
};

class HWReg {
  // 0..31: Gp. 32..63: Vec. 128: invalid.
  uint8_t index_;

  explicit constexpr HWReg(uint8_t index) : index_(index) {}

 public:
  struct GpX {};
  struct VecD {};

  constexpr HWReg() : index_(0xFF) {}
  explicit constexpr HWReg(uint8_t index, GpX) : index_(index) {}
  explicit constexpr HWReg(uint8_t index, VecD) : index_(index + 32) {}
  explicit constexpr HWReg(const x86::Gp &gp) : HWReg(gp.id(), GpX{}) {}
  explicit constexpr HWReg(const x86::Xmm &xmm) : HWReg(xmm.id(), VecD{}) {}

  static constexpr HWReg gpX(uint8_t index) {
    assert(index < 16 && "invalid Gp");
    return HWReg(index, GpX{});
  }
  static constexpr HWReg vecD(uint8_t index) {
    assert(index < 16 && "invalid Xmm");
    return HWReg(index, VecD{});
  }

  operator bool() const {
    return isValid();
  }
  bool isValid() const {
    return index_ != 0xFF;
  }
  bool isValidGpX() const {
    return index_ < 32;
  }
  bool isValidVecD() const {
    return index_ >= 32 && index_ < 64;
  }
  bool isGpX() const {
    assert(isValid());
    return index_ < 32;
  }
  bool isVecD() const {
    assert(isValid());
    return index_ >= 32 && index_ < 64;
  }

  x86::Gp gpq() const {
    assert(isGpX());
    return x86::gpq(indexInClass());
  }
  x86::Xmm xmm() const {
    assert(isVecD());
    return x86::xmm(indexInClass());
  }

  uint8_t combinedIndex() const {
    assert(isValid());
    return index_ & 63;
  }
  uint8_t indexInClass() const {
    assert(isValid());
    return index_ & 31;
  }

  bool operator==(const HWReg &other) const {
    return index_ == other.index_;
  }
  bool operator!=(const HWReg &other) const {
    return index_ != other.index_;
  }
};

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const hermes::vm::x86_64::HWReg &hwReg);

/// A frame register can reside simultaneously in one or more of the following
/// locations:
/// - The stack frame
/// - A global callee-save register (which can be either Gp or Vec)
/// - A local Gp register
/// - A local Vec register.
/// A frame register always has an allocated slot in the frame, even if it never
/// uses it.
/// Additionally, it may have an associated global reg, and two local regs.
/// Having them associated with the frame reg does not necessarily mean that the
/// hardware registers contain the most up-to-date value. The following
/// invariants apply:
/// - If there are local registers, they always contain the latest value.
/// - If there is more than one local register, they all contain the same bit
/// pattern.
/// - if there is a global register, it contains the latest value, unless
/// globalRegUpToDate is not set, in which case the latest value *must* be in
/// local registers. The state where there is a global reg, but the latest value
/// is only in the frame is not valid, as it is not useful.
/// - if frameUpToDate is set, then the frame contains the latest value.
struct FRState {
  /// Type that applies for the entire function.
  FRType globalType = FRType::UnknownPtr;
  /// Type in the current basic block, could be narrower. This applies, until
  /// it is reset, to the up-to-date value, local or not.
  FRType localType = FRType::UnknownPtr;

  /// Pre-allocated global register.
  HWReg globalReg{};
  /// Register in the current basic block.
  HWReg localGpX{};
  HWReg localVecD{};

  /// Whether the latest value has been written to the frame.
  bool frameUpToDate = false;
  /// Whether the global register exists and contains an up-to-date value. If
  /// false, either there is no globalReg, or there must be a local register
  /// allocated.
  bool globalRegUpToDate = false;

#ifndef NDEBUG
  /// Whether the currently associated register is dirty and about to be
  /// overwritten. This FR should not be read when in this state.
  bool regIsDirty = false;
#endif
};

struct HWRegState {
  FR contains{};
};

// x86-64: SysV register convention (see the design spec).
static constexpr auto xRuntime = x86::r15;
static constexpr auto xFrame = x86::r14;
// Reserved scratch, never allocated by TempRegAlloc.
static constexpr auto xScratch = x86::r11;
// GPR temps: {rax,rcx,rdx} and {rsi,rdi,r8,r9,r10} (asmjit ids 0-2,6-10);
// rbx/rsp/rbp (3-5) sit between the ranges and are never temps.
static constexpr std::pair<uint8_t, uint8_t> kGPTemp1(0, 2);
static constexpr std::pair<uint8_t, uint8_t> kGPTemp2(6, 10);
// Callee-saved global pool: rbx (also the return-value stash, the x21
// analogue), r12, r13. Not contiguous, so a list, not a range.
static constexpr uint8_t kGPSavedList[] = {3, 12, 13};
// rbx: return-value stash (arm64 x21 analogue); must equal kGPSavedList[0].
static constexpr uint8_t kGPReturnStash = 3;
// Vector temps: all of xmm0-xmm15; no callee-saved vector registers
// exist in SysV, so there are no vector globals.
static constexpr std::pair<uint8_t, uint8_t> kVecTemp(0, 15);

static constexpr uint32_t bitMask32(unsigned first, unsigned last) {
  return ((1u << (last - first + 1)) - 1u) << first;
}
template <typename T>
static constexpr uint32_t bitMask32(std::pair<T, T> range) {
  return bitMask32(range.first, range.second);
}

class TempRegAlloc {
  unsigned first_;
  SimpleLRU<unsigned> lru_{};
  std::vector<unsigned *> map_{};
  uint32_t availBits_;

 public:
  explicit TempRegAlloc(std::pair<uint8_t, uint8_t> range)
      : first_(range.first), lru_(range.second - range.first + 1) {
    map_.resize(range.second - range.first + 1);
    availBits_ = bitMask32(range);
  }
  explicit TempRegAlloc(
      std::pair<uint8_t, uint8_t> range1,
      std::pair<uint8_t, uint8_t> range2)
      : first_(range1.first),
        lru_(
            range1.second - range1.first + 1 + range2.second - range2.first +
            1) {
    map_.resize(range2.second - range1.first + 1);
    availBits_ = bitMask32(range1) | bitMask32(range2);
  }

  llvh::Optional<unsigned> alloc(
      llvh::Optional<unsigned> preferred = llvh::None) {
    if (availBits_ == 0)
      return llvh::None;

    unsigned index;
    if (preferred && (availBits_ & (1u << *preferred)))
      index = *preferred;
    else
      index = llvh::findFirstSet(availBits_);
    availBits_ &= ~(1u << index);
    assert(index >= first_ && "Invalid tmpreg index");
    assert(!map_[index - first_] && "map shows the index as occupied");
    map_[index - first_] = lru_.add(index);

    return index;
  }

  void use(unsigned index) {
    assert(index >= first_ && "Invalid tmpreg index");
    if (!(availBits_ & (1u << index)))
      lru_.use(map_[index - first_]);
  }

  void free(unsigned index) {
    assert(index >= first_ && "Invalid tmpreg index");
    assert(map_[index - first_] && "map shows the tmpreg is freed");
    assert(!(availBits_ & (1u << index)) && "bitmask shows tmpreg is freed");

    availBits_ |= (1u << index);
    lru_.remove(map_[index - first_]);
    map_[index - first_] = nullptr;
  }

  bool isAllocated(unsigned index) {
    assert(index >= first_ && "Invalid tmpreg index");
    return availBits_ & (1u << index);
  }

  unsigned leastRecentlyUsed() {
    return *lru_.leastRecent();
  }

 private:
};

class Emitter {
  // x86-64: unreferenced in the milestone-1 tree -- used from milestone 2;
  // keeps -Wunused-private-field clean until then.
  [[maybe_unused]] Runtime &runtime_;
  [[maybe_unused]] JITContext::Impl &jitImpl_;

  /// Level of dumping JIT code. Bit 0 indicates code printing on or off.
  unsigned const dumpJitCode_;
  /// Whether to emit asserts in the JIT'ed code.
  bool const emitAsserts_;
  /// Whether to verify FR type assumptions in the JIT'ed code.
  bool const emitTypeAsserts_;
  /// Whether to emit counters in the JIT'ed code.
  // x86-64: unreferenced in the milestone-1 tree -- used from milestone 2;
  // keeps -Wunused-private-field clean until then.
  [[maybe_unused]] bool const emitCounters_;

#ifndef ASMJIT_NO_LOGGING
  std::unique_ptr<asmjit::Logger> logger_{};
#endif

  std::unique_ptr<asmjit::ErrorHandler> errorHandler_;
  asmjit::Error expectedError_ = asmjit::kErrorOk;

  std::vector<FRState> frameRegs_;
  std::array<HWRegState, 64> hwRegs_;

  /// GP temp registers.
  TempRegAlloc gpTemp_{kGPTemp1, kGPTemp2};
  /// Vec temp registers.
  TempRegAlloc vecTemp_{kVecTemp};

  /// A deferred slow path, emitted at the end of the function.
  ///
  /// Everything the slow path needs beyond the common fields below is held in
  /// the lambda passed to the constructor, stored inline in \c storage_. This
  /// keeps each slow path's state private to the one place that produces and
  /// consumes it, instead of a shared set of fields that any slow path could
  /// read whether or not its producer set them.
  class SlowPath {
   public:
    /// Label of the slow path.
    asmjit::Label slowPathLab;
    /// Label to jump to after the slow path.
    asmjit::Label contLab;
    /// Bytecode IP of the instruction that this is a slow path for.
    const inst::Inst *emittingIP;

    /// \param l is invoked as l(Emitter &, SlowPath &) to emit the slow path.
    /// Its captures are copied into \c storage_ and never destroyed, so they
    /// must be trivially destructible and must fit.
    template <typename L>
    SlowPath(
        asmjit::Label slowPathLab,
        asmjit::Label contLab,
        const inst::Inst *emittingIP,
        L &&l)
        : slowPathLab(slowPathLab),
          contLab(contLab),
          emittingIP(emittingIP),
          emit_([](Emitter &em, SlowPath &sp) {
            (*reinterpret_cast<std::decay_t<L> *>(sp.storage_))(em, sp);
          }) {
      using Lambda = std::decay_t<L>;
      static_assert(
          sizeof(Lambda) <= sizeof(storage_),
          "slow path captures too much; enlarge storage_ or capture less");
      static_assert(
          alignof(Lambda) <= alignof(void *),
          "slow path captures are over-aligned");
      static_assert(
          std::is_trivially_destructible_v<Lambda>,
          "slow path captures must be trivially destructible");
      ::new (storage_) Lambda(std::forward<L>(l));
    }

    /// Overload for slow paths that do not branch back to a continuation.
    template <typename L>
    SlowPath(asmjit::Label slowPathLab, const inst::Inst *emittingIP, L &&l)
        : SlowPath(
              slowPathLab,
              asmjit::Label(),
              emittingIP,
              std::forward<L>(l)) {}

    /// Non-copyable and non-movable: \c storage_ holds a type-erased lambda
    /// that cannot be relocated by the implicit memberwise copy. std::deque
    /// never relocates existing elements, so emplace_back and pop_front are
    /// all that is needed.
    SlowPath(const SlowPath &) = delete;
    SlowPath &operator=(const SlowPath &) = delete;

    /// Emit this slow path.
    void emit(Emitter &em) {
      emit_(em, *this);
    }

   private:
    void (*emit_)(Emitter &em, SlowPath &sp);
    /// Inline storage for the lambda's captures. Sized to match arm64, whose
    /// largest slow path captures an asmjit::Label (16 bytes, an Operand)
    /// plus three pointers, two FRs and two bools. Raising this is fine; the
    /// static_assert above is what keeps a too-large capture from becoming a
    /// silent heap allocation.
    alignas(void *) char storage_[56];
  };
  /// Queue of slow paths.
  std::deque<SlowPath> slowPaths_{};
  /// x86-64: monotonically increasing counter used to name SLOW_/CONT_
  /// labels. Unlike arm64 -- which still names labels from
  /// slowPaths_.size() -- this backend does not reuse an index once it is
  /// handed out. See newSlowPathLabel()/newContLabel().
  unsigned slowPathLabelCounter_{0};

  /// FRs written by the bytecode instruction currently being emitted whose
  /// global register class requires a check. Drained at each instruction
  /// boundary by emitPendingTypeAsserts().
  llvh::SmallVector<FR, 4> typeAssertPendingWrites_{};

  /// Descriptor for a single RO data entry.
  struct DataDesc {
    /// Size in bytes.
    int32_t size;
    asmjit::TypeId typeId;
    int32_t itemCount;
    /// Optional comment.
    const char *comment;
  };
  /// Used for pretty printing when logging data.
  std::vector<DataDesc> roDataDesc_{};
  std::vector<uint8_t> roData_{};
  asmjit::Label roDataLabel_{};

  /// Map from the bit pattern of a double value to offset in constant pool.
  llvh::DenseMap<hermes::DenseUInt64, int32_t> fp64ConstMap_{};

  /// Label to branch to when returning from a function. The return value
  /// will be in rbx.
  asmjit::Label returnLabel_{};

  /// Label to branch to when catching an exception with setjmp.
  /// Invalid if there's no try/catch in the function.
  asmjit::Label catchTableLabel_{};

  /// The bytecode codeblock.
  CodeBlock *const codeBlock_;

  /// Optionally, the offset of the string name, used for debug printing.
  int32_t roOfsDebugFunctionName_ = -1;

  /// Offset in RODATA of the pointer to the start of the read property
  /// cache.
  int32_t roOfsReadPropertyCachePtr_;
  /// Offset in RODATA of the pointer to the start of the write property
  /// cache.
  int32_t roOfsWritePropertyCachePtr_;
  /// Offset in RODATA of the pointer to the start of the private name
  /// cache.
  int32_t roOfsPrivateNameCachePtr_;

  /// Number of entries of kGPSavedList pushed by the prologue. Always at
  /// least one, since rbx is saved even when no FR uses it.
  unsigned gpSaveCount_ = 0;

 public:
  asmjit::CodeHolder code{};
  x86::Assembler a{};
  /// The IP of the instruction being emitted.
  const inst::Inst *emittingIP{nullptr};

  /// Create an Emitter, but do not emit any actual code.
  /// Use \c enter to set up the stack frame before emitting the actual code.
  explicit Emitter(
      Runtime &runtime,
      JITContext::Impl &jitImpl,
      unsigned dumpJitCode,
      bool emitAsserts,
      bool emitTypeAsserts,
      bool emitCounters,
      PerfJitDump *perfJitDump,
      CodeBlock *codeBlock,
      const std::function<void(std::string &&message)> &longjmpError);

  /// Add the jitted function to the JIT runtime and return a pointer to it.
  JITCompiledFunctionPtr addToRuntime(asmjit::JitRuntime &jr);

#ifdef NDEBUG
  void assertPostInstructionInvariants() {}
#else
  void assertPostInstructionInvariants();
#endif

  /// Allocate global registers and set up the stack frame.
  /// Must be called before emitting any real code.
  /// \param numCount the first numCount registers are "number" registers.
  /// \param npCount the first npCount registers after the number registers are
  ///   non-pointer registers.
  void enter(uint32_t numCount, uint32_t npCount);

  /// Log a comment.
  /// Annotated with printf-style format.
  /// Defined inline below the class so the logger check is visible in every
  /// translation unit; the formatting itself is out of line in commentV().
  void comment(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

  /// Format \p fmt with \p args and pass the result to the assembler. Out of
  /// line so that vsnprintf is not duplicated into every caller.
  void commentV(const char *fmt, va_list args);

  /// Emit the catch table, slow paths and RO data,
  /// then reset the stack, end any try, and return.
  /// \param exceptionHandlers the labels for the exception handler table.
  void leave(llvh::ArrayRef<const asmjit::Label *> exceptionHandlers);
  void newBasicBlock(const asmjit::Label &label);

  /// Abort execution.
  void unreachable();

  /// Emit profiling information if profiling is enabled.
  void profilePoint(uint16_t point);

  void directEval(FR frRes, FR frText, bool strictCaller);

  /// Call a JS function.
  void call(FR frRes, FR frCallee, uint32_t argc);
  void callN(FR frRes, FR frCallee, llvh::ArrayRef<FR> args);
  void callBuiltin(FR frRes, uint32_t builtinIndex, uint32_t argc);
  void callWithNewTarget(FR frRes, FR frCallee, FR frNewTarget, uint32_t argc);
  /// Note that this technically allows different arguments at runtime because
  /// argc is a register.
  void callWithNewTargetLong(FR frRes, FR frCallee, FR frNewTarget, FR frArgc);

  /// Special bytecode for calling Metro require.
  void callRequire(FR frRes, FR frRequireFunc, uint32_t modIndex);

  /// Get a builtin closure.
  void getBuiltinClosure(FR frRes, uint32_t builtinIndex);

  void catchInst(FR frRes);

  /// Save the return value.
  void ret(FR frValue);
  void mov(FR frRes, FR frInput, bool logComment = true);
  void loadParam(FR frRes, uint32_t paramIndex);
  void loadConstDouble(FR frRes, double val, const char *name);
  void loadConstBits64(FR frRes, uint64_t val, FRType type, const char *name);
  void
  loadConstString(FR frRes, RuntimeModule *runtimeModule, uint32_t stringID);
  void
  loadConstBigInt(FR frRes, RuntimeModule *runtimeModule, uint32_t bigIntID);
  void toNumber(FR frRes, FR frInput);
  void toNumeric(FR frRes, FR frInput);
  void toInt32(FR frRes, FR frInput, bool isSigned);
  void addEmptyString(FR frRes, FR frInput);

  void addS(FR frRes, FR frLeft, FR frRight);
  void mod(bool forceNumber, FR frRes, FR frLeft, FR frRight);

  // x86-64: the arm64 body is a three-operand `fadd res, dl, dr`; the VEX
  // form is likewise three-operand, so res may alias either source without
  // an extra move.
#define DECL_BINOP(methodName, forceNum, commentStr, slowCall, x86body) \
  void methodName(FR rRes, FR rLeft, FR rRight) {                       \
    arithBinOp(                                                         \
        forceNum,                                                       \
        rRes,                                                           \
        rLeft,                                                          \
        rRight,                                                         \
        commentStr,                                                     \
        [](x86::Assembler & as,                                         \
           const x86::Xmm &res,                                         \
           const x86::Xmm &dl,                                          \
           const x86::Xmm &dr) x86body,                                 \
        (void *)slowCall,                                               \
        #slowCall);                                                     \
  }

  DECL_BINOP(mul, false, "mul", _sh_ljs_mul_rjs, { as.vmulsd(res, dl, dr); })
  DECL_BINOP(add, false, "add", _sh_ljs_add_rjs, { as.vaddsd(res, dl, dr); })
  DECL_BINOP(sub, false, "sub", _sh_ljs_sub_rjs, { as.vsubsd(res, dl, dr); })
  DECL_BINOP(div, false, "div", _sh_ljs_div_rjs, { as.vdivsd(res, dl, dr); })
  DECL_BINOP(mulN, true, "mulN", _sh_ljs_mul_rjs, { as.vmulsd(res, dl, dr); })
  DECL_BINOP(addN, true, "addN", _sh_ljs_add_rjs, { as.vaddsd(res, dl, dr); })
  DECL_BINOP(subN, true, "subN", _sh_ljs_sub_rjs, { as.vsubsd(res, dl, dr); })
  DECL_BINOP(divN, true, "divN", _sh_ljs_div_rjs, { as.vdivsd(res, dl, dr); })
#undef DECL_BINOP

  void bitAnd(FR rRes, FR rLeft, FR rRight);
  void bitOr(FR rRes, FR rLeft, FR rRight);
  void bitXor(FR rRes, FR rLeft, FR rRight);
  void lShift(FR rRes, FR rLeft, FR rRight);
  void rShift(FR rRes, FR rLeft, FR rRight);
  void urShift(FR rRes, FR rLeft, FR rRight);

  // x86-64: the fast body receives the Emitter rather than the assembler,
  // because x86 has no floating-point immediate: arm64's `fmov tmp, 1.0`
  // becomes a load of an interned RO-data constant, which only the Emitter
  // can hand out (roConst64()).
#define DECL_UNOP(methodName, forceNum, commentStr, slowCall, x86body) \
  void methodName(FR rRes, FR rInput) {                                \
    arithUnop(                                                         \
        forceNum,                                                      \
        rRes,                                                          \
        rInput,                                                        \
        commentStr,                                                    \
        [](Emitter & em,                                               \
           const x86::Xmm &d,                                          \
           const x86::Xmm &s,                                          \
           const x86::Xmm &tmp) x86body,                               \
        (void *)slowCall,                                              \
        #slowCall);                                                    \
  }

  // The addend is taken straight from memory: `vaddsd xmm, xmm, m64` reads
  // 8 bytes with no alignment requirement, so no temp is needed and the
  // `tmp` operand goes unused (unlike arm64, which needs it for `fmov`).
  DECL_UNOP(dec, false, "dec", _sh_ljs_dec_rjs, {
    (void)tmp;
    em.a.vaddsd(d, s, em.roConst64(llvh::DoubleToBits(-1.0), "-1.0"));
  })
  DECL_UNOP(inc, false, "inc", _sh_ljs_inc_rjs, {
    (void)tmp;
    em.a.vaddsd(d, s, em.roConst64(llvh::DoubleToBits(1.0), "1.0"));
  })
  // x86-64: arm64's `fneg` becomes a sign-bit flip. The mask is loaded into
  // the vector temp first rather than used as a `vxorpd` memory operand,
  // because that operand is 128 bits wide and would read past an 8-byte
  // RO-data entry. `tmp` is `d` whenever `d != s`, so the load never
  // clobbers the source.
  DECL_UNOP(negate, false, "neg", _sh_ljs_minus_rjs, {
    em.a.vmovsd(tmp, em.roConst64(UINT64_C(1) << 63, "-0.0"));
    em.a.vxorpd(d, s, tmp);
  });
#undef DECL_UNOP

  void jmpTrueFalse(bool onTrue, const asmjit::Label &target, FR frInput);
  void jmpUndefined(const asmjit::Label &target, FR frInput);
  void jmp(const asmjit::Label &target);
  void jmpBuiltinIs(
      bool invert,
      const asmjit::Label &target,
      uint8_t builtinIndex,
      FR frInput);

  void booleanNot(FR frRes, FR frInput);
  void bitNot(FR frRes, FR frInput);
  void typeOf(FR frRes, FR frInput);

  void getPNameList(FR frRes, FR frObj, FR frIdx, FR frSize);
  void getNextPName(FR frRes, FR frProps, FR frObj, FR frIdx, FR frSize);

  void toPropertyKey(FR frRes, FR frVal);

  void privateIsIn(FR frRes, FR frPrivateName, FR frTarget, uint8_t cacheIdx);

  void createPrivateName(FR frRes, SHSymbolID symID);

  void greater(FR rRes, FR rLeft, FR rRight);
  void greaterEqual(FR rRes, FR rLeft, FR rRight);
  void less(FR rRes, FR rLeft, FR rRight);
  void lessEqual(FR rRes, FR rLeft, FR rRight);
  void equal(FR rRes, FR rLeft, FR rRight);
  void notEqual(FR rRes, FR rLeft, FR rRight);

  void strictEqual(FR frRes, FR frLeft, FR frRight);
  void strictNotEqual(FR frRes, FR frLeft, FR frRight);

  void jGreater(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void
  jGreaterEqual(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void jLess(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void
  jLessEqual(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void jLessN(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void
  jLessEqualN(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);
  void jEqual(bool invert, const asmjit::Label &target, FR rLeft, FR rRight);

  void
  jmpTypeOfIs(const asmjit::Label &target, FR frInput, TypeOfIsTypes types);

  void typeOfIs(FR frRes, FR frInput, TypeOfIsTypes types);

  void
  jStrictEqual(bool invert, const asmjit::Label &target, FR frLeft, FR frRight);

  void uintSwitchImm(
      FR frInput,
      const asmjit::Label &defaultLabel,
      llvh::ArrayRef<const asmjit::Label *> labels,
      uint32_t minVal,
      uint32_t maxVal);

  /// Information for a case of a StringSwitchImm instruction.
  struct StringSwitchCase {
    // The string id of the case label.
    uint32_t caseLabelStringId;
    // A JIT label for the start of JITted code for the the basic block
    // corresponding to the case.
    const asmjit::Label *target;

    StringSwitchCase(uint32_t caseLabelStringId, const asmjit::Label *target)
        : caseLabelStringId(caseLabelStringId), target(target) {}
  };

  /// Emit a string switch. The lookup table is identified at runtime by
  /// (\p runtimeModule, \p tableIndex) rather than by baking its address into
  /// the code, since the module's table vector may be reallocated after this
  /// code is compiled (e.g. by lazy compilation).
  void stringSwitchImm(
      FR frInput,
      RuntimeModule *runtimeModule,
      uint32_t tableIndex,
      const asmjit::Label &defaultLabel,
      llvh::ArrayRef<StringSwitchCase> cases);

  void getByVal(FR frRes, FR frSource, FR frKey);
  void getByIndex(FR frRes, FR frSource, uint32_t key);

  void putByValLoose(FR frTarget, FR frKey, FR frValue);
  void putByValStrict(FR frTarget, FR frKey, FR frValue);

  void putByValWithReceiver(
      FR frTarget,
      FR frKey,
      FR frValue,
      FR frReceiver,
      bool isStrict);

  void getById(FR frRes, SHSymbolID symID, FR frSource, uint8_t cacheIdx);
  void tryGetById(FR frRes, SHSymbolID symID, FR frSource, uint8_t cacheIdx);

  void getByIdWithReceiver(
      FR frRes,
      SHSymbolID symID,
      FR frSource,
      FR frReceiver,
      uint8_t cacheIdx);
  void getByValWithReceiver(FR frRes, FR frSource, FR frKey, FR frReceiver);

  void
  putByIdLoose(FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx);
  void
  putByIdStrict(FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx);
  void
  tryPutByIdLoose(FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx);
  void
  tryPutByIdStrict(FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx);

  void defineOwnInDenseArray(FR frArray, FR frProp, uint32_t idx);

  void
  defineOwnById(FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx);
  void defineOwnByIndex(FR frTarget, FR frValue, uint32_t key);
  void defineOwnByVal(FR frTarget, FR frValue, FR frKey, bool enumerable);
  void defineOwnGetterSetterByVal(
      FR frTarget,
      FR frKey,
      FR frGetter,
      FR frSetter,
      bool enumerable);

  void getOwnBySlotIdx(FR frRes, FR frTarget, uint32_t slotIdx);
  void putOwnBySlotIdx(FR frTarget, FR frValue, uint32_t slotIdx);

  void delByVal(FR frRes, FR frTarget, FR frKey, bool strict);

  void addOwnPrivateBySym(FR frTarget, FR frKey, FR frValue);

  void getOwnPrivateBySym(FR frRes, FR frTarget, FR frKey, uint8_t cacheIdx);

  void putOwnPrivateBySym(FR frTarget, FR frKey, FR frValue, uint8_t cacheIdx);

  void instanceOf(FR frRes, FR frLeft, FR frRight);
  void isIn(FR frRes, FR frLeft, FR frRight);

  asmjit::Label newPrefLabel(const char *pref, size_t index);

  void newObject(FR frRes);
  void newObjectWithParent(FR frRes, FR frParent, uint32_t shapeTableIndex);
  void newObjectWithBuffer(
      FR frRes,
      uint32_t shapeTableIndex,
      uint32_t valBufferOffset);
  void newObjectWithBufferAndParent(
      FR frRes,
      FR frParent,
      uint32_t shapeTableIndex,
      uint32_t valBufferOffset);

  void newTypedObjectWithBuffer(
      FR frRes,
      FR frParent,
      uint32_t shapeTableIndex,
      uint32_t valBufferOffset,
      uint8_t nonEnumerable);

  void newArray(FR frRes, uint32_t size);
  void newArrayWithBuffer(
      FR frRes,
      uint32_t numElements,
      uint32_t numLiterals,
      uint32_t bufferIndex);

  void newFastArray(FR frRes, FR frProto, uint32_t size);
  void fastArrayLength(FR frRes, FR arr);
  void fastArrayLoad(FR frRes, FR arr, FR idx);
  void fastArrayStore(FR arr, FR idx, FR val);
  void fastArrayPush(FR arr, FR val);
  void fastArrayAppend(FR arr, FR other);

  void getGlobalObject(FR frRes);
  void declareGlobalVar(SHSymbolID symID);
  void createTopLevelEnvironment(FR frRes, uint32_t size);
  void createFunctionEnvironment(FR frRes, uint32_t size);
  void createEnvironment(FR frRes, FR frParent, uint32_t size);
  void getParentEnvironment(FR frRes, uint32_t level);
  void getEnvironment(FR frRes, FR frSource, uint32_t level);
  void getClosureEnvironment(FR frRes, FR frClosure);
  void loadFromEnvironment(FR frRes, FR frEnv, uint32_t slot);
  void storeToEnvironment(bool np, FR frEnv, uint32_t slot, FR frValue);
  void createClosure(
      FR frRes,
      FR frEnv,
      RuntimeModule *runtimeModule,
      uint32_t functionID);
  void createBaseClass(FR frRes, FR frPrototypeOut, FR frEnv);
  void
  createDerivedClass(FR frRes, FR frPrototypeOut, FR frEnv, FR frSuperClass);
  void createGenerator(
      FR frRes,
      FR frEnv,
      RuntimeModule *runtimeModule,
      uint32_t functionID);

  void getArgumentsPropByValLoose(FR frRes, FR frIndex, FR frLazyReg);
  void getArgumentsPropByValStrict(FR frRes, FR frIndex, FR frLazyReg);

  void reifyArgumentsLoose(FR frLazyReg);
  void reifyArgumentsStrict(FR frLazyReg);

  void getArgumentsLength(FR frRes, FR frLazyReg);

  void createThis(
      FR frRes,
      FR frCallee,
      FR frNewTarget,
      uint8_t cacheIdx,
      uint32_t shapeTableIndex);
  void selectObject(FR frRes, FR frThis, FR frConstructed);

  void loadThisNS(FR frRes);
  void coerceThisNS(FR frRes, FR frThis);
  void getNewTarget(FR frRes);

  void iteratorBegin(FR frRes, FR frSource);
  void iteratorNext(FR frRes, FR frIteratorOrIdx, FR frSourceOrNext);
  void iteratorClose(FR frIteratorOrIdx, bool ignoreExceptions);

  void debugger();
  void throwInst(FR frInput);
  void throwIfEmpty(FR frRes, FR frInput);
  void throwIfUndefined(FR frRes, FR frInput);
  void throwIfThisInitialized(FR frInput);

  void createRegExp(
      FR frRes,
      SHSymbolID patternID,
      SHSymbolID flagsID,
      uint32_t regexpID);

  void loadParentNoTraps(FR frRes, FR frObj);
  void typedLoadParent(FR frRes, FR frObj);

  /// Emit, only when emitTypeAsserts_ is set, a trap-on-violation check
  /// that the value of \p fr, currently held in \p hwVal, satisfies
  /// \p pred.
  ///
  /// x86-64: a placeholder. The check sequences land with the type-assert
  /// milestone; until then enter() declines any function that asks for
  /// them, so the flag is never set here. The call sites exist already so
  /// that the ported emitters stay textually identical to arm64's.
  void emitTypeAssert(FR fr, HWReg hwVal, TypePred pred) {
    (void)fr;
    (void)hwVal;
    (void)pred;
    assert(!emitTypeAsserts_ && "type asserts are not implemented yet");
  }

  /// Emit, at a bytecode instruction boundary, the global-register-class
  /// checks for every FR recorded since the last call.
  /// x86-64: the check sequences themselves land with the tag-helper
  /// milestone. The recording side is ported now so that the register
  /// allocator stays textually identical to arm64's; draining here keeps
  /// newBasicBlock()'s "nothing pending at a boundary" invariant true.
  /// enter() declines any function when type asserts are requested, so a
  /// caller asking for them never silently gets code without them.
  void emitPendingTypeAsserts() {
    typeAssertPendingWrites_.clear();
  }

 private:
  /// \return the byte offset of \p fr's slot from xFrame.
  static constexpr inline uint32_t frByteOffset(FR fr) {
    return (fr.index() + hbc::StackFrameLayout::FirstLocal) *
        sizeof(SHLegacyValue);
  }

  /// Create an x86::Mem addressing a specific frame register.
  /// x86-64: every memory operand takes a signed 32-bit displacement, which
  /// spans any frame the bytecode can describe, so unlike arm64 there is no
  /// immediate-range fallback anywhere in the frame accessors.
  static inline x86::Mem frMem(FR fr) {
    return x86::qword_ptr(xFrame, (int32_t)frByteOffset(fr));
  }

  /// Return true if we are logging, false otherwise.
  bool hasLogger() {
#ifndef ASMJIT_NO_LOGGING
    return logger_ != nullptr;
#else
    return false;
#endif
  }

  /// Load an arbitrary bit pattern into a Gp.
  /// x86-64: any 64-bit constant is a single mov, so there is neither a
  /// cheap/expensive split nor an RO-data fallback. \p constName is kept
  /// for signature parity with arm64.
  template <typename REG>
  void loadBits64InGp(const REG &dest, uint64_t bits, const char *constName) {
    (void)constName;
    a.mov(dest, asmjit::Imm(bits));
  }

  void _loadFrame(HWReg dest, FR rFrom) {
    if (dest.isGpX())
      a.mov(dest.gpq(), frMem(rFrom));
    else
      a.vmovsd(dest.xmm(), frMem(rFrom));
  }
  void _storeFrame(HWReg src, FR rFrom) {
    if (src.isGpX())
      a.mov(frMem(rFrom), src.gpq());
    else
      a.vmovsd(frMem(rFrom), src.xmm());
  }

  bool isTempGpX(HWReg hwReg) const {
    assert(hwReg.isGpX());
    unsigned index = hwReg.indexInClass();
    return (index >= kGPTemp1.first && index <= kGPTemp1.second) ||
        (index >= kGPTemp2.first && index <= kGPTemp2.second);
  }

  bool isTempVecD(HWReg hwReg) const {
    assert(hwReg.isVecD());
    unsigned index = hwReg.indexInClass();
    return index >= kVecTemp.first && index <= kVecTemp.second;
  }

  bool isTemp(HWReg hwReg) const {
    return hwReg.isGpX() ? isTempGpX(hwReg) : isTempVecD(hwReg);
  }

  template <bool use>
  void movHWFromHW(HWReg dst, HWReg src);
  void _storeHWToFrame(FR fr, HWReg src);
  void movHWFromFR(HWReg hwRes, FR src);
  void movHWFromMem(HWReg hwRes, x86::Mem src);

  /// Move a value from a hardware register \p src to the frame register \p
  /// frDest.
  void movFRFromHW(FR frDest, HWReg src, FRType type);

  /// In rare cases, such as when we have in/out parameters to operations, the
  /// frame may get updated with a new value. This will ensure that the frame is
  /// marked up-to-date, and that any associated global register holds the same
  /// value.
  void syncFrameOutParam(FR fr, FRType type = FRType::UnknownPtr);

  template <class TAG>
  HWReg _allocTemp(TempRegAlloc &ra, llvh::Optional<HWReg> preferred);
  HWReg allocTempGpX(llvh::Optional<HWReg> preferred = llvh::None) {
    assert((!preferred || preferred->isGpX()) && "invalid preferred register");
    return _allocTemp<HWReg::GpX>(gpTemp_, preferred);
  }
  HWReg allocTempVecD(llvh::Optional<HWReg> preferred = llvh::None) {
    assert((!preferred || preferred->isVecD()) && "invalid preferred register");
    return _allocTemp<HWReg::VecD>(vecTemp_, preferred);
  }
  HWReg allocAndLogTempGpX() {
    HWReg res = allocTempGpX();
    comment("    ; alloc: r%u (temp)", res.indexInClass());
    return res;
  }
  /// Free \p hwReg, which may be any HWReg.
  void freeReg(HWReg hwReg);
  /// If \p hwReg is a valid temp associated with an FR, sync it to the global
  /// register if the FR has one, else store it in the frame. Then free the
  /// temp, making it available to be used again.
  /// Else, do nothing.
  void syncAndFreeTempReg(HWReg hwReg);
  HWReg useReg(HWReg hwReg);

  /// Ensure that an HWReg currently containing an FR is available to be used
  /// again by "spilling" its value to its canonical location (either the frame
  /// or a global reg). Conceptually it then frees the HWReg and immediately
  /// allocates it again, so now it is as if it was just allocated.
  /// \pre \p toSpill must have a corresponding FR.
  void _spillTempForFR(HWReg toSpill);

  /// Ensure that \p fr is stored in the frame so that we can take its address
  /// (e.g. when passing the address of \p fr as a param to a function).
  ///
  /// Store any temporary or global register associated with \p fr to the frame
  /// in memory.
  void syncToFrame(FR fr);

  /// Ensure all FRs have their values stored in either global registers or the
  /// frame, not just temporary registers.
  /// Must run before calls because temporary registers will be clobbered by the
  /// call.
  ///
  /// Sync all temporary registers associated with FRs to either the global
  /// register or the frame.
  /// \param exceptFR If specified, do not sync this FR (used for output FRs
  /// that we aren't going to load from before storing to them anyway).
  void syncAllFRTempExcept(FR exceptFR);

  /// Free all temporary registers associated with FRs except \p exceptFR.
  void freeAllFRTempExcept(FR exceptFR);

  /// Free any temporary register associated with \p FR.
  void freeFRTemp(FR fr);

  void _assignAllocatedLocalHWReg(FR fr, HWReg hwReg);

  /// \return a valid register if the FR is in a hw register, otherwise invalid.
  HWReg _isFRInRegister(FR fr);
  HWReg getOrAllocFRInVecD(
      FR fr,
      bool load,
      llvh::Optional<HWReg> preferred = llvh::None);
  HWReg getOrAllocFRInGpX(
      FR fr,
      bool load,
      llvh::Optional<HWReg> preferred = llvh::None);
  HWReg getOrAllocFRInAnyReg(
      FR fr,
      bool load,
      llvh::Optional<HWReg> preferred = llvh::None);

  void
  frUpdatedWithHW(FR fr, HWReg hwReg, FRType localType = FRType::UnknownPtr);
  void frUpdateType(FR fr, FRType type);

  /// \return true if the FR is currently known to contain the specified type.
  bool isFRKnownType(FR fr, FRType frType) const {
    auto &frState = frameRegs_[fr.index()];
    return frState.globalType == frType || frState.localType == frType;
  }
  /// \return true if the FR is currently known to contain a number.
  bool isFRKnownNumber(FR fr) const {
    return isFRKnownType(fr, FRType::Number);
  }
  /// \return true if the FR is currently known to contain a number.
  bool isFRKnownBool(FR fr) const {
    return isFRKnownType(fr, FRType::Bool);
  }
  /// \return true if the FR is currently known to contain an OtherNonPtr.
  bool isFRKnownOtherNonPtr(FR fr) const {
    return isFRKnownType(fr, FRType::OtherNonPtr);
  }

  /// Record that \p fr was written, so that the instruction boundary can
  /// check the value against its global register class. Records nothing
  /// unless the FR owns a global register. Callers must check
  /// emitTypeAsserts_ themselves; this does not.
  void recordFRWriteForAssert(FR fr);

  /// Like \c emitTypeAssert, but for an \p fr that the fast path never
  /// materializes into a register. x86-64: a placeholder, exactly as
  /// \c emitTypeAssert is.
  void emitTypeAssertFR(FR fr, TypePred pred) {
    (void)fr;
    (void)pred;
    assert(!emitTypeAsserts_ && "type asserts are not implemented yet");
  }

  /// Load the address of \p frameReg's frame slot into \p dst.
  /// x86-64: every frame slot is reachable through lea's signed 32-bit
  /// displacement, so unlike arm64 there is no immediate-range fallback.
  void loadFrameAddr(const x86::Gp &dst, FR frameReg) {
    a.lea(dst, x86::ptr(xFrame, (int32_t)frByteOffset(frameReg)));
  }

  /// \return a RIP-relative qword operand addressing the 64-bit constant
  /// \p bits, interning it in RO data on first use. x86 has no
  /// floating-point immediate, so a constant an emitter needs in a vector
  /// register has to come from memory.
  ///
  /// The RO data block is emitted wherever the code stream happens to end,
  /// so its absolute address carries no alignment beyond the code buffer's
  /// own. Entries are therefore usable only by instructions with no
  /// alignment requirement -- which is every VEX-encoded memory operand
  /// this backend emits, but not the legacy aligned-SSE forms. An emitter
  /// that needs an aligned operand must first make emitROData() align the
  /// block, with an \c a.align() before it binds roDataLabel_.
  x86::Mem roConst64(uint64_t bits, const char *comment) {
    return x86::qword_ptr(roDataLabel_, uint64Const(bits, comment));
  }

  void arithUnop(
      bool forceNumber,
      FR frRes,
      FR frInput,
      const char *name,
      void (*fast)(
          Emitter &em,
          const x86::Xmm &d,
          const x86::Xmm &s,
          const x86::Xmm &tmp),
      void *slowCall,
      const char *slowCallName);

  void arithBinOp(
      bool forceNumber,
      FR frRes,
      FR frLeft,
      FR frRight,
      const char *name,
      void (*fast)(
          x86::Assembler &a,
          const x86::Xmm &res,
          const x86::Xmm &dl,
          const x86::Xmm &dr),
      void *slowCall,
      const char *slowCallName);

 private:
  /// Allocate or return the offset in RO DATA of the current function's debug
  /// name, in the format ID(name).
  int32_t getDebugFunctionName();

  /// \return the number of bytes subtracted from/added to rsp by the single
  /// sub/add that follows the pushes. Covers the optional SHJmpBuf and saved
  /// SHLocals, plus the padding that restores 16-byte alignment.
  uint32_t getStackSize() const {
    return getSavedRegsPadding() + getExceptionAreaSize();
  }

  /// \return the size of the optional SHJmpBuf + saved SHLocals area, which
  /// is a multiple of 16 so that it cannot perturb the stack alignment.
  uint32_t getExceptionAreaSize() const {
    return catchTableLabel_.isValid() ? llvh::alignTo(sizeof(SHJmpBuf) + 8, 16)
                                      : 0;
  }

  /// \return the alignment padding between the saved registers and the
  /// exception area (SHLocals*/SHJmpBuf). See the arithmetic in
  /// frameSetup().
  uint32_t getSavedRegsPadding() const {
    return 8 * (gpSaveCount_ % 2);
  }

  /// \return the offset of the SHJmpBuf from rsp. The SHJmpBuf sits directly
  /// above rsp (below the padding), so it is 16-byte aligned regardless of
  /// the parity of gpSaveCount_ -- see the frame-layout comment in
  /// frameSetup().
  uint32_t getJmpBufOffset() const {
    assert(catchTableLabel_.isValid() && "no SHJmpBuf on stack");
    return 0;
  }

  /// \return the offset of the saved SHLocals pointer from rsp.
  uint32_t getSavedSHLocalsOffset() const {
    assert(catchTableLabel_.isValid() && "no saved SHLocals * on stack");
    return sizeof(SHJmpBuf);
  }

  /// \return true if \c emittingIP is in a try (i.e. exceptions can be observed
  ///   in this function).
  bool isInTry() const {
    return codeBlock_->findCatchTargetOffset(
               codeBlock_->getOffsetOf(emittingIP)) != -1;
  }

  /// Emit the function prologue.
  /// \param numFrameRegs the number of JS frame registers to reserve.
  /// \param gpSaveCount the number of kGPSavedList entries used by globals.
  void frameSetup(unsigned numFrameRegs, unsigned gpSaveCount);

  // x86-64: unlike arm64, label names here are drawn from a dedicated
  // monotonically-increasing counter rather than slowPaths_.size(). The
  // latter is non-monotonic -- emitSlowPaths() pops from the front as it
  // emits -- and two label pairs created before either is emplaced would
  // sample the same size() and collide. newSlowPathLabel() advances the
  // counter and newContLabel() reads it back, so a SLOW/CONT pair always
  // shares one index and no two pairs ever collide.
  asmjit::Label newSlowPathLabel() {
    return newPrefLabel("SLOW_", ++slowPathLabelCounter_);
  }
  asmjit::Label newContLabel() {
    return newPrefLabel("CONT_", slowPathLabelCounter_);
  }

  /// Emit a call to \p fn, saving the bytecode IP to Runtime::currentIP
  /// before making the call. This should be used for all calls that may
  /// observe the IP, such as calls that may throw exceptions, or perform
  /// allocations.
  void callRuntimeWithSavedIP(void *fn, const char *name);

  /// Emit a call to \p fn without saving the IP. This should be used only
  /// where saving the IP is unnecessary or incorrect.
  void callRuntime(void *fn, const char *name);

  void emitSlowPaths();

  int32_t reserveData(
      int32_t dsize,
      size_t align,
      asmjit::TypeId typeId,
      int32_t itemCount,
      const char *comment = nullptr);
  /// Register a 64-bit constant in RO DATA and return its offset.
  int32_t uint64Const(uint64_t bits, const char *comment);

  void emitROData();

  /// Report that the emitter does not yet support \p what, aborting
  /// compilation of the current function cleanly (the JITContext::Compiler
  /// driver catches this and falls back to the interpreter). Routes through
  /// the same expected-error / longjmp mechanism as a genuine AsmJit error.
  [[noreturn]] void unsupported(const char *what);
}; // class Emitter

/// Only the logger check lives here; the formatting is out of line in
/// commentV(). The check has to be visible to every emitter translation unit:
/// when ASMJIT_NO_LOGGING is defined hasLogger() folds to a constant false, so
/// the compiler can drop the call and dead-strip the format string. With the
/// whole body in JitEmitter.cpp, callers in other translation units had to
/// materialise and pass every string, which cost ~4KB of .cstring. Keeping
/// vsnprintf out of line means enabling logging does not duplicate the
/// formatting code into each caller.
inline void Emitter::comment(const char *fmt, ...) {
  if (!hasLogger())
    return;
  va_list args;
  va_start(args, fmt);
  commentV(fmt, args);
  va_end(args);
}

template <bool use>
void Emitter::movHWFromHW(HWReg dst, HWReg src) {
  if (dst != src) {
    // x86-64: vmovaps is the reg-to-reg vector move (no partial-register
    // merge, unlike the three-operand vmovsd), and vmovq moves the whole
    // 64-bit pattern between a Gp and the low half of an Xmm.
    if (dst.isVecD() && src.isVecD())
      a.vmovaps(dst.xmm(), src.xmm());
    else if (dst.isVecD())
      a.vmovq(dst.xmm(), src.gpq());
    else if (src.isVecD())
      a.vmovq(dst.gpq(), src.xmm());
    else
      a.mov(dst.gpq(), src.gpq());
  }
  if constexpr (use) {
    useReg(src);
    useReg(dst);
  }
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

} // namespace hermes::vm::x86_64
