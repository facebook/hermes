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
  PerfJitDump(int fd);

  /// Write the JIT_CODE_LOAD record to the jitdump file.
  /// \param codePtr The address of the jitted code.
  /// \param codeSize The size of the jitted code.
  /// \param fname The name of the function that the jitted code is for.
  void writeCodeLoadRecord(
      const char *codePtr,
      uint32_t codeSize,
      llvh::StringRef fname);

 private:
  /// Write the jitdump header.
  void writePerfJitHeader();

  /// The current code index used to distinguish each jitted function.
  uint64_t codeIndex_{0};
  /// Output file stream to write jitdump.
  llvh::raw_fd_ostream os_;
};
} // namespace hermes::vm

#else // HERMES_ENABLE_PERF_PROF

#include "llvh/ADT/StringRef.h"

/// An no-op implementation of PerfJitDump.
namespace hermes::vm {
class PerfJitDump {
 public:
  PerfJitDump(int) {}

  void writeCodeLoadRecord(const char *, uint32_t, llvh::StringRef) {}
};
} // namespace hermes::vm

#endif // HERMES_ENABLE_PERF_PROF
