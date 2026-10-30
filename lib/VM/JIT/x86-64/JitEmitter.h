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
  Runtime &runtime_;
  JITContext::Impl &jitImpl_;

  /// Level of dumping JIT code. Bit 0 indicates code printing on or off.
  unsigned const dumpJitCode_;
  /// Whether to emit asserts in the JIT'ed code.
  bool const emitAsserts_;
  /// Whether to verify FR type assumptions in the JIT'ed code.
  bool const emitTypeAsserts_;
  /// Whether to emit counters in the JIT'ed code.
  bool const emitCounters_;

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

  /// Label to branch to when returning from a function.
  asmjit::Label returnLabel_{};

  /// The bytecode codeblock.
  CodeBlock *const codeBlock_;

  /// Offset in RODATA of the pointer to the start of the read property
  /// cache.
  int32_t roOfsReadPropertyCachePtr_;
  /// Offset in RODATA of the pointer to the start of the write property
  /// cache.
  int32_t roOfsWritePropertyCachePtr_;
  /// Offset in RODATA of the pointer to the start of the private name
  /// cache.
  int32_t roOfsPrivateNameCachePtr_;

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

  void mul(FR rRes, FR rLeft, FR rRight);
  void add(FR rRes, FR rLeft, FR rRight);
  void sub(FR rRes, FR rLeft, FR rRight);
  void div(FR rRes, FR rLeft, FR rRight);
  void mulN(FR rRes, FR rLeft, FR rRight);
  void addN(FR rRes, FR rLeft, FR rRight);
  void subN(FR rRes, FR rLeft, FR rRight);
  void divN(FR rRes, FR rLeft, FR rRight);

  void bitAnd(FR rRes, FR rLeft, FR rRight);
  void bitOr(FR rRes, FR rLeft, FR rRight);
  void bitXor(FR rRes, FR rLeft, FR rRight);
  void lShift(FR rRes, FR rLeft, FR rRight);
  void rShift(FR rRes, FR rLeft, FR rRight);
  void urShift(FR rRes, FR rLeft, FR rRight);

  void dec(FR rRes, FR rInput);
  void inc(FR rRes, FR rInput);
  void negate(FR rRes, FR rInput);

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

  /// Emit, at a bytecode instruction boundary, the global-register-class
  /// checks for every FR recorded since the last call. Task 2: no code is
  /// ever emitted, so there is nothing pending; this is a no-op stub until
  /// Task 3 lands the real substrate.
  void emitPendingTypeAsserts() {}

 private:
  /// Return true if we are logging, false otherwise.
  bool hasLogger() {
#ifndef ASMJIT_NO_LOGGING
    return logger_ != nullptr;
#else
    return false;
#endif
  }

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

} // namespace hermes::vm::x86_64
