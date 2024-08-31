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
  (codeBlock->getRuntimeModule()        \
       ->getSymbolIDMustExist(stringID) \
       .unsafeGetIndex())

JITCompiledFunctionPtr JITContext::compileImpl(
    Runtime &runtime,
    CodeBlock *codeBlock) {
  std::string funcName{};
  if (dumpJITCode_ & (DumpJitCode::Code | DumpJitCode::CompileStatus)) {
    funcName = codeBlock->getNameString();
    llvh::outs() << "\nJIT compilation of FunctionID "
                 << codeBlock->getFunctionID() << ", '" << funcName << "'\n";
  }

  std::vector<uint32_t> basicBlocks{};
  llvh::DenseMap<uint32_t, unsigned> ofsToBBIndex{};

  bool fail = false;
  discoverBasicBlocks(codeBlock, basicBlocks, ofsToBBIndex);

  const char *funcStart = (const char *)codeBlock->begin();

  if ((dumpJITCode_ & DumpJitCode::Code) && !funcName.empty())
    llvh::outs() << "\n" << funcName << ":\n";

  // TODO: is getFrameSize() the right thing to call?
  Emitter em(
      impl_->jr,
      getDumpJITCode(),
      codeBlock,
      codeBlock->propertyCache(),
      codeBlock->writePropertyCache(),
      codeBlock->getFrameSize(),
      0,
      0);
  std::vector<asmjit::Label> labels{};
  labels.reserve(basicBlocks.size() - 1);
  for (unsigned bbIndex = 0; bbIndex < basicBlocks.size() - 1; ++bbIndex)
    labels.push_back(em.newPrefLabel("BB", bbIndex));

  for (unsigned bbIndex = 0; bbIndex < basicBlocks.size() - 1; ++bbIndex) {
    uint32_t startOfs = basicBlocks[bbIndex];
    uint32_t endOfs = basicBlocks[bbIndex + 1];

    em.newBasicBlock(labels[bbIndex]);
    auto *ip = reinterpret_cast<const inst::Inst *>(funcStart + startOfs);
    auto *to = reinterpret_cast<const inst::Inst *>(funcStart + endOfs);

    SHSymbolID idVal;
    uint8_t cacheIdx;
    const inst::Inst *nextIP;

    while (ip != to) {
      switch (ip->opCode) {
        case inst::OpCode::LoadParam:
          em.loadParam(FR(ip->iLoadParam.op1), ip->iLoadParam.op2);
          ip = NEXTINST(LoadParam);
          break;
        case inst::OpCode::LoadConstZero:
          em.loadConstDouble(FR(ip->iLoadConstZero.op1), 0, "Zero");
          ip = NEXTINST(LoadConstZero);
          break;
        case inst::OpCode::LoadConstUInt8:
          em.loadConstDouble(
              FR(ip->iLoadConstUInt8.op1), ip->iLoadConstUInt8.op2, "UInt8");
          ip = NEXTINST(LoadConstUInt8);
          break;
        case inst::OpCode::LoadConstInt:
          em.loadConstDouble(
              FR(ip->iLoadConstInt.op1), ip->iLoadConstInt.op2, "Int");
          ip = NEXTINST(LoadConstInt);
          break;
        case inst::OpCode::LoadConstDouble:
          em.loadConstDouble(
              FR(ip->iLoadConstDouble.op1), ip->iLoadConstDouble.op2, "Double");
          ip = NEXTINST(LoadConstDouble);
          break;
#define LOAD_CONST(NAME, val, type)                                     \
  case inst::OpCode::LoadConst##NAME:                                   \
    em.loadConstBits64(FR(ip->iLoadConst##NAME.op1), val, type, #NAME); \
    ip = NEXTINST(LoadConst##NAME);                                     \
    break;
          LOAD_CONST(Empty, _sh_ljs_empty().raw, FRType::Unknown);
          LOAD_CONST(Undefined, _sh_ljs_undefined().raw, FRType::Unknown);
          LOAD_CONST(Null, _sh_ljs_null().raw, FRType::Unknown);
          LOAD_CONST(True, _sh_ljs_bool(true).raw, FRType::Bool);
          LOAD_CONST(False, _sh_ljs_bool(false).raw, FRType::Bool);
#undef LOAD_CONST
        case inst::OpCode::LoadConstString:
          em.loadConstString(
              FR(ip->iLoadConstString.op1),
              codeBlock->getRuntimeModule(),
              ip->iLoadConstString.op2);
          ip = NEXTINST(LoadConstString);
          break;
        case inst::OpCode::LoadConstStringLongIndex:
          em.loadConstString(
              FR(ip->iLoadConstStringLongIndex.op1),
              codeBlock->getRuntimeModule(),
              ip->iLoadConstStringLongIndex.op2);
          ip = NEXTINST(LoadConstStringLongIndex);
          break;

        case inst::OpCode::Mov:
          em.mov(FR(ip->iMov.op1), FR(ip->iMov.op2));
          ip = NEXTINST(Mov);
          break;
        case inst::OpCode::ToNumber:
          em.toNumber(FR(ip->iToNumber.op1), FR(ip->iToNumber.op2));
          ip = NEXTINST(ToNumber);
          break;
        case inst::OpCode::ToNumeric:
          em.toNumeric(FR(ip->iToNumeric.op1), FR(ip->iToNumeric.op2));
          ip = NEXTINST(ToNumeric);
          break;

        case inst::OpCode::Greater:
          em.greater(
              FR(ip->iGreater.op1), FR(ip->iGreater.op2), FR(ip->iGreater.op3));
          ip = NEXTINST(Greater);
          break;
        case inst::OpCode::GreaterEq:
          em.greaterEqual(
              FR(ip->iGreaterEq.op1),
              FR(ip->iGreaterEq.op2),
              FR(ip->iGreaterEq.op3));
          ip = NEXTINST(GreaterEq);
          break;
        case inst::OpCode::Less:
          em.less(FR(ip->iLess.op1), FR(ip->iLess.op2), FR(ip->iLess.op3));
          ip = NEXTINST(Less);
          break;
        case inst::OpCode::LessEq:
          em.lessEqual(
              FR(ip->iLessEq.op1), FR(ip->iLessEq.op2), FR(ip->iLessEq.op3));
          ip = NEXTINST(LessEq);
          break;
        case inst::OpCode::Eq:
          em.equal(FR(ip->iEq.op1), FR(ip->iEq.op2), FR(ip->iEq.op3));
          ip = NEXTINST(Eq);
          break;
        case inst::OpCode::Neq:
          em.notEqual(FR(ip->iNeq.op1), FR(ip->iNeq.op2), FR(ip->iNeq.op3));
          ip = NEXTINST(Neq);
          break;
        case inst::OpCode::StrictEq:
          em.strictEqual(
              FR(ip->iStrictEq.op1),
              FR(ip->iStrictEq.op2),
              FR(ip->iStrictEq.op3));
          ip = NEXTINST(StrictEq);
          break;
        case inst::OpCode::StrictNeq:
          em.strictNotEqual(
              FR(ip->iStrictNeq.op1),
              FR(ip->iStrictNeq.op2),
              FR(ip->iStrictNeq.op3));
          ip = NEXTINST(StrictNeq);
          break;

        case inst::OpCode::Add:
          em.add(FR(ip->iAdd.op1), FR(ip->iAdd.op2), FR(ip->iAdd.op3));
          ip = NEXTINST(Add);
          break;
        case inst::OpCode::AddN:
          em.addN(FR(ip->iAdd.op1), FR(ip->iAdd.op2), FR(ip->iAdd.op3));
          ip = NEXTINST(Add);
          break;
        case inst::OpCode::Sub:
          em.sub(FR(ip->iSub.op1), FR(ip->iSub.op2), FR(ip->iSub.op3));
          ip = NEXTINST(Add);
          break;
        case inst::OpCode::SubN:
          em.subN(FR(ip->iSub.op1), FR(ip->iSub.op2), FR(ip->iSub.op3));
          ip = NEXTINST(SubN);
          break;
        case inst::OpCode::Mul:
          em.mul(FR(ip->iMul.op1), FR(ip->iMul.op2), FR(ip->iMul.op3));
          ip = NEXTINST(Mul);
          break;
        case inst::OpCode::MulN:
          em.mulN(FR(ip->iMul.op1), FR(ip->iMul.op2), FR(ip->iMul.op3));
          ip = NEXTINST(Mul);
          break;
        case inst::OpCode::Div:
          em.div(FR(ip->iDiv.op1), FR(ip->iDiv.op2), FR(ip->iDiv.op3));
          ip = NEXTINST(Div);
          break;
        case inst::OpCode::DivN:
          em.divN(FR(ip->iDivN.op1), FR(ip->iDivN.op2), FR(ip->iDivN.op3));
          ip = NEXTINST(DivN);
          break;
        case inst::OpCode::Mod:
          em.mod(false, FR(ip->iMod.op1), FR(ip->iMod.op2), FR(ip->iMod.op3));
          ip = NEXTINST(Mod);
          break;
        case inst::OpCode::BitAnd:
          em.bitAnd(
              FR(ip->iBitAnd.op1), FR(ip->iBitAnd.op2), FR(ip->iBitAnd.op3));
          ip = NEXTINST(BitAnd);
          break;
        case inst::OpCode::BitOr:
          em.bitOr(FR(ip->iBitOr.op1), FR(ip->iBitOr.op2), FR(ip->iBitOr.op3));
          ip = NEXTINST(BitOr);
          break;
        case inst::OpCode::BitXor:
          em.bitXor(
              FR(ip->iBitXor.op1), FR(ip->iBitXor.op2), FR(ip->iBitXor.op3));
          ip = NEXTINST(BitXor);
          break;
        case inst::OpCode::LShift:
          em.lShift(
              FR(ip->iLShift.op1), FR(ip->iLShift.op2), FR(ip->iLShift.op3));
          ip = NEXTINST(LShift);
          break;
        case inst::OpCode::RShift:
          em.rShift(
              FR(ip->iRShift.op1), FR(ip->iRShift.op2), FR(ip->iRShift.op3));
          ip = NEXTINST(RShift);
          break;
        case inst::OpCode::URshift:
          em.urShift(
              FR(ip->iURshift.op1), FR(ip->iURshift.op2), FR(ip->iURshift.op3));
          ip = NEXTINST(URshift);
          break;

        case inst::OpCode::Inc:
          em.inc(FR(ip->iInc.op1), FR(ip->iInc.op2));
          ip = NEXTINST(Inc);
          break;
        case inst::OpCode::Dec:
          em.dec(FR(ip->iDec.op1), FR(ip->iDec.op2));
          ip = NEXTINST(Dec);
          break;

        case inst::OpCode::Not:
          em.booleanNot(FR(ip->iNot.op1), FR(ip->iNot.op2));
          ip = NEXTINST(Not);
          break;
        case inst::OpCode::BitNot:
          em.bitNot(FR(ip->iNot.op1), FR(ip->iNot.op2));
          ip = NEXTINST(Not);
          break;
        case inst::OpCode::Negate:
          em.negate(FR(ip->iNegate.op1), FR(ip->iNegate.op2));
          ip = NEXTINST(Negate);
          break;
        case inst::OpCode::TypeOf:
          em.typeOf(FR(ip->iTypeOf.op1), FR(ip->iTypeOf.op2));
          ip = NEXTINST(TypeOf);
          break;

#define JUMP(name, emit, invert)                                 \
  case inst::OpCode::name:                                       \
    em.emit(                                                     \
        invert,                                                  \
        labels[ofsToBBIndex                                      \
                   [(const char *)ip - (const char *)funcStart + \
                    ip->i##name.op1]],                           \
        FR(ip->i##name.op2),                                     \
        FR(ip->i##name.op3));                                    \
    ip = NEXTINST(name);                                         \
    break;                                                       \
  case inst::OpCode::name##Long:                                 \
    em.emit(                                                     \
        invert,                                                  \
        labels[ofsToBBIndex                                      \
                   [(const char *)ip - (const char *)funcStart + \
                    ip->i##name##Long.op1]],                     \
        FR(ip->i##name##Long.op2),                               \
        FR(ip->i##name##Long.op3));                              \
    ip = NEXTINST(name##Long);                                   \
    break;

          JUMP(JLessEqual, jLessEqual, false)
          JUMP(JLessEqualN, jLessEqualN, false)
          JUMP(JNotLessEqual, jLessEqual, true)
          JUMP(JNotLessEqualN, jLessEqualN, true)
          JUMP(JLess, jLess, false)
          JUMP(JLessN, jLessN, false)
          JUMP(JNotLess, jLess, true)
          JUMP(JNotLessN, jLessN, true)
          JUMP(JGreaterEqual, jGreaterEqual, false)
          JUMP(JGreaterEqualN, jGreaterEqualN, false)
          JUMP(JNotGreaterEqual, jGreaterEqual, true)
          JUMP(JNotGreaterEqualN, jGreaterEqualN, true)
          JUMP(JGreater, jGreater, false)
          JUMP(JGreaterN, jGreaterN, false)
          JUMP(JNotGreater, jGreater, true)
          JUMP(JNotGreaterN, jGreaterN, true)
          JUMP(JEqual, jEqual, false)
          JUMP(JNotEqual, jEqual, true)
          JUMP(JStrictEqual, jStrictEqual, false)
          JUMP(JStrictNotEqual, jStrictEqual, true)
#undef JUMP

        case inst::OpCode::JmpTrue:
          em.jmpTrueFalse(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart + ip->iJmpTrue.op1]],
              FR(ip->iJmpTrue.op2));
          ip = NEXTINST(JmpTrue);
          break;
        case inst::OpCode::JmpTrueLong:
          em.jmpTrueFalse(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart + ip->iJmpTrueLong.op1]],
              FR(ip->iJmpTrueLong.op2));
          ip = NEXTINST(JmpTrueLong);
          break;
        case inst::OpCode::JmpFalse:
          em.jmpTrueFalse(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart + ip->iJmpFalse.op1]],
              FR(ip->iJmpFalse.op2));
          ip = NEXTINST(JmpFalse);
          break;
        case inst::OpCode::JmpFalseLong:
          em.jmpTrueFalse(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart +
                          ip->iJmpFalseLong.op1]],
              FR(ip->iJmpFalseLong.op2));
          ip = NEXTINST(JmpFalseLong);
          break;
        case inst::OpCode::Jmp:
          em.jmp(
              labels
                  [ofsToBBIndex[(const char *)ip - funcStart + ip->iJmp.op1]]);
          ip = NEXTINST(Jmp);
          break;
        case inst::OpCode::JmpLong:
          em.jmp(labels[ofsToBBIndex
                            [(const char *)ip - funcStart + ip->iJmpLong.op1]]);
          ip = NEXTINST(JmpLong);
          break;
        case inst::OpCode::JmpUndefined:
          em.jmpUndefined(
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart +
                          ip->iJmpUndefined.op1]],
              FR(ip->iJmpUndefined.op2));
          ip = NEXTINST(JmpUndefined);
          break;
        case inst::OpCode::JmpUndefinedLong:
          em.jmpUndefined(
              labels[ofsToBBIndex
                         [(const char *)ip - funcStart +
                          ip->iJmpUndefinedLong.op1]],
              FR(ip->iJmpUndefinedLong.op2));
          ip = NEXTINST(JmpUndefinedLong);
          break;

        case inst::OpCode::TryGetByIdLong:
          idVal = ID(ip->iTryGetByIdLong.op4);
          cacheIdx = ip->iTryGetByIdLong.op3;
          nextIP = NEXTINST(TryGetByIdLong);
          goto tryGetById;
        case inst::OpCode::TryGetById:
          idVal = ID(ip->iTryGetById.op4);
          cacheIdx = ip->iTryGetById.op3;
          nextIP = NEXTINST(TryGetById);
          goto tryGetById;
        tryGetById: {
          em.tryGetById(
              FR(ip->iTryGetById.op1),
              idVal,
              FR(ip->iTryGetById.op2),
              cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::GetByIdLong:
          idVal = ID(ip->iGetByIdLong.op4);
          cacheIdx = ip->iGetByIdLong.op3;
          nextIP = NEXTINST(GetByIdLong);
          goto getById;
        case inst::OpCode::GetById:
          idVal = ID(ip->iGetById.op4);
          cacheIdx = ip->iGetById.op3;
          nextIP = NEXTINST(GetById);
          goto getById;
        case inst::OpCode::GetByIdShort:
          idVal = ID(ip->iGetByIdShort.op4);
          cacheIdx = ip->iGetByIdShort.op3;
          nextIP = NEXTINST(GetByIdShort);
          goto getById;
        getById: {
          em.getById(
              FR(ip->iGetById.op1), idVal, FR(ip->iGetById.op2), cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::TryPutByIdLooseLong:
          idVal = ID(ip->iTryPutByIdLooseLong.op4);
          cacheIdx = ip->iTryPutByIdLooseLong.op3;
          nextIP = NEXTINST(TryPutByIdLooseLong);
          goto tryPutByIdLoose;
        case inst::OpCode::TryPutByIdLoose:
          idVal = ID(ip->iTryPutByIdLoose.op4);
          cacheIdx = ip->iTryPutByIdLoose.op3;
          nextIP = NEXTINST(TryPutByIdLoose);
          goto tryPutByIdLoose;
        tryPutByIdLoose: {
          em.tryPutByIdLoose(
              FR(ip->iTryPutByIdLoose.op1),
              idVal,
              FR(ip->iTryPutByIdLoose.op2),
              cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::TryPutByIdStrictLong:
          idVal = ID(ip->iTryPutByIdStrictLong.op4);
          cacheIdx = ip->iTryPutByIdStrictLong.op3;
          nextIP = NEXTINST(TryPutByIdStrictLong);
          goto tryPutByIdStrict;
        case inst::OpCode::TryPutByIdStrict:
          idVal = ID(ip->iTryPutByIdStrict.op4);
          cacheIdx = ip->iTryPutByIdStrict.op3;
          nextIP = NEXTINST(TryPutByIdStrict);
          goto tryPutByIdStrict;
        tryPutByIdStrict: {
          em.tryPutByIdStrict(
              FR(ip->iTryPutByIdStrict.op1),
              idVal,
              FR(ip->iTryPutByIdStrict.op2),
              cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::PutByIdLooseLong:
          idVal = ID(ip->iPutByIdLooseLong.op4);
          cacheIdx = ip->iPutByIdLooseLong.op3;
          nextIP = NEXTINST(PutByIdLooseLong);
          goto putByIdLoose;
        case inst::OpCode::PutByIdLoose:
          idVal = ID(ip->iPutByIdLoose.op4);
          cacheIdx = ip->iPutByIdLoose.op3;
          nextIP = NEXTINST(PutByIdLoose);
          goto putByIdLoose;
        putByIdLoose: {
          em.putByIdLoose(
              FR(ip->iPutByIdLoose.op1),
              idVal,
              FR(ip->iPutByIdLoose.op2),
              cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::PutByIdStrictLong:
          idVal = ID(ip->iPutByIdStrictLong.op4);
          cacheIdx = ip->iPutByIdStrictLong.op3;
          nextIP = NEXTINST(PutByIdStrictLong);
          goto putByIdStrict;
        case inst::OpCode::PutByIdStrict:
          idVal = ID(ip->iPutByIdStrict.op4);
          cacheIdx = ip->iPutByIdStrict.op3;
          nextIP = NEXTINST(PutByIdStrict);
          goto putByIdStrict;
        putByIdStrict: {
          em.putByIdStrict(
              FR(ip->iPutByIdStrict.op1),
              idVal,
              FR(ip->iPutByIdStrict.op2),
              cacheIdx);
          ip = nextIP;
          break;
        }

        case inst::OpCode::GetByVal:
          em.getByVal(
              FR(ip->iGetByVal.op1),
              FR(ip->iGetByVal.op2),
              FR(ip->iGetByVal.op3));
          ip = NEXTINST(GetByVal);
          break;
        case inst::OpCode::PutByValLoose:
          em.putByValLoose(
              FR(ip->iPutByValLoose.op1),
              FR(ip->iPutByValLoose.op2),
              FR(ip->iPutByValLoose.op3));
          ip = NEXTINST(PutByValLoose);
          break;
        case inst::OpCode::PutByValStrict:
          em.putByValStrict(
              FR(ip->iPutByValStrict.op1),
              FR(ip->iPutByValStrict.op2),
              FR(ip->iPutByValStrict.op3));
          ip = NEXTINST(PutByValStrict);
          break;

        case inst::OpCode::GetByIndex:
          em.getByIndex(
              FR(ip->iGetByIndex.op1),
              FR(ip->iGetByIndex.op2),
              ip->iGetByIndex.op3);
          ip = NEXTINST(GetByIndex);
          break;

        case inst::OpCode::PutOwnByIndex:
          em.putOwnByIndex(
              FR(ip->iPutOwnByIndex.op1),
              FR(ip->iPutOwnByIndex.op2),
              ip->iPutOwnByIndex.op3);
          ip = NEXTINST(PutOwnByIndex);
          break;
        case inst::OpCode::PutOwnByIndexL:
          em.putOwnByIndex(
              FR(ip->iPutOwnByIndexL.op1),
              FR(ip->iPutOwnByIndexL.op2),
              ip->iPutOwnByIndexL.op3);
          ip = NEXTINST(PutOwnByIndexL);
          break;

        case inst::OpCode::PutOwnByVal:
          em.putOwnByVal(
              FR(ip->iPutOwnByVal.op1),
              FR(ip->iPutOwnByVal.op2),
              FR(ip->iPutOwnByVal.op3),
              (bool)ip->iPutOwnByVal.op4);
          ip = NEXTINST(PutOwnByVal);
          break;

        case inst::OpCode::PutNewOwnById:
          em.putNewOwnById(
              FR(ip->iPutNewOwnById.op1),
              FR(ip->iPutNewOwnById.op2),
              ID(ip->iPutNewOwnById.op3),
              true);
          ip = NEXTINST(PutNewOwnById);
          break;
        case inst::OpCode::PutNewOwnByIdLong:
          em.putNewOwnById(
              FR(ip->iPutNewOwnByIdLong.op1),
              FR(ip->iPutNewOwnByIdLong.op2),
              ID(ip->iPutNewOwnByIdLong.op3),
              true);
          ip = NEXTINST(PutNewOwnByIdLong);
          break;
        case inst::OpCode::PutNewOwnByIdShort:
          em.putNewOwnById(
              FR(ip->iPutNewOwnByIdShort.op1),
              FR(ip->iPutNewOwnByIdShort.op2),
              ID(ip->iPutNewOwnByIdShort.op3),
              true);
          ip = NEXTINST(PutNewOwnByIdShort);
          break;

        case inst::OpCode::PutNewOwnNEById:
          em.putNewOwnById(
              FR(ip->iPutNewOwnNEById.op1),
              FR(ip->iPutNewOwnNEById.op2),
              ID(ip->iPutNewOwnNEById.op3),
              false);
          ip = NEXTINST(PutNewOwnNEById);
          break;
        case inst::OpCode::PutNewOwnNEByIdLong:
          em.putNewOwnById(
              FR(ip->iPutNewOwnNEByIdLong.op1),
              FR(ip->iPutNewOwnNEByIdLong.op2),
              ID(ip->iPutNewOwnNEByIdLong.op3),
              false);
          ip = NEXTINST(PutNewOwnNEByIdLong);
          break;

        case inst::OpCode::PutOwnBySlotIdx:
          em.putOwnBySlotIdx(
              FR(ip->iPutOwnBySlotIdx.op1),
              FR(ip->iPutOwnBySlotIdx.op2),
              ip->iPutOwnBySlotIdx.op3);
          ip = NEXTINST(PutOwnBySlotIdx);
          break;

        case inst::OpCode::PutOwnBySlotIdxLong:
          em.putOwnBySlotIdx(
              FR(ip->iPutOwnBySlotIdxLong.op1),
              FR(ip->iPutOwnBySlotIdxLong.op2),
              ip->iPutOwnBySlotIdxLong.op3);
          ip = NEXTINST(PutOwnBySlotIdxLong);
          break;

        case inst::OpCode::GetOwnBySlotIdx:
          em.getOwnBySlotIdx(
              FR(ip->iGetOwnBySlotIdx.op1),
              FR(ip->iGetOwnBySlotIdx.op2),
              ip->iGetOwnBySlotIdx.op3);
          ip = NEXTINST(PutOwnBySlotIdx);
          break;

        case inst::OpCode::GetOwnBySlotIdxLong:
          em.getOwnBySlotIdx(
              FR(ip->iGetOwnBySlotIdxLong.op1),
              FR(ip->iGetOwnBySlotIdxLong.op2),
              ip->iGetOwnBySlotIdxLong.op3);
          ip = NEXTINST(GetOwnBySlotIdxLong);
          break;

        case inst::OpCode::TypedLoadParent:
          em.typedLoadParent(
              FR(ip->iTypedLoadParent.op1), FR(ip->iTypedLoadParent.op2));
          ip = NEXTINST(TypedLoadParent);
          break;
        case inst::OpCode::TypedStoreParent:
          em.typedStoreParent(
              FR(ip->iTypedStoreParent.op1), FR(ip->iTypedStoreParent.op2));
          ip = NEXTINST(TypedStoreParent);
          break;

        case inst::OpCode::Ret:
          em.ret(FR(ip->iRet.op1));
          ip = NEXTINST(Ret);
          break;

        case inst::OpCode::GetGlobalObject:
          em.getGlobalObject(FR(ip->iGetGlobalObject.op1));
          ip = NEXTINST(GetGlobalObject);
          break;

        case inst::OpCode::IsIn:
          em.isIn(FR(ip->iIsIn.op1), FR(ip->iIsIn.op2), FR(ip->iIsIn.op3));
          ip = NEXTINST(IsIn);
          break;

        case inst::OpCode::Call:
          em.call(
              FR(ip->iCall.op1),
              /* callee */ FR(ip->iCall.op2),
              /* argc */ ip->iCall.op3);
          ip = NEXTINST(Call);
          break;

        case inst::OpCode::Call1:
          em.callN(
              FR(ip->iCall1.op1),
              /* callee */ FR(ip->iCall1.op2),
              /* args */ {FR(ip->iCall1.op3)});
          ip = NEXTINST(Call1);
          break;

        case inst::OpCode::Call2:
          em.callN(
              FR(ip->iCall2.op1),
              /* callee */ FR(ip->iCall2.op2),
              /* args */ {FR(ip->iCall2.op3), FR(ip->iCall2.op4)});
          ip = NEXTINST(Call2);
          break;

        case inst::OpCode::Call3:
          em.callN(
              FR(ip->iCall3.op1),
              /* callee */ FR(ip->iCall3.op2),
              /* args */
              {FR(ip->iCall3.op3), FR(ip->iCall3.op4), FR(ip->iCall3.op5)});
          ip = NEXTINST(Call3);
          break;

        case inst::OpCode::Call4:
          em.callN(
              FR(ip->iCall4.op1),
              /* callee */ FR(ip->iCall4.op2),
              /* args */
              {FR(ip->iCall4.op3),
               FR(ip->iCall4.op4),
               FR(ip->iCall4.op5),
               FR(ip->iCall4.op6)});
          ip = NEXTINST(Call4);
          break;

        case inst::OpCode::Construct:
          em.callWithNewTarget(
              FR(ip->iConstruct.op1),
              /* callee */ FR(ip->iConstruct.op2),
              /* newTarget */ FR(ip->iConstruct.op2),
              /* argc */ ip->iCall.op3);
          ip = NEXTINST(Construct);
          break;

        case inst::OpCode::CallBuiltin:
          em.callBuiltin(
              FR(ip->iCallBuiltin.op1),
              /* builtinIndex */ ip->iCallBuiltin.op2,
              /* argc */ ip->iCallBuiltin.op3);
          ip = NEXTINST(CallBuiltin);
          break;

        case inst::OpCode::GetBuiltinClosure:
          em.getBuiltinClosure(
              FR(ip->iGetBuiltinClosure.op1),
              /* builtinIndex */ ip->iGetBuiltinClosure.op2);
          ip = NEXTINST(GetBuiltinClosure);
          break;

        case inst::OpCode::DeclareGlobalVar:
          em.declareGlobalVar(ID(ip->iDeclareGlobalVar.op1));
          ip = NEXTINST(DeclareGlobalVar);
          break;
        case inst::OpCode::CreateTopLevelEnvironment:
          em.createTopLevelEnvironment(
              FR(ip->iCreateTopLevelEnvironment.op1),
              ip->iCreateTopLevelEnvironment.op2);
          ip = NEXTINST(CreateTopLevelEnvironment);
          break;
        case inst::OpCode::CreateFunctionEnvironment:
          em.createFunctionEnvironment(
              FR(ip->iCreateFunctionEnvironment.op1),
              ip->iCreateFunctionEnvironment.op2);
          ip = NEXTINST(CreateFunctionEnvironment);
          break;
        case inst::OpCode::CreateEnvironment:
          em.createEnvironment(
              FR(ip->iCreateEnvironment.op1),
              FR(ip->iCreateEnvironment.op2),
              ip->iCreateEnvironment.op3);
          ip = NEXTINST(CreateEnvironment);
          break;
        case inst::OpCode::GetParentEnvironment:
          em.getParentEnvironment(
              FR(ip->iGetParentEnvironment.op1), ip->iGetParentEnvironment.op2);
          ip = NEXTINST(GetParentEnvironment);
          break;
        case inst::OpCode::GetClosureEnvironment:
          em.getClosureEnvironment(
              FR(ip->iGetClosureEnvironment.op1),
              FR(ip->iGetClosureEnvironment.op2));
          ip = NEXTINST(GetClosureEnvironment);
          break;
        case inst::OpCode::LoadFromEnvironment:
          em.loadFromEnvironment(
              FR(ip->iLoadFromEnvironment.op1),
              FR(ip->iLoadFromEnvironment.op2),
              ip->iLoadFromEnvironment.op3);
          ip = NEXTINST(LoadFromEnvironment);
          break;
        case inst::OpCode::LoadFromEnvironmentL:
          em.loadFromEnvironment(
              FR(ip->iLoadFromEnvironmentL.op1),
              FR(ip->iLoadFromEnvironmentL.op2),
              ip->iLoadFromEnvironmentL.op3);
          ip = NEXTINST(LoadFromEnvironmentL);
          break;
        case inst::OpCode::StoreToEnvironment:
          em.storeToEnvironment(
              false,
              FR(ip->iStoreToEnvironment.op1),
              ip->iStoreToEnvironment.op2,
              FR(ip->iStoreToEnvironment.op3));
          ip = NEXTINST(StoreToEnvironment);
          break;
        case inst::OpCode::StoreToEnvironmentL:
          em.storeToEnvironment(
              false,
              FR(ip->iStoreToEnvironmentL.op1),
              ip->iStoreToEnvironmentL.op2,
              FR(ip->iStoreToEnvironmentL.op3));
          ip = NEXTINST(StoreToEnvironmentL);
          break;
        case inst::OpCode::StoreNPToEnvironment:
          em.storeToEnvironment(
              true,
              FR(ip->iStoreNPToEnvironment.op1),
              ip->iStoreToEnvironment.op2,
              FR(ip->iStoreToEnvironment.op3));
          ip = NEXTINST(StoreNPToEnvironment);
          break;
        case inst::OpCode::StoreNPToEnvironmentL:
          em.storeToEnvironment(
              true,
              FR(ip->iStoreNPToEnvironmentL.op1),
              ip->iStoreToEnvironmentL.op2,
              FR(ip->iStoreToEnvironmentL.op3));
          ip = NEXTINST(StoreNPToEnvironmentL);
          break;

        case inst::OpCode::CreateClosure:
          em.createClosure(
              FR(ip->iCreateClosure.op1),
              FR(ip->iCreateClosure.op2),
              codeBlock->getRuntimeModule(),
              ip->iCreateClosure.op3);
          ip = NEXTINST(CreateClosure);
          break;
        case inst::OpCode::CreateClosureLongIndex:
          em.createClosure(
              FR(ip->iCreateClosureLongIndex.op1),
              FR(ip->iCreateClosureLongIndex.op2),
              codeBlock->getRuntimeModule(),
              ip->iCreateClosureLongIndex.op3);
          ip = NEXTINST(CreateClosureLongIndex);
          break;

        case inst::OpCode::NewObject:
          em.newObject(FR(ip->iNewObject.op1));
          ip = NEXTINST(NewObject);
          break;
        case inst::OpCode::NewObjectWithParent:
          em.newObjectWithParent(
              FR(ip->iNewObjectWithParent.op1),
              FR(ip->iNewObjectWithParent.op2));
          ip = NEXTINST(NewObjectWithParent);
          break;
        case inst::OpCode::NewObjectWithBuffer:
          em.newObjectWithBuffer(
              FR(ip->iNewObjectWithBuffer.op1),
              ip->iNewObjectWithBuffer.op2,
              ip->iNewObjectWithBuffer.op3);
          ip = NEXTINST(NewObjectWithBuffer);
          break;

        case inst::OpCode::NewArray:
          em.newArray(FR(ip->iNewArray.op1), ip->iNewArray.op2);
          ip = NEXTINST(NewArray);
          break;
        case inst::OpCode::NewArrayWithBuffer:
          em.newArrayWithBuffer(
              FR(ip->iNewArrayWithBuffer.op1),
              ip->iNewArrayWithBuffer.op2,
              ip->iNewArrayWithBuffer.op3,
              ip->iNewArrayWithBuffer.op4);
          ip = NEXTINST(NewArrayWithBuffer);
          break;

        case inst::OpCode::GetPNameList:
          em.getPNameList(
              FR(ip->iGetPNameList.op1),
              FR(ip->iGetPNameList.op2),
              FR(ip->iGetPNameList.op3),
              FR(ip->iGetPNameList.op4));
          ip = NEXTINST(GetPNameList);
          break;
        case inst::OpCode::GetNextPName:
          em.getNextPName(
              FR(ip->iGetNextPName.op1),
              FR(ip->iGetNextPName.op2),
              FR(ip->iGetNextPName.op3),
              FR(ip->iGetNextPName.op4),
              FR(ip->iGetNextPName.op5));
          ip = NEXTINST(GetNextPName);
          break;

        case inst::OpCode::CreateThis:
          em.createThis(
              FR(ip->iCreateThis.op1),
              FR(ip->iCreateThis.op2),
              FR(ip->iCreateThis.op3));
          ip = NEXTINST(CreateThis);
          break;
        case inst::OpCode::SelectObject:
          em.selectObject(
              FR(ip->iSelectObject.op1),
              FR(ip->iSelectObject.op2),
              FR(ip->iSelectObject.op3));
          ip = NEXTINST(SelectObject);
          break;

        case inst::OpCode::LoadThisNS:
          em.loadThisNS(FR(ip->iLoadThisNS.op1));
          ip = NEXTINST(LoadThisNS);
          break;
        case inst::OpCode::CoerceThisNS:
          em.coerceThisNS(FR(ip->iCoerceThisNS.op1), FR(ip->iCoerceThisNS.op2));
          ip = NEXTINST(CoerceThisNS);
          break;

        case inst::OpCode::Debugger:
          em.debugger();
          ip = NEXTINST(Debugger);
          break;

        case inst::OpCode::AddS:
          em.addS(FR(ip->iAddS.op1), FR(ip->iAddS.op2), FR(ip->iAddS.op3));
          ip = NEXTINST(AddS);
          break;

        default:
          if (crashOnError_) {
            llvh::errs() << "*** Unsupported instruction: "
                         << llvh::format_decimal(
                                (const char *)ip - (const char *)funcStart, 3)
                         << ": " << inst::decodeInstruction(ip) << "\n";
            hermes_fatal("jit: unsupported instruction");
          } else {
            if (dumpJITCode_ &
                (DumpJitCode::Code | DumpJitCode::CompileStatus)) {
              llvh::outs() << "** Unsupported instruction: "
                           << llvh::format_decimal(
                                  (const char *)ip - (const char *)funcStart, 3)
                           << ": " << inst::decodeInstruction(ip) << "\n";
            } else {
              LLVM_DEBUG(
                  llvh::outs()
                  << "** Unsupported instruction: "
                  << llvh::format_decimal(
                         (const char *)ip - (const char *)funcStart, 3)
                  << ": " << inst::decodeInstruction(ip) << "\n");
            }
          }
          fail = true;
          goto onError;
      }
    }
  }

onError:
  if (fail) {
    codeBlock->setDontJIT(true);
    return nullptr;
  }

  em.leave();
  codeBlock->setJITCompiled(em.addToRuntime(impl_->jr));

  LLVM_DEBUG(
      llvh::outs() << "\n Bytecode:";
      for (unsigned bbIndex = 0; bbIndex < basicBlocks.size() - 1; ++bbIndex) {
        uint32_t startOfs = basicBlocks[bbIndex];
        uint32_t endOfs = basicBlocks[bbIndex + 1];
        llvh::outs() << "BB" << bbIndex << ":\n";
        auto *ip = funcStart + startOfs;
        auto *to = funcStart + endOfs;
        while (ip != to) {
          auto di = inst::decodeInstruction((const inst::Inst *)ip);
          llvh::outs() << "    " << llvh::format_decimal(ip - funcStart, 3)
                       << ": " << di << "\n";
          ip += di.meta.size;
        }
      });

  if (dumpJITCode_ & (DumpJitCode::Code | DumpJitCode::CompileStatus)) {
    funcName = codeBlock->getNameString();
    llvh::outs() << "\nJIT successfully compiled FunctionID "
                 << codeBlock->getFunctionID() << ", '" << funcName << "'\n";
  }

  return codeBlock->getJITCompiled();
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMESVM_JIT
