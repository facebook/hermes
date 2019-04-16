/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/Public/Buffer.h"
#include "hermes/Support/MemoryBuffer.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace hermes;
using namespace hermes::hbc;

using llvm::MutableArrayRef;
using llvm::raw_fd_ostream;

static llvm::cl::opt<std::string> OrderFilename(
    "order",
    llvm::cl::desc("Input order file"));

static llvm::cl::opt<std::string> InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("Input bundle"),
    llvm::cl::init("-"));

static llvm::cl::opt<std::string>
    OutputFilename("out", llvm::cl::desc("Output file"), llvm::cl::init("-"));

namespace {
/// Reads a file with one integer by line into a vector.
bool readOrder(
    std::unique_ptr<llvm::MemoryBuffer> buffer,
    std::vector<unsigned> &result) {
  llvm::line_iterator it(*buffer.get(), true, '#');
  for (; !it.is_at_end(); it++) {
    unsigned number;
    if (it->getAsInteger(10, number) != 0) {
      llvm::errs() << "Order file contains non-integer on line "
                   << it.line_number() << "\n";
      return false;
    }
    result.push_back(number);
  }
  return true;
}

SmallFuncHeader *getSmallHeader(
    llvm::MutableArrayRef<uint8_t> buffer,
    unsigned funcIndex) {
  return reinterpret_cast<SmallFuncHeader *>(
      buffer.data() + sizeof(BytecodeFileHeader) +
      sizeof(SmallFuncHeader) * funcIndex);
}

void updateFunctionOffset(
    llvm::MutableArrayRef<uint8_t> buffer,
    unsigned funcIndex,
    uint32_t newOffset) {
  SmallFuncHeader *header = getSmallHeader(buffer, funcIndex);
  if (header->flags.overflowed) {
    FunctionHeader *largeHeader = reinterpret_cast<FunctionHeader *>(
        buffer.data() + header->getLargeHeaderOffset());
    largeHeader->offset = newOffset;
  } else {
    header->offset = newOffset;
    assert(
        newOffset == (header->offset) &&
        "Reordering caused function offset to go out of bounds for small header");
  }
}

/// Determines the size of a function including its jump tables.
class FunctionSizeDeterminator : public BytecodeVisitor {
 protected:
  uintptr_t opcodeStart_;
  uintptr_t opcodeEnd_;
  uintptr_t functionEnd_;
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) override {
    opcodeStart_ = (uintptr_t)bytecodeStart;
    opcodeEnd_ = llvm::alignAddr(
        bytecodeStart +
            bcProvider_->getFunctionHeader(funcId).bytecodeSizeInBytes(),
        sizeof(uint32_t));
    functionEnd_ = opcodeEnd_;
  }
  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length)
      override {
    if (opcode != inst::OpCode::SwitchImm)
      return;
    auto inst = (inst::Inst const *)ip;

    const auto *curJmpTableView =
        reinterpret_cast<const uint32_t *>(llvm::alignAddr(
            (const uint8_t *)inst + inst->iSwitchImm.op2, sizeof(uint32_t)));

    unsigned start = inst->iSwitchImm.op4;
    unsigned end = inst->iSwitchImm.op5;
    assert(start < end && "Jump table spans negative range");
    unsigned count = end - start + 1;

    uintptr_t newEnd = (uintptr_t)&curJmpTableView[count];
    if (newEnd > functionEnd_)
      functionEnd_ = newEnd;
  }

 public:
  FunctionSizeDeterminator(std::shared_ptr<BCProvider> bc)
      : BytecodeVisitor(bc) {}
  unsigned getSize() {
    return functionEnd_ - opcodeStart_;
  }
};

/// Creates a map from function index to size in bytes (including jump tables).
llvm::DenseMap<unsigned, unsigned> getSizeMap(std::shared_ptr<BCProvider> bc) {
  llvm::DenseMap<unsigned, unsigned> map;
  FunctionSizeDeterminator sizer(bc);
  for (int i = 0, e = bc->getFunctionCount(); i < e; i++) {
    sizer.visitInstructionsInFunction(i);
    map[i] = sizer.getSize();
  }
  return map;
}

/// Expands an order vector to include all integers up to a maximum. Anything
/// not in the order list will be appended sequentially, so the ordering is
/// always stable and deterministic.
bool padOrder(
    const std::vector<unsigned> &order,
    std::vector<unsigned> &result,
    int max) {
  result.clear();
  result.reserve(max);
  llvm::DenseSet<uint32_t> processed;

  for (int fixed : order) {
    if (fixed >= max) {
      llvm::errs() << "Number out of range: " << fixed << "\n";
      return false;
    }
    if (processed.insert(fixed).second) {
      result.push_back(fixed);
    }
  }

  for (int i = 0; i < max; i++) {
    if (!processed.count(i)) {
      result.push_back(i);
    }
  }
  return true;
}

/// Returns a bytecode bundle where the function bytecode is in the specified
/// order. Function IDs are not renumbered, only the backing bytecode is moved
/// around, so this is a highly transparent (and idempotent) operation.
std::unique_ptr<llvm::MemoryBuffer> reorderBundle(
    std::unique_ptr<llvm::MemoryBuffer> input,
    const std::vector<unsigned> &prescribedOrder) {
  assert(
      llvm::alignAddr(input->getBufferStart(), sizeof(uint32_t)) ==
      (uintptr_t)input->getBufferStart());

  std::vector<uint8_t> outputBuffer(input->getBufferSize());
  MutableArrayRef<uint8_t> output{outputBuffer};
  memcpy(output.data(), input->getBufferStart(), output.size());

  auto hermesBuffer = llvm::make_unique<hermes::MemoryBuffer>(input.get());
  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::move(hermesBuffer));
  if (!ret.first) {
    llvm::errs() << "Can't deserialize: " << ret.second << "\n";
    return nullptr;
  }
  std::shared_ptr<BCProvider> bc = std::move(ret.first);
  std::vector<unsigned> order;

  if (!padOrder(prescribedOrder, order, bc->getFunctionCount())) {
    return nullptr;
  }

  llvm::DenseMap<unsigned, unsigned> functionSize = getSizeMap(bc);
  llvm::DenseMap<uint32_t, uint32_t> addressMap;

  unsigned outputOffset = bc->getFunctionHeader(0).offset();
  // Find the smallest offset in case it's not 0 after a previous reordering
  for (int i : order) {
    outputOffset = std::min(outputOffset, bc->getFunctionHeader(i).offset());
  }

  for (int i : order) {
    unsigned inputOffset = bc->getFunctionHeader(i).offset();
    if (addressMap.count(inputOffset) > 0) {
      // This was a deduped function we've already copied.
      updateFunctionOffset(output, i, addressMap[inputOffset]);
      continue;
    }
    addressMap[inputOffset] = outputOffset;
    updateFunctionOffset(output, i, outputOffset);
    memcpy(
        outputOffset + output.data(),
        inputOffset + input->getBufferStart(),
        functionSize[i]);
    outputOffset += functionSize[i];
  }

  return llvm::MemoryBuffer::getMemBufferCopy(
      StringRef((const char *)output.data(), output.size()));
}

} // namespace

int main(int argc, char **argv_) {
  llvm::sys::PrintStackTraceOnErrorSignal("hbc-reorder");
  llvm::PrettyStackTraceProgram X(argc, argv_);
  llvm::SmallVector<const char *, 256> args;
  llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
  if (llvm::sys::Process::GetArgumentVector(
          args, llvm::makeArrayRef(argv_, argc), ArgAllocator)) {
    llvm::errs() << "Failed to get argc and argv.\n";
    return EXIT_FAILURE;
  }
  argc = args.size();
  const char **argv = args.data();
  llvm::llvm_shutdown_obj Y;
  llvm::cl::ParseCommandLineOptions(
      argc, argv, "Hermes bytecode reordering tool\n");

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBufOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (!fileBufOrErr) {
    llvm::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return 1;
  }

  llvm::Optional<raw_fd_ostream> fileOS;
  if (!OutputFilename.empty()) {
    std::error_code EC;
    fileOS.emplace(OutputFilename.data(), EC, llvm::sys::fs::F_None);
    if (EC) {
      llvm::errs() << "Error: fail to open file " << OutputFilename << ": "
                   << EC.message() << '\n';
      return 1;
    }
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> orderBufOrErr =
      llvm::MemoryBuffer::getFile(OrderFilename);

  if (!orderBufOrErr) {
    llvm::errs() << "Error: fail to open file: " << OrderFilename << ": "
                 << orderBufOrErr.getError().message() << "\n";
    return 1;
  }

  std::vector<unsigned> order;
  if (!readOrder(std::move(orderBufOrErr.get()), order)) {
    return 2;
  }

  auto result = reorderBundle(std::move(fileBufOrErr.get()), order);
  if (!result) {
    return 3;
  }

  auto &output = fileOS ? *fileOS : llvm::outs();
  output << result->getBuffer();
  output.flush();
  return 0;
}
