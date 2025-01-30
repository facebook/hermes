/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H
#define HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H

#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/FileAndSourceMapIdCache.h"
#include "hermes/BCGen/HBC/HBC.h"

#include "llvh/ADT/Optional.h"

namespace hermes {
namespace hbc {

#ifndef HERMESVM_LEAN
/// BCProviderFromSrc is used when we are construction the bytecode from
/// source compilation, i.e. we generate BytecodeModule/BytecodeFunction
/// in this code path, and all the data are stored in those classes.
/// We should only depend on this when running from eval code path to
/// make sure maximum efficiency when loading from bytecode file.
class BCProviderFromSrc final : public BCProviderBase {
 public:
  /// The data needed to rerun on the compiler on more code that uses variables
  /// in this BCProvider.
  /// Copying this will increment the use count of shared pointers.
  class CompilationData {
   public:
    /// The options used to generate the BytecodeModule.
    BytecodeGenerationOptions genOptions;

    /// IR Module used for compiling more code into this BytecodeModule.
    /// May be shared with other BCProviders during local eval,
    /// which reuses SemContext and the IR but makes a new BCProvider.
    std::shared_ptr<Module> M;

    /// SemContext used for compiling more code into this BytecodeModule.
    /// May be shared with other BCProviders during local eval,
    /// which reuses SemContext and the IR but makes a new BCProvider.
    std::shared_ptr<sema::SemContext> semCtx;

    /// The file and source map ID cache used for compiling more code into this
    /// BytecodeModule.
    /// Keep alive between lazy compilation calls to avoid expensive lookups
    /// for huge data URLs in sourceMappingURL.
    FileAndSourceMapIdCache fileAndSourceMapIdCache;

    explicit CompilationData(
        const BytecodeGenerationOptions &genOptions,
        const std::shared_ptr<Module> &M,
        const std::shared_ptr<sema::SemContext> &semCtx)
        : genOptions(genOptions), M(M), semCtx(semCtx) {}
  };

 private:
  /// Data needed for compiling more code that uses the same
  /// variables/information.
  CompilationData compilationData_;

  /// The BytecodeModule that provides the bytecode data.
  /// Placed below CompilationData to ensure its destruction before the
  /// Module gets deleted.
  std::unique_ptr<hbc::BytecodeModule> module_;

  /// Hash of all source files.
  SHA1 sourceHash_{SHA1{}};

  explicit BCProviderFromSrc(
      std::unique_ptr<hbc::BytecodeModule> &&module,
      CompilationData &&compilationData);

  /// No need to do anything since it's already created as part of
  /// BytecodeModule and set to the local member.
  void createDebugInfo() override {}

 public:
  /// Creates a BCProviderFromSrc by compiling the given JavaScript and
  /// optionally optimizing it with the supplied callback.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param topLevelFunctionName the name of the global function
  ///   "eval" for eval, "global" for ordinary scripts.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param sourceMap optional input source map for \p buffer.
  /// \param compileFlags self explanatory
  /// \param diagHandler handler for errors/warnings/notes.
  /// \param diagContext opaque data that will be passed to diagHandler.
  /// \param runOptimizationPasses if optimization is enabled in the settings
  ///     and this is non-null, invoke this callback with the IR module to
  ///     perform optimizations. This allows us to defer the decision of
  ///     whether to link all optimizations to the caller.
  /// \param defaultBytecodeGenerationOptions the starting bytecode generation
  ///     options that will be used during the bytecode generation phase.
  ///     Some options will be overriden depending on other arguments passed in.
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message (if diagHandler was provided, the error message is "error").
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string> create(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      std::unique_ptr<SourceMap> sourceMap,
      const CompileFlags &compileFlags,
      llvh::StringRef topLevelFunctionName = "global",
      SourceErrorManager::DiagHandlerTy diagHandler = {},
      void *diagContext = nullptr,
      const std::function<void(Module &)> &runOptimizationPasses = {},
      const BytecodeGenerationOptions &defaultBytecodeGenerationOptions =
          BytecodeGenerationOptions::defaults());

  /// Wrap an existing BytecodeModule in a BCProviderFromSrc.
  static std::unique_ptr<BCProviderFromSrc> createFromBytecodeModule(
      std::unique_ptr<hbc::BytecodeModule> bcModule,
      CompilationData &&compilationData) {
    auto result = std::unique_ptr<BCProviderFromSrc>(
        new BCProviderFromSrc(std::move(bcModule), std::move(compilationData)));
    result->module_->setBCProviderFromSrc(result.get());
    return result;
  }

  /// Set any references in the BCProvider based on the underlying data.
  /// This must be called after construction or after any underlying data
  /// (string tables, regexp tables, etc.) changes during lazy compilation.
  void setBytecodeModuleRefs();

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

  /// \return whether the provider can be loaded as persistent,
  /// which is not possible if the underlying storage may be mutated,
  /// e.g. in the case of lazy compilation.
  bool allowPersistent() const override {
    // Persistent RuntimeModules are only supported when there is no lazy
    // compilation that could create new functions.
    return !compilationData_.M ||
        !compilationData_.M->getContext().isLazyCompilation();
  }

  hbc::BytecodeModule *getBytecodeModule() override {
    return module_.get();
  }

  const BytecodeGenerationOptions &getBytecodeGenerationOptions() const {
    return compilationData_.genOptions;
  }

  Module *getModule() {
    return compilationData_.M.get();
  }

  /// \return the shared_ptr for the Module so it can be copied.
  const std::shared_ptr<Module> &shareModule() const {
    return compilationData_.M;
  }

  sema::SemContext *getSemCtx() {
    return compilationData_.semCtx.get();
  }

  /// \return the shared_ptr for the SemContext so it can be copied.
  const std::shared_ptr<sema::SemContext> &shareSemCtx() const {
    return compilationData_.semCtx;
  }

  /// \return the FileAndSourceMapIdCache for debug IDs.
  FileAndSourceMapIdCache &getFileAndSourceMapIdCache() {
    return compilationData_.fileAndSourceMapIdCache;
  }

  SHA1 getSourceHash() const override {
    return sourceHash_;
  };

  void setSourceHash(const SHA1 &hash) {
    sourceHash_ = hash;
  };

  static bool classof(const BCProviderBase *provider) {
    return provider->getKind() == BCProviderKind::BCProviderFromSrc;
  }
};

#endif // HERMESVM_LEAN
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H
