/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/instrumentation/ProcessStats.h"

#if defined(__MACH__)
#include <mach/mach.h>
#elif defined(__linux__)
#include <unistd.h>
#include <fstream>
#endif // __MACH__, __linux__

namespace hermes {
namespace vm {

namespace {

ProcessStats::Info getProcessStatSnapshot() {
  int64_t rss, va;

#if defined(__MACH__)
  const task_t self = mach_task_self();

  struct task_basic_info info;
  mach_msg_type_number_t fields = TASK_BASIC_INFO_COUNT;

  auto ret = task_info(
      self, TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &fields);

  (void)ret;
  assert(ret == KERN_SUCCESS && "Failed to get process stats.");
  assert(fields == TASK_BASIC_INFO_COUNT && "Not all process stats returned.");

  rss = info.resident_size / 1024;
  va = info.virtual_size / 1024;
#elif defined(__linux__)
  std::ifstream statm("/proc/self/statm");
  statm >> va >> rss;

  // Linux outputs these metrics as a number of pages, correct for that.
  const size_t PS = getpagesize();
  rss *= PS / 1024;
  va *= PS / 1024;
#else
#error "Unsupported platform"
#endif // __MACH__, __linux__

  return {.RSSkB = rss, .VAkB = va};
}

} // namespace

void ProcessStats::Info::printJSON(llvm::raw_ostream &os) {
  os << "{\n "
     << "\t\"Integral of RSS kBms\": " << RSSkB << ",\n"
     << "\t\"Integral of VA kBms\": " << VAkB << "\n"
     << "}\n";
}

ProcessStats::ProcessStats()
    : initTime_(Clock::now()), initInfo_(getProcessStatSnapshot()) {}

void ProcessStats::sample(ProcessStats::Clock::time_point now) {
  using namespace std::chrono;

  Info info = getProcessStatSnapshot();
  int64_t deltaTms = duration_cast<milliseconds>(now - initTime_).count();
  iRSSkBms_.push(deltaTms, info.RSSkB - initInfo_.RSSkB);
  iVAkBms_.push(deltaTms, info.VAkB - initInfo_.VAkB);
}

ProcessStats::Info ProcessStats::getIntegratedInfo() const {
  return {.RSSkB = iRSSkBms_.area(), .VAkB = iVAkBms_.area()};
}

} // namespace vm
} // namespace hermes
