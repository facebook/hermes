/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeGenerator.h"

#include "hermes/FrontEndDefs/Builtins.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/Format.h"

#include <unordered_map>

namespace hermes {
namespace hbc {

unsigned BytecodeFunctionGenerator::getStringID(LiteralString *value) const {
  return BMGen_.getStringID(value->getValue().str());
}

unsigned BytecodeFunctionGenerator::getIdentifierID(
    LiteralString *value) const {
  return BMGen_.getIdentifierID(value->getValue().str());
}

uint32_t BytecodeFunctionGenerator::addBigInt(bigint::ParsedBigInt bigint) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addBigInt(std::move(bigint));
}

uint32_t BytecodeFunctionGenerator::addRegExp(CompiledRegExp *regexp) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addRegExp(regexp);
}

uint32_t BytecodeFunctionGenerator::addFilename(llvh::StringRef filename) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  return BMGen_.addFilename(filename);
}

void BytecodeFunctionGenerator::addExceptionHandler(
    HBCExceptionHandlerInfo info) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  exceptionHandlers_.push_back(info);
}

void BytecodeFunctionGenerator::patchDebugSourceLocations(
    const llvh::DenseMap<unsigned, unsigned> &scopeDescOffsetMap) {
  if (debugLocations_.empty()) {
    return;
  }

  auto offset = scopeDescOffsetMap.find(sourceLocation_.scopeAddress);
  assert(offset != scopeDescOffsetMap.end());
  sourceLocation_.scopeAddress = offset->second;
  for (DebugSourceLocation &dl : debugLocations_) {
    offset = scopeDescOffsetMap.find(dl.scopeAddress);
    assert(offset != scopeDescOffsetMap.end());
    dl.scopeAddress = offset->second;
  }
}

void BytecodeFunctionGenerator::addDebugSourceLocation(
    const DebugSourceLocation &info) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  // If an address is repeated, it means no actual bytecode was emitted for the
  // previous source location.
  if (!debugLocations_.empty() &&
      debugLocations_.back().address == info.address) {
    debugLocations_.back() = info;
  } else {
    debugLocations_.push_back(info);
  }
}

void BytecodeFunctionGenerator::setJumpTable(
    std::vector<uint32_t> &&jumpTable) {
  assert(
      !complete_ &&
      "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
  assert(!jumpTable.empty() && "invoked with no jump table");

  jumpTable_ = std::move(jumpTable);
}

std::unique_ptr<BytecodeFunction>
BytecodeFunctionGenerator::generateBytecodeFunction(
    Function::DefinitionKind definitionKind,
    ValueKind valueKind,
    bool strictMode,
    uint32_t paramCount,
    uint32_t environmentSize,
    uint32_t nameID) {
  if (!complete_) {
    bytecodeGenerationComplete();
  }

  FunctionHeader header{
      bytecodeSize_,
      paramCount,
      frameSize_,
      environmentSize,
      nameID,
      highestReadCacheIndex_,
      highestWriteCacheIndex_};

  switch (definitionKind) {
    case Function::DefinitionKind::ES6Arrow:
    case Function::DefinitionKind::ES6Method:
      header.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitConstruct;
      break;
    case Function::DefinitionKind::ES6Constructor:
      header.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitCall;
      break;
    default:
      // ES9.0 9.2.3 step 4 states that generator functions and async
      // functions cannot be constructed.
      // We place this check outside the `DefinitionKind` because generator
      // functions may also be ES6 methods, for example, and are not included
      // in the DefinitionKind enum.
      // Note that we only have to check for GeneratorFunctionKind in this
      // case, because ES6 methods are already checked above, and ES6
      // constructors are prohibited from being generator functions.
      // As such, this is the only case in which we must change the
      // prohibitInvoke flag based on valueKind.
      header.flags.prohibitInvoke =
          (valueKind == ValueKind::GeneratorFunctionKind ||
           valueKind == ValueKind::AsyncFunctionKind)
          ? FunctionHeaderFlag::ProhibitConstruct
          : FunctionHeaderFlag::ProhibitNone;
      break;
  }

  header.flags.strictMode = strictMode;
  header.flags.hasExceptionHandler = exceptionHandlers_.size();

  return std::make_unique<BytecodeFunction>(
      std::move(opcodes_), std::move(header), std::move(exceptionHandlers_));
}

unsigned BytecodeFunctionGenerator::getFunctionID(Function *F) {
  return BMGen_.addFunction(F);
}

unsigned BytecodeFunctionGenerator::getScopeDescID(ScopeDesc *S) {
  return BMGen_.addScopeDesc(S);
}

void BytecodeFunctionGenerator::shrinkJump(offset_t loc) {
  // We are shrinking a long jump into a short jump.
  // The size of operand reduces from 4 bytes to 1 byte, a delta of 3.
  opcodes_.erase(opcodes_.begin() + loc, opcodes_.begin() + loc + 3);

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

void BytecodeFunctionGenerator::bytecodeGenerationComplete() {
  assert(!complete_ && "Can only call bytecodeGenerationComplete once");
  complete_ = true;
  bytecodeSize_ = opcodes_.size();

  // Add the jump tables inline with the opcodes, as a 4-byte aligned section at
  // the end of the opcode array.
  if (!jumpTable_.empty()) {
    uint32_t alignedOpcodes = llvh::alignTo<sizeof(uint32_t)>(bytecodeSize_);
    uint32_t jumpTableBytes = jumpTable_.size() * sizeof(uint32_t);
    opcodes_.reserve(alignedOpcodes + jumpTableBytes);
    opcodes_.resize(alignedOpcodes, 0);
    const opcode_atom_t *jumpTableStart =
        reinterpret_cast<opcode_atom_t *>(jumpTable_.data());
    opcodes_.insert(
        opcodes_.end(), jumpTableStart, jumpTableStart + jumpTableBytes);
  }
}

unsigned BytecodeModuleGenerator::addFunction(Function *F) {
  lazyFunctions_ |= F->isLazy();
  asyncFunctions_ |= llvh::isa<AsyncFunction>(F);
  return functionIDMap_.allocate(F);
}

unsigned BytecodeModuleGenerator::addScopeDesc(ScopeDesc *S) {
  if (S && S->hasFunction() &&
      S->getFunction()->getContext().getDebugInfoSetting() !=
          DebugInfoSetting::ALL) {
    // short-circuit S to be nullptr when not emitting the debug information,
    // which effectively causes all scopes to use the same (empty) descriptor.
    S = nullptr;
  }

  newScopeDescs_.insert(S);
  return scopeDescIDMap_.allocate(S);
}

namespace {
/// \return the UTF8 encoding for \p name, replacing surrogates if any is
/// present.
static Identifier ensureUTF8Identifer(
    StringTable &st,
    Identifier name,
    std::string &nameUTF8Buffer) {
  // Check if name has any surrogates. If it doesn't, then it already is a valid
  // UTF8 string, and as such can be written to the debug info.
  bool hasSurrogate = false;
  const char *it = name.str().begin();
  const char *nameEnd = name.str().end();
  while (it < nameEnd && !hasSurrogate) {
    constexpr bool allowSurrogates = false;
    decodeUTF8<allowSurrogates>(
        it, [&hasSurrogate](const llvh::Twine &) { hasSurrogate = true; });
  }

  if (hasSurrogate) {
    nameUTF8Buffer.clear();

    // name has surrogates, meaning it is not a valid UTF8 string (these strings
    // are still valid within Hermes itself).
    convertUTF8WithSurrogatesToUTF8WithReplacements(nameUTF8Buffer, name.str());
    name = st.getIdentifier(nameUTF8Buffer);
  }

  return name;
}
} // namespace

unsigned BytecodeModuleGenerator::serializeScopeChain(
    StringTable &st,
    DebugInfoGenerator &debugInfoGen,
    ScopeDesc *S) {
  unsigned ID = addScopeDesc(S);
  auto it = scopeDescIDAddr_.find(ID);
  if (it != scopeDescIDAddr_.end()) {
    return it->second;
  }

  OptValue<unsigned> parentScopeOffset;
  bool isInnerScope{false};
  llvh::SmallVector<Identifier, 4> names;
  if (S) {
    if (S->getParent() && S->getParent()->hasFunction()) {
      parentScopeOffset = serializeScopeChain(st, debugInfoGen, S->getParent());
    }
    isInnerScope =
        S->hasFunction() && S->getFunction()->getFunctionScopeDesc() != S;

    std::string nameUTF8Buffer;
    for (Variable *V : S->getVariables()) {
      Identifier nameUTF8 =
          ensureUTF8Identifer(st, V->getName(), nameUTF8Buffer);
      names.push_back(nameUTF8);
    }
  }

  unsigned offset =
      debugInfoGen.appendScopeDesc(parentScopeOffset, isInnerScope, names);
  scopeDescIDAddr_[ID] = offset;
  return offset;
}

void BytecodeModuleGenerator::setFunctionGenerator(
    Function *F,
    unique_ptr<BytecodeFunctionGenerator> BFG) {
  assert(
      functionGenerators_.find(F) == functionGenerators_.end() &&
      "Adding same function twice.");
  assert(
      !BFG->hasEncodingError() && "Error should have been reported already.");
  functionGenerators_[F] = std::move(BFG);
}

unsigned BytecodeModuleGenerator::getStringID(llvh::StringRef str) const {
  return stringTable_.getStringID(str);
}

unsigned BytecodeModuleGenerator::getIdentifierID(llvh::StringRef str) const {
  return stringTable_.getIdentifierID(str);
}

void BytecodeModuleGenerator::initializeStringTable(
    StringLiteralTable stringTable) {
  assert(stringTable_.empty() && "String table must be empty");
  stringTable_ = std::move(stringTable);
}

uint32_t BytecodeModuleGenerator::addBigInt(bigint::ParsedBigInt bigint) {
  return bigIntTable_.addBigInt(std::move(bigint));
}

void BytecodeModuleGenerator::initializeSerializedLiterals(
    std::vector<unsigned char> &&arrayBuffer,
    std::vector<unsigned char> &&keyBuffer,
    std::vector<unsigned char> &&valBuffer,
    hermes::hbc::BytecodeModuleGenerator::LiteralOffsetMapTy &&offsetMap) {
  assert(
      arrayBuffer_.empty() && objKeyBuffer_.empty() && objValBuffer_.empty() &&
      literalOffsetMap_.empty() && "serialized literals already initialized");
  arrayBuffer_ = std::move(arrayBuffer);
  objKeyBuffer_ = std::move(keyBuffer);
  objValBuffer_ = std::move(valBuffer);
  literalOffsetMap_ = std::move(offsetMap);
}

uint32_t BytecodeModuleGenerator::addRegExp(CompiledRegExp *regexp) {
  return regExpTable_.addRegExp(regexp);
}

uint32_t BytecodeModuleGenerator::addFilename(llvh::StringRef filename) {
  return filenameTable_.addFilename(filename);
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
  cjsModulesStatic_.push_back({moduleID, functionID});
}

void BytecodeModuleGenerator::addFunctionSource(
    uint32_t functionID,
    uint32_t stringID) {
  functionSourceTable_.push_back({functionID, stringID});
}

std::unique_ptr<BytecodeModule> BytecodeModuleGenerator::generate() {
  assert(
      valid_ &&
      "BytecodeModuleGenerator::generate() cannot be called more than once");
  valid_ = false;

  assert(
      functionIDMap_.getElements().size() == functionGenerators_.size() &&
      "Missing functions.");

  auto kinds = stringTable_.getStringKinds();
  auto hashes = stringTable_.getIdentifierHashes();

  BytecodeOptions bytecodeOptions;
  bytecodeOptions.hasAsync = asyncFunctions_;
  bytecodeOptions.staticBuiltins = options_.staticBuiltinsEnabled;
  bytecodeOptions.cjsModulesStaticallyResolved = !cjsModulesStatic_.empty();
  std::unique_ptr<BytecodeModule> BM{new BytecodeModule(
      functionGenerators_.size(),
      std::move(kinds),
      std::move(hashes),
      stringTable_.acquireStringTable(),
      stringTable_.acquireStringStorage(),
      bigIntTable_.getEntryList(),
      bigIntTable_.getDigitsBuffer(),
      regExpTable_.getEntryList(),
      regExpTable_.getBytecodeBuffer(),
      entryPointIndex_,
      std::move(arrayBuffer_),
      std::move(objKeyBuffer_),
      std::move(objValBuffer_),
      segmentID_,
      std::move(cjsModules_),
      std::move(cjsModulesStatic_),
      std::move(functionSourceTable_),
      bytecodeOptions)};

  DebugInfoGenerator debugInfoGen{std::move(filenameTable_)};

  const uint32_t strippedFunctionNameId =
      options_.stripFunctionNames ? getStringID(kStrippedFunctionName) : 0;
  auto functions = functionIDMap_.getElements();
  for (unsigned i = 0, e = functions.size(); i < e; ++i) {
    auto *F = functions[i];
    auto &BFG = *functionGenerators_[F];

    uint32_t functionNameId = options_.stripFunctionNames
        ? strippedFunctionNameId
        : getStringID(functions[i]->getOriginalOrInferredName().str());

    std::unique_ptr<BytecodeFunction> func = BFG.generateBytecodeFunction(
        F->getDefinitionKind(),
        F->getKind(),
        F->isStrictMode(),
        F->getExpectedParamCountIncludingThis(),
        F->getFunctionScopeDesc()->getVariables().size(),
        functionNameId);

    if (F->isLazy()) {
      auto lazyData = std::make_unique<LazyCompilationData>();
      lazyData->context = F->getParent()->shareContext();
      lazyData->parentScope = F->getLazyScope();
      lazyData->span = F->getLazySource().functionRange;
      lazyData->nodeKind = F->getLazySource().nodeKind;
      lazyData->paramYield = F->getLazySource().paramYield;
      lazyData->paramAwait = F->getLazySource().paramAwait;
      lazyData->bufferId = F->getLazySource().bufferId;
      lazyData->originalName = F->getOriginalOrInferredName();
      lazyData->closureAlias = F->getLazyClosureAlias()
          ? F->getLazyClosureAlias()->getName()
          : Identifier();
      lazyData->strictMode = F->isStrictMode();
      func->setLazyCompilationData(std::move(lazyData));
    }

    if (BFG.hasDebugInfo()) {
      // Only serialize the scopes if any debug info has been output. Note that
      // using a for each construct is wrong as serializeScopeChain may add
      // entries to newScopeDescs_ (via addScopeDesc) which may in turn
      // re-allocate the vector being iterated on.
      for (uint32_t s = 0, end = newScopeDescs_.size(); s < end; ++s) {
        serializeScopeChain(
            F->getContext().getStringTable(), debugInfoGen, newScopeDescs_[s]);
      }

      // All scopes in newScopeDesc_ have already been serialized, thus
      // newScopeDescs_ can be cleared.
      newScopeDescs_.clear();

      // The scope descriptor table is serialized, thus it can be used to patch
      // any references to it in the current function's debug data.
      BFG.patchDebugSourceLocations(scopeDescIDAddr_);

      uint32_t sourceLocOffset = debugInfoGen.appendSourceLocations(
          BFG.getSourceLocation(), i, BFG.getDebugLocations());
      uint32_t topLevelScopeDescDataOffset = serializeScopeChain(
          F->getContext().getStringTable(),
          debugInfoGen,
          F->getFunctionScopeDesc());
      uint32_t textifiedCalleesOffset =
          debugInfoGen.appendTextifiedCalleeData(BFG.getTextifiedCallees());
      func->setDebugOffsets(
          {sourceLocOffset,
           topLevelScopeDescDataOffset,
           textifiedCalleesOffset});
    }

    BM->setFunction(i, std::move(func));
  }

  BM->setDebugInfo(debugInfoGen.serializeWithMove());
  return BM;
}

} // namespace hbc
} // namespace hermes
