/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_X86_64_FASTJIT_H
#define HERMES_VM_JIT_X86_64_FASTJIT_H

#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/VM/JIT/DenseUInt64.h"
#include "hermes/VM/JIT/x86-64/Emitter.h"
#include "hermes/VM/JIT/x86-64/JIT.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Debug.h"

namespace hermes {
namespace vm {

using hermes::hbc::StackFrameLayout;
using namespace hermes::inst;

namespace x86_64 {

/// Encodes the kind of operation the relocation performs.
enum class ReloKind : uint8_t {
  None = 0,
  /// *((int8_t *)relo.address) = target - (relo.address + 1).
  Int8,
  /// *((int32_t *)relo.address) = target - (relo.address + 4).
  Int32,
};

/// Information about a single relocation in the executable code.
/// A relocation encodes an operation that is applied to the executable code
/// after a target address becomes known.
struct Relo {
  /// What kind of operation the relocation performs.
  ReloKind kind;
  /// The location in the executable code which is updated when the relocation
  /// is applied.
  uint8_t *address;
  /// The relocation target, in other words, the address that wasn't yet known
  /// when the relocation was created.
  unsigned targetBCBBIndex;

  Relo(ReloKind kind, uint8_t *address, unsigned int targetBCBBIndex)
      : kind(kind), address(address), targetBCBBIndex(targetBCBBIndex) {}
  Relo() = default;
};

/// A pair of emitters for the fast path and the slow path. This class must be
/// passed and returned only by value (for performance reasons).
class Emitters {
 public:
  Emitter fast;
  Emitter slow;
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// An instance of this class is constructed to compile a single CodeBlock to
/// native code.
class FastJIT {
 public:
  FastJIT(JITContext *context, CodeBlock *codeBlock);

  /// Attempt to compile the associated CodeBlock. On success, the JIT function
  /// pointer in the CodeBlock will be set to the compiled body.
  void compile();

  /// A pointer to binOpN instruction's compilation function.
  typedef Emitters (FastJIT::*compileBinOpNPtr)(Emitters emit, const Inst *ip);

 private:
  /// Raise the error flag and record an error message.
  void error(const llvm::Twine &msg);

  /// Allocate executable memory using a conservative size estimate based on
  /// bytecode length. On failure it sets the error message and flag.
  /// \param bytecodeLength the length of the bytecode we will be compiling.
  /// \param[out] sizes on successful exit contains the size of the two
  ///     allocatedmemory memory blocks (fast paths and slow paths). Undefined
  ///     on failure.
  /// \return pointers to both allocated blocks on success.
  llvm::Optional<ExecHeap::BlockPair> allocRWX(
      size_t bytecodeLength,
      ExecHeap::SizePair &sizes);

  /// Initialize a newly allocated executable memory pool, for example by pre-
  /// allocating some blocks and initializing their contents.
  void initializeNewPool(ExecHeap::DualPool *pool);

  /// Disassemble a range of executable code.
  /// \param withAddr whether to dump the addresses and bytes of instructions.
  void disassembleRange(
      const uint8_t *from,
      const uint8_t *to,
      llvm::raw_ostream &OS,
      bool withAddr) const;

  /// Disassemble the entire compiled function.
  /// \param withAddr whether to dump the addresses and bytes of instructions.
  void disassembleResult(Emitters emit, llvm::raw_ostream &OS, bool withAddr)
      const;

  /// Apply a single relocation with an already resolved address \p target.
  void applyRelocation(const Relo &relo, const uint8_t *target);

  /// Resolve all recorded relocations and clear the relocation list \c relocs_.
  void resolveRelocations();

  /// Apply a 32-bit RIP offset to the previous 32-bit word which was just
  /// emitted.
  void applyRIP32Offset(uint8_t *current, const uint8_t *target) {
    *reinterpret_cast<uint32_t *>(current - 4) += target - current;
  }

  /// \return true if we can safely write at least \c kMinInstructionSpace of
  ///   bytes in the fast and slow path buffers. Set the error flag and message
  ///   and return false otherwise.
  inline bool checkSpace(const Emitters &emit);

  /// \return the offset in bytes from RegFrame to access the specified local
  ///   Hermes register.
  static inline int32_t localHermesRegByteOffset(uint32_t regIndex) {
    return sizeof(HermesValue) * StackFrameLayout::localOffset(regIndex);
  }

#ifndef NDEBUG
  /// Add a descriptor for the last emitted section in the slow path, so it
  /// can be disassembled correctly.
  /// \param slow the current slow emitter.
  /// \param data whether the section was code or data.
  void describeSlowPathSection(Emitter slow, bool data);
#else
  void describeSlowPathSection(Emitter slow, bool data) {}
#endif

  /// @name Emitters
  /// Every emitter function receives Emitters as a first parameter
  /// and returns it after updating it internally. Emitter function assume that
  /// checkSpace() has already been called, except where noted.
  /// @{

  /// Emit the function prologue. Calls checkSpace() before emitting.
  Emitters emitPrologue(Emitters emit);
  /// Emit the function epilogue. Calls checkSpace() before emitting.
  Emitters emitEpilogue(Emitters emit);

  /// Emit the code for a basic block. Calls checkSpace() before processing
  /// every bytecode instruction.
  Emitters compileBB(Emitters emit);

  /// Lookup or add the specified constant and return its offset.
  /// \param slow the slow-path emitter.
  /// \param cval the constant
  /// \param[out] constAddr will be initialized with the address of the
  ///     constant.
  /// \return the updated slow-path emitter.
  Emitter getConstant(Emitter slow, uint64_t cval, uint8_t *&constAddr);
  Emitter getConstant(Emitter slow, HermesValue hv, uint8_t *&constAddr) {
    return getConstant(slow, hv.getRaw(), constAddr);
  }
  Emitter getConstant(Emitter slow, void *addr, uint8_t *&constAddr) {
    return getConstant(slow, (uint64_t)addr, constAddr);
  }

  /// \return the basic block's index according to the current \p ip and
  /// the offset \p ipOffset.
  unsigned getBBIndex(const Inst *ip, uint32_t ipOffset) {
    uint32_t bcOffset = (const uint8_t *)ip + ipOffset - codeBlock_->begin();
    return bcLabels_[bcOffset];
  }

  /// Load the specified HermesValue constant \p cval into the specified native
  /// register \p reg. Whether the register is floating point or not is
  /// determined by \p fp.
  template <bool fp = false>
  Emitters loadConstantIntoNativeReg(Emitters emit, HermesValue cval, Reg reg);
  /// Load the specified address constant \p addr into the specified native
  /// register \p reg. The address must be in the C heap, which is constant
  /// and non-movable by GC.
  Emitters loadConstantAddrIntoNativeReg(Emitters emit, void *addr, Reg reg);

  /// Load the specified double constant \p value into Hermes register \p
  /// hermesReg.
  Emitters
  loadDoubleConstant(Emitters emit, OperandReg32 hermesReg, double value);

  /// Load the specified HermesValue constant \p value into Hermes register
  /// \p hermesReg.
  Emitters loadHermesValueConstant(
      Emitters emit,
      OperandReg32 hermesReg,
      HermesValue value);

  /// Mov the specified native register \p nativeReg to hermes register \p
  /// hermesReg. \p fp determines whether the native register is floating point.
  template <bool fp = false>
  Emitter
  movNativeRegToHermesReg(Emitter emit, Reg nativeReg, OperandReg32 hermesReg);

  /// Mov the specified hermes register \p hermesReg to native register \p
  /// nativeReg. \p fp determines whether the native register is floating point.
  template <bool fp = false>
  Emitter
  movHermesRegToNativeReg(Emitter emit, OperandReg32 hermesReg, Reg nativeReg);

  /// Move hermes reg \p src to hermes reg \p dst.
  Emitter
  movHermesRegToHermesReg(Emitter emit, OperandReg32 src, OperandReg32 dst);

  /// Move a Runtime member variable \p runtimeVar to hermes reg \p dst.
  Emitter
  movRuntimeVarToHermesReg(Emitter emit, uint32_t runtimeVar, OperandReg32 dst);

  /// Move a hermes reg \p src to a Runtime member variable \p runtimeVar.
  Emitter
  movHermesRegToRuntimeVar(Emitter emit, uint32_t runtimeVar, OperandReg32 src);

  /// Load the address of the specified hermes register \p hermesReg into the
  /// native register \p nativeReg.
  Emitter leaHermesReg(Emitter emit, OperandReg32 hermesReg, Reg nativeReg);

  /// Encode the \p nativeReg with a bool HermesValue tag
  /// The \p nativeReg must already contain a bool value (0 or 1)
  Emitters encodeBoolHVInNativeReg(Emitters emit, Reg nativeReg);

  /// Emit a 64-bit call to an absolute address.
  Emitter callAbsolute(Emitter emit, const void *dest);

  /// Load rdi with the Runtime register and emit a call to an external
  /// function, check for exception and store the successful result in
  /// \p resultReg.
  /// \param dest the address of the location in the contant pool whose
  /// value points to the address of the external call
  /// \param ip the current ip used to find the corresponding catch handler if
  /// an exception is returned by the external call.
  Emitter callExternal(
      Emitter emit,
      const uint8_t *dest,
      OperandReg32 resultReg,
      const Inst *ip);

  /// Load rdi with the Runtime register and emit a call to an external
  /// function, check for exception, no return value.
  /// \param dest the address of the location in the contant pool whose
  /// value points to the address of the external call
  /// \param ip the current ip used to find the corresponding catch handler if
  /// an exception is returned by the external call.
  Emitter
  callExternalNoReturnedVal(Emitter emit, const uint8_t *dest, const Inst *ip);

  /// Load rdi with the Runtime register and emit a call to an external
  /// function whose address \p dest is in the constant pool, storing
  /// the result in \p resultReg, and no execution exception.
  Emitter callExternalWithReturnedVal(
      Emitter emit,
      const uint8_t *dest,
      OperandReg32 resultReg);

  /// Load rsi and rdx with the second and third operand of the binary operation
  /// instruction, and emit an external slow path call at \p externBinOp.
  /// \param externBinOp an address in the constant pool whose value points to a
  /// slow-path binary operation function defined in ExternalCalls.h
  Emitters
  callSlowPathBinOp(Emitters emit, const Inst *ip, const uint8_t *externBinOp);

  /// Load rsi and rdx with the second and third operand of the equality
  /// test/jump instructions, emit an external call to
  /// externAbstractEqualityTest, and check the returned ExecutionStatus.
  Emitters
  callExternEqTest(Emitters emit, const Inst *ip, uint32_t reg1, uint32_t reg2);

  /// Load rdi and rsi with the second and third operand of the strict equality
  /// test/jump instructions, and emit an external call to StrictEqualityTest.
  Emitters callExternStrictEqTest(
      Emitters emit,
      const Inst *ip,
      uint32_t reg1,
      uint32_t reg2);

  /// Emit a jump to a bytecode block.
  /// Receives and \returns the fast path emitter.
  Emitter jmpToBytecodeBB(Emitter emit, unsigned bytecodeBB);

  /// Emit a conditional jump to a bytecode block.
  /// Receives and \returns the fast path emitter.
  Emitter cjmpToBytecodeBB(Emitter emit, uint8_t opCode, unsigned bytecodeBB);

  Emitters
  getByIdHelper(Emitters emit, const Inst *ip, bool tryProp, uint32_t idVal);
  Emitters
  putByIdHelper(Emitters emit, const Inst *ip, bool tryProp, uint32_t idVal);
  Emitters callHelper(
      Emitters emit,
      const Inst *ip,
      uint32_t argCount,
      bool isConstruct);
  Emitters jmpUndefinedHelper(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t regIndex);

  /// When \p regIndex is the result reg:
  /// In the fast path:
  ///     f1) Load the value in hermes reg \p regIndex to %rax.
  ///     f2) Emit a check that if %rax has an object tag: if yes, do nothing;
  ///         if not, jump to slow path.
  /// In the slow path:
  ///     s1) firstly emit a check that if \p regIndex has a null or undefined
  ///         tag: if not, jump to s2); if yes, move runtime->global_ to result
  ///         reg.
  ///     s2) Emit a slow path call to slowPathCoerceThis, store the return
  ///         value to result reg.
  /// When \p regIndex is not the result reg:
  /// In the fast path:
  ///     f1) Load the value in hermes reg \p regIndex to %rax, and also save it
  ///         in %rdx.
  ///     f2) Emit a check that if %rax has an object tag.
  ///     f3a) If not, jump to slow path.
  ///     f3b) If yes, move %rdx to the result reg.
  /// In the slow path:
  ///     s1) firstly emit a check that if \p regIndex has a null or undefined
  ///         tag: if yes, move runtime->global to %rdx, jump back to f3b);
  ///     s2) Emit a slow path call to slowPathCoerceThis, the returned value is
  ///         in %rdx, jump back to f3b).
  Emitters coerceThisHelper(Emitters emit, const Inst *ip, uint32_t regIndex);

  /// Emit an external call to Interpreter::createObjectFromBuffer.
  /// \param keyIdx index in the object key buffer table.
  /// \param valIdx index in the object value buffer table.
  Emitters newObjectWithBufferHelper(
      Emitters emit,
      const Inst *ip,
      uint32_t keyIdx,
      uint32_t valIdx);

  /// Emit an external call to externStoreToEnvironment or
  /// externStoreNPToEnvironment depending on if the value \p op3 is a pointer.
  /// \param op1 the Hermes reg containing the environment
  /// \param idx the environment index slot number
  /// \param op3 the Hermes reg containing the value to be stored
  /// \param isNP if the current instruction is StoreNPToEnvironment
  Emitters storeToEnvironmentHelper(
      Emitters emit,
      uint32_t op1,
      uint32_t idx,
      uint32_t op3,
      bool isNP);

  // Individual instruction emitters
  Emitters compileTypeOf(Emitters emit, const Inst *ip);

  Emitters compileGetById(Emitters emit, const Inst *ip);
  Emitters compileGetByIdLong(Emitters emit, const Inst *ip);
  Emitters compileGetByIdShort(Emitters emit, const Inst *ip);
  Emitters compileTryGetById(Emitters emit, const Inst *ip);
  Emitters compileTryGetByIdLong(Emitters emit, const Inst *ip);

  Emitters compilePutById(Emitters emit, const Inst *ip);
  Emitters compilePutByIdLong(Emitters emit, const Inst *ip);
  Emitters compileTryPutById(Emitters emit, const Inst *ip);
  Emitters compileTryPutByIdLong(Emitters emit, const Inst *ip);

  Emitters compileCall(Emitters emit, const Inst *ip);
  Emitters compileCallLong(Emitters emit, const Inst *ip);
  Emitters compileConstruct(Emitters emit, const Inst *ip);
  Emitters compileConstructLong(Emitters emit, const Inst *ip);

  /// Emit a check that whether the value in the Hermes register \p regIndex is
  /// a number; if not, emit a jump to the slow path \p callStub.
  Emitter isNumber(Emitter emit, uint32_t regIndex, uint8_t *callStub);

  /// Emit a check that whether the value in the Hermes register \p regIndex is
  /// a string; if not, emit a jump to the slow path \p callStub.
  Emitter isString(Emitter emit, uint32_t regIndex, uint8_t *callStub);

  /// Emit a comparison between the higher 32 bits of the value in the Hermes
  /// register \p regIndex and the given \p tag. The given \p tag could only be
  /// a non-pointer tag. The comparison instruction will set the flag registers
  /// based on the result, and the caller could then perform accordingly.
  Emitter cmpSomeNPTag(Emitter emit, uint32_t regIndex, uint32_t tag);

  /// Load the HermesValue in \p regIndex to a temporary register i.e. %rax,
  /// right shift %rax by HermesValue::kNumDataBits bits, then emit a comparison
  /// between it and the given pointer type (object/str) tag \p tag.
  /// Technically, this function could also be used to compare non-pointer tags,
  /// but it will not be as efficient as cmpSomeNPTag.
  Emitter cmpSomePointerTag(Emitter emit, uint32_t regIndex, TagKind tag);

  /// If a number is undefined or null, its highest 17 bits is 0xfff90 or
  /// 0xfff91, so we only need to check if its highest 16 bits are 0xfff9.
  /// This function emit a comparison between the highest 16 bits of a Hermes
  /// reg \p regIdx and 0xfff9, then the caller could perform based on the
  /// result accordingly.
  Emitter cmpNullOrUndefinedTag(Emitter emit, uint32_t regIdx);

  // Individual instruction emitters.
  Emitters compileDeclareGlobalVar(Emitters emit, const Inst *ip);
  Emitters compileCreateEnvironment(Emitters emit, const Inst *ip);
  Emitters compileCreateClosure(Emitters emit, const Inst *ip);
  Emitters compileGetGlobalObject(Emitters emit, const Inst *ip);
  Emitters compileLoadConstZero(Emitters emit, const Inst *ip);
  Emitters compileLoadParam(Emitters emit, const Inst *ip);
  Emitters compileBinOp(
      Emitters emit,
      const Inst *ip,
      void *slowPathBinOp,
      compileBinOpNPtr binOpNPtr);
  Emitters compileAddN(Emitters emit, const Inst *ip);
  Emitters compileSubN(Emitters emit, const Inst *ip);
  Emitters compileMulN(Emitters emit, const Inst *ip);
  Emitters compileDivN(Emitters emit, const Inst *ip);
  Emitters compileMov(Emitters emit, const Inst *ip);
  Emitters compileMovLong(Emitters emit, const Inst *ip);
  Emitters compileToNumber(Emitters emit, const Inst *ip);
  Emitters compileAddEmptyString(Emitters emit, const Inst *ip);
  Emitters compileRet(Emitters emit, const Inst *ip);
  Emitters compileCondJumpN(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t reg1,
      uint32_t reg2,
      uint8_t opCode);
  Emitters compileCondJump(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t reg1,
      uint32_t reg2,
      uint8_t opCode,
      void *slowPathCall);
  Emitters
  compileLoadConstString(Emitters emit, const Inst *ip, uint32_t stringID);
  Emitters compileStrictEqJump(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t reg1,
      uint32_t reg2,
      uint8_t opCode);
  Emitters compileEqJump(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t reg1,
      uint32_t reg2,
      uint8_t opCode);

  /// Fast path emits a check that if the value in hermes reg \p regIdx is a
  /// bool HermesValue: if yes, directly jump according to the bool value; if
  /// not, jump to the slow path.
  /// Slow path emits an external call to Operation::toBoolean.
  Emitters compileBoolJmp(
      Emitters emit,
      const Inst *ip,
      uint32_t ipOffset,
      uint32_t regIdx,
      uint8_t opCode);
  Emitters compileJmpUndefined(Emitters emit, const Inst *ip);
  Emitters compileJmpUndefinedLong(Emitters emit, const Inst *ip);
  Emitters compileEqTest(Emitters emit, const Inst *ip, bool isNeq);
  Emitters compileStrictEqTest(Emitters emit, const Inst *ip, bool isNeq);
  Emitters compileJmp(Emitters emit, const Inst *ip, uint32_t ipOffset);
  Emitters compileCondOp(
      Emitters emit,
      const Inst *ip,
      uint8_t opCode,
      void *slowPathCall);
  Emitters compileCondOpN(Emitters emit, const Inst *ip, uint8_t opCode);
  Emitters compileNewObject(Emitters emit, const Inst *ip);

  /// Compile instructions with the layout (name, Reg8, Reg8, Reg8).
  /// Load rsi and rdx with the second and third operand, and emit an external
  /// call at \p externCallAddr.
  Emitters
  compile3RegsInst(Emitters emit, const Inst *ip, void *externCallAddr);

  Emitters compileSelectObject(Emitters emit, const Inst *ip);
  Emitters compileNewArray(Emitters emit, const Inst *ip);
  Emitters
  compileNewArrayWithBuffer(Emitters emit, const Inst *ip, uint32_t idx);
  Emitters compilePutOwnByIndex(Emitters emit, const Inst *ip, uint32_t idx);
  Emitters compilePutNewOwnById(Emitters emit, const Inst *ip, uint32_t idx);
  Emitters compileLoadThisNS(Emitters emit, const Inst *ip);
  Emitters compileCoerceThisNS(Emitters emit, const Inst *ip);

  /// \return the corresponding catch handler basic block index if it exists;
  /// or \return the index of the exit block if not. (The caller will continue
  /// to recursively finding the catch hanlder.)
  unsigned getCatchHandlerBBIndex(const Inst *ip);
  Emitters compileCatch(Emitters emit, const Inst *ip);
  Emitters compileThrow(Emitters emit, const Inst *ip);
  Emitters compileNewObjectWithBuffer(Emitters emit, const Inst *ip);
  Emitters compileNewObjectWithBufferLong(Emitters emit, const Inst *ip);
  Emitters compilePutByVal(Emitters emit, const Inst *ip);
  Emitters compileDelByVal(Emitters emit, const Inst *ip);
  Emitters compileStoreToEnvironment(Emitters emit, const Inst *ip);
  Emitters compileStoreToEnvironmentL(Emitters emit, const Inst *ip);
  Emitters compileStoreNPToEnvironment(Emitters emit, const Inst *ip);
  Emitters compileStoreNPToEnvironmentL(Emitters emit, const Inst *ip);
  Emitters
  compileLoadFromEnvironment(Emitters emit, const Inst *ip, uint32_t idx);
  Emitters compileNot(Emitters emit, const Inst *ip);
  Emitters compileGetEnvironment(Emitters emit, const Inst *ip);
  Emitters compileNegate(Emitters emit, const Inst *ip);
  Emitters compileGetPNameList(Emitters emit, const Inst *ip);
  Emitters compileGetNextPName(Emitters emit, const Inst *ip);
  Emitters compileReifyArguments(Emitters emit, const Inst *ip);
  Emitters compileGetArgumentsPropByVal(Emitters emit, const Inst *ip);
  Emitters compileBitNot(Emitters emit, const Inst *ip);
  Emitters compileGetArgumentsLength(Emitters emit, const Inst *ip);
  Emitters compileCreateRegExp(Emitters emit, const Inst *ip);

  /// @}

 private:
  /// The JITContect we are associated with.
  JITContext *const context_;
  /// The CodeBlock we are compiling.
  CodeBlock *const codeBlock_;

  /// Minimum number of instruction buffer space we need available at any
  /// point.
  static constexpr unsigned kMinInstructionSpace = 1024;

  /// The starting offset of every bytecode basic block in order. The last
  /// entry is the end of the bytecode.
  std::vector<uint32_t> bcBasicBlocks_{};

  /// Map from a bytecode target label offset to a basic block index.
  llvm::DenseMap<uint32_t, unsigned> bcLabels_{};

  /// The native code offset of every compiled bc BB.
  std::vector<uint8_t *> nativeBBAddress_{};

  /// Relocations.
  std::vector<Relo> relocs_{};

  /// Index of the bytecode basic block (in \c bcBasicBlocks_) that we are
  /// currently compiling.
  unsigned curBytecodeBBIndex_ = 0;

  /// Set if an error occurred.
  bool error_ = false;
  /// Optional error message, set the first time we record an error.
  std::string errorMsg_{};

  // The fast-path execution region.
  llvm::MutableArrayRef<uint8_t> fast_;
  // The slow-path execution region.
  llvm::MutableArrayRef<uint8_t> slow_;

  llvm::DenseMap<DenseUInt64, uint8_t *> doubleConstants_{};

#ifndef NDEBUG
  /// A section describing a section of code for disassembly.
  struct Section {
    // Is the section code or data.
    bool data;
    const uint8_t *start;
    const uint8_t *end;

    Section(bool data, uint8_t const *start, uint8_t const *end)
        : data(data), start(start), end(end) {}
  };

  // Keep track of slow path sections for disassembly.
  std::vector<Section> slowPathSections_{};
#endif
};

inline bool FastJIT::checkSpace(const Emitters &emit) {
  if (LLVM_UNLIKELY(fast_.end() - emit.fast.current() < kMinInstructionSpace)) {
    error("fast-path overflow");
    return false;
  }
  if (LLVM_UNLIKELY(slow_.end() - emit.slow.current() < kMinInstructionSpace)) {
    error("slow-path overflow");
    return false;
  }
  return true;
}

/// Callee-save register pointing to "Runtime" throughout the function.
constexpr auto RegRuntime = Reg::rbx;
/// Callee-save register pointing to the first local Hermes register.
constexpr auto RegFrame = Reg::r15;

} // namespace x86_64
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_X86_64_FASTJIT_H
