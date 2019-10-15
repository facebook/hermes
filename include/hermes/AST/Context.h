/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_CONTEXT_H
#define HERMES_AST_CONTEXT_H

#include "hermes/Parser/PreParser.h"
#include "hermes/Support/Allocator.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/StringTable.h"

namespace hermes {

namespace hbc {
class BackendContext;
};

/// Choices for bundling format, applicable to cross module opts
enum class BundlerKind { none, metromin };

struct CodeGenerationSettings {
  /// Whether we should emit TDZ checks.
  bool enableTDZ{true};
  /// Whether we can assume there are unlimited number of registers.
  /// This affects how we generate the IR, as we can decide whether
  /// to hold as many temporary values as we like.
  bool unlimitedRegisters{true};
  /// Dump registers assigned to instruction operands.
  bool dumpOperandRegisters{false};
  /// Print source location information in IR dumps.
  bool dumpSourceLocation{false};
  /// Print the use list if the instruction has any users.
  bool dumpUseList{false};
  /// Dump IR after every pass.
  bool dumpIRBetweenPasses{false};
};

struct OutliningSettings {
  /// If true, place outlined functions near one of their callers. Otherwise,
  /// put them all together at the end of the module.
  bool placeNearCaller{true};
  /// Maximum number of outlining rounds.
  unsigned maxRounds{1};
  /// Minimum length (number of instructions) to consider outlining.
  unsigned minLength{64};
  /// Minimum number of parameters for outlined functions.
  unsigned minParameters{0};
  /// Maximum number of parameters for outlined functions.
  unsigned maxParameters{5};
};

struct OptimizationSettings {
  /// Enable constant property optimization
  bool constantPropertyOptimizations{false};

  /// Enable unused method optimization
  bool uncalledMethodOptimizations{false};

  /// Enable cross-module closure analysis (if CLA is enabled)
  BundlerKind crossModuleClosureAnalysis{BundlerKind::none};

  /// Enable aggressive non-strict mode optimizations. These optimizations
  /// assume that:
  ///   - function arguments are never modified indirectly
  ///   - local "eval()" or "with" are not used.
  bool aggressiveNonStrictModeOptimizations{true};

  /// Enable any inlining of functions.
  bool inlining{true};

  /// Enable IR outlining.
  bool outlining{false};

  /// Specific settings for the outliner.
  OutliningSettings outliningSettings;

  /// Reuse property cache entries for same property name.
  bool reusePropCache{true};

  /// Recognize calls to global functions like Object.keys() and turn them
  /// into builtin calls.
  bool staticBuiltins{false};

  /// Attempt to resolve CommonJS require() calls at compile time.
  bool staticRequire{false};
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

/// Holds shared dependencies and state.
class Context {
 public:
  using Allocator = hermes::BumpPtrAllocator;

  /// Mapping from require() arguments to actual resolved paths.
  /// Strings owned by a JSON Parser allocator that the user of Context should
  /// manage.
  using ResolutionTableEntry = llvm::DenseMap<llvm::StringRef, llvm::StringRef>;

  /// Mapping from file names to the dictionary to use to look up resolutions.
  /// Strings owned by a JSON Parser allocator that the user of Context should
  /// manage.
  using ResolutionTable = llvm::DenseMap<llvm::StringRef, ResolutionTableEntry>;

  /// Represents a range of modules used in a given segment.
  struct SegmentRange {
    /// ID of the segment this range represents.
    uint32_t segment;

    /// ID of the first module in this segment, inclusive.
    uint32_t first;

    /// ID of the last module in this segment, inclusive.
    uint32_t last;
  };

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

  /// If true, wrap each file in the CommonJS module wrapper function,
  /// and use that for requiring modules.
  bool useCJSModules_{false};

  /// If non-null, the resolution table which resolves static require().
  const std::unique_ptr<ResolutionTable> resolutionTable_;

  /// The table of segment ranges. The ranges are contiguous and generated based
  /// on the user's metadata input to the compiler when splitting the bundle.
  /// Determines which CJS modules are placed into which segment when splitting
  /// the result bundle.
  const std::vector<SegmentRange> segmentRanges_;

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

 public:
  explicit Context(
      SourceErrorManager &sm,
      CodeGenerationSettings codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<SegmentRange> segmentRanges = {})
      : sm_(sm),
        resolutionTable_(std::move(resolutionTable)),
        segmentRanges_(std::move(segmentRanges)),
        codeGenerationSettings_(std::move(codeGenOpts)),
        optimizationSettings_(std::move(optimizationOpts)) {}

  explicit Context(
      CodeGenerationSettings codeGenOpts = CodeGenerationSettings(),
      OptimizationSettings optimizationOpts = OptimizationSettings(),
      std::unique_ptr<ResolutionTable> resolutionTable = nullptr,
      std::vector<SegmentRange> segmentRanges = {})
      : ownSm_(new SourceErrorManager()),
        sm_(*ownSm_),
        resolutionTable_(std::move(resolutionTable)),
        segmentRanges_(std::move(segmentRanges)),
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

  parser::PreParsedBufferInfo *getPreParsedBufferInfo(uint32_t bufferId) {
    if (!preParsed_)
      preParsed_ = llvm::make_unique<parser::PreParsedData>();
    return preParsed_->getBufferInfo(bufferId);
  }

  SourceErrorManager &getSourceErrorManager() {
    return sm_;
  }

  /// \return the table for static require resolution, nullptr if not supplied.
  const std::vector<SegmentRange> &getSegmentRanges() const {
    return segmentRanges_;
  }

  /// \return the table for static require resolution, nullptr if not supplied.
  const ResolutionTable *getResolutionTable() const {
    return resolutionTable_.get();
  }

  /// Get or create a new identifier for the string \p str. The method copies
  /// the content of the string.
  Identifier getIdentifier(llvm::StringRef str) {
    return stringTable_.getIdentifier(str);
  }

  /// Return the textual representation of the identifier.
  StringRef toString(Identifier iden) {
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

  bool isLazyCompilation() const {
    return lazyCompilation_;
  }

  void setLazyCompilation(bool lazyCompilation) {
    lazyCompilation_ = lazyCompilation;
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

  hbc::BackendContext *getHBCBackendContext() {
    return hbcBackendContext_.get();
  }

  void setHBCBackendContext(
      std::shared_ptr<hbc::BackendContext> hbcBackendContext) {
    hbcBackendContext_ = std::move(hbcBackendContext);
  }
};

}; // namespace hermes

#endif // HERMES_AST_CONTEXT_H
