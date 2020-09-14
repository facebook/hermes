/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/CompilerDriver/CompilerDriver.h"

#include "hermes/AST/CommonJS.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/RegAlloc.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/FlowParser/FlowParser.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Runtime/Libhermes.h"
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/SourceMap/SourceMapTranslator.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/Warning.h"
#include "hermes/Utils/Dumper.h"
#include "hermes/Utils/Options.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/Debug.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/Process.h"
#include "llvh/Support/SHA1.h"
#include "llvh/Support/raw_ostream.h"

#include "zip/src/zip.h"

#include <sstream>

#define DEBUG_TYPE "hermes"

using llvh::ArrayRef;
using llvh::cast;
using llvh::dyn_cast;
using llvh::Optional;
using llvh::raw_fd_ostream;
using llvh::sys::fs::F_None;
using llvh::sys::fs::F_Text;

using namespace hermes;
using namespace hermes::driver;

namespace cl {
using llvh::cl::cat;
using llvh::cl::desc;
using llvh::cl::Hidden;
using llvh::cl::init;
using llvh::cl::list;
using llvh::cl::opt;
using llvh::cl::OptionCategory;
using llvh::cl::Positional;
using llvh::cl::value_desc;
using llvh::cl::values;

/// Encapsulate a compiler flag: for example, "-fflag/-fno-flag", or
/// "-Wflag/-Wno-flag".
class CLFlag {
  std::string yesName_;
  std::string yesHelp_;
  std::string noName_;
  std::string noHelp_;
  llvh::cl::opt<bool> yes_;
  llvh::cl::opt<bool> no_;
  const bool defaultValue_;

 public:
  CLFlag(const CLFlag &) = delete;
  void operator=(CLFlag &) = delete;

  /// \param flagChar is the character that will be prepended to the flag name.
  /// \param name is the name for the command line option
  /// \param defaultValue is the default if neither is specified.
  /// \param desc is the description starting with lower case like " inlining of
  /// functions".
  CLFlag(
      char flagChar,
      const llvh::Twine &name,
      bool defaultValue,
      const llvh::Twine &desc,
      llvh::cl::OptionCategory &category)
      : yesName_((llvh::Twine(flagChar) + name).str()),
        yesHelp_(("Enable " + desc).str()),
        noName_((llvh::Twine(flagChar) + "no-" + name).str()),
        noHelp_(("Disable " + desc).str()),
        yes_(
            StringRef(yesName_),
            llvh::cl::ValueDisallowed,
            llvh::cl::desc(StringRef(yesHelp_)),
            llvh::cl::cat(category)),
        no_(StringRef(noName_),
            llvh::cl::ValueDisallowed,
            llvh::cl::Hidden,
            llvh::cl::desc(StringRef(noHelp_)),
            llvh::cl::cat(category)),
        defaultValue_(defaultValue) {}

  /// Resolve the value of the flag depending on which command line option is
  /// present and which one is last.
  bool getValue() const {
    if (yes_.getPosition() > no_.getPosition())
      return true;
    if (yes_.getPosition() < no_.getPosition())
      return false;
    return defaultValue_;
  }

  /// Casting to bool always makes sense, so no "explicit" needed here.
  operator bool() const {
    return getValue();
  }
};

static OptionCategory CompilerCategory(
    "Compiler Options",
    "These options change how JS is compiled.");

list<std::string> InputFilenames(desc("<file1> <file2>..."), Positional);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
static opt<bool> PrintStats("print-stats", desc("Print statistics"));
#endif

enum class OptLevel {
  O0,
  Og,
  OMax,
};

cl::opt<OptLevel> OptimizationLevel(
    cl::desc("Choose optimization level:"),
    cl::init(OptLevel::OMax),
    cl::values(
        clEnumValN(OptLevel::O0, "O0", "No optimizations"),
        clEnumValN(OptLevel::Og, "Og", "Optimizations suitable for debugging"),
        clEnumValN(OptLevel::OMax, "O", "Expensive optimizations")),
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

static list<std::string> CustomOptimize(
    "custom-opt",
    desc("Custom optimzations"),
    Hidden,
    cat(CompilerCategory));

static opt<OutputFormatKind> DumpTarget(
    desc("Choose output:"),
    init(Execute),
    values(
        clEnumValN(Execute, "exec", "Execute the provided script (default)"),
        clEnumValN(DumpAST, "dump-ast", "Dump the AST as text in JSON"),
        clEnumValN(
            DumpTransformedAST,
            "dump-transformed-ast",
            "Dump the transformed AST as text after validation"),
#ifndef NDEBUG
        clEnumValN(ViewCFG, "view-cfg", "View the CFG."),
#endif
        clEnumValN(DumpIR, "dump-ir", "Dump the IR as text"),
        clEnumValN(DumpLIR, "dump-lir", "Dump the Lowered IR as text"),
        clEnumValN(DumpRA, "dump-ra", "Dump the register-allocated IR as text"),
        clEnumValN(
            DumpLRA,
            "dump-lra",
            "Dump register-allocated Lowered IR as text"),
        clEnumValN(
            DumpPostRA,
            "dump-postra",
            "Dump the Lowered IR after register allocation"),
        clEnumValN(DumpBytecode, "dump-bytecode", "Dump bytecode as text"),
        clEnumValN(EmitBundle, "emit-binary", "Emit compiled binary")),
    cat(CompilerCategory));

static opt<bool> PrettyJSON(
    "pretty-json",
    init(false),
    desc("Pretty print the JSON AST"),
    cat(CompilerCategory));

static opt<bool> PrettyDisassemble(
    "pretty-disassemble",
    init(true),
    desc("Pretty print the disassembled bytecode"),
    cat(CompilerCategory));

/// Unused option kept for backwards compatibility.
static opt<bool> unused_HermesParser(
    "hermes-parser",
    desc("Treat the input as JavaScript"),
    Hidden,
    cat(CompilerCategory));

static opt<bool> FlowParser(
    "Xflow-parser",
    init(false),
    desc("Use libflowparser instead of the hermes parser"),
    Hidden,
    cat(CompilerCategory));

static opt<bool> BytecodeMode(
    "b",
    desc("Treat the input as executable bytecode"));

static opt<bool> NonStrictMode(
    "non-strict",
    desc("Enable non-strict mode."),
    cat(CompilerCategory));
static opt<bool>
    StrictMode("strict", desc("Enable strict mode."), cat(CompilerCategory));

static opt<bool> LazyCompilation(
    "lazy",
    init(false),
    desc("Compile source lazily when executing (HBC only)"),
    cat(CompilerCategory));

/// The following flags are exported so it may be used by the VM driver as well.
opt<bool> BasicBlockProfiling(
    "basic-block-profiling",
    init(false),
    desc("Enable basic block profiling (HBC only)"));

opt<bool>
    EnableEval("enable-eval", init(true), desc("Enable support for eval()"));

// This is normally a compiler option, but it also applies to strings given
// to eval or the Function constructor.
opt<bool> VerifyIR(
    "verify-ir",
#ifdef HERMES_SLOW_DEBUG
    init(true),
#else
    init(false),
    Hidden,
#endif
    desc("Verify the IR after creating it"),
    cat(CompilerCategory));

opt<bool> EmitAsyncBreakCheck(
    "emit-async-break-check",
    desc("Emit instruction to check async break request"),
    init(false),
    cat(CompilerCategory));

opt<bool> AllowFunctionToString(
    "allow-function-to-string",
    init(false),
    desc("Enables Function.toString() to return source-code when available"));

opt<bool> OptimizedEval(
    "optimized-eval",
    desc("Turn on compiler optimizations in eval."),
    init(false));

static list<std::string> IncludeGlobals(
    "include-globals",
    desc("Include the definitions of global properties (can be "
         "specified more than once)"),
    value_desc("filename"));

enum BytecodeFormatKind {
  HBC,
};

// Enable Debug Options to be specified on the command line
static opt<BytecodeFormatKind> BytecodeFormat(
    "target",
    init(HBC),
    desc("Set the bytecode format:"),
    values(clEnumVal(HBC, "Emit HBC bytecode (default)")),
    cat(CompilerCategory));

static opt<std::string> BytecodeOutputFilename(
    "out",
    desc("Output file name"),
    cat(CompilerCategory));

static opt<std::string> BytecodeManifestFilename(
    "bytecode-output-manifest",
    init("manifest.json"),
    desc(
        "Name of the manifest file generated when compiling multiple segments to bytecode"),
    cat(CompilerCategory));

/// Emit debug info for every instruction instead of just the throwing ones.
static opt<bool> EmitDebugInfo(
    "g",
    desc("Emit debug info for all instructions"),
    cat(CompilerCategory));

static opt<std::string> InputSourceMap(
    "source-map",
    desc("Specify a matching source map for the input JS file"),
    cat(CompilerCategory));

static opt<bool> OutputSourceMap(
    "output-source-map",
    desc("Emit a source map to the output filename with .map extension"),
    cat(CompilerCategory));

static opt<bool> DumpOperandRegisters(
    "dump-operand-registers",
    desc("Dump registers assigned to instruction operands"),
    cat(CompilerCategory));

static opt<bool> DumpUseList(
    "dump-instr-uselist",
    desc("Print the use list if the instruction has any users."),
    init(false),
    cat(CompilerCategory));

static opt<bool> DumpSourceLocation(
    "dump-source-location",
    desc("Print source location information in IR or AST dumps."),
    cat(CompilerCategory));

static opt<bool> IncludeEmptyASTNodes(
    "Xinclude-empty-ast-nodes",
    desc("Print all AST nodes, including nodes that are hidden when empty."),
    Hidden,
    cat(CompilerCategory));

static opt<bool> DumpBetweenPasses(
    "Xdump-between-passes",
    init(false),
    Hidden,
    desc("Print IR after every optimization pass"),
    cat(CompilerCategory));

#ifndef NDEBUG

static opt<bool> LexerOnly(
    "Xlexer-only",
    desc("Only run the lexer on the input (debug builds only)"),
    Hidden,
    cat(CompilerCategory));

#endif

static opt<int> MaxDiagnosticWidth(
    "max-diagnostic-width",
    llvh::cl::desc("Preferred diagnostic maximum width"),
    llvh::cl::init(0),
    cat(CompilerCategory));

static opt<bool> EnableCPO(
    "enable-cpo",
    desc("Enable constant property optimizations"),
    init(false),
    cat(CompilerCategory));

static opt<bool> EnableUMO(
    "enable-umo",
    desc("Enable uncalled method optimizations"),
    init(false),
    cat(CompilerCategory));

static opt<BundlerKind> EnableCrossModuleCLA(
    "enable-xm",
    desc("Enable cross module CLA, if doing CLA"),
    cl::values(
        clEnumValN(BundlerKind::none, "none", "no cross-module optimization"),
        clEnumValN(
            BundlerKind::metromin,
            "metromin",
            "Minified metro bundling")),
    cat(CompilerCategory));

static opt<bool> CommonJS(
    "commonjs",
    desc("Use CommonJS modules"),
    init(false),
    cat(CompilerCategory));

#if HERMES_PARSE_JSX
static opt<bool>
    JSX("parse-jsx", desc("Parse JSX"), init(false), cat(CompilerCategory));
#endif

#if HERMES_PARSE_FLOW
static opt<bool> ParseFlow(
    "parse-flow",
    desc("Parse Flow"),
    init(false),
    cat(CompilerCategory));
#endif

static CLFlag StaticRequire(
    'f',
    "static-require",
    false,
    "resolving of CommonJS require() calls at compile time",
    CompilerCategory);

static opt<unsigned> ErrorLimit(
    "ferror-limit",
    desc("Maximum number of errors (0 means unlimited)"),
    init(20),
    cat(CompilerCategory));

static CLFlag Werror(
    'W',
    "error",
    false,
    "Treat all warnings as errors",
    CompilerCategory);

static opt<bool> DisableAllWarnings(
    "w",
    desc("Disable all warnings"),
    init(false),
    cat(CompilerCategory));

static CLFlag UndefinedVariableWarning(
    'W',
    "undefined-variable",
    true,
    "Do not warn when an undefined variable is referenced.",
    CompilerCategory);

static opt<bool> ReusePropCache(
    "reuse-prop-cache",
    desc("Reuse property cache entries for same property name"),
    init(true));

static CLFlag
    Inline('f', "inline", true, "inlining of functions", CompilerCategory);

static CLFlag Outline(
    'f',
    "outline",
    false,
    "IR outlining to reduce code size",
    CompilerCategory);

static CLFlag StripFunctionNames(
    'f',
    "strip-function-names",
    false,
    "Strip function names to reduce string table size",
    CompilerCategory);

static CLFlag EnableTDZ(
    'f',
    "enable-tdz",
    true,
    "Enable TDZ checks for let/const",
    CompilerCategory);

static opt<bool> OutliningPlaceNearCaller(
    "outline-near-caller",
    init(OutliningSettings{}.placeNearCaller),
    desc("Place outlined functions near callers instead of at the end"),
    Hidden,
    cat(CompilerCategory));

static opt<unsigned> OutliningMaxRounds(
    "outline-max-rounds",
    init(OutliningSettings{}.maxRounds),
    desc("Maximum number of outlining rounds to perform"),
    Hidden,
    cat(CompilerCategory));

static opt<unsigned> OutliningMinLength(
    "outline-min-length",
    init(OutliningSettings{}.minLength),
    desc("Minimum number of instructions to consider outlining"),
    Hidden,
    cat(CompilerCategory));

static opt<unsigned> OutliningMinParameters(
    "outline-min-params",
    init(OutliningSettings{}.minParameters),
    desc("Minimum number of parameters in outlined functions"),
    Hidden,
    cat(CompilerCategory));

static opt<unsigned> OutliningMaxParameters(
    "outline-max-params",
    init(OutliningSettings{}.maxParameters),
    desc("Maximum number of parameters in outlined functions"),
    Hidden,
    cat(CompilerCategory));

static CLFlag DirectEvalWarning(
    'W',
    "direct-eval",
    true,
    "Warning when attempting a direct (local) eval",
    CompilerCategory);

static opt<std::string> BaseBytecodeFile(
    "base-bytecode",
    llvh::cl::desc("input base bytecode for delta optimizing mode"),
    llvh::cl::init(""),
    cat(CompilerCategory));

static opt<unsigned> PadFunctionBodiesPercent(
    "pad-function-bodies-percent",
    desc(
        "Add this much garbage after each function body (relative to its size)."),
    init(0),
    Hidden,
    cat(CompilerCategory));

static opt<bool> InstrumentIR(
    "instrument",
    desc("Instrument code for dynamic analysis"),
    init(false),
    Hidden,
    cat(CompilerCategory));
} // namespace cl

namespace {

struct ModuleInSegment {
  /// Index of the module, to be used as the ID when generating IR.
  uint32_t id;

  /// Input source file. May be a JavaScript source file or an HBC file.
  std::unique_ptr<llvh::MemoryBuffer> file;

  /// SourceMap file. nullptr if not specified by the user.
  std::unique_ptr<llvh::MemoryBuffer> sourceMap;
};

/// Encodes a list of files that are part of a given segment.
using SegmentTableEntry = std::vector<ModuleInSegment>;

/// Mapping from segment index to the file buffers in that segment.
/// For a given table, table[i][j] is the j-indexed file in segment i.
/// Use an std::map to ensure that the order of iteration is guaranteed here,
/// allowing the assumption that the segments have strictly increasing
/// module IDs. The entry point must be found at table[0][0].
/// If multiple segments or multiple input files are not being used,
/// the only input will be at table[0][0].
using SegmentTable = std::map<uint32_t, SegmentTableEntry>;

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

/// Read a file from \p path relative to the root of the zip file \p zip
/// into a memory buffer. Print error messages to llvh::errs().
/// \param zip the zip file to read from (must not be null).
/// \param path the path in the zip file, must be null-terminated.
/// \return the read file, nullptr on error.
std::unique_ptr<llvh::MemoryBuffer>
memoryBufferFromZipFile(zip_t *zip, const char *path, bool silent = false) {
  assert(zip && "zip file must not be null");
  int result = 0;

  result = zip_entry_open(zip, path);
  if (result == -1) {
    if (!silent) {
      llvh::errs() << "Zip error reading " << path << ": File does not exist\n";
    }
    return nullptr;
  }

  size_t size = zip_entry_size(zip);

  // Read data from the file, ensuring null termination of the data.
  std::unique_ptr<llvh::MemoryBuffer> buf =
      llvh::WritableMemoryBuffer::getNewMemBuffer(size, path);
  zip_entry_noallocread(zip, const_cast<char *>(buf->getBufferStart()), size);
  zip_entry_close(zip);

  return buf;
}

/// Manage an output file safely.
class OutputStream {
 public:
  /// Creates an empty object.
  OutputStream() : os_(nullptr) {}
  /// Create an object which initially holds the \p defaultStream.
  OutputStream(llvh::raw_ostream &defaultStream) : os_(&defaultStream) {}

  ~OutputStream() {
    discard();
  }

  /// Replaces the stream with an open stream to a temporary file
  /// named based on \p fileName.  This method will write error
  /// messages, if any, to llvh::errs().  This method can only be
  /// called once on an object.  \return true if the temp file was
  /// created and false otherwise.  If the object is destroyed without
  /// close() being called, the temp file is removed.
  bool open(llvh::Twine fileName, llvh::sys::fs::OpenFlags openFlags) {
    assert(!fdos_ && "OutputStream::open() can be called only once.");

    // Newer versions of llvm have a safe createUniqueFile overload
    // which takes OpenFlags.  Hermes's llvm doesn't, so we have to do
    // it this way, which is a hypothetical race.
    std::error_code EC = llvh::sys::fs::getPotentiallyUniqueFileName(
        fileName + ".%%%%%%", tempName_);
    if (EC) {
      llvh::errs() << "Failed to get temp file for " << fileName << ": "
                   << EC.message() << '\n';
      return false;
    }

    fdos_ = llvh::make_unique<raw_fd_ostream>(tempName_, EC, openFlags);
    if (EC) {
      llvh::errs() << "Failed to open file " << tempName_ << ": "
                   << EC.message() << '\n';
      fdos_.reset();
      return false;
    }
    os_ = fdos_.get();
    fileName_ = fileName.str();
    return true;
  }

  /// If a temporary file was created, it is renamed to \p fileName.
  /// If renaming fails, it will be deleted.  This method will write
  /// error messages, if any, to llvh::errs().  \return true if a temp
  /// file was never created or was renamed here; or false otherwise.
  bool close() {
    if (!fdos_) {
      return true;
    }
    fdos_->close();
    fdos_.reset();
    std::error_code EC = llvh::sys::fs::rename(tempName_, fileName_);
    if (EC) {
      llvh::errs() << "Failed to write file " << fileName_ << ": "
                   << EC.message() << '\n';
      llvh::sys::fs::remove(tempName_);
      return false;
    }
    return true;
  }

  /// If a temporary file was created, it is deleted.
  void discard() {
    if (!fdos_) {
      return;
    }

    fdos_->close();
    fdos_.reset();
    llvh::sys::fs::remove(tempName_);
  }

  raw_ostream &os() {
    assert(os_ && "OutputStream never initialized");
    return *os_;
  }

 private:
  llvh::raw_ostream *os_;
  llvh::SmallString<32> tempName_;
  std::unique_ptr<raw_fd_ostream> fdos_;
  std::string fileName_;
};

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
  if (cl::MaxDiagnosticWidth < 0) {
    result.preferredMaxErrorWidth = SourceErrorOutputOptions::UnlimitedWidth;
  } else if (cl::MaxDiagnosticWidth > 0) {
    result.preferredMaxErrorWidth = static_cast<size_t>(cl::MaxDiagnosticWidth);
  }
  return result;
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

  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));
  if (sourceMap != nullptr && sourceMapTranslator != nullptr) {
    sourceMapTranslator->addSourceMap(fileBufId, std::move(sourceMap));
  }

  auto mode = parser::FullParse;

  if (context->isLazyCompilation()) {
    if (!parser::JSParser::preParseBuffer(
            *context, fileBufId, useStaticBuiltinDetected)) {
      return nullptr;
    }
    mode = parser::LazyParse;
  }

  Optional<ESTree::ProgramNode *> parsedJs;

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

  if (cl::StaticBuiltins == cl::StaticBuiltinSetting::AutoDetect) {
    context->setStaticBuiltinOptimization(useStaticBuiltinDetected);
  }

  if (wrapCJSModule) {
    parsedAST =
        hermes::wrapCJSModule(context, cast<ESTree::ProgramNode>(parsedAST));
    if (!parsedAST) {
      return nullptr;
    }
  }

  if (cl::DumpTarget == DumpAST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cl::PrettyJSON /* pretty */,
        cl::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                 : ESTreeDumpMode::HideEmpty,
        cl::DumpSourceLocation ? &context->getSourceErrorManager() : nullptr);
  }

  if (!hermes::sem::validateAST(*context, semCtx, parsedAST)) {
    return nullptr;
  }

  if (cl::DumpTarget == DumpTransformedAST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cl::PrettyJSON /* pretty */,
        cl::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                 : ESTreeDumpMode::HideEmpty,
        cl::DumpSourceLocation ? &context->getSourceErrorManager() : nullptr);
  }

  return parsedAST;
}

/// Apply custom logic for flag initialization.
void setFlagDefaults() {
  // We haven't been given any file names; just use "-", which acts as stdin.
  if (cl::InputFilenames.empty()) {
    cl::InputFilenames.push_back("-");
  }

  // If bytecode mode is not explicitly specified, check the input extension.
  // of the input file.
  if (!cl::BytecodeMode && cl::InputFilenames.size() == 1 &&
      llvh::sys::path::extension(cl::InputFilenames[0]) == ".hbc") {
    cl::BytecodeMode = true;
  }

  if (cl::LazyCompilation && cl::OptimizationLevel > cl::OptLevel::Og) {
    cl::OptimizationLevel = cl::OptLevel::Og;
  }
}

/// Validate command line flags.
/// \return true if the flags are valid, false if not. On a false return, an
/// error will have been printed to stderr.
bool validateFlags() {
  // Helper to print an error message and return false.
  bool errored = false;
  auto err = [&errored](const char *msg) {
    if (!errored) {
      llvh::errs() << msg << '\n';
      errored = true;
    }
  };

  // Validate strict vs non strict mode.
  if (cl::NonStrictMode && cl::StrictMode) {
    err("Error! Cannot use both -strict and -non-strict");
  }

  // Validate bytecode output file.
  if (cl::DumpTarget == EmitBundle && cl::BytecodeOutputFilename.empty() &&
      oscompat::isatty(STDOUT_FILENO)) {
    // To skip this check and trash the terminal, use -out /dev/stdout.
    err("Refusing to write binary bundle to terminal.\n"
        "Specify output file with -out filename.");
  }

  // Validate lazy compilation flags.
  if (cl::LazyCompilation) {
    if (cl::BytecodeFormat != cl::BytecodeFormatKind::HBC)
      err("-lazy only works with -target=HBC");
    if (cl::OptimizationLevel > cl::OptLevel::Og)
      err("-lazy does not work with -O");
    if (cl::BytecodeMode) {
      err("-lazy doesn't make sense with bytecode");
    }
    if (!cl::CustomOptimize.empty()) {
      // We don't currently pass these around to be applied later.
      err("-lazy doesn't allow custom optimizations");
    }
    if (cl::CommonJS) {
      err("-lazy doesn't support CommonJS modules");
    }
  }

  // Validate flags for more than one input file.
  if (cl::InputFilenames.size() > 1) {
    if (cl::BytecodeMode)
      err("Hermes can only load one bytecode file.");
    if (cl::BytecodeFormat != cl::BytecodeFormatKind::HBC)
      err("Multiple files are only supported with HBC.");
    if (!cl::CommonJS)
      err("Multiple files must use CommonJS modules.");
  }

  // Validate source map output flags.
  if (cl::OutputSourceMap) {
    if (cl::BytecodeOutputFilename.empty())
      err("-output-source-map requires -out to be set");
    if (cl::BytecodeFormat != cl::BytecodeFormatKind::HBC)
      err("-output-source-map requires HBC target");
    if (cl::DumpTarget != EmitBundle)
      err("-output-source-map only works with -emit-binary");
  }

  // Validate bytecode dumping flags.
  if (cl::BytecodeMode && cl::DumpTarget != Execute) {
    if (cl::BytecodeFormat != cl::BytecodeFormatKind::HBC)
      err("Only Hermes bytecode files may be dumped");
    if (cl::DumpTarget != DumpBytecode)
      err("You can only dump bytecode for HBC bytecode file.");
  }

#ifndef HERMES_ENABLE_IR_INSTRUMENTATION
  if (cl::InstrumentIR) {
    err("Instrumentation is requested, but support is not compiled in");
  }
#endif

  return !errored;
}

/// Create a Context, respecting the command line flags.
/// \return the Context.
std::shared_ptr<Context> createContext(
    std::unique_ptr<Context::ResolutionTable> resolutionTable,
    std::vector<Context::SegmentRange> segmentRanges) {
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.enableTDZ = cl::EnableTDZ;
  codeGenOpts.dumpOperandRegisters = cl::DumpOperandRegisters;
  codeGenOpts.dumpUseList = cl::DumpUseList;
  codeGenOpts.dumpSourceLocation = cl::DumpSourceLocation;
  codeGenOpts.dumpIRBetweenPasses = cl::DumpBetweenPasses;
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC) {
    codeGenOpts.unlimitedRegisters = false;
  }
  codeGenOpts.instrumentIR = cl::InstrumentIR;

  OptimizationSettings optimizationOpts;
  optimizationOpts.constantPropertyOptimizations = cl::EnableCPO;
  optimizationOpts.uncalledMethodOptimizations = cl::EnableUMO;
  optimizationOpts.crossModuleClosureAnalysis =
      cl::EnableCrossModuleCLA.getValue();

  // Enable aggressiveNonStrictModeOptimizations if the target is HBC.
  optimizationOpts.aggressiveNonStrictModeOptimizations =
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC;

  optimizationOpts.inlining = cl::OptimizationLevel != cl::OptLevel::O0 &&
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC && cl::Inline;
  optimizationOpts.outlining =
      cl::OptimizationLevel != cl::OptLevel::O0 && cl::Outline;

  optimizationOpts.outliningSettings.placeNearCaller =
      cl::OutliningPlaceNearCaller;
  optimizationOpts.outliningSettings.maxRounds = cl::OutliningMaxRounds;
  optimizationOpts.outliningSettings.minLength = cl::OutliningMinLength;
  optimizationOpts.outliningSettings.minParameters = cl::OutliningMinParameters;
  optimizationOpts.outliningSettings.maxParameters = cl::OutliningMaxParameters;

  optimizationOpts.reusePropCache = cl::ReusePropCache;

  // When the setting is auto-detect, we will set the correct value after
  // parsing.
  optimizationOpts.staticBuiltins =
      cl::StaticBuiltins == cl::StaticBuiltinSetting::ForceOn;
  optimizationOpts.staticRequire = cl::StaticRequire;

  auto context = std::make_shared<Context>(
      codeGenOpts,
      optimizationOpts,
      std::move(resolutionTable),
      std::move(segmentRanges));

  // Default is non-strict mode.
  context->setStrictMode(!cl::NonStrictMode && cl::StrictMode);
  context->setEnableEval(cl::EnableEval);
  context->getSourceErrorManager().setOutputOptions(guessErrorOutputOptions());
  context->getSourceErrorManager().setWarningsAreErrors(cl::Werror);
  context->getSourceErrorManager().setWarningStatus(
      Warning::UndefinedVariable, cl::UndefinedVariableWarning);
  context->getSourceErrorManager().setWarningStatus(
      Warning::DirectEval, cl::DirectEvalWarning);
  if (cl::DisableAllWarnings)
    context->getSourceErrorManager().disableAllWarnings();
  context->getSourceErrorManager().setErrorLimit(cl::ErrorLimit);

  if (cl::CommonJS) {
    context->setUseCJSModules(true);
  }

#if HERMES_PARSE_JSX
  if (cl::JSX) {
    context->setParseJSX(true);
  }
#endif

#if HERMES_PARSE_FLOW
  if (cl::ParseFlow) {
    context->setParseFlow(true);
  }
#endif

  if (cl::EmitDebugInfo) {
    context->setDebugInfoSetting(DebugInfoSetting::ALL);
  } else if (cl::OutputSourceMap) {
    context->setDebugInfoSetting(DebugInfoSetting::SOURCE_MAP);
  } else {
    context->setDebugInfoSetting(DebugInfoSetting::THROWING);
  }
  context->setEmitAsyncBreakCheck(cl::EmitAsyncBreakCheck);
  return context;
}

/// Parse \p file into a JSON value.
/// \param alloc the allocator to use for JSON parsing.
/// \return a metadata JSONObject allocated in the user-specified allocator,
/// nullptr on failure. All error messages are printed to stderr.
::hermes::parser::JSONValue *parseJSONFile(
    std::unique_ptr<llvh::MemoryBuffer> &file,
    ::hermes::parser::JSLexer::Allocator &alloc) {
  using namespace ::hermes::parser;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  JSONParser parser(factory, *file, sm);
  auto root = parser.parse();
  if (!root) {
    llvh::errs()
        << "Failed to parse metadata: Unable to parse a valid JSON object\n";
    return nullptr;
  }
  return root.getValue();
}

/// Given the root path to the directory or zip file, the file name, and
/// a zip struct that represents the zip file if it's a zip, return
/// the memory buffer of the file content.
std::unique_ptr<llvh::MemoryBuffer> getFileFromDirectoryOrZip(
    zip_t *zip,
    llvh::StringRef rootPath,
    llvh::Twine fileName,
    bool silent = false) {
  llvh::SmallString<32> path{};
  if (!zip) {
    llvh::sys::path::append(path, llvh::sys::path::Style::posix, rootPath);
  }
  llvh::sys::path::append(path, llvh::sys::path::Style::posix, fileName);
  llvh::sys::path::remove_dots(path, false, llvh::sys::path::Style::posix);
  return zip ? memoryBufferFromZipFile(zip, path.c_str(), silent)
             : memoryBufferFromFile(path, false, silent);
}

/// Read input filenames from the given path and populate the files in \p
/// fileBufs.
/// In case of failure, ensure fileBufs is empty.
/// \param inputPath the path to the directory or zip file containing metadata
/// and files.
/// \param[out] fileBufs table of file buffers.
/// \param alloc the allocator to use for JSON parsing of metadata.
/// \return a pointer to the metadata JSON object, nullptr on failure.
::hermes::parser::JSONObject *readInputFilenamesFromDirectoryOrZip(
    llvh::StringRef inputPath,
    SegmentTable &fileBufs,
    std::vector<Context::SegmentRange> &segmentRanges,
    ::hermes::parser::JSLexer::Allocator &alloc,
    struct zip_t *zip) {
  auto metadataBuf = getFileFromDirectoryOrZip(zip, inputPath, "metadata.json");
  if (!metadataBuf) {
    llvh::errs()
        << "Failed to read metadata: Input must contain a metadata.json file\n";
    return nullptr;
  }

  auto *metadataVal = parseJSONFile(metadataBuf, alloc);
  if (!metadataVal) {
    // parseJSONFile prints any error messages.
    return nullptr;
  }

  // Pull data from the metadata JSON object into C++ data structures.
  // The metadata format is documented at doc/Modules.md.

  auto *metadata = dyn_cast<parser::JSONObject>(metadataVal);
  if (!metadata) {
    llvh::errs() << "Metadata must be a JSON object\n";
    return nullptr;
  }

  auto *segments =
      llvh::dyn_cast_or_null<parser::JSONObject>(metadata->get("segments"));
  if (!segments) {
    llvh::errs() << "Metadata must contain segment information\n";
    return nullptr;
  }

  uint32_t moduleIdx = 0;

  for (auto it : *segments) {
    Context::SegmentRange range;
    if (it.first->str().getAsInteger(10, range.segment)) {
      // getAsInteger returns true to signal error.
      llvh::errs()
          << "Metadata segment indices must be unsigned integers: Found "
          << it.first->str() << '\n';
      return nullptr;
    }

    auto *segment = llvh::dyn_cast_or_null<parser::JSONArray>(it.second);
    if (!segment) {
      llvh::errs() << "Metadata segment information must be an array\n";
      return nullptr;
    }

    range.first = moduleIdx;

    SegmentTableEntry segmentBufs{};
    for (auto val : *segment) {
      auto *relPath = llvh::dyn_cast_or_null<parser::JSONString>(val);
      if (!relPath) {
        llvh::errs() << "Segment paths must be strings\n";
        return nullptr;
      }
      auto fileBuf = getFileFromDirectoryOrZip(zip, inputPath, relPath->str());
      if (!fileBuf) {
        return nullptr;
      }
      auto mapBuf = getFileFromDirectoryOrZip(
          zip, inputPath, llvh::Twine(relPath->str(), ".map"), true);
      // mapBuf is optional, so simply pass it through if it's null.
      segmentBufs.push_back(
          {moduleIdx++, std::move(fileBuf), std::move(mapBuf)});
    }

    range.last = moduleIdx - 1;

    auto emplaceRes = fileBufs.emplace(range.segment, std::move(segmentBufs));
    if (!emplaceRes.second) {
      llvh::errs() << "Duplicate segment entry in metadata: " << range.segment
                   << "\n";
      return nullptr;
    }

    segmentRanges.push_back(std::move(range));
  }

  return metadata;
}

/// A map from segment ID to the deserialized base bytecode of that segment.
using BaseBytecodeMap =
    llvh::DenseMap<uint32_t, std::unique_ptr<hbc::BCProviderFromBuffer>>;

/// Load the base bytecode provider from given file buffer \fileBuf.
/// \return the base bytecode provider, or nullptr if an error happened.
std::unique_ptr<hbc::BCProviderFromBuffer> loadBaseBytecodeProvider(
    std::unique_ptr<llvh::MemoryBuffer> fileBuf) {
  if (!fileBuf) {
    llvh::errs() << "Unable to read from base bytecode file.\n";
    return nullptr;
  }
  // Transfer ownership to an owned memory buffer.
  auto ownedBuf = llvh::make_unique<OwnedMemoryBuffer>(std::move(fileBuf));
  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::move(ownedBuf));
  if (!ret.first) {
    llvh::errs() << "Error deserializing base bytecode: " << ret.second;
    return nullptr;
  }
  return std::move(ret.first);
}

/// Read the base bytecode provider map from either a directory or a zip file.
/// This is used when commonjs is used and we need to optimize for delta
/// bytecode updates. A metadata.hbc.json file is expected to exist in the
/// directory or zip, which contains a map from segment ID to the file name of
/// the base bytecode file for that segment.
/// Returns whether the read succeeded.
bool readBaseBytecodeFromDirectoryOrZip(
    BaseBytecodeMap &map,
    llvh::StringRef inputPath,
    ::hermes::parser::JSLexer::Allocator &alloc,
    struct zip_t *zip) {
  auto manifestBuf = getFileFromDirectoryOrZip(zip, inputPath, "manifest.json");
  if (!manifestBuf) {
    llvh::errs()
        << "Failed to read manifest: Input must contain a manifest.json file\n";
    return false;
  }

  auto *manifestVal = parseJSONFile(manifestBuf, alloc);
  if (!manifestVal) {
    // parseJSONFile prints any error messages.
    return false;
  }

  // Pull data from the manifest JSON object into C++ data structures.
  // The manifest format is documented at doc/Modules.md.

  auto *manifest = dyn_cast<parser::JSONArray>(manifestVal);
  if (!manifest) {
    llvh::errs() << "Manifest must be a JSON array.\n";
    return false;
  }

  for (auto it : *manifest) {
    auto *segment = llvh::dyn_cast_or_null<parser::JSONObject>(it);
    if (!segment) {
      llvh::errs() << "Each segment entry must be a JSON object.\n";
      return false;
    }
    llvh::StringRef prefix{"hbc-seg-"};
    auto *flavor =
        llvh::dyn_cast_or_null<parser::JSONString>(segment->get("flavor"));
    if (!flavor || flavor->str().size() <= prefix.size() ||
        !flavor->str().startswith(prefix)) {
      llvh::errs() << "flavor must be a string that prefix a number with "
                   << prefix << ".\n";
      return false;
    }
    uint32_t segmentID;
    if (flavor->str().substr(prefix.size()).getAsInteger(10, segmentID)) {
      // getAsInteger returns true to signal error.
      llvh::errs() << "flavor must be a string that prefix a number with "
                   << prefix << ". Found " << flavor->str() << '\n';
      return false;
    }

    auto *location =
        llvh::dyn_cast_or_null<parser::JSONString>(segment->get("location"));
    if (!location) {
      llvh::errs() << "Segment bytecode location must be a string.\n";
      return false;
    }

    auto fileBuf = getFileFromDirectoryOrZip(zip, inputPath, location->str());
    if (!fileBuf) {
      llvh::errs() << "Base bytecode does not exist: " << location->str()
                   << ".\n";
      return false;
    }

    auto bcProvider = loadBaseBytecodeProvider(std::move(fileBuf));
    if (!bcProvider) {
      return false;
    }

    map[segmentID] = std::move(bcProvider);
  }

  return true;
}

/// Read base bytecode and returns whether it succeeded.
bool readBaseBytecodeMap(
    BaseBytecodeMap &map,
    llvh::StringRef inputPath,
    ::hermes::parser::JSLexer::Allocator &alloc) {
  assert(!inputPath.empty() && "No base bytecode file requested");
  struct zip_t *zip = zip_open(inputPath.data(), 0, 'r');

  if (llvh::sys::fs::is_directory(inputPath) || zip) {
    auto ret = readBaseBytecodeFromDirectoryOrZip(map, inputPath, alloc, zip);
    if (zip) {
      zip_close(zip);
    }
    return ret;
  }
  auto bcProvider = loadBaseBytecodeProvider(memoryBufferFromFile(inputPath));
  if (!bcProvider) {
    return false;
  }
  map[0] = std::move(bcProvider);
  return true;
}

/// Read a resolution table. Given a file name, it maps every require string
/// to the actual file which must be required.
/// Prints out error messages to stderr in case of failure.
/// \param metadata the full metadata JSONObject. Contains "resolutionTable".
/// \return the resolution table read from the metadata, nullptr on failure.
std::unique_ptr<Context::ResolutionTable> readResolutionTable(
    ::hermes::parser::JSONObject *metadata) {
  assert(metadata && "No metadata to read resolution table from");

  using namespace ::hermes::parser;

  auto result = hermes::make_unique<Context::ResolutionTable>();

  JSONObject *resolutionTable =
      llvh::dyn_cast_or_null<JSONObject>(metadata->get("resolutionTable"));
  if (!resolutionTable) {
    return nullptr;
  }

  for (auto itFile : *resolutionTable) {
    llvh::StringRef filename =
        llvh::sys::path::remove_leading_dotslash(itFile.first->str());
    JSONObject *fileTable = llvh::dyn_cast<JSONObject>(itFile.second);
    if (!fileTable) {
      llvh::errs() << "Invalid value in resolution table for file: " << filename
                   << '\n';
      return nullptr;
    }
    Context::ResolutionTableEntry map{};
    for (auto itEntry : *fileTable) {
      JSONString *src = itEntry.first;
      JSONString *dstJSON = llvh::dyn_cast<JSONString>(itEntry.second);
      if (!dstJSON) {
        llvh::errs() << "Invalid value in resolution table: " << filename << '@'
                     << src->str() << '\n';
        return nullptr;
      }
      llvh::StringRef dst =
          llvh::sys::path::remove_leading_dotslash(dstJSON->str());
      auto emplaceRes = map.try_emplace(src->str(), dst);
      if (!emplaceRes.second) {
        llvh::errs() << "Duplicate entry in resolution table: " << filename
                     << '@' << src->str() << '\n';
        return nullptr;
      }
    }
    auto emplaceRes = result->try_emplace(filename, std::move(map));
    if (!emplaceRes.second) {
      llvh::errs() << "Duplicate entry in resolution table for file: "
                   << filename << '\n';
      return nullptr;
    }
  }

  return result;
}

/// Generate IR for CJS modules into the Module \p M for the source files in
/// \p fileBufs. Treat the first element in fileBufs as the entry point.
/// \param sourceMapGen the parsed versions of the input source maps,
/// in the order in which the files were compiled.
/// \return true on success, false on error, in which case an error will be
/// printed.
bool generateIRForSourcesAsCJSModules(
    Module &M,
    sem::SemContext &semCtx,
    const DeclarationFileListTy &declFileList,
    SegmentTable fileBufs,
    SourceMapGenerator *sourceMapGen) {
  auto context = M.shareContext();
  llvh::SmallString<64> rootPath{fileBufs[0][0].file->getBufferIdentifier()};
  llvh::sys::path::remove_filename(rootPath, llvh::sys::path::Style::posix);

  // Construct a MemoryBuffer for our global entry point.
  llvh::SmallString<64> entryPointFilename{
      fileBufs[0][0].file->getBufferIdentifier()};
  llvh::sys::path::replace_path_prefix(
      entryPointFilename, rootPath, "./", llvh::sys::path::Style::posix);

  // The top-level function is empty, due to the fact that it is not intended to
  // be executed. The Runtime must choose and execute the correct entry point
  // (main) module, from which other modules may be `require`d.
  auto globalMemBuffer = llvh::MemoryBuffer::getMemBufferCopy("", "<global>");

  auto *globalAST = parseJS(context, semCtx, std::move(globalMemBuffer));
  generateIRFromESTree(globalAST, &M, declFileList, {});

  std::vector<std::unique_ptr<SourceMap>> inputSourceMaps{};
  inputSourceMaps.push_back(nullptr);
  std::vector<std::string> sources{"<global>"};

  Function *topLevelFunction = M.getTopLevelFunction();
  for (auto &entry : fileBufs) {
    for (ModuleInSegment &moduleInSegment : entry.second) {
      auto &fileBuf = moduleInSegment.file;
      llvh::SmallString<64> filename{fileBuf->getBufferIdentifier()};
      if (sourceMapGen) {
        sources.push_back(fileBuf->getBufferIdentifier());
      }
      llvh::sys::path::replace_path_prefix(
          filename, rootPath, "./", llvh::sys::path::Style::posix);
      // TODO: use sourceMapTranslator for CJS module.
      auto *ast = parseJS(
          context,
          semCtx,
          std::move(fileBuf),
          /*sourceMap*/ nullptr,
          /*sourceMapTranslator*/ nullptr,
          /*wrapCJSModule*/ true);
      if (!ast) {
        return false;
      }
      if (cl::DumpTarget < DumpIR) {
        continue;
      }
      generateIRForCJSModule(
          cast<ESTree::FunctionExpressionNode>(ast),
          moduleInSegment.id,
          llvh::sys::path::remove_leading_dotslash(filename),
          &M,
          topLevelFunction,
          declFileList);
      if (moduleInSegment.sourceMap) {
        auto inputMap = SourceMapParser::parse(*moduleInSegment.sourceMap);
        if (!inputMap) {
          // parse() returns nullptr on failure and reports its own errors.
          return false;
        }
        inputSourceMaps.push_back(std::move(inputMap));
      } else {
        inputSourceMaps.push_back(nullptr);
      }
    }
  }

  if (sourceMapGen) {
    for (const auto &source : sources) {
      sourceMapGen->addSource(source);
    }
    sourceMapGen->setInputSourceMaps(std::move(inputSourceMaps));
  }

  return true;
}

/// Disassemble the BCProvider \p bytecode to the output stream specified by the
/// command line flags. \return a CompileResult for the disassembly.
CompileResult disassembleBytecode(std::unique_ptr<hbc::BCProvider> bytecode) {
  assert(
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC &&
      "validateFlags() should enforce only HBC files may be disassembled");

  OutputStream fileOS(llvh::outs());
  if (!cl::BytecodeOutputFilename.empty() &&
      !fileOS.open(cl::BytecodeOutputFilename, F_Text)) {
    return OutputFileError;
  }

  hbc::DisassemblyOptions disassemblyOptions = cl::PrettyDisassemble
      ? hbc::DisassemblyOptions::Pretty
      : hbc::DisassemblyOptions::None;
  hbc::BytecodeDisassembler disassembler(std::move(bytecode));
  disassembler.setOptions(disassemblyOptions);
  disassembler.disassemble(fileOS.os());
  if (!fileOS.close())
    return OutputFileError;
  return Success;
}

/// Process the bytecode file given in \p fileBuf. Disassemble it if requested,
/// otherwise return it as the CompileResult artifact. \return a compile result.
CompileResult processBytecodeFile(std::unique_ptr<llvh::MemoryBuffer> fileBuf) {
  assert(cl::BytecodeMode && "Input files must be bytecode");
  assert(
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC &&
      "Only HBC bytecode format may be loaded");

  bool isMmapped =
      fileBuf->getBufferKind() == llvh::MemoryBuffer::MemoryBuffer_MMap;
  char *bufStart = const_cast<char *>(fileBuf->getBufferStart());
  size_t bufSize = fileBuf->getBufferSize();
  std::string filename = fileBuf->getBufferIdentifier();

  std::unique_ptr<hbc::BCProviderFromBuffer> bytecode;
  auto buffer = llvh::make_unique<OwnedMemoryBuffer>(std::move(fileBuf));
  auto ret =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(std::move(buffer));
  if (!ret.first) {
    llvh::errs() << "Error deserializing bytecode: " << ret.second;
    return InputFileError;
  }
  bytecode = std::move(ret.first);
  if (cl::DumpTarget != Execute) {
    assert(
        cl::DumpTarget == DumpBytecode &&
        "validateFlags() should enforce bytecode files "
        "may only have a dump target of bytecode");
    return disassembleBytecode(std::move(bytecode));
  } else {
    CompileResult result{Success};
    result.bytecodeProvider = std::move(bytecode);
    result.bytecodeBufferInfo =
        BytecodeBufferInfo{isMmapped, bufStart, bufSize, std::move(filename)};
    return result;
  }
}

/// Compile the given module \p M with the options \p genOptions in a form
/// suitable for immediate execution (i.e. no expectation of persistence).
/// \return the compile result.
CompileResult generateBytecodeForExecution(
    Module &M,
    const BytecodeGenerationOptions &genOptions) {
  std::shared_ptr<Context> context = M.shareContext();
  CompileResult result{Success};
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC) {
    result.bytecodeProvider = hbc::BCProviderFromSrc::createBCProviderFromSrc(
        hbc::generateBytecodeModule(&M, M.getTopLevelFunction(), genOptions));

  } else {
    llvm_unreachable("Invalid bytecode kind for execution");
    result = InvalidFlags;
  }
  return result;
}

/// Compile the module \p M with the options \p genOptions, serializing the
/// result to \p OS. If sourceMapGenOrNull is not null, populate it.
/// \return the CompileResult.
/// The corresponding base bytecode will be removed from \baseBytecodeMap.
CompileResult generateBytecodeForSerialization(
    raw_ostream &OS,
    Module &M,
    const BytecodeGenerationOptions &genOptions,
    const SHA1 &sourceHash,
    OptValue<Context::SegmentRange> range,
    SourceMapGenerator *sourceMapGenOrNull,
    BaseBytecodeMap &baseBytecodeMap) {
  // Serialize the bytecode to the file.
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC) {
    std::unique_ptr<hbc::BCProviderFromBuffer> baseBCProvider = nullptr;
    auto itr = baseBytecodeMap.find(range ? range->segment : 0);
    if (itr != baseBytecodeMap.end()) {
      baseBCProvider = std::move(itr->second);
      // We want to erase it from the map because unique_ptr can only
      // have one owner.
      baseBytecodeMap.erase(itr);
    }
    auto bytecodeModule = hbc::generateBytecode(
        &M,
        OS,
        genOptions,
        sourceHash,
        range,
        sourceMapGenOrNull,
        std::move(baseBCProvider));

    if (cl::DumpTarget == DumpBytecode) {
      disassembleBytecode(hbc::BCProviderFromSrc::createBCProviderFromSrc(
          std::move(bytecodeModule)));
    }
  } else {
    llvm_unreachable("Invalid bytecode kind");
  }
  return Success;
}

/// Compiles the given files \p fileBufs with the context \p context,
/// respecting the command line flags.
/// \return a CompileResult containing the compilation status and artifacts.
CompileResult processSourceFiles(
    std::shared_ptr<Context> context,
    SegmentTable fileBufs) {
  assert(!fileBufs.empty() && "Need at least one file to compile");
  assert(context && "Need a context to compile using");
  assert(!cl::BytecodeMode && "Input files must not be bytecode");

  llvh::SHA1 hasher;
  for (const auto &entry : fileBufs) {
    for (const auto &fileAndMap : entry.second) {
      const auto &file = fileAndMap.file;
      hasher.update(
          llvh::StringRef(file->getBufferStart(), file->getBufferSize()));
    }
  }
  auto rawFinalHash = hasher.final();
  SHA1 sourceHash{};
  assert(
      rawFinalHash.size() == SHA1_NUM_BYTES && "Incorrect length of SHA1 hash");
  std::copy(rawFinalHash.begin(), rawFinalHash.end(), sourceHash.begin());
#ifndef NDEBUG
  if (cl::LexerOnly) {
    unsigned count = 0;
    for (auto &entry : fileBufs) {
      for (auto &fileAndMap : entry.second) {
        parser::JSLexer jsLexer(
            std::move(fileAndMap.file),
            context->getSourceErrorManager(),
            context->getAllocator());
        while (jsLexer.advance()->getKind() != parser::TokenKind::eof)
          ++count;
      }
    }
    llvh::outs() << count << " tokens lexed\n";
    return Success;
  }
#endif

  // A list of parsed global definition files.
  DeclarationFileListTy declFileList;

  // Load the runtime library.
  std::unique_ptr<llvh::MemoryBuffer> libBuffer;
  switch (cl::BytecodeFormat) {
    case cl::BytecodeFormatKind::HBC:
      libBuffer = llvh::MemoryBuffer::getMemBuffer(libhermes);
      break;
  }
  if (!loadGlobalDefinition(*context, std::move(libBuffer), declFileList)) {
    return LoadGlobalsFailed;
  }

  // Load the global property definitions.
  for (const auto &fileName : cl::IncludeGlobals) {
    auto fileBuf = memoryBufferFromFile(fileName);
    if (!fileBuf)
      return InputFileError;
    LLVM_DEBUG(
        llvh::dbgs() << "Parsing global definitions from " << fileName << '\n');
    if (!loadGlobalDefinition(*context, std::move(fileBuf), declFileList)) {
      return LoadGlobalsFailed;
    }
  }

  // Enable lazy compilation if requested.
  context->setLazyCompilation(cl::LazyCompilation);

  // Allows Function.toString() to return original source code. As with lazy
  // compilation this requires source buffers to be retained after compilation.
  context->setAllowFunctionToStringWithRuntimeSource(cl::AllowFunctionToString);

  // Create the source map if requested.
  llvh::Optional<SourceMapGenerator> sourceMapGen{};
  if (cl::OutputSourceMap) {
    sourceMapGen = SourceMapGenerator{};
  }

  Module M(context);
  sem::SemContext semCtx{};

  if (context->getUseCJSModules()) {
    // Allow the IR generation function to populate inputSourceMaps to ensure
    // proper source map ordering.
    if (!generateIRForSourcesAsCJSModules(
            M,
            semCtx,
            declFileList,
            std::move(fileBufs),
            sourceMapGen ? &*sourceMapGen : nullptr)) {
      return ParsingFailed;
    }
    if (cl::DumpTarget < DumpIR) {
      return Success;
    }
  } else {
    if (sourceMapGen) {
      for (const auto &filename : cl::InputFilenames) {
        sourceMapGen->addSource(filename == "-" ? "<stdin>" : filename);
      }
    }

    auto &mainFileBuf = fileBufs[0][0];
    std::unique_ptr<SourceMap> sourceMap{nullptr};
    if (mainFileBuf.sourceMap) {
      sourceMap = SourceMapParser::parse(*mainFileBuf.sourceMap);
      if (!sourceMap) {
        // parse() returns nullptr on failure and reports its own errors.
        return InputFileError;
      }
    }

    auto sourceMapTranslator =
        std::make_shared<SourceMapTranslator>(context->getSourceErrorManager());
    context->getSourceErrorManager().setTranslator(sourceMapTranslator);
    ESTree::NodePtr ast = parseJS(
        context,
        semCtx,
        std::move(mainFileBuf.file),
        std::move(sourceMap),
        sourceMapTranslator);
    if (!ast) {
      return ParsingFailed;
    }
    if (cl::DumpTarget < DumpIR) {
      return Success;
    }
    generateIRFromESTree(ast, &M, declFileList, {});
  }

  // Bail out if there were any errors. We can't ensure that the module is in
  // a valid state.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return ParsingFailed;
  }

  // Run custom optimization pipeline.
  if (!cl::CustomOptimize.empty()) {
    std::vector<std::string> opts(
        cl::CustomOptimize.begin(), cl::CustomOptimize.end());
    if (!runCustomOptimizationPasses(M, opts)) {
      llvh::errs() << "Invalid custom optimizations selected.\n\n"
                   << PassManager::getCustomPassText();
      return InvalidFlags;
    }
  } else {
    switch (cl::OptimizationLevel) {
      case cl::OptLevel::O0:
        runNoOptimizationPasses(M);
        break;
      case cl::OptLevel::Og:
        runDebugOptimizationPasses(M);
        break;
      case cl::OptLevel::OMax:
        runFullOptimizationPasses(M);
        break;
    }
  }

  // In dbg builds, verify the module before we emit bytecode.
  if (cl::VerifyIR) {
    bool failedVerification = verifyModule(M, &llvh::errs());
    if (failedVerification) {
      M.dump();
      return VerificationFailed;
    }
    assert(!failedVerification && "Module verification failed!");
  }

  if (cl::DumpTarget == DumpIR) {
    M.dump();
    return Success;
  }

#ifndef NDEBUG
  if (cl::DumpTarget == ViewCFG) {
    M.viewGraph();
    return Success;
  }
#endif

  BytecodeGenerationOptions genOptions{cl::DumpTarget};
  genOptions.optimizationEnabled = cl::OptimizationLevel > cl::OptLevel::Og;
  genOptions.prettyDisassemble = cl::PrettyDisassemble;
  genOptions.basicBlockProfiling = cl::BasicBlockProfiling;
  // The static builtin setting should be set correctly after command line
  // options parsing and js parsing. Set the bytecode header flag here.
  genOptions.staticBuiltinsEnabled = context->getStaticBuiltinOptimization();
  genOptions.padFunctionBodiesPercent = cl::PadFunctionBodiesPercent;

  // If the user requests to output a source map, then do not also emit debug
  // info into the bytecode.
  genOptions.stripDebugInfoSection = cl::OutputSourceMap;

  genOptions.stripFunctionNames = cl::StripFunctionNames;

  // If the dump target is None, return bytecode in an executable form.
  if (cl::DumpTarget == Execute) {
    assert(
        !sourceMapGen &&
        "validateFlags() should enforce no source map output for execution");
    return generateBytecodeForExecution(M, genOptions);
  }

  BaseBytecodeMap baseBytecodeMap;
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC &&
      !cl::BaseBytecodeFile.empty()) {
    if (!readBaseBytecodeMap(
            baseBytecodeMap, cl::BaseBytecodeFile, context->getAllocator())) {
      return InputFileError;
    }
  }

  CompileResult result{Success};
  StringRef base = cl::BytecodeOutputFilename;
  if (context->getSegmentRanges().size() < 2) {
    OutputStream fileOS{llvh::outs()};
    if (!base.empty() && !fileOS.open(base, F_None)) {
      return OutputFileError;
    }
    auto result = generateBytecodeForSerialization(
        fileOS.os(),
        M,
        genOptions,
        sourceHash,
        llvh::None,
        sourceMapGen ? sourceMapGen.getPointer() : nullptr,
        baseBytecodeMap);
    if (result.status != Success) {
      return result;
    }
    if (!fileOS.close())
      return OutputFileError;
  } else {
    OutputStream manifestOS{llvh::nulls()};
    if (!base.empty() && !cl::BytecodeManifestFilename.empty()) {
      llvh::SmallString<32> manifestPath = llvh::sys::path::parent_path(base);
      llvh::sys::path::append(manifestPath, cl::BytecodeManifestFilename);
      if (!manifestOS.open(manifestPath, F_Text))
        return OutputFileError;
    }
    JSONEmitter manifest{manifestOS.os(), /* pretty */ true};
    manifest.openArray();

    for (const auto &range : context->getSegmentRanges()) {
      std::string filename = base.str();
      if (range.segment != 0) {
        filename += "." + oscompat::to_string(range.segment);
      }
      std::string flavor = "hbc-seg-" + oscompat::to_string(range.segment);

      OutputStream fileOS{llvh::outs()};
      if (!base.empty() && !fileOS.open(filename, F_None)) {
        return OutputFileError;
      }
      auto segResult = generateBytecodeForSerialization(
          fileOS.os(),
          M,
          genOptions,
          sourceHash,
          range,
          sourceMapGen ? sourceMapGen.getPointer() : nullptr,
          baseBytecodeMap);
      if (segResult.status != Success) {
        return segResult;
      }
      if (!fileOS.close())
        return OutputFileError;

      // Add to the manifest.
      manifest.openDict();
      manifest.emitKeyValue("resource", llvh::sys::path::filename(base));
      manifest.emitKeyValue("flavor", flavor);
      manifest.emitKeyValue("location", llvh::sys::path::filename(filename));

      manifest.closeDict();
    }
    manifest.closeArray();

    if (!manifestOS.close()) {
      return OutputFileError;
    }

    result = Success;
  }

  // Output the source map if requested.
  if (cl::OutputSourceMap) {
    OutputStream OS;
    if (!OS.open(base.str() + ".map", F_Text))
      return OutputFileError;
    sourceMapGen->outputAsJSON(OS.os());
    if (!OS.close())
      return OutputFileError;
  }

  return result;
}

/// Print the Hermes version to the stream \p s, outputting the \p vmStr (which
/// may be empty).
/// \param features when true, print the list of enabled features.
void printHermesVersion(
    llvh::raw_ostream &s,
    const char *vmStr = "",
    bool features = true) {
  s << "Hermes JavaScript compiler" << vmStr << ".\n"
#ifdef HERMES_RELEASE_VERSION
    << "  Hermes release version: " << HERMES_RELEASE_VERSION << "\n"
#endif
    << "  HBC bytecode version: " << hermes::hbc::BYTECODE_VERSION << "\n"
    << "\n";
  if (features) {
    s << "  Features:\n"
#ifdef HERMES_ENABLE_DEBUGGER
      << "    Debugger\n"
#endif
      << "    Zip file input\n";
  }
}

} // namespace

namespace hermes {
namespace driver {

void printHermesCompilerVMVersion(llvh::raw_ostream &s) {
  printHermesVersion(s, " and Virtual Machine");
}
void printHermesCompilerVersion(llvh::raw_ostream &s) {
  printHermesVersion(s);
}

OutputFormatKind outputFormatFromCommandLineOptions() {
  return cl::DumpTarget;
}

CompileResult compileFromCommandLineOptions() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
  if (cl::PrintStats)
    hermes::EnableStatistics();
#endif

  // Set up and validate flags.
  setFlagDefaults();
  if (!validateFlags())
    return InvalidFlags;

  // Load input files.
  SegmentTable fileBufs{};

  // Allocator for the metadata table.
  ::hermes::parser::JSLexer::Allocator metadataAlloc;

  // Resolution table in metadata, null if none could be read.
  std::unique_ptr<Context::ResolutionTable> resolutionTable = nullptr;

  // Segment table in metadata.
  std::vector<Context::SegmentRange> segmentRanges;

  // Attempt to open the first file as a Zip file.
  struct zip_t *zip = zip_open(cl::InputFilenames[0].data(), 0, 'r');

  if (llvh::sys::fs::is_directory(cl::InputFilenames[0]) || zip) {
    ::hermes::parser::JSONObject *metadata =
        readInputFilenamesFromDirectoryOrZip(
            cl::InputFilenames[0], fileBufs, segmentRanges, metadataAlloc, zip);

    if (zip) {
      zip_close(zip);
    }
    if (!metadata) {
      return InputFileError;
    }

    resolutionTable = readResolutionTable(metadata);
  } else {
    // If we aren't reading from a dir or a zip, we have only one segment.
    Context::SegmentRange range;
    range.first = 0;
    range.last = cl::InputFilenames.size();
    range.segment = 0;
    segmentRanges.push_back(std::move(range));

    uint32_t id = 0;
    SegmentTableEntry entry{};
    for (const std::string &filename : cl::InputFilenames) {
      auto fileBuf = memoryBufferFromFile(filename, true);
      if (!fileBuf)
        return InputFileError;
      entry.push_back({id++, std::move(fileBuf), nullptr});
    }

    // Read input source map if available.
    if (!cl::InputSourceMap.empty()) {
      // TODO: support multiple JS sources from command line.
      if (cl::InputFilenames.size() != 1) {
        llvh::errs()
            << "Error: only support single js file for input source map."
            << '\n';
        return InvalidFlags;
      }
      assert(entry.size() == 1 && "Can't have more than one entries.");
      entry[0].sourceMap =
          memoryBufferFromFile(cl::InputSourceMap, /*stdinOk*/ false);
    }

    fileBufs.emplace(0, std::move(entry));
  }

  if (cl::BytecodeMode) {
    assert(
        fileBufs.size() == 1 && fileBufs[0].size() == 1 &&
        "validateFlags() should enforce exactly one bytecode input file");
    return processBytecodeFile(std::move(fileBufs[0][0].file));
  } else {
    std::shared_ptr<Context> context =
        createContext(std::move(resolutionTable), std::move(segmentRanges));
    return processSourceFiles(context, std::move(fileBufs));
  }
}
} // namespace driver
} // namespace hermes
