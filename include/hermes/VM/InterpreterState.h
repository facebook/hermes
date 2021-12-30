/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INTERPRETERSTATE_H
#define HERMES_VM_INTERPRETERSTATE_H
#include <cstdint>

#include "CallResult.h"

namespace hermes {
namespace vm {

class CodeBlock;

/// Contains some information on the interpreter at some point in time.
/// Passed between calls to the interpreter to help with stepping.
struct InterpreterState {
  /// The executing code block.
  CodeBlock *codeBlock{nullptr};

  /// The offset from the start of the code block of the next instruction to
  /// execute.
  uint32_t offset{0};

  InterpreterState() {}

  InterpreterState(CodeBlock *codeBlock, uint32_t offset)
      : codeBlock(codeBlock), offset(offset) {}

  InterpreterState(const InterpreterState &) = default;
  InterpreterState &operator=(const InterpreterState &) = default;

  bool operator==(const InterpreterState &other) const {
    return codeBlock == other.codeBlock && offset == other.offset;
  }
  bool operator!=(const InterpreterState &other) const {
    return !(*this == other);
  }
};
} // namespace vm
} // namespace hermes

#endif
