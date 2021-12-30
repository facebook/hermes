/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/BCGen/HBC/StringKind.h"
#include "hermes/Public/Buffer.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/LEB128.h"
#include "hermes/Support/MemoryBuffer.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

/*
 * hbc-attribute attributes bundle size to each function. Additional tools
 * allow symbolicating and grouping the information into files. Data shared
 * entirely/partially between functions are correctly attributed.
 *
 * The basic principle is to emit a nested type, size and deduplication key
 * for each type of data. For example:
 *
 *     Function   Type                Key    Size
 *     Foo        data:string:entry   1      8
 *     Bar        data:string:entry   1      8
 *     Bar        data:string:entry   2      8
 *
 * The following is assumed to hold:
 *
 *  1. Records of different Type will not overlap.
 *  2. Records of the same Type but different Key will not overlap.
 *  3. Records of the same Type and same Key will completely overlap.
 *  4. Records of the same Type and same Key will have the same Size.
 *
 */

using namespace hermes;
using namespace hermes::hbc;
using namespace hermes::inst;

using llvh::MutableArrayRef;
using llvh::raw_fd_ostream;
using SLG = hermes::hbc::SerializedLiteralGenerator;

/* This tool is highly dependent upon the current bytecode format.
 *
 * If you have added a simple instruction, make sure string parameters (if any)
 * are marked with the OPERAND_STRING_ID macro and bump the version below.
 *
 * If you have added or modified sections, make sure they're counted properly.
 */

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::Positional,
    llvh::cl::desc("Input bundle"),
    llvh::cl::init("-"));

static llvh::cl::opt<std::string>
    OutputFilename("out", llvh::cl::desc("Output file"), llvh::cl::init("-"));

namespace {
template <typename T>
unsigned byteSize(llvh::ArrayRef<T> ref) {
  return ref.size() * sizeof(T);
}

/// Walks the bytecode and outputs usage info.
class UsageCounter : public BytecodeVisitor {
 protected:
  JSONEmitter &emitter_;
  llvh::DenseMap<unsigned, unsigned> virtualOffsets_;
  uintptr_t bundleStart_;

  unsigned currentFuncId_;
  uintptr_t opcodeStart_;
  uintptr_t opcodeEnd_;
  uintptr_t functionEnd_;

  llvh::DenseMap<std::pair<StringRef, unsigned>, unsigned> emitted_;

  /// Indices into the bytecode's string table corresponding to the (exclusive)
  /// end of each string kind entry.
  std::vector<uint32_t> stringKindEnds_;

  void appendRecord(llvh::StringRef type, unsigned dedupKey, unsigned size) {
    assert(size < (2 << 20) && "Abnormally large size!");

    if (size == 0) {
      // Don't bother counting anything of size 0.
      return;
    }

    // Do one pass of deduplication while emitting. This cuts output in half.
    std::pair<StringRef, unsigned> key = {type, dedupKey};
    if (emitted_.count(key)) {
      assert(emitted_[key] == size && "Expected deduped entry to be same size");
      return;
    }
    emitted_[key] = size;

    emitter_.openDict();
    emitter_.emitKeyValue("type", type);
    emitter_.emitKeyValue("dedupKey", dedupKey);
    emitter_.emitKeyValue("size", size);
    emitter_.closeDict();
  }

  /// Emits available location data we have, which is often just virtualOffset.
  void emitFunctionLocation() {
    auto debugInfo = bcProvider_->getDebugInfo();
    auto offsets = bcProvider_->getDebugOffsets(currentFuncId_);

    emitter_.emitKey("location");
    emitter_.openDict();
    if (offsets && offsets->sourceLocations != DebugOffsets::NO_OFFSET) {
      if (auto pos =
              debugInfo->getLocationForAddress(offsets->sourceLocations, 0)) {
        emitter_.emitKeyValue(
            "file", debugInfo->getFilenameByID(pos->filenameId));
        emitter_.emitKeyValue("line", pos->line);
        emitter_.emitKeyValue("column", pos->column);
      }
    }
    emitter_.emitKeyValue("virtualOffset", virtualOffsets_[currentFuncId_]);
    emitter_.emitKeyValue(
        "bytecodeSize",
        bcProvider_->getFunctionHeader(currentFuncId_).bytecodeSizeInBytes());
    emitter_.closeDict();
  }

  /// Emits per-bundle information.
  void emitGlobalInfo() {
    appendRecord("headers:global:bundle", 0, sizeof(BytecodeFileHeader));
    appendRecord("headers:global:debuginfo", 0, sizeof(DebugInfoHeader));
    // FIXME: Some padding is not included.
  }

  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) override {
    currentFuncId_ = funcId;
    emitted_.clear();

    opcodeStart_ = (uintptr_t)bytecodeStart;
    opcodeEnd_ = llvh::alignAddr(
        bytecodeStart +
            bcProvider_->getFunctionHeader(funcId).bytecodeSizeInBytes(),
        sizeof(uint32_t));
    functionEnd_ = opcodeEnd_;

    emitter_.emitKeyValue("functionId", funcId);
    emitFunctionLocation();

    emitter_.emitKey("usage");
    emitter_.openArray();
  }

  void countDebugInfo() {
    // FIXME: Avoid reimplementing this logic.
    auto *offsets = bcProvider_->getDebugOffsets(currentFuncId_);
    if (!offsets)
      return;

    if (offsets->sourceLocations != DebugOffsets::NO_OFFSET) {
      auto data = bcProvider_->getDebugInfo()->viewData().getData();
      auto offset = offsets->sourceLocations;
      int64_t n, trash;
      for (int i = 0; i < 3; i++) {
        offset += readSignedLEB128(data, offset, &n);
      }
      do {
        offset += readSignedLEB128(data, offset, &n);
        if (n == -1)
          break;
        offset += readSignedLEB128(data, offset, &n);
        if (n & 1)
          offset += readSignedLEB128(data, offset, &n);
        offset += readSignedLEB128(data, offset, &trash);
      } while (true);
      appendRecord(
          "debuginfo:sourcelocations",
          offsets->sourceLocations,
          offset - offsets->sourceLocations);
    }

    if (offsets->lexicalData &&
        offsets->lexicalData != DebugOffsets::NO_OFFSET) {
      auto data = bcProvider_->getDebugInfo()->viewData().getData();
      unsigned start = offsets->lexicalData +
          bcProvider_->getDebugInfo()->lexicalDataOffset();
      unsigned offset = start;
      int64_t trash;

      // Read parent id
      offset += readSignedLEB128(data, offset, &trash);

      // Read variable count
      int64_t count;
      offset += readSignedLEB128(data, offset, &count);
      // Read variables
      for (int64_t i = 0; i < count; i++) {
        int64_t stringLength;
        offset += readSignedLEB128(data, offset, &stringLength);
        offset += stringLength;
      }
      appendRecord(
          "debuginfo:lexicaldata", offsets->lexicalData, offset - start);
    }
  }

  void afterStart() override {
    auto header = bcProvider_->getFunctionHeader(currentFuncId_);
    auto exceptionTable = bcProvider_->getExceptionTable(currentFuncId_);

    // We always have a small header, and sometimes a large one too.
    appendRecord(
        "headers:function:small", currentFuncId_, sizeof(SmallFuncHeader));
    if (header.flags().overflowed) {
      appendRecord(
          "headers:function:large", currentFuncId_, sizeof(FunctionHeader));
    }

    countStringLiteral(header.functionName());

    if (header.flags().hasExceptionHandler) {
      // Exception tables are not deduplicated by function.
      appendRecord(
          "headers:exceptions",
          currentFuncId_,
          sizeof(ExceptionHandlerTableHeader));
      appendRecord(
          "bytecode:tables:exception",
          currentFuncId_,
          byteSize(exceptionTable));
    }

    appendRecord(
        "bytecode:instructions", header.offset(), opcodeEnd_ - opcodeStart_);
    appendRecord(
        "bytecode:tables:jump", header.offset(), functionEnd_ - opcodeEnd_);

    countDebugInfo();

    // Assign global headers to the global function
    if (bcProvider_->getGlobalFunctionIndex() == currentFuncId_) {
      emitGlobalInfo();
    }
    emitter_.closeArray();
  }

  void countStringKind(unsigned stringIndex) {
    // Map from string table index to kind index.
    auto it = std::upper_bound(
        stringKindEnds_.begin(), stringKindEnds_.end(), stringIndex);

    assert(it != stringKindEnds_.end() && "String index out of range");
    auto kindIndex = std::distance(stringKindEnds_.begin(), it);
    appendRecord("data:string:kind", kindIndex, sizeof(StringKind::Entry));

    StringKind::Kind kind = bcProvider_->getStringKinds()[kindIndex].kind();
    if (kind != StringKind::String) {
      // Strings whose kind are not "String" are Identifiers and have a
      // translation field.
      appendRecord(
          "data:string:identifier_translation", stringIndex, sizeof(uint32_t));
    }
  }

  void countStringLiteral(unsigned stringIndex) {
    countStringKind(stringIndex);

    auto entry = bcProvider_->getStringTableEntry(stringIndex);
    auto wasLarge = SmallStringTableEntry(entry, 0).isOverflowed();

    // Like functions headers, we have two kinds of string entries.
    appendRecord(
        "data:string:small_entry", stringIndex, sizeof(SmallStringTableEntry));
    if (wasLarge) {
      appendRecord(
          "data:string:overflow_entry",
          stringIndex,
          sizeof(OverflowStringTableEntry));
    }

    auto offset = entry.getOffset();
    auto length = entry.getLength() * (entry.isUTF16() ? 2 : 1);

    // Emit each byte separately for deduplication to allow overlap.
    // We could emit the entire length for non-overlapped strings, but those
    // make up maybe 10% of them so there's little to gain.
    for (unsigned i = 0; i < length; i++) {
      appendRecord("data:string:chars", offset + i, 1);
    }
  }

  void countRegex(uint32_t index) {
    if (index == 0xFFFFFFFF)
      return;

    auto regex = bcProvider_->getRegExpTable()[index];
    appendRecord("data:regex:entry", index, sizeof(RegExpTableEntry));
    appendRecord("data:regex:bytecode", index, regex.length);
  }

  void countSerializedLiteral(
      SLG::TagType tag,
      const unsigned char *buff,
      unsigned int *ind) {
    // FIXME: Avoid duplicating this logic.

    unsigned bundleOffset = (uintptr_t)(*ind + buff - bundleStart_);
    switch (tag) {
      case SLG::ByteStringTag: {
        uint8_t val = llvh::support::endian::read<uint8_t, 1>(
            buff + *ind, llvh::support::endianness::little);
        appendRecord("data:literalbuffer:bytestring", bundleOffset, 1);
        countStringLiteral(val);
        *ind += 1;
      } break;
      case SLG::ShortStringTag: {
        uint16_t val = llvh::support::endian::read<uint16_t, 1>(
            buff + *ind, llvh::support::endianness::little);
        appendRecord("data:literalbuffer:shortstring", bundleOffset, 2);
        countStringLiteral(val);
        *ind += 2;
      } break;
      case SLG::LongStringTag: {
        uint32_t val = llvh::support::endian::read<uint32_t, 1>(
            buff + *ind, llvh::support::endianness::little);
        appendRecord("data:literalbuffer:longstring", bundleOffset, 4);
        countStringLiteral(val);
        *ind += 4;
      } break;
      case SLG::NumberTag: {
        appendRecord("data:literalbuffer:double", bundleOffset, 8);
        *ind += 8;
      } break;
      case SLG::IntegerTag: {
        appendRecord("data:literalbuffer:int", bundleOffset, 4);
        *ind += 4;
      } break;
      case SLG::NullTag:
      case SLG::TrueTag:
      case SLG::FalseTag:
        break;
    }
  }

  void countSerializedLiterals(
      llvh::ArrayRef<unsigned char> array,
      unsigned offset,
      unsigned count) {
    const unsigned char *ptr = array.data();
    unsigned keyInd = offset;
    while (count > 0) {
      std::pair<int, SLG::TagType> keyTag = checkBufferTag(ptr + keyInd);
      auto tagLength = (keyTag.first > 0x0f ? 2 : 1);
      // This could conceivably overlap if the tag+data fits in the middle of
      // an existing 'double' literal, but we'll assume it's uncommon enough.
      appendRecord(
          "data:literalbuffer:tag",
          (unsigned)(uintptr_t)(keyInd + ptr - bundleStart_),
          tagLength);
      keyInd += tagLength;
      for (int i = 0; i < keyTag.first && count; i++) {
        countSerializedLiteral(keyTag.second, ptr, &keyInd);
        count--;
      }
    }
  }

  void visitSwitchImm(const inst::Inst *inst) {
    assert(inst->opCode == inst::OpCode::SwitchImm);

    const auto *curJmpTableView =
        reinterpret_cast<const uint32_t *>(llvh::alignAddr(
            (const uint8_t *)inst + inst->iSwitchImm.op2, sizeof(uint32_t)));

    unsigned start = inst->iSwitchImm.op4;
    unsigned end = inst->iSwitchImm.op5;
    assert(start < end && "Jump table spans negative range");
    unsigned count = end - start + 1;

    uintptr_t newEnd = (uintptr_t)&curJmpTableView[count];
    if (newEnd > functionEnd_)
      functionEnd_ = newEnd;
  }

  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length)
      override {
    auto inst = (inst::Inst const *)ip;

// Count all strings
#define OPERAND_STRING_ID(OP, N)           \
  if (opcode == OpCode::OP) {              \
    countStringLiteral(inst->i##OP.op##N); \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"

    // Count non-string misc references
    switch (opcode) {
      case OpCode::SwitchImm:
        visitSwitchImm(inst);
        break;
      case OpCode::NewObjectWithBuffer:
        countSerializedLiterals(
            bcProvider_->getObjectKeyBuffer(),
            inst->iNewObjectWithBuffer.op4,
            inst->iNewObjectWithBuffer.op3);
        countSerializedLiterals(
            bcProvider_->getObjectValueBuffer(),
            inst->iNewObjectWithBuffer.op5,
            inst->iNewObjectWithBuffer.op3);
        break;
      case OpCode::NewObjectWithBufferLong:
        countSerializedLiterals(
            bcProvider_->getObjectKeyBuffer(),
            inst->iNewObjectWithBufferLong.op4,
            inst->iNewObjectWithBufferLong.op3);
        countSerializedLiterals(
            bcProvider_->getObjectValueBuffer(),
            inst->iNewObjectWithBufferLong.op5,
            inst->iNewObjectWithBufferLong.op3);
        break;
      case OpCode::NewArrayWithBuffer:
        countSerializedLiterals(
            bcProvider_->getArrayBuffer(),
            inst->iNewArrayWithBuffer.op4,
            inst->iNewArrayWithBuffer.op3);
        break;
      case OpCode::NewArrayWithBufferLong:
        countSerializedLiterals(
            bcProvider_->getArrayBuffer(),
            inst->iNewArrayWithBufferLong.op4,
            inst->iNewArrayWithBufferLong.op3);
        break;
      case OpCode::CreateRegExp:
        countRegex(inst->iCreateRegExp.op4);
      default:
        break;
    }
  }

 public:
  UsageCounter(
      std::shared_ptr<BCProvider> bc,
      JSONEmitter &emitter,
      llvh::DenseMap<unsigned, unsigned> offsets,
      uintptr_t bundleStart)
      : BytecodeVisitor(bc),
        emitter_(emitter),
        virtualOffsets_(offsets),
        bundleStart_(bundleStart) {
    unsigned end = 0;
    for (auto entry : bc->getStringKinds()) {
      end += entry.count();
      stringKindEnds_.push_back(end);
    }
  }
};

// Getting all virtual offsets is O(N^2) unless we do them in a single pass.
llvh::DenseMap<unsigned, unsigned> getVirtualOffsets(
    std::shared_ptr<BCProvider> bc) {
  llvh::DenseMap<unsigned, unsigned> map(bc->getFunctionCount());

  unsigned virtualOffset = 0;
  for (unsigned i = 0, e = bc->getFunctionCount(); i < e; i++) {
    auto header = bc->getFunctionHeader(i);
    map[i] = virtualOffset;
    virtualOffset += header.bytecodeSizeInBytes();
  }
  return map;
}

bool attribute(
    std::unique_ptr<llvh::MemoryBuffer> input,
    JSONEmitter &emitter) {
  assert(
      llvh::alignAddr(input->getBufferStart(), sizeof(uint32_t)) ==
      (uintptr_t)input->getBufferStart());

  uintptr_t bundleStart = (uintptr_t)input->getBuffer().data();
  auto hermesBuffer = std::make_unique<hermes::MemoryBuffer>(input.get());
  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::move(hermesBuffer));
  if (!ret.first) {
    llvh::errs() << "Can't deserialize: " << ret.second << "\n";
    return false;
  }
  std::shared_ptr<BCProvider> bc = std::move(ret.first);

  // TODO: Add records for the bytecode header and similar.
  UsageCounter counter(bc, emitter, getVirtualOffsets(bc), bundleStart);
  for (int i = 0, e = bc->getFunctionCount(); i < e; i++) {
    emitter.openDict();
    counter.visitInstructionsInFunction(i);
    emitter.closeDict();
    emitter.endJSONL();
  }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
  llvh::sys::PrintStackTraceOnErrorSignal("hbc-attribute");
  llvh::PrettyStackTraceProgram X(argc, argv);
  llvh::llvm_shutdown_obj Y;
  llvh::cl::ParseCommandLineOptions(
      argc, argv, "Hermes bytecode size attribution tool\n");

  llvh::ErrorOr<std::unique_ptr<llvh::MemoryBuffer>> fileBufOrErr =
      llvh::MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (!fileBufOrErr) {
    llvh::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return 1;
  }

  llvh::Optional<raw_fd_ostream> fileOS;
  if (!OutputFilename.empty()) {
    std::error_code EC;
    fileOS.emplace(OutputFilename.data(), EC, llvh::sys::fs::F_Text);
    if (EC) {
      llvh::errs() << "Error: fail to open file " << OutputFilename << ": "
                   << EC.message() << '\n';
      return 1;
    }
  }

  auto &output = fileOS ? *fileOS : llvh::outs();
  JSONEmitter emitter(output);
  if (!attribute(std::move(fileBufOrErr.get()), emitter)) {
    return 3;
  }
  output.flush();

  return 0;
}
