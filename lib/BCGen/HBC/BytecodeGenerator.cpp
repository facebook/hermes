/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/BytecodeGenerator.h"

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/Inst/Builtins.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/UTF8.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <locale>
#include <unordered_map>

using hermes::oscompat::to_string;

namespace hermes {
namespace hbc {

unsigned BytecodeFunctionGenerator::addConstantString(
    LiteralString *value,
    bool isIdentifier) {
  // Add the string to the global string table, and return the index.
  return BMGen_.addString(value->getValue().str(), isIdentifier);
}

uint32_t BytecodeFunctionGenerator::addRegExp(CompiledRegExp regexp) {
  return BMGen_.addRegExp(std::move(regexp));
}

uint32_t BytecodeFunctionGenerator::addFilename(StringRef filename) {
  return BMGen_.addFilename(filename);
}

void BytecodeFunctionGenerator::addExceptionHandler(
    HBCExceptionHandlerInfo info) {
  exceptionHandlers_.push_back(info);
}

void BytecodeFunctionGenerator::addDebugSourceLocation(
    const DebugSourceLocation &info) {
  debugLocations_.push_back(info);
}

void BytecodeFunctionGenerator::setJumpTable(
    std::vector<uint32_t> &&jumpTable) {
  assert(!jumpTable.empty() && "invoked with no jump table");

  jumpTable_ = std::move(jumpTable);
}

uint32_t BytecodeModuleGenerator::addArrayBuffer(ArrayRef<Literal *> elements) {
  return literalGenerator_.serializeBuffer(elements, arrayBuffer_, false);
}

std::pair<uint32_t, uint32_t> BytecodeModuleGenerator::addObjectBuffer(
    ArrayRef<Literal *> keys,
    ArrayRef<Literal *> vals) {
  return std::pair<uint32_t, uint32_t>{
      literalGenerator_.serializeBuffer(keys, objKeyBuffer_, true),
      literalGenerator_.serializeBuffer(vals, objValBuffer_, false)};
}

std::unique_ptr<BytecodeFunction>
BytecodeFunctionGenerator::generateBytecodeFunction(
    Function::DefinitionKind definitionKind,
    bool strictMode,
    uint32_t paramCount,
    uint32_t environmentSize,
    uint32_t nameID) {
  return std::unique_ptr<BytecodeFunction>(new BytecodeFunction(
      std::move(opcodes_),
      definitionKind,
      strictMode,
      FunctionHeader(
          bytecodeSize_,
          paramCount,
          frameSize_,
          environmentSize,
          nameID,
          highestReadCacheIndex_,
          highestWriteCacheIndex_),
      std::move(exceptionHandlers_),
      std::move(jumpTable_)));
}

unsigned BytecodeFunctionGenerator::getFunctionID(Function *F) {
  return BMGen_.addFunction(F);
}

void BytecodeFunctionGenerator::shrinkJump(offset_t loc) {
  // We are shrinking a long jump into a short jump.
  // The size of operand reduces from 4 bytes to 1 byte, a delta of 3.
  const static int ShrinkOffset = 3;
  std::rotate(
      opcodes_.begin() + loc,
      opcodes_.begin() + loc + ShrinkOffset,
      opcodes_.end());
  opcodes_.resize(opcodes_.size() - ShrinkOffset);

  // Change this instruction from long jump to short jump.
  longToShortJump(loc - 1);
}

void BytecodeFunctionGenerator::updateJumpTarget(
    offset_t loc,
    int newVal,
    int bytes) {
  // The jump target is encoded in little-endian. Update it correctly
  // regardless of host byte order.
  for (; bytes; --bytes, ++loc) {
    opcodes_[loc] = (opcode_atom_t)(newVal);
    newVal >>= 8;
  }
}

void BytecodeFunctionGenerator::updateJumpTableOffset(
    offset_t loc,
    uint32_t jumpTableOffset,
    uint32_t instLoc) {
  assert(opcodes_.size() > instLoc && "invalid switchimm offset");

  // The offset is not aligned, but will be aligned when read in the
  // interpreter.
  updateJumpTarget(
      loc,
      opcodes_.size() + jumpTableOffset * sizeof(uint32_t) - instLoc,
      sizeof(uint32_t));
}

unsigned BytecodeModuleGenerator::addFunction(Function *F) {
  lazyFunctions_ |= F->isLazy();
  return functionIDMap_.allocate(F);
}

void BytecodeModuleGenerator::setFunctionGenerator(
    Function *F,
    unique_ptr<BytecodeFunctionGenerator> BFG) {
  assert(
      functionGenerators_.find(F) == functionGenerators_.end() &&
      "Adding same function twice.");
  functionGenerators_[F] = std::move(BFG);
}

unsigned BytecodeModuleGenerator::addString(StringRef str, bool isIdentifier) {
  auto id = stringTable_.addString(str);
  if (isIdentifier) {
    identifiers_.insert(id);
  }
  return id;
}

void BytecodeModuleGenerator::initializeStringsFromStorage(
    ConsecutiveStringStorage &&css) {
  assert(stringTable_.empty() && "String table must be empty");
  stringTable_ = UniquingStringTable{std::move(css)};
}

uint32_t BytecodeModuleGenerator::addRegExp(CompiledRegExp regexp) {
  return regExpTable_.addRegExp(std::move(regexp));
}

uint32_t BytecodeModuleGenerator::addFilename(StringRef filename) {
  return filenameTable_.addString(filename);
}

void BytecodeModuleGenerator::addCJSModule(
    uint32_t functionID,
    uint32_t nameID) {
  assert(
      cjsModulesStatic_.empty() &&
      "Statically resolved modules must be in cjsModulesStatic_");
  cjsModules_.push_back({nameID, functionID});
}

void BytecodeModuleGenerator::addCJSModuleStatic(
    uint32_t moduleID,
    uint32_t functionID) {
  assert(cjsModules_.empty() && "Unresolved modules must be in cjsModules_");
  assert(
      moduleID - cjsModuleOffset_ == cjsModulesStatic_.size() &&
      "Module ID out of order in cjsModulesStatic_");
  (void)moduleID;
  cjsModulesStatic_.push_back(functionID);
}

ConsecutiveStringStorage BytecodeModuleGenerator::generateStringStorage() {
  ConsecutiveStringStorage result =
      stringTable_.generateStorage(options_.optimizationEnabled);
  // Mark the identifiers.
  for (auto entryIdx : identifiers_) {
    result.markEntryAsIdentifier(entryIdx);
  }
  return result;
}

void BytecodeModuleGenerator::serializeFunctionNames() {
  if (options_.stripFunctionNames) {
    addString(kStrippedFunctionName, false);
    return;
  }
  auto functions = functionIDMap_.getElements();
  for (unsigned i = 0, e = functions.size(); i < e; ++i) {
    addString(functions[i]->getOriginalOrInferredName().str(), false);
  }
}

std::unique_ptr<BytecodeModule> BytecodeModuleGenerator::generate() {
  assert(
      valid_ &&
      "BytecodeModuleGenerator::generate() cannot be called more than once");
  valid_ = false;

  assert(
      functionIDMap_.getElements().size() == functionGenerators_.size() &&
      "Missing functions.");

  // Function names have to be serialized before generateStringStorage() call.
  serializeFunctionNames();

  ConsecutiveStringStorage stringStorage = generateStringStorage();
  auto hashes = stringStorage.getIdentifierHashes();

  BytecodeOptions bytecodeOptions;
  bytecodeOptions.staticBuiltins = options_.staticBuiltinsEnabled;
  std::unique_ptr<BytecodeModule> BM{new BytecodeModule(
      functionGenerators_.size(),
      stringStorage.acquireStringTable(),
      std::move(hashes),
      stringStorage.acquireStringStorage(),
      regExpTable_.getEntryList(),
      regExpTable_.getBytecodeBuffer(),
      entryPointIndex_,
      std::move(arrayBuffer_),
      std::move(objKeyBuffer_),
      std::move(objValBuffer_),
      cjsModuleOffset_,
      std::move(cjsModules_),
      std::move(cjsModulesStatic_),
      bytecodeOptions)};

  DebugInfoGenerator debugInfoGen{std::move(filenameTable_)};

  const uint32_t strippedFunctionNameId = options_.stripFunctionNames
      ? stringTable_.getExistingStringId(kStrippedFunctionName)
      : 0;
  auto functions = functionIDMap_.getElements();
  for (unsigned i = 0, e = functions.size(); i < e; ++i) {
    auto *F = functions[i];
    auto &BFG = *functionGenerators_[F];

    uint32_t functionNameId = options_.stripFunctionNames
        ? strippedFunctionNameId
        : stringTable_.getExistingStringId(
              functions[i]->getOriginalOrInferredName().str());

    std::unique_ptr<BytecodeFunction> func = BFG.generateBytecodeFunction(
        F->getDefinitionKind(),
        F->isStrictMode(),
        F->getParamCountIncludingThis(),
        F->getFunctionScope()->getVariables().size(),
        functionNameId);

    if (F->isLazy()) {
#ifdef HERMESVM_LEAN
      llvm_unreachable("Lazy support compiled out");
#else
      auto lazyData = llvm::make_unique<LazyCompilationData>();
      lazyData->context = F->getParent()->shareContext();
      lazyData->parentScope = F->getLazyScope();
      lazyData->span = F->getLazySource().functionRange;
      lazyData->nodeKind = F->getLazySource().nodeKind;
      lazyData->bufferId = F->getLazySource().bufferId;
      lazyData->originalName = F->getOriginalOrInferredName();
      lazyData->closureAlias = F->getLazyClosureAlias()
          ? F->getLazyClosureAlias()->getName()
          : Identifier();
      lazyData->strictMode = F->isStrictMode();
      func->setLazyCompilationData(std::move(lazyData));
#endif
    }

    if (BFG.hasDebugInfo()) {
      uint32_t sourceLocOffset = debugInfoGen.appendSourceLocations(
          BFG.getSourceLocation(), i, BFG.getDebugLocations());
      uint32_t lexicalDataOffset = debugInfoGen.appendLexicalData(
          BFG.getLexicalParentID(), BFG.getDebugVariableNames());
      func->setDebugOffsets({sourceLocOffset, lexicalDataOffset});
    }
    BM->setFunction(i, std::move(func));
  }

  BM->setDebugInfo(debugInfoGen.serializeWithMove());
  return BM;
}

} // namespace hbc
} // namespace hermes
