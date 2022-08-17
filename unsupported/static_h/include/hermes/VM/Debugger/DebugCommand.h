/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DEBUGGER_DEBUGCOMMAND_H
#define HERMES_VM_DEBUGGER_DEBUGCOMMAND_H

#ifdef HERMES_ENABLE_DEBUGGER

#include "hermes/Public/DebuggerTypes.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/StringExtras.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

#include <string>

namespace hermes {
namespace vm {

class Debugger;
struct InterpreterState;

/// Commands that can be sent to the debugger.
enum class DebugCommandType { NONE, CONTINUE, STEP, EVAL };

struct CreateBreakpointArgs {
  uint32_t filenameId;
  uint32_t line;
  uint32_t column;
};

struct PrintVarsArgs {
  /// Index of the call stack frame to print. 0 means the topmost frame.
  uint32_t frameIdx;
};

struct EvalArgs {
  /// Index of the call stack frame in which to eval. 0 means the topmost frame.
  uint32_t frameIdx;
};

struct StepArgs {
  /// Stepping mode.
  ::facebook::hermes::debugger::StepMode mode;
};

struct DebugCommand {
  DebugCommandType type;
  union {
    EvalArgs evalArgs;
    StepArgs stepArgs;
  };

  /// A generic string argument, whose interpretation is command dependent.
  std::string text = {};

  DebugCommand(DebugCommandType type) : type(type) {}

  static DebugCommand makeNone() {
    return DebugCommand{DebugCommandType::NONE};
  }

  static DebugCommand makeContinue() {
    return DebugCommand{DebugCommandType::CONTINUE};
  }

  static DebugCommand makeStep(::facebook::hermes::debugger::StepMode mode) {
    DebugCommand cmd = {DebugCommandType::STEP};
    cmd.stepArgs = StepArgs{mode};
    return cmd;
  }

  static DebugCommand makeEval(uint32_t frameIdx, std::string text) {
    DebugCommand cmd{DebugCommandType::EVAL};
    cmd.evalArgs.frameIdx = frameIdx;
    cmd.text = std::move(text);
    return cmd;
  }
};

} // namespace vm
} // namespace hermes

#endif

#endif
