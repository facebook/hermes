/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/PerfJitDump.h"

#ifdef HERMES_ENABLE_PERF_PROF

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"

#include <llvh/Support/Format.h>
#include <llvh/Support/raw_ostream.h>

#include <elf.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <thread>

namespace {

/// Header for JIT dump file.
struct PerfJitHeader {
  /// Characters "jItD". This is used to identify endianness of the file. In
  /// Hermes we only support little endian.
  uint32_t magic = 0x4A695444;
  /// Header version
  uint32_t version = 1;
  /// Total size of header
  uint32_t totalSize;
  /// ELF mach target. We only support AArch64 for now.
  uint32_t elfMach = EM_AARCH64;
  /// Reserved for future use.
  uint32_t pad1 = 0;
  /// JIT process id.
  uint32_t pid;
  /// A timestamp of when the file was created.
  uint64_t timestamp;
  /// A bitmask of flags. Currently, the only flag is about architecture
  /// specific timestamp, which we don't use yet.
  uint64_t flags = 0;
};

/// Value for each record type.
enum PerfJitRecordType {
  JIT_CODE_LOAD = 0,
  JIT_CODE_MOVE = 1,
  JIT_CODE_DEBUG_INFO = 2,
  JIT_CODE_CLOSE = 3,
  JIT_CODE_UNWINDING_INFO = 4,

  JIT_CODE_MAX,
};

/// Every record starts with this prefix.
struct PerfJitPrefix {
  /// Record type as defined by PerfJitRecordType.
  uint32_t id;
  /// Total size of the record, including the record header.
  uint32_t totalSize;
  /// A timestamp of when the record was created.
  uint64_t timestamp;
};

/// Record for jitted function.
struct PerfJitCodeLoad {
  /// Common fields of every record.
  PerfJitPrefix prefix;

  /// OS process id of the runtime generating the jitted code.
  uint32_t pid;
  /// OS thread identification of the runtime thread generating the jitted code.
  uint32_t tid;
  /// Virtual address of jitted code start.
  uint64_t vma;
  /// Code start address for the jitted code. By default vma = codeAddr.
  uint64_t codeAddr;
  /// Size in bytes of the generated jitted code.
  uint64_t codeSize;
  /// Unique identifier for the jitted code.
  uint64_t codeIndex;
  /// Function name in ASCII including the null termination
  /// char fname[];
  /// Followed by the raw bytes encoding of the jitted code.
  /// uint8_t code[codeSize];
};

/// Get timestamp in nanoseconds.
uint64_t getTimestamp() {
  struct timespec ts;
  int result = clock_gettime(CLOCK_MONOTONIC, &ts);
  assert(result == 0 && "clock_gettime failed");
  static constexpr uint64_t kNsecPerSec = 1000000000;
  return (ts.tv_sec * kNsecPerSec) + ts.tv_nsec;
}

} // namespace

namespace hermes::vm {

PerfJitDump::PerfJitDump(int fd) : os_(fd, /*shouldClose*/ false) {
  assert(fd != -1 && "Invalid file descriptor");

  // mmap the file so that there is a mmap record in the perf.data file.
  // The mapping must be PROT_EXEC to ensure it is not ignored by perf record.
  // This is how the perf tool knows about the jitdump file when injecting it to
  // perf.data.
  size_t pageSize = oscompat::page_size();
  void *markerAddr_ =
      mmap(nullptr, pageSize, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
  if (markerAddr_ == MAP_FAILED)
    hermes_fatal("Failed to mmap jit dump file");
  // unmap the file, perf is aware of the jitdump file now.
  munmap(markerAddr_, oscompat::page_size());

  writePerfJitHeader();
}

void PerfJitDump::writeCodeLoadRecord(
    const char *codePtr,
    uint32_t codeSize,
    llvh::StringRef fname) {
  PerfJitCodeLoad codeLoad;
  codeLoad.prefix.id = PerfJitRecordType::JIT_CODE_LOAD;
  // Total size is the size of this record plus the size of the jitted function
  // name (including null terminator) and the code size.
  codeLoad.prefix.totalSize = sizeof(codeLoad) + fname.size() + 1 + codeSize;
  codeLoad.prefix.timestamp = getTimestamp();
  codeLoad.pid = hermes::oscompat::process_id();
  codeLoad.tid = hermes::oscompat::global_thread_id();
  codeLoad.vma = reinterpret_cast<uint64_t>(codePtr);
  codeLoad.codeAddr = codeLoad.vma;
  codeLoad.codeSize = codeSize;
  codeLoad.codeIndex = codeIndex_++;
  os_.write(reinterpret_cast<const char *>(&codeLoad), sizeof(codeLoad));
  os_.write(fname.data(), fname.size() + 1);
  os_.write(codePtr, codeSize);
}

void PerfJitDump::writePerfJitHeader() {
  PerfJitHeader header;
  header.totalSize = sizeof(header);
  header.pid = hermes::oscompat::process_id();
  header.timestamp = getTimestamp();
  os_.write(reinterpret_cast<const char *>(&header), sizeof(header));
}

} // namespace hermes::vm

#endif // HERMES_ENABLE_PERF_PROF
