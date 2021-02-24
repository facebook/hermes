/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_JIT_DISASSEMBLER

#include "hermes/VM/JIT/LLVMDisassembler.h"

#include "llvh/MC/MCAsmBackend.h"
#include "llvh/MC/MCAsmInfo.h"
#include "llvh/MC/MCCodeEmitter.h"
#include "llvh/MC/MCContext.h"
#include "llvh/MC/MCDisassembler/MCDisassembler.h"
#include "llvh/MC/MCInst.h"
#include "llvh/MC/MCInstPrinter.h"
#include "llvh/MC/MCInstrInfo.h"
#include "llvh/MC/MCObjectFileInfo.h"
#include "llvh/MC/MCRegisterInfo.h"
#include "llvh/MC/MCStreamer.h"
#include "llvh/MC/MCSubtargetInfo.h"
#include "llvh/Support/TargetRegistry.h"
#include "llvh/Support/TargetSelect.h"

namespace hermes {
namespace vm {

static void initLLVM() {
  // Initialize targets and assembly printers/parsers.
  llvh::InitializeAllTargetInfos();
  llvh::InitializeAllTargetMCs();
  llvh::InitializeAllAsmParsers();
  llvh::InitializeAllDisassemblers();
}

class LLVMDisassembler::Impl {
  std::unique_ptr<llvh::MCRegisterInfo> MRI_;
  std::unique_ptr<llvh::MCAsmInfo> MAI_;
  std::unique_ptr<llvh::MCObjectFileInfo> MOFI_;
  std::unique_ptr<llvh::MCContext> ctx_;
  std::unique_ptr<llvh::MCInstrInfo> MCII_;

  llvh::SmallString<64> buf_;
  llvh::raw_svector_ostream OS_{buf_};

  std::unique_ptr<llvh::MCStreamer> streamer_;
  std::unique_ptr<llvh::MCSubtargetInfo> STI_;
  std::unique_ptr<const llvh::MCDisassembler> disAsm_;

 public:
  Impl(const char *tripleName, unsigned asmOutputVariant);

  /// Format a single instruction into a string.
  /// \param address the address in the memory space of a region of the first
  ///   byte of the array.
  /// \param[out] size is initialized with the byte size of the decoded
  ///   instruction.
  /// \return the instruction string on success or an empty optional on error.
  llvh::Optional<llvh::StringRef> formatInstruction(
      llvh::ArrayRef<uint8_t> bytes,
      uint64_t address,
      uint64_t &size);
};

LLVMDisassembler::Impl::Impl(
    const char *tripleName,
    unsigned asmOutputVariant) {
  // Initialize LLVM in a thread-safe manner.
  static char dummyInit = (initLLVM(), (char)'\0');
  (void)dummyInit;

  // Obtain the target.
  llvh::Triple triple{llvh::Triple::normalize(tripleName)};
  std::string error;
  auto *target = llvh::TargetRegistry::lookupTarget("", triple, error);
  if (!target) {
    llvh::report_fatal_error(
        llvh::Twine("Can't get target for triple ") + tripleName);
  }

  MRI_.reset(target->createMCRegInfo(triple.getTriple()));
  assert(MRI_ && "Unable to create target register info!");

  MAI_.reset(target->createMCAsmInfo(*MRI_, triple.getTriple()));
  assert(MAI_ && "Unable to create target asm info!");

  // Note circular dependency between MOFI_ and ctx_.
  MOFI_.reset(new llvh::MCObjectFileInfo());
  ctx_.reset(new llvh::MCContext(MAI_.get(), MRI_.get(), MOFI_.get()));
  MOFI_->InitMCObjectFileInfo(triple, true, *ctx_);

  MCII_.reset(target->createMCInstrInfo());

  std::unique_ptr<llvh::MCInstPrinter> IP{target->createMCInstPrinter(
      triple, asmOutputVariant, *MAI_, *MCII_, *MRI_)};

  streamer_.reset(target->createAsmStreamer(
      *ctx_,
      std::make_unique<llvh::formatted_raw_ostream>(OS_),
      /*asmverbose*/ true,
      /*useDwarfDirectory*/ true,
      IP.release(),
      nullptr,
      nullptr,
      false));

  streamer_->InitSections(false);
  buf_.clear();

  STI_.reset(target->createMCSubtargetInfo(triple.getTriple(), "", ""));
  disAsm_.reset(target->createMCDisassembler(*STI_, *ctx_));
  if (!disAsm_) {
    llvh::report_fatal_error(
        llvh::Twine("No disassembler for triple") + tripleName);
  }
}

llvh::Optional<llvh::StringRef> LLVMDisassembler::Impl::formatInstruction(
    llvh::ArrayRef<uint8_t> bytes,
    uint64_t address,
    uint64_t &size) {
  llvh::MCInst inst;
  llvh::MCDisassembler::DecodeStatus status;

  status = disAsm_->getInstruction(
      inst, size, bytes, address, /*REMOVE*/ llvh::nulls(), llvh::nulls());
  switch (status) {
    case llvh::MCDisassembler::Fail:
      if (size == 0)
        size = 1; // skip illegible bytes
      return llvh::None;

    case llvh::MCDisassembler::SoftFail:
    case llvh::MCDisassembler::Success:
      buf_.clear();
      streamer_->EmitInstruction(inst, *STI_);
      // Remove trailing "\n".
      auto res = buf_.str();
      res.consume_front("\t");
      res.consume_back("\n");
      return res;
  }
  llvm_unreachable("invalid decode status");
}

LLVMDisassembler::LLVMDisassembler(
    const char *tripleName,
    unsigned asmOutputVariant)
    : impl_(new Impl(tripleName, asmOutputVariant)) {}

LLVMDisassembler::~LLVMDisassembler() {
  delete impl_;
}

llvh::Optional<llvh::StringRef> LLVMDisassembler::formatInstruction(
    llvh::ArrayRef<uint8_t> bytes,
    uint64_t address,
    uint64_t &size) {
  return impl_->formatInstruction(bytes, address, size);
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_JIT_DISASSEMBLER
