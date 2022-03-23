/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CODEBLOCK_H
#define HERMES_VM_CODEBLOCK_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Profiler.h"
#include "hermes/VM/PropertyCache.h"
#include "hermes/VM/SerializedLiteralParser.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/Optional.h"
#include "llvh/Support/TrailingObjects.h"

#include <memory>
#include <vector>

namespace hermes {
namespace vm {

class RuntimeModule;
class CodeBlock;

/// A pointer to JIT-compiled function.
typedef CallResult<HermesValue> (*JITCompiledFunctionPtr)(Runtime &runtime);

/// A sequence of instructions representing the body of a function.
class CodeBlock final
    : private llvh::TrailingObjects<CodeBlock, PropertyCacheEntry> {
  friend TrailingObjects;
  /// Points to the runtime module with the information required for this code
  /// block.
  RuntimeModule *const runtimeModule_;

  /// Pointer to the function header.
  hbc::RuntimeFunctionHeader functionHeader_;

  /// Pointer to the bytecode opcodes.
  const uint8_t *bytecode_;

  /// ID of this function in the module's function list.
  uint32_t functionID_;

  /// Total size of the property cache.
  const uint32_t propertyCacheSize_;

  /// Offset of the write property cache, which occurs after the read property
  /// cache.
  const uint32_t writePropCacheOffset_;

#ifndef HERMESVM_LEAN
  /// Compiles a lazy CodeBlock. Intended to be called from lazyCompile.
  void lazyCompileImpl(Runtime &runtime);
#endif

  /// Helper function for getting start and end locations.
  /// Given an SMLoc, returns the source coordinates of it in the lazy function.
  /// \param start if true, return the start coordinates, else end coordinates.
  SourceErrorManager::SourceCoords getLazyFunctionLoc(bool start) const;

  /// \return the base pointer of the property cache.
  PropertyCacheEntry *propertyCache() {
    return getTrailingObjects<PropertyCacheEntry>();
  }

  PropertyCacheEntry *writePropertyCache() {
    return getTrailingObjects<PropertyCacheEntry>() + writePropCacheOffset_;
  }

  CodeBlock(
      RuntimeModule *runtimeModule,
      hbc::RuntimeFunctionHeader header,
      const uint8_t *bytecode,
      uint32_t functionID,
      uint32_t cacheSize,
      uint32_t writePropCacheOffset)
      : runtimeModule_(runtimeModule),
        functionHeader_(header),
        bytecode_(bytecode),
        functionID_(functionID),
        propertyCacheSize_(cacheSize),
        writePropCacheOffset_(writePropCacheOffset) {
    std::uninitialized_fill_n(propertyCache(), cacheSize, PropertyCacheEntry{});
  }

 public:
#if defined(HERMESVM_PROFILER_JSFUNCTION) || defined(HERMESVM_PROFILER_EXTERN)
  /// ID written/read by JS function profiler on first/later function events.
  ProfilerID profilerID{NO_PROFILER_ID};
#endif

  /// Create a CodeBlock for a given runtime module \p runtimeModule. The result
  /// must be deallocated via delete, which is overridden.
  /// TODO: it would be nice to have this return a unique_ptr with a custom
  /// deleter; however lazy compilation requires that multiple RuntimeModules
  /// reference the same CodeBlock, so it is not yet possible.
  static CodeBlock *create(
      RuntimeModule *runtimeModule,
      hbc::RuntimeFunctionHeader header,
      const uint8_t *bytecode,
      uint32_t functionID,
      uint32_t cacheSize,
      uint32_t writePropCacheOffset) {
    auto allocSize = totalSizeToAlloc<PropertyCacheEntry>(cacheSize);
    void *mem = checkedMalloc(allocSize);
    return new (mem) CodeBlock(
        runtimeModule,
        header,
        bytecode,
        functionID,
        cacheSize,
        writePropCacheOffset);
  }

  /// Override of delete that balances the memory allocated in our create()
  /// function. Note the destructor has run already.
  static void operator delete(void *cb) {
    free(cb);
  }

  using const_iterator = const uint8_t *;

  uint32_t getParamCount() const {
    return functionHeader_.paramCount();
  }
  uint32_t getFrameSize() const {
    return functionHeader_.frameSize();
  }
  uint32_t getEnvironmentSize() const {
    return functionHeader_.environmentSize();
  }
  uint32_t getFunctionID() const {
    return functionID_;
  }

  /// Given the offset of the instruction where exception happened,
  /// \returns the offset of the exception handler to jump to.
  /// \returns -1 if a handler is not found.
  int32_t findCatchTargetOffset(uint32_t exceptionOffset);

  /// \return the offset of the function in a virtual bytecode stream, in which
  /// each function emits its bytecode in order. This is used for error
  /// backtraces when debug info is not present.
  uint32_t getVirtualOffset() const;

  SerializedLiteralParser getArrayBufferIter(
      uint32_t idx,
      unsigned int numLiterals) const;

  SerializedLiteralParser getObjectBufferKeyIter(
      uint32_t idx,
      unsigned int numLiterals) const;

  SerializedLiteralParser getObjectBufferValueIter(
      uint32_t idx,
      unsigned int numLiterals) const;

  RuntimeModule *getRuntimeModule() const {
    return runtimeModule_;
  }

  hbc::FunctionHeaderFlag getHeaderFlags() const {
    return functionHeader_.flags();
  }

  bool isStrictMode() const {
    return functionHeader_.flags().strictMode;
  }

  SymbolID getNameMayAllocate() const;

  /// \return The name of this code block, as a UTF-8 encoded string.
  /// Does no JS heap allocation.
  std::string getNameString(GCBase::GCCallbacks &runtime) const;

  const_iterator begin() const {
    return bytecode_;
  }
  const_iterator end() const {
    return bytecode_ + functionHeader_.bytecodeSizeInBytes();
  }
  llvh::ArrayRef<uint8_t> getOpcodeArray() const {
    return {bytecode_, functionHeader_.bytecodeSizeInBytes()};
  }

  /// \return true when \p inst is in this code block, false otherwise.
  bool contains(const inst::Inst *inst) const {
    return begin() <= reinterpret_cast<const uint8_t *>(inst) &&
        reinterpret_cast<const uint8_t *>(inst) < end();
  }

  OptValue<uint32_t> getDebugSourceLocationsOffset() const;

  /// \return the source location of the given instruction offset \p offset in
  /// the code block \p codeBlock.
  OptValue<hbc::DebugSourceLocation> getSourceLocation(
      uint32_t offset = 0) const;

  /// Look up the function source table and \return the String ID associated
  /// with the current function if an entry is found, or llvh::None if not.
  OptValue<uint32_t> getFunctionSourceID() const;

  OptValue<uint32_t> getDebugLexicalDataOffset() const;

  const inst::Inst *getOffsetPtr(uint32_t offset) const {
    assert(begin() + offset < end() && "offset out of bounds");
    return reinterpret_cast<const inst::Inst *>(begin() + offset);
  }

  uint32_t getOffsetOf(const inst::Inst *inst) const {
    assert(
        reinterpret_cast<const uint8_t *>(inst) >= begin() &&
        "inst not in this codeBlock");
    uint32_t offset = reinterpret_cast<const uint8_t *>(inst) - begin();
    assert(begin() + offset < end() && "inst not in this codeBlock");
    return offset;
  }

#ifndef HERMESVM_LEAN
  /// Checks whether this function is lazily compiled.
  bool isLazy() const {
    // null bytecode_ indicates that this is a lazy code block.
    return !bytecode_;
  }

  /// Compiles this CodeBlock, if it's lazy and not already compiled.
  void lazyCompile(Runtime &runtime) {
    if (LLVM_UNLIKELY(isLazy())) {
      lazyCompileImpl(runtime);
    }
  }
#else
  /// Checks whether this function is lazily compiled.
  bool isLazy() const {
    return false;
  }
  void lazyCompile(Runtime &) {}
#endif

  /// Get the start location of this function, if it's lazy.
  SourceErrorManager::SourceCoords getLazyFunctionStartLoc() const {
    return getLazyFunctionLoc(true);
  }

  /// Get the end location of this function, if it's lazy.
  SourceErrorManager::SourceCoords getLazyFunctionEndLoc() const {
    return getLazyFunctionLoc(false);
  }

  inline PropertyCacheEntry *getReadCacheEntry(uint8_t idx) {
    assert(idx < writePropCacheOffset_ && "idx out of ReadCache bound");
    return &propertyCache()[idx];
  }

  inline PropertyCacheEntry *getWriteCacheEntry(uint8_t idx) {
    assert(
        writePropCacheOffset_ + idx < propertyCacheSize_ &&
        "idx out of WriteCache bound");
    return &propertyCache()[writePropCacheOffset_ + idx];
  }

  // Mark all hidden classes in the property cache as roots.
  void markCachedHiddenClasses(Runtime &runtime, WeakRootAcceptor &acceptor);

  static CodeBlock *createCodeBlock(
      RuntimeModule *runtimeModule,
      hbc::RuntimeFunctionHeader header,
      const uint8_t *bytecode,
      uint32_t functionID);

  /// \return an estimate of the size of additional memory used by this
  /// CodeBlock.
  size_t additionalMemorySize() const {
    return propertyCacheSize_ * sizeof(PropertyCacheEntry);
  }

#ifdef HERMES_ENABLE_DEBUGGER
  inst::OpCode getOpCode(uint32_t offset) const {
    auto opcodes = getOpcodeArray();
    assert(offset < opcodes.size() && "opCode offset out of bounds");
    const auto *inst = reinterpret_cast<const inst::Inst *>(&opcodes[offset]);
    return inst->opCode;
  }

  /// Installs in the debugger instruction into the opcode stream
  /// at location \p offset.
  /// Requires that there's a breakpoint registered at \p offset.
  /// Increments the user count of the associated runtime module.
  void installBreakpointAtOffset(uint32_t offset);

  /// Uninstalls the debugger instruction from the opcode stream
  /// at location \p offset, replacing it with \p opCode.
  /// Requires the opcodes at \p offset is DebuggerInst.
  /// Requires that a breakpoint has been set at \p offset.
  /// Decrements the user count of the associated runtime module.
  void uninstallBreakpointAtOffset(uint32_t offset, uint8_t opCode);

  /// \return the offset of the next instruction after the one at \p offset.
  uint32_t getNextOffset(uint32_t offset) const;
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CODEBLOCK_H
