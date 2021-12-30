/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/instrumentation/PerfEvents.h"

#if defined(__linux__) && \
    (!defined(__ANDROID__) || defined(HERMES_ANDROID_PERF_EVENTS))
#include "llvh/Support/raw_ostream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/syscall.h>
#ifdef ANDROID_LINUX_PERF_PATH
#include ANDROID_LINUX_PERF_PATH
#else
#include <linux/perf_event.h>
#endif

#include <asm/unistd.h>

namespace hermes {
namespace vm {
namespace instrumentation {

// Simple struct to allow aggregate initialization and avoid global
// constructors/destructors.
struct PerfCounter {
  bool ensureInit();
  bool begin();
  bool endAndInsertStats(std::string &jsonStats);
  int fd_;
  const char *const name_;
  const uint32_t type_;
  const uint64_t config_;
};

static PerfCounter counters[] = {
    {-1, "instructions", PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS},
    {-1, "cpu-cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES},
    {-1,
     "L1-icache-load-misses",
     PERF_TYPE_HW_CACHE,
     (PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
      (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {-1,
     "L1-dcache-load-misses",
     PERF_TYPE_HW_CACHE,
     (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
      (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    {-1, "major-faults", PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ},
};

bool PerfCounter::ensureInit() {
  perf_event_attr pe;
  memset(&pe, 0, sizeof(perf_event_attr));
  pe.type = type_;
  pe.size = sizeof(perf_event_attr);
  pe.config = config_;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  fd_ = syscall(
      __NR_perf_event_open,
      &pe,
      0, /* this process */
      -1, /* any CPU */
      -1, /* no group */
      0 /* flags */);
  return fd_ != -1;
}

bool PerfCounter::begin() {
  if (fd_ == -1 && !ensureInit()) {
    return false;
  }
  ioctl(fd_, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0);
  return true;
}

bool PerfCounter::endAndInsertStats(std::string &jsonStats) {
  if (fd_ == -1)
    return false;
  uint64_t count = 0;
  ioctl(fd_, PERF_EVENT_IOC_DISABLE, 0);
  auto res = read(fd_, &count, sizeof(count));
  if (res <= 0)
    return false;
  std::string stats;
  llvh::raw_string_ostream os{stats};
  os << "\"perfEvent_" << name_ << "\": " << count << ",\n\t\t";
  os.flush();
  auto pos = jsonStats.find("\"totalTime\"");
  if (pos == std::string::npos)
    return false;
  jsonStats.insert(pos, stats);
  return true;
}

bool PerfEvents::begin() {
  for (auto &counter : counters)
    if (!counter.begin())
      return false;
  return true;
}

bool PerfEvents::endAndInsertStats(std::string &jsonStats) {
  for (auto &counter : counters)
    if (!counter.endAndInsertStats(jsonStats))
      return false;
  return true;
}

} // namespace instrumentation
} // namespace vm
} // namespace hermes

#else

// Empty implementations for unsupported environments.

namespace hermes {
namespace vm {
namespace instrumentation {

bool PerfEvents::begin() {
  return false;
}

bool PerfEvents::endAndInsertStats(std::string &jsonStats) {
  return false;
}

} // namespace instrumentation
} // namespace vm
} // namespace hermes
#endif
