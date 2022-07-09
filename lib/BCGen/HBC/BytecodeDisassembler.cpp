/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeDisassembler.h"

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/Support/JenkinsHash.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Support/SHA1.h"

#include <cstdint>
#include <iomanip>
#include <locale>
#include <string>

#include "llvh/Support/Endian.h"
#include "llvh/Support/ErrorHandling.h"

using namespace hermes::inst;
using SLG = hermes::hbc::SerializedLiteralGenerator;

namespace hermes {
namespace hbc {

enum class BytecodeTable {
  None,
  String,
  BigInt,
};

/// \return a BytecodeTable other than BytecodeTable::None value if
/// \p operandIndex (a zero-based operand index of \p opCode) references an
/// entry in a table in bytecode. BytecodeTable::None is returned otherwise.
static BytecodeTable getBytecodeTableForOperand(
    OpCode opCode,
    unsigned operandIndex) {
#define OPERAND_STRING_ID(name, operandNumber)                     \
  if (opCode == OpCode::name && operandIndex == operandNumber - 1) \
    return BytecodeTable::String;

#define OPERAND_BIGINT_ID(name, operandNumber)                     \
  if (opCode == OpCode::name && operandIndex == operandNumber - 1) \
    return BytecodeTable::BigInt;

#include "hermes/BCGen/HBC/BytecodeList.def"

  return BytecodeTable::None;
}

/// Check if the zero based \p operandIndex in instruction \p opCode is a
/// string table ID.
static bool isOperandStringID(OpCode opCode, unsigned operandIndex) {
  return getBytecodeTableForOperand(opCode, operandIndex) ==
      BytecodeTable::String;
}

std::pair<int, SLG::TagType> checkBufferTag(const unsigned char *buff) {
  auto keyTag = buff[0];
  if (keyTag & 0x80) {
    return std::pair<int, SLG::TagType>{
        ((keyTag & 0x0f) << 8) | (buff[1]), keyTag & SLG::TagMask};
  } else {
    return std::pair<int, SLG::TagType>{keyTag & 0x0f, keyTag & SLG::TagMask};
  }
}

namespace {

std::string SLPToString(SLG::TagType tag, const unsigned char *buff, int *ind) {
  std::string rBracket{"]"};
  switch (tag) {
    case SLG::ByteStringTag: {
      uint8_t val = llvh::support::endian::read<uint8_t, 1>(
          buff + *ind, llvh::support::endianness::little);
      *ind += 1;
      return std::string("[String ") + std::to_string(val) + rBracket;
    }
    case SLG::ShortStringTag: {
      uint16_t val = llvh::support::endian::read<uint16_t, 1>(
          buff + *ind, llvh::support::endianness::little);
      *ind += 2;
      return std::string("[String ") + std::to_string(val) + rBracket;
    }
    case SLG::LongStringTag: {
      uint32_t val = llvh::support::endian::read<uint32_t, 1>(
          buff + *ind, llvh::support::endianness::little);
      *ind += 4;
      return std::string("[String ") + std::to_string(val) + rBracket;
    }
    case SLG::NumberTag: {
      double val = llvh::support::endian::read<double, 1>(
          buff + *ind, llvh::support::endianness::little);
      *ind += 8;
      return std::string("[double ") + std::to_string(val) + rBracket;
    }
    case SLG::IntegerTag: {
      uint32_t val = llvh::support::endian::read<uint32_t, 1>(
          buff + *ind, llvh::support::endianness::little);
      *ind += 4;
      return std::string("[int ") + std::to_string(val) + rBracket;
    }
    case SLG::NullTag:
      return "null";
    case SLG::TrueTag:
      return "true";
    case SLG::FalseTag:
      return "false";
  }
  return "empty";
}

const char *stringKindTag(StringKind::Kind kind) {
  switch (kind) {
    case StringKind::String:
      return "s";
    case StringKind::Identifier:
      return "i";
  }

  llvm_unreachable("Unrecognised String Kind.");
}

} // namespace

void BytecodeDisassembler::disassembleBytecodeFileHeader(raw_ostream &OS) {
  const auto bcopts = bcProvider_->getBytecodeOptions();
  OS << "Bytecode File Information:\n";
  // If the version number in the bytecode file differs from this, the bytecode
  // provider would have exited with an error message.
  OS << "  Bytecode version number: " << hbc::BYTECODE_VERSION << "\n";
  OS << "  Source hash: " << hashAsString(bcProvider_->getSourceHash()) << "\n";
  OS << "  Function count: " << bcProvider_->getFunctionCount() << "\n";
  OS << "  String count: " << bcProvider_->getStringCount() << "\n";
  OS << "  BigInt count: " << bcProvider_->getBigIntCount() << "\n";
  OS << "  String Kind Entry count: " << bcProvider_->getStringKinds().size()
     << "\n";
  OS << "  RegExp count: " << bcProvider_->getRegExpTable().size() << "\n";
  OS << "  Segment ID: " << bcProvider_->getSegmentID() << "\n";
  OS << "  CommonJS module count: " << bcProvider_->getCJSModuleTable().size()
     << "\n";
  OS << "  CommonJS module count (static): "
     << bcProvider_->getCJSModuleTableStatic().size() << "\n";
  OS << "  Function source count: "
     << bcProvider_->getFunctionSourceTable().size() << "\n";
  OS << "  Bytecode options:\n";
  OS << "    staticBuiltins: " << bcopts.staticBuiltins << "\n";
  OS << "    cjsModulesStaticallyResolved: "
     << bcopts.cjsModulesStaticallyResolved << "\n";
  OS << "\n";
}

void BytecodeDisassembler::disassembleStringStorage(raw_ostream &OS) {
  auto strStorage = bcProvider_->getStringStorage();
  auto hashes = bcProvider_->getIdentifierHashes();

  const auto strCount = bcProvider_->getStringCount();
  const auto hashCount = hashes.size();

  if (strCount == 0)
    return;

  auto kinds = bcProvider_->getStringKinds();

  uint32_t strID = 0;
  uint32_t hashID = 0;

  OS << "Global String Table:\n";
  const std::locale loc("C");
  for (auto kindEntry : kinds) {
    for (uint32_t i = 0; i < kindEntry.count(); ++i, ++strID) {
      auto strEntry = bcProvider_->getStringTableEntry(strID);
      OS << stringKindTag(kindEntry.kind()) << strID << "[";

      uint32_t offset = strEntry.getOffset();
      uint32_t length = strEntry.getLength();
      if (strEntry.isUTF16()) {
        OS << "UTF-16";
        length *= 2;
      } else {
        OS << "ASCII";
      }

      int64_t end = static_cast<int64_t>(offset) + length - 1;
      OS << ", " << offset << ".." << end << "]";

      switch (kindEntry.kind()) {
        case StringKind::Identifier:
          OS << " #"
             << llvh::format_hex_no_prefix(
                    hashes[hashID++], 8, /* Upper */ true);
          break;

        default:
          break;
      }

      OS << ": ";
      for (unsigned j = 0; j < length; ++j) {
        unsigned char c = strStorage[offset + j];
        if (!strEntry.isUTF16() && isprint((char)c, loc)) {
          OS << c;
        } else {
          OS << "\\x" << llvh::format_hex_no_prefix(c, 2, true);
        }
      }
      OS << "\n";
    }
  }
  OS << "\n";

  assert(strID == strCount && "Visited all strings.");
  (void)strCount;
  assert(hashID == hashCount && "Visited all hashes.");
  (void)hashCount;
}

/// NOTE: The output might not show the value of every literal used
/// by NewArrayWithBuffer (explained in serializeBuffer's header).
void BytecodeDisassembler::disassembleArrayBuffer(raw_ostream &OS) {
  auto arrayBuffer = bcProvider_->getArrayBuffer();
  if (arrayBuffer.size() == 0)
    return;

  OS << "Array Buffer:\n";
  int ind = 0;
  while ((size_t)ind < arrayBuffer.size()) {
    std::pair<int, SLG::TagType> tag = checkBufferTag(arrayBuffer.data() + ind);
    ind += (tag.first > 0x0f ? 2 : 1);
    for (int i = 0; i < tag.first; i++) {
      OS << SLPToString(tag.second, arrayBuffer.data(), &ind) << "\n";
    }
  }
}

/// NOTE: The output might not show the value of every literal used
/// by NewObjectWithBuffer (explained in serializeBuffer's header).
void BytecodeDisassembler::disassembleObjectBuffer(raw_ostream &OS) {
  auto objKeyBuffer = bcProvider_->getObjectKeyBuffer();
  auto objValueBuffer = bcProvider_->getObjectValueBuffer();
  if (objKeyBuffer.size() == 0)
    return;

  int keyInd = 0;
  int valInd = 0;

  OS << "Object Key Buffer:\n";
  while ((size_t)keyInd < objKeyBuffer.size()) {
    std::pair<int, SLG::TagType> keyTag =
        checkBufferTag(objKeyBuffer.data() + keyInd);
    keyInd += (keyTag.first > 0x0f ? 2 : 1);
    for (int i = 0; i < keyTag.first; i++) {
      OS << SLPToString(keyTag.second, objKeyBuffer.data(), &keyInd) << "\n";
    }
  }

  OS << "Object Value Buffer:\n";
  while ((size_t)valInd < objValueBuffer.size()) {
    std::pair<int, SLG::TagType> valTag =
        checkBufferTag(objValueBuffer.data() + valInd);
    valInd += (valTag.first > 0x0f ? 2 : 1);
    for (int i = 0; i < valTag.first; i++) {
      OS << SLPToString(valTag.second, objValueBuffer.data(), &valInd) << "\n";
    }
  }
}

/// Converts the given bigint magnitude \p bytes to a string in base 10.
static std::string bigintMagnitudeToLengthLimitedString(
    llvh::ArrayRef<uint8_t> bytes) {
  const uint8_t kBase10 = 10;
  std::string value;
  if (LLVM_UNLIKELY(
          bigint::toString(value, bytes, kBase10) !=
          bigint::OperationStatus::RETURNED)) {
    value = "error printing bigint";
  }
  static constexpr size_t LEN_LIMIT = 16;
  const size_t numValueDigits = value.size() - (value[0] == '-' ? 1 : 0);
  const bool tooLong = numValueDigits > LEN_LIMIT;
  if (tooLong) {
    std::stringstream ss;
    ss << value.substr(0, LEN_LIMIT) << "... (" << numValueDigits
       << " decimal digits)";
    value = std::move(ss).str();
  }

  return value;
}

/// Print the content of the bigint storage.
void BytecodeDisassembler::disassembleBigIntStorage(raw_ostream &OS) {
  auto bigintStorage = bcProvider_->getBigIntStorage();
  auto bigintTable = bcProvider_->getBigIntTable();

  const uint32_t bigintCount = bcProvider_->getBigIntCount();

  assert(
      bigintTable.empty() == bigintStorage.empty() &&
      "inconsistent bigint arrays");

  if (bigintTable.empty()) {
    return;
  }

  OS << "Global BigInt Table:\n";

  for (uint32_t i = 0; i < bigintCount; ++i) {
    const auto &entry = bigintTable[i];
    const uint32_t start = entry.offset;
    const uint32_t count = entry.length;
    OS << " " << i << "[";
    if (count == 0) {
      OS << " " << i << "[empty]";
    } else {
      auto bytes = bigintStorage.slice(start, count);
      const uint32_t end = start + count - 1;
      OS << " " << i << "[" << end << ".." << start
         << "]: " << bigintMagnitudeToLengthLimitedString(bytes);
    }
    OS << "\n";
  }

  OS << "\n";
}

void BytecodeDisassembler::disassembleCJSModuleTable(raw_ostream &OS) {
  auto cjsModules = bcProvider_->getCJSModuleTable();
  if (!cjsModules.empty()) {
    OS << "CommonJS Modules:\n";
    for (const auto &pair : cjsModules) {
      OS << "  File ID " << pair.first << " -> function ID " << pair.second
         << '\n';
    }
    OS << '\n';
  }

  auto cjsModulesStatic = bcProvider_->getCJSModuleTableStatic();
  if (!cjsModulesStatic.empty()) {
    OS << "CommonJS Modules (Static):\n";
    for (uint32_t i = 0; i < cjsModulesStatic.size(); ++i) {
      uint32_t moduleID = cjsModulesStatic[i].first;
      uint32_t functionID = cjsModulesStatic[i].second;
      OS << "Module ID " << moduleID << " -> function ID " << functionID
         << '\n';
    }
    OS << '\n';
  }
}

void BytecodeDisassembler::disassembleFunctionSourceTable(raw_ostream &OS) {
  auto functionSources = bcProvider_->getFunctionSourceTable();
  if (!functionSources.empty()) {
    OS << "Function Source Table:\n";
    for (const auto &pair : functionSources) {
      OS << "  Function ID " << pair.first << " -> s" << pair.second << '\n';
    }
    OS << '\n';
  }
}

void BytecodeDisassembler::disassembleExceptionHandlers(
    unsigned funcId,
    raw_ostream &OS) {
  auto funcExceptionHandlers = bcProvider_->getExceptionTable(funcId);
  if (funcExceptionHandlers.size() == 0)
    return;
  OS << "Exception Handlers:\n";
  for (unsigned i = 0, e = funcExceptionHandlers.size(); i < e; ++i) {
    const auto &entry = funcExceptionHandlers[i];
    OS << i << ": start = " << entry.start << ", end = " << entry.end
       << ", target = " << entry.target << "\n";
  }
  OS << "\n";
}

void BytecodeDisassembler::disassembleExceptionHandlersPretty(
    unsigned funcId,
    const JumpTargetsTy &jumpTargets,
    raw_ostream &OS) {
  auto funcExceptionHandlers = bcProvider_->getExceptionTable(funcId);
  if (funcExceptionHandlers.size() == 0)
    return;

  const uint8_t *bytecodeStart = bcProvider_->getBytecode(funcId);

  OS << "Exception Handlers:\n";
  for (unsigned i = 0, e = funcExceptionHandlers.size(); i < e; ++i) {
    const auto &entry = funcExceptionHandlers[i];
    OS << i << ": start = L" << jumpTargets.at(bytecodeStart + entry.start)
       << ", end = L" << jumpTargets.at(bytecodeStart + entry.end)
       << ", target = L" << jumpTargets.at(bytecodeStart + entry.target)
       << "\n";
  }
  OS << "\n";
}

namespace {

/// Given a SwitchImm instruction, loop through each entry of the associated
/// jump table.
/// F: (current index into primary jump table, jump target offset, destination
/// instruction) -> void.
template <typename F>
void switchJumpTableForEach(const inst::Inst *inst, F f) {
  assert(inst->opCode == inst::OpCode::SwitchImm && "expected SwitchImm");
  unsigned start = inst->iSwitchImm.op4;
  unsigned end = inst->iSwitchImm.op5;
  assert(start < end);
  unsigned numberOfEntries = end - start;

  /// Get the current SwitchImm instruction's subview [start, end] start pointer
  /// from primary jump table. This is the same computation done by the
  /// interpreter to figure out the start of the jump table view.
  const auto *curJmpTableView =
      reinterpret_cast<const uint32_t *>(llvh::alignAddr(
          (const uint8_t *)inst + inst->iSwitchImm.op2, sizeof(uint32_t)));

  for (unsigned curJmpTableViewOffset = 0;
       curJmpTableViewOffset <= numberOfEntries;
       curJmpTableViewOffset++) {
    auto jumpTargetOffset = curJmpTableView[curJmpTableViewOffset];
    f(curJmpTableViewOffset + start,
      jumpTargetOffset,
      (const uint8_t *)inst + jumpTargetOffset);
  }
}
} // namespace

void BytecodeVisitor::visitInstructionsInFunction(unsigned funcId) {
  funcId_ = funcId;
  RuntimeFunctionHeader functionHeader = bcProvider_->getFunctionHeader(funcId);
  const uint8_t *bytecodeStart = bcProvider_->getBytecode(funcId);
  const uint8_t *bytecodeEnd =
      bytecodeStart + functionHeader.bytecodeSizeInBytes();

  beforeStart(funcId, bytecodeStart);
  visitInstructionsInBody(
      bytecodeStart, bytecodeEnd, /* visitSwitchImmTargets = */ true);
  afterStart();
} // namespace hbc

void BytecodeVisitor::visitInstructionsInBody(
    const uint8_t *bytecodeStart,
    const uint8_t *bytecodeEnd,
    bool visitSwitchImmTargets) {
  auto ip = bytecodeStart;
  while (ip < bytecodeEnd) {
    const auto md = inst::getInstMetaData(
        (reinterpret_cast<const inst::Inst *>(ip))->opCode);
    OpCode op = md.opCode;
    auto instLength = md.size;
    preVisitInstruction(md.opCode, ip, instLength);

    // Visit branch targets of the SwitchImm instruction.
    if (op == OpCode::SwitchImm && visitSwitchImmTargets) {
      switchJumpTableForEach(
          (inst::Inst const *)ip,
          [this](uint32_t jmpIdx, int32_t offset, const uint8_t *dest) {
            this->visitSwitchImmTargets(jmpIdx, offset, dest);
          });
    }

    const uint8_t *operandBuf = ip + sizeof(op);
    int operandCount = md.numOperands;
    for (int operandIndex = 0; operandIndex < operandCount; operandIndex++) {
      auto operandType = md.operandType[operandIndex];

      visitOperand(ip, operandType, operandBuf, operandIndex);
      operandBuf += getOperandSize(operandType);
    }
    postVisitInstruction(op, ip, instLength);
    ip += instLength;
  }
}

class BytecodeHasher : public BytecodeVisitor {
 protected:
  uint32_t hash_{0};
  bool useStrings_;
  bool useIntConstants_;
  uint8_t opcode_{0xff};

  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length)
      override {
    hash_ = updateJenkinsHash(hash_, opcode);
    opcode_ = static_cast<uint8_t>(opcode);
  }

  void visitOperand(
      const uint8_t *ip,
      OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) override {
    const bool isStringID =
        isOperandStringID(static_cast<OpCode>(opcode_), operandIndex);
    switch (operandType) {
#define DEFINE_OPERAND_TYPE(name, ctype)                \
  case OperandType::name: {                             \
    ctype operandVal;                                   \
    decodeOperand(operandBuf, &operandVal);             \
    if (useStrings_ && isStringID) {                    \
      hashOperandString(operandVal);                    \
    } else if (                                         \
        useIntConstants_ &&                             \
        (operandType == OperandType::Imm32 ||           \
         opcode_ == (uint8_t)OpCode::LoadConstUInt8)) { \
      hashImmediate(operandVal);                        \
    }                                                   \
    break;                                              \
  }

#include "hermes/BCGen/HBC/BytecodeList.def"
    }
  }

  void hashOperandString(StringID stringID) {
    auto strStorage = bcProvider_->getStringStorage();
    auto entry = bcProvider_->getStringTableEntry(stringID);

    auto stringBegin = strStorage.begin() + entry.getOffset();
    auto stringEnd = stringBegin + entry.getLength();

    if (entry.isUTF16()) {
      for (auto from = stringBegin; from < stringEnd; from += 2) {
        hash_ =
            updateJenkinsHash(hash_, *reinterpret_cast<const char16_t *>(from));
      }
    } else {
      for (auto from = stringBegin; from < stringEnd; ++from) {
        hash_ = updateJenkinsHash(hash_, *from);
      }
    }
  }

  void hashImmediate(uint32_t imm) {
    while (imm) {
      hash_ = updateJenkinsHash(hash_, static_cast<uint8_t>(imm));
      imm >>= 8;
    }
  }

 public:
  BytecodeHasher(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      bool useStrings,
      bool useIntConstants)
      : BytecodeVisitor(bcProvider),
        useStrings_(useStrings),
        useIntConstants_(useIntConstants) {}

  uint32_t getHash() const {
    return hash_;
  }
};

uint32_t BytecodeDisassembler::fuzzyHashBytecode(
    unsigned funcId,
    bool useStrings,
    bool useIntConstants) {
  BytecodeHasher hasher(bcProvider_, useStrings, useIntConstants);
  hasher.visitInstructionsInFunction(funcId);
  return hasher.getHash();
}

void JumpTargetsVisitor::afterStart() {
  for (const auto &entry : bcProvider_->getExceptionTable(funcId_)) {
    createOrSetLabel(bytecodeStart_ + entry.start);
    createOrSetLabel(bytecodeStart_ + entry.end);
    createOrSetLabel(bytecodeStart_ + entry.target);
  }
}

void JumpTargetsVisitor::preVisitInstruction(
    OpCode opcode,
    const uint8_t *ip,
    int length) {
  switch (opcode) {
    case OpCode::SwitchImm:
      // Decode jump table of SwitchImm instruction.
      switchInsts_.push_back((inst::Inst const *)ip);
      break;

    case OpCode::Ret:
    case OpCode::Throw:
    case OpCode::Jmp:
    case OpCode::JmpLong:
      createOrSetLabel(ip + length);
      break;

    default:
      break;
  }
}

void JumpTargetsVisitor::visitOperand(
    const uint8_t *ip,
    OperandType operandType,
    const uint8_t *operandBuf,
    int operandIndex) {
  switch (operandType) {
#define DEFINE_OPERAND_TYPE(name, ctype)          \
  case OperandType::name: {                       \
    if (operandType == OperandType::Addr8 ||      \
        operandType == OperandType::Addr32) {     \
      ctype operandVal;                           \
      decodeOperand(operandBuf, &operandVal);     \
      /* operandVal is relative to current ip.*/  \
      createOrSetLabel(ip + (int32_t)operandVal); \
    }                                             \
    break;                                        \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"
  }
}

void PrettyDisassembleVisitor::dumpOperandBigInt(
    BigIntID bigintID,
    raw_ostream &OS) {
  auto bigintStorage = bcProvider_->getBigIntStorage();
  const auto &entry = bcProvider_->getBigIntTable()[bigintID];

  const uint32_t count = entry.length;
  const uint32_t start = entry.offset;

  OS << bigintMagnitudeToLengthLimitedString(bigintStorage.slice(start, count))
     << "n";
}

/// Dump a string table entry referenced by an opcode operand. It is truncated
/// to about 16 characters (by appending "...") and all non-ASCII values are
/// escaped.
void PrettyDisassembleVisitor::dumpOperandString(
    StringID stringID,
    raw_ostream &OS) {
  // After this limit we truncate the string.
  static constexpr unsigned LEN_LIMIT = 16;
  unsigned len = 0;

  os_ << '"';
  auto strStorage = bcProvider_->getStringStorage();
  auto entry = bcProvider_->getStringTableEntry(stringID);

  auto stringBegin = strStorage.begin() + entry.getOffset();
  auto stringEnd = stringBegin + entry.getLength();

  if (entry.isUTF16()) {
    for (auto from = stringBegin; from < stringEnd; from += 2) {
      if (len > LEN_LIMIT) {
        OS << "\"...";
        return;
      }

      uint16_t cp = *(const uint16_t *)from;
      if (cp == '"') {
        OS << "\\\"";
        len += 2;
        continue;
      }
      if (cp < 32) {
        OS << "\\x" << llvh::format_hex_no_prefix(cp, 2);
        len += 4;
        continue;
      }
      if (cp > 127) {
        OS << "\\u" << llvh::format_hex_no_prefix(cp, 4);
        len += 6;
        continue;
      }
      OS << (char)cp;
      ++len;
    }
  } else {
    for (auto from = stringBegin; from < stringEnd; ++from) {
      if (len > LEN_LIMIT) {
        OS << "\"...";
        return;
      }
      OS << *from;
      ++len;
    }
  }

  OS << '"';
}

unsigned PrettyDisassembleVisitor::getIndentation() {
  return 0;
}

void PrettyDisassembleVisitor::beforeStart(
    unsigned funcId,
    const uint8_t *bytecodeStart) {
  bytecodeStart_ = bytecodeStart;
  funcVirtualOffset_ = bcProvider_->getVirtualOffsetForFunction(funcId);
  // Print source line for the function.
  printSourceLineForOffset(0);
}

void PrettyDisassembleVisitor::preVisitInstruction(
    OpCode opcode,
    const uint8_t *ip,
    int length) {
  opcode_ = opcode;
  auto label = jumpTargets_.find(ip);
  assert(ip >= bytecodeStart_ && "Why is ip less than bytecodeStart_?");
  uint32_t offset = ip - bytecodeStart_;
  if (label != jumpTargets_.end()) {
    os_ << "L" << label->second << ":\n";
    printSourceLineForOffset(offset);
    // Use the overrided indention for next line's output.
    os_ << llvh::left_justify("", getIndentation());
  }
  uint32_t globalVirtualOffset = funcVirtualOffset_ + offset;
  if ((options_ & DisassemblyOptions::IncludeVirtualOffsets) ==
      DisassemblyOptions::IncludeVirtualOffsets) {
    os_ << "    ";
    os_ << llvh::right_justify(formatString("%d", globalVirtualOffset), 10);
  }
  os_ << "    ";
  os_ << llvh::left_justify(getOpCodeString(opcode), 17);
}

void PrettyDisassembleVisitor::visitOperand(
    const uint8_t *ip,
    OperandType operandType,
    const uint8_t *operandBuf,
    int operandIndex) {
  if (operandIndex) {
    os_ << ",";
  }
  os_ << " ";

  // Special handling for CallBuiltin and GetBuiltinClosure.
  if (operandIndex == 1 &&
      (opcode_ == inst::OpCode::CallBuiltin ||
       opcode_ == inst::OpCode::CallBuiltinLong ||
       opcode_ == inst::OpCode::GetBuiltinClosure)) {
    uint8_t builtinIndex;
    decodeOperand(operandBuf, &builtinIndex);
    os_ << '"' << getBuiltinMethodName(builtinIndex) << '"';
    return;
  }

  if (operandType == OperandType::Reg8 || operandType == OperandType::Reg32) {
    os_ << "r";
  }

  const BytecodeTable bytecodeTable =
      getBytecodeTableForOperand(opcode_, operandIndex);

  switch (operandType) {
#define DEFINE_OPERAND_TYPE(name, ctype)                            \
  case OperandType::name: {                                         \
    ctype operandVal;                                               \
    decodeOperand(operandBuf, &operandVal);                         \
    if (operandType == OperandType::Addr8 ||                        \
        operandType == OperandType::Addr32) {                       \
      /* operandVal is relative to current ip.*/                    \
      os_ << "L" << jumpTargets_[ip + (int32_t)operandVal];         \
    } else if (bytecodeTable == BytecodeTable::String) {            \
      dumpOperandString(operandVal, os_);                           \
    } else if (bytecodeTable == BytecodeTable::BigInt) {            \
      dumpOperandBigInt(operandVal, os_);                           \
    } else if (operandType == OperandType::Double) {                \
      char buf[hermes::NUMBER_TO_STRING_BUF_SIZE];                  \
      (void)hermes::numberToString(operandVal, buf, sizeof(buf));   \
      os_ << buf;                                                   \
    } else {                                                        \
      /* Trick to print out 1-byte value as int instead of char. */ \
      os_ << +operandVal;                                           \
    }                                                               \
    break;                                                          \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"
  }
}

void PrettyDisassembleVisitor::printSourceLineForOffset(uint32_t opcodeOffset) {
  if ((options_ & DisassemblyOptions::IncludeSource) ==
      DisassemblyOptions::IncludeSource) {
    llvh::Optional<SourceMapTextLocation> sourceLocOpt =
        bcProvider_->getLocationForAddress(funcId_, opcodeOffset);
    if (sourceLocOpt.hasValue()) {
      const std::string &fileNameStr = sourceLocOpt.getValue().fileName;
      os_ << formatString(
                 "%s[%d:%d]",
                 fileNameStr.c_str(),
                 sourceLocOpt.getValue().line,
                 sourceLocOpt.getValue().column)
          << "\n";
    }
  }
}

/// Visitor to disassemble a function in non-pretty mode which
/// does not display jump labels and decode string operands.
class DisassembleVisitor : public BytecodeVisitor {
 private:
  raw_ostream &os_;
  std::vector<inst::Inst const *> switchInsts_{};

 protected:
  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    int offset = ip - bcProvider_->getBytecode(funcId_);
    assert(offset >= 0);
    os_ << "[@ " << offset << "] " << getOpCodeString(opcode);
    if (opcode == OpCode::SwitchImm) {
      const inst::Inst *inst = (inst::Inst const *)ip;
      switchInsts_.push_back(inst);
    }
  }

  void postVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    os_ << "\n";
  }

  void visitOperand(
      const uint8_t *ip,
      OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) {
    if (operandIndex > 0) {
      os_ << ",";
    }

    switch (operandType) {
// For each operand, we load it based on the size of
// this operand's type. The + sign before the value is a
// trick to print out 1-byte value as int instead of char.
#define DEFINE_OPERAND_TYPE(name, ctype)              \
  case OperandType::name: {                           \
    ctype operandVal;                                 \
    decodeOperand(operandBuf, &operandVal);           \
    os_ << " " << +operandVal << "<" << #name << ">"; \
    break;                                            \
  }

#include "hermes/BCGen/HBC/BytecodeList.def"
    }
  }

 public:
  DisassembleVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      raw_ostream &os)
      : BytecodeVisitor(bcProvider), os_(os) {}

  std::vector<inst::Inst const *> &getSwitchIntructions() {
    return switchInsts_;
  }
};

BytecodeSectionWalker::BytecodeSectionWalker(
    const uint8_t *bytecodeStart,
    std::shared_ptr<hbc::BCProviderFromBuffer> bcProvider,
    llvh::raw_ostream &os)
    : bytecodeStart_(bytecodeStart), bcProvider_(bcProvider), os_(os) {
  const auto *fileHeader =
      reinterpret_cast<const hbc::BytecodeFileHeader *>(bytecodeStart_);
  auto functionHeadersStart = bcProvider->getSmallFunctionHeaders().begin();
  addSection(
      "Function table",
      functionHeadersStart,
      functionHeadersStart + fileHeader->functionCount);
  addSection(
      "String Kinds",
      bcProvider->getStringKinds().begin(),
      bcProvider->getStringKinds().end());
  addSection(
      "Identifier hashes",
      bcProvider->getIdentifierHashes().begin(),
      bcProvider->getIdentifierHashes().end());
  addSection(
      "String table",
      bcProvider->getSmallStringTableEntries().begin(),
      bcProvider->getSmallStringTableEntries().end());
  addSection(
      "Overflow String table",
      bcProvider->getOverflowStringTableEntries().begin(),
      bcProvider->getOverflowStringTableEntries().end());
  addSection(
      "String storage",
      bcProvider->getStringStorage().begin(),
      bcProvider->getStringStorage().end());
  addSection(
      "Array buffer",
      bcProvider->getArrayBuffer().begin(),
      bcProvider->getArrayBuffer().end());
  addSection(
      "Object key buffer",
      bcProvider->getObjectKeyBuffer().begin(),
      bcProvider->getObjectKeyBuffer().end());
  addSection(
      "Object value buffer",
      bcProvider->getObjectValueBuffer().begin(),
      bcProvider->getObjectValueBuffer().end());
  addSection(
      "BigInt storage",
      bcProvider->getBigIntStorage().begin(),
      bcProvider->getBigIntStorage().end());
  addSection(
      "Regular expression table",
      bcProvider->getRegExpTable().begin(),
      bcProvider->getRegExpTable().end());
  addSection(
      "Regular expression storage",
      bcProvider->getRegExpStorage().begin(),
      bcProvider->getRegExpStorage().end());
  addSection(
      "CommonJS module table",
      bcProvider->getCJSModuleTable().begin(),
      bcProvider->getCJSModuleTable().end());

  auto firstFuncStart = bcProvider->getBytecode(0);
  auto firstFuncHeader = bcProvider->getFunctionHeader(0);
  auto firstFuncInfoStart = bytecodeStart + firstFuncHeader.infoOffset();
  auto debugInfoStart = bytecodeStart + fileHeader->debugInfoOffset;
  addSection("Function body", firstFuncStart, firstFuncInfoStart);
  addSection("Function info", firstFuncInfoStart, debugInfoStart);
  addSection(
      "Debug info", debugInfoStart, bytecodeStart + fileHeader->fileLength);

  assert(
      sectionNames_.size() == sectionStarts_.size() &&
      sectionStarts_.size() == sectionEnds_.size());
}

void BytecodeSectionWalker::printSectionRanges(bool human) {
  os_ << "Byte range of each section in bytecode:\n";
  for (unsigned i = 0; i < sectionNames_.size(); ++i) {
    os_ << sectionNames_[i] << ": [";
    std::stringstream ss;
    if (human) {
      ss << "0x" << std::hex << std::setfill('0')
         << sectionStarts_[i] - bytecodeStart_ << ", "
         << "0x" << std::hex << std::setfill('0')
         << sectionEnds_[i] - bytecodeStart_ << ")\n";
    } else {
      ss << sectionStarts_[i] - bytecodeStart_ << ", "
         << sectionEnds_[i] - bytecodeStart_ << ")\n";
    }
    os_ << ss.str();
  }
}

/// Visitor to build regex pattern/flag string table.
class RegexStringTableVisitor : public BytecodeVisitor {
 private:
  /// RegexIndex => <RegexPattern_StringID, RegexFlag_StringID> map.
  std::vector<std::pair<uint32_t, uint32_t>> regexStringIDMap_;

 protected:
  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    if (opcode == OpCode::CreateRegExp) {
      assert(
          getInstMetaData(OpCode::CreateRegExp).numOperands == 4 &&
          "CreateRegExp should have 4 operands.");
      uint32_t patternStringId, flagStringId, regexId;
      auto patternOperandBuffer = ip + sizeof(opcode) + sizeof(uint8_t);
      decodeOperand(patternOperandBuffer, &patternStringId);
      decodeOperand(
          patternOperandBuffer + sizeof(patternStringId), &flagStringId);
      decodeOperand(
          patternOperandBuffer + sizeof(patternStringId) + sizeof(flagStringId),
          &regexId);
      assert(regexId < regexStringIDMap_.size() && "Invalid regex id");
      regexStringIDMap_[regexId].first = patternStringId;
      regexStringIDMap_[regexId].second = flagStringId;
    }
  }

 public:
  RegexStringTableVisitor(std::shared_ptr<hbc::BCProvider> bcProvider)
      : BytecodeVisitor(bcProvider) {
    regexStringIDMap_.resize(bcProvider_->getRegExpTable().size());
  }

  std::vector<std::pair<uint32_t, uint32_t>> &getRegexStringIDMap() {
    return regexStringIDMap_;
  }
};

void BytecodeDisassembler::disassembleFunctionPretty(
    unsigned funcId,
    raw_ostream &OS) {
  // Build jump targets table so that we can map from label address to label
  // number.
  JumpTargetsVisitor jumpVisitor(bcProvider_);
  jumpVisitor.visitInstructionsInFunction(funcId);

  auto &jumpTargets = jumpVisitor.getJumpTargets();
  PrettyDisassembleVisitor disassembleVisitor(
      bcProvider_, jumpTargets, OS, options_);
  disassembleVisitor.visitInstructionsInFunction(funcId);

  // Print out switch jump tables, if any.
  auto &switchInsts = jumpVisitor.getSwitchIntructions();
  if (!switchInsts.empty()) {
    OS << "\n "
       << "Jump Tables: \n";
    for (auto *inst : switchInsts) {
      OS << "  "
         << "offset " << inst->iSwitchImm.op2 << "\n";
      switchJumpTableForEach(
          inst, [&](uint32_t jmpIdx, int32_t offset, const uint8_t *dest) {
            OS << "   " << jmpIdx << " : "
               << "L" << jumpTargets[dest] << "\n";
          });
    }
  }

  OS << "\n";
  disassembleExceptionHandlersPretty(funcId, jumpTargets, OS);
}

void BytecodeDisassembler::disassembleFunctionRaw(
    unsigned funcId,
    raw_ostream &OS) {
  DisassembleVisitor disassembleVisitor(bcProvider_, OS);
  disassembleVisitor.visitInstructionsInFunction(funcId);

  // Print out switch jump tables, if any.
  auto &switchInsts = disassembleVisitor.getSwitchIntructions();
  if (!switchInsts.empty()) {
    OS << "\n "
       << "Jump Tables: \n";
    for (auto *inst : switchInsts) {
      OS << "  "
         << "offset " << inst->iSwitchImm.op2 << "\n";
      switchJumpTableForEach(
          inst, [&](uint32_t jmpIdx, int32_t offset, const uint8_t *dest) {
            OS << "   " << jmpIdx << " : " << offset << "\n";
          });
    }
  }

  OS << "\n";
  disassembleExceptionHandlers(funcId, OS);
}

std::vector<std::pair<uint32_t, uint32_t>>
BytecodeDisassembler::generateRegexStringIDMap() {
  RegexStringTableVisitor regexVisitor(bcProvider_);
  for (unsigned funcId = 0; funcId < bcProvider_->getFunctionCount();
       ++funcId) {
    regexVisitor.visitInstructionsInFunction(funcId);
  }
  return regexVisitor.getRegexStringIDMap();
}

void BytecodeDisassembler::disassembleRegexs(raw_ostream &OS) {
  auto regexStorage = bcProvider_->getRegExpStorage();
  if (regexStorage.empty()) {
    return;
  }

  std::vector<std::pair<uint32_t, uint32_t>> regexStringIDMap =
      generateRegexStringIDMap();

  OS << "RegExp Bytecodes:\n";
  uint32_t index = 0;
  for (auto &entry : bcProvider_->getRegExpTable()) {
    OS << index << ": /"
       << bcProvider_->getStringRefFromID(regexStringIDMap[index].first) << "/"
       << bcProvider_->getStringRefFromID(regexStringIDMap[index].second)
       << "\n";
    auto bytecode = regexStorage.slice(entry.offset, entry.length);
    dumpRegexBytecode(bytecode, OS);
    ++index;
  }
  OS << '\n';
}

/// Outputs disassembly in a format resembling that of the "objdump" tool.
/// This is meant to be consumed by tools that expect such a format, not
/// for humans to read.
class ObjdumpDisassembleVisitor : public BytecodeVisitor {
 private:
  unsigned funcId_ = 0;
  unsigned funcOffset_ = 0;
  const uint8_t *bytecodeStart_ = nullptr;
  raw_ostream &os_;

  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) override {
    funcId_ = funcId;
    funcOffset_ = bcProvider_->getFunctionHeader(funcId).offset();
    bytecodeStart_ = bytecodeStart;
    os_ << "\n"
        << llvh::format_hex_no_prefix(funcOffset_, 16) << " <_" << funcId
        << ">:\n";
  }

  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length)
      override {
    os_ << llvh::format_hex_no_prefix(ip - bytecodeStart_ + funcOffset_, 8)
        << ":\t";
    for (int i = 0; i < length; ++i)
      os_ << llvh::format_hex_no_prefix(ip[i], 2) << " ";
    // Align/justify to help any humans debugging the output.
    for (int i = length; i < 20; ++i)
      os_ << "   ";
    os_ << llvh::left_justify(getOpCodeString(opcode), 32);
  }

  void postVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length)
      override {
    os_ << "\n";
  }

  void visitOperand(
      const uint8_t *ip,
      inst::OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) override {
    if (operandIndex) {
      os_ << ",";
    }
    os_ << " ";

    switch (operandType) {
#define DEFINE_OPERAND_TYPE(name, ctype)                                     \
  case OperandType::name: {                                                  \
    ctype operandVal;                                                        \
    decodeOperand(operandBuf, &operandVal);                                  \
    if (operandType == OperandType::Addr8 ||                                 \
        operandType == OperandType::Addr32) {                                \
      /* operandVal is relative to current ip.*/                             \
      os_ << llvh::format_hex_no_prefix(                                     \
          ip + (int32_t)operandVal - bytecodeStart_ + funcOffset_, 8);       \
    } else if (operandType == OperandType::Double) {                         \
      uint64_t raw;                                                          \
      memcpy(&raw, operandBuf, sizeof(raw));                                 \
      os_ << "$" << llvh::format_hex(raw, sizeof(raw));                      \
    } else if (                                                              \
        operandType == OperandType::Reg8 ||                                  \
        operandType == OperandType::Reg32) {                                 \
      /* "+" is a trick to print out 1-byte value as int instead of char. */ \
      os_ << "%r" << +operandVal;                                            \
    } else {                                                                 \
      os_ << "$"                                                             \
          << llvh::format_hex(operandVal, getOperandSize(operandType) * 2);  \
    }                                                                        \
    break;                                                                   \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"
    }
  }

 public:
  ObjdumpDisassembleVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      raw_ostream &os)
      : BytecodeVisitor(bcProvider), os_(os) {}

  /// Disassemble a synthetic function with all opcodes with all zero operands.
  void listOpCodes() {
    os_ << "\n"
        << llvh::format_hex_no_prefix((unsigned)-1, 16) << " <_" << (unsigned)-1
        << ">:\n";

    // Synthesize the function body.
    std::vector<uint8_t> bytecode;
    for (uint8_t op = 0; op < (uint8_t)inst::OpCode::_last; ++op) {
      bytecode.push_back(op);
      auto instLength =
          inst::getInstMetaData(static_cast<inst::OpCode>(op)).size;
      bytecode.resize(bytecode.size() + instLength - 1);
    }

    bytecodeStart_ = bytecode.data();
    visitInstructionsInBody(
        bytecode.data(),
        bytecode.data() + bytecode.size(),
        /* visitSwitchImmTargets = */ false);
  }
};

void BytecodeDisassembler::disassemble(raw_ostream &OS) {
  if ((options_ & DisassemblyOptions::Objdump) == DisassemblyOptions::Objdump) {
    OS << "\n" << hashAsString(bcProvider_->getSourceHash()) << ":     ";
    OS << "file format HBC-" << hbc::BYTECODE_VERSION << "\n\n\n";
    OS << "Disassembly of section .text:\n";
    for (unsigned funcId = 0; funcId < bcProvider_->getFunctionCount();
         ++funcId) {
      ObjdumpDisassembleVisitor disassembleVisitor(bcProvider_, OS);
      disassembleVisitor.visitInstructionsInFunction(funcId);
    }
    if ((options_ & DisassemblyOptions::IncludeOpCodeList) ==
        DisassemblyOptions::IncludeOpCodeList) {
      ObjdumpDisassembleVisitor disassembleVisitor(bcProvider_, OS);
      disassembleVisitor.listOpCodes();
    }
    return;
  }

  disassembleBytecodeFileHeader(OS);
  disassembleStringStorage(OS);
  disassembleArrayBuffer(OS);
  disassembleObjectBuffer(OS);
  disassembleBigIntStorage(OS);
  disassembleCJSModuleTable(OS);
  disassembleFunctionSourceTable(OS);

  for (unsigned funcId = 0; funcId < bcProvider_->getFunctionCount();
       ++funcId) {
    RuntimeFunctionHeader functionHeader =
        bcProvider_->getFunctionHeader(funcId);

    auto functionName =
        bcProvider_->getStringRefFromID(functionHeader.functionName());

    StringRef defKindStr{};
    switch (functionHeader.flags().prohibitInvoke) {
      case FunctionHeaderFlag::ProhibitCall:
        defKindStr = "Constructor";
        break;
      case FunctionHeaderFlag::ProhibitConstruct:
        defKindStr = "NCFunction";
        break;
      default:
        defKindStr = "Function";
        break;
    }

    OS << defKindStr << "<" << functionName << ">";
    if ((options_ & DisassemblyOptions::IncludeFunctionIds) ==
        DisassemblyOptions::IncludeFunctionIds) {
      OS << funcId;
    }
    OS << "(" << functionHeader.paramCount() << " params, "
       << functionHeader.frameSize() << " registers, "
       << static_cast<unsigned int>(functionHeader.environmentSize())
       << " symbols)";
    OS << ":\n";

    auto *funcDebugOffsets = bcProvider_->getDebugOffsets(funcId);
    if (functionHeader.flags().hasDebugInfo && funcDebugOffsets != nullptr) {
      OS << "Offset in debug table: source ";
      uint32_t debugSourceOffset = funcDebugOffsets->sourceLocations;
      if (debugSourceOffset == DebugOffsets::NO_OFFSET) {
        OS << "none";
      } else {
        OS << llvh::format_hex(debugSourceOffset, 6);
      }
      OS << ", lexical ";
      uint32_t debugLexicalOffset = funcDebugOffsets->lexicalData;
      if (debugLexicalOffset == DebugOffsets::NO_OFFSET) {
        OS << "none";
      } else {
        OS << llvh::format_hex(debugLexicalOffset, 6);
      }
      OS << '\n';
    }
    disassembleFunction(funcId, OS);
  }
  disassembleRegexs(OS);
  bcProvider_->getDebugInfo()->disassemble(OS);
}

} // namespace hbc
} // namespace hermes
