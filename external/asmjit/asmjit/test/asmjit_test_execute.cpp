// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

static void printInfo() noexcept {
  printf("AsmJit Execute Test-Suite v%u.%u.%u\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF));
}

#if !defined(ASMJIT_NO_JIT) && ( \
    (ASMJIT_ARCH_X86 != 0  && !defined(ASMJIT_NO_X86    )) || \
    (ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)) )

#if ASMJIT_ARCH_X86 != 0
#include <asmjit/x86.h>
#endif

#if ASMJIT_ARCH_ARM == 64
#include <asmjit/a64.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// Signature of the generated function.
typedef void (*EmptyFunc)(void);

// Generate Empty Function
// -----------------------

#if ASMJIT_ARCH_X86 != 0
static void generateEmptyFunc(CodeHolder& code) noexcept {
  x86::Assembler a(&code);
  a.ret();
}
#endif

#if ASMJIT_ARCH_ARM == 64
static void generateEmptyFunc(CodeHolder& code) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);
}
#endif

// Testing
// -------

static void executeEmptyFunc(JitRuntime& rt) noexcept {
  CodeHolder code;
  code.init(rt.environment(), rt.cpuFeatures());

  EmptyFunc fn;

  generateEmptyFunc(code);
  Error err = rt.add(&fn, &code);

  if (err) {
    printf("** FAILURE: JitRuntime::add() failed: %s **\n", DebugUtils::errorAsString(err));
    exit(1);
  }

  fn();

  rt.release(&fn);
}

int main() {
  printInfo();
  printf("\n");

  {
    printf("Trying to execute empty function with JitRuntime (default settings)\n");
    JitRuntime rt;
    executeEmptyFunc(rt);
  }

  if (VirtMem::hardenedRuntimeInfo().hasFlag(VirtMem::HardenedRuntimeFlags::kDualMapping)) {
    printf("Trying to execute empty function with JitRuntime (dual-mapped)\n");
    JitAllocator::CreateParams params {};
    params.options |= JitAllocatorOptions::kUseDualMapping;
    JitRuntime rt;
    executeEmptyFunc(rt);
  }

  // If we are here we were successful, otherwise the process would crash.
  printf("** SUCCESS **\n");
  return 0;
}
#else
int main() {
  printInfo();
  printf("\nThis test is currently disabled - no JIT or no support for the target architecture\n");
  return 0;
}
#endif // ASMJIT_ARCH_X86 && !ASMJIT_NO_X86 && !ASMJIT_NO_JIT
