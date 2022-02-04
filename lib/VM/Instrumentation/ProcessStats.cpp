/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/instrumentation/ProcessStats.h"

#if defined(_WINDOWS)
// Include windows.h first because other includes from windows API need it.
// The blank line after the include is necessary to avoid lint error.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // do not define min/max macros
#include <windows.h>

#include <psapi.h>
#elif defined(__MACH__)
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

#if defined(_WINDOWS)
  PROCESS_MEMORY_COUNTERS pmc;
  BOOL ret = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  assert(ret != 0 && "Failed to call GetProcessMemoryInfo");
  (void)ret;

  MEMORYSTATUSEX ms;
  ms.dwLength = sizeof(ms);
  ret = GlobalMemoryStatusEx(&ms);
  assert(ret != 0 && "Failed to call GlobalMemoryStatusEx");
  (void)ret;

  rss = pmc.WorkingSetSize / 1024;
  va = (ms.ullTotalVirtual - ms.ullAvailVirtual) / 1024;
#elif defined(__MACH__)
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
#elif defined(__EMSCRIPTEN__)
  rss = va = 0;
#else
#error "Unsupported platform"
#endif

  ProcessStats::Info result;
  result.RSSkB = rss;
  result.VAkB = va;
  return result;
}

} // namespace

void ProcessStats::Info::printJSON(llvh::raw_ostream &os) {
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
  ProcessStats::Info ret;
  ret.RSSkB = iRSSkBms_.area();
  ret.VAkB = iVAkBms_.area();
  return ret;
}

} // namespace vm
} // namespace hermes
