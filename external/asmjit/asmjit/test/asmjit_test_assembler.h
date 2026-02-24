// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_ASSEMBLER_H_INCLUDED
#define ASMJIT_TEST_ASSEMBLER_H_INCLUDED

#include <asmjit/core.h>
#include <stdio.h>

struct TestSettings {
  bool verbose;
  bool validate;
};

template<typename AssemblerType>
class AssemblerTester {
public:
  asmjit::Environment env {};
  asmjit::CodeHolder code {};
  AssemblerType assembler {};
  asmjit::Label L0 {};
  const TestSettings& settings;

  size_t passed {};
  size_t count {};

  AssemblerTester(asmjit::Arch arch, const TestSettings& settings) noexcept
    : env(arch),
      settings(settings) {
    prepare();
  }

  void printHeader(const char* archName) noexcept {
    printf("%s assembler tests:\n", archName);
  }

  void printSummary() noexcept {
    printf("  Passed: %zu / %zu tests\n\n", passed, count);
  }

  bool didPass() const noexcept { return passed == count; }

  void prepare() noexcept {
    code.reset();
    code.init(env, 0);
    code.attach(&assembler);
    L0 = assembler.newLabel();

    if (settings.validate)
      assembler.addDiagnosticOptions(asmjit::DiagnosticOptions::kValidateAssembler);
  }

  ASMJIT_NOINLINE bool testValidInstruction(const char* s, const char* expectedOpcode, asmjit::Error err = asmjit::kErrorOk) noexcept {
    count++;

    if (err) {
      printf("  !! %s\n"
             "    <%s>\n", s, asmjit::DebugUtils::errorAsString(err));
      prepare();
      return false;
    }

    asmjit::String encodedOpcode;
    asmjit::Section* text = code.textSection();

    encodedOpcode.appendHex(text->data(), text->bufferSize());
    if (encodedOpcode != expectedOpcode) {
      printf("  !! [%s] <- %s\n"
             "     [%s] (Expected)\n", encodedOpcode.data(), s, expectedOpcode);
      prepare();
      return false;
    }

    if (settings.verbose)
      printf("  OK [%s] <- %s\n", encodedOpcode.data(), s);

    passed++;
    prepare();
    return true;
  }

  ASMJIT_NOINLINE bool testInvalidInstruction(const char* s, asmjit::Error expectedError, asmjit::Error err) noexcept {
    count++;

    if (err == asmjit::kErrorOk) {
      printf("  !! %s passed, but should have failed with <%s> error\n", s, asmjit::DebugUtils::errorAsString(expectedError));
      prepare();
      return false;
    }

    if (err != asmjit::kErrorOk) {
      printf("  !! %s failed with <%s>, but should have failed with <%s>\n", s, asmjit::DebugUtils::errorAsString(err), asmjit::DebugUtils::errorAsString(expectedError));
      prepare();
      return false;
    }

    if (settings.verbose)
      printf("  OK [%s] <- %s\n", asmjit::DebugUtils::errorAsString(err), s);

    passed++;
    prepare();
    return true;
  }
};

#endif // ASMJIT_TEST_ASSEMBLER_H_INCLUDED
