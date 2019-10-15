/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HBCDUMP_PROFILEANALYZER_H
#define HERMES_TOOLS_HBCDUMP_PROFILEANALYZER_H

#include "HBCParser.h"
#include "StructuredPrinter.h"

#include "llvm/Support/raw_ostream.h"

#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace hermes {

/// <func_checksum, <profile_index, execution_count>> hash map.
using ExecutionInfo =
    std::unordered_map<std::string, std::unordered_map<uint16_t, uint64_t>>;

/// Basic block profile trace data.
struct ProfileData {
  // Profile version.
  uint16_t version;
  // Profile target machine page size.
  uint32_t pageSize;
  // Function execution information.
  ExecutionInfo executionInfo;
};

// Function runtime profile statistics.
struct FunctionRuntimeStatistics {
  // Runtime instruction frequency.
  uint64_t instFrequency{0};
  // Entry count for this function.
  uint64_t entryCount{0};
  // Maps profile_index => bb_instruction_frequency.
  std::unordered_map<uint16_t, uint64_t> basicBlockStats;
};

/// Analyzer for basic block profile trace.
class ProfileAnalyzer {
 private:
  llvm::raw_ostream &os_;
  HBCParser hbcParser_;
  llvm::Optional<ProfileData> profileDataOpt_;
  // Total number of executed instruction from this profile trace.
  uint64_t totalRuntimeInstructionCount_{0};
  // Maps from function_id to its runtime statiscs from trace.
  std::unordered_map<unsigned, FunctionRuntimeStatistics> funcRuntimeStats_;
  // Caches any unused function checksums in profile trace.
  std::unordered_set<std::string> unusedChecksumsInTrace_;

  ProfileData deserializeTrace(
      std::unique_ptr<llvm::MemoryBuffer> profileBuffer);

  /// Loop through each traced function entry in profile and call \p f.
  /// \p f: (BCProvider, function_executionInfo, function_profileIndexMap,
  /// function_id) -> void.
  template <typename Callback>
  void forEachTracedFunction(Callback f) {
    const bool checkUnmatchedChecksums = unusedChecksumsInTrace_.empty();
    assertTraceAvailable();
    auto &execInfo = profileDataOpt_.getValue().executionInfo;
    for (auto &infoEntry : execInfo) {
      const std::string &funcChecksum = infoEntry.first;
      std::unordered_map<uint16_t, uint64_t> &funcExecInfo = infoEntry.second;
      llvm::Optional<uint32_t> funcIdOpt =
          hbcParser_.functionIdFromChecksum(funcChecksum);
      if (funcIdOpt.hasValue()) {
        f(hbcParser_.getBCProvider(),
          funcExecInfo,
          hbcParser_.getProfileIndexMap(funcIdOpt.getValue()),
          funcIdOpt.getValue());
      } else if (checkUnmatchedChecksums) {
        unusedChecksumsInTrace_.insert(funcChecksum);
      }
    }
  }

  // Report any unmatched trace entry in profile file.
  void reportUnmatchedChecksums();

  void assertTraceAvailable() {
    assert(profileDataOpt_.hasValue() && "profileDataOpt_ is empty.");
  }

  /// Build and cache each function's runtime statistics map if not built yet.
  void buildFunctionRuntimeStatisticsMapIfNeeded();

  /// Check if  funcId has any basic block number overflow(more than 2^16
  /// blocks) and report it.
  void checkAndReportAccuracyForFunction(unsigned funcId);

  /// Check the trace for basic block number overflow(more than 2^16 blocks) and
  /// report it.
  void checkAndReportAccuracy() {
    assertTraceAvailable();
    auto &profileData = profileDataOpt_.getValue();
    for (const auto &entry : profileData.executionInfo) {
      llvm::Optional<unsigned> funcIdOpt =
          hbcParser_.functionIdFromChecksum(entry.first);
      if (funcIdOpt.hasValue()) {
        checkAndReportAccuracyForFunction(funcIdOpt.getValue());
      }
    }
  }

 public:
  ProfileAnalyzer(
      llvm::raw_ostream &os,
      std::shared_ptr<hbc::BCProvider> bcProvider,
      llvm::Optional<std::unique_ptr<llvm::MemoryBuffer>> profileBufferOpt)
      : os_(os), hbcParser_(std::move(bcProvider)) {
    if (profileBufferOpt.hasValue()) {
      profileDataOpt_ =
          deserializeTrace(std::move(profileBufferOpt.getValue()));
      checkAndReportAccuracy();
    }
  }

  // Print each instruction's runtime statistics.
  void dumpInstructionStats();
  // Print top K basic blocks' runtime statistics.
  void dumpBasicBlockStats();
  // Print top K functions' runtime statistics.
  void dumpFunctionStats();
  // Print a single function's detailed basic block view for \p funcId.
  void dumpFunctionBasicBlockStat(unsigned funcId);
  // Print page I/O access information.
  void dumpIO();
  // Print the string corresponding to \p stringID.
  void dumpString(uint32_t stringID) {
    os_ << hbcParser_.getBCProvider()->getStringRefFromID(stringID);
  }
  // Print the file name string corresponding to \p filenameId.
  void dumpFileName(uint32_t filenameId) {
    const auto *debugInfo = hbcParser_.getBCProvider()->getDebugInfo();
    std::string fileName = debugInfo->getFilenameByID(filenameId);
    os_ << fileName;
  }
  // Print bundle epilogue.
  void dumpEpilogue();
  // Print a high-level summary for the profile trace.
  void dumpSummary();
  // Print offsets of a function.
  void dumpFunctionOffsets(uint32_t funcId, StructuredPrinter &printer);
  // Print offsets for all functions in bundle.
  void dumpAllFunctionOffsets(StructuredPrinter &printer) {
    printer.openArray();
    for (uint32_t i = 0, e = hbcParser_.getBCProvider()->getFunctionCount();
         i < e;
         i++) {
      dumpFunctionOffsets(i, printer);
    }
    printer.closeArray();
  }
  // Return the ID of the function, if any, found at a given virtual offset.
  llvm::Optional<uint32_t> getFunctionFromVirtualOffset(uint32_t virtualOffset);
};

} // namespace hermes

#endif // HERMES_TOOLS_HBCDUMP_PROFILEANALYZER_H
