/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CODEBLOCK_H
#define HERMES_VM_CODEBLOCK_H

#include "JIT/Config.h"
#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Profiler.h"
#include "hermes/VM/PropertyCache.h"
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
typedef HermesValue (*JITCompiledFunctionPtr)(Runtime *runtime);

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

#ifndef HERMESVM_JIT
#error "JIT config not included"
#endif
#if HERMESVM_JIT
  /// Set to true if for some reason we don't want to JIT this block, for
  /// example because it contains constructs that the JIT can't handle.
  bool dontJIT_ = false;

  /// If this CodeBlock was compiled, a pointer to the body.
  JITCompiledFunctionPtr JITCompiled_ = nullptr;

  /// Function execution count.
  /// Ideally, a function's hotness should also include if it has a loop and how
  /// hot that loop is.
  uint32_t executionCount_ = 0;
#endif

#ifdef HERMES_ENABLE_DEBUGGER
  /// The number of breakpoints currently installed in this function.
  uint32_t numInstalledBreakpoints_ = 0;
#endif

  /// Total size of the property cache.
  const uint32_t propertyCacheSize_;

  /// Offset of the write property cache, which occurs after the read property
  /// cache.
  const uint32_t writePropCacheOffset_;

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
#if defined(HERMESVM_PROFILER_JSFUNCTION)
  /// ID written/read by JS function profiler on first/later function events.
  ProfilerID profilerID{NO_PROFILER_ID};
#endif

  /// Override of delete that balances the memory allocated in our create()
  /// function. Note the destructor has run already.
  static void operator delete(void *cb) {
    free(cb);
  }

  using const_iterator = const uint8_t *;

  /// \return the base pointer of the property cache.
  PropertyCacheEntry *propertyCache() {
    return getTrailingObjects<PropertyCacheEntry>();
  }

  PropertyCacheEntry *writePropertyCache() {
    return getTrailingObjects<PropertyCacheEntry>() + writePropCacheOffset_;
  }

  uint32_t getParamCount() const {
    return functionHeader_.paramCount();
  }
  uint32_t getFrameSize() const {
    return functionHeader_.frameSize();
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

  RuntimeModule *getRuntimeModule() const {
    return runtimeModule_;
  }

  const hbc::RuntimeFunctionHeader &getFunctionHeader() const {
    return functionHeader_;
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
  std::string getNameString() const;

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
  OptValue<hbc::DebugSourceLocation> getSourceLocation(uint32_t offset) const;

  /// \return the source location of the function.
  OptValue<hbc::DebugSourceLocation> getSourceLocationForFunction() const;

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

  /// Checks whether this function is lazily compiled.
#ifndef HERMESVM_LEAN
  bool isLazy() const {
    return !bytecode_;
  }
  ExecutionStatus lazyCompile(Runtime &runtime) {
    if (LLVM_LIKELY(!isLazy()))
      return ExecutionStatus::RETURNED;
    return lazyCompileImpl(runtime);
  }
  ExecutionStatus lazyCompileImpl(Runtime &runtime);

  /// \pre isLazy() is true.
  /// \return whether the coordinates are in the lazy function.
  bool coordsInLazyFunction(SMLoc loc) const;

  /// \return a vector representing the number of Variables for each depth
  ///   of the VariableScope chain.
  std::vector<uint32_t> getVariableCounts() const;

  /// \param depth the depth of the VariableScope to lookup, 0 is the
  ///   the current CodeBlock.
  /// \param variableIndex the index of the Variable in the VariableScope.
  /// \return the name of the Variable at a given index at the given depth.
  llvh::StringRef getVariableNameAtDepth(uint32_t depth, uint32_t variableIndex)
      const;
#else
  bool isLazy() const {
    return false;
  }
  ExecutionStatus lazyCompile(Runtime &runtime) {
    return ExecutionStatus::RETURNED;
  }
  std::vector<uint32_t> getVariableCounts() const {
    hermes_fatal("unavailable in lean VM");
  }

  llvh::StringRef getVariableNameAtDepth(uint32_t, uint32_t) const {
    hermes_fatal("unavailable in lean VM");
  }
#endif

#if HERMESVM_JIT
  /// \return true if JIT is disabled for this function.
  bool getDontJIT() const {
    return dontJIT_;
  }

  /// Enable or disable JIT compilation of this function.
  void setDontJIT(bool dontJIT) {
    dontJIT_ = dontJIT;
  }

  /// \return the native code for this function, or null if it hasn't been
  ///   compiled to native.
  JITCompiledFunctionPtr getJITCompiled() const {
    return JITCompiled_;
  }

  /// Set the native code for this function.
  void setJITCompiled(JITCompiledFunctionPtr JITCompiled) {
    JITCompiled_ = JITCompiled;
  }

  /// Increment the function execution count.
  void incrementExecutionCount() {
    executionCount_++;
  }

  /// \return the function execution count
  uint32_t getExecutionCount() const {
    return executionCount_;
  }

  /// Reset the function execution count to 0
  void clearExecutionCount() {
    executionCount_ = 0;
  }
#else
  /// \return true if JIT is disabled for this function.
  bool getDontJIT() const {
    return true;
  }

  /// Enable or disable JIT compilation of this function.
  void setDontJIT(bool dontJIT) {}

  /// \return the native code for this function, or null if it hasn't been
  ///   compiled to native.
  JITCompiledFunctionPtr getJITCompiled() const {
    return nullptr;
  }

  /// Set the native code for this function.
  void setJITCompiled(JITCompiledFunctionPtr JITCompiled) {}

  /// Increment the function executionCount_ count
  void incrementExecutionCount() {}

  /// \return the function executionCount_ count as 0 if the JIT is not enabled
  uint32_t getExecutionCount() const {
    return 0;
  }

  /// Reset the function executionCount_ count to 0
  void clearExecutionCount() {}
#endif

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

  /// Create a CodeBlock for a given runtime module \p runtimeModule.
  /// The result must be deallocated via the overridden delete operator,
  /// which is why we use unique_ptr.
  /// \param runtimeModule the RuntimeModule that it will belong to.
  /// \param header the header of the function.
  /// \param bytecode the bytecode of the function.
  /// \param functionID the ID of the function in the bytecode.
  /// \return a unique pointer to the CodeBlock.
  static std::unique_ptr<CodeBlock> createCodeBlock(
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

  /// \return the number of breakpoints installed in this code block.
  uint32_t getNumInstalledBreakpoints() const {
    return numInstalledBreakpoints_;
  }
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CODEBLOCK_H
