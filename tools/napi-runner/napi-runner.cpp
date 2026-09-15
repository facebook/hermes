/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// A minimal tool that runs a .js or .hbc file through the Hermes NAPI layer.
/// .js files are compiled and executed via hermes_run_script.
/// .hbc files are executed via hermes_run_bytecode.
/// --compile writes serialized bytecode to a file instead of executing.

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Runtime.h"
#include "napi/hermes_napi.h"
#include "napi/hermes_napi_compile.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

#include <cstring>

using namespace hermes;

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::desc("<input file>"),
    llvh::cl::Positional,
    llvh::cl::Required);

static llvh::cl::opt<std::string> CompileOutput(
    "compile",
    llvh::cl::desc("Compile JS to bytecode and write to <file>"),
    llvh::cl::value_desc("file"));

static llvh::cl::opt<bool> Optimize(
    "O",
    llvh::cl::desc("Enable optimizations when compiling"),
    llvh::cl::init(false));

static llvh::cl::opt<bool> EnableTS(
    "enable-ts",
    llvh::cl::desc("Enable TypeScript parsing (strip type annotations)"),
    llvh::cl::init(false));

/// Print any pending NAPI exception to stderr and clear it.
/// Returns true if an exception was pending.
static bool printAndClearException(napi_env env) {
  bool hasPending = false;
  napi_is_exception_pending(env, &hasPending);
  if (!hasPending)
    return false;

  napi_value exception;
  napi_get_and_clear_last_exception(env, &exception);

  // Try to get the exception's message or toString().
  napi_value str;
  if (napi_coerce_to_string(env, exception, &str) == napi_ok) {
    size_t len = 0;
    napi_get_value_string_utf8(env, str, nullptr, 0, &len);
    std::string buf(len, '\0');
    napi_get_value_string_utf8(env, str, &buf[0], len + 1, &len);
    llvh::errs() << "Error: " << buf << "\n";
  } else {
    llvh::errs() << "Error: (could not convert exception to string)\n";
    // Clear any exception from the coercion attempt.
    napi_value ignored;
    napi_get_and_clear_last_exception(env, &ignored);
  }
  return true;
}

/// Print a napi_value to stdout. Handles strings, numbers, booleans,
/// undefined, null, and falls back to toString() for objects.
static void printResult(napi_env env, napi_value result) {
  napi_valuetype type;
  napi_typeof(env, result, &type);

  if (type == napi_undefined) {
    // Don't print undefined (matches hermes CLI behavior).
    return;
  }

  napi_value str;
  if (napi_coerce_to_string(env, result, &str) != napi_ok) {
    // If coercion throws, print the exception.
    printAndClearException(env);
    return;
  }

  size_t len = 0;
  napi_get_value_string_utf8(env, str, nullptr, 0, &len);
  std::string buf(len, '\0');
  napi_get_value_string_utf8(env, str, &buf[0], len + 1, &len);
  llvh::outs() << buf << "\n";
}

int main(int argc, char **argv) {
  llvh::InitLLVM initLLVM(argc, argv);
  llvh::sys::PrintStackTraceOnErrorSignal("napi-runner");
  llvh::PrettyStackTraceProgram X(argc, argv);
  llvh::cl::ParseCommandLineOptions(argc, argv, "Hermes NAPI runner\n");

  // Read input file.
  auto fileBufOrErr = llvh::MemoryBuffer::getFile(InputFilename);
  if (!fileBufOrErr) {
    llvh::errs() << "Error: could not open file: " << InputFilename << "\n";
    return 1;
  }
  auto &fileBuf = *fileBufOrErr;

  // Determine mode from file extension.
  auto ext = llvh::sys::path::extension(InputFilename);
  bool isBytecode = (ext == ".hbc");

  // Create runtime and NAPI env.
  auto rtConfig = vm::RuntimeConfig::Builder().build();
  auto runtime = vm::Runtime::create(rtConfig);
  napi_env env = hermes_napi_create_env(&*runtime);

  napi_handle_scope scope;
  napi_open_handle_scope(env, &scope);

  napi_status status;
  napi_value result;

  const auto *data =
      reinterpret_cast<const uint8_t *>(fileBuf->getBufferStart());
  size_t size = fileBuf->getBufferSize();

  if (!CompileOutput.empty()) {
    // --compile mode: compile JS source and write bytecode to file.
    if (isBytecode) {
      llvh::errs() << "Error: --compile cannot be used with .hbc input\n";
      napi_close_handle_scope(env, scope);
      return 1;
    }
    hermes_compile_flags cflags{};
    cflags.struct_size = sizeof(cflags);
    cflags.optimize = Optimize;
    cflags.enable_ts = EnableTS;
    uint8_t *bytecode = nullptr;
    size_t bytecodeSize = 0;
    status = hermes_compile_to_bytecode(
        env,
        data,
        size,
        InputFilename.c_str(),
        &cflags,
        &bytecode,
        &bytecodeSize);
    if (status != napi_ok) {
      printAndClearException(env);
      napi_close_handle_scope(env, scope);
      return 1;
    }
    std::error_code ec;
    llvh::raw_fd_ostream os(CompileOutput, ec);
    if (ec) {
      llvh::errs() << "Error: could not open output file: " << CompileOutput
                   << ": " << ec.message() << "\n";
      hermes_free_bytecode(bytecode);
      napi_close_handle_scope(env, scope);
      return 1;
    }
    os.write(reinterpret_cast<const char *>(bytecode), bytecodeSize);
    hermes_free_bytecode(bytecode);
    napi_close_handle_scope(env, scope);
    return 0;
  }

  if (isBytecode) {
    status = hermes_run_bytecode(
        env,
        data,
        size,
        nullptr,
        nullptr,
        InputFilename.c_str(),
        nullptr,
        &result);
  } else {
    // JS source — compile and run via hermes_run_script.
    hermes_run_script_flags rflags{};
    rflags.struct_size = sizeof(rflags);
    rflags.enable_ts = EnableTS;
    status = hermes_run_script(
        env,
        data,
        size,
        nullptr,
        nullptr,
        InputFilename.c_str(),
        &rflags,
        &result);
  }

  int exitCode = 0;
  if (status != napi_ok) {
    printAndClearException(env);
    exitCode = 1;
  } else {
    printResult(env, result);
  }

  napi_close_handle_scope(env, scope);
  // env is owned by the runtime; it dies with `runtime` going out of scope.
  return exitCode;
}
