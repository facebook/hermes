// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_PERF_H_INCLUDED
#define ASMJIT_TEST_PERF_H_INCLUDED

#include <asmjit/core.h>
#include "asmjitutils.h"
#include "performancetimer.h"

namespace asmjit_perf_utils {

class TestErrorHandler : public asmjit::ErrorHandler {
  void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) {
    (void)err;
    (void)origin;
    printf("ERROR: %s\n", message);
    abort();
  }
};

#ifndef ASMJIT_NO_BUILDER
template<typename BuilderT, typename FuncT>
static uint32_t calculateInstructionCount(asmjit::CodeHolder& code, asmjit::Arch arch, const FuncT& func) noexcept {
  BuilderT builder;
  TestErrorHandler eh;

  asmjit::Environment env(arch);
  code.init(env);
  code.setErrorHandler(&eh);
  code.attach(&builder);
  func(builder);

  uint32_t count = 0;
  asmjit::BaseNode* node = builder.firstNode();

  while (node) {
    count += uint32_t(node->isInst());
    node = node->next();
  }

  code.reset();
  return count;
}
#endif

static inline double calculateMBPS(double duration, uint64_t outputSize) noexcept {
  if (duration == 0)
    return 0.0;

  double bytesTotal = double(outputSize);
  return (bytesTotal * 1000) / (duration * 1024 * 1024);
}

static inline double calculateMIPS(double duration, uint64_t instCount) noexcept {
  if (duration == 0)
    return 0.0;

  return double(instCount) * 1000.0 / (duration * 1e6);
}

template<typename EmitterT, typename FuncT>
static void bench(asmjit::CodeHolder& code, asmjit::Arch arch, uint32_t numIterations, const char* testName, uint32_t instCount, const FuncT& func) noexcept {
  EmitterT emitter;
  TestErrorHandler eh;

  const char* archName = asmjitArchAsString(arch);
  const char* emitterName =
    emitter.isAssembler() ? "Assembler" :
    emitter.isCompiler()  ? "Compiler"  :
    emitter.isBuilder()   ? "Builder"   : "Unknown";

  uint64_t codeSize = 0;
  asmjit::Environment env(arch);

  PerformanceTimer timer;
  double duration = std::numeric_limits<double>::infinity();

  for (uint32_t r = 0; r < numIterations; r++) {
    codeSize = 0;
    code.init(env);
    code.setErrorHandler(&eh);
    code.attach(&emitter);

    timer.start();
    func(emitter);
    timer.stop();

    codeSize += code.codeSize();

    code.reset();
    duration = asmjit::Support::min(duration, timer.duration());
  }

  printf("  [%s] %-9s %-16s | CodeSize:%5llu [B] | Time:%8.4f [ms]", archName, emitterName, testName, (unsigned long long)codeSize, duration);
  if (codeSize) {
    printf(" | Speed:%7.1f [MiB/s]", calculateMBPS(duration, codeSize));
  }
  else {
    printf(" | Speed:    N/A        ");
  }

  if (instCount) {
    printf(", %8.1f [MInst/s]", calculateMIPS(duration, instCount));
  }

  printf("\n");
}

} // {asmjit_perf_utils}

#endif // ASMJIT_TEST_PERF_H_INCLUDED
