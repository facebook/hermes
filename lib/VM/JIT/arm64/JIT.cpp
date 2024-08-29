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
  if (dumpJITCode_) {
    funcName = codeBlock->getNameString();
    llvh::outs() << "\nJIT compilation of FunctionID "
                 << codeBlock->getFunctionID() << ", '" << funcName << "'\n";
  }

  std::vector<uint32_t> basicBlocks{};
  llvh::DenseMap<uint32_t, unsigned> ofsToBBIndex{};

  bool fail = false;
  discoverBasicBlocks(codeBlock, basicBlocks, ofsToBBIndex);

  const char *funcStart = (const char *)codeBlock->begin();

  if (dumpJITCode_ && !funcName.empty())
    llvh::outs() << "\n" << funcName << ":\n";

  // TODO: is getFrameSize() the right thing to call?
  Emitter em(
      impl_->jr,
      getDumpJITCode(),
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

        case inst::OpCode::Dec:
          em.dec(FR(ip->iDec.op1), FR(ip->iDec.op2));
          ip = NEXTINST(Dec);
          break;

        case inst::OpCode::JGreaterEqual:
          em.jGreaterEqual(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJGreaterEqual.op1]],
              FR(ip->iJGreaterEqual.op2),
              FR(ip->iJGreaterEqual.op3));
          ip = NEXTINST(JGreaterEqual);
          break;
        case inst::OpCode::JGreaterEqualN:
          em.jGreaterEqualN(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJGreaterEqual.op1]],
              FR(ip->iJGreaterEqual.op2),
              FR(ip->iJGreaterEqual.op3));
          ip = NEXTINST(JGreaterEqual);
          break;
        case inst::OpCode::JNotGreaterEqual:
          em.jGreaterEqual(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJNotGreaterEqual.op1]],
              FR(ip->iJNotGreaterEqual.op2),
              FR(ip->iJNotGreaterEqual.op3));
          ip = NEXTINST(JNotGreaterEqual);
          break;
        case inst::OpCode::JNotGreaterEqualN:
          em.jGreaterEqualN(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJNotGreaterEqual.op1]],
              FR(ip->iJNotGreaterEqual.op2),
              FR(ip->iJNotGreaterEqual.op3));
          ip = NEXTINST(JNotGreaterEqual);
          break;
        case inst::OpCode::JGreater:
          em.jGreater(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJGreater.op1]],
              FR(ip->iJGreater.op2),
              FR(ip->iJGreater.op3));
          ip = NEXTINST(JGreater);
          break;
        case inst::OpCode::JGreaterN:
          em.jGreaterN(
              false,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJGreater.op1]],
              FR(ip->iJGreater.op2),
              FR(ip->iJGreater.op3));
          ip = NEXTINST(JGreater);
          break;
        case inst::OpCode::JNotGreater:
          em.jGreater(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJNotGreater.op1]],
              FR(ip->iJNotGreater.op2),
              FR(ip->iJNotGreater.op3));
          ip = NEXTINST(JNotGreater);
          break;
        case inst::OpCode::JNotGreaterN:
          em.jGreaterN(
              true,
              labels[ofsToBBIndex
                         [(const char *)ip - (const char *)funcStart +
                          ip->iJNotGreater.op1]],
              FR(ip->iJNotGreater.op2),
              FR(ip->iJNotGreater.op3));
          ip = NEXTINST(JNotGreater);
          break;

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
        case inst::OpCode::GetParentEnvironment:
          em.getParentEnvironment(
              FR(ip->iGetParentEnvironment.op1), ip->iGetParentEnvironment.op2);
          ip = NEXTINST(GetParentEnvironment);
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
              codeBlock,
              FR(ip->iNewObjectWithBuffer.op1),
              ip->iNewObjectWithBuffer.op2,
              ip->iNewObjectWithBuffer.op3);
          ip = NEXTINST(NewObjectWithBuffer);
          break;

        default:
          if (crashOnError_) {
            llvh::errs() << "*** Unsupported instruction: "
                         << llvh::format_decimal(
                                (const char *)ip - (const char *)funcStart, 3)
                         << ": " << inst::decodeInstruction(ip) << "\n";
            hermes_fatal("jit: unsupported instruction");
          } else {
            if (dumpJITCode_) {
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

  if (dumpJITCode_) {
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
