/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEDISASSEMBLER_H
#define HERMES_BCGEN_HBC_BYTECODEDISASSEMBLER_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/DisassemblyOptions.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Inst/InstDecode.h"

#include <unordered_map>

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {
namespace hbc {

using llvh::raw_ostream;
using BigIntID = uint32_t;
using StringID = uint32_t;

/// The reverse of emitOperand, loading an operand encoded in little-endian
/// according to the size.
template <typename T>
void decodeOperand(const uint8_t *operandBuf, T *val) {
  using param_t = int64_t;
  param_t ret = 0;
  for (unsigned i = 0; i < sizeof(T); ++i) {
    ret |= ((param_t)operandBuf[i]) << (i * 8);
  }

  if (std::is_same<T, double>::value) {
    *val = llvh::BitsToDouble(ret);
  } else {
    *val = ret;
  }
}

template <typename... Ts>
std::string formatString(const char *format, Ts... args) {
  size_t size =
      snprintf(nullptr, 0, format, args...) + 1; // Extra space for '\0'
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format, args...);
  return std::string(
      buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

/// Returns the number of elements and tag type at the given position.
std::pair<int, SerializedLiteralGenerator::TagType> checkBufferTag(
    const unsigned char *buff);

/// Base class for walking bytecodes of a function.
class BytecodeVisitor {
 protected:
  std::shared_ptr<hbc::BCProvider> bcProvider_{};
  unsigned funcId_{0};

 protected:
  /// Called before we start walking instructions.
  /// \p funcId: id of the function we will walk.
  /// \p bytecodeStart: the start address of the bytecode content.
  virtual void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) {}

  /// Called after finishing walking instructions.
  virtual void afterStart() {}

  /// Called before visiting a bytecode instruction.
  /// \p opcode: opcode of current instruction.
  /// \p ip: pointer to current instruction in memory.
  /// \p length: length of current instruction.
  virtual void
  preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {}

  /// Called after visiting a bytecode instruction.
  /// \p opcode: opcode of current instruction.
  /// \p ip: pointer to current instruction in memory.
  /// \p length: length of current instruction.
  virtual void
  postVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {}

  /// Called while decoding jump table of SwitchImm instruction.
  /// \p jmpIdx: index into the switch jump table.
  /// \p offset: switch branch's offset relative to current ip.
  /// \p dest: pointer to current switch branch target.
  virtual void
  visitSwitchImmTargets(uint32_t jmpIdx, int32_t offset, const uint8_t *dest) {}

  /// Called while visiting operand.
  /// \p ip: pointer to current instruction in memory.
  /// \p operandType: the type of the visited operand.
  /// \p operandBuf: pointer to the visited operand in memory.
  /// \p operandIndex: index of the visited operand.
  virtual void visitOperand(
      const uint8_t *ip,
      inst::OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) {}

  // Helper function to visit all instructions in a range of bytecode.
  void visitInstructionsInBody(
      const uint8_t *bytecodeStart,
      const uint8_t *bytecodeEnd,
      bool visitSwitchImmTargets);

 public:
  BytecodeVisitor(std::shared_ptr<hbc::BCProvider> bcProvider)
      : bcProvider_(bcProvider) {}

  virtual ~BytecodeVisitor() = default;

  /// Start walking all bytecode instructions for \p funcId.
  void visitInstructionsInFunction(unsigned funcId);
};

using JumpTargetsTy = std::unordered_map<const void *, unsigned>;

/// Visitor to build jump targets table for jump or SwitchImm instructions.
class JumpTargetsVisitor : public BytecodeVisitor {
 private:
  unsigned funcId_ = 0;
  const uint8_t *bytecodeStart_ = nullptr;

  unsigned labelNumber_{0};
  // Remember all SwitchImm so we can loop through their associated jump tables.
  std::vector<inst::Inst const *> switchInsts_{};
  // Maps from jump_target_ip => label_number.
  JumpTargetsTy jumpTargets_{};

 private:
  void createOrSetLabel(const void *dest) {
    auto res = jumpTargets_.emplace(dest, 0);
    if (res.second)
      res.first->second = ++labelNumber_;
  }

 protected:
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) override {
    funcId_ = funcId;
    bytecodeStart_ = bytecodeStart;
  }

  void afterStart() override;

  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length)
      override;

  void visitSwitchImmTargets(
      uint32_t jmpIdx,
      int32_t offset,
      const uint8_t *dest) override {
    createOrSetLabel(dest);
  }

  void visitOperand(
      const uint8_t *ip,
      inst::OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) override;

 public:
  JumpTargetsVisitor(std::shared_ptr<hbc::BCProvider> bcProvider)
      : BytecodeVisitor(bcProvider) {}

  JumpTargetsTy &getJumpTargets() {
    return jumpTargets_;
  }
  std::vector<inst::Inst const *> &getSwitchIntructions() {
    return switchInsts_;
  }
};

/// Visitor to disassemble a function in pretty mode:
/// 1. Display jump label for basic blocks.
/// 2. Decode string type operands.
/// 3. No instruction index, no operand type.
class PrettyDisassembleVisitor : public BytecodeVisitor {
 private:
  inst::OpCode opcode_;
  JumpTargetsTy &jumpTargets_;
  const uint8_t *bytecodeStart_ = nullptr;
  uint32_t funcVirtualOffset_{0};

 protected:
  raw_ostream &os_;
  DisassemblyOptions options_;

 private:
  /// Dump a string table entry referenced by an opcode operand. It is truncated
  /// to about 16 characters (by appending "...") and all non-ASCII values are
  /// escaped.
  void dumpOperandString(StringID stringID, raw_ostream &OS);

  /// Dump the bigint table entry referenced by an opcode operand. It is
  /// truncated to about 8 uint8_t (by appending "...") at the end.
  void dumpOperandBigInt(BigIntID bigintID, raw_ostream &OS);

 protected:
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart);
  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length);
  void
  postVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {
    os_ << "\n";
  }
  void visitOperand(
      const uint8_t *ip,
      inst::OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex);

  /// Get indention for the disassembly output.
  virtual unsigned getIndentation();

  /// Helper method to print source line information for \p opcodeOffset.
  void printSourceLineForOffset(uint32_t opcodeOffset);

 public:
  PrettyDisassembleVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      JumpTargetsTy &jumpTargets,
      raw_ostream &os,
      DisassemblyOptions options = DisassemblyOptions::None)
      : BytecodeVisitor(bcProvider),
        jumpTargets_(jumpTargets),
        os_(os),
        options_(options) {}

  JumpTargetsTy &getJumpTargets() {
    return jumpTargets_;
  }
};

class BytecodeSectionWalker {
 private:
  /// Pointer to the start of the bytecode buffer.
  const uint8_t *bytecodeStart_{nullptr};
  std::shared_ptr<hbc::BCProviderFromBuffer> bcProvider_{};
  llvh::raw_ostream &os_;

  std::vector<const char *> sectionNames_{};
  std::vector<const uint8_t *> sectionStarts_{};
  std::vector<const uint8_t *> sectionEnds_{};

  /// \param start Pointer to the start of the section in bytecode buffer.
  /// \param end Pointer to the end of the section in bytecode buffer.
  template <typename T>
  void addSection(const char *sectionName, const T *start, const T *end) {
    sectionNames_.push_back(sectionName);
    sectionStarts_.push_back(reinterpret_cast<const uint8_t *>(start));
    sectionEnds_.push_back(reinterpret_cast<const uint8_t *>(end));
  }

 public:
  /// Adds names, pointers to start and end of each section in bytecode.
  /// For definitions of bytecode format, see:
  /// hermes/BCGen/HBC/BytecodeFileFormat.h
  /// For order of sections, see:
  /// hermes/lib/BCGen/HBC/BytecodeStream.cpp
  BytecodeSectionWalker(
      const uint8_t *bytecodeStart,
      std::shared_ptr<hbc::BCProviderFromBuffer> bcProvider,
      llvh::raw_ostream &os);

  /// Prints the ranges of each section in the bytecode to the output.
  /// \param human When set to true, prints the range in hex, for ease of
  /// reading by human; when set to false, prints the range in bytes
  void printSectionRanges(bool human);
};

/// This class is used by the hermes frontend.
/// It prints disassembly output of the bytecode list in BCProvider.
class BytecodeDisassembler {
 private:
  std::shared_ptr<hbc::BCProvider> bcProvider_{};

  /// Disassembly output config options.
  DisassemblyOptions options_{DisassemblyOptions::Pretty};

  /// Print information from the bytecode header.
  void disassembleBytecodeFileHeader(raw_ostream &OS);

  /// Print the disassembled bytecode of \p funcId to \p OS.
  void disassembleFunctionRaw(unsigned funcId, raw_ostream &OS);

  /// Print the content of the bytecode list into the output stream \p OS in
  /// a more human-friendly form.
  void disassembleFunctionPretty(unsigned funcId, raw_ostream &OS);

  /// Print the content of the string storage.
  void disassembleStringStorage(raw_ostream &OS);

  /// Print the content of the array buffer table into \p OS.
  void disassembleArrayBuffer(raw_ostream &OS);

  /// Print the content of the bigint storage.
  void disassembleBigIntStorage(raw_ostream &OS);

  /// Print the content of the object buffer table into \p OS.
  void disassembleObjectBuffer(raw_ostream &OS);

  /// Print the contents of the CJS module table to \p OS,
  /// if it contains any entries.
  void disassembleCJSModuleTable(raw_ostream &OS);

  /// Print the contents of the function source table to \p OS,
  /// if it contains any entries.
  void disassembleFunctionSourceTable(raw_ostream &OS);

  /// Print the content of the exception handler table into \p OS.
  void disassembleExceptionHandlers(unsigned funcId, raw_ostream &OS);

  /// Print the content of the exception handler table into \p OS.
  void disassembleExceptionHandlersPretty(
      unsigned funcId,
      const JumpTargetsTy &jumpTargets,
      raw_ostream &OS);

  /// Print the contents of the regexp bytecode table into \p OS.
  void disassembleRegexs(raw_ostream &OS);

  /// Generate RegexIndex => <RegexPattern_StringID, RegexFlag_StringID> map so
  /// that caller can lookup regex's pattern/flag strings from its index.
  /// \p regexStringIDMap is the [out] parameter container for the result.
  std::vector<std::pair<uint32_t, uint32_t>> generateRegexStringIDMap();

 public:
  explicit BytecodeDisassembler(std::shared_ptr<hbc::BCProvider> bcProvider)
      : bcProvider_(bcProvider) {}

  /// Set options for disassembly output.
  void setOptions(DisassemblyOptions options) {
    options_ = options;
  }

  /// Get the current options for disassembly output.
  DisassemblyOptions getOptions() const {
    return options_;
  }

  /// Print the disassembled bytecode of \p funcId to \p OS.
  void disassembleFunction(unsigned funcId, raw_ostream &OS) {
    if ((options_ & DisassemblyOptions::Pretty) == DisassemblyOptions::Pretty) {
      disassembleFunctionPretty(funcId, OS);
    } else {
      disassembleFunctionRaw(funcId, OS);
    }
  }

  /// Print the content of the bytecode list into the output stream \p OS.
  void disassemble(raw_ostream &OS);

  uint32_t getFunctionCount() const {
    return bcProvider_->getFunctionCount();
  }

  /// Hash a function's bytecode in a way that ignores details like table
  /// indices that are easily disturbed by other functions being added/removed.
  uint32_t fuzzyHashBytecode(
      unsigned funcId,
      bool useStrings = true,
      bool useIntConstants = true);
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEDISASSEMBLER_H
