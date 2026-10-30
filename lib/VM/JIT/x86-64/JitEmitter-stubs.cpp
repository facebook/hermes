/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// \file
/// The per-opcode Emitter methods that the x86-64 backend does not implement
/// yet. Each reports itself as unsupported, which aborts compilation of the
/// current function (see Emitter::unsupported()) so the interpreter runs it
/// instead. Later milestones replace these bodies with real code generation,
/// one opcode family at a time, moving each into the topic file that matches
/// the arm64 layout, without changing any signature here.

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter.h"

namespace hermes::vm::x86_64 {

void Emitter::unreachable() {
  unsupported("unreachable");
}

void Emitter::profilePoint(uint16_t point) {
  unsupported("profilePoint");
}

void Emitter::directEval(FR frRes, FR frText, bool strictCaller) {
  unsupported("directEval");
}

void Emitter::catchInst(FR frRes) {
  unsupported("catchInst");
}

void Emitter::loadConstString(
    FR frRes,
    RuntimeModule *runtimeModule,
    uint32_t stringID) {
  unsupported("loadConstString");
}

void Emitter::loadConstBigInt(
    FR frRes,
    RuntimeModule *runtimeModule,
    uint32_t bigIntID) {
  unsupported("loadConstBigInt");
}

void Emitter::addEmptyString(FR frRes, FR frInput) {
  unsupported("addEmptyString");
}

void Emitter::addS(FR frRes, FR frLeft, FR frRight) {
  unsupported("addS");
}

void Emitter::jmpBuiltinIs(
    bool invert,
    const asmjit::Label &target,
    uint8_t builtinIndex,
    FR frInput) {
  unsupported("jmpBuiltinIs");
}

void Emitter::typeOf(FR frRes, FR frInput) {
  unsupported("typeOf");
}

void Emitter::getPNameList(FR frRes, FR frObj, FR frIdx, FR frSize) {
  unsupported("getPNameList");
}

void Emitter::getNextPName(
    FR frRes,
    FR frProps,
    FR frObj,
    FR frIdx,
    FR frSize) {
  unsupported("getNextPName");
}

void Emitter::toPropertyKey(FR frRes, FR frVal) {
  unsupported("toPropertyKey");
}

void Emitter::jmpTypeOfIs(
    const asmjit::Label &target,
    FR frInput,
    TypeOfIsTypes types) {
  unsupported("jmpTypeOfIs");
}

void Emitter::typeOfIs(FR frRes, FR frInput, TypeOfIsTypes types) {
  unsupported("typeOfIs");
}

void Emitter::uintSwitchImm(
    FR frInput,
    const asmjit::Label &defaultLabel,
    llvh::ArrayRef<const asmjit::Label *> labels,
    uint32_t minVal,
    uint32_t maxVal) {
  unsupported("uintSwitchImm");
}

void Emitter::stringSwitchImm(
    FR frInput,
    RuntimeModule *runtimeModule,
    uint32_t tableIndex,
    const asmjit::Label &defaultLabel,
    llvh::ArrayRef<StringSwitchCase> cases) {
  unsupported("stringSwitchImm");
}

void Emitter::newArray(FR frRes, uint32_t size) {
  unsupported("newArray");
}

void Emitter::newArrayWithBuffer(
    FR frRes,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex) {
  unsupported("newArrayWithBuffer");
}

void Emitter::newFastArray(FR frRes, FR frProto, uint32_t size) {
  unsupported("newFastArray");
}

void Emitter::fastArrayLength(FR frRes, FR arr) {
  unsupported("fastArrayLength");
}

void Emitter::fastArrayLoad(FR frRes, FR arr, FR idx) {
  unsupported("fastArrayLoad");
}

void Emitter::fastArrayStore(FR arr, FR idx, FR val) {
  unsupported("fastArrayStore");
}

void Emitter::fastArrayPush(FR arr, FR val) {
  unsupported("fastArrayPush");
}

void Emitter::fastArrayAppend(FR arr, FR other) {
  unsupported("fastArrayAppend");
}

void Emitter::getArgumentsPropByValLoose(FR frRes, FR frIndex, FR frLazyReg) {
  unsupported("getArgumentsPropByValLoose");
}

void Emitter::getArgumentsPropByValStrict(FR frRes, FR frIndex, FR frLazyReg) {
  unsupported("getArgumentsPropByValStrict");
}

void Emitter::reifyArgumentsLoose(FR frLazyReg) {
  unsupported("reifyArgumentsLoose");
}

void Emitter::reifyArgumentsStrict(FR frLazyReg) {
  unsupported("reifyArgumentsStrict");
}

void Emitter::getArgumentsLength(FR frRes, FR frLazyReg) {
  unsupported("getArgumentsLength");
}

void Emitter::iteratorBegin(FR frRes, FR frSource) {
  unsupported("iteratorBegin");
}

void Emitter::iteratorNext(FR frRes, FR frIteratorOrIdx, FR frSourceOrNext) {
  unsupported("iteratorNext");
}

void Emitter::iteratorClose(FR frIteratorOrIdx, bool ignoreExceptions) {
  unsupported("iteratorClose");
}

void Emitter::debugger() {
  unsupported("debugger");
}

void Emitter::throwInst(FR frInput) {
  unsupported("throwInst");
}

void Emitter::throwIfEmpty(FR frRes, FR frInput) {
  unsupported("throwIfEmpty");
}

void Emitter::throwIfUndefined(FR frRes, FR frInput) {
  unsupported("throwIfUndefined");
}

void Emitter::throwIfThisInitialized(FR frInput) {
  unsupported("throwIfThisInitialized");
}

void Emitter::createRegExp(
    FR frRes,
    SHSymbolID patternID,
    SHSymbolID flagsID,
    uint32_t regexpID) {
  unsupported("createRegExp");
}

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
