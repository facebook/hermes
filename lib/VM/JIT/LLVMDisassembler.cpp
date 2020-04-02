/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_JIT_DISASSEMBLER

#include "hermes/VM/JIT/LLVMDisassembler.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

namespace hermes {
namespace vm {

static void initLLVM() {
  // Initialize targets and assembly printers/parsers.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();
}

class LLVMDisassembler::Impl {
  std::unique_ptr<llvm::MCRegisterInfo> MRI_;
  std::unique_ptr<llvm::MCAsmInfo> MAI_;
  std::unique_ptr<llvm::MCObjectFileInfo> MOFI_;
  std::unique_ptr<llvm::MCContext> ctx_;
  std::unique_ptr<llvm::MCInstrInfo> MCII_;

  llvm::SmallString<64> buf_;
  llvm::raw_svector_ostream OS_{buf_};

  std::unique_ptr<llvm::MCStreamer> streamer_;
  std::unique_ptr<llvm::MCSubtargetInfo> STI_;
  std::unique_ptr<const llvm::MCDisassembler> disAsm_;

 public:
  Impl(const char *tripleName, unsigned asmOutputVariant);

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

LLVMDisassembler::Impl::Impl(
    const char *tripleName,
    unsigned asmOutputVariant) {
  // Initialize LLVM in a thread-safe manner.
  static char dummyInit = (initLLVM(), (char)'\0');
  (void)dummyInit;

  // Obtain the target.
  llvm::Triple triple{llvm::Triple::normalize(tripleName)};
  std::string error;
  auto *target = llvm::TargetRegistry::lookupTarget("", triple, error);
  if (!target) {
    llvm::report_fatal_error(
        llvm::Twine("Can't get target for triple ") + tripleName);
  }

  MRI_.reset(target->createMCRegInfo(triple.getTriple()));
  assert(MRI_ && "Unable to create target register info!");

  MAI_.reset(target->createMCAsmInfo(*MRI_, triple.getTriple()));
  assert(MAI_ && "Unable to create target asm info!");

  // Note circular dependency between MOFI_ and ctx_.
  MOFI_.reset(new llvm::MCObjectFileInfo());
  ctx_.reset(new llvm::MCContext(MAI_.get(), MRI_.get(), MOFI_.get()));
  MOFI_->InitMCObjectFileInfo(triple, true, *ctx_);

  MCII_.reset(target->createMCInstrInfo());

  std::unique_ptr<llvm::MCInstPrinter> IP{target->createMCInstPrinter(
      triple, asmOutputVariant, *MAI_, *MCII_, *MRI_)};

  streamer_.reset(target->createAsmStreamer(
      *ctx_,
      llvm::make_unique<llvm::formatted_raw_ostream>(OS_),
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
    llvm::report_fatal_error(
        llvm::Twine("No disassembler for triple") + tripleName);
  }
}

llvm::Optional<llvm::StringRef> LLVMDisassembler::Impl::formatInstruction(
    llvm::ArrayRef<uint8_t> bytes,
    uint64_t address,
    uint64_t &size) {
  llvm::MCInst inst;
  llvm::MCDisassembler::DecodeStatus status;

  status = disAsm_->getInstruction(
      inst, size, bytes, address, /*REMOVE*/ llvm::nulls(), llvm::nulls());
  switch (status) {
    case llvm::MCDisassembler::Fail:
      if (size == 0)
        size = 1; // skip illegible bytes
      return llvm::None;

    case llvm::MCDisassembler::SoftFail:
    case llvm::MCDisassembler::Success:
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

llvm::Optional<llvm::StringRef> LLVMDisassembler::formatInstruction(
    llvm::ArrayRef<uint8_t> bytes,
    uint64_t address,
    uint64_t &size) {
  return impl_->formatInstruction(bytes, address, size);
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_JIT_DISASSEMBLER
