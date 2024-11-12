/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "asmjit/a64.h"

#include "hermes/ADT/DenseUInt64.h"
#include "hermes/ADT/SimpleLRU.h"
#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/static_h.h"

#include "llvh/ADT/DenseMap.h"

#include <deque>

namespace hermes::vm::arm64 {

namespace a64 = asmjit::a64;

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
  // 0..31: GpX. 32..63: VecD. 128: invalid.
  uint8_t index_;

  explicit constexpr HWReg(uint8_t index) : index_(index) {}

 public:
  struct GpX {};
  struct VecD {};

  constexpr HWReg() : index_(0xFF) {}
  explicit constexpr HWReg(uint8_t index, GpX) : index_(index) {}
  explicit constexpr HWReg(uint8_t index, VecD) : index_(index + 32) {}
  explicit constexpr HWReg(const a64::GpX &gpx) : HWReg(gpx.id(), GpX{}) {}
  explicit constexpr HWReg(const a64::VecD &vecd) : HWReg(vecd.id(), VecD{}) {}

  static constexpr HWReg gpX(uint8_t index) {
    assert(index < 31 && "invalid GpX");
    return HWReg(index, GpX{});
  }
  static constexpr HWReg vecD(uint8_t index) {
    assert(index < 32 && "invalid VecD");
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

  a64::GpX a64GpX() const {
    assert(isGpX());
    return a64::GpX(indexInClass());
  }
  a64::VecD a64VecD() const {
    assert(isVecD());
    return a64::VecD(indexInClass());
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
    const hermes::vm::arm64::HWReg &hwReg);

/// A frame register can reside simultaneously in one or more of the following
/// locations:
/// - The stack frame
/// - A global callee-save register (which can be either GpX or VecD)
/// - A local GpX register
/// - A local VecD register.
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

// x19 is runtime
static constexpr auto xRuntime = a64::x19;
// x20 is frame
static constexpr auto xFrame = a64::x20;

/// GP arg registers (inclusive).
// static constexpr std::pair<uint8_t, uint8_t> kGPArgs(0, 7);
/// Temporary GP registers (inclusive).
static constexpr std::pair<uint8_t, uint8_t> kGPTemp(0, 15);
/// Callee-saved GP registers (inclusive).
static constexpr std::pair<uint8_t, uint8_t> kGPSaved(21, 28);

/// Vec arg registers (inclusive).
// static constexpr std::pair<uint8_t, uint8_t> kVecArgs(0, 7);
/// Temporary vec registers (inclusive).
static constexpr std::pair<uint8_t, uint8_t> kVecTemp1(0, 7);
static constexpr std::pair<uint8_t, uint8_t> kVecTemp2(16, 31);
/// Callee-saved vec registers (inclusive).
static constexpr std::pair<uint8_t, uint8_t> kVecSaved(8, 15);

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
  /// Level of dumping JIT code. Bit 0 indicates code printing on or off.
  unsigned const dumpJitCode_;
  /// Whether to emit asserts in the JIT'ed code.
  bool const emitAsserts_;

  std::unique_ptr<asmjit::Logger> logger_{};
  std::unique_ptr<asmjit::ErrorHandler> errorHandler_;
  asmjit::Error expectedError_ = asmjit::kErrorOk;

  std::vector<FRState> frameRegs_;
  std::array<HWRegState, 64> hwRegs_;

  /// GP temp registers.
  TempRegAlloc gpTemp_{kGPTemp};
  /// VecD temp registers.
  TempRegAlloc vecTemp_{kVecTemp1, kVecTemp2};

  /// Keep enough information to generate a slow path at the end of the
  /// function.
  struct SlowPath {
    /// Label of the slow path.
    asmjit::Label slowPathLab;
    /// Label to jump to after the slow path.
    asmjit::Label contLab;
    /// Target if this is a branch.
    asmjit::Label target;

    /// Name of the slow path.
    const char *name;
    /// Frame register indexes;
    FR frRes, frInput1, frInput2;
    /// Optional hardware register for the result.
    HWReg hwRes;
    /// Whether to invert a condition.
    bool invert;
    /// Whether to pass arguments by value to the slow path.
    bool passArgsByVal;

    /// Pointer to the slow path function that must be called.
    void *slowCall;
    /// The name of the slow path function.
    const char *slowCallName;

    /// Bytecode IP of the instruction that this is a slow path for.
    const inst::Inst *emittingIP;

    /// Callback to actually emit.
    void (*emit)(Emitter &em, SlowPath &sl);
  };
  /// Queue of slow paths.
  std::deque<SlowPath> slowPaths_{};

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

  /// Each thunk contains the offset of the function pointer in roData.
  std::vector<std::pair<asmjit::Label, int32_t>> thunks_{};
  llvh::DenseMap<void *, size_t> thunkMap_{};

  /// Map from the bit pattern of a double value to offset in constant pool.
  llvh::DenseMap<hermes::DenseUInt64, int32_t> fp64ConstMap_{};

  /// Label to branch to when returning from a function. Return value will be
  /// in x22.
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
  /// Offset in RODATA of the pointer to the start of the read property
  /// cache.
  int32_t roOfsWritePropertyCachePtr_;

  unsigned gpSaveCount_ = 0;
  unsigned vecSaveCount_ = 0;

 public:
  asmjit::CodeHolder code{};
  a64::Assembler a{};
  /// The IP of the instruction being emitted.
  const inst::Inst *emittingIP{nullptr};

  /// Create an Emitter, but do not emit any actual code.
  /// Use \c enter to set up the stack frame before emitting the actual code.
  explicit Emitter(
      asmjit::JitRuntime &jitRT,
      unsigned dumpJitCode,
      bool emitAsserts,
      CodeBlock *codeBlock,
      PropertyCacheEntry *readPropertyCache,
      PropertyCacheEntry *writePropertyCache,
      uint32_t numFrameRegs,
      const std::function<void(std::string &&message)> &longjmpError);

  /// Add the jitted function to the JIT runtime and return a pointer to it.
  /// \param exceptionHandlers the labels for the exception handler table.
  JITCompiledFunctionPtr addToRuntime(
      asmjit::JitRuntime &jr,
      llvh::ArrayRef<const asmjit::Label *> exceptionHandlers);

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
  void comment(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

  void leave();
  void newBasicBlock(const asmjit::Label &label);

  /// Abort execution.
  void unreachable();

  /// Emit profiling information if profiling is enabled.
  void profilePoint(uint16_t point);

  /// Call a JS function.
  void call(FR frRes, FR frCallee, uint32_t argc);
  void callN(FR frRes, FR frCallee, llvh::ArrayRef<FR> args);
  void callBuiltin(FR frRes, uint32_t builtinIndex, uint32_t argc);
  void callWithNewTarget(FR frRes, FR frCallee, FR frNewTarget, uint32_t argc);
  /// Note that this technically allows different arguments at runtime because
  /// argc is a register.
  void callWithNewTargetLong(FR frRes, FR frCallee, FR frNewTarget, FR frArgc);

  /// Get a builtin closure.
  void getBuiltinClosure(FR frRes, uint32_t builtinIndex);

  void catchInst(FR frRes);

  /// Save the return value in x22.
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
  void toInt32(FR frRes, FR frInput);
  void addEmptyString(FR frRes, FR frInput);

  void addS(FR frRes, FR frLeft, FR frRight);
  void mod(bool forceNumber, FR frRes, FR frLeft, FR frRight);

#define DECL_BINOP(methodName, forceNum, commentStr, slowCall, a64body) \
  void methodName(FR rRes, FR rLeft, FR rRight) {                       \
    arithBinOp(                                                         \
        forceNum,                                                       \
        rRes,                                                           \
        rLeft,                                                          \
        rRight,                                                         \
        commentStr,                                                     \
        [](a64::Assembler & as,                                         \
           const a64::VecD &res,                                        \
           const a64::VecD &dl,                                         \
           const a64::VecD &dr) a64body,                                \
        (void *)slowCall,                                               \
        #slowCall);                                                     \
  }

  DECL_BINOP(mul, false, "mul", _sh_ljs_mul_rjs, { as.fmul(res, dl, dr); })
  DECL_BINOP(add, false, "add", _sh_ljs_add_rjs, { as.fadd(res, dl, dr); })
  DECL_BINOP(sub, false, "sub", _sh_ljs_sub_rjs, { as.fsub(res, dl, dr); })
  DECL_BINOP(div, false, "div", _sh_ljs_div_rjs, { as.fdiv(res, dl, dr); })
  DECL_BINOP(mulN, true, "mulN", _sh_ljs_mul_rjs, { as.fmul(res, dl, dr); })
  DECL_BINOP(addN, true, "addN", _sh_ljs_add_rjs, { as.fadd(res, dl, dr); })
  DECL_BINOP(subN, true, "subN", _sh_ljs_sub_rjs, { as.fsub(res, dl, dr); })
  DECL_BINOP(divN, true, "divN", _sh_ljs_div_rjs, { as.fdiv(res, dl, dr); })
#undef DECL_BINOP

#define DECL_BIT_BINOP(methodName, commentStr, slowCall, a64body) \
  void methodName(FR rRes, FR rLeft, FR rRight) {                 \
    bitBinOp(                                                     \
        rRes,                                                     \
        rLeft,                                                    \
        rRight,                                                   \
        commentStr,                                               \
        slowCall,                                                 \
        #slowCall,                                                \
        [](a64::Assembler & a,                                    \
           const a64::GpX &res,                                   \
           const a64::GpX &dl,                                    \
           const a64::GpX &dr) a64body);                          \
  }

  DECL_BIT_BINOP(bitAnd, "bit_and", _sh_ljs_bit_and_rjs, {
    a.and_(res, dl, dr);
  })
  DECL_BIT_BINOP(bitOr, "bit_or", _sh_ljs_bit_or_rjs, { a.orr(res, dl, dr); })
  DECL_BIT_BINOP(bitXor, "bit_xor", _sh_ljs_bit_xor_rjs, {
    a.eor(res, dl, dr);
  })
  DECL_BIT_BINOP(lShift, "lshift", _sh_ljs_left_shift_rjs, {
    a.lsl(res.w(), dl.w(), dr.w());
  })
  DECL_BIT_BINOP(rShift, "rshift", _sh_ljs_right_shift_rjs, {
    a.asr(res.w(), dl.w(), dr.w());
  })
  DECL_BIT_BINOP(urShift, "rshiftu", _sh_ljs_unsigned_right_shift_rjs, {
    a.lsr(res.w(), dl.w(), dr.w());
  })

#undef DECL_BIT_BINOP

#define DECL_UNOP(methodName, forceNum, commentStr, slowCall, a64body) \
  void methodName(FR rRes, FR rInput) {                                \
    arithUnop(                                                         \
        forceNum,                                                      \
        rRes,                                                          \
        rInput,                                                        \
        commentStr,                                                    \
        [](a64::Assembler & as,                                        \
           const a64::VecD &d,                                         \
           const a64::VecD &s,                                         \
           const a64::VecD &tmp) a64body,                              \
        (void *)slowCall,                                              \
        #slowCall);                                                    \
  }

  DECL_UNOP(dec, false, "dec", _sh_ljs_dec_rjs, {
    as.fmov(tmp, -1.0);
    as.fadd(d, s, tmp);
  })
  DECL_UNOP(inc, false, "inc", _sh_ljs_inc_rjs, {
    as.fmov(tmp, 1.0);
    as.fadd(d, s, tmp);
  })
  DECL_UNOP(negate, false, "neg", _sh_ljs_minus_rjs, { as.fneg(d, s); });

#undef DECL_UNOP

  void jmpTrueFalse(bool onTrue, const asmjit::Label &target, FR frInput);
  void jmpUndefined(const asmjit::Label &target, FR frInput);
  void jmp(const asmjit::Label &target);

  void booleanNot(FR frRes, FR frInput);
  void bitNot(FR frRes, FR frInput);
  void typeOf(FR frRes, FR frInput);

  void getPNameList(FR frRes, FR frObj, FR frIdx, FR frSize);
  void getNextPName(FR frRes, FR frProps, FR frObj, FR frIdx, FR frSize);

#define DECL_COMPARE(                                                   \
    methodName, commentStr, slowCall, condCode, invSlow, passArgsByVal) \
  void methodName(FR rRes, FR rLeft, FR rRight) {                       \
    compareImpl(                                                        \
        rRes,                                                           \
        rLeft,                                                          \
        rRight,                                                         \
        commentStr,                                                     \
        a64::CondCode::condCode,                                        \
        (void *)slowCall,                                               \
        #slowCall,                                                      \
        invSlow,                                                        \
        passArgsByVal);                                                 \
  }
  DECL_COMPARE(greater, "greater", _sh_ljs_greater_rjs, kGT, false, false)
  DECL_COMPARE(
      greaterEqual,
      "greater_equal",
      _sh_ljs_greater_equal_rjs,
      kGE,
      false,
      false)
  DECL_COMPARE(less, "less", _sh_ljs_less_rjs, kMI, false, false)
  DECL_COMPARE(
      lessEqual,
      "less_equal",
      _sh_ljs_less_equal_rjs,
      kLS,
      false,
      false)
  DECL_COMPARE(equal, "Eq", _sh_ljs_equal_rjs, kEQ, false, false)
  DECL_COMPARE(strictEqual, "StrictEq", _sh_ljs_strict_equal, kEQ, false, true)
  DECL_COMPARE(notEqual, "Neq", _sh_ljs_equal_rjs, kNE, true, false)
  DECL_COMPARE(
      strictNotEqual,
      "StrictNeq",
      _sh_ljs_strict_equal,
      kNE,
      true,
      true)
#undef DECL_COMPARE

#define DECL_JCOND(                                                      \
    methodName, forceNum, passArgsByVal, commentStr, slowCall, condCode) \
  void methodName(                                                       \
      bool invert, const asmjit::Label &target, FR rLeft, FR rRight) {   \
    jCond(                                                               \
        forceNum,                                                        \
        invert,                                                          \
        passArgsByVal,                                                   \
        target,                                                          \
        rLeft,                                                           \
        rRight,                                                          \
        commentStr,                                                      \
        a64::CondCode::condCode,                                         \
        (void *)slowCall,                                                \
        #slowCall);                                                      \
  }
  DECL_JCOND(jGreater, false, false, "greater", _sh_ljs_greater_rjs, kGT)
  DECL_JCOND(
      jGreaterEqual,
      false,
      false,
      "greater_equal",
      _sh_ljs_greater_equal_rjs,
      kGE)
  DECL_JCOND(jLess, false, false, "less", _sh_ljs_less_rjs, kMI)
  DECL_JCOND(
      jLessEqual,
      false,
      false,
      "less_equal",
      _sh_ljs_less_equal_rjs,
      kLS)
  DECL_JCOND(jLessN, true, false, "less_n", _sh_ljs_less_rjs, kMI)
  DECL_JCOND(
      jLessEqualN,
      true,
      false,
      "less_equal_n",
      _sh_ljs_less_equal_rjs,
      kLS)
  DECL_JCOND(jEqual, false, false, "eq", _sh_ljs_equal_rjs, kEQ)
  DECL_JCOND(jStrictEqual, false, true, "strict_eq", _sh_ljs_strict_equal, kEQ)
#undef DECL_JCOND

  void switchImm(
      FR frInput,
      const asmjit::Label &defaultLabel,
      llvh::ArrayRef<const asmjit::Label *> labels,
      uint32_t minVal,
      uint32_t maxVal);

  void getByVal(FR frRes, FR frSource, FR frKey);
  void getByIndex(FR frRes, FR frSource, uint32_t key);

#define DECL_PUT_BY_VAL(methodName, commentStr, shFn)                \
  void methodName(FR frTarget, FR frKey, FR frValue) {               \
    putByValImpl(frTarget, frKey, frValue, commentStr, shFn, #shFn); \
  }

  DECL_PUT_BY_VAL(putByValLoose, "putByValLoose", _sh_ljs_put_by_val_loose_rjs);
  DECL_PUT_BY_VAL(
      putByValStrict,
      "putByValStrict",
      _sh_ljs_put_by_val_strict_rjs);

#define DECL_GET_BY_ID(methodName, commentStr, shFn)                           \
  void methodName(FR frRes, SHSymbolID symID, FR frSource, uint8_t cacheIdx) { \
    getByIdImpl(frRes, symID, frSource, cacheIdx, commentStr, shFn, #shFn);    \
  }

  DECL_GET_BY_ID(getById, "getById", _sh_ljs_get_by_id_rjs)
  DECL_GET_BY_ID(tryGetById, "tryGetById", _sh_ljs_try_get_by_id_rjs)

#define DECL_PUT_BY_ID(methodName, commentStr, shFn)                          \
  void methodName(                                                            \
      FR frTarget, SHSymbolID symID, FR frValue, uint8_t cacheIdx) {          \
    putByIdImpl(frTarget, symID, frValue, cacheIdx, commentStr, shFn, #shFn); \
  }

  DECL_PUT_BY_ID(putByIdLoose, "putByIdLoose", _sh_ljs_put_by_id_loose_rjs);
  DECL_PUT_BY_ID(putByIdStrict, "putByIdStrict", _sh_ljs_put_by_id_strict_rjs);
  DECL_PUT_BY_ID(
      tryPutByIdLoose,
      "tryPutByIdLoose",
      _sh_ljs_try_put_by_id_loose_rjs);
  DECL_PUT_BY_ID(
      tryPutByIdStrict,
      "tryPutByIdStrict",
      _sh_ljs_try_put_by_id_strict_rjs);

  void putOwnByIndex(FR frTarget, FR frValue, uint32_t key);
  void putOwnByVal(FR frTarget, FR frValue, FR frKey, bool enumerable);
  void putOwnGetterSetterByVal(
      FR frTarget,
      FR frKey,
      FR frGetter,
      FR frSetter,
      bool enumerable);

  void putNewOwnById(FR frTarget, FR frValue, SHSymbolID key, bool enumerable);

  void getOwnBySlotIdx(FR frRes, FR frTarget, uint32_t slotIdx);
  void putOwnBySlotIdx(FR frTarget, FR frValue, uint32_t slotIdx);

#define DECL_DEL_BY_ID(methodName, commentStr, shFn)            \
  void methodName(FR frRes, FR frTarget, SHSymbolID key) {      \
    delByIdImpl(frRes, frTarget, key, commentStr, shFn, #shFn); \
  }

  DECL_DEL_BY_ID(delByIdLoose, "delByIdLoose", _sh_ljs_del_by_id_loose);
  DECL_DEL_BY_ID(delByIdStrict, "delByIdStrict", _sh_ljs_del_by_id_strict);

#define DECL_DEL_BY_VAL(methodName, commentStr, shFn)              \
  void methodName(FR frRes, FR frTarget, FR frKey) {               \
    delByValImpl(frRes, frTarget, frKey, commentStr, shFn, #shFn); \
  }

  DECL_DEL_BY_VAL(delByValLoose, "delByValLoose", _sh_ljs_del_by_val_loose);
  DECL_DEL_BY_VAL(delByValStrict, "delByValStrict", _sh_ljs_del_by_val_strict);

  void instanceOf(FR frRes, FR frLeft, FR frRight);
  void isIn(FR frRes, FR frLeft, FR frRight);

  asmjit::Label newPrefLabel(const char *pref, size_t index);

  void newObject(FR frRes);
  void newObjectWithParent(FR frRes, FR frParent);
  void newObjectWithBuffer(
      FR frRes,
      uint32_t shapeTableIndex,
      uint32_t valBufferOffset);

  void newArray(FR frRes, uint32_t size);
  void newArrayWithBuffer(
      FR frRes,
      uint32_t numElements,
      uint32_t numLiterals,
      uint32_t bufferIndex);

  void newFastArray(FR frRes, uint32_t size);
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
  void getClosureEnvironment(FR frRes, FR frClosure);
  void loadFromEnvironment(FR frRes, FR frEnv, uint32_t slot);
  void storeToEnvironment(bool np, FR frEnv, uint32_t slot, FR frValue);
  void createClosure(
      FR frRes,
      FR frEnv,
      RuntimeModule *runtimeModule,
      uint32_t functionID);
  void createGenerator(
      FR frRes,
      FR frEnv,
      RuntimeModule *runtimeModule,
      uint32_t functionID);

#define DECL_GET_ARGUMENTS_PROP_BY_VAL(methodName, commentStr, shFn) \
  void methodName(FR frRes, FR frIndex, FR frLazyReg) {              \
    getArgumentsPropByValImpl(                                       \
        frRes, frIndex, frLazyReg, commentStr, shFn, #shFn);         \
  }

  DECL_GET_ARGUMENTS_PROP_BY_VAL(
      getArgumentsPropByValLoose,
      "GetArgumentsPropByValLoose",
      _sh_ljs_get_arguments_prop_by_val_loose);
  DECL_GET_ARGUMENTS_PROP_BY_VAL(
      getArgumentsPropByValStrict,
      "GetArgumentsPropByValStrict",
      _sh_ljs_get_arguments_prop_by_val_strict);

  void reifyArgumentsLoose(FR frLazyReg) {
    reifyArgumentsImpl(frLazyReg, false, "ReifyArgumentsLoose");
  }
  void reifyArgumentsStrict(FR frLazyReg) {
    reifyArgumentsImpl(frLazyReg, true, "ReifyArgumentsStrict");
  }

  void getArgumentsLength(FR frRes, FR frLazyReg);

  void createThis(FR frRes, FR frCallee, FR frNewTarget, uint8_t cacheIdx);
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

  void createRegExp(
      FR frRes,
      SHSymbolID patternID,
      SHSymbolID flagsID,
      uint32_t regexpID);

  void loadParentNoTraps(FR frRes, FR frObj);
  void typedLoadParent(FR frRes, FR frObj);
  void typedStoreParent(FR frStoredValue, FR frObj);

 private:
  /// Create an a64::Mem to a specifc frame register.
  static constexpr inline a64::Mem frA64Mem(FR fr) {
    // FIXME: check if the offset fits
    auto ofs = (fr.index() + hbc::StackFrameLayout::FirstLocal) *
        sizeof(SHLegacyValue);
    return a64::Mem(xFrame, ofs);
  }

  /// Load an arbitrary bit pattern into a Gp.
  template <typename REG>
  void loadBits64InGp(const REG &dest, uint64_t bits, const char *constName);

  void _loadFrame(HWReg dest, FR rFrom) {
    // FIXME: check if the offset fits
    if (dest.isGpX())
      a.ldr(dest.a64GpX(), frA64Mem(rFrom));
    else
      a.ldr(dest.a64VecD(), frA64Mem(rFrom));
  }
  void _storeFrame(HWReg src, FR rFrom) {
    // FIXME: check if the offset fits
    if (src.isGpX())
      a.str(src.a64GpX(), frA64Mem(rFrom));
    else
      a.str(src.a64VecD(), frA64Mem(rFrom));
  }

  bool isTempGpX(HWReg hwReg) const {
    assert(hwReg.isGpX());
    unsigned index = hwReg.indexInClass();
    return index >= kGPTemp.first && index <= kGPTemp.second;
  }

  bool isTempVecD(HWReg hwReg) const {
    assert(hwReg.isVecD());
    unsigned index = hwReg.indexInClass();
    return (index >= kVecTemp1.first && index <= kVecTemp1.second) ||
        (index >= kVecTemp2.first && index <= kVecTemp2.second);
  }

  bool isTemp(HWReg hwReg) const {
    return hwReg.isGpX() ? isTempGpX(hwReg) : isTempVecD(hwReg);
  }

  void loadFrameAddr(a64::GpX dst, FR frameReg);
  template <bool use>
  void movHWFromHW(HWReg dst, HWReg src);
  void _storeHWToFrame(FR fr, HWReg src);
  void movHWFromFR(HWReg hwRes, FR src);
  void movHWFromMem(HWReg hwRes, a64::Mem src);

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
    comment("    ; alloc: x%u (temp)", res.indexInClass());
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

  /// Get the current bytecode IP in \p xOut.
  void getBytecodeIP(const a64::GpX &xOut);

 private:
  /// Allocate or return the offset in RO DATA of the current function's debug
  /// name, in the format ID(name).
  int32_t getDebugFunctionName();

  /// \return the stack size subtracted/added to sp when entering/leaving the
  /// function. Includes the two words at the top of the stack for x29 and x30.
  uint32_t getStackSize() const {
    return ((((gpSaveCount_ + 1) & ~1) + ((vecSaveCount_ + 1) & ~1) + 2) * 8) +
        getSavedRegsOffset();
  }

  /// \return the offset of the saved registers in the stack frame.
  uint32_t getSavedRegsOffset() const {
    // If there's exceptions, allocate space for SHJmpBuf and saved SHLocals in
    // the stack.
    return catchTableLabel_.isValid() ? llvh::alignTo(sizeof(SHJmpBuf) + 8, 16)
                                      : 0;
  }

  /// \return the offset of the SHJmpBuf in the stack frame.
  uint32_t getJmpBufOffset() const {
    assert(catchTableLabel_.isValid() && "no SHJmpBuf on stack");
    return 0;
  }

  /// \return the offset of the saved SHLocals pointer in the stack frame.
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

  void frameSetup(
      unsigned numFrameRegs,
      unsigned gpSaveCount,
      unsigned vecSaveCount);

  asmjit::Label newSlowPathLabel() {
    return newPrefLabel("SLOW_", slowPaths_.size());
  }
  asmjit::Label newContLabel() {
    return newPrefLabel("CONT_", slowPaths_.size());
  }

  int32_t reserveData(
      int32_t dsize,
      size_t align,
      asmjit::TypeId typeId,
      int32_t itemCount,
      const char *comment = nullptr);
  /// Register a 64-bit constant in RO DATA and return its offset.
  int32_t uint64Const(uint64_t bits, const char *comment);

  /// Register \p fn as a thunk and return its label.
  /// \param name is an optional name for the thunk.
  asmjit::Label registerThunk(void *fn, const char *name = nullptr);

  /// Register a call as a thunk and emit a call to it. Note that most calls
  /// into runtime functions should use \c callThunkWithSavedIP below.
  void callThunk(void *fn, const char *name);

  /// Register a call as a thunk and emit a call to it, saving the bytecode IP
  /// to Runtime::currentIP before making the call. This should be used for all
  /// calls that may observe the IP, such as calls that may throw exceptions, or
  /// perform allocations.
  void callThunkWithSavedIP(void *fn, const char *name);

  /// Call a function without registering it as a thunk. This should be used for
  /// functions that will only have a single call site in the emitted function,
  /// and therefore do not benefit from a thunk. Note that like \c callThunk,
  /// this does not save the IP.
  void callWithoutThunk(void *fn, const char *name);

  /// Emit the code that runs when this function is longjmped to.
  /// Performs the catch table lookup and jumps to the appropriate catch block,
  /// and if no catch block is found, pops the SHJmpBuf and rethrows the
  /// exception.
  void emitCatchTable(llvh::ArrayRef<const asmjit::Label *> exceptionHandlers);
  void emitSlowPaths();
  void emitThunks();
  void emitROData();

 private:
  /// Set up the call frame and perform the call. The caller should have already
  /// populated the arg count and new target registers.
  /// \param frRes is the frame register that will contain the result.
  /// \param frCallee is a frame register containing the callee.
  void callImpl(FR frRes, FR frCallee);

  void arithUnop(
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
      const char *slowCallName);

  void arithBinOp(
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
      const char *slowCallName);

  void bitBinOp(
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
          const a64::GpX &dr));

  void compareImpl(
      FR frRes,
      FR frLeft,
      FR frRight,
      const char *name,
      a64::CondCode condCode,
      void *slowCall,
      const char *slowCallName,
      bool invSlow,
      bool passArgsByVal);

  void jCond(
      bool forceNumber,
      bool invert,
      bool passArgsByVal,
      const asmjit::Label &target,
      FR frLeft,
      FR frRight,
      const char *name,
      a64::CondCode condCode,
      void *slowCall,
      const char *slowCallName);

  void putByValImpl(
      FR frTarget,
      FR frKey,
      FR frValue,
      const char *name,
      void (*shImpl)(
          SHRuntime *shr,
          SHLegacyValue *target,
          SHLegacyValue *key,
          SHLegacyValue *value),
      const char *shImplName);

  void delByIdImpl(
      FR frRes,
      FR frTarget,
      SHSymbolID key,
      const char *name,
      SHLegacyValue (
          *shImpl)(SHRuntime *shr, SHLegacyValue *target, SHSymbolID key),
      const char *shImplName);

  void delByValImpl(
      FR frRes,
      FR frTarget,
      FR frKey,
      const char *name,
      SHLegacyValue (
          *shImpl)(SHRuntime *shr, SHLegacyValue *target, SHLegacyValue *key),
      const char *shImplName);

  void getByIdImpl(
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
      const char *shImplName);

  void putByIdImpl(
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
      const char *shImplName);

  void getArgumentsPropByValImpl(
      FR frRes,
      FR frIndex,
      FR frLazyReg,
      const char *name,
      SHLegacyValue (*shImpl)(
          SHRuntime *shr,
          SHLegacyValue *frame,
          SHLegacyValue *idx,
          SHLegacyValue *lazyReg),
      const char *shImplName);

  void reifyArgumentsImpl(FR frLazyReg, bool strict, const char *name);
}; // class Emitter

} // namespace hermes::vm::arm64
