/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H
#define HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"

#include "llvh/ADT/Optional.h"

namespace hermes {
namespace hbc {

/// Flags passed to createBCProviderFromSrc to set parameters on execution.
struct CompileFlags {
  bool optimize{false};
  bool debug{false};
  bool lazy{false};
  bool allowFunctionToStringWithRuntimeSource{false};
  bool strict{false};
  /// The value is optional; when it is set, the optimization setting is based
  /// on the value; when it is unset, it means the parser needs to automatically
  /// detect the 'use static builtin' directive and set the optimization setting
  /// accordingly.
  llvh::Optional<bool> staticBuiltins;
  bool verifyIR{false};
  /// If set, the compiler emits async break check instructions.  These may be
  /// used for several purposes, for example, to enforce a time limit on
  /// execution.  Other flags may also cause these instructions to be emitted,
  /// for example debugging.
  bool emitAsyncBreakCheck{false};
  /// Include libhermes declarations when compiling the file. This is done in
  /// normal compilation, but not for eval().
  bool includeLibHermes{true};
  /// If set, instrument the IR for dynamic checks.
  bool instrumentIR{false};
};

// The minimum code size in bytes before enabling lazy compilation.
// Lazy compilation has significant per-module overhead, and is best applied
// to large bundles with a lot of unused code. Eager compilation is more
// efficient when compiling many small bundles with little unused code, such as
// when the API user loads smaller chunks of JS code on demand.
static constexpr unsigned kDefaultSizeThresholdForLazyCompilation = 1 << 16;

#ifndef HERMESVM_LEAN
/// BCProviderFromSrc is used when we are construction the bytecode from
/// source compilation, i.e. we generate BytecodeModule/BytecodeFunction
/// in this code path, and all the data are stored in those classes.
/// We should only depend on this when running from eval code path to
/// make sure maximum efficiency when loading from bytecode file.
class BCProviderFromSrc final : public BCProviderBase {
  /// The BytecodeModule that provides the bytecode data.
  std::unique_ptr<hbc::BytecodeModule> module_;

  /// Whether the module constitutes a single function
  bool singleFunction_;

  explicit BCProviderFromSrc(std::unique_ptr<hbc::BytecodeModule> module);

  /// No need to do anything since it's already created as part of
  /// BytecodeModule and set to the local member.
  void createDebugInfo() override {}

 public:
  static std::unique_ptr<BCProviderFromSrc> createBCProviderFromSrc(
      std::unique_ptr<hbc::BytecodeModule> module) {
    return std::unique_ptr<BCProviderFromSrc>(
        new BCProviderFromSrc(std::move(module)));
  }

  /// Creates a BCProviderFromSrc by compiling the given JavaScript.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param compileFlags self explanatory
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message.
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      const CompileFlags &compileFlags);

  /// Creates a BCProviderFromSrc by compiling the given JavaScript.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param sourceMap optional input source map for \p buffer.
  /// \param compileFlags self explanatory
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message.
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      std::unique_ptr<SourceMap> sourceMap,
      const CompileFlags &compileFlags);

  /// Creates a BCProviderFromSrc by compiling the given JavaScript.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param sourceMap optional input source map for \p buffer.
  /// \param compileFlags self explanatory
  /// \param scopeChain a scope chain for local variable resolution
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message.
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      std::unique_ptr<SourceMap> sourceMap,
      const CompileFlags &compileFlags,
      const ScopeChain &scopeChain);

  /// Creates a BCProviderFromSrc by compiling the given JavaScript and
  /// optionally optimizing it with the supplied callback.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param sourceMap optional input source map for \p buffer.
  /// \param compileFlags self explanatory
  /// \param scopeChain a scope chain for local variable resolution
  /// \param runOptimizationPasses if optimization is enabled in the settings
  ///     and this is non-null, invoke this callback with the IR module to
  ///     perform optimizations. This allows us to defer the decision of
  ///     whether to link all optimizations to the caller.
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message.
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      std::unique_ptr<SourceMap> sourceMap,
      const CompileFlags &compileFlags,
      const ScopeChain &scopeChain,
      const std::function<void(Module &)> &runOptimizationPasses);

  RuntimeFunctionHeader getFunctionHeader(uint32_t functionID) const override {
    return RuntimeFunctionHeader(&module_->getFunction(functionID).getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const override {
    assert(index < stringCount_ && "invalid string table index");
    return module_->getStringTable()[index];
  }

  const uint8_t *getBytecode(uint32_t functionID) const override {
    return module_->getFunction(functionID).getOpcodeArray().data();
  }

  llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t functionID) const override {
    return module_->getFunction(functionID).getExceptionHandlers();
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t functionID) const override {
    return module_->getFunction(functionID).getDebugOffsets();
  }

  bool isFunctionLazy(uint32_t functionID) const override {
    return module_->getFunction(functionID).isLazy();
  }

  bool isLazy() const override {
    return false;
  }

  bool isSingleFunction() const {
    return singleFunction_;
  }

  hbc::BytecodeModule *getBytecodeModule() {
    return module_.get();
  }

#ifdef HERMESVM_SERIALIZE
  /// Serialize this BCProviderFromSrc.
  void serialize(vm::Serializer &s) const override;
#endif

#ifndef HERMESVM_LEAN
  llvh::SMRange getFunctionSourceRange(uint32_t functionID) const override {
    return module_->getFunctionSourceRange(functionID);
  }
#endif
};

/// BCProviderLazy is used during lazy compilation. When a function is created
/// to be lazily compiled later, we create a BCProviderLazy object with
/// a pointer to such BytecodeFunction.
class BCProviderLazy final : public BCProviderBase {
  hbc::BytecodeModule *bytecodeModule_;

  /// Pointer to the BytecodeFunction.
  hbc::BytecodeFunction *bytecodeFunction_;

  explicit BCProviderLazy(
      hbc::BytecodeModule *bytecodeModule,
      hbc::BytecodeFunction *bytecodeFunction);

  /// No debug information will be available without compiling it.
  void createDebugInfo() override {
    llvm_unreachable("Accessing debug info from a lazy module");
  }

 public:
  static std::unique_ptr<BCProviderBase> createBCProviderLazy(
      hbc::BytecodeModule *bytecodeModule,
      hbc::BytecodeFunction *bytecodeFunction) {
    return std::unique_ptr<BCProviderBase>(
        new BCProviderLazy(bytecodeModule, bytecodeFunction));
  }

  RuntimeFunctionHeader getFunctionHeader(uint32_t) const override {
    return RuntimeFunctionHeader(&bytecodeFunction_->getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const override {
    llvm_unreachable("Accessing string table from a lazy module");
  }

  const uint8_t *getBytecode(uint32_t) const override {
    llvm_unreachable("Accessing bytecode from a lazy module");
  }

  llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t) const override {
    llvm_unreachable("Accessing exception info from a lazy module");
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t) const override {
    llvm_unreachable("Accessing debug offsets from a lazy module");
  }

  bool isFunctionLazy(uint32_t) const override {
    return true;
  }

  bool isLazy() const override {
    return true;
  }

  /// \return the pointer to the BytecodeFunction.
  hbc::BytecodeFunction *getBytecodeFunction() {
    return bytecodeFunction_;
  }

  hbc::BytecodeModule *getBytecodeModule() {
    return bytecodeModule_;
  }

#ifdef HERMESVM_SERIALIZE
  /// Serialize this BCProviderLazy.
  void serialize(vm::Serializer &s) const override;
#endif

  llvh::SMRange getFunctionSourceRange(uint32_t functionID) const override {
    return bytecodeModule_->getFunctionSourceRange(functionID);
  }
};
#endif // HERMESVM_LEAN
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H
