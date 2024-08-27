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
  Emitter em(impl_->jr, getDumpJITCode(), codeBlock->getFrameSize(), 0, 0);
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

    while (ip != to) {
      switch (ip->opCode) {
        case inst::OpCode::LoadParam:
          em.loadParam(FR(ip->iLoadParam.op1), ip->iLoadParam.op2);
          ip = NEXTINST(LoadParam);
          break;
        case inst::OpCode::LoadConstZero:
          em.loadConstUInt8(FR(ip->iLoadConstZero.op1), 0);
          ip = NEXTINST(LoadConstZero);
          break;
        case inst::OpCode::LoadConstUInt8:
          em.loadConstUInt8(
              FR(ip->iLoadConstUInt8.op1), ip->iLoadConstUInt8.op2);
          ip = NEXTINST(LoadConstUInt8);
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

        case inst::OpCode::Ret:
          em.ret(FR(ip->iRet.op1));
          ip = NEXTINST(Ret);
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
