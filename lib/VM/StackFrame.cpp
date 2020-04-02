/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StackFrame.h"

#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace vm {

void dumpStackFrame(
    ConstStackFramePtr frame,
    llvm::raw_ostream &OS,
    const PinnedHermesValue *next) {
  auto format_ptr = [](const void *p) {
    return llvm::format_hex((uintptr_t)p, 10);
  };

  OS << "Frame @" << format_ptr(frame.ptr()) << "\n";
  if (next) {
    OS << "  size [regs]     : " << frame.ptr() - next << "\n";
  }
  OS << "  PreviousFrame   : " << format_ptr(frame.getPreviousFramePointer())
     << "\n"
     << "  SavedIP         : " << format_ptr(frame.getSavedIP()) << "\n"
     << "  SavedCodeBlock  : " << format_ptr(frame.getSavedCodeBlock()) << "\n"
     << "  DebugEnvironment: " << frame.getDebugEnvironmentRef() << "\n"
     << "  ArgCount        : " << frame.getArgCount() << "\n"
     << "  NewTarget       : " << frame.getNewTargetRef() << "\n"
     << "  CalleeClosure   : " << frame.getCalleeClosureOrCBRef() << "\n"
     << "  ThisArg         : " << frame.getThisArgRef() << "\n"
     << "  Args: ";
  for (uint32_t i = 0, count = frame.getArgCount(); i != count; ++i) {
    if (i != 0)
      OS << ", ";
    OS << frame.getArgRef(i);
  }
  OS << "\n";
}

LLVM_ATTRIBUTE_NOINLINE
void dumpStackFrame(ConstStackFramePtr frame) {
  dumpStackFrame(frame, llvm::errs());
}

LLVM_ATTRIBUTE_NOINLINE
void dumpStackFrame(StackFramePtr frame) {
  dumpStackFrame(frame, llvm::errs());
}

} // namespace vm
} // namespace hermes
