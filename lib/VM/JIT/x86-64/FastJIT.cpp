/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FastJIT.h"

#include "../ExternalCalls.h"
#include "RuntimeOffsets.h"
#include "hermes/Inst/InstDecode.h"
#include "hermes/VM/JIT/DiscoverBB.h"
#include "hermes/VM/Operations.h"

#define DEBUG_TYPE "jit"

namespace hermes {
namespace vm {
namespace x86_64 {
using hermes::inst::Inst;

FastJIT::FastJIT(JITContext *context, CodeBlock *codeBlock)
    : context_(context), codeBlock_(codeBlock) {}

void FastJIT::compile() {
  LLVM_DEBUG(
      llvh::dbgs() << "JIT compilation of FunctionID "
                   << codeBlock_->getFunctionID() << "\n");

  discoverBasicBlocks(codeBlock_, bcBasicBlocks_, bcLabels_);

  ExecHeap::SizePair sizes;
  auto blocks = allocRWX(codeBlock_->getOpcodeArray().size(), sizes);
  if (!blocks)
    return;

  fast_ = llvh::makeMutableArrayRef(blocks->first, sizes.first);
  slow_ = llvh::makeMutableArrayRef(blocks->second, sizes.second);

  Emitters emit{Emitter{fast_.begin()}, Emitter{slow_.begin()}};
  emit = emitPrologue(emit);

  nativeBBAddress_.resize(bcBasicBlocks_.size());

  // Compile every basic block and record its starting address.
  unsigned bcBasicBlocksCount = bcBasicBlocks_.size() - 1;
  for (curBytecodeBBIndex_ = 0; curBytecodeBBIndex_ != bcBasicBlocksCount;
       ++curBytecodeBBIndex_) {
    nativeBBAddress_[curBytecodeBBIndex_] = emit.fast.current();

    emit = compileBB(emit);
    // FIXME: we should break here, but for now we continue so we can debug
    // FIXME: more instructions. We should also check error_ below to avoid
    // FIXME: doing unnecessary work.
    //    if (error_) {
    //      llvh::outs() << "FastJIT error: " << errorMsg_ << "\n";
    //      std::abort();
    //    }
  }

  // Emit the function epilogue.
  nativeBBAddress_[curBytecodeBBIndex_] = emit.fast.current();
  emit = emitEpilogue(emit);

  resolveRelocations();

  LLVM_DEBUG(disassembleResult(emit, llvh::dbgs(), true));
  if (context_->getDumpJITCode())
    disassembleResult(emit, llvh::outs(), false);

  if (!error_) {
    context_->getHeap().freeRemaining(
        *blocks,
        {emit.fast.current() - fast_.data(),
         emit.slow.current() - slow_.data()});
    codeBlock_->setJITCompiled((JITCompiledFunctionPtr)fast_.data());

    // Dump the heap at the end.
    LLVM_DEBUG(context_->getHeap().dump(llvh::dbgs()));
  } else {
    context_->getHeap().free(*blocks);
    if (context_->getCrashOnError()) {
      hermes_fatal(errorMsg_.c_str());
    }
  }
}

void FastJIT::error(const llvh::Twine &msg) {
  if (!error_)
    codeBlock_->setDontJIT(true);
  error_ = true;
  if (errorMsg_.empty())
    errorMsg_ = msg.str();
  LLVM_DEBUG(llvh::dbgs() << "FastJIT error: " << msg << "\n");
}

llvh::Optional<ExecHeap::BlockPair> FastJIT::allocRWX(
    size_t bytecodeLength,
    ExecHeap::SizePair &sizes) {
  sizes = ExecHeap::SizePair{
      bytecodeLength * 50 + kMinInstructionSpace,
      bytecodeLength * 50 + kMinInstructionSpace};

  auto blocks = context_->getHeap().alloc(sizes);
  // If the allocation failed, add a new pool, initialize it and retry.
  if (!blocks) {
    auto newPool = context_->getHeap().addPool();
    if (!newPool) {
      error("out of executable memory");
      return llvh::None;
    }
    initializeNewPool(newPool);

    blocks = newPool->alloc(sizes);
    if (!blocks) {
      error("bytecode size too large");
      return llvh::None;
    }
  }

  assert(blocks && "allocation should have succeeded");
  return blocks;
}

void FastJIT::initializeNewPool(ExecHeap::DualPool *pool) {}

void FastJIT::disassembleRange(
    const uint8_t *from,
    const uint8_t *to,
    llvh::raw_ostream &OS,
    bool withAddr) const {
  if (to != from) {
    context_->getDisassembler().disassembleBuffer(
        OS, {from, to}, from - fast_.data(), withAddr);
  }
}

void FastJIT::disassembleResult(
    Emitters emit,
    llvh::raw_ostream &OS,
    bool withAddr) const {
  OS << "\n\nCompiled Code of FunctionID: " << codeBlock_->getFunctionID()
     << "\n";
  auto *last = fast_.data();

  for (size_t i = 0; i < nativeBBAddress_.size(); ++i) {
    disassembleRange(last, nativeBBAddress_[i], OS, withAddr);
    last = nativeBBAddress_[i];
    OS << "BB" << i << ":\n";
  }
  disassembleRange(last, emit.fast.current(), OS, withAddr);

#ifndef NDEBUG
  OS << "\n;SLOW PATHS\n";

  for (const auto sec : slowPathSections_) {
    if (sec.data)
      continue;
    disassembleRange(sec.start, sec.end, OS, withAddr);
    OS << "\n";
  }
#endif
}

void FastJIT::applyRelocation(const Relo &relo, const uint8_t *target) {
  auto offset = target - relo.address;
  switch (relo.kind) {
    case ReloKind::Int8:
      offset -= 1;
      assert(
          INT8_MIN <= offset && offset <= INT8_MAX &&
          "int8 relocation overflow");
      *reinterpret_cast<uint8_t *>(relo.address) = offset;
      break;

    case ReloKind::Int32:
      offset -= 4;
      assert(
          INT32_MIN <= offset && offset <= INT32_MAX &&
          "int32 relocation overflow");
      *reinterpret_cast<uint32_t *>(relo.address) = offset;
      break;

    case ReloKind::None:
      llvm_unreachable("ReloKind::None can not be relocated. ");
  }
}

void FastJIT::resolveRelocations() {
  for (const auto &relo : relocs_)
    applyRelocation(relo, nativeBBAddress_[relo.targetBCBBIndex]);
  relocs_.clear();
}

#ifndef NDEBUG
void FastJIT::describeSlowPathSection(Emitter slow, bool data) {
  const uint8_t *last =
      slowPathSections_.empty() ? slow_.data() : slowPathSections_.back().end;

  if (slow.current() <= last)
    return;

  // Merge data sections together.
  if (data && !slowPathSections_.empty() &&
      slowPathSections_.back().data == data)
    slowPathSections_.back().end = slow.current();
  else
    slowPathSections_.emplace_back(data, last, slow.current());
}
#endif

Emitters FastJIT::emitPrologue(Emitters emit) {
  if (!checkSpace(emit))
    return emit;

  emit.fast.pushqReg(Reg::rbp);
  emit.fast.movRegToReg<S::Q>(Reg::rsp, Reg::rbp);

  // Save callee save registers.
  emit.fast.pushqReg(RegFrame);
  emit.fast.pushqReg(RegRuntime);

  // Move the first parameter (Runtime *) into its register.
  emit.fast.movRegToReg<S::Q>(Reg::rdi, RegRuntime);

  // Push runtime->currentFrame into the native stack.
  emit.fast.pushqRM(RegRuntime, Reg::NoIndex, RuntimeOffsets::currentFrame);
  // Align the native stack to 16 bytes.
  emit.fast.pushqReg(Reg::rcx);

  // Load runtime->stackPointer_ top into RegFrame
  emit.fast.movRMToReg<S::Q>(
      RegRuntime, Reg::NoIndex, RuntimeOffsets::stackPointer, RegFrame);
  // Store RegFrame into runtime->currentFrame_.
  emit.fast.movRegToRM<S::Q>(
      RegFrame, RegRuntime, Reg::NoIndex, RuntimeOffsets::currentFrame);
  // Allocate and clear registers for the frame and update the top of the stack.
  // runtime->stackPointer = RegFrame - 8*numRegsNeeded.
  const int numRegsNeeded = codeBlock_->getFrameSize() +
      StackFrameLayout::CalleeExtraRegistersAtStart;
  emit = loadConstantIntoNativeReg(
      emit, HermesValue::encodeUndefinedValue(), Reg::rax);
  for (int i = 1; i <= numRegsNeeded; ++i) {
    emit.fast.movRegToRM<S::Q>(
        Reg::rax, RegFrame, Reg::NoIndex, -i * sizeof(HermesValue));
  }
  emit.fast.leaRMToReg<S::Q>(
      RegFrame,
      Reg::NoIndex,
      -(int)sizeof(HermesValue) * numRegsNeeded,
      Reg::rax);
  emit.fast.movRegToRM<S::Q>(
      Reg::rax, RegRuntime, Reg::NoIndex, RuntimeOffsets::stackPointer);

  return emit;
}

Emitters FastJIT::emitEpilogue(Emitters emit) {
  if (!checkSpace(emit))
    return emit;

  // Restore the VM stack pointer: runtime->stackPointer = RegFrame.
  emit.fast.movRegToRM<S::Q>(
      RegFrame, RegRuntime, Reg::NoIndex, RuntimeOffsets::stackPointer);

  // Pop the stack alignment.
  emit.fast.popqReg(Reg::rcx);
  // Pop runtime->currentFrame_ from the native stack.
  emit.fast.popqRM(RegRuntime, Reg::NoIndex, RuntimeOffsets::currentFrame);

  // Restore callee saved registers.
  emit.fast.popqReg(RegRuntime);
  emit.fast.popqReg(RegFrame);

  emit.fast.popqReg(Reg::rbp);
  emit.fast.retq();

  return emit;
}

// Calculate the address of the next instruction given the name of the current
// one.
#define NEXTINST(name) ((const Inst *)(&ip->i##name + 1))

Emitters FastJIT::compileBB(Emitters emit) {
  auto *ip = reinterpret_cast<const Inst *>(
      codeBlock_->begin() + bcBasicBlocks_[curBytecodeBBIndex_]);
  auto *to = reinterpret_cast<const Inst *>(
      codeBlock_->begin() + bcBasicBlocks_[curBytecodeBBIndex_ + 1]);

  while (ip != to) {
    if (!checkSpace(emit))
      return emit;

    LLVM_DEBUG(llvh::dbgs() << ";   " << decodeInstruction(ip) << "\n");
#ifndef NDEBUG
    auto sav = emit;
#endif

    switch (ip->opCode) {
#define CASE(name)                  \
  case OpCode::name:                \
    emit = compile##name(emit, ip); \
    ip = NEXTINST(name);            \
    break

/// Implement a comparison jump with a fast path, a slow path, and a long
/// version \param name the name of the instruction. The fast path case will
/// have a
///     "N" appended to the name.
/// \param suffix  Optional suffix to be added to the end (e.g. Long)
/// \param cc the conditional code indicating when to jump.
/// \param slowPathCall function to call for the slow-path comparison.
#define JCOND_IMPL(name, suffix, cc, slowPathCall) \
  case OpCode::name##suffix:                       \
    emit = compileCondJump(                        \
        emit,                                      \
        ip,                                        \
        ip->i##name##suffix.op1,                   \
        ip->i##name##suffix.op2,                   \
        ip->i##name##suffix.op3,                   \
        CJumpOp<cc>::OP,                           \
        (void *)slowPathCall);                     \
    ip = NEXTINST(name##suffix);                   \
    break;                                         \
  case OpCode::name##N##suffix:                    \
    emit = compileCondJumpN(                       \
        emit,                                      \
        ip,                                        \
        ip->i##name##N##suffix.op1,                \
        ip->i##name##N##suffix.op2,                \
        ip->i##name##N##suffix.op3,                \
        CJumpOp<cc>::OP);                          \
    ip = NEXTINST(name##N##suffix);                \
    break

#define JCOND(name, cc, slowPathCall)   \
  JCOND_IMPL(name, , cc, slowPathCall); \
  JCOND_IMPL(name, Long, cc, slowPathCall);

/// Implement a jump based on equality test and its long version
/// \param name the name of the instruction.
/// \param cc the conditional code indicating when to jump.
/// \param compilation the compilation function for this instruction.
#define JEQ(name, cc, compilation) \
  case OpCode::name: {             \
    emit = compilation(            \
        emit,                      \
        ip,                        \
        ip->i##name.op1,           \
        ip->i##name.op2,           \
        ip->i##name.op3,           \
        CJumpOp<cc>::OP);          \
    ip = NEXTINST(name);           \
    break;                         \
  }                                \
  case OpCode::name##Long: {       \
    emit = compilation(            \
        emit,                      \
        ip,                        \
        ip->i##name##Long.op1,     \
        ip->i##name##Long.op2,     \
        ip->i##name##Long.op3,     \
        CJumpOp<cc>::OP);          \
    ip = NEXTINST(name##Long);     \
    break;                         \
  }

/// Implement a bool jump instruction and its long version.
/// \param name the name of the instruction.
/// \param cc the conditional code indicating when to jump.
#define JBOOL(name, cc)                                               \
  case OpCode::name:                                                  \
    emit = compileBoolJmp(                                            \
        emit, ip, ip->i##name.op1, ip->i##name.op2, CJumpOp<cc>::OP); \
    ip = NEXTINST(name);                                              \
    break;                                                            \
  case OpCode::name##Long:                                            \
    emit = compileBoolJmp(                                            \
        emit,                                                         \
        ip,                                                           \
        ip->i##name##Long.op1,                                        \
        ip->i##name##Long.op2,                                        \
        CJumpOp<cc>::OP);                                             \
    ip = NEXTINST(name##Long);                                        \
    break

#define BINOP(name)                                                    \
  case OpCode::name:                                                   \
    emit = compileBinOp(                                               \
        emit, ip, (void *)slowPath##name, &FastJIT::compile##name##N); \
    ip = NEXTINST(name);                                               \
    break

#define COND_OP(name, cc)                                                    \
  case OpCode::name:                                                         \
    emit = compileCondOp(emit, ip, CJumpOp<cc>::OP, (void *)slowPath##name); \
    ip = NEXTINST(name);                                                     \
    break

#define LOAD_CONST_STRING(name)                               \
  case OpCode::name:                                          \
    emit = compileLoadConstString(emit, ip, ip->i##name.op2); \
    ip = NEXTINST(name);                                      \
    break

#define LOAD_CONST_INT(name, val)                               \
  case OpCode::name:                                            \
    emit = loadHermesValueConstant(emit, ip->i##name.op1, val); \
    ip = NEXTINST(name);                                        \
    break

#define EQ_TEST(name, isNeq)                     \
  case OpCode::name:                             \
    emit = compileEqTest(emit, ip, isNeq);       \
    ip = NEXTINST(name);                         \
    break;                                       \
  case OpCode::Strict##name:                     \
    emit = compileStrictEqTest(emit, ip, isNeq); \
    ip = NEXTINST(Strict##name);                 \
    break

/// Compile an instruction and its long or short version.
/// It can only be used when the variable length operand is the last operand,
/// otherwise the offsets of other operands are wrong.
/// \param name the name of the short version instruction.
/// \param suffix  Optional suffix to be added to the end (e.g. Long, Short, L)
/// \param op the variable length operand, it could be UInt8/16/32, or Addr8/32
#define CASE_WITH_SUFFIX(name, suffix, op)                  \
  case OpCode::name##suffix:                                \
    emit = compile##name(emit, ip, ip->i##name##suffix.op); \
    ip = NEXTINST(name##suffix);                            \
    break

/// Compile instructions with the layout (name, Reg8, Reg8, Reg8)
#define CASE_3REG(name)                                      \
  case OpCode::name:                                         \
    emit = compile3RegsInst(emit, ip, (void *)extern##name); \
    ip = NEXTINST(name);                                     \
    break

      CASE(DeclareGlobalVar);
      CASE(CreateEnvironment);
      CASE(CreateClosure);
      CASE(GetGlobalObject);
      CASE(PutById);
      CASE(TryPutById);
      CASE(PutByIdLong);
      CASE(TryPutByIdLong);
      CASE(GetById);
      CASE(GetByIdLong);
      CASE(GetByIdShort);
      CASE(TryGetById);
      CASE(TryGetByIdLong);
      CASE(DelById);
      CASE(DelByIdLong);
      CASE(Call1);
      CASE(Call2);
      CASE(Call3);
      CASE(Call4);
      CASE(Call);
      CASE(CallLong);
      CASE(Construct);
      CASE(ConstructLong);
      CASE(LoadConstZero);
      LOAD_CONST_STRING(LoadConstString);
      LOAD_CONST_STRING(LoadConstStringLongIndex);
      CASE(LoadParam);
      BINOP(Add);
      CASE(AddN);
      BINOP(Sub);
      CASE(SubN);
      BINOP(Mul);
      CASE(MulN);
      BINOP(Div);
      CASE(DivN);
      CASE(TypeOf);
      CASE(Mov);
      CASE(MovLong);
      CASE(ToNumber);
      CASE(ToInt32);
      CASE(AddEmptyString);
      CASE(Ret);

      JCOND(JLess, CCode::B, slowPathLess);
      JCOND(JLessEqual, CCode::BE, slowPathLessEq);
      JCOND(JGreater, CCode::A, slowPathGreater);
      JCOND(JGreaterEqual, CCode::AE, slowPathGreaterEq);
      JCOND(JNotLess, CCode::NB, slowPathGreaterEq);
      JCOND(JNotLessEqual, CCode::NBE, slowPathGreater);
      JCOND(JNotGreater, CCode::NA, slowPathLessEq);
      JCOND(JNotGreaterEqual, CCode::NAE, slowPathLess);

      // JEqual jumps when the equality test returns non-zero (true)
      JEQ(JEqual, CCode::NZ, compileEqJump);
      // JNotEqual jumps when the equality test returns zero (false)
      JEQ(JNotEqual, CCode::Z, compileEqJump);
      JEQ(JStrictEqual, CCode::NZ, compileStrictEqJump);
      JEQ(JStrictNotEqual, CCode::Z, compileStrictEqJump);

      // JmpTrue jumps when the operand register is non-zero (true)
      JBOOL(JmpTrue, CCode::NZ);
      // JmpFalse jumps when the equality test returns zero (false)
      JBOOL(JmpFalse, CCode::Z);
      CASE_WITH_SUFFIX(Jmp, , op1);
      CASE_WITH_SUFFIX(Jmp, Long, op1);
      CASE(JmpUndefined);
      CASE(JmpUndefinedLong);

      EQ_TEST(Eq, /*isNeq*/ false);
      EQ_TEST(Neq, /*isNeq*/ true);

      COND_OP(Less, CCode::B);
      COND_OP(LessEq, CCode::BE);
      COND_OP(Greater, CCode::A);
      COND_OP(GreaterEq, CCode::AE);

      LOAD_CONST_INT(
          LoadConstInt, HermesValue::encodeDoubleValue(ip->iLoadConstInt.op2));
      LOAD_CONST_INT(
          LoadConstUInt8,
          HermesValue::encodeDoubleValue(ip->iLoadConstUInt8.op2));
      LOAD_CONST_INT(
          LoadConstDouble,
          HermesValue::encodeDoubleValue(ip->iLoadConstDouble.op2));
      LOAD_CONST_INT(LoadConstUndefined, HermesValue::encodeUndefinedValue());
      LOAD_CONST_INT(LoadConstTrue, HermesValue::encodeBoolValue(true));
      LOAD_CONST_INT(LoadConstFalse, HermesValue::encodeBoolValue(false));
      LOAD_CONST_INT(LoadConstNull, HermesValue::encodeNullValue());

      CASE(NewObject);
      CASE_3REG(CreateThis);
      CASE(SelectObject);
      CASE(NewArray);
      CASE_WITH_SUFFIX(NewArrayWithBuffer, , op4);
      CASE_WITH_SUFFIX(NewArrayWithBuffer, Long, op4);
      CASE_WITH_SUFFIX(PutOwnByIndex, , op3);
      CASE_WITH_SUFFIX(PutOwnByIndex, L, op3);
      CASE_WITH_SUFFIX(PutNewOwnById, , op3);
      CASE_WITH_SUFFIX(PutNewOwnById, Short, op3);
      CASE_WITH_SUFFIX(PutNewOwnById, Long, op3);
      CASE(LoadThisNS);
      CASE(CoerceThisNS);
      CASE(Throw);
      CASE(NewObjectWithBuffer);
      CASE(NewObjectWithBufferLong);
      CASE_3REG(GetByVal);
      CASE(PutByVal);
      CASE(DelByVal);
      CASE(StoreToEnvironment);
      CASE(StoreToEnvironmentL);
      CASE(StoreNPToEnvironment);
      CASE(StoreNPToEnvironmentL);
      CASE_WITH_SUFFIX(LoadFromEnvironment, , op3);
      CASE_WITH_SUFFIX(LoadFromEnvironment, L, op3);
      CASE_3REG(Mod);
      CASE(Not);
      CASE_3REG(LShift);
      CASE_3REG(RShift);
      CASE_3REG(URshift);
      CASE_3REG(BitAnd);
      CASE_3REG(BitOr);
      CASE_3REG(BitXor);
      CASE(GetEnvironment);
      CASE(Catch);
      CASE(Negate);
      CASE(GetPNameList);
      CASE(GetNextPName);
      CASE(ReifyArguments);
      CASE(GetArgumentsPropByVal);
      CASE(BitNot);
      CASE(GetArgumentsLength);
      CASE_3REG(IsIn);
      CASE_3REG(InstanceOf);
      CASE(CreateRegExp);

      default:
        error(
            llvh::Twine("unsupported opcode ") + llvh::Twine((int)ip->opCode)
#ifndef NDEBUG
            + " " + getOpCodeString(ip->opCode)
#endif
        );
        return emit;
    }
#undef CASE

    LLVM_DEBUG(
        disassembleRange(
            sav.fast.current(), emit.fast.current(), llvh::dbgs(), true);
        if (sav.slow.current() != emit.slow.current() &&
            !slowPathSections_.back().data) {
          llvh::dbgs() << "; SLOW PATH\n";
          disassembleRange(
              sav.slow.current(), emit.slow.current(), llvh::dbgs(), true);
        });
  }

  return emit;
}

Emitter FastJIT::getConstant(Emitter slow, uint64_t cval, uint8_t *&constAddr) {
  // Find or emit the actual constant as a number.
  auto it = doubleConstants_.find(cval);
  if (it == doubleConstants_.end()) {
    // Add a new constant.
    slow.align<sizeof(uint64_t)>();
    constAddr = slow.current();
    slow.numericConst(cval);
    describeSlowPathSection(slow, true);

    doubleConstants_.try_emplace(cval, constAddr);
  } else {
    constAddr = it->second;
  }

  return slow;
}

template <bool fp>
Emitters
FastJIT::loadConstantIntoNativeReg(Emitters emit, HermesValue cval, Reg reg) {
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, cval, constAddr);

  // Load the constant into the register
  if (fp) {
    emit.fast.movfpRMToReg<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0, reg);
  } else {
    emit.fast.movRMToReg<S::Q, ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0, reg);
  }

  applyRIP32Offset(emit.fast.current(), constAddr);

  return emit;
}

Emitters
FastJIT::loadConstantAddrIntoNativeReg(Emitters emit, void *addr, Reg reg) {
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, addr, constAddr);

  // Load the constant address into the register
  emit.fast.movRMToReg<S::Q, ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0, reg);

  applyRIP32Offset(emit.fast.current(), constAddr);

  return emit;
}

Emitters FastJIT::loadDoubleConstant(
    Emitters emit,
    OperandReg32 hermesReg,
    double value) {
  emit = loadConstantIntoNativeReg(
      emit, HermesValue::encodeDoubleValue(value), Reg::rax);
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, hermesReg);
  return emit;
}
Emitters FastJIT::loadHermesValueConstant(
    Emitters emit,
    OperandReg32 hermesReg,
    HermesValue value) {
  emit = loadConstantIntoNativeReg(emit, value, Reg::rax);
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, hermesReg);
  return emit;
}

template <bool fp>
inline Emitter FastJIT::movNativeRegToHermesReg(
    Emitter emit,
    Reg nativeReg,
    OperandReg32 hermesReg) {
  if (fp) {
    emit.movfpRegToRM(
        nativeReg, RegFrame, Reg::NoIndex, localHermesRegByteOffset(hermesReg));

  } else {
    emit.movRegToRM<S::Q>(
        nativeReg, RegFrame, Reg::NoIndex, localHermesRegByteOffset(hermesReg));
  }
  return emit;
}

template <bool fp>
inline Emitter FastJIT::movHermesRegToNativeReg(
    Emitter emit,
    OperandReg32 hermesReg,
    Reg nativeReg) {
  if (fp) {
    emit.movfpRMToReg(
        RegFrame, Reg::NoIndex, localHermesRegByteOffset(hermesReg), nativeReg);
  } else {
    emit.movRMToReg<S::Q>(
        RegFrame, Reg::NoIndex, localHermesRegByteOffset(hermesReg), nativeReg);
  }
  return emit;
}

Emitter FastJIT::movHermesRegToHermesReg(
    Emitter emit,
    OperandReg32 src,
    OperandReg32 dst) {
  if (src == dst)
    return emit;
  emit = movHermesRegToNativeReg<false>(emit, src, Reg::rax);
  emit = movNativeRegToHermesReg<false>(emit, Reg::rax, dst);
  return emit;
}

inline Emitter
FastJIT::leaHermesReg(Emitter emit, OperandReg32 hermesReg, Reg nativeReg) {
  emit.leaRMToReg<S::Q>(
      RegFrame, Reg::NoIndex, localHermesRegByteOffset(hermesReg), nativeReg);
  return emit;
}
Emitters FastJIT::compileTypeOf(Emitters emit, const Inst *ip) {
  emit.fast = leaHermesReg(emit.fast, ip->iTypeOf.op2, Reg::rsi);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externTypeOf, constAddr);
  emit.fast =
      callExternalWithReturnedVal(emit.fast, constAddr, ip->iTypeOf.op1);
  return emit;
}

inline Emitter FastJIT::callAbsolute(Emitter emit, const void *dest) {
  emit.movqImmToReg((uint64_t)dest, Reg::rax);
  emit.callReg(Reg::rax);
  return emit;
}

Emitter FastJIT::callExternal(
    Emitter emit,
    const uint8_t *dest,
    OperandReg32 resultReg,
    const Inst *ip) {
  // Runtime -> arg1.
  emit.movRegToReg<S::Q>(RegRuntime, Reg::rdi);

  emit.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.current(), dest);

  // eax: status
  // rdx: HermesValue

  // Exception?
  emit.testRegToReg<S::L>(Reg::eax, Reg::eax);

  emit =
      cjmpToBytecodeBB(emit, CJumpOp<CCode::Z>::OP, getCatchHandlerBBIndex(ip));

  // Move the result value to the destination register.
  emit = movNativeRegToHermesReg(emit, Reg::rdx, resultReg);

  return emit;
}

Emitter FastJIT::callExternalNoReturnedVal(
    Emitter emit,
    const uint8_t *dest,
    const Inst *ip) {
  // Runtime -> arg1.
  emit.movRegToReg<S::Q>(RegRuntime, Reg::rdi);

  emit.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.current(), dest);

  // eax: status
  // Exception?
  emit.testRegToReg<S::L>(Reg::eax, Reg::eax);

  emit =
      cjmpToBytecodeBB(emit, CJumpOp<CCode::Z>::OP, getCatchHandlerBBIndex(ip));

  return emit;
}

Emitter FastJIT::callExternalWithReturnedVal(
    Emitter emit,
    const uint8_t *dest,
    OperandReg32 resultReg) {
  // Runtime -> arg1.
  emit.movRegToReg<S::Q>(RegRuntime, Reg::rdi);

  emit.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.current(), dest);

  // Move the result value to the destination register.
  emit = movNativeRegToHermesReg(emit, Reg::rax, resultReg);

  return emit;
}

Emitters FastJIT::callHelper(
    Emitters emit,
    const Inst *ip,
    uint32_t argCount,
    bool isConstruct) {
  //&callable -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iCall.op2, Reg::rsi);

  // argCount (uint32_t) -> arg3
  emit.fast.movImmToReg<S::L>(argCount, Reg::edx);

  // stack pointer -> arg4
  emit.fast.movRMToReg<S::Q>(
      RegRuntime, Reg::NoIndex, RuntimeOffsets::stackPointer, Reg::rcx);

  // ip -> arg5
  emit = loadConstantAddrIntoNativeReg(emit, (void *)ip, Reg::r8);

  // currentFrame -> arg6
  emit.fast.movRegToReg<S::Q>(RegFrame, Reg::r9);

  uint8_t *constAddr;
  emit.slow = getConstant(
      emit.slow,
      isConstruct ? (void *)externConstruct : (void *)externCall,
      constAddr);
  emit.fast = callExternal(emit.fast, constAddr, ip->iCall.op1, ip);
  return emit;
}

inline Emitter FastJIT::movHermesRegToCalleeArg(
    Emitter emit,
    OperandReg32 hermesReg,
    int32_t argIndex) {
  const auto RegTemp = Reg::rcx;
  emit = movHermesRegToNativeReg(emit, hermesReg, RegTemp);

  // Load runtime->stackPointer_ into StackPointerReg.
  // Since callee frame is not setup yet caller's stackPointer_ is
  // used to represent callee frame pointer.
  const auto StackPointerReg = Reg::rax;
  emit.movRMToReg<S::Q>(
      RegRuntime, Reg::NoIndex, RuntimeOffsets::stackPointer, StackPointerReg);

  emit.movRegToRM<S::Q>(
      RegTemp, StackPointerReg, Reg::NoIndex, hermesArgByteOffset(argIndex));
  return emit;
}

Emitters FastJIT::compileCall1(Emitters emit, const Inst *ip) {
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall1.op3, /*argIndex*/ -1);
  return callHelper(emit, ip, 1, false);
}
Emitters FastJIT::compileCall2(Emitters emit, const Inst *ip) {
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall2.op3, /*argIndex*/ -1);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall2.op4, /*argIndex*/ 0);
  return callHelper(emit, ip, 2, false);
}
Emitters FastJIT::compileCall3(Emitters emit, const Inst *ip) {
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall3.op3, /*argIndex*/ -1);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall3.op4, /*argIndex*/ 0);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall3.op5, /*argIndex*/ 1);
  return callHelper(emit, ip, 3, false);
}
Emitters FastJIT::compileCall4(Emitters emit, const Inst *ip) {
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall4.op3, /*argIndex*/ -1);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall4.op4, /*argIndex*/ 0);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall4.op5, /*argIndex*/ 1);
  emit.fast =
      movHermesRegToCalleeArg(emit.fast, ip->iCall4.op6, /*argIndex*/ 2);
  return callHelper(emit, ip, 4, false);
}
Emitters FastJIT::compileCall(Emitters emit, const Inst *ip) {
  return callHelper(emit, ip, ip->iCall.op3, false);
}
Emitters FastJIT::compileCallLong(Emitters emit, const Inst *ip) {
  return callHelper(emit, ip, ip->iCallLong.op3, false);
}
Emitters FastJIT::compileConstruct(Emitters emit, const Inst *ip) {
  return callHelper(emit, ip, ip->iConstruct.op3, true);
}
Emitters FastJIT::compileConstructLong(Emitters emit, const Inst *ip) {
  return callHelper(emit, ip, ip->iConstructLong.op3, true);
}

Emitter FastJIT::jmpToBytecodeBB(Emitter emit, unsigned bytecodeBB) {
  // If jumping to the next BB, do nothing.
  if (bytecodeBB == curBytecodeBBIndex_ + 1)
    return emit;

  // Backwards branch doesn't need a relocation and we can determine the offset.
  if (bytecodeBB <= curBytecodeBBIndex_) {
    emit.jmp<OffsetType::Auto>(nativeBBAddress_[bytecodeBB]);
  } else {
    // Forward branch: emit a long jump and record a relocation.
    emit.jmp<OffsetType::Int32>(emit.current());
    relocs_.emplace_back(ReloKind::Int32, emit.current() - 4, bytecodeBB);
  }
  return emit;
}

Emitter
FastJIT::cjmpToBytecodeBB(Emitter emit, uint8_t opCode, unsigned bytecodeBB) {
  // Backwards branch doesn't need a relocation and we can determine the offset.
  if (bytecodeBB <= curBytecodeBBIndex_) {
    emit.cjumpOP<OffsetType::Auto>(opCode, nativeBBAddress_[bytecodeBB]);
  } else {
    // Forward branch: emit a long jump and record a relocation.
    emit.cjumpOP<OffsetType::Int32>(opCode, emit.current());
    relocs_.emplace_back(ReloKind::Int32, emit.current() - 4, bytecodeBB);
  }
  return emit;
}

Emitters FastJIT::compileCondOp(
    Emitters emit,
    const Inst *ip,
    uint8_t opCode,
    void *slowPathCall) {
  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(emit.slow, slowPathCall, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  emit = callSlowPathBinOp(emit, ip, slowPathConstAddr);
  // Since slow path is emitted earlier, we need to relocate its last
  // jump-back-to-fast-path instruction.
  Relo relo{ReloKind::Int32, emit.slow.current() - 4, 0};

  // isNumber op2?
  emit.fast = isNumber(emit.fast, ip->iLess.op2, slowPathAddr);
  // isNumber op3?
  emit.fast = isNumber(emit.fast, ip->iLess.op3, slowPathAddr);
  // Fast path
  emit = compileCondOpN(emit, ip, opCode);

  applyRelocation(relo, emit.fast.current());

  return emit;
}

Emitters
FastJIT::compileCondOpN(Emitters emit, const Inst *ip, uint8_t opCode) {
  emit.fast =
      movHermesRegToNativeReg<true>(emit.fast, ip->iLess.op2, Reg::XMM0);
  emit.fast.ucomisRMToReg(
      RegFrame,
      Reg::NoIndex,
      localHermesRegByteOffset(ip->iLess.op3),
      Reg::XMM0);

  // encode a bool HermesValue tag first
  constexpr uint64_t tagq = (uint64_t)BoolTag << HermesValue::kNumDataBits;
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, tagq, constAddr);
  emit.fast.movRMToReg<S::Q, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 0, Reg::rax);
  applyRIP32Offset(emit.fast.current(), constAddr);
  // set the comparison result in the same register, which will not clear the
  // tag we already set.
  emit.fast.csetOP(opCode, Reg::al);

  // store the bool to result Hermes register
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iLess.op1);
  return emit;
}

inline Emitters FastJIT::getByIdHelper(
    Emitters emit,
    const Inst *ip,
    bool tryProp,
    uint32_t idVal) {
  auto defaultPropOpFlags = codeBlock_->isStrictMode()
      ? PropOpFlags().plusThrowOnError()
      : PropOpFlags();
  auto flags =
      !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist();
  // PropOpFlags  -> arg2
  emit.fast.movImmToReg<S::L>(flags.getRaw(), Reg::esi);

  // IdentifierID (uint32_t) -> arg3
  // The symbol must already exist in the string id map, so we could just pass
  // the IdentifierID
  emit.fast.movImmToReg<S::L>(
      codeBlock_->getRuntimeModule()
          ->getSymbolIDMustExist(idVal)
          .unsafeGetIndex(),
      Reg::edx);
  //&target -> arg4
  emit.fast = leaHermesReg(emit.fast, ip->iGetById.op2, Reg::rcx);
  // cacheIdx -> arg5
  // cacheIdx is uint8_t, but it's more efficient to just set whole 32 bits
  emit.fast.movImmToReg<S::L>(ip->iGetById.op3, Reg::r8d);
  // current code block -> arg6
  emit = loadConstantAddrIntoNativeReg(emit, codeBlock_, Reg::r9);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externGetById, constAddr);
  emit.fast = callExternal(emit.fast, constAddr, ip->iGetById.op1, ip);
  return emit;
}

Emitters FastJIT::compileGetById(Emitters emit, const Inst *ip) {
  return getByIdHelper(emit, ip, false, ip->iGetById.op4);
}
Emitters FastJIT::compileGetByIdShort(Emitters emit, const Inst *ip) {
  return getByIdHelper(emit, ip, false, ip->iGetByIdShort.op4);
}
Emitters FastJIT::compileGetByIdLong(Emitters emit, const Inst *ip) {
  return getByIdHelper(emit, ip, false, ip->iGetByIdLong.op4);
}
Emitters FastJIT::compileTryGetById(Emitters emit, const Inst *ip) {
  return getByIdHelper(emit, ip, true, ip->iTryGetById.op4);
}
Emitters FastJIT::compileTryGetByIdLong(Emitters emit, const Inst *ip) {
  return getByIdHelper(emit, ip, true, ip->iTryGetByIdLong.op4);
}

Emitters FastJIT::delByIdHelper(
    hermes::vm::x86_64::Emitters emit,
    const Inst *ip,
    uint32_t idVal) {
  // PropOpFlags -> arg4
  auto defaultPropOpFlags = codeBlock_->isStrictMode()
      ? PropOpFlags().plusThrowOnError()
      : PropOpFlags();
  emit.fast.movImmToReg<S::L>(defaultPropOpFlags.getRaw(), Reg::ecx);

  // target -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iDelById.op2, Reg::rsi);
  // IdentifierID (uint32_t) -> arg3
  // The symbol must already exist in the string id map, so we could just pass
  // the IdentifierID
  emit.fast.movImmToReg<S::L>(
      codeBlock_->getRuntimeModule()
          ->getSymbolIDMustExist(idVal)
          .unsafeGetIndex(),
      Reg::edx);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externDelById, constAddr);
  emit.fast = callExternal(emit.fast, constAddr, ip->iDelById.op1, ip);

  return emit;
}

Emitters FastJIT::compileDelById(
    hermes::vm::x86_64::Emitters emit,
    const Inst *ip) {
  return delByIdHelper(emit, ip, ip->iDelById.op3);
}

Emitters FastJIT::compileDelByIdLong(
    hermes::vm::x86_64::Emitters emit,
    const Inst *ip) {
  return delByIdHelper(emit, ip, ip->iDelByIdLong.op3);
}

inline Emitters FastJIT::putByIdHelper(
    Emitters emit,
    const Inst *ip,
    bool tryProp,
    uint32_t idVal) {
  auto defaultPropOpFlags = codeBlock_->isStrictMode()
      ? PropOpFlags().plusThrowOnError()
      : PropOpFlags();
  auto flags =
      !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist();
  // PropOpFlags  -> arg2
  emit.fast.movImmToReg<S::L>(flags.getRaw(), Reg::esi);
  // IdentifierID (uint32_t) -> arg3
  // The symbol must already exist in the map, so we could just pass the
  // IdentifierID
  emit.fast.movImmToReg<S::L>(
      codeBlock_->getRuntimeModule()
          ->getSymbolIDMustExist(idVal)
          .unsafeGetIndex(),
      Reg::edx);
  //&target -> arg4
  emit.fast = leaHermesReg(emit.fast, ip->iPutById.op1, Reg::rcx);
  //&prop -> arg5
  emit.fast = leaHermesReg(emit.fast, ip->iPutById.op2, Reg::r8);
  // cacheIdx -> arg6
  // cacheIdx is uint8_t, but it's more efficient to just set whole 32 bits
  emit.fast.movImmToReg<S::L>(ip->iPutById.op3, Reg::r9d);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externPutById, constAddr);
  emit.fast = callExternalNoReturnedVal(emit.fast, constAddr, ip);
  return emit;
}

Emitters FastJIT::compilePutById(Emitters emit, const Inst *ip) {
  return putByIdHelper(emit, ip, false, ip->iPutById.op4);
}
Emitters FastJIT::compilePutByIdLong(Emitters emit, const Inst *ip) {
  return putByIdHelper(emit, ip, false, ip->iPutByIdLong.op4);
}
Emitters FastJIT::compileTryPutById(Emitters emit, const Inst *ip) {
  return putByIdHelper(emit, ip, true, ip->iTryPutById.op4);
}
Emitters FastJIT::compileTryPutByIdLong(Emitters emit, const Inst *ip) {
  return putByIdHelper(emit, ip, true, ip->iTryPutByIdLong.op4);
}

Emitters FastJIT::compileDeclareGlobalVar(Emitters emit, const Inst *ip) {
  // StringID (uint32_t) -> arg2
  emit.fast.movImmToReg<S::L>(ip->iDeclareGlobalVar.op1, Reg::esi);

  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externCallDeclareGlobalVar, constAddr);
  emit.fast = callExternalNoReturnedVal(emit.fast, constAddr, ip);
  return emit;
}

Emitters FastJIT::compileCreateEnvironment(Emitters emit, const Inst *ip) {
  // current frame -> arg2
  emit.fast.movRegToReg<S::Q>(RegFrame, Reg::rsi);

  // uint32_t envSize -> arg3
  emit.fast.movImmToReg<S::L>(codeBlock_->getEnvironmentSize(), Reg::edx);

  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externCreateEnvironment, constAddr);
  emit.fast =
      callExternal(emit.fast, constAddr, ip->iCreateEnvironment.op1, ip);
  return emit;
}

Emitters FastJIT::compileCreateClosure(Emitters emit, const Inst *ip) {
  // Code blocks are allocated in C heap, so their addresses are constant,
  // and can be embedded in JIT'ed code.
  // &calleeCodeBlock  -> arg2
  CodeBlock *calleeBlock =
      codeBlock_->getRuntimeModule()->getCodeBlockMayAllocate(
          ip->iCreateClosure.op3);
  emit = loadConstantAddrIntoNativeReg(emit, calleeBlock, Reg::rsi);

  //&env -> arg3
  emit.fast = leaHermesReg(emit.fast, ip->iCreateClosure.op2, Reg::rdx);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externCreateClosure, constAddr);
  emit.fast = callExternal(emit.fast, constAddr, ip->iCreateClosure.op1, ip);
  return emit;
}

Emitters FastJIT::compileGetGlobalObject(Emitters emit, const Inst *ip) {
  emit.fast.movRMToReg<S::Q>(
      RegRuntime, Reg::NoIndex, RuntimeOffsets::globalObject, Reg::rax);
  emit.fast =
      movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iGetGlobalObject.op1);
  return emit;
}

Emitters FastJIT::compileLoadConstZero(Emitters emit, const Inst *ip) {
  emit.fast.xorRegToReg<S::Q>(Reg::rax, Reg::rax);
  emit.fast =
      movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iLoadConstZero.op1);
  return emit;
}

Emitters FastJIT::compileLoadConstString(
    Emitters emit,
    const Inst *ip,
    uint32_t stringID) {
  // stringID -> arg1
  emit.fast.movImmToReg<S::L>(stringID, Reg::edi);
  // runtime module -> arg2
  emit = loadConstantAddrIntoNativeReg(
      emit, codeBlock_->getRuntimeModule(), Reg::rsi);

  uint8_t *constAddr;
  emit.slow = getConstant(
      emit.slow, (void *)externLoadConstStringMayAllocate, constAddr);

  emit.fast.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.fast.current(), constAddr);

  // Move the result value to the destination register.
  emit.fast =
      movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iLoadConstString.op1);

  return emit;
}

Emitters FastJIT::compileStrictEqJump(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t reg1,
    uint32_t reg2,
    uint8_t opCode) {
  emit = callExternStrictEqTest(emit, ip, reg1, reg2);
  // al : bool
  emit.fast.testRegToReg<S::B>(Reg::al, Reg::al);

  emit.fast = cjmpToBytecodeBB(emit.fast, opCode, getBBIndex(ip, ipOffset));
  return emit;
}
Emitters
FastJIT::compileStrictEqTest(Emitters emit, const Inst *ip, bool isNeq) {
  emit = callExternStrictEqTest(emit, ip, ip->iStrictEq.op2, ip->iStrictEq.op3);

  // rax is the returned bool value (not HermesValue)
  if (isNeq)
    emit.fast.xorImmToReg<S::B>((uint8_t)0x01, Reg::al);

  emit = encodeBoolHVInNativeReg(emit, Reg::rax);

  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iStrictEq.op1);
  return emit;
}
Emitters FastJIT::callExternStrictEqTest(
    Emitters emit,
    const Inst *ip,
    uint32_t reg1,
    uint32_t reg2) {
  // arg1
  emit.fast = movHermesRegToNativeReg(emit.fast, reg1, Reg::rdi);
  // arg2
  emit.fast = movHermesRegToNativeReg(emit.fast, reg2, Reg::rsi);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)strictEqualityTest, constAddr);
  emit.fast.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.fast.current(), constAddr);
  return emit;
}

Emitters FastJIT::callExternEqTest(
    Emitters emit,
    const Inst *ip,
    uint32_t reg1,
    uint32_t reg2) {
  // arg2
  emit.fast = leaHermesReg(emit.fast, reg1, Reg::rsi);
  // arg3
  emit.fast = leaHermesReg(emit.fast, reg2, Reg::rdx);

  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(
      emit.slow, (void *)externAbstractEqualityTest, slowPathConstAddr);
  // Call external AbstractEqualityTest, which actually returns a bool
  // HermesValue, but we can't store it to the result reg directly, since we may
  // need to toggle it.
  emit.fast = callExternalNoReturnedVal(emit.fast, slowPathConstAddr, ip);
  return emit;
}

Emitters FastJIT::compileEqJump(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t reg1,
    uint32_t reg2,
    uint8_t opCode) {
  emit = callExternEqTest(emit, ip, reg1, reg2);

  // Whether the return value is true, namely whether its lower 32 bits is 0x01.
  // edx : bool (The tag is in the higher 32 bit of rdx, edx is 1 or 0.)
  emit.fast.testRegToReg<S::L>(Reg::edx, Reg::edx);

  emit.fast = cjmpToBytecodeBB(emit.fast, opCode, getBBIndex(ip, ipOffset));
  return emit;
}

Emitters FastJIT::compileEqTest(Emitters emit, const Inst *ip, bool isNeq) {
  emit = callExternEqTest(emit, ip, ip->iEq.op2, ip->iEq.op3);

  // rdx is the returned bool HermesValue
  if (isNeq)
    // Set the lower 1 byte does not change the value of higher 5 bytes
    emit.fast.xorImmToReg<S::B>((uint8_t)0x01, Reg::dl);
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rdx, ip->iEq.op1);
  return emit;
}

Emitters FastJIT::compileBoolJmp(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t regIdx,
    uint8_t opCode) {
  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(emit.slow, (void *)toBoolean, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  // Fast path: the operand is a boolean
  emit.fast = cmpSomeNPTag<BoolTag>(emit.fast, regIdx);
  emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);

  emit.fast.cmpImmToRM<S::B>(
      (uint8_t)0x0,
      RegFrame,
      Reg::NoIndex,
      // Compare the lowest 8 bits (the bool value) of the HermesValue
      localHermesRegByteOffset(regIdx));
  emit.fast = cjmpToBytecodeBB(emit.fast, opCode, getBBIndex(ip, ipOffset));

  // Slow path: emit an external call to toBoolean
  emit.slow = movHermesRegToNativeReg(emit.slow, regIdx, Reg::rdi);
  emit.slow.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.slow.current(), slowPathConstAddr);
  emit.slow.testRegToReg<S::B>(Reg::al, Reg::al);
  emit.slow = cjmpToBytecodeBB(emit.slow, opCode, getBBIndex(ip, ipOffset));
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
  describeSlowPathSection(emit.slow, false);

  return emit;
}

template <TagKind tag>
inline Emitter FastJIT::cmpSomeNPTag(Emitter emit, uint32_t regIndex) {
  static_assert(
      tag < FirstPointerTag,
      "String or object tag could not be compared directly with a HermesValue's higher 32 bits.");
  emit.cmpImmToRM<S::L>(
      tag << (32 - HermesValue::kTagWidth),
      RegFrame,
      Reg::NoIndex,
      // Compare the higher 32 bits (tag) of the HermesValue
      localHermesRegByteOffset(regIndex) + 4);
  return emit;
}
template <ETag etag>
inline Emitter FastJIT::cmpSomeNPETag(Emitter emit, uint32_t regIndex) {
  static_assert(
      etag < ETag::FirstPointer,
      "String or object tag could not be compared directly with a HermesValue's higher 32 bits.");
  emit.cmpImmToRM<S::L>(
      (uint32_t)etag << (32 - HermesValue::kETagWidth),
      RegFrame,
      Reg::NoIndex,
      // Compare the higher 32 bits (tag) of the HermesValue
      localHermesRegByteOffset(regIndex) + 4);
  return emit;
}
inline Emitters FastJIT::jmpUndefinedHelper(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t regIndex) {
  emit.fast = cmpSomeNPETag<ETag::Undefined>(emit.fast, regIndex);
  emit.fast = cjmpToBytecodeBB(
      emit.fast, CJumpOp<CCode::E>::OP, getBBIndex(ip, ipOffset));
  return emit;
}
Emitters FastJIT::compileJmpUndefined(Emitters emit, const Inst *ip) {
  return jmpUndefinedHelper(
      emit, ip, ip->iJmpUndefined.op1, ip->iJmpUndefined.op2);
}
Emitters FastJIT::compileJmpUndefinedLong(Emitters emit, const Inst *ip) {
  return jmpUndefinedHelper(
      emit, ip, ip->iJmpUndefinedLong.op1, ip->iJmpUndefinedLong.op2);
}

Emitters FastJIT::compileLoadParam(Emitters emit, const Inst *ip) {
  // rax = undefined
  emit = loadConstantIntoNativeReg(
      emit, HermesValue::encodeUndefinedValue(), Reg::rax);
  emit.fast.cmpImmToRM<S::L>(
      ip->iLoadParam.op2,
      RegFrame,
      Reg::NoIndex,
      sizeof(HermesValue) * StackFrameLayout::ArgCount);

  emit.fast.cjump<CCode::B, OffsetType::Int8>(emit.fast.current());
  Relo relo{ReloKind::Int8, emit.fast.current() - 1, 0};

  emit.fast.movRMToReg<S::Q>(
      RegFrame,
      Reg::NoIndex,
      sizeof(HermesValue) * StackFrameLayout::argOffset(ip->iLoadParam.op2 - 1),
      Reg::rax);

  applyRelocation(relo, emit.fast.current());

  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iLoadParam.op1);
  return emit;
}

Emitters FastJIT::compileBinOp(
    Emitters emit,
    const Inst *ip,
    void *slowPathBinOp,
    compileBinOpNPtr binOpNPtr) {
  uint8_t *externAddr;
  emit.slow = getConstant(emit.slow, slowPathBinOp, externAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  // isNumber op2?
  emit.fast = isNumber(emit.fast, ip->iSub.op2, slowPathAddr);
  // isNumber op3?
  emit.fast = isNumber(emit.fast, ip->iSub.op3, slowPathAddr);
  emit = (this->*binOpNPtr)(emit, ip);
  return callSlowPathBinOp(emit, ip, externAddr);
}

Emitters FastJIT::compileAddN(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg<true>(emit.fast, ip->iAdd.op2, Reg::XMM0);
  emit.fast.addfpRMToReg(
      RegFrame,
      Reg::NoIndex,
      localHermesRegByteOffset(ip->iAdd.op3),
      Reg::XMM0);
  emit.fast = movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iAdd.op1);
  return emit;
}

Emitters FastJIT::compileSubN(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg<true>(emit.fast, ip->iAdd.op2, Reg::XMM0);
  emit.fast.subfpRMFromReg(
      RegFrame,
      Reg::NoIndex,
      localHermesRegByteOffset(ip->iSubN.op3),
      Reg::XMM0);
  emit.fast = movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iAdd.op1);
  return emit;
}

Emitters FastJIT::compileMulN(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg<true>(emit.fast, ip->iAdd.op2, Reg::XMM0);
  emit.fast.mulfpRMToReg(
      RegFrame,
      Reg::NoIndex,
      localHermesRegByteOffset(ip->iMul.op3),
      Reg::XMM0);
  emit.fast = movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iAdd.op1);
  return emit;
}

Emitters FastJIT::compileDivN(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg<true>(emit.fast, ip->iDiv.op2, Reg::XMM0);
  emit.fast.divfpRMFromReg(
      RegFrame,
      Reg::NoIndex,
      localHermesRegByteOffset(ip->iDiv.op3),
      Reg::XMM0);
  emit.fast = movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iDiv.op1);
  return emit;
}

Emitter FastJIT::isNumber(Emitter emit, uint32_t regIndex, uint8_t *callStub) {
  emit.cmpImmToRM<S::L>(
      FirstTag << (32 - HermesValue::kTagWidth),
      RegFrame,
      Reg::NoIndex,
      // Compare the higher 32 bits (tag) of the HermesValue
      localHermesRegByteOffset(regIndex) + 4);
  emit.cjump<CCode::AE, OffsetType::Int32>(callStub);
  return emit;
}

Emitter FastJIT::isString(Emitter emit, uint32_t regIndex, uint8_t *callStub) {
  emit = cmpSomePointerTag(emit, regIndex, StrTag);
  emit.cjump<CCode::NE, OffsetType::Int32>(callStub);
  return emit;
}

Emitters FastJIT::callSlowPathBinOp(
    Emitters emit,
    const Inst *ip,
    const uint8_t *externBinOp) {
  // Emit the slow path.
  // &op2 -> arg2, &op3 -> arg3
  emit.slow = leaHermesReg(emit.slow, ip->iAdd.op2, Reg::rsi);
  emit.slow = leaHermesReg(emit.slow, ip->iAdd.op3, Reg::rdx);

  // Call slowPath binary operation
  emit.slow = callExternal(emit.slow, externBinOp, ip->iAdd.op1, ip);

  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());

  describeSlowPathSection(emit.slow, false);
  return emit;
}

Emitters FastJIT::compileMov(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg(emit.fast, ip->iMov.op2, Reg::rax);
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iMov.op1);
  return emit;
}
Emitters FastJIT::compileMovLong(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg(emit.fast, ip->iMovLong.op2, Reg::rax);
  emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iMovLong.op1);
  return emit;
}

Emitters FastJIT::compileToNumber(Emitters emit, const Inst *ip) {
  // Put the external call stub at the beginning of the slow path
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)slowPathToNumber, constAddr);

  // isNumber op2?
  emit.fast = isNumber(emit.fast, ip->iToNumber.op2, emit.slow.current());

  emit.fast =
      movHermesRegToHermesReg(emit.fast, ip->iToNumber.op2, ip->iToNumber.op1);

  // Emit the slow path.
  // &Source -> arg2.
  emit.slow = leaHermesReg(emit.slow, ip->iToNumber.op2, Reg::rsi);

  // Call toNumber
  emit.slow = callExternal(emit.slow, constAddr, ip->iToNumber.op1, ip);

  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());

  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileToInt32(Emitters emit, const Inst *ip) {
  // arg2
  emit.fast = leaHermesReg(emit.fast, ip->iToInt32.op2, Reg::rsi);

  uint8_t *externConstAddr;
  emit.slow = getConstant(emit.slow, (void *)externToInt32, externConstAddr);
  emit.fast = callExternal(emit.fast, externConstAddr, ip->iToInt32.op1, ip);
  return emit;
}

Emitters FastJIT::compileAddEmptyString(Emitters emit, const Inst *ip) {
  // Put the external call stub at the beginning of the slow path
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)slowPathAddEmptyString, constAddr);

  // isString op2?
  emit.fast = isString(emit.fast, ip->iAddEmptyString.op2, emit.slow.current());

  emit.fast = movHermesRegToHermesReg(
      emit.fast, ip->iAddEmptyString.op2, ip->iAddEmptyString.op1);

  // Emit the slow path.
  // &Source -> arg2.
  emit.slow = leaHermesReg(emit.slow, ip->iAddEmptyString.op2, Reg::rsi);

  // Call slowPathAddEmptyString
  emit.slow = callExternal(emit.slow, constAddr, ip->iAddEmptyString.op1, ip);

  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());

  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileRet(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToNativeReg(emit.fast, ip->iRet.op1, Reg::rdx);
  emit.fast.movImmToReg<S::L>(1, Reg::eax);

  emit.fast = jmpToBytecodeBB(emit.fast, bcBasicBlocks_.size() - 1);

  return emit;
}

Emitters FastJIT::compileJmp(Emitters emit, const Inst *ip, uint32_t ipOffset) {
  emit.fast = jmpToBytecodeBB(emit.fast, getBBIndex(ip, ipOffset));
  return emit;
}

Emitters FastJIT::compileCondJumpN(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t reg1,
    uint32_t reg2,
    uint8_t opCode) {
  emit.fast = movHermesRegToNativeReg<true>(emit.fast, reg1, Reg::XMM0);
  emit.fast.ucomisRMToReg(
      RegFrame, Reg::NoIndex, localHermesRegByteOffset(reg2), Reg::XMM0);

  emit.fast = cjmpToBytecodeBB(emit.fast, opCode, getBBIndex(ip, ipOffset));

  return emit;
}

Emitters FastJIT::compileCondJump(
    Emitters emit,
    const Inst *ip,
    uint32_t ipOffset,
    uint32_t reg1,
    uint32_t reg2,
    uint8_t opCode,
    void *slowPathCall) {
  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(emit.slow, slowPathCall, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  emit.fast = isNumber(emit.fast, reg1, slowPathAddr);
  emit.fast = isNumber(emit.fast, reg2, slowPathAddr);

  // Fast path
  emit = compileCondJumpN(emit, ip, ipOffset, reg1, reg2, opCode);

  // Slow path
  emit.slow = leaHermesReg(emit.slow, reg1, Reg::rsi);
  emit.slow = leaHermesReg(emit.slow, reg2, Reg::rdx);

  // Call slowPath comparison operation, which actually returns a bool
  // HermesValue, but we don't need to store it to a Hermes reg, instead, we
  // need to examine it explicitly.
  emit.slow = callExternalNoReturnedVal(emit.slow, slowPathConstAddr, ip);

  // Whether the return value is true, namely whether its lower 32 bits is 0x01.
  // edx : bool (The tag is in the higher 32 bit of rdx, edx is 1 or 0.)
  emit.slow.testRegToReg<S::L>(Reg::edx, Reg::edx);
  // Another option to examine bool: emit.slow.andImm8ToReg((uint8_t)0x01,
  // Reg::edx);

  // Jump to the target BB if true

  emit.slow = cjmpToBytecodeBB(
      emit.slow, CJumpOp<CCode::NZ>::OP, getBBIndex(ip, ipOffset));
  // Jump to next ip if false
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());

  describeSlowPathSection(emit.slow, false);
  return emit;
}

Emitters FastJIT::compileNewObject(Emitters emit, const Inst *ip) {
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externNewObject, constAddr);
  emit.fast =
      callExternalWithReturnedVal(emit.fast, constAddr, ip->iNewObject.op1);
  return emit;
}

Emitters
FastJIT::compile3RegsInst(Emitters emit, const Inst *ip, void *externCallAddr) {
  emit.fast = leaHermesReg(emit.fast, ip->iCreateThis.op2, Reg::rsi);
  emit.fast = leaHermesReg(emit.fast, ip->iCreateThis.op3, Reg::rdx);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, externCallAddr, constAddr);

  emit.fast = callExternal(emit.fast, constAddr, ip->iCreateThis.op1, ip);
  return emit;
}

Emitter
FastJIT::cmpSomePointerTag(Emitter emit, uint32_t regIndex, TagKind tag) {
  emit = movHermesRegToNativeReg(emit, regIndex, Reg::rax);
  emit.shrImm8ToReg(HermesValue::kNumDataBits, Reg::rax);
  emit.cmpImmToRM<S::L, ScaleRegAccess>(tag, Reg::eax, Reg::none, 0);
  return emit;
}

Emitters FastJIT::compileSelectObject(Emitters emit, const Inst *ip) {
  emit.fast = cmpSomePointerTag(emit.fast, ip->iSelectObject.op3, ObjectTag);

  if (ip->iSelectObject.op1 == ip->iSelectObject.op3) {
    // If op3 is object, jump to the end, since we don't need to mov between two
    // same regs op1 and op3
    emit.fast.cjump<CCode::E, OffsetType::Int8>(emit.fast.current());
    Relo reloToEnd{ReloKind::Int8, emit.fast.current() - 1, 0};
    // else if op3 is not object
    emit.fast = movHermesRegToHermesReg(
        emit.fast, ip->iSelectObject.op2, ip->iSelectObject.op1);

    applyRelocation(reloToEnd, emit.fast.current());

  } else if (ip->iSelectObject.op1 == ip->iSelectObject.op2) {
    // If op3 is not object, do nothing in fast path, since we don't need to mov
    // between two same regs op1 and op2, otherwise jmp to slow path
    emit.fast.cjump<CCode::E, OffsetType::Int32>(emit.slow.current());

    // slow path, if op3 is object
    emit.slow = movHermesRegToHermesReg(
        emit.slow, ip->iSelectObject.op3, ip->iSelectObject.op1);
    emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
    describeSlowPathSection(emit.slow, false);
  } else {
    // If op3 is object, jump to slow path
    emit.fast.cjump<CCode::E, OffsetType::Int32>(emit.slow.current());

    emit.fast =
        movHermesRegToNativeReg(emit.fast, ip->iSelectObject.op2, Reg::rax);

    // slow path.
    emit.slow =
        movHermesRegToNativeReg(emit.slow, ip->iSelectObject.op3, Reg::rax);
    emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
    describeSlowPathSection(emit.slow, false);

    // store the result
    emit.fast =
        movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iSelectObject.op1);
  }

  return emit;
}

Emitters FastJIT::compileNewArray(Emitters emit, const Inst *ip) {
  emit.fast.movImmToReg<S::L>(ip->iNewArray.op2, Reg::esi);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externNewArray, constAddr);
  emit.fast = callExternal(emit.fast, constAddr, ip->iNewArray.op1, ip);
  return emit;
}

Emitters FastJIT::compileNewArrayWithBuffer(
    Emitters emit,
    const Inst *ip,
    uint32_t idx) {
  // current  code block -> arg2
  emit = loadConstantAddrIntoNativeReg(emit, codeBlock_, Reg::rsi);
  //  a preallocation size hint (uint16_t) -> arg3
  emit.fast.movImmToReg<S::L>(ip->iNewArrayWithBuffer.op2, Reg::edx);
  // the number of static elements(uint16_t) -> arg4
  emit.fast.movImmToReg<S::L>(ip->iNewArrayWithBuffer.op3, Reg::ecx);
  // the index in the array buffer table (uint16_t/uint32_t) -> arg5
  emit.fast.movImmToReg<S::L>(idx, Reg::r8d);

  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externNewArrayWithBuffer, constAddr);
  emit.fast =
      callExternal(emit.fast, constAddr, ip->iNewArrayWithBuffer.op1, ip);
  return emit;
}

Emitters
FastJIT::compilePutOwnByIndex(Emitters emit, const Inst *ip, uint32_t idx) {
  // Object to put in -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iPutOwnByIndex.op1, Reg::rsi);
  // Property to be put -> arg3
  emit.fast = leaHermesReg(emit.fast, ip->iPutOwnByIndex.op2, Reg::rdx);
  // Property index -> arg4
  emit.fast.movImmToReg<S::L>(idx, Reg::ecx);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externPutOwnByIndex, constAddr);
  emit.fast = callExternalNoReturnedVal(emit.fast, constAddr, ip);

  return emit;
}

Emitters
FastJIT::compilePutNewOwnById(Emitters emit, const Inst *ip, uint32_t idx) {
  // Object to put property in -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iPutOwnByIndex.op1, Reg::rsi);
  // Property to be put -> arg3
  emit.fast = leaHermesReg(emit.fast, ip->iPutOwnByIndex.op2, Reg::rdx);
  // The symbol must already exist in the map, so we could just pass the
  // IdentifierID
  emit.fast.movImmToReg<S::L>(
      codeBlock_->getRuntimeModule()
          ->getSymbolIDMustExist(idx)
          .unsafeGetIndex(),
      Reg::ecx);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externPutNewOwnById, constAddr);
  emit.fast = callExternalNoReturnedVal(emit.fast, constAddr, ip);

  return emit;
}

Emitters FastJIT::compileLoadThisNS(Emitters emit, const Inst *ip) {
  // StackFrameLayout::ThisArg is not technically a local register, but we could
  // still use the same way to read it.
  return coerceThisHelper(
      emit, ip, StackFrameLayout::FirstLocal - StackFrameLayout::ThisArg);
}
Emitters FastJIT::compileCoerceThisNS(Emitters emit, const Inst *ip) {
  return coerceThisHelper(emit, ip, ip->iCoerceThisNS.op2);
}

inline Emitter FastJIT::cmpNullOrUndefinedTag(Emitter emit, uint32_t regIdx) {
  return cmpSomeNPTag<UndefinedNullTag>(emit, regIdx);
}

Emitters
FastJIT::coerceThisHelper(Emitters emit, const Inst *ip, uint32_t regIndex) {
  uint8_t *slowPathConstAddr;
  emit.slow =
      getConstant(emit.slow, (void *)slowPathCoerceThis, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  if (regIndex != ip->iLoadThisNS.op1) {
    emit.fast = movHermesRegToNativeReg(emit.fast, regIndex, Reg::rax);
    emit.fast.movRegToReg<S::Q>(Reg::rax, Reg::rdx);
    emit.fast.shrImm8ToReg(HermesValue::kNumDataBits, Reg::rax);
    emit.fast.cmpImmToRM<S::L, ScaleRegAccess>(
        ObjectTag, Reg::eax, Reg::none, 0);

    emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);

    uint8_t *fastPathJmpBackAddr = emit.fast.current();
    emit.fast =
        movNativeRegToHermesReg(emit.fast, Reg::rdx, ip->iLoadThisNS.op1);

    // Slow path:
    // If regIndex is null or undefined
    emit.slow = cmpNullOrUndefinedTag(emit.slow, regIndex);

    emit.slow.cjump<CCode::NE, OffsetType::Int8>(emit.slow.current());
    Relo reloToElse{ReloKind::Int8, emit.slow.current() - 1, 0};

    emit.slow.movRMToReg<S::Q>(
        RegRuntime, Reg::NoIndex, RuntimeOffsets::globalObject, Reg::rdx);
    emit.slow.jmp<OffsetType::Int32>(fastPathJmpBackAddr);

    applyRelocation(reloToElse, emit.slow.current());
    // Else: emit the actual slow path call
    emit.slow = leaHermesReg(emit.slow, regIndex, Reg::rsi);
    emit.slow = callExternalNoReturnedVal(emit.slow, slowPathConstAddr, ip);
    emit.slow.jmp<OffsetType::Int32>(fastPathJmpBackAddr);
    describeSlowPathSection(emit.slow, false);

  } else {
    emit.fast = cmpSomePointerTag(emit.fast, regIndex, ObjectTag);
    emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);

    // Slow path:
    // If regIndex is null or undefined
    emit.slow = cmpNullOrUndefinedTag(emit.slow, regIndex);

    emit.slow.cjump<CCode::NE, OffsetType::Int8>(emit.slow.current());
    Relo reloToElse{ReloKind::Int8, emit.slow.current() - 1, 0};

    emit.slow = movRuntimeVarToHermesReg(
        emit.slow, RuntimeOffsets::globalObject, ip->iLoadThisNS.op1);
    emit.slow.jmp<OffsetType::Int32>(emit.fast.current());

    applyRelocation(reloToElse, emit.slow.current());
    // Else: emit the actual slow path call
    emit.slow = leaHermesReg(emit.slow, regIndex, Reg::rsi);
    emit.slow =
        callExternal(emit.slow, slowPathConstAddr, ip->iLoadThisNS.op1, ip);
    emit.slow.jmp<OffsetType::Int32>(emit.fast.current());
    describeSlowPathSection(emit.slow, false);
  }

  return emit;
}

inline Emitter FastJIT::movRuntimeVarToHermesReg(
    Emitter emit,
    uint32_t runtimeVar,
    OperandReg32 dst) {
  emit.movRMToReg<S::Q>(RegRuntime, Reg::NoIndex, runtimeVar, Reg::rax);
  emit = movNativeRegToHermesReg(emit, Reg::rax, dst);
  return emit;
}

inline Emitter FastJIT::movHermesRegToRuntimeVar(
    Emitter emit,
    uint32_t runtimeVar,
    OperandReg32 src) {
  emit = movHermesRegToNativeReg(emit, src, Reg::rax);
  emit.movRegToRM<S::Q>(Reg::rax, RegRuntime, Reg::NoIndex, runtimeVar);
  return emit;
}

unsigned FastJIT::getCatchHandlerBBIndex(const Inst *ip) {
  // The offset between catch handler ip and codeBlock_->begin()
  int32_t handlerOffset = codeBlock_->findCatchTargetOffset(
      (const uint8_t *)ip - (const uint8_t *)codeBlock_->begin());
  assert(
      bcBasicBlocks_.size() > 0 &&
      "We should at least have one BB which is epilogue.");
  if (handlerOffset == -1)
    return bcBasicBlocks_.size() - 1; // exit block
  else {
    // The last BB is the epilogue which could not be a catch handler BB.
    assert(
        handlerOffset >= 0 && (uint32_t)handlerOffset < bcBasicBlocks_.back() &&
        "The catch handler basic block offset is out of bound.");
    assert(
        bcLabels_.find(handlerOffset) != bcLabels_.end() &&
        "handlerOffset not in bcLabels_");
    return bcLabels_[handlerOffset];
  }
}

Emitters FastJIT::compileCatch(Emitters emit, const Inst *ip) {
  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, HermesValue::encodeEmptyValue(), constAddr);

  emit.fast = movRuntimeVarToHermesReg(
      emit.fast, RuntimeOffsets::thrownValue, ip->iCatch.op1);
  // Clear Runtime::thrownValue_
  emit.fast.movRMToReg<S::Q, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 0, Reg::rax);
  applyRIP32Offset(emit.fast.current(), constAddr);
  emit.fast.movRegToRM<S::Q>(
      Reg::rax, RegRuntime, Reg::NoIndex, RuntimeOffsets::thrownValue);

  return emit;
}

Emitters FastJIT::compileThrow(Emitters emit, const Inst *ip) {
  emit.fast = movHermesRegToRuntimeVar(
      emit.fast, RuntimeOffsets::thrownValue, ip->iThrow.op1);

  // return exception
  emit.fast.xorRegToReg<S::L>(Reg::eax, Reg::eax);
  emit.fast = jmpToBytecodeBB(emit.fast, getCatchHandlerBBIndex(ip));
  return emit;
}

Emitters FastJIT::compileNewObjectWithBuffer(Emitters emit, const Inst *ip) {
  return newObjectWithBufferHelper(
      emit, ip, ip->iNewObjectWithBuffer.op4, ip->iNewObjectWithBuffer.op5);
}
Emitters FastJIT::compileNewObjectWithBufferLong(
    Emitters emit,
    const Inst *ip) {
  return newObjectWithBufferHelper(
      emit,
      ip,
      ip->iNewObjectWithBufferLong.op4,
      ip->iNewObjectWithBufferLong.op5);
}
Emitters FastJIT::newObjectWithBufferHelper(
    Emitters emit,
    const Inst *ip,
    uint32_t keyIdx,
    uint32_t valIdx) {
  // current  code block -> arg2
  emit = loadConstantAddrIntoNativeReg(emit, codeBlock_, Reg::rsi);
  // the number of static elements. (uint16_t) -> arg3
  emit.fast.movImmToReg<S::L>(ip->iNewObjectWithBuffer.op3, Reg::edx);
  // the index in the object key buffer table (uint16_t/uint32_t) -> arg4
  emit.fast.movImmToReg<S::L>(keyIdx, Reg::ecx);
  // the index in the object val buffer table (uint16_t/uint32_t) -> arg5
  emit.fast.movImmToReg<S::L>(valIdx, Reg::r8d);

  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externNewObjectWithBuffer, constAddr);
  emit.fast =
      callExternal(emit.fast, constAddr, ip->iNewObjectWithBuffer.op1, ip);
  return emit;
}

Emitters FastJIT::compilePutByVal(Emitters emit, const Inst *ip) {
  // object -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iPutByVal.op1, Reg::rsi);
  // nameVal -> arg3
  emit.fast = leaHermesReg(emit.fast, ip->iPutByVal.op2, Reg::rdx);
  // property value -> arg4
  emit.fast = leaHermesReg(emit.fast, ip->iPutByVal.op3, Reg::rcx);
  // PropOpFlags -> arg5
  auto defaultPropOpFlags = codeBlock_->isStrictMode()
      ? PropOpFlags().plusThrowOnError()
      : PropOpFlags();
  emit.fast.movImmToReg<S::L>(defaultPropOpFlags.getRaw(), Reg::r8d);

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externPutByVal, constAddr);
  emit.fast = callExternalNoReturnedVal(emit.fast, constAddr, ip);
  return emit;
}

Emitters FastJIT::compileDelByVal(Emitters emit, const Inst *ip) {
  // PropOpFlags -> arg4
  auto defaultPropOpFlags = codeBlock_->isStrictMode()
      ? PropOpFlags().plusThrowOnError()
      : PropOpFlags();
  emit.fast.movImmToReg<S::L>(defaultPropOpFlags.getRaw(), Reg::ecx);
  return compile3RegsInst(emit, ip, (void *)externDelByVal);
}

Emitters FastJIT::storeToEnvironmentHelper(
    Emitters emit,
    uint32_t op1,
    uint32_t idx,
    uint32_t op3,
    bool isNP) {
  // environment -> arg1
  emit.fast = leaHermesReg(emit.fast, op1, Reg::rdi);
  // slot index -> arg2
  emit.fast.movImmToReg<S::L>(idx, Reg::rsi);
  // value -> arg3
  emit.fast = leaHermesReg(emit.fast, op3, Reg::rdx);

  if (!isNP)
    // runtime -> arg4
    emit.fast.movRegToReg<S::Q>(RegRuntime, Reg::rcx);

  uint8_t *constAddr;
  emit.slow = getConstant(
      emit.slow,
      isNP ? (void *)externStoreNPToEnvironment
           : (void *)externStoreToEnvironment,
      constAddr);
  emit.fast.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.fast.current(), constAddr);
  // the external call returns void

  return emit;
}
Emitters FastJIT::compileStoreToEnvironment(Emitters emit, const Inst *ip) {
  return storeToEnvironmentHelper(
      emit,
      ip->iStoreToEnvironment.op1,
      ip->iStoreToEnvironment.op2,
      ip->iStoreToEnvironment.op3,
      false);
}
Emitters FastJIT::compileStoreToEnvironmentL(Emitters emit, const Inst *ip) {
  return storeToEnvironmentHelper(
      emit,
      ip->iStoreToEnvironmentL.op1,
      ip->iStoreToEnvironmentL.op2,
      ip->iStoreToEnvironmentL.op3,
      false);
}
Emitters FastJIT::compileStoreNPToEnvironment(Emitters emit, const Inst *ip) {
  return storeToEnvironmentHelper(
      emit,
      ip->iStoreNPToEnvironment.op1,
      ip->iStoreNPToEnvironment.op2,
      ip->iStoreNPToEnvironment.op3,
      true);
}
Emitters FastJIT::compileStoreNPToEnvironmentL(Emitters emit, const Inst *ip) {
  return storeToEnvironmentHelper(
      emit,
      ip->iStoreNPToEnvironmentL.op1,
      ip->iStoreNPToEnvironmentL.op2,
      ip->iStoreNPToEnvironmentL.op3,
      true);
}

Emitters FastJIT::compileLoadFromEnvironment(
    Emitters emit,
    const Inst *ip,
    uint32_t idx) {
  emit.fast = leaHermesReg(emit.fast, ip->iLoadFromEnvironment.op2, Reg::rdi);
  emit.fast.movImmToReg<S::L>(idx, Reg::rsi);
  uint8_t *constAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externLoadFromEnvironment, constAddr);
  emit.fast.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.fast.current(), constAddr);

  emit.fast = movNativeRegToHermesReg(
      emit.fast, Reg::rax, ip->iLoadFromEnvironment.op1);
  return emit;
}

Emitters FastJIT::compileNot(Emitters emit, const Inst *ip) {
  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(emit.slow, (void *)toBoolean, slowPathConstAddr);
  constexpr uint64_t tagq = (uint64_t)BoolTag << HermesValue::kNumDataBits;
  uint8_t *boolTagconstAddr;
  emit.slow = getConstant(emit.slow, tagq, boolTagconstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  // Fast Path: check if op2 is a bool
  emit.fast = cmpSomeNPTag<BoolTag>(emit.fast, ip->iNot.op2);
  emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);
  uint8_t *jmpBacktoFp;

  bool sameReg = ip->iNot.op2 == ip->iNot.op1;
  if (sameReg) {
    // Toggle the bool value directly in memory.
    emit.fast.xorImmToRM<S::B>(
        (uint8_t)0x01,
        RegFrame,
        Reg::NoIndex,
        localHermesRegByteOffset(ip->iNot.op1));
    jmpBacktoFp = emit.fast.current();

  } else {
    emit.fast = movHermesRegToNativeReg(emit.fast, ip->iNot.op2, Reg::rax);
    jmpBacktoFp = emit.fast.current();
    // Toggle the bool result in rax.
    emit.fast.xorImmToReg<S::B>((uint8_t)0x01, Reg::al);
    emit.fast = movNativeRegToHermesReg(emit.fast, Reg::rax, ip->iNot.op1);
  }

  // Slow path: emit the external call to toBoolean
  emit.slow = movHermesRegToNativeReg(emit.slow, ip->iNot.op2, Reg::rdi);
  emit.slow.callRM<ScaleRIPAddr32>(Reg::none, Reg::NoIndex, 0);
  applyRIP32Offset(emit.slow.current(), slowPathConstAddr);
  // Encode the bool tag in %rax, while remaining the bool value
  emit.slow.orRmToReg<S::Q, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 0, Reg::rax);
  applyRIP32Offset(emit.slow.current(), boolTagconstAddr);

  if (sameReg) {
    emit.slow.xorImmToReg<S::B>((uint8_t)0x01, Reg::al);
    emit.slow = movNativeRegToHermesReg(emit.slow, Reg::rax, ip->iNot.op1);
  }
  emit.slow.jmp<OffsetType::Int32>(jmpBacktoFp);

  describeSlowPathSection(emit.slow, false);

  return emit;
}

inline Emitters FastJIT::encodeBoolHVInNativeReg(Emitters emit, Reg nativeReg) {
  constexpr uint64_t tagq = (uint64_t)BoolTag << HermesValue::kNumDataBits;
  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, tagq, constAddr);
  // The encoding of bool HermesValue is not likely to happen in slow path since
  // they are external calls, which will return encoded bool HermesValue
  // directly if needed.
  emit.fast.orRmToReg<S::Q, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 0, nativeReg);
  applyRIP32Offset(emit.fast.current(), constAddr);
  return emit;
}

Emitters FastJIT::compileGetEnvironment(Emitters emit, const Inst *ip) {
  // TODO: emit sequential inline code when levels are small, e.g. 1-3;
  // TODO: otherwise emit a compact loop instead of external call

  uint8_t *constAddr;
  emit.slow = getConstant(emit.slow, (void *)externGetEnvironment, constAddr);

  // frame -> arg2
  emit.fast.movRegToReg<S::Q>(RegFrame, Reg::rsi);
  // numLevel -> arg3
  emit.fast.movImmToReg<S::L>(ip->iGetEnvironment.op2, Reg::rdx);

  emit.fast = callExternalWithReturnedVal(
      emit.fast, constAddr, ip->iGetEnvironment.op1);

  return emit;
}

Emitters FastJIT::compileNegate(Emitters emit, const Inst *ip) {
  uint8_t *externAddr;
  emit.slow = getConstant(emit.slow, (void *)slowPathNegate, externAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  // isNumber op2?
  emit.fast = isNumber(emit.fast, ip->iNegate.op2, slowPathAddr);

  if (ip->iNegate.op2 == ip->iNegate.op1) {
    // Negate the highest sign bit directly
    emit.fast.xorImmToRM<S::B>(
        (uint8_t)0x80,
        RegFrame,
        Reg::NoIndex,
        localHermesRegByteOffset(ip->iNegate.op2) + 7);
  } else {
    // We could also load op1 to XMM0 and xor an imm64 (0x80....0) to it, but it
    // takes 8 extra bytes to hold the imm64
    emit.fast.xorfpRegToReg(Reg::XMM0, Reg::XMM0);
    emit.fast.subfpRMFromReg(
        RegFrame,
        Reg::NoIndex,
        localHermesRegByteOffset(ip->iNegate.op2),
        Reg::XMM0);
    emit.fast =
        movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iNegate.op1);
  }

  // slow path
  emit.slow = leaHermesReg(emit.slow, ip->iNegate.op2, Reg::rsi);
  emit.slow = callExternal(emit.slow, externAddr, ip->iNegate.op1, ip);
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileGetPNameList(Emitters emit, const Inst *ip) {
  uint8_t *externAddr;
  emit.slow = getConstant(
      emit.slow, (void *)Interpreter::handleGetPNameList, externAddr);
  // The frameRegs used in Interpreter::handleGetPNameList is actually the first
  // local variable, not the stack pointer; so we just pass the address of r0.
  emit.fast = leaHermesReg(emit.fast, 0, Reg::rsi);
  emit = loadConstantAddrIntoNativeReg(emit, (void *)ip, Reg::rdx);
  emit.fast = callExternalNoReturnedVal(emit.fast, externAddr, ip);
  return emit;
}

Emitters FastJIT::compileGetNextPName(Emitters emit, const Inst *ip) {
  uint8_t *externAddr;
  emit.slow = getConstant(emit.slow, (void *)externGetNextPName, externAddr);

  // array of props -> arg2
  emit.fast = leaHermesReg(emit.fast, ip->iGetNextPName.op2, Reg::rsi);
  // object -> arg3
  emit.fast = leaHermesReg(emit.fast, ip->iGetNextPName.op3, Reg::rdx);
  // iterating index reg -> arg4
  emit.fast = leaHermesReg(emit.fast, ip->iGetNextPName.op4, Reg::rcx);
  // size of the property list -> arg5
  emit.fast = leaHermesReg(emit.fast, ip->iGetNextPName.op5, Reg::r8);

  emit.fast =
      callExternalWithReturnedVal(emit.fast, externAddr, ip->iGetNextPName.op1);

  return emit;
}

Emitters FastJIT::compileReifyArguments(Emitters emit, const Inst *ip) {
  uint8_t *externConstAddr;
  emit.slow = getConstant(
      emit.slow, (void *)externSlowPathReifyArguments, externConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  emit.fast =
      cmpSomeNPETag<ETag::Undefined>(emit.fast, ip->iReifyArguments.op1);
  emit.fast.cjump<CCode::E, OffsetType::Int32>(slowPathAddr);
  // Fast path: if the arguments object was already created, do nothing.

  // Slow path
  emit.slow.movRegToReg<S::Q>(RegFrame, Reg::rsi);
  emit.slow.movImmToReg<S::L>(codeBlock_->isStrictMode(), Reg::edx);
  emit.slow =
      callExternal(emit.slow, externConstAddr, ip->iReifyArguments.op1, ip);
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileGetArgumentsPropByVal(Emitters emit, const Inst *ip) {
  // TODO: Add a fast path similar to the interpreter one.
  emit.fast = leaHermesReg(emit.fast, ip->iGetArgumentsPropByVal.op3, Reg::rsi);
  emit.fast = leaHermesReg(emit.fast, ip->iGetArgumentsPropByVal.op2, Reg::rdx);
  emit.fast.movRegToReg<S::Q>(RegFrame, Reg::rcx);
  emit.fast.movImmToReg<S::Q>(codeBlock_->isStrictMode(), Reg::r8);

  uint8_t *externConstAddr;
  emit.slow = getConstant(
      emit.slow, (void *)externSlowPathGetArgumentsPropByVal, externConstAddr);
  emit.fast = callExternal(
      emit.fast, externConstAddr, ip->iGetArgumentsPropByVal.op1, ip);
  return emit;
}

Emitters FastJIT::compileBitNot(Emitters emit, const Inst *ip) {
  uint8_t *slowPathConstAddr;
  emit.slow =
      getConstant(emit.slow, (void *)externSlowPathBitNot, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  emit.fast = isNumber(emit.fast, ip->iBitNot.op2, slowPathAddr);
  emit.fast =
      movHermesRegToNativeReg<true>(emit.fast, ip->iBitNot.op2, Reg::XMM0);
  // Convert with Truncation Scalar Double-Precision Floating-Point Value to
  // Signed Integer
  emit.fast.cvttsd2siRegToReg(Reg::XMM0, Reg::eax);
  // Convert Doubleword Integer to Scalar Double-Precision Floating-Point Value
  emit.fast.cvtsi2sdRegToReg(Reg::eax, Reg::XMM1);
  emit.fast.ucomisRegToReg(Reg::XMM0, Reg::XMM1);
  // if op2 is not already a int32, jump to slow path
  emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);
  emit.fast.notReg<S::L>(Reg::eax);
  emit.fast.cvtsi2sdRegToReg(Reg::eax, Reg::XMM0);
  emit.fast =
      movNativeRegToHermesReg<true>(emit.fast, Reg::XMM0, ip->iBitNot.op1);

  // Slow path
  emit.slow = leaHermesReg(emit.slow, ip->iBitNot.op2, Reg::rsi);
  emit.slow = callExternal(emit.slow, slowPathConstAddr, ip->iBitNot.op1, ip);
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileGetArgumentsLength(Emitters emit, const Inst *ip) {
  uint8_t *slowPathConstAddr;
  emit.slow = getConstant(
      emit.slow, (void *)slowPathGetArgumentsLength, slowPathConstAddr);
  uint8_t *slowPathAddr = emit.slow.current();

  // Fast path: when op2 is undefined
  emit.fast =
      cmpSomeNPETag<ETag::Undefined>(emit.fast, ip->iGetArgumentsLength.op2);
  emit.fast.cjump<CCode::NE, OffsetType::Int32>(slowPathAddr);
  // We only load the lower 32 bits value which are the real arg count, while
  // the higher 32 bits are the native tag
  emit.fast.movRMToReg<S::L>(
      RegFrame,
      Reg::NoIndex,
      sizeof(HermesValue) * StackFrameLayout::ArgCount,
      Reg::eax);
  emit.fast.cvtsi2sdRegToReg(Reg::rax, Reg::XMM0);
  emit.fast = movNativeRegToHermesReg<true>(
      emit.fast, Reg::XMM0, ip->iGetArgumentsLength.op1);

  // Slow path:
  emit.slow = leaHermesReg(emit.slow, ip->iGetArgumentsLength.op2, Reg::rsi);
  emit.slow = callExternal(
      emit.slow, slowPathConstAddr, ip->iGetArgumentsLength.op1, ip);
  emit.slow.jmp<OffsetType::Auto>(emit.fast.current());
  describeSlowPathSection(emit.slow, false);

  return emit;
}

Emitters FastJIT::compileCreateRegExp(Emitters emit, const Inst *ip) {
  emit.fast.movImmToReg<S::L>(ip->iCreateRegExp.op2, Reg::esi);
  emit.fast.movImmToReg<S::L>(ip->iCreateRegExp.op3, Reg::edx);
  emit.fast.movImmToReg<S::L>(ip->iCreateRegExp.op4, Reg::ecx);
  emit = loadConstantAddrIntoNativeReg(emit, codeBlock_, Reg::r8);

  uint8_t *externConstAddr;
  emit.slow = getConstant(
      emit.slow, (void *)externCreateRegExpMayAllocate, externConstAddr);
  emit.fast =
      callExternal(emit.fast, externConstAddr, ip->iCreateRegExp.op1, ip);
  return emit;
}

} // namespace x86_64
} // namespace vm
} // namespace hermes
