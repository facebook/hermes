/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProfileAnalyzer.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/Parser/JSONParser.h"

#include <set>
#include <vector>

// Allow using inst::OpCode as key in unordered_map.
namespace std {
template <>
struct hash<hermes::inst::OpCode> {
  size_t operator()(const hermes::inst::OpCode &v) const {
    return hash<uint8_t>()(static_cast<uint8_t>(v));
  }
};
} // namespace std

namespace hermes {

using namespace hermes::parser;
using namespace hermes::hbc;
using inst::OpCode;

static uint64_t getFunctionRuntimeInstructionCount(
    std::unordered_map<uint16_t, uint64_t> &basicBlockRuntimeInstCountMap) {
  // Function's total runtime instruction count == sum up all basic
  // blocks' runtime instruction counts.
  uint64_t funcRuntimeInstructionCount = 0;
  for (const auto &entry : basicBlockRuntimeInstCountMap) {
    funcRuntimeInstructionCount += entry.second;
  }
  return funcRuntimeInstructionCount;
}

// Return function name for \p funcId.
static std::string getFunctionName(
    std::shared_ptr<hbc::BCProvider> bcProvider,
    uint32_t funcId) {
  hbc::RuntimeFunctionHeader functionHeader =
      bcProvider->getFunctionHeader(funcId);
  llvh::StringRef functionName =
      bcProvider->getStringRefFromID(functionHeader.functionName());
  return functionName.str();
}

/// Visitor to compute all basic blocks' runtime instruction count for a
/// function.
class BasicBlockRuntimeInstructionFrequencyVisitor
    : public hermes::hbc::BytecodeVisitor {
 private:
  // State field to record current basic block's profile_index.
  uint16_t curProfileIndex_{0};
  // State field to record current basic block's static instruction count.
  uint32_t curBlockStaticInstCount_{0};
  // Maps <profile_index => block_execution_count>.
  std::unordered_map<uint16_t, uint64_t> &funcLogData_;
  ProfileIndexMap &profileIndexMap_;
  // Maps <profile_index => runtime_instruction_count>.
  std::unordered_map<uint16_t, uint64_t> basicBlockRuntimeInstCountMap_;

 protected:
  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {
    // Count static instructions of each basic block.
    if (profileIndexMap_.find(ip) != profileIndexMap_.end()) {
      // We are entering a new basic block which ends last block. Let's
      // summerize the last finishing block's runtime instruction count
      // unless this is the first block of the function which does not have
      // last finishing block.(indicated by curBlockStaticInstCount_ == 0).
      if (curBlockStaticInstCount_ != 0) {
        basicBlockRuntimeInstCountMap_[curProfileIndex_] =
            funcLogData_[curProfileIndex_] * curBlockStaticInstCount_;
      }
      // Reset state fields for the new block.
      curBlockStaticInstCount_ = 0;
      curProfileIndex_ = profileIndexMap_[ip];
      assert(
          funcLogData_.find(curProfileIndex_) != funcLogData_.end() &&
          "Why is the profile index not in profile data?");
    }
    ++curBlockStaticInstCount_;
  }

  void afterStart() {
    // Since we add a basic block's runtime instruction count while entering a
    // new basic block. we need to add last block's runtime instruction count
    // before wrapping up.
    assert(curBlockStaticInstCount_ > 0 && "Last block should never be empty.");
    basicBlockRuntimeInstCountMap_[curProfileIndex_] =
        funcLogData_[curProfileIndex_] * curBlockStaticInstCount_;
  }

 public:
  BasicBlockRuntimeInstructionFrequencyVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      std::unordered_map<uint16_t, uint64_t> &funcLogData,
      ProfileIndexMap &profileIndexMap)
      : BytecodeVisitor(bcProvider),
        funcLogData_(funcLogData),
        profileIndexMap_(profileIndexMap) {}

  std::unordered_map<uint16_t, uint64_t> &getBasicBlockRuntimeInstCountMap() {
    return basicBlockRuntimeInstCountMap_;
  }
};

void ProfileAnalyzer::buildFunctionRuntimeStatisticsMapIfNeeded() {
  if (this->funcRuntimeStats_.empty()) {
    this->totalRuntimeInstructionCount_ = 0;
    forEachTracedFunction(
        [this](
            std::shared_ptr<hbc::BCProvider> bcProvider,
            std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
            ProfileIndexMap &profileIndexMap,
            unsigned funcId) {
          BasicBlockRuntimeInstructionFrequencyVisitor blockInstFreqVisitor(
              bcProvider, funcExecInfo, profileIndexMap);
          blockInstFreqVisitor.visitInstructionsInFunction(funcId);

          FunctionRuntimeStatistics &funcStat = this->funcRuntimeStats_[funcId];
          // Use entry basic block's frequency count as function's entry count.
          // Since entry basic block will have biggest profileIndex so we find
          // maximum profile index entry and retrieve its value.
          const auto maxProfileIndexItemIter =
              std::max_element(funcExecInfo.begin(), funcExecInfo.end());
          assert(maxProfileIndexItemIter != funcExecInfo.end());
          funcStat.entryCount = maxProfileIndexItemIter->second;

          funcStat.basicBlockStats =
              blockInstFreqVisitor.getBasicBlockRuntimeInstCountMap();

          this->totalRuntimeInstructionCount_ += funcStat.instFrequency =
              getFunctionRuntimeInstructionCount(funcStat.basicBlockStats);
        });
  }
}

void ProfileAnalyzer::checkAndReportAccuracyForFunction(unsigned funcId) {
  assertTraceAvailable();
  llvh::StringRef funcChecksum = hbcParser_.getFunctionChecksum(funcId);
  auto &executionInfo = profileDataOpt_.getValue().executionInfo;
  assert(executionInfo.find(funcChecksum) != executionInfo.end());
  std::unordered_map<uint16_t, uint64_t> &funcExecInfo =
      executionInfo[funcChecksum];
  // When basic block number overflows(more than 2^16), all overflowed basic
  // block will have profile_index zero so if index zero entry has non-zero
  // hit count it indicates an overflow.
  if (funcExecInfo[0] > 0) {
    std::string functionName =
        getFunctionName(hbcParser_.getBCProvider(), funcId);
    os_ << "Warning: " << (functionName.empty() ? "<N/A>" : functionName) << "("
        << funcId << ") with checksum " << funcChecksum
        << " has overflowed profile index so the result may be inaccurate.\n";
  }
}

ProfileData ProfileAnalyzer::deserializeTrace(
    std::unique_ptr<llvh::MemoryBuffer> profileBuffer) {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  JSONParser jsonParser(factory, profileBuffer->getBuffer(), sm);

  auto checkInvalidTraceObjectAndExit = [](const void *json,
                                           const char *details) {
    if (json == nullptr) {
      llvh::errs() << "Invalid trace file format: " << details << "\n";
      exit(-3);
    }
  };

  auto *json = llvh::dyn_cast<JSONObject>(jsonParser.parse().getValue());
  checkInvalidTraceObjectAndExit(json, "root is not JSONObject");

  // TODO: share BASIC_BLOCK_STAT_VERSION with VM.
  const static int32_t BASIC_BLOCK_STAT_VERSION = 2;
  ProfileData profileData;
  auto *version = llvh::dyn_cast<JSONNumber>(json->at("version"));
  checkInvalidTraceObjectAndExit(
      version, "fail to fetch 'version' entry from root object");
  profileData.version = (uint16_t)version->getValue();
  if (profileData.version != BASIC_BLOCK_STAT_VERSION) {
    llvh::errs() << "Mismatch profile trace version. Expected "
                 << BASIC_BLOCK_STAT_VERSION << " but got "
                 << profileData.version;
    exit(-3);
  }

  auto *pageSize = llvh::dyn_cast<JSONNumber>(json->at("page_size"));
  checkInvalidTraceObjectAndExit(
      pageSize, "fail to fetch 'page_size' entry from root object");
  profileData.pageSize = (uint32_t)pageSize->getValue();

  auto *functions = llvh::dyn_cast<JSONArray>(json->at("functions"));
  checkInvalidTraceObjectAndExit(
      functions, "fail to fetch 'functions' entry from root object");

  for (size_t i = 0; i < functions->size(); ++i) {
    auto *func = llvh::dyn_cast<JSONObject>(functions->at(i));
    checkInvalidTraceObjectAndExit(func, "fail to fetch function entry");
    auto *checksum = llvh::dyn_cast<JSONString>(func->at("checksum"));
    checkInvalidTraceObjectAndExit(checksum, "fail to fetch 'checksum' field");
    auto *basicBlocks = llvh::dyn_cast<JSONArray>(func->at("basic_blocks"));
    checkInvalidTraceObjectAndExit(
        basicBlocks, "fail to fetch 'basic_blocks' field");
    for (size_t i = 0; i < basicBlocks->size(); ++i) {
      auto *basicBlock = llvh::dyn_cast<JSONObject>(basicBlocks->at(i));
      checkInvalidTraceObjectAndExit(
          basicBlock, "fail to fetch basic block entry");
      auto *profileIndex =
          llvh::dyn_cast<JSONNumber>(basicBlock->at("profile_index"));
      checkInvalidTraceObjectAndExit(
          profileIndex, "fail to fetch 'profile_index' field");
      auto *executionCount =
          llvh::dyn_cast<JSONNumber>(basicBlock->at("execution_count"));
      checkInvalidTraceObjectAndExit(
          executionCount, "fail to fetch 'execution_count' field");
      profileData
          .executionInfo[checksum->str()][(uint16_t)profileIndex->getValue()] =
          executionCount->getValue();
    }
  }
  return profileData;
}

/// Visitor to summarize instruction runtime frequency by opcode type.
class InstructionSummaryVisitor : public hermes::hbc::BytecodeVisitor {
 private:
  // State flag to record current basic block's profile_index.
  uint16_t curProfileIndex_{0};
  // Maps <profile_index => execution_count>.
  std::unordered_map<uint16_t, uint64_t> &funcLogData_;
  ProfileIndexMap &profileIndexMap_;
  // Maps <opcode => execution_count>.
  std::unordered_map<OpCode, uint64_t> &instFrequencies_;

 protected:
  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {
    if (profileIndexMap_.find(ip) != profileIndexMap_.end()) {
      curProfileIndex_ = profileIndexMap_[ip];
    }
    assert(
        funcLogData_.find(curProfileIndex_) != funcLogData_.end() &&
        "Why is the profile index not in profile data?");
    instFrequencies_[opcode] += funcLogData_[curProfileIndex_];
  }

 public:
  InstructionSummaryVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      std::unordered_map<uint16_t, uint64_t> &funcLogData,
      ProfileIndexMap &profileIndexMap,
      std::unordered_map<OpCode, uint64_t> &instFrequencies)
      : BytecodeVisitor(bcProvider),
        funcLogData_(funcLogData),
        profileIndexMap_(profileIndexMap),
        instFrequencies_(instFrequencies) {}
};

void ProfileAnalyzer::dumpInstructionStats() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }

  // <instruction opcode => runtime frequency>
  std::unordered_map<OpCode, uint64_t> instSummary;
  forEachTracedFunction(
      [&instSummary](
          std::shared_ptr<hbc::BCProvider> bcProvider,
          std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
          ProfileIndexMap &profileIndexMap,
          unsigned funcId) {
        InstructionSummaryVisitor instSummaryVisitor(
            bcProvider, funcExecInfo, profileIndexMap, instSummary);
        instSummaryVisitor.visitInstructionsInFunction(funcId);
      });
  reportUnmatchedChecksums();

  // Sort the result by instruction frequency in descending order.
  std::vector<std::pair<OpCode, uint64_t>> sortedElements(
      instSummary.begin(), instSummary.end());
  std::sort(
      sortedElements.begin(),
      sortedElements.end(),
      [](std::pair<OpCode, uint64_t> &x, std::pair<OpCode, uint64_t> &y) {
        return x.second > y.second;
      });

  for (const auto &entry : sortedElements) {
    os_ << getOpCodeString(entry.first) << ": " << entry.second << "\n";
  }
}

void ProfileAnalyzer::reportUnmatchedChecksums() {
  if (!this->unusedChecksumsInTrace_.empty()) {
    os_ << "WARNING: Cannot find matching checksum for the following profile entries: \n";
    for (const auto &entry : this->unusedChecksumsInTrace_) {
      os_ << entry << "\n";
    }
  }
}

void ProfileAnalyzer::dumpFunctionStats() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }
  buildFunctionRuntimeStatisticsMapIfNeeded();
  reportUnmatchedChecksums();

  // Copy function stats to vector then sort by instruction frequency in
  // descending order.
  std::vector<std::pair<unsigned, FunctionRuntimeStatistics>> sortedElements;
  std::transform(
      this->funcRuntimeStats_.begin(),
      this->funcRuntimeStats_.end(),
      std::back_inserter(sortedElements),
      [](const std::pair<unsigned, FunctionRuntimeStatistics> &entry) {
        return entry;
      });
  std::sort(
      sortedElements.begin(),
      sortedElements.end(),
      [](std::pair<unsigned, FunctionRuntimeStatistics> &x,
         std::pair<unsigned, FunctionRuntimeStatistics> &y) {
        return x.second.instFrequency > y.second.instFrequency;
      });

  int maxOutputCount = 100;
  // Put function name as the last column because its length varies a lot.
  os_ << llvh::left_justify("Inst(%)", 12)
      << llvh::left_justify("Inst Acc(%)", 12)
      << llvh::left_justify("Inst(#)", 12) << llvh::left_justify("Entry(#)", 12)
      << llvh::left_justify("Size", 12) << llvh::left_justify("Size Acc", 12)
      << llvh::left_justify("Function", 24) << "Source\n";
  std::shared_ptr<hbc::BCProvider> bcProvider = hbcParser_.getBCProvider();

  float funcInstFreqPercentageAcc = 0.0;
  uint32_t funcSizeAcc = 0;
  for (const auto &entry : sortedElements) {
    if (maxOutputCount-- == 0) {
      break;
    }

    const auto funcId = entry.first;
    llvh::Optional<SourceMapTextLocation> funcStartSourceLocOpt =
        hbcParser_.getSourceLocation(funcId, 0);

    uint64_t funcInstFrequency = entry.second.instFrequency;
    std::string funcNameStr = getFunctionName(bcProvider, funcId);

    double funcInstFreqPercentage =
        100.0 * funcInstFrequency / this->totalRuntimeInstructionCount_;
    funcInstFreqPercentageAcc += funcInstFreqPercentage;
    // Print instruction stats.
    os_ << llvh::left_justify(
               formatString("%.3f%%", funcInstFreqPercentage), 12)
        << llvh::left_justify(
               formatString("%.3f%%", funcInstFreqPercentageAcc), 12)
        << llvh::left_justify(std::to_string(funcInstFrequency), 12)
        << llvh::left_justify(std::to_string(entry.second.entryCount), 12);

    auto funcSize = bcProvider->getFunctionHeader(funcId).bytecodeSizeInBytes();
    funcSizeAcc += funcSize;
    os_ << llvh::left_justify(std::to_string(funcSize), 12)
        << llvh::left_justify(std::to_string(funcSizeAcc), 12);

    // Print function name/source.
    if (funcStartSourceLocOpt.hasValue()) {
      const std::string &fileNameStr =
          funcStartSourceLocOpt.getValue().fileName;
      os_ << llvh::left_justify(
                 formatString("%s(%d)", funcNameStr.c_str(), funcId), 24)
          << formatString(
                 "%s[%d:%d]",
                 fileNameStr.c_str(),
                 funcStartSourceLocOpt.getValue().line,
                 funcStartSourceLocOpt.getValue().column);
    } else {
      os_ << formatString("%s(%d)", funcNameStr.c_str(), funcId);
    }
    os_ << "\n";
  }
}

void ProfileAnalyzer::dumpUsedFunctionIDs() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }
  buildFunctionRuntimeStatisticsMapIfNeeded();
  reportUnmatchedChecksums();

  for (const auto &entry : funcRuntimeStats_) {
    os_ << entry.first << "\n";
  }
}

/// Visitor to dump a function's runtime basic block statistics in details.
class FunctionBasicBlockStatsVisitor : public hbc::PrettyDisassembleVisitor {
  ProfileIndexMap &profileIndexMap_;
  // Total runtime instruction count of the profile trace.
  uint64_t totalProfileRuntimeInstructionCount_;
  FunctionRuntimeStatistics &funcStat_;
  std::unordered_map<uint16_t, uint64_t> &funcExecInfo_;
  const uint8_t *bytecodeStart_ = nullptr;

 protected:
  void beforeStart(unsigned funcId, const uint8_t *bytecodeStart) {
    bytecodeStart_ = bytecodeStart;

    hbc::RuntimeFunctionHeader functionHeader =
        bcProvider_->getFunctionHeader(funcId);

    auto functionName =
        bcProvider_->getStringRefFromID(functionHeader.functionName());
    os_ << "Function<" << functionName << ">";
    if (strncmp(functionName.data(), "global", functionName.size()) != 0) {
      os_ << "(" << functionHeader.paramCount() << " params, "
          << functionHeader.frameSize() << " registers, "
          << static_cast<unsigned int>(functionHeader.environmentSize())
          << " symbols)";
    }
    os_ << ":\n";
    os_ << "Percentage in trace: "
        << formatString(
               "%.3f%%",
               100 * funcStat_.instFrequency /
                   (double)totalProfileRuntimeInstructionCount_)
        << "\n";

    // Print header.
    os_ << llvh::left_justify("inst(%)", 12)
        << llvh::left_justify("inst(#)", 12)
        << llvh::left_justify("hits(#)", 12)
        << llvh::left_justify("avg loop(#)", 12) << "bytecode\n";
  }

  unsigned getIndentation() {
    return 48; // Total length of the columns before bytecode output.
  }

  void preVisitInstruction(inst::OpCode opcode, const uint8_t *ip, int length) {
    // Print block runtime statistics at first line of the block.
    if (profileIndexMap_.find(ip) != profileIndexMap_.end()) {
      assert(ip >= bytecodeStart_);
      printSourceLineForOffset(ip - bytecodeStart_);

      uint16_t curProfileIndex_ = profileIndexMap_[ip];
      assert(
          funcStat_.basicBlockStats.find(curProfileIndex_) !=
          funcStat_.basicBlockStats.end());
      uint64_t curBlockRuntimeInstructionCount =
          funcStat_.basicBlockStats[curProfileIndex_];
      double curBlockPercentage = 100 * curBlockRuntimeInstructionCount /
          (double)funcStat_.instFrequency;

      // Average loop count of the basic block per function entry.
      const double averageLoopCount =
          funcExecInfo_[curProfileIndex_] / (double)funcStat_.entryCount;

      assert(funcExecInfo_.find(curProfileIndex_) != funcExecInfo_.end());
      os_ << llvh::left_justify(formatString("%.3f%%", curBlockPercentage), 12)
          << llvh::left_justify(
                 std::to_string(curBlockRuntimeInstructionCount), 12)
          << llvh::left_justify(
                 std::to_string(funcExecInfo_[curProfileIndex_]), 12)
          << llvh::left_justify(formatString("%.1f", averageLoopCount), 12);
    } else {
      // Middle of the block, just indent.
      os_ << llvh::left_justify("", getIndentation());
    }
    PrettyDisassembleVisitor::preVisitInstruction(opcode, ip, length);
  }

 public:
  FunctionBasicBlockStatsVisitor(
      std::shared_ptr<hbc::BCProvider> bcProvider,
      ProfileIndexMap &profileIndexMap,
      uint64_t totalProfileRuntimeInstructionCount,
      FunctionRuntimeStatistics &funcStat,
      std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
      JumpTargetsTy &jumpTargets,
      llvh::raw_ostream &os)
      : PrettyDisassembleVisitor(bcProvider, jumpTargets, os),
        profileIndexMap_(profileIndexMap),
        totalProfileRuntimeInstructionCount_(
            totalProfileRuntimeInstructionCount),
        funcStat_(funcStat),
        funcExecInfo_(funcExecInfo) {}
};

void ProfileAnalyzer::dumpFunctionBasicBlockStat(unsigned funcId) {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }
  assertTraceAvailable();
  llvh::StringRef funcChecksum = hbcParser_.getFunctionChecksum(funcId);
  std::unordered_map<uint16_t, uint64_t> &funcExecInfo =
      profileDataOpt_.getValue().executionInfo[funcChecksum];
  ProfileIndexMap &funcProfileIndexMap = hbcParser_.getProfileIndexMap(funcId);

  auto bcProvider = hbcParser_.getBCProvider();
  hbc::JumpTargetsVisitor jumpVisitor(bcProvider);
  jumpVisitor.visitInstructionsInFunction(funcId);

  BasicBlockRuntimeInstructionFrequencyVisitor blockInstFreqVisitor(
      bcProvider, funcExecInfo, funcProfileIndexMap);
  blockInstFreqVisitor.visitInstructionsInFunction(funcId);

  buildFunctionRuntimeStatisticsMapIfNeeded();
  assert(
      funcRuntimeStats_.find(funcId) != funcRuntimeStats_.end() &&
      "buildFunctionRuntimeStatisticsMapIfNeeded() should have built funcRuntimeStats_.");

  FunctionBasicBlockStatsVisitor funcDetailsVisitor(
      bcProvider,
      funcProfileIndexMap,
      totalRuntimeInstructionCount_,
      funcRuntimeStats_[funcId],
      funcExecInfo,
      jumpVisitor.getJumpTargets(),
      os_);
  funcDetailsVisitor.visitInstructionsInFunction(funcId);
}

void ProfileAnalyzer::dumpBasicBlockStats() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }
  struct BasicBlockRuntimeStatistics {
    BasicBlockRuntimeStatistics(
        uint32_t funcId,
        uint16_t profileIndex,
        uint32_t offset,
        uint64_t hitCount,
        uint64_t runtimeInstCount,
        double avgLoopCount)
        : funcId(funcId),
          profileIndex(profileIndex),
          offset(offset),
          hitCount(hitCount),
          runtimeInstCount(runtimeInstCount),
          avgLoopCount(avgLoopCount) {}
    // Id of the parent function.
    uint32_t funcId{0};
    // Block's profile index.
    uint16_t profileIndex{0};
    // Block's bytecode offset from beginning of function.
    uint32_t offset{0};
    // Block's runtime execution hit count.
    uint64_t hitCount{0};
    // Total number of executed instructions at runtime.
    uint64_t runtimeInstCount{0};
    // Block's average loop count per funciton entry.
    double avgLoopCount{0.0};
  };
  std::vector<BasicBlockRuntimeStatistics> blockRuntimeStats;

  buildFunctionRuntimeStatisticsMapIfNeeded();
  forEachTracedFunction(
      [this, &blockRuntimeStats](
          std::shared_ptr<hbc::BCProvider> bcProvider,
          std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
          ProfileIndexMap &profileIndexMap,
          unsigned funcId) {
        FunctionRuntimeStatistics &funcStat = this->funcRuntimeStats_[funcId];
        for (const auto &entry : funcExecInfo) {
          const uint16_t profileIndex = entry.first;
          double averageLoopCount =
              funcExecInfo[profileIndex] / (double)funcStat.entryCount;
          // Skip overflow block for now.
          // TODO: show overflow block stats as well.
          if (profileIndex == 0) {
            continue;
          }
          uint32_t offset =
              this->hbcParser_.getBasicBlockOffset(funcId, profileIndex);
          blockRuntimeStats.emplace_back(
              funcId,
              profileIndex,
              offset,
              entry.second,
              funcStat.basicBlockStats[profileIndex],
              averageLoopCount);
        }
      });
  std::sort(
      blockRuntimeStats.begin(),
      blockRuntimeStats.end(),
      [](const BasicBlockRuntimeStatistics &x,
         const BasicBlockRuntimeStatistics &y) {
        return x.runtimeInstCount > y.runtimeInstCount;
      });

  int maxOutputCount = 100;
  os_ << llvh::left_justify("Inst(#)", 12) << llvh::left_justify("Inst(%)", 12)
      << llvh::left_justify("Hits(#)", 12)
      << llvh::left_justify("Avg Loop(#)", 12)
      << llvh::left_justify("ProfileIndex", 12)
      << llvh::left_justify("Function", 24) << "Source"
      << "\n";
  for (const auto &entry : blockRuntimeStats) {
    if (maxOutputCount-- == 0) {
      break;
    }
    auto funcId = entry.funcId;
    const auto percentage =
        100 * entry.runtimeInstCount / (double)totalRuntimeInstructionCount_;
    auto funcName = getFunctionName(hbcParser_.getBCProvider(), funcId);

    os_ << llvh::left_justify(std::to_string(entry.runtimeInstCount), 12)
        << llvh::left_justify(formatString("%.3f%%", percentage), 12)
        << llvh::left_justify(std::to_string(entry.hitCount), 12)
        << llvh::left_justify(formatString("%.1f", entry.avgLoopCount), 12)
        << llvh::left_justify(std::to_string(entry.profileIndex), 12)
        << llvh::left_justify(
               formatString("%s(%d)", funcName.c_str(), funcId), 24);

    // Print source location for the block if available.
    llvh::Optional<SourceMapTextLocation> sourceLocOpt =
        hbcParser_.getSourceLocation(funcId, entry.offset);
    if (sourceLocOpt.hasValue()) {
      const std::string &fileNameStr = sourceLocOpt.getValue().fileName;
      os_ << formatString(
          "%s[%d:%d]",
          fileNameStr.c_str(),
          sourceLocOpt.getValue().line,
          sourceLocOpt.getValue().column);
    }
    os_ << "\n";
  }
}

void ProfileAnalyzer::dumpIO() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }
  uint32_t pageSize = profileDataOpt_.getValue().pageSize;
  auto &executionInfo = profileDataOpt_.getValue().executionInfo;
  auto bcProvider = hbcParser_.getBCProvider();

  auto getPageIndexFromOffset = [pageSize](uint32_t offset) {
    return offset / pageSize;
  };

  uint32_t funcCount = bcProvider->getFunctionCount();
  assert(funcCount > 0 && "There should be at least one function.");
  os_ << executionInfo.size() << " functions accessed out of total "
      << funcCount << " functions\n";

  uint32_t funcRegionStartOffset = bcProvider->getFunctionHeader(0).offset();
  uint32_t funcRegionEndOffset =
      bcProvider->getFunctionHeader(funcCount - 1).offset() +
      bcProvider->getFunctionHeader(funcCount - 1).bytecodeSizeInBytes() - 1;

  uint32_t funcRegionStartPage = getPageIndexFromOffset(funcRegionStartOffset);
  uint32_t funcRegionEndPage = getPageIndexFromOffset(funcRegionEndOffset);
  os_ << "Bundle code region occupies total "
      << funcRegionEndPage - funcRegionStartPage + 1 << " pages: ["
      << funcRegionStartPage << " - " << funcRegionEndPage << "]\n";

  std::set<uint32_t> pageIndexSet;
  forEachTracedFunction(
      [&pageIndexSet, getPageIndexFromOffset](
          std::shared_ptr<hbc::BCProvider> bcProvider,
          std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
          ProfileIndexMap &profileIndexMap,
          unsigned funcId) {
        hbc::RuntimeFunctionHeader functionHeader =
            bcProvider->getFunctionHeader(funcId);
        // TODO: check each individual basic block's range instead of function.
        uint32_t startOffset = functionHeader.offset();
        uint32_t endOffset =
            functionHeader.offset() + functionHeader.bytecodeSizeInBytes() - 1;
        uint32_t funcStartPage = getPageIndexFromOffset(startOffset);
        uint32_t funcEndPage = getPageIndexFromOffset(endOffset);
        for (uint32_t i = funcStartPage; i <= funcEndPage; ++i) {
          pageIndexSet.insert(i);
        }
      });
  reportUnmatchedChecksums();

  uint32_t smallestPageIndex = *pageIndexSet.begin();
  uint32_t largestPageIndex = *pageIndexSet.rbegin();

  os_ << "Trace total touched " << pageIndexSet.size() << " pages, spanning "
      << largestPageIndex - smallestPageIndex + 1 << " pages:\n";
  int i = 0;
  const int outputPerRow = 16;
  for (auto index : pageIndexSet) {
    os_ << llvh::left_justify(std::to_string(index), 6);
    if (++i % outputPerRow == 0) {
      os_ << "\n";
    }
  }
  os_ << "\n";
}

void ProfileAnalyzer::dumpEpilogue() {
  llvh::ArrayRef<uint8_t> epilogue = hbcParser_.getBCProvider()->getEpilogue();
  std::string epiStr(
      reinterpret_cast<const char *>(epilogue.data()), epilogue.size());
  os_ << epiStr << "\n";
}

void ProfileAnalyzer::dumpSummary() {
  if (!profileDataOpt_.hasValue()) {
    os_ << "This command requires trace profile to run (-profile-file).\n";
    return;
  }

  assertTraceAvailable();
  uint32_t tracedFuncCount = profileDataOpt_.getValue().executionInfo.size();
  uint32_t totalFuncCount = hbcParser_.getBCProvider()->getFunctionCount();

  struct TraceSummary {
    // Executed unique basic blocks count. If a basic block got more than
    // one hit count, it will only count as one.
    uint64_t blockUniqueCount = 0;
    // Accumulated basic block runtime hit count.
    uint64_t blockAccumulatedCount = 0;
    // Total number of basic blocks in traced function, including un-executed
    // basic blocks.
    uint64_t functTotalBlockCount = 0;
    // Executed basic block's total static instruction count, excluding
    // un-executed blocks.
    uint64_t blockStaticInstCount = 0;
    // Total static instruction count in traced function, including un-executed
    // basic block in traced function.
    uint64_t funcTotalStaticInstCount = 0;
  };
  TraceSummary sumamry;

  using TraceEntry = std::pair<uint16_t, uint64_t>;
  // Count summary statistics data for each traced function.
  forEachTracedFunction(
      [this, &sumamry](
          std::shared_ptr<hbc::BCProvider> bcProvider,
          std::unordered_map<uint16_t, uint64_t> &funcExecInfo,
          ProfileIndexMap &profileIndexMap,
          unsigned funcId) {
        assert(
            funcExecInfo.size() < (1 << 16) &&
            "How can funcExecInfo have more than 2^16 entries?");

        // Executed block will have non-zero frequency.
        sumamry.blockUniqueCount += std::count_if(
            funcExecInfo.begin(),
            funcExecInfo.end(),
            [](const TraceEntry &blockEntry) {
              return blockEntry.second != 0;
            });

        sumamry.blockAccumulatedCount += std::accumulate(
            funcExecInfo.begin(),
            funcExecInfo.end(),
            0,
            [](uint64_t acc, const TraceEntry &item) {
              return acc + item.second;
            });

        // funcExecInfo includes zero-index entry for overflow which we should
        // exclude.
        sumamry.functTotalBlockCount += (funcExecInfo.size() - 1);

        BasicBlockStaticInstCountMap &staticInstCountMap =
            hbcParser_.getBasicBlockStaticInstCountMap(funcId);
        // funcExecInfo should have one more entry than staticInstCountMap(zero
        // ProfileIndex entry) unless ProfileIndex overflowed which they have
        // equal size.
        assert(
            staticInstCountMap.size() + 1 == funcExecInfo.size() ||
            (staticInstCountMap.size() == funcExecInfo.size() &&
             funcExecInfo.size() == (1 << 16)));
        sumamry.blockStaticInstCount += std::accumulate(
            funcExecInfo.begin(),
            funcExecInfo.end(),
            0,
            [&staticInstCountMap](uint64_t acc, const TraceEntry &blockEntry) {
              // Only count this block as executed if its frequency is non-zero.
              return blockEntry.second != 0
                  ? acc + staticInstCountMap[blockEntry.first]
                  : acc;
            });
        sumamry.funcTotalStaticInstCount += std::accumulate(
            staticInstCountMap.begin(),
            staticInstCountMap.end(),
            0,
            [](uint64_t acc, const TraceEntry &entry) {
              return acc + entry.second;
            });
      });
  buildFunctionRuntimeStatisticsMapIfNeeded();
  os_ << "Average instructions per basic block: "
      << llvh::format(
             "%.3f",
             (double)totalRuntimeInstructionCount_ /
                 sumamry.blockAccumulatedCount)
      << "\n"
      << "Total functions in hbc: " << totalFuncCount << "\n"
      << "Executed functions in trace: " << tracedFuncCount << "\n"
      << "Percentage: "
      << llvh::format("%.3f%%", 100.0 * tracedFuncCount / totalFuncCount)
      << "\n"
      << "Total basic block count in traced functions: "
      << sumamry.functTotalBlockCount << "\n"
      << "Executed basic block count: " << sumamry.blockUniqueCount << "\n"
      << "Percentage: "
      << llvh::format(
             "%.3f%%",
             100.0 * sumamry.blockUniqueCount / sumamry.functTotalBlockCount)
      << "\n"
      << "Total traced function static instruction count: "
      << sumamry.funcTotalStaticInstCount << "\n"
      << "Executed basic block static instruction count: "
      << sumamry.blockStaticInstCount << "\n"
      << "Percentage: "
      << llvh::format(
             "%.3f%%",
             100.0 * sumamry.blockStaticInstCount /
                 sumamry.funcTotalStaticInstCount)
      << "\n";
}

void ProfileAnalyzer::dumpFunctionInfo(uint32_t funcId, JSONEmitter &json) {
  auto bcProvider = hbcParser_.getBCProvider();
  if (funcId >= bcProvider->getFunctionCount()) {
    os_ << "FunctionID " << funcId << " is invalid.\n";
    return;
  }

  json.openDict();

  auto header = bcProvider->getFunctionHeader(funcId);
  json.emitKeyValue("FunctionID", funcId);
  json.emitKeyValue("Offset", header.offset());
  json.emitKeyValue(
      "VirtualOffset", bcProvider->getVirtualOffsetForFunction(funcId));
  json.emitKeyValue("Size", header.bytecodeSizeInBytes());
  json.emitKeyValue(
      "Name", bcProvider->getStringRefFromID(header.functionName()));

  auto dbg = bcProvider->getDebugOffsets(funcId);
  if (dbg) {
    if (dbg->sourceLocations != DebugOffsets::NO_OFFSET) {
      json.emitKeyValue("DebugSourceLocation: ", dbg->sourceLocations);
    }
    if (dbg->lexicalData != DebugOffsets::NO_OFFSET) {
      json.emitKeyValue("DebugLexicalData: ", dbg->lexicalData);
    }
  }

  llvh::Optional<SourceMapTextLocation> sourceLocOpt =
      bcProvider->getLocationForAddress(funcId, /* offsetInFunction */ 0);
  if (sourceLocOpt.hasValue()) {
    json.emitKey("FinalSourceLocation");
    json.openDict();
    json.emitKeyValue("Source", sourceLocOpt->fileName);
    json.emitKeyValue("Line", sourceLocOpt->line);
    json.emitKeyValue("Column", sourceLocOpt->column);
    json.closeDict();
    if (sourceMap_) {
      auto originalSourceLoc = sourceMap_->getLocationForAddress(
          sourceLocOpt->line, sourceLocOpt->column);
      if (originalSourceLoc.hasValue()) {
        json.emitKey("OriginalSourceLocation");
        json.openDict();
        json.emitKeyValue("Source", originalSourceLoc->fileName);
        json.emitKeyValue("Line", originalSourceLoc->line);
        json.emitKeyValue("Column", originalSourceLoc->column);
        json.closeDict();
      }
    }
  }

  json.closeDict();
}

llvh::Optional<uint32_t> ProfileAnalyzer::getFunctionFromVirtualOffset(
    uint32_t virtualOffset) {
  auto *bcProvider = hbcParser_.getBCProvider().get();
  uint32_t funcCount = bcProvider->getFunctionCount();

  uint32_t endVirtualOffset = 0;
  for (uint32_t i = 0; i < funcCount; ++i) {
    endVirtualOffset += bcProvider->getFunctionHeader(i).bytecodeSizeInBytes();
    if (virtualOffset < endVirtualOffset) {
      return i;
    }
  }
  return llvh::None;
}

llvh::Optional<uint32_t> ProfileAnalyzer::getFunctionFromOffset(
    uint32_t offset) {
  auto *bcProvider = hbcParser_.getBCProvider().get();
  uint32_t funcCount = bcProvider->getFunctionCount();

  for (uint32_t i = 0; i < funcCount; ++i) {
    auto header = bcProvider->getFunctionHeader(i);
    if (offset >= header.offset() &&
        offset < header.offset() + header.bytecodeSizeInBytes()) {
      return i;
    }
  }
  return llvh::None;
}

} // namespace hermes
