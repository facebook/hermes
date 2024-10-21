/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CLFlag.h"
#include "ParseJSFile.h"
#include "compile.h"

#include "hermes/AST/ASTUtils.h"
#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/AST/NativeContext.h"
#include "hermes/AST/TS2Flow.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Runtime/Libhermes.h"
#include "hermes/Sema/SemContext.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/SourceMap/SourceMapTranslator.h"
#include "hermes/Support/OSCompat.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Process.h"
#include "llvh/Support/Program.h"

using namespace hermes;
namespace cl = llvh::cl;

namespace {

OutputFormatKind toOutputFormatKind(OutputLevelKind level) {
  switch (level) {
    case OutputLevelKind::None:
      return OutputFormatKind::DumpNone;
    case OutputLevelKind::AST:
      return OutputFormatKind::DumpAST;
    case OutputLevelKind::Sema:
    case OutputLevelKind::TranspiledAST:
    case OutputLevelKind::TransformedAST:
      return OutputFormatKind::DumpTransformedAST;
    case OutputLevelKind::CFG:
      return OutputFormatKind::ViewCFG;
    case OutputLevelKind::IR:
      return OutputFormatKind::DumpIR;
    case OutputLevelKind::LIR:
      return OutputFormatKind::DumpLIR;
    case OutputLevelKind::RA:
      return OutputFormatKind::DumpRA;
    case OutputLevelKind::LRA:
      return OutputFormatKind::DumpLRA;
    case OutputLevelKind::C:
    case OutputLevelKind::Asm:
    case OutputLevelKind::Obj:
    case OutputLevelKind::SharedObj:
    case OutputLevelKind::Executable:
    case OutputLevelKind::Run:
      return OutputFormatKind::DumpBytecode;
  }
  llvm_unreachable("Unexpected OutputLevelKind");
}

} // namespace

namespace cli {

static cl::OptionCategory CompilerCategory(
    "Compiler Options",
    "These options change how JS is compiled.");

cl::list<std::string> InputFilenames(
    cl::desc("<file1> <file2>..."),
    cl::Positional);

cl::list<std::string> ExecArgs(
    "Wx,",
    cl::desc("Pass extra exec arguments (comma separated) to the runtime"),
    cl::Prefix,
    cl::CommaSeparated);

static cl::opt<std::string> OutputFilename(
    "o",
    cl::desc("Output file name"),
    cl::cat(CompilerCategory));

static cl::opt<bool> Verbose(
    "v",
    cl::desc("Enable verbose mode"),
    cl::ValueDisallowed,
    cl::ZeroOrMore);

cl::list<std::string> ExtraCCOptions(
    "Wc,",
    cl::desc("Pass extra arguments (comma separated) directly to cc"),
    cl::Prefix,
    cl::CommaSeparated);

cl::list<std::string>
    Libs("l", cl::desc("Link with the given library"), cl::Prefix);
cl::list<std::string> LibSearchPaths(
    "L",
    cl::desc("Add to the library search path and rpath"),
    cl::Prefix);

cl::opt<OptLevel> OptimizationLevel(
    cl::desc("Choose optimization level:"),
    cl::init(OptLevel::OMax),
    cl::values(
        clEnumValN(OptLevel::O0, "O0", "No optimizations"),
        clEnumValN(OptLevel::Og, "Og", "Optimizations suitable for debugging"),
        clEnumValN(OptLevel::Os, "Os", "Optimize for size"),
        clEnumValN(OptLevel::OMax, "O", "Expensive optimizations")),
    cl::cat(CompilerCategory));

cl::opt<DebugLevel> DebugInfoLevel(
    cl::desc("Choose debug info level:"),
    cl::init(DebugLevel::g0),
    cl::values(
        clEnumValN(DebugLevel::g0, "g0", "Do not emit debug info"),
        clEnumValN(DebugLevel::g1, "g1", "Emit location info for backtraces"),
        clEnumValN(
            DebugLevel::g2,
            "g2",
            "Emit location info for all instructions"),
        clEnumValN(DebugLevel::g3, "g3", "Emit full info for debugging"),
        clEnumValN(DebugLevel::g3, "g", "Equivalent to -g3")),
    cl::cat(CompilerCategory));

cl::opt<std::string> InputSourceMap(
    "source-map",
    cl::desc("Specify a matching source map for the input file"),
    cl::cat(CompilerCategory));

/// How to handle magic source map comments.
cl::opt<SourceMappingCommentMode> SourceMappingComments(
    "sm-comment",
    cl::desc("Choose how to handle //# sourceMappingURL= comments:"),
    cl::init(SourceMappingCommentMode::File),
    cl::values(
        clEnumValN(SourceMappingCommentMode::Off, "off", "Ignore them"),
        clEnumValN(
            SourceMappingCommentMode::Data,
            "data",
            "Only use if data URL"),
        clEnumValN(
            SourceMappingCommentMode::File,
            "file",
            "Only use if data or file URL")),
    cl::cat(CompilerCategory));

static cl::list<std::string> CustomOptimize(
    "Xcustom-opt",
    cl::desc("Custom optimizations"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(CompilerCategory));

cl::opt<bool> EnableAsserts(
    "enable-asserts",
#ifdef NDEBUG
    cl::desc("(default false) Whether assertions in compiled code are enabled"),
    cl::init(false),
#else
    cl::desc("(default true) Whether assertions in compiled code are enabled"),
    cl::init(true),
#endif
    cl::cat(CompilerCategory));

cl::opt<bool> Lean(
    "lean",
    cl::init(false),
    cl::desc("Link the lean VM"),
    cl::cat(CompilerCategory));

cl::opt<bool> StaticLink(
    "static-link",
    cl::init(false),
    cl::desc("Statically link against the VM"),
    cl::cat(CompilerCategory));

cl::opt<bool> KeepTemp(
    "keep-temp",
    cl::init(false),
    cl::desc("Keep temporary files made along the way (for debugging)"),
    cl::cat(CompilerCategory));

cl::opt<StaticBuiltinSetting> StaticBuiltins(
    cl::desc(
        "recognizing of calls to global functions like Object.keys() statically"),
    cl::init(StaticBuiltinSetting::AutoDetect),
    cl::values(
        clEnumValN(
            StaticBuiltinSetting::ForceOn,
            "fstatic-builtins",
            "Enable static builtins."),
        clEnumValN(
            StaticBuiltinSetting::ForceOff,
            "fno-static-builtins",
            "Disable static builtins."),
        clEnumValN(
            StaticBuiltinSetting::AutoDetect,
            "fauto-detect-static-builtins",
            "Automatically detect 'use static builtin' directive from the source.")),
    cl::cat(CompilerCategory));

cl::opt<OutputLevelKind> OutputLevel(
    cl::desc("Choose output:"),
    cl::init(OutputLevelKind::Executable),
    cl::Optional,
    cl::values(
        clEnumValN(OutputLevelKind::AST, "dump-ast", "AST as text in JSON"),
        clEnumValN(
            OutputLevelKind::TranspiledAST,
            "dump-transpiled-ast",
            "Transformed AST as text after optional early transpilation"),
        clEnumValN(
            OutputLevelKind::TransformedAST,
            "dump-transformed-ast",
            "Transformed AST as text after validation"),
        clEnumValN(OutputLevelKind::Sema, "dump-sema", "Sema tables"),
#ifndef NDEBUG
        clEnumValN(OutputLevelKind::CFG, "view-cfg", "View the CFG."),
#endif
        clEnumValN(OutputLevelKind::IR, "dump-ir", "IR as text"),
        clEnumValN(OutputLevelKind::LIR, "dump-lir", "Lowered IR as text"),
        clEnumValN(
            OutputLevelKind::RA,
            "dump-ra",
            "Register-allocated IR as text"),
        clEnumValN(
            OutputLevelKind::LRA,
            "dump-lra",
            "Register-allocated Lowered IR as text"),
        clEnumValN(OutputLevelKind::C, "emit-c", "Generated C"),
        clEnumValN(OutputLevelKind::Asm, "S", "Assembly output"),
        clEnumValN(OutputLevelKind::Obj, "c", "Object file"),
        clEnumValN(
            OutputLevelKind::Run,
            "exec",
            "Execute the compiled binary")),
    cl::cat(CompilerCategory));

static cl::list<std::string> DumpFunctions(
    "Xdump-functions",
    cl::desc("Only dump the IR for the given functions"),
    cl::Hidden,
    cl::CommaSeparated,
    cl::cat(CompilerCategory));
static cl::list<std::string> NoDumpFunctions(
    "Xno-dump-functions",
    cl::desc("Exclude the given functions from IR dumps"),
    cl::Hidden,
    cl::CommaSeparated,
    cl::cat(CompilerCategory));

static cl::opt<bool> PrintCompilerTiming(
    "ftime-report",
    cl::init(false),
    cl::desc("Turn on compiler timing output."),
    cl::cat(CompilerCategory));

static cl::opt<std::string> ExportedUnit(
    "exported-unit",
    cl::desc("Produce an SHUnit with the given name to be used by other code. "
             "When this is specified, no main function will be produced."),
    cl::cat(CompilerCategory));

cl::opt<bool> DumpBetweenPasses(
    "Xdump-between-passes",
    cl::init(false),
    cl::Hidden,
    cl::desc("Print IR after every optimization pass"),
    cl::cat(CompilerCategory));

cl::opt<bool> Pretty(
    "pretty",
    cl::init(true),
    cl::desc("Pretty print JSON, JS or disassembled bytecode"),
    cl::cat(CompilerCategory));

cl::opt<bool> Colors(
    "colors",
    cl::init(false),
    cl::desc("Use colors in some dumps"),
    cl::cat(CompilerCategory));

#if HERMES_PARSE_JSX
cl::opt<bool> JSX(
    "parse-jsx",
    cl::desc("Parse JSX"),
    cl::init(false),
    cl::cat(CompilerCategory));
#endif

#if HERMES_PARSE_FLOW
cl::opt<bool> ParseFlow(
    "parse-flow",
    cl::desc("Parse Flow"),
    cl::init(false),
    cl::cat(CompilerCategory));
#else
const bool ParseFlow = false;
#endif

#if HERMES_PARSE_TS
cl::opt<bool> ParseTS(
    "parse-ts",
    cl::desc("Parse TypeScript"),
    cl::init(false),
    cl::cat(CompilerCategory));
#else
const bool ParseTS = false;
#endif

cl::opt<bool> ES6Class{
    "Xes6-class",
    llvh::cl::Hidden,
    llvh::cl::desc("Enable support for ES6 Class"),
    llvh::cl::init(false),
    llvh::cl::cat(CompilerCategory)};

cl::opt<bool> Typed(
    "typed",
    cl::desc("Enable typed mode"),
    cl::init(false),
    cl::cat(CompilerCategory));

cl::opt<bool> Script(
    "script",
    cl::desc("Enable script mode"),
    cl::init(false),
    cl::cat(CompilerCategory));

CLFlag StdGlobals(
    'f',
    "std-globals",
    true,
    "registration of standard globals",
    CompilerCategory);

cl::opt<unsigned> ErrorLimit(
    "ferror-limit",
    cl::desc("Maximum number of errors (0 means unlimited)"),
    cl::init(20),
    cl::cat(CompilerCategory));

#define WARNING_CATEGORY(name, specifier, description) \
  CLFlag name##Warning('W', specifier, true, description, CompilerCategory);
#include "hermes/Support/Warnings.def"

static cl::ValuesClass warningValues{
#define WARNING_CATEGORY_HIDDEN(name, specifier, description) \
  clEnumValN(Warning::name, specifier, description),
#include "hermes/Support/Warnings.def"

};

cl::list<Warning> Werror(
    llvh::cl::ValueOptional,
    "Werror",
    cl::value_desc("category"),
    cl::desc(
        "Treat all warnings as errors, or treat warnings of a particular category as errors"),
    warningValues,
    cl::cat(CompilerCategory));

cl::list<Warning> Wnoerror(
    llvh::cl::ValueOptional,
    "Wno-error",
    cl::value_desc("category"),
    cl::Hidden,
    cl::desc(
        "Treat no warnings as errors, or treat warnings of a particular category as warnings"),
    warningValues,
    cl::cat(CompilerCategory));

cl::opt<bool> DisableAllWarnings(
    "w",
    cl::desc("Disable all warnings"),
    cl::init(false),
    cl::cat(CompilerCategory));

cl::opt<bool> ReusePropCache(
    "reuse-prop-cache",
    cl::desc("Reuse property cache entries for same property name"),
    cl::init(true));

CLFlag Inline('f', "inline", true, "inlining of functions", CompilerCategory);

cl::opt<unsigned> InlineMaxSize(
    "Xinline-max-size",
    cl::init(1),
    cl::desc("Suppress inlining of functions larger than the given size"),
    cl::Hidden,
    cl::cat(CompilerCategory));

static cl::opt<bool> LegacyMem2Reg(
    "Xlegacy-mem2reg",
    cl::init(false),
    cl::Hidden,
    cl::desc("Use the legacy Mem2Reg pass."),
    cl::cat(CompilerCategory));

CLFlag StripFunctionNames(
    'f',
    "strip-function-names",
    false,
    "Strip function names to reduce string table size",
    CompilerCategory);

cl::opt<bool> Test262(
    "test262",
    cl::init(false),
    cl::desc(
        "Increase compliance with test262 by moving more checks to runtime"),
    cl::cat(CompilerCategory));

cl::opt<bool> EnableTDZ(
    "Xenable-tdz",
    cl::init(false),
    cl::Hidden,
    cl::desc("UNSUPPORTED: Enable TDZ checks for let/const"),
    cl::cat(CompilerCategory));

cl::opt<bool> StrictMode(
    "strict",
    cl::desc("Enable strict mode."),
    cl::cat(CompilerCategory));

cl::opt<bool> EnableEval(
    "enable-eval",
    cl::init(true),
    cl::desc("Enable support for eval()"));

// This is normally a compiler option, but it also applies to strings given
// to eval or the Function constructor.
cl::opt<bool> VerifyIR(
    "verify-ir",
#ifdef HERMES_SLOW_DEBUG
    cl::init(true),
#else
    cl::init(false),
    cl::Hidden,
#endif
    cl::desc("Verify the IR after each pass."),
    cl::cat(CompilerCategory));

cl::opt<bool> DumpRegisterInterval(
    "dump-register-interval",
    cl::desc("Dump the liveness interval of allocated registers"),
    cl::init(false),
    cl::cat(CompilerCategory));

cl::opt<bool> DumpUseList(
    "dump-instr-uselist",
    cl::desc("Print the use list if the instruction has any users."),
    cl::init(false),
    cl::cat(CompilerCategory));

cl::opt<LocationDumpMode> DumpSourceLocation(
    "dump-source-location",
    cl::desc("Print source location information in IR or AST dumps."),
    cl::init(LocationDumpMode::None),
    cl::values(
        clEnumValN(
            LocationDumpMode::LocAndRange,
            "both",
            "Print both source location and byte range"),
        clEnumValN(LocationDumpMode::Loc, "loc", "Print only source location"),
        clEnumValN(LocationDumpMode::Range, "range", "Print only byte range")),
    cl::cat(CompilerCategory));

cl::opt<bool> ForceLineDirectives(
    "Xline-directives",
    cl::desc("Force line directives to be emitted in C."),
    cl::init(false),
    cl::Hidden,
    cl::cat(CompilerCategory));

cl::opt<bool> IncludeEmptyASTNodes(
    "Xinclude-empty-ast-nodes",
    cl::desc(
        "Print all AST nodes, including nodes that are hidden when empty."),
    cl::Hidden,
    cl::cat(CompilerCategory));

cl::opt<bool> IncludeRawASTProp(
    "Xinclude-raw-ast-prop",
    cl::desc("Print the 'raw' AST property, when available."),
    cl::init(true),
    cl::Hidden,
    cl::cat(CompilerCategory));

CLFlag CheckNativeStack(
    'f',
    "check-native-stack",
    true,
    "Emit stack overflow checks for native stack",
    CompilerCategory);

cl::opt<std::string> XNativeTarget(
    "Xnative-target",
    cl::desc("Specify the native target triple"),
    cl::Hidden,
    cl::cat(CompilerCategory));

} // namespace cli

namespace {

/// Attempt to guess the best error output options by inspecting stderr
SourceErrorOutputOptions guessErrorOutputOptions() {
  SourceErrorOutputOptions result;

  result.showColors = oscompat::should_color(STDERR_FILENO);
  result.preferredMaxErrorWidth = SourceErrorOutputOptions::UnlimitedWidth;
  if (oscompat::isatty(STDERR_FILENO)) {
    result.preferredMaxErrorWidth = llvh::sys::Process::StandardErrColumns();
  }

  // Respect MaxDiagnosticWidth if nonzero
  // if (cl::MaxDiagnosticWidth < 0) {
  //  result.preferredMaxErrorWidth = SourceErrorOutputOptions::UnlimitedWidth;
  //} else if (cl::MaxDiagnosticWidth > 0) {
  //  result.preferredMaxErrorWidth =
  //  static_cast<size_t>(cl::MaxDiagnosticWidth);
  //}
  return result;
}

/// Apply the -Werror, -Wno-error, -Werror=<category> and -Wno-error=<category>
/// flags to \c sm from left to right.
void setWarningsAreErrorsFromFlags(SourceErrorManager &sm) {
  std::vector<Warning>::iterator yesIt = cli::Werror.begin();
  std::vector<Warning>::iterator noIt = cli::Wnoerror.begin();
  // Argument positions are indices into argv and start at 1 (or 2 if there's a
  // subcommand). See llvh::cl::CommandLineParser::ParseCommandLineOptions().
  // In this loop, position 0 represents the lack of a value.
  unsigned noPos = 0, yesPos = 0;
  while (true) {
    if (noIt != cli::Wnoerror.end()) {
      noPos = cli::Wnoerror.getPosition(noIt - cli::Wnoerror.begin());
    } else {
      noPos = 0;
    }
    if (yesIt != cli::Werror.end()) {
      yesPos = cli::Werror.getPosition(yesIt - cli::Werror.begin());
    } else {
      yesPos = 0;
    }

    Warning warning;
    bool enable;
    if (yesPos != 0 && (noPos == 0 || yesPos < noPos)) {
      warning = *yesIt;
      enable = true;
      ++yesIt;
    } else if (noPos != 0 && (yesPos == 0 || noPos < yesPos)) {
      warning = *noIt;
      enable = false;
      ++noIt;
    } else {
      break;
    }

    if (warning == Warning::NoWarning) {
      sm.setWarningsAreErrors(enable);
    } else {
      sm.setWarningIsError(warning, enable);
    }
  }
}

/// Create a Context, respecting the command line flags.
/// \return the Context.
std::shared_ptr<Context> createContext() {
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.test262 = cli::Test262;
  // Test262 enables TDZ checking by default, unless the latter has been
  // specified explicitly.
  codeGenOpts.enableTDZ = cli::Test262 && !cli::EnableTDZ.getNumOccurrences()
      ? true
      : cli::EnableTDZ;
  codeGenOpts.dumpRegisterInterval = cli::DumpRegisterInterval;
  codeGenOpts.dumpUseList = cli::DumpUseList;
  codeGenOpts.dumpSourceLocation =
      cli::DumpSourceLocation != LocationDumpMode::None;
  codeGenOpts.dumpIRBetweenPasses = cli::DumpBetweenPasses;
  codeGenOpts.verifyIRBetweenPasses = cli::VerifyIR;
  codeGenOpts.colors = cli::Colors;
  codeGenOpts.dumpFunctions.insert(
      cli::DumpFunctions.begin(), cli::DumpFunctions.end());
  codeGenOpts.noDumpFunctions.insert(
      cli::NoDumpFunctions.begin(), cli::NoDumpFunctions.end());
  codeGenOpts.timeCompiler = cli::PrintCompilerTiming;

  OptimizationSettings optimizationOpts;

  optimizationOpts.inlining =
      cli::OptimizationLevel != OptLevel::O0 && cli::Inline;
  optimizationOpts.inlineMaxSize = cli::InlineMaxSize;

  optimizationOpts.reusePropCache = cli::ReusePropCache;

  // Auto enable static builtins in typed mode.
  if (cli::Typed && cli::StaticBuiltins != StaticBuiltinSetting::ForceOff)
    cli::StaticBuiltins = StaticBuiltinSetting::ForceOn;

  // When the setting is auto-detect, we will set the correct value after
  // parsing.
  optimizationOpts.staticBuiltins =
      cli::StaticBuiltins == StaticBuiltinSetting::ForceOn;
  // optimizationOpts.staticRequire = cl::StaticRequire;
  //

  optimizationOpts.useLegacyMem2Reg = cli::LegacyMem2Reg;

  NativeSettings nativeSettings{};
  nativeSettings.emitCheckNativeStack = cli::CheckNativeStack;
  // TODO: error checking, etc.
  nativeSettings.targetTriple = llvh::Triple(cli::XNativeTarget);

  auto context = std::make_shared<Context>(
      std::move(codeGenOpts), optimizationOpts, &nativeSettings);

  if (codeGenOpts.dumpIRBetweenPasses)
    context->createPersistentIRNamer();

  // Typed mode forces strict mode.
  if (cli::Typed && !cli::StrictMode && cli::StrictMode.getNumOccurrences()) {
    llvh::errs() << "error: types are incompatible with loose mode\n";
    return nullptr;
  }
  context->setStrictMode(cli::Typed || cli::StrictMode);
  context->setEnableEval(cli::EnableEval);
  context->setConvertES6Classes(cli::ES6Class);
  context->getSourceErrorManager().setOutputOptions(guessErrorOutputOptions());

  setWarningsAreErrorsFromFlags(context->getSourceErrorManager());

#define WARNING_CATEGORY(name, specifier, description) \
  context->getSourceErrorManager().setWarningStatus(   \
      Warning::name, cli::name##Warning);
#include "hermes/Support/Warnings.def"

  if (cli::DisableAllWarnings)
    context->getSourceErrorManager().disableAllWarnings();
  context->getSourceErrorManager().setErrorLimit(cli::ErrorLimit);

  // Make sure nothing is lazy
  context->setLazyCompilation(false);

#if HERMES_PARSE_JSX
  if (cli::JSX) {
    context->setParseJSX(true);
  }
#endif

#if HERMES_PARSE_FLOW
  if (!cli::ParseFlow && !cli::ParseTS)
    cli::ParseFlow = true;
  if (cli::ParseFlow)
    context->setParseFlow(ParseFlowSetting::ALL);
#endif

#if HERMES_PARSE_TS
  if (!cli::ParseFlow && !cli::ParseTS)
    cli::ParseTS = true;
  if (cli::ParseTS)
    context->setParseTS(true);
#endif

  if (!cli::ParseFlow && !cli::ParseTS && cli::Typed) {
    llvh::errs() << "error: no typed dialect parser is configured\n";
    return nullptr;
  }

  if (cli::DebugInfoLevel >= DebugLevel::g3) {
    context->setDebugInfoSetting(DebugInfoSetting::ALL);
  } else if (cli::DebugInfoLevel == DebugLevel::g2) {
    context->setDebugInfoSetting(DebugInfoSetting::SOURCE_MAP);
  } else if (cli::DebugInfoLevel == DebugLevel::g1) {
    // -g1 or -g0. If -g0, we'll strip debug info later.
    context->setDebugInfoSetting(DebugInfoSetting::THROWING);
  } else {
    context->setDebugInfoSetting(DebugInfoSetting::NONE);
  }
  // context->setEmitAsyncBreakCheck(cl::EmitAsyncBreakCheck);

  return context;
}

/// Parse the given files and return a single AST pointer.
/// \param singleInputSourceMap if non-empty, the source map to use for the
///     single input buffer.
/// \return A pointer to the new validated AST, nullptr if parsing failed.
/// If using CJS modules, return a FunctionExpressionNode, else a ProgramNode.
ESTree::NodePtr parseJS(
    std::shared_ptr<Context> &context,
    sema::SemContext &semCtx,
    flow::FlowContext *flowContext,
    const DeclarationFileListTy &ambientDecls,
    std::vector<std::unique_ptr<llvh::MemoryBuffer>> fileBufs,
    llvh::StringRef singleInputSourceMap) {
  std::vector<ESTree::ProgramNode *> programs{};
  std::shared_ptr<SourceMapTranslator> sourceMapTranslator = nullptr;

  assert(context && "Need a context to compile using");

  if (!singleInputSourceMap.empty() && fileBufs.size() > 1) {
    llvh::errs() << "Error: --source-map can only be used with a single "
                    "input file.\n";
    return nullptr;
  }

  // Save the previous strictness and force strict mode if we are parsing a
  // typed file.
  auto onExit = llvh::make_scope_exit(
      [&context, saveStrictness = context->isStrictMode()]() {
        context->setStrictMode(saveStrictness);
      });
  if (flowContext)
    context->setStrictMode(true);

  assert(
      (singleInputSourceMap.empty() || fileBufs.size() == 1) &&
      "singleInputSourceMap can only be specified for a single input file");

  // Whether a parse error ocurred in one of the inputs.
  bool parseError = false;
  for (std::unique_ptr<llvh::MemoryBuffer> &fileBuf : fileBufs) {
    assert(fileBuf && "Need a file to compile");

    if (ESTree::ProgramNode *parsedAST = parseJSFile(
            context.get(),
            cli::SourceMappingComments,
            cli::StaticBuiltins,
            context->getSourceErrorManager().addNewSourceBuffer(
                std::move(fileBuf)),
            std::exchange(singleInputSourceMap, {}),
            sourceMapTranslator)) {
      programs.push_back(parsedAST);
    } else {
      parseError = true;
    }
  }

  if (parseError)
    return nullptr;

  // If we have any source maps, we would have initialized sourceMapTranslator.
  // Set it if so.
  if (sourceMapTranslator)
    context->getSourceErrorManager().setTranslator(sourceMapTranslator);

  ESTree::ProgramNode *parsedAST = programs[0];

  // If there's multiple files, concat the programs together.
  if (programs.size() > 1) {
    ESTree::NodeList &allStmts = parsedAST->_body;
    for (size_t i = 1, e = programs.size(); i < e; ++i) {
      allStmts.splice(allStmts.end(), programs[i]->_body);
    }
  }

  if (cli::OutputLevel == OutputLevelKind::AST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cli::Pretty /* pretty */,
        cli::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                  : ESTreeDumpMode::HideEmpty,
        context->getSourceErrorManager(),
        cli::DumpSourceLocation,
        cli::IncludeRawASTProp ? ESTreeRawProp::Include
                               : ESTreeRawProp::Exclude);
    return parsedAST;
  }

  // Convert TS AST to Flow AST as an intermediate step until we have a
  // separate TS type checker.
  if (flowContext && context->getParseTS()) {
    parsedAST = convertTSToFlow(*context, parsedAST);
    if (!parsedAST) {
      return nullptr;
    }
  }

  // If we are executing in typed mode and not script, then wrap the program.
  if (cli::Typed && !cli::Script) {
    parsedAST = wrapInIIFE(context, parsedAST);
    // In case this API decides it can fail in the future, check for a
    // nullptr.
    if (!parsedAST) {
      return nullptr;
    }
  }

  if (cli::OutputLevel == OutputLevelKind::TranspiledAST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cli::Pretty /* pretty */,
        cli::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                  : ESTreeDumpMode::HideEmpty,
        context->getSourceErrorManager(),
        cli::DumpSourceLocation,
        cli::IncludeRawASTProp ? ESTreeRawProp::Include
                               : ESTreeRawProp::Exclude);
    return parsedAST;
  }
  // if (cli::OutputLevel == DumpJS) {
  //   hermes::generateJS(llvh::outs(), parsedAST, cl::Pretty /* pretty */);
  //   return parsedAST;
  // }

  if (!sema::resolveAST(
          *context, semCtx, flowContext, parsedAST, ambientDecls)) {
    return nullptr;
  }

  if (cli::OutputLevel == OutputLevelKind::TransformedAST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cli::Pretty /* pretty */,
        cli::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                  : ESTreeDumpMode::HideEmpty,
        context->getSourceErrorManager(),
        cli::DumpSourceLocation,
        cli::IncludeRawASTProp ? ESTreeRawProp::Include
                               : ESTreeRawProp::Exclude);
  }
  if (cli::OutputLevel == OutputLevelKind::Sema) {
    sema::semDump(llvh::outs(), *context, semCtx, flowContext, parsedAST);
  }

  return parsedAST;
}

bool compileFromCommandLineOptions() {
  if (cli::OutputLevel != OutputLevelKind::Run && !cli::ExecArgs.empty()) {
    llvh::errs() << "Error: unused exec arguments\n";
    return false;
  }
  if (!cli::ExportedUnit.empty()) {
    if (cli::OutputLevel == OutputLevelKind::Run ||
        cli::OutputLevel == OutputLevelKind::Executable) {
      llvh::errs()
          << "Error: cannot produce an executable when exported unit is specified.\n";
      return false;
    }

    if (!isValidSHUnitName(cli::ExportedUnit)) {
      llvh::errs() << "Error: exported name may only contain alphanumeric "
                      "characters and underscores.\n";
      return false;
    }
  }
  if (cli::InputFilenames.empty()) {
    llvh::errs() << "Error: must provide an input filename.\n";
    return false;
  }

  std::shared_ptr<Context> context = createContext();
  if (!context)
    return false;

  // A list of parsed global definition files.
  DeclarationFileListTy declFileList;

  // Load the runtime library.
  if (cli::StdGlobals) {
    if (!loadGlobalDefinition(
            *context,
            llvh::MemoryBuffer::getMemBuffer(libhermes),
            declFileList)) {
      return false;
    }
  }

  Module M(context);
  sema::SemContext semCtx(*context);
  flow::FlowContext flowContext{};
  std::vector<std::unique_ptr<llvh::MemoryBuffer>> fileBufs{};

  for (llvh::StringRef filename : cli::InputFilenames) {
    std::unique_ptr<llvh::MemoryBuffer> fileBuf =
        memoryBufferFromFile(filename, "input file", true);
    if (!fileBuf)
      return false;
    fileBufs.push_back(std::move(fileBuf));
  }

  // TODO: support input source map.
  ESTree::NodePtr ast = parseJS(
      context,
      semCtx,
      cli::Typed ? &flowContext : nullptr,
      declFileList,
      std::move(fileBufs),
      cli::InputSourceMap);
  if (!ast) {
    auto N = context->getSourceErrorManager().getErrorCount();
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }
  if (cli::OutputLevel.getNumOccurrences() &&
      cli::OutputLevel < OutputLevelKind::CFG) {
    return true;
  }
  generateIRFromESTree(&M, semCtx, flowContext, ast);

  // Bail out if there were any errors. We can't ensure that the module is in
  // a valid state.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }

  // Verify the IR before we run optimizations on it.
  if (cli::VerifyIR) {
    if (!verifyModule(M, &llvh::errs())) {
      llvh::errs() << "IRGen produced invalid IR\n";
      return false;
    }
  }

  if (!cli::CustomOptimize.empty()) {
    if (!runCustomOptimizationPasses(M, cli::CustomOptimize)) {
      llvh::errs() << "Invalid custom optimizations selected.\n\n"
                   << PassManager::getCustomPassText();
      return false;
    }
  } else {
    // Always run the native backend optimizations.
    runNativeBackendOptimizationPasses(M);

    switch (cli::OptimizationLevel) {
      case OptLevel::O0:
        runNoOptimizationPasses(M);
        break;
      case OptLevel::Og:
        runDebugOptimizationPasses(M);
        break;
      case OptLevel::Os:
      case OptLevel::OMax:
        runFullOptimizationPasses(M);
        break;
    }
  }

  // Bail out if there were any errors during optimization.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }

  if (cli::OutputLevel == OutputLevelKind::IR) {
    M.dump();
    return true;
  }

#ifndef NDEBUG
  if (cli::OutputLevel == OutputLevelKind::CFG) {
    M.viewGraph();
    return true;
  }
#endif

  BytecodeGenerationOptions genOptions{toOutputFormatKind(cli::OutputLevel)};
  genOptions.optimizationEnabled = cli::OptimizationLevel > OptLevel::Og;
  // genOptions.basicBlockProfiling = cl::BasicBlockProfiling;
  //  The static builtin setting should be set correctly after command line
  //  options parsing and js parsing. Set the bytecode header flag here.
  genOptions.staticBuiltinsEnabled = context->getStaticBuiltinOptimization();
  // genOptions.padFunctionBodiesPercent = cl::PadFunctionBodiesPercent;

  genOptions.verifyIR = cli::VerifyIR;

  // If the user requests to output a source map, then do not also emit debug
  // info into the bytecode.
  // genOptions.stripDebugInfoSection =
  //    cl::OutputSourceMap || cl::DebugInfoLevel == cl::DebugLevel::g0;

  genOptions.stripFunctionNames = cli::StripFunctionNames;

  // If we are not exporting a unit, produce the main function.
  genOptions.emitMain = cli::ExportedUnit.empty();
  if (!cli::ExportedUnit.empty())
    genOptions.unitName = cli::ExportedUnit;

  genOptions.emitSourceLocations =
      cli::DumpSourceLocation != LocationDumpMode::None;

  // Emit line directives if we have full debug info enabled or it was
  // explicitly requested.
  genOptions.emitLineDirectives =
      cli::DebugInfoLevel >= DebugLevel::g3 || cli::ForceLineDirectives;

  ShermesCompileParams params(genOptions);
  // Populate all fields of ShermesCompileParams.
  params.nativeOptimize = cli::OptimizationLevel;
  params.enableAsserts = cli::EnableAsserts
      ? ShermesCompileParams::EnableAsserts::on
      : ShermesCompileParams::EnableAsserts::off;
  params.lean = cli::Lean ? ShermesCompileParams::Lean::on
                          : ShermesCompileParams::Lean::off;
  params.staticLink = cli::StaticLink ? ShermesCompileParams::StaticLink::on
                                      : ShermesCompileParams::StaticLink::off;
  params.extraCCOptions = cli::ExtraCCOptions;
  params.libs = cli::Libs;
  params.libSearchPaths = cli::LibSearchPaths;
  params.keepTemp = cli::KeepTemp ? ShermesCompileParams::KeepTemp::on
                                  : ShermesCompileParams::KeepTemp::off;
  params.verbosity = cli::Verbose.getNumOccurrences();

  return shermesCompile(
      context.get(),
      M,
      params,
      cli::OutputLevel,
      cli::InputFilenames[cli::InputFilenames.size() - 1],
      cli::OutputFilename,
      cli::ExecArgs);
}

} // namespace

int main(int argc, char **argv) {
#ifndef HERMES_FBCODE_BUILD
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
#else
  // When both HERMES_FBCODE_BUILD and sanitizers are enabled, InitLLVM may have
  // been already created and destroyed before main() is invoked. This presents
  // a problem because InitLLVM can't be instantiated more than once in the same
  // process. The most important functionality InitLLVM provides is shutting
  // down LLVM in its destructor. We can use "llvm_shutdown_obj" to do the same.
  llvh::llvm_shutdown_obj Y;
#endif
  llvh::cl::AddExtraVersionPrinter(
      [](llvh::raw_ostream &OS) { OS << "Static Hermes JS Compiler v0.0\n"; });
  llvh::cl::ParseCommandLineOptions(argc, argv, "Static Hermes\n");

  if (!compileFromCommandLineOptions())
    return 1;
  return 0;
}
