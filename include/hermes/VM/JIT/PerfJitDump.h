/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/JIT/Config.h"

#ifdef HERMES_ENABLE_PERF_PROF

#include <llvh/Support/raw_ostream.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "llvh/Support/raw_ostream.h"

namespace hermes::vm {

/// Generate the JIT dump file for current run. The specification of the jitdump
/// format is defined at:
/// https://github.com/torvalds/linux/blob/master/tools/perf/Documentation/jitdump-specification.txt
class PerfJitDump {
 public:
  /// Call mmap on the jitdump file to let perf know about it, and write out
  /// the jitdump header. Note that the jitdump file name should have the form
  /// of "<dir>/jit-<pid>.dump". And the caller is supposed to close the file.
  /// \param fd The file descriptor of the opened jitdump file.
  /// \param commentFile The path of the file to store the comments.
  /// \param commentFileOS The output stream to write the comments.
  PerfJitDump(int jitdumpFd, int commentFd, const std::string &commentFile);
  ~PerfJitDump();

  /// Write the JIT_CODE_LOAD record to the jitdump file.
  /// \param codePtr The address of the jitted code.
  /// \param codeSize The size of the jitted code.
  /// \param fname The name of the function that the jitted code is for.
  void writeCodeLoadRecord(
      const char *codePtr,
      uint32_t codeSize,
      llvh::StringRef fname);

  /// Add comments of jitted code as debug info to the jitdump file. These
  /// comment lines for all jitted functions will be saved to commentFile_ and
  /// used in the debug info record for each of them.
  /// \param comment The comment to be added.
  /// \param offset Current offset of the jitted code.
  void addCodeComment(llvh::StringRef comment, uint32_t offset);

 private:
  struct DebugEntry;

  /// Write the jitdump header.
  void writePerfJitHeader();
  /// Write the JIT_CODE_DEBUG_INFO record to the jitdump file.
  void writeDebugInfoRecord(const char *codePtr);
  /// Write the comment line into commentFile_.
  void writeCommentLine(llvh::StringRef comment);

  /// The current code index used to distinguish each jitted function.
  uint64_t codeIndex_{0};
  /// The current line number of the commentFile_.
  int curCommentLine_{0};
  /// Output file stream to write jitdump.
  llvh::raw_fd_ostream os_;
  /// The debug info entries.
  std::vector<DebugEntry> debugEntries_;
  /// Path of the file to store comments.
  std::string commentFile_;
  /// OS for commentFile_.
  llvh::raw_fd_ostream commentFileOS_;
};
} // namespace hermes::vm

#else // HERMES_ENABLE_PERF_PROF

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

/// An no-op implementation of PerfJitDump.
namespace hermes::vm {
class PerfJitDump {
 public:
  PerfJitDump(int, int, const std::string &) {}

  void writeCodeLoadRecord(const char *, uint32_t, llvh::StringRef) {}

  void addCodeComment(llvh::StringRef, uint32_t offset) {}
};
} // namespace hermes::vm

#endif // HERMES_ENABLE_PERF_PROF
