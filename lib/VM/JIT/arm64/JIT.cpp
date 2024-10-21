/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "hermes/VM/JIT/arm64/JIT.h"

#include "JitEmitter.h"

#include "hermes/Inst/InstDecode.h"
#include "hermes/VM/JIT/DiscoverBB.h"
#include "hermes/VM/RuntimeModule.h"

#define DEBUG_TYPE "jit"

namespace hermes {
namespace vm {
namespace arm64 {

class JITContext::Impl {
 public:
  asmjit::JitRuntime jr{};
};

JITContext::JITContext(bool enable) : enabled_(enable) {
  if (!enable)
    return;
  impl_ = std::make_unique<Impl>();
}

JITContext::~JITContext() = default;

// Calculate the address of the next instruction given the name of the
// current one.
#define NEXTINST(name) ((const inst::Inst *)(&ip->i##name + 1))

// Add an arbitrary byte offset to ip.
#define IPADD(val) ((const inst::Inst *)((const uint8_t *)ip + (val)))

/// Map from a string ID encoded in the operand to an SHSymbolID.
/// This string ID must be used explicitly as identifier.
#define ID(stringID)                    \
  (codeBlock_->getRuntimeModule()       \
       ->getSymbolIDMustExist(stringID) \
       .unsafeGetIndex())

/// JIT_INLINE forces some methods to be inlined, but only in release mode.
#ifdef NDEBUG
#define JIT_INLINE LLVM_ATTRIBUTE_ALWAYS_INLINE inline
#else
#define JIT_INLINE inline
#endif

class JITContext::Compiler {
  JITContext &jc_;
  /// The implementation of the assembly emitter.
  Emitter em_;
  /// The CodeBlock compiled by this instance.
  CodeBlock *const codeBlock_;
  /// Pointer to the first bytecode instruction.
  const char *const funcStart_;
  /// The byte offset of every bytecode basic block start. The last entry is
  /// the exclusive end of the bytecode.
  std::vector<uint32_t> basicBlocks_{};
  /// Map bytecode offset to a basic block.
  llvh::DenseMap<uint32_t, unsigned> ofsToBBIndex_{};
  /// The ASMJIT label associated with every basic block.
  std::vector<asmjit::Label> bbLabels_{};
  /// The function name for debugging.
  std::string funcName_{};

  /// Jump buffer used for errors.
  jmp_buf errorJmpBuf_{};

  enum class Error {
    NoError,
    UnsupportedInst,
    Other,
  };
  /// In case of error, the error reason is stored here.
  Error error_ = Error::NoError;
  /// In case of "other" error, the error message is recorded here.
  std::string otherErrorMessage_{};

 public:
  Compiler(JITContext &jc, CodeBlock *codeBlock)
      : jc_(jc),
        em_(jc.impl_->jr,
            jc.getDumpJITCode(),
            codeBlock,
            codeBlock->propertyCache(),
            codeBlock->writePropertyCache(),
            // TODO: is getFrameSize() the right thing to call?
            codeBlock->getFrameSize(),
            [this](std::string &&message) {
              otherErrorMessage_ = std::move(message);
              error_ = Error::Other;
              _longjmp(errorJmpBuf_, 1);
            }),
        codeBlock_(codeBlock),
        funcStart_((const char *)codeBlock->begin()) {}

  /// Compile the codeblock that this object was instantiated for. On failure,
  /// set the "don't JIT" flag of the codeblock.
  /// \return the compiled native function or nullptr.
  JITCompiledFunctionPtr compileCodeBlock();

 private:
  /// Compile the codeblock that this object was instantiated for. On failure,
  /// longjmp(errorJmpBuf).
  /// \return the compiled native function.
  JITCompiledFunctionPtr compileCodeBlockImpl();

  /// Compile the basic block with index \p bbIndex.
  JIT_INLINE void compileBB(uint32_t bbIndex) {
    uint32_t startOfs = basicBlocks_[bbIndex];
    uint32_t endOfs = basicBlocks_[bbIndex + 1];
    em_.newBasicBlock(bbLabels_[bbIndex]);
    auto *ip = reinterpret_cast<const inst::Inst *>(funcStart_ + startOfs);
    auto *to = reinterpret_cast<const inst::Inst *>(funcStart_ + endOfs);

    while (ip != to) {
      em_.emittingIP = ip;
      ip = dispatch(ip);
      em_.assertPostInstructionInvariants();
    }
    em_.emittingIP = nullptr;
  }

  /// Compile a single instruction by dispatching to its emitter method.
  /// \return the instruction pointer for the next instruction.
  JIT_INLINE const inst::Inst *dispatch(const inst::Inst *ip) {
    switch (ip->opCode) {
#define DEFINE_OPCODE(name)   \
  case inst::OpCode::name:    \
    emit##name(&ip->i##name); \
    ip = NEXTINST(name);      \
    break;
#include "hermes/BCGen/HBC/BytecodeList.def"

#undef DEFINE_OPCODE

      case inst::OpCode::_last:
      default:
        hermes_fatal("Invalid opcode");
    }
    return ip;
  }

  /// Calculate the target branch offset relative to the current instruction
  /// and return the AsmJit label associated with the BB starting at that
  /// address.
  ///
  /// \param inst pointer to the start of he current instruction.
  /// \param targetOfs offset of the branch targer relative to \p inst.
  ///
  /// \return the corresponding AsmJit label.
  const asmjit::Label &bbLabelFromInst(const void *inst, int32_t targetOfs)
      const {
    uint32_t addr = (const char *)inst - funcStart_ + targetOfs;
    auto bbIndexIt = ofsToBBIndex_.find(addr);
    if (LLVM_UNLIKELY(bbIndexIt == ofsToBBIndex_.end())) {
      llvh::errs() << "bbLabelFromInst: invalid addr "
                   << llvh::format_hex(addr, 4) << "\n";
      hermes_fatal("jit: invalid BB addr");
    }
    return bbLabels_[bbIndexIt->second];
  }

#define DEFINE_OPCODE(name) \
  JIT_INLINE void emit##name(const inst::name##Inst *inst);
#include "hermes/BCGen/HBC/BytecodeList.def"

#undef DEFINE_OPCODE
}; // class

JITCompiledFunctionPtr JITContext::compileImpl(
    Runtime &runtime,
    CodeBlock *codeBlock_) {
  Compiler compiler(*this, codeBlock_);
  return compiler.compileCodeBlock();
}

JITCompiledFunctionPtr JITContext::Compiler::compileCodeBlock() {
  if (_setjmp(errorJmpBuf_) == 0) {
    return compileCodeBlockImpl();
  } else {
    // We arrive here on error.

    const char *errMsg = error_ == Error::UnsupportedInst
        ? "unsupported instruction"
        : otherErrorMessage_.c_str();
    auto printError = [this, errMsg](llvh::raw_ostream &OS) {
      OS << "jit error: " << errMsg << '\n';
      if (em_.emittingIP) {
        OS << "Emitting:\n";
        OS << llvh::format_decimal(
                  (const char *)em_.emittingIP - (const char *)funcStart_, 3)
           << ": " << inst::decodeInstruction(em_.emittingIP) << "\n";
      }
    };

    if (jc_.crashOnError_) {
      printError(llvh::errs());
      hermes_fatal(errMsg);
    } else {
      if (jc_.dumpJITCode_ &
          (DumpJitCode::Code | DumpJitCode::CompileStatus |
           DumpJitCode::InstErr)) {
        printError(llvh::outs());
      } else {
        LLVM_DEBUG(printError(llvh::outs()));
      }
    }

    codeBlock_->setDontJIT(true);
    return nullptr;
  }
}

JITCompiledFunctionPtr JITContext::Compiler::compileCodeBlockImpl() {
  if (jc_.dumpJITCode_ & (DumpJitCode::Code | DumpJitCode::CompileStatus)) {
    funcName_ = codeBlock_->getNameString();
    llvh::outs() << "\nJIT compilation of FunctionID "
                 << codeBlock_->getFunctionID() << ", '" << funcName_ << "'\n";
  }

  discoverBasicBlocks(codeBlock_, basicBlocks_, ofsToBBIndex_);

  if ((jc_.dumpJITCode_ & DumpJitCode::Code) && !funcName_.empty())
    llvh::outs() << "\n" << funcName_ << ":\n";

  bbLabels_.reserve(basicBlocks_.size() - 1);
  for (unsigned bbIndex = 0, e = basicBlocks_.size() - 1; bbIndex < e;
       ++bbIndex) {
    bbLabels_.push_back(em_.newPrefLabel("BB", bbIndex));
  }

  // Any code emitted at the start gets treated as the first instruction.
  em_.emittingIP = (const inst::Inst *)codeBlock_->begin();
  em_.enter(
      codeBlock_->getFunctionHeader().numberRegCount(),
      codeBlock_->getFunctionHeader().nonPtrRegCount());

  for (uint32_t bbIndex = 0, e = basicBlocks_.size() - 1; bbIndex < e;
       ++bbIndex) {
    compileBB(bbIndex);
  }

  em_.leave();
  codeBlock_->setJITCompiled(em_.addToRuntime(jc_.impl_->jr));

  LLVM_DEBUG(
      llvh::outs() << "\n Bytecode:";
      for (unsigned bbIndex = 0; bbIndex < basicBlocks_.size() - 1; ++bbIndex) {
        uint32_t startOfs = basicBlocks_[bbIndex];
        uint32_t endOfs = basicBlocks_[bbIndex + 1];
        llvh::outs() << "BB" << bbIndex << ":\n";
        auto *ip = funcStart_ + startOfs;
        auto *to = funcStart_ + endOfs;
        while (ip != to) {
          auto di = inst::decodeInstruction((const inst::Inst *)ip);
          llvh::outs() << "    " << llvh::format_decimal(ip - funcStart_, 3)
                       << ": " << di << "\n";
          ip += di.meta.size;
        }
      });

  if (jc_.dumpJITCode_ & (DumpJitCode::Code | DumpJitCode::CompileStatus)) {
    llvh::outs() << "\nJIT successfully compiled FunctionID "
                 << codeBlock_->getFunctionID() << ", '" << funcName_ << "'\n";
  }

  return codeBlock_->getJITCompiled();
}

#define EMIT_UNIMPLEMENTED(name)                                               \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    error_ = Error::UnsupportedInst;                                           \
    _longjmp(errorJmpBuf_, 1);                                                 \
  }

EMIT_UNIMPLEMENTED(GetEnvironment)
EMIT_UNIMPLEMENTED(Catch)
EMIT_UNIMPLEMENTED(DirectEval)
EMIT_UNIMPLEMENTED(AsyncBreakCheck)

#undef EMIT_UNIMPLEMENTED

inline void JITContext::Compiler::emitUnreachable(
    const inst::UnreachableInst *inst) {
  em_.unreachable();
}

inline void JITContext::Compiler::emitProfilePoint(
    const inst::ProfilePointInst *inst) {
  em_.profilePoint(inst->op1);
}

inline void JITContext::Compiler::emitLoadParam(
    const inst::LoadParamInst *inst) {
  em_.loadParam(FR(inst->op1), inst->op2);
}
inline void JITContext::Compiler::emitLoadParamLong(
    const inst::LoadParamLongInst *inst) {
  em_.loadParam(FR(inst->op1), inst->op2);
}

inline void JITContext::Compiler::emitLoadConstZero(
    const inst::LoadConstZeroInst *inst) {
  em_.loadConstDouble(FR(inst->op1), 0, "Zero");
}

inline void JITContext::Compiler::emitLoadConstUInt8(
    const inst::LoadConstUInt8Inst *inst) {
  em_.loadConstDouble(FR(inst->op1), inst->op2, "UInt8");
}

inline void JITContext::Compiler::emitLoadConstInt(
    const inst::LoadConstIntInst *inst) {
  em_.loadConstDouble(FR(inst->op1), inst->op2, "Int");
}

inline void JITContext::Compiler::emitLoadConstDouble(
    const inst::LoadConstDoubleInst *inst) {
  em_.loadConstDouble(FR(inst->op1), inst->op2, "Double");
}

#define EMIT_LOAD_CONST(NAME, val, type)                  \
  inline void JITContext::Compiler::emitLoadConst##NAME(  \
      const inst::LoadConst##NAME##Inst *inst) {          \
    em_.loadConstBits64(FR(inst->op1), val, type, #NAME); \
  }

EMIT_LOAD_CONST(Empty, _sh_ljs_empty().raw, FRType::UnknownNonPtr);
EMIT_LOAD_CONST(Undefined, _sh_ljs_undefined().raw, FRType::UnknownNonPtr);
EMIT_LOAD_CONST(Null, _sh_ljs_null().raw, FRType::UnknownNonPtr);
EMIT_LOAD_CONST(True, _sh_ljs_bool(true).raw, FRType::Bool);
EMIT_LOAD_CONST(False, _sh_ljs_bool(false).raw, FRType::Bool);

#undef EMIT_LOAD_CONST

inline void JITContext::Compiler::emitLoadConstString(
    const inst::LoadConstStringInst *inst) {
  em_.loadConstString(FR(inst->op1), codeBlock_->getRuntimeModule(), inst->op2);
}

#define EMIT_LOAD_CONST_BIGINT(name)                                           \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.loadConstBigInt(                                                       \
        FR(inst->op1), codeBlock_->getRuntimeModule(), inst->op2);             \
  }

EMIT_LOAD_CONST_BIGINT(LoadConstBigInt);
EMIT_LOAD_CONST_BIGINT(LoadConstBigIntLongIndex);

#undef EMIT_LOAD_CONST_BIGINT

inline void JITContext::Compiler::emitLoadConstStringLongIndex(
    const inst::LoadConstStringLongIndexInst *inst) {
  em_.loadConstString(FR(inst->op1), codeBlock_->getRuntimeModule(), inst->op2);
}

inline void JITContext::Compiler::emitMov(const inst::MovInst *inst) {
  em_.mov(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitMovLong(const inst::MovLongInst *inst) {
  em_.mov(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitToNumber(const inst::ToNumberInst *inst) {
  em_.toNumber(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitToNumeric(
    const inst::ToNumericInst *inst) {
  em_.toNumeric(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitToInt32(const inst::ToInt32Inst *inst) {
  em_.toInt32(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitAddEmptyString(
    const inst::AddEmptyStringInst *inst) {
  em_.addEmptyString(FR(inst->op1), FR(inst->op2));
}
#define EMIT_BINARY_OP(NAME, op)                                               \
  inline void JITContext::Compiler::emit##NAME(const inst::NAME##Inst *inst) { \
    em_.op(FR(inst->op1), FR(inst->op2), FR(inst->op3));                       \
  }

EMIT_BINARY_OP(Greater, greater)
EMIT_BINARY_OP(Less, less)
EMIT_BINARY_OP(Eq, equal)
EMIT_BINARY_OP(Neq, notEqual)
EMIT_BINARY_OP(StrictEq, strictEqual)
EMIT_BINARY_OP(StrictNeq, strictNotEqual)

EMIT_BINARY_OP(GreaterEq, greaterEqual)
EMIT_BINARY_OP(LessEq, lessEqual)

EMIT_BINARY_OP(Add, add)
EMIT_BINARY_OP(AddN, addN)
EMIT_BINARY_OP(Sub, sub)
EMIT_BINARY_OP(SubN, subN)
EMIT_BINARY_OP(Mul, mul)
EMIT_BINARY_OP(MulN, mulN)
EMIT_BINARY_OP(Div, div)
EMIT_BINARY_OP(DivN, divN)
EMIT_BINARY_OP(BitAnd, bitAnd)
EMIT_BINARY_OP(BitOr, bitOr)
EMIT_BINARY_OP(BitXor, bitXor)
EMIT_BINARY_OP(LShift, lShift)
EMIT_BINARY_OP(RShift, rShift)
EMIT_BINARY_OP(URshift, urShift)

#undef EMIT_BINARY_OP
inline void JITContext::Compiler::emitMod(const inst::ModInst *inst) {
  em_.mod(false, FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

#define EMIT_UNARY_OP(NAME, op)                                                \
  inline void JITContext::Compiler::emit##NAME(const inst::NAME##Inst *inst) { \
    em_.op(FR(inst->op1), FR(inst->op2));                                      \
  }

EMIT_UNARY_OP(Inc, inc)
EMIT_UNARY_OP(Dec, dec)
EMIT_UNARY_OP(Not, booleanNot)
EMIT_UNARY_OP(BitNot, bitNot)
EMIT_UNARY_OP(Negate, negate)
EMIT_UNARY_OP(TypeOf, typeOf)

#undef EMIT_UNARY_OP

#define EMIT_JCOND(name, impl, invert)                                         \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.impl(                                                                  \
        invert,                                                                \
        bbLabelFromInst(inst, inst->op1),                                      \
        FR(inst->op2),                                                         \
        FR(inst->op3));                                                        \
  }                                                                            \
  inline void JITContext::Compiler::emit##name##Long(                          \
      const inst::name##LongInst *inst) {                                      \
    em_.impl(                                                                  \
        invert,                                                                \
        bbLabelFromInst(inst, inst->op1),                                      \
        FR(inst->op2),                                                         \
        FR(inst->op3));                                                        \
  }

EMIT_JCOND(JLessEqual, jLessEqual, false)
EMIT_JCOND(JLessEqualN, jLessEqualN, false)
EMIT_JCOND(JNotLessEqual, jLessEqual, true)
EMIT_JCOND(JNotLessEqualN, jLessEqualN, true)
EMIT_JCOND(JLess, jLess, false)
EMIT_JCOND(JLessN, jLessN, false)
EMIT_JCOND(JNotLess, jLess, true)
EMIT_JCOND(JNotLessN, jLessN, true)
EMIT_JCOND(JGreaterEqual, jGreaterEqual, false)
EMIT_JCOND(JGreaterEqualN, jGreaterEqualN, false)
EMIT_JCOND(JNotGreaterEqual, jGreaterEqual, true)
EMIT_JCOND(JNotGreaterEqualN, jGreaterEqualN, true)
EMIT_JCOND(JGreater, jGreater, false)
EMIT_JCOND(JGreaterN, jGreaterN, false)
EMIT_JCOND(JNotGreater, jGreater, true)
EMIT_JCOND(JNotGreaterN, jGreaterN, true)
EMIT_JCOND(JEqual, jEqual, false)
EMIT_JCOND(JNotEqual, jEqual, true)
EMIT_JCOND(JStrictEqual, jStrictEqual, false)
EMIT_JCOND(JStrictNotEqual, jStrictEqual, true)

#undef EMIT_JCOND

#define EMIT_JMP_TRUE_FALSE(name, onTrue)                                      \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.jmpTrueFalse(onTrue, bbLabelFromInst(inst, inst->op1), FR(inst->op2)); \
  }                                                                            \
  inline void JITContext::Compiler::emit##name##Long(                          \
      const inst::name##LongInst *inst) {                                      \
    em_.jmpTrueFalse(onTrue, bbLabelFromInst(inst, inst->op1), FR(inst->op2)); \
  }

EMIT_JMP_TRUE_FALSE(JmpTrue, true)
EMIT_JMP_TRUE_FALSE(JmpFalse, false)
#undef EMIT_JMP_TRUE_FALSE
inline void JITContext::Compiler::emitJmpUndefined(
    const inst::JmpUndefinedInst *inst) {
  em_.jmpUndefined(bbLabelFromInst(inst, inst->op1), FR(inst->op2));
}
inline void JITContext::Compiler::emitJmpUndefinedLong(
    const inst::JmpUndefinedLongInst *inst) {
  em_.jmpUndefined(bbLabelFromInst(inst, inst->op1), FR(inst->op2));
}

#define EMIT_JMP_NO_COND(name)                                                 \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.jmp(bbLabelFromInst(inst, inst->op1));                                 \
  }                                                                            \
  inline void JITContext::Compiler::emit##name##Long(                          \
      const inst::name##LongInst *inst) {                                      \
    em_.jmp(bbLabelFromInst(inst, inst->op1));                                 \
  }

EMIT_JMP_NO_COND(Jmp)

#undef EMIT_JMP_NO_COND

inline void JITContext::Compiler::emitSwitchImm(
    const inst::SwitchImmInst *inst) {
  uint32_t min = inst->op4;
  uint32_t max = inst->op5;
  // Max is inclusive, so add 1 to get the number of entries.
  uint32_t entries = max - min + 1;

  // Calculate the offset into the bytecode where the jump table for
  // this SwitchImm starts.
  const uint8_t *tablestart = (const uint8_t *)llvh::alignAddr(
      (const uint8_t *)inst + inst->op2, sizeof(uint32_t));

  std::vector<const asmjit::Label *> jumpTableLabels{};
  jumpTableLabels.reserve(entries);

  // Add a label for each offset in the table.
  for (uint32_t i = 0; i < entries; ++i) {
    const int32_t *loc = (const int32_t *)tablestart + i;
    int32_t offset = *loc;
    jumpTableLabels.push_back(&bbLabelFromInst(inst, offset));
  }

  em_.switchImm(
      FR(inst->op1),
      bbLabelFromInst(inst, inst->op3),
      jumpTableLabels,
      min,
      max);
}

inline void JITContext::Compiler::emitTryGetByIdLong(
    const inst::TryGetByIdLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryGetById(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitTryGetById(
    const inst::TryGetByIdInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryGetById(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitGetByIdLong(
    const inst::GetByIdLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.getById(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitGetById(const inst::GetByIdInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.getById(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitGetByIdShort(
    const inst::GetByIdShortInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.getById(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitTryPutByIdLooseLong(
    const inst::TryPutByIdLooseLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryPutByIdLoose(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitTryPutByIdLoose(
    const inst::TryPutByIdLooseInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryPutByIdLoose(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitTryPutByIdStrictLong(
    const inst::TryPutByIdStrictLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryPutByIdStrict(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitTryPutByIdStrict(
    const inst::TryPutByIdStrictInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.tryPutByIdStrict(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitPutByIdLooseLong(
    const inst::PutByIdLooseLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.putByIdLoose(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitPutByIdLoose(
    const inst::PutByIdLooseInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.putByIdLoose(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitPutByIdStrictLong(
    const inst::PutByIdStrictLongInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.putByIdStrict(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitPutByIdStrict(
    const inst::PutByIdStrictInst *inst) {
  auto idVal = ID(inst->op4);
  auto cacheIdx = inst->op3;
  em_.putByIdStrict(FR(inst->op1), idVal, FR(inst->op2), cacheIdx);
}

#define EMIT_BY_VAL(name, op)                                                  \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.op(FR(inst->op1), FR(inst->op2), FR(inst->op3));                       \
  }

EMIT_BY_VAL(GetByVal, getByVal)
EMIT_BY_VAL(PutByValLoose, putByValLoose)
EMIT_BY_VAL(PutByValStrict, putByValStrict)

#undef EMIT_BY_VAL

#define EMIT_DEL_BY_ID(name, op)                                               \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.delById##op(FR(inst->op1), FR(inst->op2), ID(inst->op3));              \
  }

EMIT_DEL_BY_ID(DelByIdLoose, Loose)
EMIT_DEL_BY_ID(DelByIdLooseLong, Loose)
EMIT_DEL_BY_ID(DelByIdStrict, Strict)
EMIT_DEL_BY_ID(DelByIdStrictLong, Strict)

#undef EMIT_DEL_BY_ID

#define EMIT_DEL_BY_VAL(name, op)                                              \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.delByVal##op(FR(inst->op1), FR(inst->op2), FR(inst->op3));             \
  }

EMIT_DEL_BY_VAL(DelByValLoose, Loose)
EMIT_DEL_BY_VAL(DelByValStrict, Strict)

#undef EMIT_DEL_BY_VAL

inline void JITContext::Compiler::emitGetByIndex(
    const inst::GetByIndexInst *inst) {
  em_.getByIndex(FR(inst->op1), FR(inst->op2), inst->op3);
}

inline void JITContext::Compiler::emitPutOwnByIndex(
    const inst::PutOwnByIndexInst *inst) {
  em_.putOwnByIndex(FR(inst->op1), FR(inst->op2), inst->op3);
}

inline void JITContext::Compiler::emitPutOwnByIndexL(
    const inst::PutOwnByIndexLInst *inst) {
  em_.putOwnByIndex(FR(inst->op1), FR(inst->op2), inst->op3);
}

inline void JITContext::Compiler::emitPutOwnByVal(
    const inst::PutOwnByValInst *inst) {
  em_.putOwnByVal(FR(inst->op1), FR(inst->op2), FR(inst->op3), (bool)inst->op4);
}

inline void JITContext::Compiler::emitPutOwnGetterSetterByVal(
    const inst::PutOwnGetterSetterByValInst *inst) {
  em_.putOwnGetterSetterByVal(
      FR(inst->op1),
      FR(inst->op2),
      FR(inst->op3),
      FR(inst->op4),
      (bool)inst->op5);
}

#define EMIT_PUT_NEW_OWN_BY_ID(name, enumerable)                               \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.putNewOwnById(                                                         \
        FR(inst->op1), FR(inst->op2), ID(inst->op3), enumerable);              \
  }

EMIT_PUT_NEW_OWN_BY_ID(PutNewOwnById, true)
EMIT_PUT_NEW_OWN_BY_ID(PutNewOwnByIdLong, true)
EMIT_PUT_NEW_OWN_BY_ID(PutNewOwnByIdShort, true)
EMIT_PUT_NEW_OWN_BY_ID(PutNewOwnNEById, false)
EMIT_PUT_NEW_OWN_BY_ID(PutNewOwnNEByIdLong, false)

#undef EMIT_PUT_NEW_OWN_BY_ID

#define EMIT_OWN_BY_SLOT_IDX(name, op)                                         \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.op(FR(inst->op1), FR(inst->op2), inst->op3);                           \
  }                                                                            \
  inline void JITContext::Compiler::emit##name##Long(                          \
      const inst::name##LongInst *inst) {                                      \
    em_.op(FR(inst->op1), FR(inst->op2), inst->op3);                           \
  }

EMIT_OWN_BY_SLOT_IDX(PutOwnBySlotIdx, putOwnBySlotIdx)
EMIT_OWN_BY_SLOT_IDX(GetOwnBySlotIdx, getOwnBySlotIdx)

#undef EMIT_OWN_BY_SLOT_IDX

inline void JITContext::Compiler::emitLoadParentNoTraps(
    const inst::LoadParentNoTrapsInst *inst) {
  em_.loadParentNoTraps(FR(inst->op1), FR(inst->op2));
}
inline void JITContext::Compiler::emitTypedLoadParent(
    const inst::TypedLoadParentInst *inst) {
  em_.typedLoadParent(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitTypedStoreParent(
    const inst::TypedStoreParentInst *inst) {
  em_.typedStoreParent(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitRet(const inst::RetInst *inst) {
  em_.ret(FR(inst->op1));
}

inline void JITContext::Compiler::emitGetGlobalObject(
    const inst::GetGlobalObjectInst *inst) {
  em_.getGlobalObject(FR(inst->op1));
}

inline void JITContext::Compiler::emitInstanceOf(
    const inst::InstanceOfInst *inst) {
  em_.instanceOf(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

inline void JITContext::Compiler::emitIsIn(const inst::IsInInst *inst) {
  em_.isIn(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

inline void JITContext::Compiler::emitCall(const inst::CallInst *inst) {
  em_.call(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* argc */ inst->op3);
}

inline void JITContext::Compiler::emitCall1(const inst::Call1Inst *inst) {
  em_.callN(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* args */ {FR(inst->op3)});
}

inline void JITContext::Compiler::emitCall2(const inst::Call2Inst *inst) {
  em_.callN(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* args */ {FR(inst->op3), FR(inst->op4)});
}

inline void JITContext::Compiler::emitCall3(const inst::Call3Inst *inst) {
  em_.callN(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* args */
      {FR(inst->op3), FR(inst->op4), FR(inst->op5)});
}

inline void JITContext::Compiler::emitCall4(const inst::Call4Inst *inst) {
  em_.callN(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* args */
      {FR(inst->op3), FR(inst->op4), FR(inst->op5), FR(inst->op6)});
}

inline void JITContext::Compiler::emitConstruct(
    const inst::ConstructInst *inst) {
  em_.callWithNewTarget(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* newTarget */ FR(inst->op2),
      /* argc */ inst->op3);
}

inline void JITContext::Compiler::emitCallBuiltin(
    const inst::CallBuiltinInst *inst) {
  em_.callBuiltin(
      FR(inst->op1),
      /* builtinIndex */ inst->op2,
      /* argc */ inst->op3);
}
inline void JITContext::Compiler::emitCallBuiltinLong(
    const inst::CallBuiltinLongInst *inst) {
  em_.callBuiltin(
      FR(inst->op1),
      /* builtinIndex */ inst->op2,
      /* argc */ inst->op3);
}

inline void JITContext::Compiler::emitCallWithNewTarget(
    const inst::CallWithNewTargetInst *inst) {
  em_.callWithNewTarget(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* newTarget */ FR(inst->op3),
      /* argc */ inst->op4);
}

inline void JITContext::Compiler::emitCallWithNewTargetLong(
    const inst::CallWithNewTargetLongInst *inst) {
  em_.callWithNewTargetLong(
      FR(inst->op1),
      /* callee */ FR(inst->op2),
      /* newTarget */ FR(inst->op3),
      /* argc */ FR(inst->op4));
}

inline void JITContext::Compiler::emitGetBuiltinClosure(
    const inst::GetBuiltinClosureInst *inst) {
  em_.getBuiltinClosure(
      FR(inst->op1),
      /* builtinIndex */ inst->op2);
}

inline void JITContext::Compiler::emitDeclareGlobalVar(
    const inst::DeclareGlobalVarInst *inst) {
  em_.declareGlobalVar(ID(inst->op1));
}

inline void JITContext::Compiler::emitCreateTopLevelEnvironment(
    const inst::CreateTopLevelEnvironmentInst *inst) {
  em_.createTopLevelEnvironment(FR(inst->op1), inst->op2);
}

inline void JITContext::Compiler::emitCreateFunctionEnvironment(
    const inst::CreateFunctionEnvironmentInst *inst) {
  em_.createFunctionEnvironment(FR(inst->op1), inst->op2);
}

inline void JITContext::Compiler::emitCreateEnvironment(
    const inst::CreateEnvironmentInst *inst) {
  em_.createEnvironment(FR(inst->op1), FR(inst->op2), inst->op3);
}

inline void JITContext::Compiler::emitGetParentEnvironment(
    const inst::GetParentEnvironmentInst *inst) {
  em_.getParentEnvironment(FR(inst->op1), inst->op2);
}

inline void JITContext::Compiler::emitGetClosureEnvironment(
    const inst::GetClosureEnvironmentInst *inst) {
  em_.getClosureEnvironment(FR(inst->op1), FR(inst->op2));
}

#define EMIT_LOAD_FROM_ENV(name)                                               \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.loadFromEnvironment(FR(inst->op1), FR(inst->op2), inst->op3);          \
  }

EMIT_LOAD_FROM_ENV(LoadFromEnvironment)
EMIT_LOAD_FROM_ENV(LoadFromEnvironmentL)

#undef EMIT_LOAD_FROM_ENV

#define EMIT_STORE_TO_ENV(name, np)                                            \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.storeToEnvironment(np, FR(inst->op1), inst->op2, FR(inst->op3));       \
  }

EMIT_STORE_TO_ENV(StoreToEnvironment, false)
EMIT_STORE_TO_ENV(StoreToEnvironmentL, false)
EMIT_STORE_TO_ENV(StoreNPToEnvironment, true)
EMIT_STORE_TO_ENV(StoreNPToEnvironmentL, true)

#undef EMIT_STORE_TO_ENV

#define EMIT_CREATE_CLOSURE(name)                                              \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.createClosure(                                                         \
        FR(inst->op1),                                                         \
        FR(inst->op2),                                                         \
        codeBlock_->getRuntimeModule(),                                        \
        inst->op3);                                                            \
  }

EMIT_CREATE_CLOSURE(CreateClosure)
EMIT_CREATE_CLOSURE(CreateClosureLongIndex)

#undef EMIT_CREATE_CLOSURE

#define EMIT_CREATE_GENERATOR(name)                                            \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.createGenerator(                                                       \
        FR(inst->op1),                                                         \
        FR(inst->op2),                                                         \
        codeBlock_->getRuntimeModule(),                                        \
        inst->op3);                                                            \
  }

EMIT_CREATE_GENERATOR(CreateGenerator)
EMIT_CREATE_GENERATOR(CreateGeneratorLongIndex)

#undef EMIT_CREATE_GENERATOR

inline void JITContext::Compiler::emitNewObject(
    const inst::NewObjectInst *inst) {
  em_.newObject(FR(inst->op1));
}

inline void JITContext::Compiler::emitNewObjectWithParent(
    const inst::NewObjectWithParentInst *inst) {
  em_.newObjectWithParent(FR(inst->op1), FR(inst->op2));
}

#define EMIT_NEW_OBJECT_WITH_BUFFER(name)                                      \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.newObjectWithBuffer(FR(inst->op1), inst->op2, inst->op3);              \
  }

EMIT_NEW_OBJECT_WITH_BUFFER(NewObjectWithBuffer)
EMIT_NEW_OBJECT_WITH_BUFFER(NewObjectWithBufferLong)

#undef EMIT_NEW_OBJECT_WITH_BUFFER

inline void JITContext::Compiler::emitNewArray(const inst::NewArrayInst *inst) {
  em_.newArray(FR(inst->op1), inst->op2);
}

#define EMIT_NEW_ARRAY_WITH_BUFFER(name)                                       \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.newArrayWithBuffer(FR(inst->op1), inst->op2, inst->op3, inst->op4);    \
  }

EMIT_NEW_ARRAY_WITH_BUFFER(NewArrayWithBuffer)
EMIT_NEW_ARRAY_WITH_BUFFER(NewArrayWithBufferLong)

#undef EMIT_NEW_ARRAY_WITH_BUFFER

inline void JITContext::Compiler::emitNewFastArray(
    const inst::NewFastArrayInst *inst) {
  em_.newFastArray(FR(inst->op1), inst->op2);
}
inline void JITContext::Compiler::emitFastArrayLength(
    const inst::FastArrayLengthInst *inst) {
  em_.fastArrayLength(FR(inst->op1), FR(inst->op2));
}
inline void JITContext::Compiler::emitFastArrayLoad(
    const inst::FastArrayLoadInst *inst) {
  em_.fastArrayLoad(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}
inline void JITContext::Compiler::emitFastArrayStore(
    const inst::FastArrayStoreInst *inst) {
  em_.fastArrayStore(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}
inline void JITContext::Compiler::emitFastArrayPush(
    const inst::FastArrayPushInst *inst) {
  em_.fastArrayPush(FR(inst->op1), FR(inst->op2));
}
inline void JITContext::Compiler::emitFastArrayAppend(
    const inst::FastArrayAppendInst *inst) {
  em_.fastArrayAppend(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitGetPNameList(
    const inst::GetPNameListInst *inst) {
  em_.getPNameList(FR(inst->op1), FR(inst->op2), FR(inst->op3), FR(inst->op4));
}

inline void JITContext::Compiler::emitGetNextPName(
    const inst::GetNextPNameInst *inst) {
  em_.getNextPName(
      FR(inst->op1),
      FR(inst->op2),
      FR(inst->op3),
      FR(inst->op4),
      FR(inst->op5));
}

inline void JITContext::Compiler::emitIteratorBegin(
    const inst::IteratorBeginInst *inst) {
  em_.iteratorBegin(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitIteratorNext(
    const inst::IteratorNextInst *inst) {
  em_.iteratorNext(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

inline void JITContext::Compiler::emitIteratorClose(
    const inst::IteratorCloseInst *inst) {
  em_.iteratorClose(FR(inst->op1), (bool)inst->op2);
}

#define EMIT_GET_ARGUMENTS_PROP_BY_VAL(name, op)                               \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.getArgumentsPropByVal##op(                                             \
        FR(inst->op1), FR(inst->op2), FR(inst->op3));                          \
  }

EMIT_GET_ARGUMENTS_PROP_BY_VAL(GetArgumentsPropByValLoose, Loose)
EMIT_GET_ARGUMENTS_PROP_BY_VAL(GetArgumentsPropByValStrict, Strict)

#undef EMIT_GET_ARGUMENTS_PROP_BY_VAL

inline void JITContext::Compiler::emitGetArgumentsLength(
    const inst::GetArgumentsLengthInst *inst) {
  em_.getArgumentsLength(FR(inst->op1), FR(inst->op2));
}

#define EMIT_REIFY_ARGUMENTS(name, op)                                         \
  inline void JITContext::Compiler::emit##name(const inst::name##Inst *inst) { \
    em_.reifyArguments##op(FR(inst->op1));                                     \
  }

EMIT_REIFY_ARGUMENTS(ReifyArgumentsLoose, Loose)
EMIT_REIFY_ARGUMENTS(ReifyArgumentsStrict, Strict)

#undef EMIT_REIFY_ARGUMENTS

inline void JITContext::Compiler::emitCreateThisForNew(
    const inst::CreateThisForNewInst *inst) {
  auto cacheIdx = inst->op3;
  em_.createThis(FR(inst->op1), FR(inst->op2), FR(inst->op2), cacheIdx);
}

inline void JITContext::Compiler::emitSelectObject(
    const inst::SelectObjectInst *inst) {
  em_.selectObject(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

inline void JITContext::Compiler::emitLoadThisNS(
    const inst::LoadThisNSInst *inst) {
  em_.loadThisNS(FR(inst->op1));
}

inline void JITContext::Compiler::emitCoerceThisNS(
    const inst::CoerceThisNSInst *inst) {
  em_.coerceThisNS(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitGetNewTarget(
    const inst::GetNewTargetInst *inst) {
  em_.getNewTarget(FR(inst->op1));
}

inline void JITContext::Compiler::emitDebugger(const inst::DebuggerInst *inst) {
  em_.debugger();
}

inline void JITContext::Compiler::emitThrow(const inst::ThrowInst *inst) {
  em_.throwInst(FR(inst->op1));
}

inline void JITContext::Compiler::emitThrowIfEmpty(
    const inst::ThrowIfEmptyInst *inst) {
  em_.throwIfEmpty(FR(inst->op1), FR(inst->op2));
}

inline void JITContext::Compiler::emitAddS(const inst::AddSInst *inst) {
  em_.addS(FR(inst->op1), FR(inst->op2), FR(inst->op3));
}

inline void JITContext::Compiler::emitCreateRegExp(
    const inst::CreateRegExpInst *inst) {
  em_.createRegExp(
      FR(inst->op1),
      codeBlock_->getRuntimeModule()
          ->getSymbolIDFromStringIDMayAllocate(inst->op2)
          .unsafeGetRaw(),
      codeBlock_->getRuntimeModule()
          ->getSymbolIDFromStringIDMayAllocate(inst->op3)
          .unsafeGetRaw(),
      inst->op4);
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMESVM_JIT
