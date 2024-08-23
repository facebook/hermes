// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_assembler.h"
#include "cmdline.h"

using namespace asmjit;

#if !defined(ASMJIT_NO_X86)
bool testX86Assembler(const TestSettings& settings) noexcept;
bool testX64Assembler(const TestSettings& settings) noexcept;
#endif

#if !defined(ASMJIT_NO_AARCH64)
bool testA64Assembler(const TestSettings& settings) noexcept;
#endif

int main(int argc, char* argv[]) {
  CmdLine cmdLine(argc, argv);

  TestSettings settings {};
  settings.verbose = cmdLine.hasArg("--verbose");
  settings.validate = cmdLine.hasArg("--validate");

  printf("AsmJit Assembler Test-Suite v%u.%u.%u:\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF));

  printf("Usage:\n");
  printf("  --help         Show usage only\n");
  printf("  --verbose      Show only assembling errors [%s]\n", settings.verbose ? "x" : " ");
  printf("  --validate     Use instruction validation [%s]\n", settings.validate ? "x" : " ");
  printf("  --arch=<ARCH>  Select architecture to run ('all' by default)\n");
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

  const char* arch = cmdLine.valueOf("--arch", "all");
  bool x86Failed = false;
  bool x64Failed = false;
  bool aarch64Failed = false;

#if !defined(ASMJIT_NO_X86)
  if ((strcmp(arch, "all") == 0 || strcmp(arch, "x86") == 0))
    x86Failed = !testX86Assembler(settings);

  if ((strcmp(arch, "all") == 0 || strcmp(arch, "x64") == 0))
    x64Failed = !testX64Assembler(settings);
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if ((strcmp(arch, "all") == 0 || strcmp(arch, "aarch64") == 0))
    aarch64Failed = !testA64Assembler(settings);
#endif

  bool failed = x86Failed || x64Failed || aarch64Failed;

  if (failed) {
    if (x86Failed)
      printf("** X86 test suite failed **\n");

    if (x64Failed)
      printf("** X64 test suite failed **\n");

    if (aarch64Failed)
      printf("** AArch64 test suite failed **\n");

    printf("** FAILURE **\n");
  }
  else {
    printf("** SUCCESS **\n");
  }

  return failed ? 1 : 0;
}
