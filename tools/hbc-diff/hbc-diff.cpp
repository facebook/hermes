/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <unordered_map>

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Support/MemoryBuffer.h"

#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#define DEBUG_TYPE "hbc-diff"

using namespace hermes;

enum ExecutionStatus {
  Success,
  ExecutionFailed,
  LoadFileFailed,
};

static std::array<const char *, 15> sectionNames = {
    {"Total",
     "Function headers",
     "Small string table",
     "Overflow string table",
     "String storage",
     "Array buffer",
     "Object key buffer",
     "Object value buffer",
     "Regexp table",
     "Regexp storage",
     "CommonJS module table",
     "CommonJS module table (static)",
     "Function bodies",
     "Function info",
     "Debug info"}};

/// Prints the number of bytes represented by \p size to \p os.
/// \param humanize display sizes in KiB, MiB, GiB instead of just bytes.
static void printBytes(int64_t size, llvh::raw_ostream &os, bool humanize) {
  const uint64_t base = 1024;
  if (size < 0) {
    os << '-';
  }
  uint64_t absSize = std::abs(size);
  if (!humanize || absSize < base) {
    os << absSize << " B";
    return;
  }

  // exponent = log_base(bytes).
  uint64_t exponent = std::log(absSize) / std::log(base);
  if (exponent > 3) {
    os << "Section diff is in the terabyte range.\nFile is corrupt.\n";
    std::exit(ExecutionFailed);
  }
  char prefix = "KMG"[exponent - 1];
  os << llvh::format(
      "%.2f %ciB", (double)absSize / std::pow(base, exponent), prefix);
}

/// Performs a size-diff of each section in two bytecode files
static ExecutionStatus diffFiles(
    const std::vector<std::unique_ptr<llvh::MemoryBuffer>> &fileBufs,
    const std::vector<std::string> &filenames,
    bool humanize) {
  assert(
      fileBufs.size() == 2 && filenames.size() == 2 &&
      "can only diff exactly two files");

  std::vector<std::vector<int64_t>> fileSizes(fileBufs.size());
  std::vector<std::unordered_map<uint32_t, uint32_t>> funcHashToSize(
      fileBufs.size());

  for (uint32_t i = 0; i < fileBufs.size(); ++i) {
    auto buffer = std::make_unique<MemoryBuffer>(fileBufs[i].get());
    const auto *fileHeader =
        reinterpret_cast<const hbc::BytecodeFileHeader *>(buffer->data());

    auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
        std::move(buffer));
    if (!ret.first) {
      llvh::errs() << ret.second << '\n';
      return ExecutionFailed;
    }
    auto bytecode = std::move(ret.first);

    fileSizes[i].push_back(fileHeader->fileLength);
    fileSizes[i].push_back(
        bytecode->getFunctionCount() * sizeof(hbc::SmallFuncHeader));
    fileSizes[i].push_back(
        bytecode->getSmallStringTableEntries().size() *
        sizeof(hbc::SmallStringTableEntry));
    fileSizes[i].push_back(
        bytecode->getOverflowStringTableEntries().size() *
        sizeof(hbc::OverflowStringTableEntry));
    fileSizes[i].push_back(bytecode->getStringStorage().size());
    fileSizes[i].push_back(bytecode->getArrayBuffer().size());
    fileSizes[i].push_back(bytecode->getObjectKeyBuffer().size());
    fileSizes[i].push_back(bytecode->getObjectValueBuffer().size());
    fileSizes[i].push_back(
        bytecode->getRegExpTable().size() * sizeof(RegExpTableEntry));
    fileSizes[i].push_back(bytecode->getRegExpStorage().size());
    fileSizes[i].push_back(
        bytecode->getCJSModuleTable().size() *
        sizeof(std::pair<uint32_t, uint32_t>));
    fileSizes[i].push_back(
        bytecode->getCJSModuleTableStatic().size() *
        sizeof(std::pair<uint32_t, uint32_t>));

    // function bytecode
    auto functionCount = bytecode->getFunctionCount();
    auto start = bytecode->getBytecode(0);
    auto lastFuncStart = start;
    uint32_t lastFuncId = 0;
    for (uint32_t funcId = 0; funcId < functionCount; ++funcId) {
      auto funcStart = bytecode->getBytecode(funcId);
      if (funcStart > lastFuncStart) {
        lastFuncStart = funcStart;
        lastFuncId = funcId;
      }
    }
    auto lastFuncHeader = bytecode->getFunctionHeader(lastFuncId);
    auto lastFuncEnd = lastFuncStart + lastFuncHeader.bytecodeSizeInBytes();
    fileSizes[i].push_back(lastFuncEnd - start);

    // function info, debug info
    auto firstFuncHeader = bytecode->getFunctionHeader(0);
    auto funcInfoStart = firstFuncHeader.infoOffset();
    auto debugInfoStart = fileHeader->debugInfoOffset;
    fileSizes[i].push_back(debugInfoStart - funcInfoStart);
    fileSizes[i].push_back(fileHeader->fileLength - debugInfoStart);

    if (fileSizes[i].size() != sectionNames.size()) {
      llvh::errs() << "Mismatch between size data and size section count\n";
      return ExecutionFailed;
    }

    hbc::BCProvider *raw_bc = bytecode.get();
    hbc::BytecodeDisassembler disas(std::move(bytecode));
    for (uint32_t funcId = 0; funcId < functionCount; ++funcId) {
      uint32_t size = raw_bc->getFunctionHeader(funcId).bytecodeSizeInBytes();
      funcHashToSize[i].insert(
          std::make_pair(disas.fuzzyHashBytecode(funcId), size));
    }
  }

  llvh::outs() << "Increase from " << filenames[0] << " to " << filenames[1]
               << ":\n";
  for (uint32_t j = 0; j < sectionNames.size(); ++j) {
    llvh::outs() << sectionNames[j] << ": ";
    printBytes(fileSizes[1][j] - fileSizes[0][j], llvh::outs(), humanize);
    llvh::outs() << "  (";
    printBytes(fileSizes[0][j], llvh::outs(), humanize);
    llvh::outs() << " -> ";
    printBytes(fileSizes[1][j], llvh::outs(), humanize);
    llvh::outs() << ")\n";
  }

  auto &before = funcHashToSize[0];
  auto &after = funcHashToSize[1];
  // (size, hash) pairs for each seemingly new function.
  std::vector<std::pair<uint32_t, uint32_t>> newHashes;
  for (auto it : after) {
    if (before.count(it.first) == 0) {
      newHashes.emplace_back(it.second, it.first);
    }
  }
  llvh::outs() << newHashes.size() << " of " << after.size()
               << " functions seem new. Largest new sizes:\n";
  sort(newHashes.begin(), newHashes.end());
  reverse(newHashes.begin(), newHashes.end());
  for (unsigned i = 0; i < 10 && i < newHashes.size(); ++i) {
    llvh::outs() << newHashes[i].first << '\n';
  }
  return Success;
}

int main(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    llvh::errs() << "usage: hbc-diff [-h] <filename1> <filename2>"
                 << "\n";
    return ExecutionFailed;
  }
  bool humanize = false;
  std::vector<std::unique_ptr<llvh::MemoryBuffer>> fileBufs{};
  std::vector<std::string> filenames;
  for (int i = 1; i < argc; ++i) {
    if (::strncmp(argv[i], "-h", 2) == 0) {
      humanize = true;
      continue;
    }
    filenames.push_back(argv[i]);
    llvh::ErrorOr<std::unique_ptr<llvh::MemoryBuffer>> fileBufOrErr =
        llvh::MemoryBuffer::getFileOrSTDIN(argv[i]);
    if (!fileBufOrErr) {
      llvh::errs() << "Error! Failed to open file: " << argv[i] << "\n";
      return LoadFileFailed;
    }
    fileBufs.push_back(std::move(fileBufOrErr.get()));
  }

  return diffFiles(std::move(fileBufs), filenames, humanize);
}
#undef DEBUG_TYPE
