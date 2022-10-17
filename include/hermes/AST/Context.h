/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_CONTEXT_H
#define HERMES_AST_CONTEXT_H

#include "hermes/Parser/PreParser.h"
#include "hermes/Support/Allocator.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/StringTable.h"

#include "llvh/ADT/StringRef.h"

namespace hermes {

namespace hbc {
class BackendContext;
}

#ifdef HERMES_RUN_WASM
class EmitWasmIntrinsicsContext;
#endif // HERMES_RUN_WASM

struct CodeGenerationSettings {
  /// Whether we should emit TDZ checks.
  bool enableTDZ{false};
  /// Whether we can assume there are unlimited number of registers.
  /// This affects how we generate the IR, as we can decide whether
  /// to hold as many temporary values as we like.
  bool unlimitedRegisters{true};
  /// Dump registers assigned to instruction operands.
  bool dumpOperandRegisters{false};
  /// Print source location information in IR dumps.
  bool dumpSourceLocation{false};
  /// Print the original scope for each instruction.
  bool dumpSourceLevelScope{false};
  /// Print the use list if the instruction has any users.
  bool dumpUseList{false};
  /// Dump IR after every pass.
  bool dumpIRBetweenPasses{false};
  /// Instrument IR for dynamic checking (if support is compiled in).
  bool instrumentIR{false};
};

struct OptimizationSettings {
  /// Enable aggressive non-strict mode optimizations. These optimizations
  /// assume that:
  ///   - function arguments are never modified indirectly
  ///   - local "eval()" or "with" are not used.
  bool aggressiveNonStrictModeOptimizations{true};

  /// Enable any inlining of functions.
  bool inlining{true};

  /// Reuse property cache entries for same property name.
  bool reusePropCache{true};

  /// Recognize calls to global functions like Object.keys() and turn them
  /// into builtin calls.
  bool staticBuiltins{false};

  /// Attempt to resolve CommonJS require() calls at compile time.
  bool staticRequire{false};

  /// Recognize and emit Asm.js/Wasm unsafe compiler intrinsics.
  bool useUnsafeIntrinsics{false};
};

enum class DebugInfoSetting {
  /// Only emit source locations for instructions that may throw, as required
  /// for generating error stack traces.
  THROWING,

  /// Emit source locations for all instructions, as required for generating
  /// a source map.
  SOURCE_MAP,

  /// Emit full debug info, including source locations for all instructions,
  /// lexical scope info, async break check instructions, etc.
  ALL,
};

enum class ParseFlowSetting {
  /// Do not parse any Flow type syntax.
  NONE,

  /// Parse all Flow type syntax.
  ALL,

  /// Parse all unambiguous Flow type syntax. Syntax that can be intepreted as
  /// either Flow types or standard JavaScript is parsed as if it were standard
  /// JavaScript.
  ///
  /// For example, `foo<T>(x)` is parsed as if it were standard JavaScript
  /// containing two comparisons, even though it could otherwise be interpreted
  /// as a call expression with Flow type arguments.
  UNAMBIGUOUS,
};

/// An enum to track the "source visibility" of functions. This notion is coined
/// to implement "directives" such as 'hide source' and 'sensitive' defined by
/// https://github.com/tc39/proposal-function-implementation-hiding, as well as
/// 'show source' Hermes proposed to explicitly preserve source for `toString`.
///
/// Members are ordered in an increasingly stronger manner, where only later
/// source visibility can override the earlier but not vice versa.
enum class SourceVisibility {
  /// The implementation-default behavior, e.g. `toString` prints
  /// `{ [bytecode] }` in Hermes.
  Default,

  /// Enforce the source code text to be available for the `toString` use.
  ShowSource,

  /// Enforce to have the syntax of NativeFunction, e.g. `toString` prints
  /// `{ [native code] }`.
  HideSource,

  /// Considered security-sensitive, e.g. `toString` printed as NativeFunction;
  /// hidden from error stack trace to protect from leaking its existence.
  Sensitive,
};

/// Holds shared dependencies and state.
class Context {
 public:
  using Allocator = hermes::BumpPtrAllocator;

  /// Mapping from require() arguments to actual resolved paths.
  /// Strings owned by a JSON Parser allocator that the user of Context should
  /// manage.
  using ResolutionTableEntry = llvh::DenseMap<llvh::StringRef, llvh::StringRef>;

  /// Mapping from file names to the dictionary to use to look up resolutions.
  /// Strings owned by a JSON Parser allocator that the user of Context should
  /// manage.
  using ResolutionTable = llvh::DenseMap<llvh::StringRef, ResolutionTableEntry>;

 private:
  /// The allocator for AST nodes, which may be rolled back to parse subtrees
  /// during pre-parsing (for lazy parsing).
  Allocator allocator_{};

  /// String/identifier table allocator. It's separate from the AST allocator
  /// because we don't want to revert the strings when we revert subtrees.
  Allocator identifierAllocator_{};

  /// Preparsed function spans and similar used during lazy parsing.
  std::unique_ptr<parser::PreParsedData> preParsed_{};

  /// The global string table.
  StringTable stringTable_{identifierAllocator_};

  std::map<std::pair<UniqueString *, UniqueString *>, CompiledRegExp>
      compiledRegExps_{};

  /// If an external SourceErrorManager was not supplied to us, we allocate out
  /// private one here.
  std::unique_ptr<SourceErrorManager> ownSm_;

  /// A reference to the manager which we are using.
  SourceErrorManager &sm_;

  /// Whether we are running in script mode. Default to strict.
  bool strictMode_{false};

  /// Is 'eval()' is enabled.
  bool enableEval_{true};

  /// If true, every function will be compiled lazily when invoked for the
  /// first time.
  bool lazyCompilation_{false};

  /// Even if lazily compiling, eagerly compile any functions under this size in
  /// bytes.
  unsigned preemptiveFunctionCompilationThreshold_{0};

  /// Even if lazily compiling, eagerly compile any files under this size in
  /// bytes.
  unsigned preemptiveFileCompilationThreshold_{0};

  /// If true, do not error on return statements that are not within functions.
  bool allowReturnOutsideFunction_{false};

  /// Allows generator functions to be compiled.
  bool generatorEnabled_{true};

  /// If true, wrap each file in the CommonJS module wrapper function,
  /// and use that for requiring modules.
  bool useCJSModules_{false};

  /// If true, allow parsing JSX as a primary expression.
  bool parseJSX_{false};

  /// Whether to parse Flow type syntax.
  ParseFlowSetting parseFlow_{ParseFlowSetting::NONE};

  /// Whether to parse TypeScript syntax.
  bool parseTS_{false};

  /// If non-null, the resolution table which resolves static require().
  const std::unique_ptr<ResolutionTable> resolutionTable_;

  /// The list of segment IDs, based on the user's metadata input to the
  /// compiler when splitting the bundle.
  const std::vector<uint32_t> segments_;

  /// The level of debug information we should emit. Defaults to
  /// DebugInfoSetting::THROWING.
  DebugInfoSetting debugInfoSetting_{DebugInfoSetting::THROWING};

  /// Whether to emit async break check instruction or not.
  bool emitAsyncBreakCheck_{false};

  CodeGenerationSettings codeGenerationSettings_;

  OptimizationSettings optimizationSettings_;

  /// The HBC backend context. We use a shared pointer to avoid any dependencies
  /// on its destructor.
  std::shared_ptr<hbc::BackendContext> hbcBackendContext_{};

#ifdef HERMES_RUN_WASM
  std::shared_ptr<EmitWasmIntrinsicsContext> wasmIntrinsicsContext_{};
#endif // HERMES_RUN_WASM

 public:
  explicit Context(
      SourceErrorManager &sm,
      CodeGenerationSettings codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<uint32_t> segments = {})
      : sm_(sm),
        resolutionTable_(std::move(resolutionTable)),
        segments_(std::move(segments)),
        codeGenerationSettings_(std::move(codeGenOpts)),
        optimizationSettings_(std::move(optimizationOpts)) {}

  explicit Context(
      CodeGenerationSettings codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<uint32_t> segments = {})
      : ownSm_(new SourceErrorManager()),
        sm_(*ownSm_),
        resolutionTable_(std::move(resolutionTable)),
        segments_(std::move(segments)),
        codeGenerationSettings_(std::move(codeGenOpts)),
        optimizationSettings_(std::move(optimizationOpts)) {}

  Context(const Context &) = delete;
  void operator=(const Context &) = delete;

  Allocator &getAllocator() {
    return allocator_;
  }

  StringTable &getStringTable() {
    return stringTable_;
  }

  void addCompiledRegExp(
      UniqueString *pattern,
      UniqueString *flags,
      CompiledRegExp &&compiled) {
    compiledRegExps_.emplace(
        std::make_pair(pattern, flags), std::move(compiled));
  }

  CompiledRegExp &getCompiledRegExp(
      UniqueString *pattern,
      UniqueString *flags) {
    auto it = compiledRegExps_.find(std::make_pair(pattern, flags));
    assert(it != compiledRegExps_.end() && "Regex hasn't been compiled");
    return it->second;
  }

  parser::PreParsedBufferInfo *getPreParsedBufferInfo(uint32_t bufferId) {
    if (!preParsed_)
      preParsed_ = std::make_unique<parser::PreParsedData>();
    return preParsed_->getBufferInfo(bufferId);
  }

  SourceErrorManager &getSourceErrorManager() {
    return sm_;
  }

  const SourceErrorManager &getSourceErrorManager() const {
    return sm_;
  }

  /// \return the table for static require resolution, nullptr if not supplied.
  const std::vector<uint32_t> &getSegments() const {
    return segments_;
  }

  /// \return the table for static require resolution, nullptr if not supplied.
  const ResolutionTable *getResolutionTable() const {
    return resolutionTable_.get();
  }

  /// Get or create a new identifier for the string \p str. The method copies
  /// the content of the string.
  Identifier getIdentifier(llvh::StringRef str) {
    return stringTable_.getIdentifier(str);
  }

  /// Return the textual representation of the identifier.
  llvh::StringRef toString(Identifier iden) {
    return iden.str();
  }

  void setStrictMode(bool strictMode) {
    strictMode_ = strictMode;
  }
  bool isStrictMode() const {
    return strictMode_;
  }

  bool getEnableEval() const {
    return enableEval_;
  }
  void setEnableEval(bool enableEval) {
    enableEval_ = enableEval;
  }

  void setDebugInfoSetting(DebugInfoSetting debugInfoSetting) {
    debugInfoSetting_ = debugInfoSetting;
  }
  DebugInfoSetting getDebugInfoSetting() const {
    return debugInfoSetting_;
  }

  void setEmitAsyncBreakCheck(bool check) {
    emitAsyncBreakCheck_ = check;
  }
  bool getEmitAsyncBreakCheck() const {
    return emitAsyncBreakCheck_;
  }

  void setUseCJSModules(bool useCJSModules) {
    useCJSModules_ = useCJSModules;
  }
  bool getUseCJSModules() const {
    return useCJSModules_;
  }

  void setParseJSX(bool parseJSX) {
    parseJSX_ = parseJSX;
  }
  bool getParseJSX() const {
    return parseJSX_;
  }

  void setParseFlow(ParseFlowSetting parseFlow) {
    parseFlow_ = parseFlow;
  }
  bool getParseFlow() const {
    return parseFlow_ != ParseFlowSetting::NONE;
  }
  bool getParseFlowAmbiguous() const {
    return parseFlow_ == ParseFlowSetting::ALL;
  }

  void setParseTS(bool parseTS) {
    parseTS_ = parseTS;
  }
  bool getParseTS() const {
    return parseTS_;
  }

  /// \return true if either TS or Flow is being parsed.
  bool getParseTypes() const {
    return getParseFlow() || getParseTS();
  }

  bool isLazyCompilation() const {
    return lazyCompilation_;
  }

  void setLazyCompilation(bool lazyCompilation) {
    lazyCompilation_ = lazyCompilation;
  }

  unsigned getPreemptiveFunctionCompilationThreshold() {
    return preemptiveFunctionCompilationThreshold_;
  }

  void setPreemptiveFunctionCompilationThreshold(unsigned byteCount) {
    preemptiveFunctionCompilationThreshold_ = byteCount;
  };

  unsigned getPreemptiveFileCompilationThreshold() {
    return preemptiveFileCompilationThreshold_;
  }

  void setPreemptiveFileCompilationThreshold(unsigned byteCount) {
    preemptiveFileCompilationThreshold_ = byteCount;
  };

  bool allowReturnOutsideFunction() const {
    return allowReturnOutsideFunction_;
  }

  void setAllowReturnOutsideFunction(bool allowReturnOutsideFunction) {
    allowReturnOutsideFunction_ = allowReturnOutsideFunction;
  }

  bool isGeneratorEnabled() const {
    return generatorEnabled_;
  }

  void setGeneratorEnabled(bool v) {
    generatorEnabled_ = v;
  }

  void setStaticBuiltinOptimization(bool staticBuiltins) {
    optimizationSettings_.staticBuiltins = staticBuiltins;
  }

  bool getStaticBuiltinOptimization() const {
    return optimizationSettings_.staticBuiltins;
  }

  bool getUseUnsafeIntrinsics() const {
    return optimizationSettings_.useUnsafeIntrinsics;
  }

  const CodeGenerationSettings &getCodeGenerationSettings() const {
    return codeGenerationSettings_;
  }

  const OptimizationSettings &getOptimizationSettings() const {
    return optimizationSettings_;
  }

  /// Allocates AST nodes. Should not be used for non-AST data because
  /// the memory may be freed during parsing.
  template <typename T>
  T *allocateNode(size_t num = 1) {
    return allocator_.template Allocate<T>(num);
  }
  void *allocateNode(size_t size, size_t alignment) {
    return allocator_.Allocate(size, alignment);
  }

  hbc::BackendContext *getHBCBackendContext() {
    return hbcBackendContext_.get();
  }

  void setHBCBackendContext(
      std::shared_ptr<hbc::BackendContext> hbcBackendContext) {
    hbcBackendContext_ = std::move(hbcBackendContext);
  }

#ifdef HERMES_RUN_WASM
  EmitWasmIntrinsicsContext *getWasmIntrinsicsContext() {
    return wasmIntrinsicsContext_.get();
  }

  void setWasmIntrinsicsContext(
      std::shared_ptr<EmitWasmIntrinsicsContext> wasmIntrinsicsContext) {
    wasmIntrinsicsContext_ = std::move(wasmIntrinsicsContext);
  }
#endif // HERMES_RUN_WASM
};

} // namespace hermes

#endif // HERMES_AST_CONTEXT_H
