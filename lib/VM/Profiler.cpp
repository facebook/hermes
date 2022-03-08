/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMESVM_PROFILER_EXTERN)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits>
#include <string>

#include "hermes/VM/Profiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

void dumpProfilerSymbolMap(Runtime &runtime, const std::string &fileOut) {
  auto fp = fopen(fileOut.c_str(), "w");

  GCScope gcScope{runtime};
  for (auto &funcInfo : runtime.functionInfo) {
    gcScope.clearAllHandles();
    fprintf(fp, "JS_%0*d_", NUM_PROFILER_SYMBOLS_DIGITS, funcInfo.profilerId);
    for (int j = 0; j < PROFILER_SYMBOL_SUFFIX_LENGTH; ++j)
      fputc('x', fp);

    auto ref = funcInfo.functionName;
    if (ref.size()) {
      fputc(' ', fp);
      for (unsigned i = 0; i < ref.size(); ++i) {
        fputc(ref[i], fp); // Only ASCII function names supported.
      }
    } else {
      fprintf(fp, " (unknown)");
    }

    fprintf(
        fp,
        " %d",
        funcInfo.bytecode.get()->getVirtualOffsetForFunction(
            funcInfo.functionID));

    fputc('\n', fp);
  }
  fclose(fp);
}

void patchProfilerSymbols(Runtime &runtime) {
  // Not allowed to write self while running, so replace self with copy.
  char selfName[PATH_MAX];
  memset(selfName, 0, sizeof(selfName));
  ssize_t readlinkLen = readlink("/proc/self/exe", selfName, sizeof(selfName));
  if (readlinkLen < 0) {
    hermes_fatal("Couldn't determine the executable path.");
  }
  int srcFile = open(selfName, O_RDONLY);
  struct stat srcStat;
  fstat(srcFile, &srcStat);
  auto size = lseek(srcFile, 0, SEEK_END);
  lseek(srcFile, 0, SEEK_SET);
  unlink(selfName);
  int dstFile = open(selfName, O_RDWR | O_CREAT, srcStat.st_mode);
  int truncRes = ftruncate(dstFile, size);
  if (truncRes < 0) {
    hermes_fatal("Unable to truncate file.");
  }
  const auto srcBytes =
      (char *)mmap(nullptr, size, PROT_READ, MAP_SHARED, srcFile, 0);
  const auto dstBytes = (char *)mmap(
      nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, dstFile, 0);
  memcpy(dstBytes, srcBytes, size);
  munmap(srcBytes, size);
  close(srcFile);

  // For each JS_n_... found, patch ... with JS function n.
  auto cur = dstBytes;
  const auto end = cur + size;
  const char *prefix = "JS_";
  const int lenPrefix = strlen(prefix);
  const int lenDigits = NUM_PROFILER_SYMBOLS_DIGITS;
  const int lenSuffix = PROFILER_SYMBOL_SUFFIX_LENGTH;
  GCScope gcScope{runtime};
  while ((cur = (char *)memmem(cur, end - cur, prefix, lenPrefix))) {
    gcScope.clearAllHandles();
    cur += lenPrefix;
    if (cur + lenDigits + 1 + lenSuffix > end)
      break;
    ProfilerID n = 0;
    for (int i = 0; i < lenDigits; ++i) {
      int dig = *cur++;
      dig -= '0';
      if (dig < 0 || dig > 9) {
        n = std::numeric_limits<ProfilerID>::max();
        break;
      }
      n = n * 10 + dig;
    }
    if (*cur++ != '_')
      continue; // Malformed.
    if (n == NUM_PROFILER_SYMBOLS - 1)
      continue;
    auto *profilerFunctionInfo = runtime.getProfilerInfo(n);
    if (!profilerFunctionInfo)
      continue;

    auto ref = profilerFunctionInfo->functionName;
    for (unsigned i = 0; i < lenSuffix; ++i) {
      *cur++ = i < ref.size() ? (char)ref[i] : '_';
    }
  }
  munmap(dstBytes, size);
  close(dstFile);
}

} // namespace vm
} // namespace hermes
#endif
