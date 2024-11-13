/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_CONTEXT_H
#define HERMES_AST_CONTEXT_H

#include "hermes/ADT/StringSetVector.h"
#include "hermes/Parser/PreParser.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Support/Allocator.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/StringTable.h"

#include "llvh/ADT/StringRef.h"

namespace hermes {

namespace irdumper {
class Namer;
}

class BackendContext;
class NativeContext;
struct NativeSettings;

struct CodeGenerationSettings {
  /// Increase compliance with test262 by moving some checks to runtime.
  bool test262{false};
  /// Whether we should emit TDZ checks.
  bool enableTDZ{false};
  /// Dump register liveness intervals.
  bool dumpRegisterInterval{false};
  /// Print source location information in IR dumps.
  bool dumpSourceLocation{false};
  /// Print the use list if the instruction has any users.
  bool dumpUseList{false};
  /// Dump IR after every pass.
  bool dumpIRBetweenPasses{false};
  /// Run the IRVerifier between every pass.
  bool verifyIRBetweenPasses{false};
  /// Use colors in IR dumps.
  bool colors{false};
  /// If not empty, restricts IR dumps only the given functions.
  StringSetVector dumpFunctions;
  /// Functions to exclude from IR dumps.
  StringSetVector noDumpFunctions;
  // Time the optimizer by phases.
  bool timeCompiler{false};
};

struct OptimizationSettings {
  /// Enable any inlining of functions.
  bool inlining{true};

  /// Maximum number of instructions (in addition to parameter handling)
  /// that is allowed for inlining of small functions.
  unsigned inlineMaxSize{1};

  /// Reuse property cache entries for same property name.
  bool reusePropCache{true};

  /// Recognize calls to global functions like Object.keys() and turn them
  /// into builtin calls.
  bool staticBuiltins{false};

  /// Attempt to resolve CommonJS require() calls at compile time.
  bool staticRequire{false};

  /// Whether to use old Mem2Reg pass instead of SimpleMem2Reg. This may produce
  /// better code for irreducible CFGs.
  bool useLegacyMem2Reg{false};
};

enum class DebugInfoSetting {
  /// Don't emit any source locations info.
  NONE,

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

/// Custom directives which were specified on a given function.
struct CustomDirectives {
  /// Source visibility of the given function.
  SourceVisibility sourceVisibility{SourceVisibility::Default};

  /// Whether we should _always_ attempt to inline the function,
  /// regardless of the number of callsites it has.
  /// It's possible the function can't be inlined if it contains
  /// code which can't be inlined, but the heuristic won't reject it.
  bool alwaysInline{false};

  /// Whether we should _never_ attempt to inline the function.
  /// Useful (at least) in tests.
  bool noInline{false};
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

  /// If true, allow parsing component syntax when also using Flow syntax.
  bool parseFlowComponentSyntax_{false};

  /// Whether to parse Flow type syntax.
  ParseFlowSetting parseFlow_{ParseFlowSetting::NONE};

  /// Whether to parse TypeScript syntax.
  bool parseTS_{false};

  /// Whether to convert ES6 classes to ES5 functions
  bool convertES6Classes_{false};

  /// Whether to enable support for ES6 block scoping.
  /// TODO: This is intended to provide a temporary way to configure block
  ///       scoping until we have debugger support for it.
  bool enableES6BlockScoping_{false};

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
  std::shared_ptr<BackendContext> hbcBackendContext_{};

  /// The separate native context. It is automatically created on construction.
  std::unique_ptr<NativeContext> nativeContext_;

  std::unique_ptr<irdumper::Namer> persistentIRNamer_;

 public:
  explicit Context(
      SourceErrorManager &sm,
      CodeGenerationSettings &&codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      const NativeSettings *nativeSettings = nullptr,
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<uint32_t> segments = {});

  explicit Context(
      CodeGenerationSettings &&codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      const NativeSettings *nativeSettings = nullptr,
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<uint32_t> segments = {});

  Context(const Context &) = delete;
  void operator=(const Context &) = delete;

  ~Context();

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
  Identifier getIdentifier(const llvh::Twine &str) {
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

  void setParseFlowComponentSyntax(bool parseFlowComponentSyntax) {
    parseFlowComponentSyntax_ = parseFlowComponentSyntax;
  }
  bool getParseFlowComponentSyntax() const {
    return parseFlowComponentSyntax_;
  }

  void setParseTS(bool parseTS) {
    parseTS_ = parseTS;
  }
  bool getParseTS() const {
    return parseTS_;
  }

  void setConvertES6Classes(bool convertES6Classes) {
    convertES6Classes_ = convertES6Classes;
  }

  bool getConvertES6Classes() const {
#ifndef HERMES_FACEBOOK_BUILD
    return convertES6Classes_;
#else
    return false;
#endif
  }

  void setEnableES6BlockScoping(bool enableES6BlockScoping) {
    enableES6BlockScoping_ = enableES6BlockScoping;
  }

  bool getEnableES6BlockScoping() const {
    return enableES6BlockScoping_;
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

  BackendContext *getBackendContext() {
    return hbcBackendContext_.get();
  }

  void setBackendContext(std::shared_ptr<BackendContext> hbcBackendContext) {
    hbcBackendContext_ = std::move(hbcBackendContext);
  }

  /// \return the native context.
  NativeContext &getNativeContext() {
    return *nativeContext_;
  }

  /// Create and install a new persistent namer for IR dumps.
  void createPersistentIRNamer();
  /// Clear and destroy the persistent namer for IR dumps.
  void clearPersistentIRNamer();

  /// Return the optional persistent namer used for IR dumps, or nullptr.
  irdumper::Namer *getPersistentIRNamer() {
    return persistentIRNamer_.get();
  }
};

} // namespace hermes

#endif // HERMES_AST_CONTEXT_H
