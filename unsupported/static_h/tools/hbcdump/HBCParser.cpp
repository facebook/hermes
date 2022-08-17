/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HBCParser.h"
#include "llvh/Support/MD5.h"

namespace hermes {

using namespace hermes::hbc;
using inst::OpCode;
using inst::OperandType;

/// Visitor to calculate each basic block's static instruction count
/// in a function.
class BasicBlockStaticInstructionCountVisitor
    : public hermes::hbc::BytecodeVisitor {
 private:
  // State field to record current basic block's profile_index.
  uint16_t curProfileIndex_{0};
  // State field to record current basic block's static instruction count.
  uint32_t curBlockStaticInstCount_{0};
  ProfileIndexMap &profileIndexMap_;
  // Maps <profile_index => static_instruction_count>.
  BasicBlockStaticInstCountMap basicBlockStaticInstCountMap_;

 protected:
  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    // Count static instructions of each basic block.
    if (profileIndexMap_.find(ip) != profileIndexMap_.end()) {
      // We are entering a new basic block which ends last block. Let's
      // summarize the last finishing block's static instruction count
      // unless this is the first block of the function which does not have
      // last finishing block (indicated by curBlockStaticInstCount_ == 0).
      if (curBlockStaticInstCount_ != 0) {
        basicBlockStaticInstCountMap_[curProfileIndex_] =
            curBlockStaticInstCount_;
      }
      // Reset state fields for the new block.
      curBlockStaticInstCount_ = 0;
      curProfileIndex_ = profileIndexMap_[ip];
    }
    ++curBlockStaticInstCount_;
  }

  void afterStart() {
    // Since we add a basic block's runtime instruction count while entering a
    // new basic block. We need to add last block's runtime instruction count
    // before wrapping up.
    assert(curBlockStaticInstCount_ > 0 && "Last block should never be empty.");
    basicBlockStaticInstCountMap_[curProfileIndex_] = curBlockStaticInstCount_;
  }

 public:
  BasicBlockStaticInstructionCountVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      ProfileIndexMap &profileIndexMap)
      : BytecodeVisitor(bcProvider), profileIndexMap_(profileIndexMap) {}

  BasicBlockStaticInstCountMap &getStaticInstCountMap() {
    return basicBlockStaticInstCountMap_;
  }
};

BasicBlockStaticInstCountMap &HBCParser::getBasicBlockStaticInstCountMap(
    uint32_t funcId) {
  auto it = funcBasicBlockStaticInstCountMap_.find(funcId);
  if (LLVM_LIKELY(it != funcBasicBlockStaticInstCountMap_.end())) {
    return it->second;
  }
  BasicBlockStaticInstructionCountVisitor staticInstCountVisitor(
      getBCProvider(), getProfileIndexMap(funcId));
  staticInstCountVisitor.visitInstructionsInFunction(funcId);
  return funcBasicBlockStaticInstCountMap_[funcId] =
             staticInstCountVisitor.getStaticInstCountMap();
}

/// Bytecode visitor to build basic block ranges in a function
/// by traversing all branch instructions.
class BasicBlockRangeVisitor : public hermes::hbc::BytecodeVisitor {
 private:
  // Whether current instruction is branch instruction(Jump, SwitchImm) or not.
  bool isBranchInst_{false};
  std::unordered_set<const uint8_t *> basicBlockStartAddresses_{};

 protected:
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) {
    // The first instruction always starts a new basic block.
    basicBlockStartAddresses_.insert(bytecodeStart);
  }

  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    isBranchInst_ = opcode == OpCode::SwitchImm;
  }

  void
  visitSwitchImmTargets(uint32_t jmpIdx, int32_t offset, const uint8_t *dest) {
    basicBlockStartAddresses_.insert(dest);
  }

  void visitOperand(
      const uint8_t *ip,
      OperandType operandType,
      const uint8_t *operandBuf,
      int operandIndex) {
    switch (operandType) {
#define DEFINE_OPERAND_TYPE(name, ctype)                         \
  case OperandType::name: {                                      \
    if (operandType == OperandType::Addr8 ||                     \
        operandType == OperandType::Addr32) {                    \
      ctype operandVal;                                          \
      decodeOperand(operandBuf, &operandVal);                    \
      /* operandVal is relative to current ip.*/                 \
      basicBlockStartAddresses_.insert(ip + (size_t)operandVal); \
      isBranchInst_ = true;                                      \
    }                                                            \
    break;                                                       \
  }
#include "hermes/BCGen/HBC/BytecodeList.def"
    }
  }

  void postVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    // The instruction after jump also starts a new basic block.
    if (isBranchInst_) {
      basicBlockStartAddresses_.insert(ip + length);
    }
  }

 public:
  BasicBlockRangeVisitor(std::shared_ptr<hbc::BCProvider> bcProvider)
      : BytecodeVisitor(bcProvider) {}

  std::unordered_set<const uint8_t *> &getBasicBlockStartAddresses() {
    return basicBlockStartAddresses_;
  }
};

/// Visitor to build bytecode instruction ip => profile_index map.
class ProfileIndexVisitor : public hermes::hbc::BytecodeVisitor {
 private:
  // Whether current basic block has seen a profile instruction or not.
  bool hasSeenProfileInst_{false};
  const uint8_t *curBasicBlockStart_{nullptr};
  std::unordered_set<const uint8_t *> &basicBlockStartAddresses_;
  ProfileIndexMap profileIndexMap_{};

 protected:
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) {
    curBasicBlockStart_ = bytecodeStart;
  }

  void preVisitInstruction(OpCode opcode, const uint8_t *ip, int length) {
    if (basicBlockStartAddresses_.find(ip) != basicBlockStartAddresses_.end()) {
      // Reset all state flags while entering a new basic block.
      curBasicBlockStart_ = ip;
      hasSeenProfileInst_ = false;
    }
    if (opcode == OpCode::ProfilePoint) {
      uint16_t profileIndex = 0;
      decodeOperand(ip + sizeof(opcode), &profileIndex);

      // Profile instruction inside basic block starts a new basic block from
      // profiler trace's point of view.
      if (hasSeenProfileInst_) {
        curBasicBlockStart_ = ip;
      }
      profileIndexMap_[curBasicBlockStart_] = profileIndex;
      hasSeenProfileInst_ = true;
    }
  }

 public:
  ProfileIndexVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      std::unordered_set<const uint8_t *> &basicBlockStartAddresses)
      : BytecodeVisitor(bcProvider),
        basicBlockStartAddresses_(basicBlockStartAddresses) {}

  ProfileIndexMap &getProfileIndexMap() {
    return profileIndexMap_;
  }
};

ProfileIndexMap HBCParser::buildProfileIndexMap(unsigned funcId) {
  // Record the range of all basic blocks from disassembly.
  BasicBlockRangeVisitor rangeVisitor(bcProvider_);
  rangeVisitor.visitInstructionsInFunction(funcId);

  // Assign profile index to each basic block.
  ProfileIndexVisitor profileIndexVisitor(
      bcProvider_, rangeVisitor.getBasicBlockStartAddresses());
  profileIndexVisitor.visitInstructionsInFunction(funcId);
  return profileIndexVisitor.getProfileIndexMap();
}

static llvh::MD5::MD5Result doMD5Checksum(llvh::ArrayRef<uint8_t> bytecode) {
  llvh::MD5 md5;
  llvh::MD5::MD5Result checksum;
  md5.update(bytecode);
  md5.final(checksum);
  return checksum;
}

std::unordered_map<unsigned, std::string>
HBCParser::generateFunctionChecksumMap() {
  std::unordered_map<unsigned, std::string> funcChecksumMap;
  for (unsigned funcId = 0; funcId < bcProvider_->getFunctionCount();
       ++funcId) {
    hbc::RuntimeFunctionHeader functionHeader =
        bcProvider_->getFunctionHeader(funcId);
    const uint8_t *bytecodeStart = bcProvider_->getBytecode(funcId);
    funcChecksumMap[funcId] =
        doMD5Checksum(llvh::ArrayRef<uint8_t>(
                          bytecodeStart, functionHeader.bytecodeSizeInBytes()))
            .digest()
            .str();
  }
  return funcChecksumMap;
}

uint32_t HBCParser::getBasicBlockOffset(
    uint32_t funcId,
    uint16_t profileIndex) {
  auto &funcProfileIndexMap = getProfileIndexMap(funcId);
  auto resultIter = std::find_if(
      funcProfileIndexMap.begin(),
      funcProfileIndexMap.end(),
      [&profileIndex](const std::pair<const uint8_t *, uint16_t> &x) {
        return x.second == profileIndex;
      });
  assert(
      resultIter != funcProfileIndexMap.end() &&
      "Cannot find profileIndex in funcProfileIndexMap.");

  const uint8_t *bytecodeStart = bcProvider_->getBytecode(funcId);
  assert(
      resultIter->first >= bytecodeStart &&
      "Block's start ip should be larger than function start.");
  return resultIter->first - bytecodeStart;
}

} // namespace hermes
