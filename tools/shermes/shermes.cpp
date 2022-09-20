/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CLFlag.h"
#include "compile.h"

#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Runtime/Libhermes.h"
#include "hermes/SourceMap/SourceMap.h"
#include "hermes/SourceMap/SourceMapTranslator.h"
#include "hermes/Support/OSCompat.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Process.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"

using namespace hermes;
namespace cl = llvh::cl;

namespace {

OutputFormatKind toOutputFormatKind(OutputLevelKind level) {
  switch (level) {
    case OutputLevelKind::None:
      return OutputFormatKind::DumpNone;
    case OutputLevelKind::AST:
      return OutputFormatKind::DumpAST;
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
    case OutputLevelKind::PostRA:
      return OutputFormatKind::DumpPostRA;
    case OutputLevelKind::C:
    case OutputLevelKind::Asm:
    case OutputLevelKind::Obj:
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

cl::opt<std::string>
    InputFilename(cl::desc("<file>"), cl::Positional, cl::Required);

static cl::opt<std::string> OutputFilename(
    "o",
    cl::desc("Output file name"),
    cl::cat(CompilerCategory));

static cl::opt<bool> Verbose(
    "v",
    cl::desc("Enable verbose mode"),
    cl::ValueDisallowed,
    cl::ZeroOrMore);

cl::opt<OptLevel> OptimizationLevel(
    cl::desc("Choose optimization level:"),
    cl::init(OptLevel::OMax),
    cl::values(
        clEnumValN(OptLevel::O0, "O0", "No optimizations"),
        clEnumValN(OptLevel::Og, "Og", "Optimizations suitable for debugging"),
        clEnumValN(OptLevel::Os, "Os", "Optimize for size"),
        clEnumValN(OptLevel::OMax, "O", "Expensive optimizations")),
    cl::cat(CompilerCategory));

cl::opt<bool> Lean(
    "lean",
    cl::init(false),
    cl::desc("Link the lean VM"),
    cl::cat(CompilerCategory));

enum class StaticBuiltinSetting {
  ForceOn,
  ForceOff,
  AutoDetect,
};

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
            OutputLevelKind::TransformedAST,
            "dump-transformed-ast",
            "Transformed AST as text after validation"),
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
        clEnumValN(
            OutputLevelKind::PostRA,
            "dump-postra",
            "Lowered IR after register allocation"),
        clEnumValN(OutputLevelKind::C, "emit-c", "Generated C"),
        clEnumValN(OutputLevelKind::Asm, "S", "Assembly output"),
        clEnumValN(OutputLevelKind::Obj, "c", "Object file"),
        clEnumValN(
            OutputLevelKind::Run,
            "exec",
            "Execute the compiled binary")),
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
#endif

#if HERMES_PARSE_TS
cl::opt<bool> ParseTS(
    "parse-ts",
    cl::desc("Parse TypeScript"),
    cl::init(false),
    cl::cat(CompilerCategory));
#endif

cl::opt<unsigned> ErrorLimit(
    "ferror-limit",
    cl::desc("Maximum number of errors (0 means unlimited)"),
    cl::init(20),
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

CLFlag StripFunctionNames(
    'f',
    "strip-function-names",
    false,
    "Strip function names to reduce string table size",
    CompilerCategory);

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
    cl::desc("Verify the IR after creating it"),
    cl::cat(CompilerCategory));

cl::opt<bool> DumpOperandRegisters(
    "dump-operand-registers",
    cl::desc("Dump registers assigned to instruction operands"),
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

CLFlag UseUnsafeIntrinsics(
    'f',
    "unsafe-intrinsics",
    false,
    "Recognize and lower Asm.js/Wasm unsafe compiler intrinsics.",
    CompilerCategory);

} // namespace cli

namespace {

/// Read a file at path \p path into a memory buffer. If \p stdinOk is set,
/// allow "-" to mean stdin.
/// \param silent if true, don't print an error message on failure.
/// \return the memory buffer, or nullptr on error, in
/// which case an error message will have been printed to llvh::errs().
std::unique_ptr<llvh::MemoryBuffer> memoryBufferFromFile(
    llvh::StringRef path,
    bool stdinOk = false,
    bool silent = false) {
  auto fileBuf = stdinOk ? llvh::MemoryBuffer::getFileOrSTDIN(path)
                         : llvh::MemoryBuffer::getFile(path);
  if (!fileBuf) {
    if (!silent) {
      llvh::errs() << "Error! Failed to open file: " << path << '\n';
    }
    return nullptr;
  }
  return std::move(*fileBuf);
}

/// Loads global definitions from MemoryBuffer and adds the definitions to \p
/// declFileList.
/// \return true on success, false on error.
bool loadGlobalDefinition(
    Context &context,
    std::unique_ptr<llvh::MemoryBuffer> content,
    DeclarationFileListTy &declFileList) {
  parser::JSParser jsParser(context, std::move(content));
  auto parsedJs = jsParser.parse();
  if (!parsedJs)
    return false;

  declFileList.push_back(parsedJs.getValue());
  return true;
}

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

/// Create a Context, respecting the command line flags.
/// \return the Context.
std::shared_ptr<Context> createContext() {
  CodeGenerationSettings codeGenOpts;
  // codeGenOpts.enableTDZ = cl::EnableTDZ;
  codeGenOpts.dumpOperandRegisters = cli::DumpOperandRegisters;
  codeGenOpts.dumpUseList = cli::DumpUseList;
  codeGenOpts.dumpSourceLocation =
      cli::DumpSourceLocation != LocationDumpMode::None;
  codeGenOpts.dumpIRBetweenPasses = cli::DumpBetweenPasses;
  // codeGenOpts.instrumentIR = cl::InstrumentIR;

  OptimizationSettings optimizationOpts;

  optimizationOpts.aggressiveNonStrictModeOptimizations = true;

  optimizationOpts.inlining =
      cli::OptimizationLevel != OptLevel::O0 && cli::Inline;

  optimizationOpts.reusePropCache = cli::ReusePropCache;

  // When the setting is auto-detect, we will set the correct value after
  // parsing.
  optimizationOpts.staticBuiltins =
      cli::StaticBuiltins == cli::StaticBuiltinSetting::ForceOn;
  // optimizationOpts.staticRequire = cl::StaticRequire;
  //
  optimizationOpts.useUnsafeIntrinsics = cli::UseUnsafeIntrinsics;

  auto context = std::make_shared<Context>(codeGenOpts, optimizationOpts);

  // Default is non-strict mode.
  context->setStrictMode(cli::StrictMode);
  context->setEnableEval(cli::EnableEval);
  context->getSourceErrorManager().setOutputOptions(guessErrorOutputOptions());

  // setWarningsAreErrorsFromFlags(context->getSourceErrorManager());

  //#define WARNING_CATEGORY(name, specifier, description) \
//  context->getSourceErrorManager().setWarningStatus(   \
//      Warning::name, cl::name##Warning);
  //#include "hermes/Support/Warnings.def"

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
  if (cli::ParseFlow) {
    context->setParseFlow(ParseFlowSetting::ALL);
  }
#endif

#if HERMES_PARSE_TS
  if (cli::ParseTS) {
    context->setParseTS(true);
  }
#endif

  // if (cl::DebugInfoLevel >= cl::DebugLevel::g3) {
  //   context->setDebugInfoSetting(DebugInfoSetting::ALL);
  // } else if (cl::DebugInfoLevel == cl::DebugLevel::g2) {
  //   context->setDebugInfoSetting(DebugInfoSetting::SOURCE_MAP);
  // } else {
  //   // -g1 or -g0. If -g0, we'll strip debug info later.
  //   context->setDebugInfoSetting(DebugInfoSetting::THROWING);
  // }
  // context->setEmitAsyncBreakCheck(cl::EmitAsyncBreakCheck);
  return context;
}

/// Parse the given files and return a single AST pointer.
/// \p sourceMap any parsed source map associated with \p fileBuf.
/// \p sourceMapTranslator input source map coordinate translator.
/// \return A pointer to the new validated AST, nullptr if parsing failed.
/// If using CJS modules, return a FunctionExpressionNode, else a ProgramNode.
ESTree::NodePtr parseJS(
    std::shared_ptr<Context> &context,
    sem::SemContext &semCtx,
    std::unique_ptr<llvh::MemoryBuffer> fileBuf,
    std::unique_ptr<SourceMap> sourceMap = nullptr,
    std::shared_ptr<SourceMapTranslator> sourceMapTranslator = nullptr,
    bool wrapCJSModule = false) {
  assert(fileBuf && "Need a file to compile");
  assert(context && "Need a context to compile using");
  // This value will be set to true if the parser detected the 'use static
  // builtin' directive in the source.
  bool useStaticBuiltinDetected = false;

  bool isLargeFile = fileBuf->getBufferSize() >=
      context->getPreemptiveFileCompilationThreshold();

  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));
  if (sourceMap != nullptr && sourceMapTranslator != nullptr) {
    sourceMapTranslator->addSourceMap(fileBufId, std::move(sourceMap));
  }

  auto mode = parser::FullParse;

  if (context->isLazyCompilation() && isLargeFile) {
    if (!parser::JSParser::preParseBuffer(
            *context, fileBufId, useStaticBuiltinDetected)) {
      return nullptr;
    }
    mode = parser::LazyParse;
  }

  llvh::Optional<ESTree::ProgramNode *> parsedJs;

#ifdef HERMES_USE_FLOWPARSER
  if (cl::FlowParser) {
    parsedJs = parser::parseFlowParser(*context, fileBufId);
  } else
#endif
  {
    parser::JSParser jsParser(*context, fileBufId, mode);
    parsedJs = jsParser.parse();
    // If we are using lazy parse mode, we should have already detected the 'use
    // static builtin' directive in the pre-parsing stage.
    if (mode != parser::LazyParse) {
      useStaticBuiltinDetected = jsParser.getUseStaticBuiltin();
    }
  }
  if (!parsedJs)
    return nullptr;
  ESTree::NodePtr parsedAST = parsedJs.getValue();

  if (cli::StaticBuiltins == cli::StaticBuiltinSetting::AutoDetect) {
    context->setStaticBuiltinOptimization(useStaticBuiltinDetected);
  }

  assert(!wrapCJSModule && "unsupported");
  // if (wrapCJSModule) {
  //   parsedAST =
  //       hermes::wrapCJSModule(context, cast<ESTree::ProgramNode>(parsedAST));
  //   if (!parsedAST) {
  //     return nullptr;
  //   }
  // }

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
  // if (cli::OutputLevel == DumpJS) {
  //   hermes::generateJS(llvh::outs(), parsedAST, cl::Pretty /* pretty */);
  //   return parsedAST;
  // }

  if (!hermes::sem::validateAST(*context, semCtx, parsedAST)) {
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

  return parsedAST;
}

bool compileFromCommandLineOptions() {
  std::unique_ptr<llvh::MemoryBuffer> fileBuf =
      memoryBufferFromFile(cli::InputFilename, true);
  if (!fileBuf)
    return false;

  std::shared_ptr<Context> context = createContext();

  // A list of parsed global definition files.
  DeclarationFileListTy declFileList;

  // Load the runtime library.
  if (!loadGlobalDefinition(
          *context,
          llvh::MemoryBuffer::getMemBuffer(libhermes),
          declFileList)) {
    return false;
  }

  Module M(context);
  sem::SemContext semCtx{};

  // TODO: support input source map.
  ESTree::NodePtr ast = parseJS(context, semCtx, std::move(fileBuf));
  if (!ast) {
    return false;
  }
  if (cli::OutputLevel.getNumOccurrences() &&
      cli::OutputLevel < OutputLevelKind::CFG) {
    return true;
  }
  generateIRFromESTree(ast, &M, declFileList, {});
  // Bail out if there were any errors. We can't ensure that the module is in
  // a valid state.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }
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

  // Bail out if there were any errors during optimization.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }

  // In dbg builds, verify the module before we emit bytecode.
  if (cli::VerifyIR) {
    bool failedVerification = verifyModule(M, &llvh::errs());
    if (failedVerification) {
      M.dump();
      return false;
    }
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
  genOptions.prettyDisassemble = cli::Pretty;
  // genOptions.basicBlockProfiling = cl::BasicBlockProfiling;
  //  The static builtin setting should be set correctly after command line
  //  options parsing and js parsing. Set the bytecode header flag here.
  genOptions.staticBuiltinsEnabled = context->getStaticBuiltinOptimization();
  // genOptions.padFunctionBodiesPercent = cl::PadFunctionBodiesPercent;

  // If the user requests to output a source map, then do not also emit debug
  // info into the bytecode.
  // genOptions.stripDebugInfoSection =
  //    cl::OutputSourceMap || cl::DebugInfoLevel == cl::DebugLevel::g0;

  genOptions.stripFunctionNames = cli::StripFunctionNames;

  return shermesCompile(
      context.get(),
      M,
      ShermesCompileParams(
          genOptions,
          cli::OptimizationLevel,
          cli::Lean ? ShermesCompileParams::Lean::on
                    : ShermesCompileParams::Lean::off,
          cli::Verbose.getNumOccurrences()),
      cli::OutputLevel,
      cli::InputFilename,
      cli::OutputFilename);
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
