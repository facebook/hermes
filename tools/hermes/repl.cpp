/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "repl.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/Parser/JSLexer.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSObject.h"
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

#define C_STRING(x) #x

#if HAVE_LIBREADLINE
static const std::string kHistoryFileBaseName = ".hermes_history";
static const int kHistoryMaxEntries = 500;
#endif

using namespace hermes;

static llvh::cl::opt<std::string> PromptString(
    "prompt",
    llvh::cl::init(">> "),
    llvh::cl::desc("Prompt string for the REPL."));

static llvh::cl::opt<std::string> Prompt2String(
    "prompt2",
    llvh::cl::init("...  "),
    llvh::cl::desc("Prompt string for continuation lines in the REPL."));

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
  llvh::outs() << prompt;
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
#ifndef __EMSCRIPTEN__
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
#else
  // Avoid unused function warnings.
  (void)handleSignal;
#endif
  llvh::outs() << prompt;
  std::string current{};
  bool success = !!std::getline(std::cin, current);
#ifndef __EMSCRIPTEN__
  action.sa_handler = oldAction.sa_handler;
  ::sigaction(SIGINT, &action, &oldAction);
#endif
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
static bool needsAnotherLine(llvh::StringRef input) {
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

  // Look at the previous token to see if the next token could possibly be the
  // beginning of a regular expression.
  // https://github.com/mikesamuel/es5-lexer/blob/master/src/guess_is_regexp.js
  auto isRegexPossible = [](OptValue<parser::TokenKind> previousTokenKind) {
    if (!previousTokenKind) {
      return true;
    }
    switch (*previousTokenKind) {
      case parser::TokenKind::rw_break:
      case parser::TokenKind::rw_case:
      case parser::TokenKind::rw_continue:
      case parser::TokenKind::rw_delete:
      case parser::TokenKind::rw_do:
      case parser::TokenKind::rw_else:
      case parser::TokenKind::rw_finally:
      case parser::TokenKind::rw_in:
      case parser::TokenKind::rw_instanceof:
      case parser::TokenKind::rw_return:
      case parser::TokenKind::rw_throw:
      case parser::TokenKind::rw_try:
      case parser::TokenKind::rw_typeof:
      case parser::TokenKind::rw_void:
      case parser::TokenKind::plus:
      case parser::TokenKind::minus:
      case parser::TokenKind::period:
      case parser::TokenKind::slash:
      case parser::TokenKind::comma:
      case parser::TokenKind::star:
      case parser::TokenKind::exclaim:
      case parser::TokenKind::percent:
      case parser::TokenKind::amp:
      case parser::TokenKind::l_paren:
      case parser::TokenKind::colon:
      case parser::TokenKind::semi:
      case parser::TokenKind::less:
      case parser::TokenKind::equal:
      case parser::TokenKind::greater:
      case parser::TokenKind::question:
      case parser::TokenKind::l_square:
      case parser::TokenKind::caret:
      case parser::TokenKind::l_brace:
      case parser::TokenKind::pipe:
      case parser::TokenKind::r_brace:
      case parser::TokenKind::tilde:
        return true;
      default:
        return false;
    }
  };

  OptValue<parser::TokenKind> previousTokenKind;

  // Use AllowRegExp when a regular expression is possible and use AllowDiv
  // otherwise so that division is correctly parsed.
  while (const parser::Token *token = lexer.advance(
             isRegexPossible(previousTokenKind)
                 ? parser::JSLexer::GrammarContext::AllowRegExp
                 : parser::JSLexer::GrammarContext::AllowDiv)) {
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
    previousTokenKind = token->getKind();
  }

  // If the stack is empty, then we can't recover from the error,
  // because there was some other problem besides mismatched delimiters.
  // TODO: Handle other classes of recoverable errors.
  return !stack.empty();
}

#if HAVE_LIBREADLINE
// Load history file or create it
static std::error_code loadHistoryFile(llvh::SmallString<128> &historyFile) {
  if (!llvh::sys::path::home_directory(historyFile)) {
    // Use ENOENT here since it could not found a home directory
    return std::error_code(ENOENT, std::system_category());
  }

  llvh::sys::path::append(historyFile, kHistoryFileBaseName);

  auto err = ::read_history(historyFile.c_str());
  if (err != 0) {
    // Return a error_code object from a errno enum
    return std::error_code(err, std::system_category());
  }

  return std::error_code();
}
#endif

// This is the vm driver.
int repl(const vm::RuntimeConfig &config) {
  auto runtime = vm::Runtime::create(config);

  vm::GCScope gcScope(runtime.get());
  installConsoleBindings(runtime.get());

  std::string code;
  code.reserve(256);

  auto global = runtime->getGlobal();
  auto propRes = vm::JSObject::getNamed_RJS(
      global, runtime.get(), vm::Predefined::getSymbolID(vm::Predefined::eval));
  if (propRes == vm::ExecutionStatus::EXCEPTION) {
    runtime->printException(
        llvh::outs(), runtime->makeHandle(runtime->getThrownValue()));
    return 1;
  }
  auto evalFn = runtime->makeHandle<vm::Callable>(std::move(*propRes));

  llvh::StringRef evaluateLineString =
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
    llvh::raw_ostream &errs = hasColors
        ? llvh::errs().changeColor(llvh::raw_ostream::Colors::RED)
        : llvh::errs();
    llvh::raw_ostream &outs = hasColors
        ? llvh::outs().changeColor(llvh::raw_ostream::Colors::RED)
        : llvh::outs();
    errs << "Unable to get REPL util function: evaluateLine.\n";
    runtime->printException(
        outs, runtime->makeHandle(runtime->getThrownValue()));
    return 1;
  }
  auto evaluateLineFn =
      runtime->makeHandle<vm::JSFunction>(std::move(*callRes));

  runtime->getHeap().runtimeWillExecute();

#if HAVE_LIBREADLINE
  llvh::SmallString<128> historyFile{};
  auto historyErr = loadHistoryFile(historyFile);
  if (historyErr && historyErr.value() != ENOENT) {
    llvh::errs() << "Could not load history file: " << historyErr.message()
                 << '\n';
  }
#endif

  // SetUnbuffered because there is no explicit flush after prompt (>>).
  // There is also no explicitly flush at end of line. (An automatic flush
  // mechanism is not guaranteed to be present, from my experiment on Windows)
  llvh::outs().SetUnbuffered();
  while (true) {
    // Main loop
    auto readResult = readInputLine(
        code.empty() ? PromptString.c_str() : Prompt2String.c_str(), code);
    if (readResult == ReadResult::FAILURE ||
        (readResult == ReadResult::INTERRUPT && code.empty())) {
      // EOF or user exit on non-continuation line.
      llvh::outs() << '\n';
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
      llvh::outs() << '\n';
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
          hasColors ? llvh::outs().changeColor(llvh::raw_ostream::Colors::RED)
                    : llvh::outs(),
          runtime->makeHandle(runtime->getThrownValue()));
      llvh::outs().resetColor();
      code.clear();
      continue;
    }

    if ((*callRes)->isUndefined()) {
      code.clear();
      continue;
    }

    // Operator resolution in the vm namespace.
    vm::SmallU16String<32> tmp;
    vm::operator<<(
        llvh::outs(),
        vm::StringPrimitive::createStringView(
            runtime.get(),
            vm::Handle<vm::StringPrimitive>::vmcast(
                runtime->makeHandle(std::move(*callRes))))
            .getUTF16Ref(tmp))
        << "\n";
    code.clear();
  }

  return 0;
}
