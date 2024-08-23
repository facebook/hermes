// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJITUTILS_H_INCLUDED
#define ASMJITUTILS_H_INCLUDED

#include <asmjit/core.h>

namespace {

ASMJIT_MAYBE_UNUSED
static inline const char* asmjitArchAsString(asmjit::Arch arch) noexcept {
  switch (arch) {
    case asmjit::Arch::kX86       : return "X86";
    case asmjit::Arch::kX64       : return "X64";

    case asmjit::Arch::kRISCV32   : return "RISCV32";
    case asmjit::Arch::kRISCV64   : return "RISCV64";

    case asmjit::Arch::kARM       : return "ARM";
    case asmjit::Arch::kAArch64   : return "AArch64";
    case asmjit::Arch::kThumb     : return "Thumb";

    case asmjit::Arch::kMIPS32_LE : return "MIPS_LE";
    case asmjit::Arch::kMIPS64_LE : return "MIPS64_LE";

    case asmjit::Arch::kARM_BE    : return "ARM_BE";
    case asmjit::Arch::kThumb_BE  : return "Thumb_BE";
    case asmjit::Arch::kAArch64_BE: return "AArch64_BE";

    case asmjit::Arch::kMIPS32_BE : return "MIPS_BE";
    case asmjit::Arch::kMIPS64_BE : return "MIPS64_BE";

    default:
      return "<Unknown>";
  }
}

ASMJIT_MAYBE_UNUSED
static inline void printIndented(const char* str, size_t indent) noexcept {
  const char* start = str;
  while (*str) {
    if (*str == '\n') {
      size_t size = (size_t)(str - start);
      printf("%*s%.*s\n", size ? int(indent) : 0, "", int(size), start);
      start = str + 1;
    }
    str++;
  }

  size_t size = (size_t)(str - start);
  if (size)
    printf("%*s%.*s\n", int(indent), "", int(size), start);
}

} // {anonymous}

#endif // ASMJITUTILS_H_INCLUDED
