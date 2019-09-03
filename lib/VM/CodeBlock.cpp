/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "codeblock"

#include "hermes/VM/CodeBlock.h"

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/SerializedLiteralParser.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

namespace hermes {
namespace vm {

using namespace hermes::inst;
using llvm::ArrayRef;
using llvm::StringRef;
using SLP = SerializedLiteralParser;

#ifndef NDEBUG

static void validateInstructions(ArrayRef<uint8_t> list, unsigned frameSize) {
  const OperandAddr32 listSize = (OperandAddr32)list.size();
  assert((size_t)listSize == list.size() && "more than 2GB instructions!");

  auto validateUInt8 = [](...) {};
  auto validateUInt16 = [](...) {};
  auto validateUInt32 = [](...) {};
  auto validateImm32 = [](...) {};
  auto validateDouble = [](...) {};
  auto validateReg8 = [&](OperandAddr32, OperandReg8 reg8) {
    assert(reg8 < frameSize && "invalid register index");
  };
  auto validateReg32 = [&](OperandAddr32, OperandReg32 reg32) {
    assert(reg32 < frameSize && "invalid register index");
  };
  auto validateAddr32 = [&](OperandAddr32 ip, OperandAddr32 offset) {
    // Check the offset while avoiding overflow.
    assert(
        (offset < 0 ? ip + offset >= 0 : offset < listSize - ip) &&
        "invalid jmp offset");
  };
  auto validateAddr8 = [&](OperandAddr32 ip, OperandAddr8 offset) {
    validateAddr32(ip, offset);
  };

  for (OperandAddr32 ip = 0; ip != listSize;) {
    assert(ip < listSize);
    auto *inst = reinterpret_cast<const Inst *>(&list[ip]);
    switch (inst->opCode) {
#define DEFINE_OPCODE_0(name)    \
  case OpCode::name:             \
    ip += sizeof(inst->i##name); \
    break;
#define DEFINE_OPCODE_1(name, op1type)        \
  case OpCode::name:                          \
    validate##op1type(ip, inst->i##name.op1); \
    ip += sizeof(inst->i##name);              \
    break;
#define DEFINE_OPCODE_2(name, op1type, op2type) \
  case OpCode::name:                            \
    validate##op1type(ip, inst->i##name.op1);   \
    validate##op2type(ip, inst->i##name.op2);   \
    ip += sizeof(inst->i##name);                \
    break;
#define DEFINE_OPCODE_3(name, op1type, op2type, op3type) \
  case OpCode::name:                                     \
    validate##op1type(ip, inst->i##name.op1);            \
    validate##op2type(ip, inst->i##name.op2);            \
    validate##op3type(ip, inst->i##name.op3);            \
    ip += sizeof(inst->i##name);                         \
    break;
#define DEFINE_OPCODE_4(name, op1type, op2type, op3type, op4type) \
  case OpCode::name:                                              \
    validate##op1type(ip, inst->i##name.op1);                     \
    validate##op2type(ip, inst->i##name.op2);                     \
    validate##op3type(ip, inst->i##name.op3);                     \
    validate##op4type(ip, inst->i##name.op4);                     \
    ip += sizeof(inst->i##name);                                  \
    break;
#define DEFINE_OPCODE_5(name, op1type, op2type, op3type, op4type, op5type) \
  case OpCode::name:                                                       \
    validate##op1type(ip, inst->i##name.op1);                              \
    validate##op2type(ip, inst->i##name.op2);                              \
    validate##op3type(ip, inst->i##name.op3);                              \
    validate##op4type(ip, inst->i##name.op4);                              \
    validate##op5type(ip, inst->i##name.op5);                              \
    ip += sizeof(inst->i##name);                                           \
    break;
#define DEFINE_OPCODE_6(                                        \
    name, op1type, op2type, op3type, op4type, op5type, op6type) \
  case OpCode::name:                                            \
    validate##op1type(ip, inst->i##name.op1);                   \
    validate##op2type(ip, inst->i##name.op2);                   \
    validate##op3type(ip, inst->i##name.op3);                   \
    validate##op4type(ip, inst->i##name.op4);                   \
    validate##op5type(ip, inst->i##name.op5);                   \
    validate##op6type(ip, inst->i##name.op6);                   \
    ip += sizeof(inst->i##name);                                \
    break;

#include "hermes/BCGen/HBC/BytecodeList.def"
      default:
        llvm_unreachable("invalid opcode");
    }
  }
}

#endif

CodeBlock *CodeBlock::createCodeBlock(
    RuntimeModule *runtimeModule,
    hbc::RuntimeFunctionHeader header,
    const uint8_t *bytecode,
    uint32_t functionID) {
#ifndef NDEBUG
  validateInstructions(
      {bytecode, header.bytecodeSizeInBytes()}, header.frameSize());
#endif

  // Compute size needed for caching from the highest accessed indices.
  // If the highest access index is 0, that function does not use this cache at
  // all so there is no reason to allocate it. If the function does access the
  // cache we need to allocate an extra slot for the no-cache indicator.
  auto sizeComputer = [](uint8_t highest) -> uint32_t {
    return highest == 0 ? 0 : highest + 1;
  };

  uint32_t readCacheSize = sizeComputer(header.highestReadCacheIndex());
  uint32_t cacheSize =
      readCacheSize + sizeComputer(header.highestWriteCacheIndex());

#ifndef HERMESVM_LEAN
  bool isCodeBlockLazy = !bytecode;
  if (!runtimeModule->isInitialized() || isCodeBlockLazy) {
    readCacheSize = sizeComputer(std::numeric_limits<uint8_t>::max());
    cacheSize = 2 * readCacheSize;
  }
#endif

  return CodeBlock::create(
      runtimeModule, header, bytecode, functionID, cacheSize, readCacheSize);
}

int32_t CodeBlock::findCatchTargetOffset(uint32_t exceptionOffset) {
  return runtimeModule_->getBytecode()->findCatchTargetOffset(
      functionID_, exceptionOffset);
}

SLP CodeBlock::getArrayBufferIter(uint32_t idx, unsigned int numLiterals)
    const {
  return SLP{runtimeModule_->getBytecode()->getArrayBuffer().slice(idx),
             numLiterals,
             runtimeModule_};
}

std::pair<SLP, SLP> CodeBlock::getObjectBufferIter(
    uint32_t keyIdx,
    uint32_t valIdx,
    unsigned int numLiterals) const {
  return std::pair<SLP, SLP>{
      SLP{runtimeModule_->getBytecode()->getObjectKeyBuffer().slice(keyIdx),
          numLiterals,
          nullptr},
      SLP{runtimeModule_->getBytecode()->getObjectValueBuffer().slice(valIdx),
          numLiterals,
          runtimeModule_}};
}

SymbolID CodeBlock::getNameMayAllocate() const {
#ifndef HERMESVM_LEAN
  if (isLazy()) {
    return runtimeModule_->getLazyName();
  }
#endif
  return runtimeModule_->getSymbolIDFromStringIDMayAllocate(
      functionHeader_.functionName());
}

bool CodeBlock::getNameString(Runtime *runtime, std::string &res) const {
#ifndef HERMESVM_LEAN
  if (isLazy()) {
    return runtimeModule_->getLazyNameString(runtime, res);
  }
#endif
  return runtimeModule_->getStringFromStringID(
      functionHeader_.functionName(), res);
}

OptValue<uint32_t> CodeBlock::getDebugSourceLocationsOffset() const {
  auto *debugOffsets =
      runtimeModule_->getBytecode()->getDebugOffsets(functionID_);
  if (!debugOffsets)
    return llvm::None;
  uint32_t ret = debugOffsets->sourceLocations;
  if (ret == hbc::DebugOffsets::NO_OFFSET)
    return llvm::None;
  return ret;
}

OptValue<uint32_t> CodeBlock::getDebugLexicalDataOffset() const {
  auto *debugOffsets =
      runtimeModule_->getBytecode()->getDebugOffsets(functionID_);
  if (!debugOffsets)
    return llvm::None;
  uint32_t ret = debugOffsets->lexicalData;
  if (ret == hbc::DebugOffsets::NO_OFFSET)
    return llvm::None;
  return ret;
}

SourceErrorManager::SourceCoords CodeBlock::getLazyFunctionLoc(
    bool start) const {
  assert(isLazy() && "Function must be lazy");
  SourceErrorManager::SourceCoords coords;
#ifndef HERMESVM_LEAN
  auto *func = ((hbc::BCProviderLazy *)runtimeModule_->getBytecode())
                   ->getBytecodeFunction();
  auto *lazyData = func->getLazyCompilationData();
  lazyData->context->getSourceErrorManager().findBufferLineAndLoc(
      start ? lazyData->span.Start : lazyData->span.End, coords);
#endif
  return coords;
}

#ifndef HERMESVM_LEAN
namespace {
std::unique_ptr<hbc::BytecodeModule> compileLazyFunction(
    hbc::LazyCompilationData *lazyData) {
  assert(lazyData);
  LLVM_DEBUG(
      llvm::dbgs() << "Compiling lazy function " << lazyData->originalName
                   << "\n");

  Module M{lazyData->context};
  Function *entryPoint = hermes::generateLazyFunctionIR(lazyData, &M);

  auto bytecodeModule = hbc::generateBytecodeModule(
      &M, entryPoint, BytecodeGenerationOptions::defaults());

  return bytecodeModule;
}
} // namespace

void CodeBlock::lazyCompileImpl(Runtime *runtime) {
  assert(isLazy() && "Laziness has not been checked");
  PerfSection perf("Lazy function compilation");
  auto *func = ((hbc::BCProviderLazy *)runtimeModule_->getBytecode())
                   ->getBytecodeFunction();
  auto bcMod = compileLazyFunction(func->getLazyCompilationData());
  runtimeModule_->initializeLazyMayAllocate(
      hbc::BCProviderFromSrc::createBCProviderFromSrc(std::move(bcMod)));
  // Reset all meta data of the CodeBlock to point to the newly
  // generated bytecode module.
  functionID_ = runtimeModule_->getBytecode()->getGlobalFunctionIndex();
  functionHeader_ =
      runtimeModule_->getBytecode()->getFunctionHeader(functionID_);
  bytecode_ = runtimeModule_->getBytecode()->getBytecode(functionID_);
}
#endif // HERMESVM_LEAN

void CodeBlock::markCachedHiddenClasses(WeakRefAcceptor &acceptor) {
  for (auto &prop :
       llvm::makeMutableArrayRef(propertyCache(), propertyCacheSize_)) {
    if (prop.clazz) {
      acceptor.acceptWeak(reinterpret_cast<void *&>(prop.clazz));
    }
  }
}

uint32_t CodeBlock::getVirtualOffset() const {
  return getRuntimeModule()->getBytecode()->getVirtualOffsetForFunction(
      functionID_);
}

#ifdef HERMES_ENABLE_DEBUGGER

uint32_t CodeBlock::getNextOffset(uint32_t offset) const {
  auto opcodes = getOpcodeArray();
  assert(offset < opcodes.size() && "invalid offset to breakOnNextInstruction");

  auto opCode = reinterpret_cast<const Inst *>(&opcodes[offset])->opCode;
  assert(opCode < OpCode::_last && "invalid opecode");

  static const uint8_t sizes[] = {
#define DEFINE_OPCODE(name) sizeof(inst::name##Inst),
#include "hermes/BCGen/HBC/BytecodeList.def"
  };

  return offset + sizes[(unsigned)opCode];
}

/// Makes the page that \p address is in writable.
/// If it fails, aborts execution.
static void makeWritable(void *address, size_t length) {
  void *endAddress = static_cast<void *>(static_cast<char *>(address) + length);

  // Align the address to page size before setting the pagesize.
  void *alignedAddress = safeTypeCast<size_t, void *>(llvm::alignDown(
      safeTypeCast<void *, size_t>(address), hermes::oscompat::page_size()));

  size_t totalLength =
      static_cast<char *>(endAddress) - static_cast<char *>(alignedAddress);

  bool success = oscompat::vm_protect(
      alignedAddress, totalLength, oscompat::ProtectMode::ReadWrite);
  if (!success) {
    hermes_fatal("mprotect failed before modifying breakpoint");
  }
}

void CodeBlock::installBreakpointAtOffset(uint32_t offset) {
  auto opcodes = getOpcodeArray();
  assert(offset < opcodes.size() && "patch offset out of bounds");
  hbc::opcode_atom_t *address =
      const_cast<hbc::opcode_atom_t *>(opcodes.begin() + offset);
  hbc::opcode_atom_t debuggerOpcode =
      static_cast<hbc::opcode_atom_t>(OpCode::Debugger);

  static_assert(
      sizeof(inst::DebuggerInst) == 1,
      "debugger instruction can only be a single opcode atom");

  makeWritable(address, sizeof(inst::DebuggerInst));
  *address = debuggerOpcode;
}

void CodeBlock::uninstallBreakpointAtOffset(
    uint32_t offset,
    hbc::opcode_atom_t opCode) {
  auto opcodes = getOpcodeArray();
  assert(offset < opcodes.size() && "unpatch offset out of bounds");
  hbc::opcode_atom_t *address =
      const_cast<hbc::opcode_atom_t *>(opcodes.begin() + offset);
  assert(
      *address == static_cast<hbc::opcode_atom_t>(OpCode::Debugger) &&
      "can't uninstall a non-debugger instruction");

  // This is valid because we can only uninstall breakpoints that we installed.
  // Therefore, the page here must be writable.
  *address = opCode;
}

#endif

#ifdef HERMESVM_SERIALIZE
void CodeBlock::serialize(Serializer &s) const {
  // The identity of a CodeBlock is its functionId. Note: We don't
  // serialize/deserialize PropertyCache as of now.
  // TODO: serialize/deserialize PropertyCacheEntry.
  s.writeInt<uint32_t>(getFunctionID());
  s.endObject(this);
}

CodeBlock *CodeBlock::deserialize(
    Deserializer &d,
    RuntimeModule *runtimeModule) {
  uint32_t index = d.readInt<uint32_t>();
  auto *res = CodeBlock::createCodeBlock(
      runtimeModule,
      runtimeModule->getBytecode()->getFunctionHeader(index),
      runtimeModule->getBytecode()->getBytecode(index),
      index);
  d.endObject(res);
  return res;
}

#endif

} // namespace vm
} // namespace hermes
