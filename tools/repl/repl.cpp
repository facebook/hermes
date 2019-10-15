/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/Parser/JSLexer.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringView.h"

#include "ReplConfig.h"

#if HAVE_LIBREADLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#ifndef _WINDOWS
#include <unistd.h>
#endif

#include <csetjmp>
#include <csignal>
#include <iostream>
#include <stack>

#define DEBUG_TYPE "hermes-repl"

#define C_STRING(x) #x

#if HAVE_LIBREADLINE
static const std::string kHistoryFileBaseName = ".hermes_history";
static const int kHistoryMaxEntries = 500;
#endif

using namespace hermes;

static llvm::cl::opt<std::string> PromptString(
    "prompt",
    llvm::cl::init(">> "),
    llvm::cl::desc("Prompt string for the REPL."));

static llvm::cl::opt<std::string> Prompt2String(
    "prompt2",
    llvm::cl::init("...  "),
    llvm::cl::desc("Prompt string for continuation lines in the REPL."));

static llvm::cl::opt<bool> GCPrintStats(
    "gc-print-stats",
    llvm::cl::desc("Output summary garbage collection statistics at exit"),
    llvm::cl::init(false));

namespace {
enum class ReadResult {
  SUCCESS,
  FAILURE,
  INTERRUPT,
};
}

/// Print the prompt \p prompt and read a line from stdin with editing, storing
/// it in \p line.
/// When the user sends SIGINT instead of providing input to the REPL,
/// we return ReadResult::INTERRUPT directly from readInputLine.
/// This works because readline allows longjmps out of it without breaking
/// (it has a signal handler that cleans up and rethrows the signal).
/// \return SUCCESS if it worked, INTERRUPT on SIGINT, FAILURE on EOF.
static ReadResult readInputLine(const char *prompt, std::string &line);

#ifdef _WINDOWS

static ReadResult readInputLine(const char *prompt, std::string &line) {
  llvm::outs() << prompt;
  std::string current{};
  bool success = !!std::getline(std::cin, current);

  if (!success) {
    return ReadResult::FAILURE;
  }
  line.append(current);
  return ReadResult::SUCCESS;
}

#else

static std::jmp_buf readlineJmpBuf;

/// Store the last sigaction so we can restore it.
static struct sigaction oldAction;

/// Handler for SIGINT in readline.
/// Clears the signal handler and longjmps to early return from readInputLine.
static void handleSignal(int sig) {
  if (sig == SIGINT) {
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = oldAction.sa_handler;
    ::sigaction(SIGINT, &action, &oldAction);
    ::longjmp(readlineJmpBuf, 1);
  }
}

static ReadResult readInputLine(const char *prompt, std::string &line) {
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (setjmp(readlineJmpBuf) != 0) {
    return ReadResult::INTERRUPT;
  }

  action.sa_handler = handleSignal;
  ::sigaction(SIGINT, &action, &oldAction);

#if HAVE_LIBREADLINE
  if (oscompat::isatty(STDIN_FILENO)) {
    char *rl = ::readline(prompt);
    action.sa_handler = oldAction.sa_handler;
    ::sigaction(SIGINT, &action, &oldAction);

    if (!rl)
      return ReadResult::FAILURE;
    line.append(rl);
    ::add_history(rl);
    ::free(rl);
    return ReadResult::SUCCESS;
  }
#endif
  llvm::outs() << prompt;
  std::string current{};
  bool success = !!std::getline(std::cin, current);
  action.sa_handler = oldAction.sa_handler;
  ::sigaction(SIGINT, &action, &oldAction);
  if (!success) {
    return ReadResult::FAILURE;
  }
  line.append(current);
  return ReadResult::SUCCESS;
}
#endif

/// Checks if the provided input needs another line to become valid.
/// Currently, this only checks if there's open delimiter tokens
/// that haven't been closed, and that the preceding closing delimiter tokens
/// are actually valid.
/// Otherwise, simply returns false, and the line is fed as-is to eval().
static bool needsAnotherLine(llvm::StringRef input) {
  SourceErrorManager sm;
  SourceErrorManager::SaveAndSuppressMessages suppress(&sm);
  hermes::BumpPtrAllocator allocator{};
  parser::JSLexer lexer(input, sm, allocator, nullptr, false);

  std::stack<parser::TokenKind> stack{};

  if (input.empty()) {
    return false;
  }

  if (input.back() == '\\') {
    return true;
  }

  // Given a right delimiter token kind, get the corresponding left one.
  auto getLeft = [](const parser::TokenKind kind) {
    switch (kind) {
      case parser::TokenKind::r_brace:
        return parser::TokenKind::l_brace;
      case parser::TokenKind::r_paren:
        return parser::TokenKind::l_paren;
      case parser::TokenKind::r_square:
        return parser::TokenKind::l_square;
      default:
        llvm_unreachable("getLeft executed with unsupported kind");
    }
  };

  while (const parser::Token *token = lexer.advance()) {
    if (token->getKind() == parser::TokenKind::eof) {
      break;
    }
    switch (token->getKind()) {
      case parser::TokenKind::l_brace:
      case parser::TokenKind::l_paren:
      case parser::TokenKind::l_square:
        // Push any left side delimiters.
        stack.push(token->getKind());
        break;
      case parser::TokenKind::r_brace:
      case parser::TokenKind::r_paren:
      case parser::TokenKind::r_square: {
        // Try to match the right delimiter, and if it can't be matched,
        // the failure is unrecoverable.
        auto left = getLeft(token->getKind());
        if (!stack.empty() && stack.top() == left) {
          // Matched the delimiter, pop it off and continue.
          stack.pop();
        } else {
          // Failed to match, so we can't be recoverable.
          return false;
        }
        break;
      }
      default:
        // Do nothing for the other tokens.
        break;
    }
  }

  // If the stack is empty, then we can't recover from the error,
  // because there was some other problem besides mismatched delimiters.
  // TODO: Handle other classes of recoverable errors.
  return !stack.empty();
}

#if HAVE_LIBREADLINE
// Load history file or create it
static std::error_code loadHistoryFile(llvm::SmallString<128> &historyFile) {
  if (!llvm::sys::path::home_directory(historyFile)) {
    // Use ENOENT here since it could not found a home directory
    return std::error_code(ENOENT, std::system_category());
  }

  llvm::sys::path::append(historyFile, kHistoryFileBaseName);

  auto err = ::read_history(historyFile.c_str());
  if (err != 0) {
    // Return a error_code object from a errno enum
    return std::error_code(err, std::system_category());
  }

  return std::error_code();
}
#endif

// This is the vm driver.
int main(int argc, char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal("Hermes REPL");
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm::llvm_shutdown_obj Y;
  llvm::cl::AddExtraVersionPrinter(driver::printHermesREPLVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes REPL driver\n");

  auto runtime = vm::Runtime::create(
      vm::RuntimeConfig::Builder()
          .withGCConfig(vm::GCConfig::Builder()
                            .withInitHeapSize(cl::InitHeapSize.bytes)
                            .withMaxHeapSize(cl::MaxHeapSize.bytes)
                            .withSanitizeConfig(
                                vm::GCSanitizeConfig::Builder()
                                    .withSanitizeRate(cl::GCSanitizeRate)
                                    .withRandomSeed(cl::GCSanitizeRandomSeed)
                                    .build())
                            .withShouldRecordStats(GCPrintStats)
                            .build())
          .withES6Symbol(cl::ES6Symbol)
          .build());

  vm::GCScope gcScope(runtime.get());
  installConsoleBindings(runtime.get());

  std::string code;
  code.reserve(256);

  auto global = runtime->getGlobal();
  auto propRes = vm::JSObject::getNamed_RJS(
      global, runtime.get(), vm::Predefined::getSymbolID(vm::Predefined::eval));
  if (propRes == vm::ExecutionStatus::EXCEPTION) {
    runtime->printException(
        llvm::outs(), runtime->makeHandle(runtime->getThrownValue()));
    return 1;
  }
  auto evalFn = runtime->makeHandle<vm::Callable>(*propRes);

  llvm::StringRef evaluateLineString =
#include "evaluate-line.js"
      ;
  bool hasColors = oscompat::should_color(STDOUT_FILENO);

  auto callRes = evalFn->executeCall1(
      evalFn,
      runtime.get(),
      global,
      vm::StringPrimitive::createNoThrow(runtime.get(), evaluateLineString)
          .getHermesValue());
  if (callRes == vm::ExecutionStatus::EXCEPTION) {
    llvm::raw_ostream &errs = hasColors
        ? llvm::errs().changeColor(llvm::raw_ostream::Colors::RED)
        : llvm::errs();
    llvm::raw_ostream &outs = hasColors
        ? llvm::outs().changeColor(llvm::raw_ostream::Colors::RED)
        : llvm::outs();
    errs << "Unable to get REPL util function: evaluateLine.\n";
    runtime->printException(
        outs, runtime->makeHandle(runtime->getThrownValue()));
    return 1;
  }
  auto evaluateLineFn = runtime->makeHandle<vm::JSFunction>(*callRes);

  runtime->getHeap().runtimeWillExecute();

#if HAVE_LIBREADLINE
  llvm::SmallString<128> historyFile{};
  auto historyErr = loadHistoryFile(historyFile);
  if (historyErr && historyErr.value() != ENOENT) {
    llvm::errs() << "Could not load history file: " << historyErr.message()
                 << '\n';
  }
#endif

  // SetUnbuffered because there is no explicit flush after prompt (>>).
  // There is also no explicitly flush at end of line. (An automatic flush
  // mechanism is not guaranteed to be present, from my experiment on Windows)
  llvm::outs().SetUnbuffered();
  while (true) {
    // Main loop
    auto readResult = readInputLine(
        code.empty() ? PromptString.c_str() : Prompt2String.c_str(), code);
    if (readResult == ReadResult::FAILURE ||
        (readResult == ReadResult::INTERRUPT && code.empty())) {
      // EOF or user exit on non-continuation line.
      llvm::outs() << '\n';
#if HAVE_LIBREADLINE
      if (history_length > 0) {
        ::stifle_history(kHistoryMaxEntries);
        ::write_history(historyFile.c_str());
      }
#endif
      return 0;
    }

    if (readResult == ReadResult::INTERRUPT) {
      // Interrupt the continuation line.
      code.clear();
      llvm::outs() << '\n';
      continue;
    }

    if (needsAnotherLine(code)) {
      code += '\n';
      continue;
    }

    // Ensure we don't keep accumulating handles.
    vm::GCScopeMarkerRAII gcMarker{runtime.get()};

    if ((callRes = evaluateLineFn->executeCall2(
             evaluateLineFn,
             runtime.get(),
             global,
             vm::StringPrimitive::createNoThrow(runtime.get(), code)
                 .getHermesValue(),
             vm::HermesValue::encodeBoolValue(hasColors))) ==
        vm::ExecutionStatus::EXCEPTION) {
      runtime->printException(
          hasColors ? llvm::outs().changeColor(llvm::raw_ostream::Colors::RED)
                    : llvm::outs(),
          runtime->makeHandle(runtime->getThrownValue()));
      llvm::outs().resetColor();
      code.clear();
      continue;
    }

    if (callRes->isUndefined()) {
      code.clear();
      continue;
    }

    // Operator resolution in the vm namespace.
    vm::SmallU16String<32> tmp;
    vm::operator<<(
        llvm::outs(),
        vm::StringPrimitive::createStringView(
            runtime.get(),
            vm::Handle<vm::StringPrimitive>::vmcast(
                runtime->makeHandle(*callRes)))
            .getUTF16Ref(tmp))
        << "\n";
    code.clear();
  }

  return 0;
}
