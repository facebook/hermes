/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_LLVMDISASSEMBLER_H
#define HERMES_VM_JIT_LLVMDISASSEMBLER_H

#ifdef HERMESVM_JIT_DISASSEMBLER

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"

namespace hermes {
namespace vm {

/// A helper class wrapping LLVM's rather complex and powerful disassembler
/// functionality in order to disassemble individual instructions into strings.
class LLVMDisassembler {
  class Impl;
  Impl *impl_;

 public:
  explicit LLVMDisassembler(
      const char *tripleName,
      unsigned asmOutputVariant = 0);
  ~LLVMDisassembler();

  LLVMDisassembler(const LLVMDisassembler &) = delete;
  void operator=(const LLVMDisassembler &) = delete;

  /// Format a single instruction into a string.
  /// \param address the address in the memory space of a region of the first
  ///   byte of the array.
  /// \param[out] size is initialized with the byte size of the decoded
  ///   instruction.
  /// \return the instruction string on success or an empty optional on error.
  llvm::Optional<llvm::StringRef> formatInstruction(
      llvm::ArrayRef<uint8_t> bytes,
      uint64_t address,
      uint64_t &size);
};

} // namespace vm
} // namespace hermes

#endif // HERMESVM_JIT_DISASSEMBLER

#endif // HERMES_VM_JIT_LLVMDISASSEMBLER_H
