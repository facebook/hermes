/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/CompilerDriver/CompilerDriver.h"

#include "hermes/AST/CommonJS.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/AST2JS/AST2JS.h"
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
#include "hermes/Support/OptValue.h"
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
using llvh::cl::ValuesClass;

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
        clEnumValN(DumpJS, "dump-js", "Dump the AST as JS"),
        clEnumValN(
            DumpTransformedJS,
            "dump-transformed-js",
            "Dump the transformed AST as JS after validation"),
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

static opt<bool> Pretty(
    "pretty",
    init(true),
    desc("Pretty print JSON, JS or disassembled bytecode"),
    cat(CompilerCategory));

static llvh::cl::alias _PrettyJSON(
    "pretty-json",
    desc("Alias for --pretty"),
    Hidden,
    llvh::cl::aliasopt(Pretty));
static llvh::cl::alias _PrettyDisassemble(
    "pretty-disassemble",
    desc("Alias for --pretty"),
    Hidden,
    llvh::cl::aliasopt(Pretty));

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
    desc("Force fully lazy compilation"),
    cat(CompilerCategory));

static opt<bool> EagerCompilation(
    "eager",
    init(false),
    desc("Force fully eager compilation"),
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

enum class DebugLevel { g0, g1, g2, g3 };

static cl::opt<DebugLevel> DebugInfoLevel(
    cl::desc("Choose debug info level:"),
    cl::init(DebugLevel::g1),
    cl::values(
        clEnumValN(DebugLevel::g3, "g", "Equivalent to -g3"),
        clEnumValN(DebugLevel::g0, "g0", "Do not emit debug info"),
        clEnumValN(DebugLevel::g1, "g1", "Emit location info for backtraces"),
        clEnumValN(
            DebugLevel::g2,
            "g2",
            "Emit location info for all instructions"),
        clEnumValN(DebugLevel::g3, "g3", "Emit full info for debugging")),
    cl::cat(CompilerCategory));

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

static opt<LocationDumpMode> DumpSourceLocation(
    "dump-source-location",
    desc("Print source location information in IR or AST dumps."),
    init(LocationDumpMode::None),
    values(
        clEnumValN(
            LocationDumpMode::LocAndRange,
            "both",
            "Print both source location and byte range"),
        clEnumValN(LocationDumpMode::Loc, "loc", "Print only source location"),
        clEnumValN(LocationDumpMode::Range, "range", "Print only byte range")),
    cat(CompilerCategory));

static opt<bool> IncludeEmptyASTNodes(
    "Xinclude-empty-ast-nodes",
    desc("Print all AST nodes, including nodes that are hidden when empty."),
    Hidden,
    cat(CompilerCategory));

static opt<bool> IncludeRawASTProp(
    "Xinclude-raw-ast-prop",
    desc("Print the 'raw' AST property, when available."),
    init(true),
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

#if HERMES_PARSE_TS
static opt<bool> ParseTS(
    "parse-ts",
    desc("Parse TypeScript"),
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

static ValuesClass warningValues{
#define WARNING_CATEGORY_HIDDEN(name, specifier, description) \
  clEnumValN(Warning::name, specifier, description),
#include "hermes/Support/Warnings.def"
};

static list<hermes::Warning> Werror(
    llvh::cl::ValueOptional,
    "Werror",
    value_desc("category"),
    desc(
        "Treat all warnings as errors, or treat warnings of a particular category as errors"),
    warningValues,
    cat(CompilerCategory));

static list<hermes::Warning> Wnoerror(
    llvh::cl::ValueOptional,
    "Wno-error",
    value_desc("category"),
    Hidden,
    desc(
        "Treat no warnings as errors, or treat warnings of a particular category as warnings"),
    warningValues,
    cat(CompilerCategory));

static opt<bool> DisableAllWarnings(
    "w",
    desc("Disable all warnings"),
    init(false),
    cat(CompilerCategory));

static opt<bool> ReusePropCache(
    "reuse-prop-cache",
    desc("Reuse property cache entries for same property name"),
    init(true));

static CLFlag
    Inline('f', "inline", true, "inlining of functions", CompilerCategory);

static CLFlag StripFunctionNames(
    'f',
    "strip-function-names",
    false,
    "Strip function names to reduce string table size",
    CompilerCategory);

static opt<bool> EnableTDZ(
    "Xenable-tdz",
    init(false),
    Hidden,
    desc("UNSUPPORTED: Enable TDZ checks for let/const"),
    cat(CompilerCategory));

#define WARNING_CATEGORY(name, specifier, description) \
  static CLFlag name##Warning(                         \
      'W', specifier, true, description, CompilerCategory);
#include "hermes/Support/Warnings.def"

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

static CLFlag UseUnsafeIntrinsics(
    'f',
    "unsafe-intrinsics",
    false,
    "Recognize and lower Asm.js/Wasm unsafe compiler intrinsics.",
    CompilerCategory);
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

/// Mapping from file name to module ID. File names are relative to the input
/// root path (directory / zip file) and normalized with
/// remove_leading_dotslash.
using ModuleIDsTable = llvh::DenseMap<llvh::StringRef, uint32_t>;

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
  if (result < 0) {
    if (!silent) {
      llvh::errs() << "Zip error: reading " << path << ": "
                   << zip_strerror(result) << "\n";
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

    fdos_ = std::make_unique<raw_fd_ostream>(tempName_, EC, openFlags);
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
        cl::Pretty /* pretty */,
        cl::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                 : ESTreeDumpMode::HideEmpty,
        context->getSourceErrorManager(),
        cl::DumpSourceLocation,
        cl::IncludeRawASTProp ? ESTreeRawProp::Include
                              : ESTreeRawProp::Exclude);
    return parsedAST;
  }
  if (cl::DumpTarget == DumpJS) {
    hermes::generateJS(llvh::outs(), parsedAST, cl::Pretty /* pretty */);
    return parsedAST;
  }

  if (!hermes::sem::validateAST(*context, semCtx, parsedAST)) {
    return nullptr;
  }

  if (cl::DumpTarget == DumpTransformedAST) {
    hermes::dumpESTreeJSON(
        llvh::outs(),
        parsedAST,
        cl::Pretty /* pretty */,
        cl::IncludeEmptyASTNodes ? ESTreeDumpMode::DumpAll
                                 : ESTreeDumpMode::HideEmpty,
        context->getSourceErrorManager(),
        cl::DumpSourceLocation,
        cl::IncludeRawASTProp ? ESTreeRawProp::Include
                              : ESTreeRawProp::Exclude);
  }
  if (cl::DumpTarget == DumpTransformedJS) {
    hermes::generateJS(llvh::outs(), parsedAST, cl::Pretty /* pretty */);
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

  if (cl::OutputSourceMap && cl::DebugInfoLevel < cl::DebugLevel::g2) {
    cl::DebugInfoLevel = cl::DebugLevel::g2;
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

  if (cl::LazyCompilation && cl::EagerCompilation) {
    err("Can't specify both -lazy and -eager");
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

/// Apply the -Werror, -Wno-error, -Werror=<category> and -Wno-error=<category>
/// flags to \c sm from left to right.
static void setWarningsAreErrorsFromFlags(SourceErrorManager &sm) {
  std::vector<Warning>::iterator yesIt = cl::Werror.begin();
  std::vector<Warning>::iterator noIt = cl::Wnoerror.begin();
  // Argument positions are indices into argv and start at 1 (or 2 if there's a
  // subcommand). See llvh::cl::CommandLineParser::ParseCommandLineOptions().
  // In this loop, position 0 represents the lack of a value.
  unsigned noPos = 0, yesPos = 0;
  while (true) {
    if (noIt != cl::Wnoerror.end()) {
      noPos = cl::Wnoerror.getPosition(noIt - cl::Wnoerror.begin());
    } else {
      noPos = 0;
    }
    if (yesIt != cl::Werror.end()) {
      yesPos = cl::Werror.getPosition(yesIt - cl::Werror.begin());
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
std::shared_ptr<Context> createContext(
    std::unique_ptr<Context::ResolutionTable> resolutionTable,
    std::vector<uint32_t> segments) {
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.enableTDZ = cl::EnableTDZ;
  codeGenOpts.dumpOperandRegisters = cl::DumpOperandRegisters;
  codeGenOpts.dumpUseList = cl::DumpUseList;
  codeGenOpts.dumpSourceLocation =
      cl::DumpSourceLocation != LocationDumpMode::None;
  codeGenOpts.dumpIRBetweenPasses = cl::DumpBetweenPasses;
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC) {
    codeGenOpts.unlimitedRegisters = false;
  }
  codeGenOpts.instrumentIR = cl::InstrumentIR;

  OptimizationSettings optimizationOpts;

  // Enable aggressiveNonStrictModeOptimizations if the target is HBC.
  optimizationOpts.aggressiveNonStrictModeOptimizations =
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC;

  optimizationOpts.inlining = cl::OptimizationLevel != cl::OptLevel::O0 &&
      cl::BytecodeFormat == cl::BytecodeFormatKind::HBC && cl::Inline;

  optimizationOpts.reusePropCache = cl::ReusePropCache;

  // When the setting is auto-detect, we will set the correct value after
  // parsing.
  optimizationOpts.staticBuiltins =
      cl::StaticBuiltins == cl::StaticBuiltinSetting::ForceOn;
  optimizationOpts.staticRequire = cl::StaticRequire;

  optimizationOpts.useUnsafeIntrinsics = cl::UseUnsafeIntrinsics;

  auto context = std::make_shared<Context>(
      codeGenOpts,
      optimizationOpts,
      std::move(resolutionTable),
      std::move(segments));

  // Default is non-strict mode.
  context->setStrictMode(!cl::NonStrictMode && cl::StrictMode);
  context->setEnableEval(cl::EnableEval);
  context->getSourceErrorManager().setOutputOptions(guessErrorOutputOptions());

  setWarningsAreErrorsFromFlags(context->getSourceErrorManager());

#define WARNING_CATEGORY(name, specifier, description) \
  context->getSourceErrorManager().setWarningStatus(   \
      Warning::name, cl::name##Warning);
#include "hermes/Support/Warnings.def"

  if (cl::DisableAllWarnings)
    context->getSourceErrorManager().disableAllWarnings();
  context->getSourceErrorManager().setErrorLimit(cl::ErrorLimit);

  {
    // Set default lazy mode using defaults from CompileFlags to keep it in one
    // place.
    hermes::hbc::CompileFlags defaultFlags{};
    context->setPreemptiveFileCompilationThreshold(
        defaultFlags.preemptiveFileCompilationThreshold);
    context->setPreemptiveFunctionCompilationThreshold(
        defaultFlags.preemptiveFunctionCompilationThreshold);
  }

  if (cl::EagerCompilation || cl::DumpTarget == EmitBundle ||
      cl::OptimizationLevel > cl::OptLevel::Og) {
    // Make sure nothing is lazy
    context->setLazyCompilation(false);
  } else if (cl::LazyCompilation) {
    // Make sure everything is lazy
    context->setLazyCompilation(true);
    context->setPreemptiveFileCompilationThreshold(0);
    context->setPreemptiveFunctionCompilationThreshold(0);
  } else {
    // By default with no optimization, use lazy compilation for "large" files
    context->setLazyCompilation(true);
  }

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
    context->setParseFlow(ParseFlowSetting::ALL);
  }
#endif

#if HERMES_PARSE_TS
  if (cl::ParseTS) {
    context->setParseTS(true);
  }
#endif

  if (cl::DebugInfoLevel >= cl::DebugLevel::g3) {
    context->setDebugInfoSetting(DebugInfoSetting::ALL);
  } else if (cl::DebugInfoLevel == cl::DebugLevel::g2) {
    context->setDebugInfoSetting(DebugInfoSetting::SOURCE_MAP);
  } else {
    // -g1 or -g0. If -g0, we'll strip debug info later.
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

/// Read a module IDs table. It maps every file name to its unique global module
/// ID. Prints out error messages to stderr in case of failure.
/// \param metadata the full metadata JSONObject. Contains "moduleIDs".
/// \return the module IDs table read from the metadata, None on failure.
llvh::Optional<ModuleIDsTable> readModuleIDs(
    ::hermes::parser::JSONObject *metadata) {
  assert(metadata && "No metadata to read module IDs from");

  using namespace ::hermes::parser;

  JSONObject *moduleIDs =
      llvh::dyn_cast_or_null<JSONObject>(metadata->get("moduleIDs"));
  if (!moduleIDs) {
    return llvh::None;
  }

  ModuleIDsTable result;

  llvh::DenseMap<uint32_t, llvh::StringRef> filenameByModuleID;

  for (auto itFile : *moduleIDs) {
    llvh::StringRef filename =
        llvh::sys::path::remove_leading_dotslash(itFile.first->str());
    JSONNumber *moduleID = llvh::dyn_cast<JSONNumber>(itFile.second);
    if (!moduleID) {
      llvh::errs() << "Invalid value in module ID table for file: " << filename
                   << '\n';
      return llvh::None;
    }
    uint32_t uintModuleID = (uint32_t)moduleID->getValue();
    if (uintModuleID != moduleID->getValue()) {
      llvh::errs() << "Module IDs must be unsigned integers: Found "
                   << moduleID->getValue() << '\n';
      return llvh::None;
    }
    auto emplaceRes = result.try_emplace(filename, uintModuleID);
    if (!emplaceRes.second) {
      llvh::errs() << "Duplicate entry in module ID table for file: "
                   << filename << '\n';
      return llvh::None;
    }
    auto inverseRes = filenameByModuleID.try_emplace(uintModuleID, filename);
    if (!inverseRes.second) {
      llvh::errs() << "Duplicate entry in module ID table for ID: "
                   << uintModuleID << '\n';
      return llvh::None;
    }
  }

  return result;
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
    std::vector<uint32_t> &segmentIDs,
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

  // Module IDs in metadata, None if none could be read.
  auto externalModuleIDs = readModuleIDs(metadata);
  // Module ID table used for assigning auto-incrementing module IDs if we
  // don't have external module IDs.
  ModuleIDsTable automaticModuleIDs;
  uint32_t nextAutomaticModuleID = 0;

  for (auto it : *segments) {
    uint32_t segmentID;
    if (it.first->str().getAsInteger(10, segmentID)) {
      // getAsInteger returns true to signal error.
      llvh::errs() << "Metadata segment IDs must be unsigned integers: Found "
                   << it.first->str() << '\n';
      return nullptr;
    }

    auto *segment = llvh::dyn_cast_or_null<parser::JSONArray>(it.second);
    if (!segment) {
      llvh::errs() << "Metadata segment information must be an array\n";
      return nullptr;
    }

    SegmentTableEntry segmentBufs{};
    for (auto val : *segment) {
      auto *relPath = llvh::dyn_cast_or_null<parser::JSONString>(val);
      if (!relPath) {
        llvh::errs() << "Segment paths must be strings\n";
        return nullptr;
      }
      auto filename = llvh::sys::path::remove_leading_dotslash(relPath->str());
      auto fileBuf = getFileFromDirectoryOrZip(zip, inputPath, filename);
      if (!fileBuf) {
        return nullptr;
      }
      auto mapBuf = getFileFromDirectoryOrZip(
          zip, inputPath, llvh::Twine(filename, ".map"), true);
      uint32_t moduleID;
      if (externalModuleIDs.hasValue()) {
        auto itr = externalModuleIDs->find(filename);
        if (itr == externalModuleIDs->end()) {
          llvh::errs() << "Module is missing in externalModuleIDs: " << filename
                       << "\n";
          return nullptr;
        }
        moduleID = itr->second;
      } else {
        auto emplaceRes =
            automaticModuleIDs.try_emplace(filename, nextAutomaticModuleID);
        if (emplaceRes.second) {
          ++nextAutomaticModuleID;
        }
        moduleID = emplaceRes.first->second;
      }
      // mapBuf is optional, so simply pass it through if it's null.
      segmentBufs.push_back({moduleID, std::move(fileBuf), std::move(mapBuf)});
    }

    auto emplaceRes = fileBufs.emplace(segmentID, std::move(segmentBufs));
    if (!emplaceRes.second) {
      llvh::errs() << "Duplicate segment entry in metadata: " << segment
                   << "\n";
      return nullptr;
    }

    segmentIDs.push_back(segmentID);
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
  auto ownedBuf = std::make_unique<OwnedMemoryBuffer>(std::move(fileBuf));
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

  auto result = std::make_unique<Context::ResolutionTable>();

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
/// \p fileBufs if IR generation was requested. Otherwise, just parse the files.
/// Treat the first element in fileBufs as the entry point.
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
  bool generateIR = cl::DumpTarget >= DumpIR;

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
  if (generateIR) {
    // If we aren't planning to do anything with the IR,
    // don't attempt to generate it.
    generateIRFromESTree(globalAST, &M, declFileList, {});
  }

  std::vector<std::unique_ptr<SourceMap>> inputSourceMaps{};
  inputSourceMaps.push_back(nullptr);
  std::vector<std::string> sources{"<global>"};

  Function *topLevelFunction = generateIR ? M.getTopLevelFunction() : nullptr;
  llvh::DenseSet<uint32_t> generatedModuleIDs;
  for (auto &entry : fileBufs) {
    uint32_t segmentID = entry.first;
    for (ModuleInSegment &moduleInSegment : entry.second) {
      auto &fileBuf = moduleInSegment.file;
      llvh::SmallString<64> filename{fileBuf->getBufferIdentifier()};

      if (sourceMapGen && generatedModuleIDs.count(moduleInSegment.id) == 0) {
        // This is the first time we're generating IR for this module.
        sources.push_back(fileBuf->getBufferIdentifier());
        if (moduleInSegment.sourceMap) {
          SourceErrorManager sm;
          auto inputMap =
              SourceMapParser::parse(*moduleInSegment.sourceMap, sm);
          if (!inputMap) {
            // parse() returns nullptr on failure and reports its own errors.
            return false;
          }
          inputSourceMaps.push_back(std::move(inputMap));
        } else {
          inputSourceMaps.push_back(nullptr);
        }
      }

      generatedModuleIDs.insert(moduleInSegment.id);

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
      if (!generateIR) {
        continue;
      }
      generateIRForCJSModule(
          cast<ESTree::FunctionExpressionNode>(ast),
          segmentID,
          moduleInSegment.id,
          llvh::sys::path::remove_leading_dotslash(filename),
          &M,
          topLevelFunction,
          declFileList);
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

  hbc::DisassemblyOptions disassemblyOptions = cl::Pretty
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
  auto buffer = std::make_unique<OwnedMemoryBuffer>(std::move(fileBuf));
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
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGenOrNull,
    BaseBytecodeMap &baseBytecodeMap) {
  // Serialize the bytecode to the file.
  if (cl::BytecodeFormat == cl::BytecodeFormatKind::HBC) {
    std::unique_ptr<hbc::BCProviderFromBuffer> baseBCProvider = nullptr;
    auto itr = baseBytecodeMap.find(segment ? *segment : 0);
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
        segment,
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
      SourceErrorManager sm;
      sourceMap = SourceMapParser::parse(*mainFileBuf.sourceMap, sm);
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

  // Bail out if there were any errors during optimization.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return OptimizationFailed;
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
  genOptions.prettyDisassemble = cl::Pretty;
  genOptions.basicBlockProfiling = cl::BasicBlockProfiling;
  // The static builtin setting should be set correctly after command line
  // options parsing and js parsing. Set the bytecode header flag here.
  genOptions.staticBuiltinsEnabled = context->getStaticBuiltinOptimization();
  genOptions.padFunctionBodiesPercent = cl::PadFunctionBodiesPercent;

  // If the user requests to output a source map, then do not also emit debug
  // info into the bytecode.
  genOptions.stripDebugInfoSection =
      cl::OutputSourceMap || cl::DebugInfoLevel == cl::DebugLevel::g0;

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
  if (context->getSegments().size() < 2) {
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

    for (const auto segment : context->getSegments()) {
      std::string filename = base.str();
      if (segment != 0) {
        filename += "." + std::to_string(segment);
      }
      std::string flavor = "hbc-seg-" + std::to_string(segment);

      OutputStream fileOS{llvh::outs()};
      if (!base.empty() && !fileOS.open(filename, F_None)) {
        return OutputFileError;
      }
      auto segResult = generateBytecodeForSerialization(
          fileOS.os(),
          M,
          genOptions,
          sourceHash,
          segment,
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

  // Segment IDs in metadata.
  std::vector<uint32_t> segments;

  // Attempt to open the first file as a Zip file.
  struct zip_t *zip = zip_open(cl::InputFilenames[0].data(), 0, 'r');

  if (llvh::sys::fs::is_directory(cl::InputFilenames[0]) || zip) {
    ::hermes::parser::JSONObject *metadata =
        readInputFilenamesFromDirectoryOrZip(
            cl::InputFilenames[0], fileBufs, segments, metadataAlloc, zip);

    if (zip) {
      zip_close(zip);
    }
    if (!metadata) {
      return InputFileError;
    }

    resolutionTable = readResolutionTable(metadata);
  } else {
    // If we aren't reading from a dir or a zip, we have only one segment.
    segments.push_back(0);

    uint32_t nextModuleID = 0;
    ModuleIDsTable moduleIDs;
    SegmentTableEntry entry{};
    for (const std::string &filename : cl::InputFilenames) {
      auto fileBuf = memoryBufferFromFile(filename, true);
      if (!fileBuf)
        return InputFileError;
      auto emplaceRes = moduleIDs.try_emplace(filename, nextModuleID);
      auto moduleID = emplaceRes.first->second;
      if (emplaceRes.second) {
        ++nextModuleID;
      }
      entry.push_back({moduleID, std::move(fileBuf), nullptr});
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
        createContext(std::move(resolutionTable), std::move(segments));
    return processSourceFiles(context, std::move(fileBufs));
  }
}
} // namespace driver
} // namespace hermes

#undef DEBUG_TYPE
