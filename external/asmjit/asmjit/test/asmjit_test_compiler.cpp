// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>
#include <chrono>

#if !defined(ASMJIT_NO_COMPILER)

#include "cmdline.h"
#include "asmjitutils.h"
#include "performancetimer.h"
#include "asmjit_test_compiler.h"

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
  void compiler_add_x86_tests(TestApp& app);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
  void compiler_add_a64_tests(TestApp& app);
#endif // !ASMJIT_NO_AARCH64

using namespace asmjit;

int TestApp::handleArgs(int argc, const char* const* argv) {
  CmdLine cmd(argc, argv);
  _arch = cmd.valueOf("--arch", "all");
  _filter = cmd.valueOf("--filter", nullptr);

  if (cmd.hasArg("--help")) _helpOnly = true;
  if (cmd.hasArg("--verbose")) _verbose = true;

#ifndef ASMJIT_NO_LOGGING
  if (cmd.hasArg("--dump-asm")) _dumpAsm = true;
#endif // !ASMJIT_NO_LOGGING

  if (cmd.hasArg("--dump-hex")) _dumpHex = true;

  return 0;
}

void TestApp::showInfo() {
  printf("AsmJit Compiler Test-Suite v%u.%u.%u (Arch=%s):\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost));

  printf("Usage:\n");
  printf("  --help          Show usage only\n");
  printf("  --arch=<NAME>   Select architecture to run ('all' by default)\n");
  printf("  --filter=<NAME> Use a filter to restrict which test is called\n");
  printf("  --verbose       Verbose output\n");
  printf("  --dump-asm      Assembler output\n");
  printf("  --dump-hex      Hexadecimal output (relocated, only for host arch)\n");
  printf("\n");
}

#ifndef ASMJIT_NO_LOGGING
class IndentedStdoutLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(IndentedStdoutLogger)

  size_t _indentation = 0;

  explicit IndentedStdoutLogger(size_t indentation) noexcept
    : _indentation(indentation) {}

  Error _log(const char* data, size_t size = SIZE_MAX) noexcept override {
    asmjit::DebugUtils::unused(size);
    printIndented(data, _indentation);
    return kErrorOk;
  }
};
#endif // !ASMJIT_NO_LOGGING

bool TestApp::shouldRun(const TestCase* tc) {
  if (!_filter)
    return true;

  return strstr(tc->name(), _filter) != nullptr;
}

int TestApp::run() {
#ifndef ASMJIT_NO_LOGGING
  FormatOptions formatOptions;
  formatOptions.addFlags(
    FormatFlags::kMachineCode |
    FormatFlags::kExplainImms |
    FormatFlags::kRegCasts   );
  formatOptions.setIndentation(FormatIndentationGroup::kCode, 2);

  IndentedStdoutLogger printLogger(4);
  printLogger.setOptions(formatOptions);

  StringLogger stringLogger;
  stringLogger.setOptions(formatOptions);

  auto printStringLoggerContent = [&]() {
    if (!_verbose) {
      printf("%s", stringLogger.data());
      fflush(stdout);
    }
  };
#else
  auto printStringLoggerContent = [&]() {};
#endif // !ASMJIT_NO_LOGGING

  // maybe unused...
  DebugUtils::unused(printStringLoggerContent);

#ifndef ASMJIT_NO_JIT
  JitRuntime runtime;
#endif // !ASMJIT_NO_JIT

  PerformanceTimer compileTimer;
  PerformanceTimer finalizeTimer;
  double compileTime = 0;
  double finalizeTime = 0;

  for (std::unique_ptr<TestCase>& test : _tests) {
    if (!shouldRun(test.get()))
      continue;

    _numTests++;

    for (uint32_t pass = 0; pass < 2; pass++) {
      bool runnable = false;
      CodeHolder code;
      SimpleErrorHandler errorHandler;

      const char* statusSeparator = " ";

      // Filter architecture to run.
      if (strcmp(_arch, "all") != 0) {
        switch (test->arch()) {
          case Arch::kX86:
            if (strcmp(_arch, "x86") == 0)
              break;
            continue;
          case Arch::kX64:
            if (strcmp(_arch, "x64") == 0)
              break;
            continue;
          case Arch::kAArch64:
            if (strcmp(_arch, "aarch64") == 0)
              break;
            continue;
          default:
            continue;
        }
      }

      // Use platform environment and CPU features when the test can run on the arch.
#ifndef ASMJIT_NO_JIT
      if (runtime.arch() == test->arch()) {
        code.init(runtime.environment(), runtime.cpuFeatures());
        runnable = true;
      }
#endif // !ASMJIT_NO_JIT

      if (!code.isInitialized()) {
        Environment customEnv;
        CpuFeatures features;

        switch (test->arch()) {
          case Arch::kX86:
          case Arch::kX64:
            features.add(CpuFeatures::X86::kADX,
                         CpuFeatures::X86::kAVX,
                         CpuFeatures::X86::kAVX2,
                         CpuFeatures::X86::kBMI,
                         CpuFeatures::X86::kBMI2,
                         CpuFeatures::X86::kCMOV,
                         CpuFeatures::X86::kF16C,
                         CpuFeatures::X86::kFMA,
                         CpuFeatures::X86::kFPU,
                         CpuFeatures::X86::kI486,
                         CpuFeatures::X86::kLZCNT,
                         CpuFeatures::X86::kMMX,
                         CpuFeatures::X86::kMMX2,
                         CpuFeatures::X86::kPOPCNT,
                         CpuFeatures::X86::kSSE,
                         CpuFeatures::X86::kSSE2,
                         CpuFeatures::X86::kSSE3,
                         CpuFeatures::X86::kSSSE3,
                         CpuFeatures::X86::kSSE4_1,
                         CpuFeatures::X86::kSSE4_2);
            break;

          case Arch::kAArch64:
            features.add(CpuFeatures::ARM::kAES,
                         CpuFeatures::ARM::kASIMD,
                         CpuFeatures::ARM::kIDIVA,
                         CpuFeatures::ARM::kIDIVT,
                         CpuFeatures::ARM::kPMULL);
            break;

          default:
            break;
        }

        customEnv.init(test->arch());
        code.init(customEnv, features);
      }

      code.setErrorHandler(&errorHandler);

      if (pass != 0) {
        printf("[Test:%s] %s", asmjitArchAsString(test->arch()), test->name());
        fflush(stdout);

#ifndef ASMJIT_NO_LOGGING
        if (_verbose || _dumpAsm || _dumpHex) {
          printf("\n");
          statusSeparator = "  ";
        }

        if (_verbose) {
          printf("  [Log]\n");
          code.setLogger(&printLogger);
        }
        else {
          stringLogger.clear();
          code.setLogger(&stringLogger);
        }
#endif // !ASMJIT_NO_LOGGING
      }

      std::unique_ptr<BaseCompiler> cc;

#ifndef ASMJIT_NO_X86
      if (code.arch() == Arch::kX86 || code.arch() == Arch::kX64)
        cc = std::unique_ptr<x86::Compiler>(new x86::Compiler(&code));
#endif // !ASMJIT_NO_X86

#ifndef ASMJIT_NO_AARCH64
      if (code.arch() == Arch::kAArch64)
        cc = std::unique_ptr<a64::Compiler>(new a64::Compiler(&code));
#endif // !ASMJIT_NO_AARCH64

      if (!cc)
        continue;

#ifndef ASMJIT_NO_LOGGING
      cc->addDiagnosticOptions(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
#endif // !ASMJIT_NO_LOGGING

      compileTimer.start();
      test->compile(*cc);
      compileTimer.stop();

      Error err = errorHandler._err;
      if (err == kErrorOk) {
        finalizeTimer.start();
        err = cc->finalize();
        finalizeTimer.stop();
      }

      // The first pass is only used for timing of serialization and compilation, because otherwise it would be
      // biased by logging, which takes much more time than finalize() does. We want to benchmark Compiler the
      // way it would be used in the production.
      if (pass == 0) {
        _outputSize += code.codeSize();
        compileTime += compileTimer.duration();
        finalizeTime += finalizeTimer.duration();
        continue;
      }

#ifndef ASMJIT_NO_LOGGING
      if (_dumpAsm) {
        String sb;
        Formatter::formatNodeList(sb, formatOptions, cc.get());
        printf("  [Assembly]\n");
        printIndented(sb.data(), 4);
      }
#endif // !ASMJIT_NO_LOGGING

#ifndef ASMJIT_NO_JIT
      if (runnable) {
        void* func = nullptr;
        if (err == kErrorOk)
          err = runtime.add(&func, &code);

        if (err == kErrorOk && _dumpHex) {
          String sb;
          sb.appendHex((void*)func, code.codeSize());
          printf("  [Hex Dump]:\n");
          for (size_t i = 0; i < sb.size(); i += 76) {
            printf("    %.60s\n", sb.data() + i);
          }
        }

        if (_verbose)
          fflush(stdout);

        if (err == kErrorOk) {
          StringTmp<128> result;
          StringTmp<128> expect;

          if (test->run(func, result, expect)) {
            if (!_verbose)
              printf("%s[RUN OK]\n", statusSeparator);
          }
          else {
            if (!_verbose)
              printf("%s[RUN FAILED]\n", statusSeparator);

            printStringLoggerContent();
            printf("  [Output]\n");
            printf("    Returned: %s\n", result.data());
            printf("    Expected: %s\n", expect.data());
            _numFailed++;
          }

          if (_dumpAsm)
            printf("\n");

          runtime.release(func);
        }
        else {
          if (!_verbose)
            printf("%s[COMPILE FAILED]\n", statusSeparator);

          printStringLoggerContent();
          printf("  [Status]\n");
          printf("    ERROR 0x%08X: %s\n", unsigned(err), errorHandler._message.data());
          _numFailed++;
        }
      }
#endif // !ASMJIT_NO_JIT

      if (!runnable) {
        if (err) {
          printf("  [Status]\n");
          printf("    ERROR 0x%08X: %s\n", unsigned(err), errorHandler._message.data());
          _numFailed++;
        }
        else {
          printf("%s[COMPILE OK]\n", statusSeparator);
        }
      }

#ifndef ASMJIT_NO_LOGGING
      if (_verbose || _dumpAsm || _dumpHex) {
        printf("\n");
      }
#endif // !ASMJIT_NO_LOGGING
    }
  }

  printf("\n");
  printf("Summary:\n");
  printf("  OutputSize: %zu bytes\n", _outputSize);
  printf("  CompileTime: %.2f ms\n", compileTime);
  printf("  FinalizeTime: %.2f ms\n", finalizeTime);
  printf("\n");

  if (_numFailed == 0)
    printf("** SUCCESS: All %u tests passed **\n", _numTests);
  else
    printf("** FAILURE: %u of %u tests failed **\n", _numFailed, _numTests);

  return _numFailed == 0 ? 0 : 1;
}

int main(int argc, char* argv[]) {
  TestApp app;

  app.handleArgs(argc, argv);
  app.showInfo();

#if !defined(ASMJIT_NO_X86)
  compiler_add_x86_tests(app);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  compiler_add_a64_tests(app);
#endif // !ASMJIT_NO_AARCH64

  return app.run();
}

#else

int main(int argc, char* argv[]) {
  DebugUtils::unused(argc, argv);

  printf("AsmJit Compiler Test suite is disabled when compiling with ASMJIT_NO_COMPILER\n\n");
  return 0;
}

#endif // !ASMJIT_NO_COMPILER
