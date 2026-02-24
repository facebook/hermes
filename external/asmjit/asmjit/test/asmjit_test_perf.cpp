// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"

using namespace asmjit;

#if !defined(ASMJIT_NO_X86)
void benchmarkX86Emitters(uint32_t numIterations, bool testX86, bool testX64) noexcept;
#endif

#if !defined(ASMJIT_NO_AARCH64)
void benchmarkA64Emitters(uint32_t numIterations);
#endif

int main(int argc, char* argv[]) {
  CmdLine cmdLine(argc, argv);
  uint32_t numIterations = 20000;

  printf("AsmJit Performance Suite v%u.%u.%u:\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF));

  printf("Usage:\n");
  printf("  --help         Show usage only\n");
  printf("  --quick        Decrease the number of iterations to make tests quicker\n");
  printf("  --arch=<ARCH>  Select architecture(s) to run ('all' by default)\n");
  printf("\n");

  printf("Architectures:\n");
#if !defined(ASMJIT_NO_X86)
  printf("  --arch=x86     32-bit X86 architecture (X86)\n");
  printf("  --arch=x64     64-bit X86 architecture (X86_64)\n");
#endif
#if !defined(ASMJIT_NO_AARCH64)
  printf("  --arch=aarch64 64-bit ARM architecture (AArch64)\n");
#endif
  printf("\n");

  if (cmdLine.hasArg("--help"))
    return 0;

  if (cmdLine.hasArg("--quick"))
    numIterations = 1000;

  const char* arch = cmdLine.valueOf("--arch", "all");

#if !defined(ASMJIT_NO_X86)
  bool testX86 = strcmp(arch, "all") == 0 || strcmp(arch, "x86") == 0;
  bool testX64 = strcmp(arch, "all") == 0 || strcmp(arch, "x64") == 0;

  if (testX86 || testX64)
    benchmarkX86Emitters(numIterations, testX86, testX64);
#endif

#if !defined(ASMJIT_NO_AARCH64)
  bool testAArch64 = strcmp(arch, "all") == 0 || strcmp(arch, "aarch64") == 0;

  if (testAArch64)
    benchmarkA64Emitters(numIterations);
#endif

  return 0;
}
