/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "compile.h"

#include "OutputStream.h"
#include "config.h"

#include "hermes/BCGen/SH/SH.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"

#define DEBUG_TYPE "shermesc"

using namespace hermes;

namespace {

/// Invoke the backend with the specified options. If the backend generates
/// an error (unlikely, but possible), print the number of errors and return
/// false.
bool invokeBackend(
    Context *context,
    Module &M,
    const BytecodeGenerationOptions &genOptions,
    llvh::raw_ostream &os) {
  assert(
      context->getSourceErrorManager().getErrorCount() == 0 &&
      "backend invocation with non-zero errors");
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }

  sh::generateSH(&M, os, genOptions);

  // Bail out if there were any errors during code generation.
  if (auto N = context->getSourceErrorManager().getErrorCount()) {
    llvh::errs() << "Emitted " << N << " errors. exiting.\n";
    return false;
  }

  return true;
}

/// Derive an output filename from the input filename by removing the path and
/// replacing the input extension with \p newExt, unless for some crazy reason
/// it already happens to be that.
///
/// \param input input filename
/// \param storage storage where the manipulated name is kept
/// \param newExt the new output extension
/// \return a reference to storage
llvh::StringRef deriveFilename(
    llvh::StringRef input,
    llvh::SmallString<32> &storage,
    llvh::StringRef newExt) {
  storage = llvh::sys::path::filename(input);
  if (llvh::sys::path::extension(storage) != newExt)
    llvh::sys::path::replace_extension(storage, newExt);
  else
    storage += newExt;
  return storage;
}

/// Invoke the backend when we are requested to "dump" one of our internal
/// representations. They are written to STDOUT by default, unless and output
/// path is specified.
bool compileToDump(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    llvh::StringRef outputFilename) {
  OutputStream fileOS{};
  // If an output file name is not specified, use STDOUT.
  if (!fileOS.open(
          outputFilename.empty() ? "-" : outputFilename,
          llvh::sys::fs::F_None)) {
    return false;
  }
  if (!invokeBackend(context, M, params.genOptions, fileOS.os()))
    return false;
  return fileOS.close();
}

/// Invoke the backend to generate C.
bool compileToC(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    llvh::StringRef inputFilename,
    llvh::StringRef outputFilename) {
  // If an output file name is not specified, derive it from the input.
  llvh::SmallString<32> outputPathBuf{};
  if (outputFilename.empty())
    outputFilename = deriveFilename(inputFilename, outputPathBuf, ".c");

  OutputStream fileOS{};
  if (!fileOS.open(outputFilename, llvh::sys::fs::F_None))
    return false;
  if (!invokeBackend(context, M, params.genOptions, fileOS.os()))
    return false;
  return fileOS.close();
}

/// Configuration for invoking the C compiler.
struct CCCfg {
  std::string cc;
  std::string cflags;
  std::string ldflags;
  std::string ldlibs;
  std::vector<std::string> hermesLibPath;
  std::vector<std::string> hermesIncludePath;
};

/// Populate CCCfg with overrides from the environment.
void populateCCCfg(CCCfg &cfg) {
  auto init = [](std::string &res, const char *name, const char *defVal) {
    if (const char *t = ::getenv(name))
      res = t;
    else
      res = defVal;
  };

  init(cfg.cc, "CC", "cc");
  init(cfg.cflags, "CFLAGS", "");
  init(cfg.ldflags, "LDFLAGS", "");
  init(cfg.ldlibs, "LDLIBS", "");

  llvh::SmallVector<llvh::StringRef, 2> vec{};
  llvh::StringLiteral(SHERMES_CC_LIB_PATH).split(vec, ':', -1, false);
  for (auto sr : vec)
    cfg.hermesLibPath.push_back(sr.str());

  vec.clear();
  llvh::StringLiteral(SHERMES_CC_INCLUDE_PATH).split(vec, ':', -1, false);
  for (auto sr : vec)
    cfg.hermesIncludePath.push_back(sr.str());
}

// Split arguments separated by whitespace and push them individually into
// `args`. Honor quotation marks.
static void splitArgs(llvh::StringRef str, std::vector<std::string> &args) {
  size_t size = str.size();
  size_t i = 0;
  // The current argument is accumulated here.
  std::string tmp{};
  while (i != size) {
    // Skip spaces
    if (isspace(str[i])) {
      ++i;
      continue;
    }

    tmp.clear();
    do {
      // Quoted sequences are copied without splitting.
      if (str[i] == '\'' || str[i] == '"') {
        size_t closingIndex = str.find(str[i], i + 1);
        if (closingIndex != llvh::StringRef::npos) {
          tmp.append(str.data() + i + 1, closingIndex - i - 1);
          i = closingIndex + 1;
          continue;
        }
      }

      tmp.push_back(str[i]);
      ++i;
    } while (i != size && !isspace(str[i]));
    args.push_back(tmp);
  }
}

/// Invoke the C compiler.
bool invokeCC(
    const ShermesCompileParams &params,
    OutputLevelKind outputLevel,
    llvh::StringRef inputPath,
    llvh::StringRef outputPath) {
  CCCfg cfg;
  populateCCCfg(cfg);

  auto res = llvh::sys::findProgramByName(cfg.cc);
  if (!res) {
    llvh::errs() << cfg.cc << ":" << res.getError().message() << "\n";
    return false;
  }
  llvh::StringRef program = *res;

  std::vector<std::string> args{};

  args.emplace_back(program);

  // Select compilation to asm, obj, binary
  switch (outputLevel) {
    case OutputLevelKind::Asm:
      args.emplace_back("-S");
      break;
    case OutputLevelKind::Obj:
      args.emplace_back("-c");
      break;
    case OutputLevelKind::Executable:
      break;
    default:
      hermes_fatal("unexpected output level");
  }

  // If CFLAGS were specified, they override our optimization level and include
  // path.
  if (cfg.cflags.empty()) {
    switch (params.nativeOptimize) {
      case OptLevel::O0:
        break;
      case OptLevel::Og:
        args.emplace_back("-Og");
        break;
      case OptLevel::Os:
        args.emplace_back("-Os");
        break;
      case OptLevel::OMax:
        args.emplace_back("-O3");
        break;
    }
    for (const auto &s : cfg.hermesIncludePath)
      args.push_back("-I" + s);

    if (params.enableAsserts == ShermesCompileParams::EnableAsserts::off) {
      args.emplace_back("-DNDEBUG");
    }
  } else {
    splitArgs(cfg.cflags, args);
  }

  // Append the library paths and library.
  if (outputLevel == OutputLevelKind::Executable) {
    if (cfg.ldflags.empty()) {
      for (const auto &s : cfg.hermesLibPath)
        args.push_back("-L" + s);

      args.emplace_back("-framework");
      args.emplace_back("CoreFoundation");
      args.emplace_back("-lc++");
    } else {
      splitArgs(cfg.ldflags, args);
    }

    args.emplace_back(
        params.lean == ShermesCompileParams::Lean::on ? "-lhermesvmlean"
                                                      : "-lhermesvm");

    splitArgs(cfg.ldlibs, args);
  }
  args.emplace_back(inputPath);
  args.emplace_back("-o");
  args.emplace_back(outputPath);

  std::vector<llvh::StringRef> refArgs{};
  refArgs.reserve(args.size());
  for (const auto &str : args)
    refArgs.emplace_back(str);

  if (params.verbosity) {
    for (size_t i = 0; i != refArgs.size(); ++i)
      llvh::errs() << (i ? " " : "") << refArgs[i];
    llvh::errs() << "\n";
  }

  std::string errMsg;
  if (llvh::sys::ExecuteAndWait(
          *res, refArgs, llvh::None, {}, 0, 0, &errMsg, nullptr) == 0) {
    return true;
  }

  if (!errMsg.empty())
    llvh::errs() << errMsg << "\n";
  else
    llvh::errs() << program << ": execution failed\n";
  return false;
}

/// Generate C source, then invoke the C compiler to compile it either to .s,
/// .o, or an executable binary.
bool compileFromC(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    OutputLevelKind outputLevel,
    llvh::StringRef inputFilename,
    llvh::StringRef outputFilename) {
  // If an output file name is not specified, derive it from the input.
  llvh::SmallString<32> outputPathBuf{};
  if (outputFilename.empty()) {
    if (outputLevel == OutputLevelKind::Executable) {
      outputFilename = "a.out";
    } else {
      assert(
          outputLevel == OutputLevelKind::Asm ||
          outputLevel == OutputLevelKind::Obj);
      outputFilename = deriveFilename(
          inputFilename,
          outputPathBuf,
          outputLevel == OutputLevelKind::Asm ? ".s" : ".o");
    }
  }

  // Synthesize a temporary file name for the .c file. It needs to have the
  // proper extension ".c". Note that createTemporaryFile() automatically
  // appends the ".".
  llvh::SmallString<32> tmpPath;
  int tmpFD = -1;
  if (auto EC = llvh::sys::fs::createTemporaryFile(
          llvh::sys::path::filename(inputFilename), "c", tmpFD, tmpPath)) {
    llvh::errs() << "Error creating " << tmpPath << ": " << EC.message()
                 << '\n';
    return false;
  }
  // Don't forget to delete the temporary on exit.
  llvh::sys::RemoveFileOnSignal(tmpPath);
  auto removeOnExit = llvh::make_scope_exit([&tmpPath]() {
    llvh::sys::DontRemoveFileOnSignal(tmpPath);
    ::remove(tmpPath.c_str());
  });

  // Emit into the temporary file.
  {
    llvh::raw_fd_ostream os{tmpFD, true};
    if (!invokeBackend(context, M, params.genOptions, os))
      return false;
    os.close();
    if (auto EC = os.error()) {
      llvh::errs() << "Error writing to " << tmpPath << ": " << EC.message()
                   << '\n';
      return false;
    }
  }

  return invokeCC(params, outputLevel, tmpPath, outputFilename);
}

/// Compile to an executable and run it.
bool execute(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    llvh::StringRef inputFilename,
    llvh::ArrayRef<std::string> execArgs) {
  llvh::SmallString<32> tmpPath;
  if (auto EC = llvh::sys::fs::createTemporaryFile(
          llvh::sys::path::filename(inputFilename), {}, tmpPath)) {
    llvh::errs() << "Error creating " << tmpPath << ": " << EC.message()
                 << '\n';
    return false;
  }
  // Don't forget to delete the temporary on exit.
  llvh::sys::RemoveFileOnSignal(tmpPath);
  auto removeOnExit = llvh::make_scope_exit([&tmpPath]() {
    llvh::sys::DontRemoveFileOnSignal(tmpPath);
    ::remove(tmpPath.c_str());
  });

  if (!compileFromC(
          context,
          M,
          params,
          OutputLevelKind::Executable,
          inputFilename,
          tmpPath)) {
    return false;
  }

  llvh::SmallVector<llvh::StringRef, 1> args{};
  args.emplace_back(tmpPath);
  for (const auto &s : execArgs)
    args.emplace_back(s);

  if (params.verbosity) {
    for (size_t i = 0; i != args.size(); ++i)
      llvh::errs() << (i ? " " : "") << args[i];
    llvh::errs() << "\n";
  }

  std::string errMsg;
  if (llvh::sys::ExecuteAndWait(
          tmpPath, args, llvh::None, {}, 0, 0, &errMsg, nullptr) == 0) {
    return true;
  }

  if (!errMsg.empty())
    llvh::errs() << errMsg << "\n";
  return false;
}

} // namespace

bool shermesCompile(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    OutputLevelKind outputLevel,
    llvh::StringRef inputFilename,
    llvh::StringRef outputFilename,
    llvh::ArrayRef<std::string> execArgs) {
  assert(
      outputLevel >= OutputLevelKind::IR &&
      "generateOutput() invoked needlessly");
  if (outputLevel < OutputLevelKind::IR)
    return true;

  if (outputLevel < OutputLevelKind::C) {
    return compileToDump(context, M, params, outputFilename);
  }
  if (outputLevel == OutputLevelKind::C) {
    return compileToC(context, M, params, inputFilename, outputFilename);
  }
  if (outputLevel <= OutputLevelKind::Executable) {
    return compileFromC(
        context, M, params, outputLevel, inputFilename, outputFilename);
  }
  if (outputLevel == OutputLevelKind::Run) {
    return execute(context, M, params, inputFilename, execArgs);
  }

  assert(false && "unsupported output level");
  llvh::errs() << "Unsupported compilation mode\n";
  return false;
}
