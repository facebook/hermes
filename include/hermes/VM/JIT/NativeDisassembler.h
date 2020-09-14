/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_NATIVEDISASSEMBLER_H
#define HERMES_VM_JIT_NATIVEDISASSEMBLER_H

#include "llvh/ADT/ArrayRef.h"

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {
namespace vm {

/// Wraps LLVMDisassembler to produce an assembly listing with addresses.
class NativeDisassembler {
 protected:
  NativeDisassembler() {}

 public:
  static const char x86_64_unknown_linux_gnu[];

  virtual ~NativeDisassembler() = 0;

  NativeDisassembler(const NativeDisassembler &) = delete;
  void operator=(const NativeDisassembler &) = delete;

  static std::unique_ptr<NativeDisassembler> create(
      const char *triple,
      unsigned asmOutputVariant = 0);

  /// Disassemble a byte buffer \p bytes into the specified output stream.
  /// \param address the address in the memory space of a region of the first
  ///   byte of the array.
  /// \param withAddr whether to dump the address and bytes of assembly
  /// instructions.
  /// \return the number of errors encountered.
  virtual int disassembleBuffer(
      llvh::raw_ostream &OS,
      llvh::ArrayRef<uint8_t> bytes,
      uint64_t address,
      bool withAddr) = 0;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_NATIVEDISASSEMBLER_H
