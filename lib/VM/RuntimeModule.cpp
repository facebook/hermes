/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/RuntimeModule.h"

#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

RuntimeModule::RuntimeModule(
    Runtime *runtime,
    RuntimeModuleFlags flags,
    llvm::StringRef sourceURL)
    : runtime_(runtime), flags_(flags), sourceURL_(sourceURL) {
  runtime_->addRuntimeModule(this);
}

SymbolID RuntimeModule::createSymbolFromStringID(
    StringID stringID,
    const StringTableEntry &entry,
    OptValue<uint32_t> mhash) {
  // Use manual pointer arithmetic to avoid out of bounds errors on empty
  // string accesses.
  auto strStorage = bcProvider_->getStringStorage();
  if (entry.isUTF16()) {
    const char16_t *s =
        (const char16_t *)(strStorage.begin() + entry.getOffset());
    UTF16Ref str{s, entry.getLength()};
    uint32_t hash = mhash ? *mhash : hashString(str);
    return mapString(str, stringID, hash);
  } else {
    // ASCII.
    const char *s = strStorage.begin() + entry.getOffset();
    ASCIIRef str{s, entry.getLength()};
    uint32_t hash = mhash ? *mhash : hashString(str);
    return mapString(str, stringID, hash);
  }
}

RuntimeModule::~RuntimeModule() {
  runtime_->removeRuntimeModule(this);

  // We may reference other CodeBlocks through lazy compilation, but we only
  // own the ones that reference us.
  for (auto *block : functionMap_) {
    if (block != nullptr && block->getRuntimeModule() == this) {
      delete block;
    }
  }

  for (auto *m : dependentModules_) {
    m->removeUser();
  }
}

void RuntimeModule::prepareForRuntimeShutdown() {
  dependentModules_.clear();

  for (int i = 0, e = functionMap_.size(); i < e; i++) {
    if (functionMap_[i] != nullptr &&
        functionMap_[i]->getRuntimeModule() != this) {
      functionMap_[i] = nullptr;
    }
  }
}

RuntimeModule *RuntimeModule::create(
    Runtime *runtime,
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    RuntimeModuleFlags flags,
    llvm::StringRef sourceURL) {
  auto result = new RuntimeModule(runtime, flags, sourceURL);
  if (bytecode) {
    result->initialize(std::move(bytecode));
  }
  return result;
}

void RuntimeModule::initialize(std::shared_ptr<hbc::BCProvider> &&bytecode) {
  assert(!bcProvider_ && "RuntimeModule already initialized");
  bcProvider_ = std::move(bytecode);
  importStringIDMap();
  initializeFunctionMap();
  importCJSModuleTable();
}

RuntimeModule *RuntimeModule::createManual(
    Runtime *runtime,
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    RuntimeModuleFlags flags) {
  auto *result = create(runtime, std::move(bytecode), flags);
  // Make sure the module never gets explicitly deleted.
  result->addUser();
  return result;
}

CodeBlock *RuntimeModule::getCodeBlockSlowPath(unsigned index) {
#ifndef HERMESVM_LEAN
  if (bcProvider_->isFunctionLazy(index)) {
    auto *lazyModule = RuntimeModule::createLazyModule(runtime_, this, index);
    functionMap_[index] = lazyModule->getOnlyLazyCodeBlock();
    return functionMap_[index];
  }
#endif
  functionMap_[index] = CodeBlock::createCodeBlock(
      this,
      bcProvider_->getFunctionHeader(index),
      bcProvider_->getBytecode(index),
      index);
  return functionMap_[index];
}

#ifndef HERMESVM_LEAN
RuntimeModule *RuntimeModule::createLazyModule(
    Runtime *runtime,
    RuntimeModule *parent,
    uint32_t functionID) {
  auto RM = createUninitialized(runtime);
  // Set the bcProvider's BytecodeModule to point to the parent's.
  assert(parent->isInitialized() && "Parent module must have been initialized");

  hbc::BytecodeFunction *bcFunction =
      &((hbc::BCProviderFromSrc *)parent->getBytecode())
           ->getBytecodeModule()
           ->getFunction(functionID);

  RM->bcProvider_ = hbc::BCProviderLazy::createBCProviderLazy(bcFunction);

  // Allow the lazy module to be deleted along with this module.
  parent->addDependency(RM);

  // We don't know which function index this block will eventually represent,
  // so just add it as 0 to ensure ownership. We'll move it later in
  // `initializeLazy`.
  RM->functionMap_.emplace_back(CodeBlock::createCodeBlock(
      RM, RM->bcProvider_->getFunctionHeader(functionID), {}, functionID));

  // The module doesn't have a string table until we've compiled the block,
  // so just add the string name as 0 in the mean time for f.name to work via
  // getLazyName(). Since it's in the stringIDMap_, it'll be correctly GC'd.
  RM->stringIDMap_.push_back(
      parent->getSymbolIDFromStringID(bcFunction->getHeader().functionName));

  return RM;
}

SymbolID RuntimeModule::getLazyName() {
  assert(functionMap_.size() == 1 && "Not a lazy module?");
  assert(stringIDMap_.size() == 1 && "Missing lazy function name symbol");
  assert(this->stringIDMap_[0].isValid() && "Invalid function name symbol");
  return this->stringIDMap_[0];
}

void RuntimeModule::addDependency(RuntimeModule *child) {
  dependentModules_.push_back(child);
  child->addUser();
}

void RuntimeModule::initializeLazy(std::unique_ptr<hbc::BCProvider> bytecode) {
  // Clear the old data provider first.
  bcProvider_ = nullptr;

  initialize(std::move(bytecode));

  // createLazyCodeBlock added a single codeblock as functionMap_[0]
  assert(functionMap_[0] && "Missing first entry");

  // We should move it to the index where it's supposed to be. This ensures a
  // 1-1 relationship between codeblocks and bytecodefunctions, which the
  // debugger relies on for setting step-out breakpoints in all functions.
  if (bcProvider_->getGlobalFunctionIndex() == 0) {
    // No move needed
    return;
  }
  assert(
      !functionMap_[bcProvider_->getGlobalFunctionIndex()] &&
      "Entry point is already occupied");
  functionMap_[bcProvider_->getGlobalFunctionIndex()] = functionMap_[0];
  functionMap_[0] = nullptr;
}
#endif

void RuntimeModule::importStringIDMap() {
  assert(bcProvider_ && "Uninitialized RuntimeModule");
  GCScope scope(runtime_);

  auto strTableSize = bcProvider_->getStringCount();

  stringIDMap_.clear();

  // Populate the string ID map with empty identifiers.
  stringIDMap_.resize(strTableSize, SymbolID::createEmpty());

  // Preallocate enough space to store all identifiers to prevent
  // unnecessary allocations.
  runtime_->getIdentifierTable().reserve(strTableSize);

  // Get the hashes of string entries marked as identifiers.
  auto identifierHashes = bcProvider_->getIdentifierHashes();
  assert(
      identifierHashes.size() <= strTableSize &&
      "Should not have more strings than identifiers");
  uint32_t identifierIndex = 0;

  for (StringID id = 0; id < strTableSize; ++id) {
    StringTableEntry entry = bcProvider_->getStringTableEntry(id);
    if (entry.isIdentifier()) {
      createSymbolFromStringID(id, entry, identifierHashes[identifierIndex++]);
    }
  }
  assert(
      identifierIndex == identifierHashes.size() &&
      "Hash count should match identifier count");
  if (strTableSize == 0) {
    // If the string table turns out to be empty,
    // we always add one empty string to it.
    // Note that this can only happen when we are creating the RuntimeModule
    // in a non-standard way, either in unit tests or the special
    // emptyCodeBlockRuntimeModule_ in Runtime where the creation happens
    // manually instead of going through bytecode module generation.
    // In those cases, functions will be created with a default nameID=0
    // without adding the name string into the string table. Hence here
    // we need to add it manually and it will have index 0.
    ASCIIRef s;
    stringIDMap_.push_back({});
    mapString(s, hashString(s));
  }
}

void RuntimeModule::initializeFunctionMap() {
  assert(bcProvider_ && "Uninitialized RuntimeModule");
  assert(
      bcProvider_->getFunctionCount() >= functionMap_.size() &&
      "Unexpected size reduction. Lazy module missing functions?");
  functionMap_.resize(bcProvider_->getFunctionCount());
}

void RuntimeModule::importCJSModuleTable() {
  assert(bcProvider_ && "Uninitialized RuntimeModule");
  for (const auto &pair : bcProvider_->getCJSModuleTable()) {
    auto index = cjsModules_.size();
    cjsModules_.emplace_back(CJSModule{pair.second});
    auto result =
        cjsModuleTable_.try_emplace(getSymbolIDFromStringID(pair.first), index);
    (void)result;
    assert(result.second && "Duplicate CJS modules");
  }
  for (const auto &functionID : bcProvider_->getCJSModuleTableStatic()) {
    cjsModules_.emplace_back(CJSModule{functionID});
  }
}

StringPrimitive *RuntimeModule::getStringPrimFromStringID(StringID stringID) {
  return runtime_->getStringPrimFromSymbolID(getSymbolIDFromStringID(stringID));
}

llvm::ArrayRef<uint8_t> RuntimeModule::getRegExpBytecodeFromRegExpID(
    uint32_t regExpId) const {
  assert(
      regExpId < bcProvider_->getRegExpTable().size() && "Invalid regexp id");
  RegExpTableEntry entry = bcProvider_->getRegExpTable()[regExpId];
  return bcProvider_->getRegExpStorage().slice(entry.offset, entry.length);
}

template <typename T>
SymbolID RuntimeModule::mapString(
    llvm::ArrayRef<T> str,
    StringID stringID,
    uint32_t hash) {
  // Create a SymbolID for a given string. In general a SymbolID holds onto an
  // intern'd StringPrimitive. As an optimization, if this RuntimeModule is
  // persistent, then it will not be deallocated before the Runtime, and we can
  // have the SymbolID hold a raw pointer into the storage and produce the
  // StringPrimitive when it is first required.
  SymbolID id;
  if (flags_.persistent) {
    // Registering a lazy identifier does not allocate, so we do not need a
    // GC scope.
    id = runtime_->getIdentifierTable().registerLazyIdentifier(str, hash);
  } else {
    // Accessing a symbol non-lazily may allocate in the GC heap, so add a scope
    // marker.
    GCScopeMarkerRAII scopeMarker{runtime_};
    id = *runtime_->ignoreAllocationFailure(
        runtime_->getIdentifierTable().getSymbolHandle(runtime_, str, hash));
  }
  stringIDMap_[stringID] = id;
  return id;
}

void RuntimeModule::markRoots(SlotAcceptor &acceptor, bool markLongLived) {
  for (auto &it : cjsModuleTable_) {
    acceptor.accept(it.first);
  }

  for (auto &it : cjsModules_) {
    acceptor.acceptPtr(it.module);
    acceptor.accept(it.cachedExports);
  }

  if (markLongLived) {
    for (auto symbol : stringIDMap_) {
      if (symbol.isValid()) {
        acceptor.accept(symbol);
      }
    }
  }
}

void RuntimeModule::markWeakRoots(SlotAcceptor &acceptor) {
  for (auto &cbPtr : functionMap_) {
    // Only mark a CodeBlock is its non-null, and has not been scanned
    // previously in this top-level markRoots invocation.
    if (cbPtr != nullptr && cbPtr->getRuntimeModule() == this) {
      cbPtr->markCachedHiddenClasses(acceptor);
    }
  }
  for (auto &entry : objectLiteralHiddenClasses_) {
    if (entry.second) {
      acceptor.accept(reinterpret_cast<void *&>(entry.second));
    }
  }
}

llvm::Optional<Handle<HiddenClass>> RuntimeModule::findCachedLiteralHiddenClass(
    unsigned keyBufferIndex,
    unsigned numLiterals) const {
  if (canGenerateLiteralHiddenClassCacheKey(keyBufferIndex, numLiterals)) {
    const auto cachedHiddenClassIter = objectLiteralHiddenClasses_.find(
        getLiteralHiddenClassCacheHashKey(keyBufferIndex, numLiterals));
    if (cachedHiddenClassIter != objectLiteralHiddenClasses_.end() &&
        cachedHiddenClassIter->second != nullptr) {
      return runtime_->makeHandle(cachedHiddenClassIter->second);
    }
  }
  return llvm::None;
}

void RuntimeModule::tryCacheLiteralHiddenClass(
    unsigned keyBufferIndex,
    HiddenClass *clazz) {
  auto numLiterals = clazz->getNumProperties();
  if (canGenerateLiteralHiddenClassCacheKey(keyBufferIndex, numLiterals)) {
    assert(
        !findCachedLiteralHiddenClass(keyBufferIndex, numLiterals).hasValue() &&
        "Why are we caching an item already cached?");
    objectLiteralHiddenClasses_[getLiteralHiddenClassCacheHashKey(
        keyBufferIndex, numLiterals)] = clazz;
  }
}

size_t RuntimeModule::additionalMemorySize() const {
  size_t total = stringIDMap_.capacity() * sizeof(SymbolID) +
      functionMap_.capacity() * sizeof(CodeBlock *) +
      dependentModules_.capacity() * sizeof(RuntimeModule *) +
      objectLiteralHiddenClasses_.getMemorySize() +
      cjsModuleTable_.getMemorySize() + cjsModules_.capacity_in_bytes();
  // Add the size of each CodeBlock
  for (const CodeBlock *cb : functionMap_) {
    // Skip the null code blocks, they are lazily inserted the first time
    // they are used.
    if (cb && cb->getRuntimeModule() == this) {
      // Only add the size of a CodeBlock if this runtime module is the owner.
      total += sizeof(CodeBlock) + cb->additionalMemorySize();
    }
  }
  return total;
}

namespace detail {

StringID mapString(RuntimeModule &module, const char *str) {
  module.stringIDMap_.push_back({});
  module.mapString(createASCIIRef(str), module.stringIDMap_.size() - 1);
  return module.stringIDMap_.size() - 1;
}

} // namespace detail

} // namespace vm

} // namespace hermes
