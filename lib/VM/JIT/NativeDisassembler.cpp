/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/NativeDisassembler.h"

#include "hermes/VM/JIT/LLVMDisassembler.h"

#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace vm {

namespace {

#ifdef HERMESVM_JIT_DISASSEMBLER

class NativeDisassemblerImpl : public NativeDisassembler {
 public:
  explicit NativeDisassemblerImpl(const char *triple, unsigned asmOutputVariant)
      : llvmDis_(triple, asmOutputVariant) {}

  int disassembleBuffer(
      llvh::raw_ostream &OS,
      llvh::ArrayRef<uint8_t> bytes,
      uint64_t address,
      bool withAddr) override;

 private:
  LLVMDisassembler llvmDis_;
};

int NativeDisassemblerImpl::disassembleBuffer(
    llvh::raw_ostream &OS,
    llvh::ArrayRef<uint8_t> bytes,
    uint64_t address,
    bool withAddr) {
  int errors = 0;
  size_t index = 0;

  while (index < bytes.size()) {
    uint64_t size;
    auto res =
        llvmDis_.formatInstruction(bytes.slice(index), address + index, size);

    if (withAddr) {
      constexpr unsigned maxBytes = 10;

      // Print the address.
      OS << llvh::format("%05lu: ", (long)address);

      // Print the instruction bytes.
      for (unsigned i = 0, end = std::max(maxBytes, (unsigned)size); i < end;
           ++i) {
        if (i < size)
          OS << " " << llvh::format_hex_no_prefix(bytes[index + i], 2);
        else
          OS << "   ";
      }
    }
    OS << " ";

    if (res) {
      OS << *res << "\n";
    } else {
      OS << "\terror\n";
      ++errors;
    }
    index += size;
    address += size;
  }

  return errors;
}

#else // HERMESVM_JIT_DISASSEMBLER
class NativeDisassemblerImpl : public NativeDisassembler {
 public:
  explicit NativeDisassemblerImpl(
      const char *triple,
      unsigned asmOutputVariant) {}

  int disassembleBuffer(
      llvh::raw_ostream &OS,
      llvh::ArrayRef<uint8_t> bytes,
      uint64_t address,
      bool withAddr) override {
    return 0;
  }
};
#endif // HERMESVM_JIT_DISASSEMBLER

} // anonymous namespace

const char NativeDisassembler::x86_64_unknown_linux_gnu[] =
    "x86_64-unknown-linux-gnu";

NativeDisassembler::~NativeDisassembler() {}

std::unique_ptr<NativeDisassembler> NativeDisassembler::create(
    const char *triple,
    unsigned int asmOutputVariant) {
  return std::make_unique<NativeDisassemblerImpl>(triple, asmOutputVariant);
}

} // namespace vm
} // namespace hermes
