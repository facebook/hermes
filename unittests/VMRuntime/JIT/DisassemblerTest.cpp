/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/LLVMDisassembler.h"
#include "hermes/VM/JIT/NativeDisassembler.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {
#ifdef HERMESVM_JIT_DISASSEMBLER

TEST(DisassemblerTest, LLVMDisassemblerTest) {
  static const uint8_t bytes[] = {
      0x48, 0x8b, 0x04, 0x25, 0x64, 0x00, 0x00, 0x00};
  uint64_t size;

  {
    LLVMDisassembler dis(NativeDisassembler::x86_64_unknown_linux_gnu, 0);
    auto str = dis.formatInstruction(bytes, 0, size);
    ASSERT_TRUE(str.hasValue());
    EXPECT_STREQ("movq\t100, %rax", std::string(str.getValue()).c_str());
  }
  {
    LLVMDisassembler dis(NativeDisassembler::x86_64_unknown_linux_gnu, 1);
    auto str = dis.formatInstruction(bytes, 0, size);
    ASSERT_TRUE(str.hasValue());
    EXPECT_STREQ(
        "mov\trax, qword ptr [100]", std::string(str.getValue()).c_str());
  }
}

TEST(DisassemblerTest, NativeDisassemblerTest) {
  static const uint8_t bytes[] = {
      0x48, 0x8b, 0x04, 0x25, 0x64, 0x00, 0x00, 0x00};

  {
    auto dis = NativeDisassembler::create(
        NativeDisassembler::x86_64_unknown_linux_gnu, 0);
    EXPECT_TRUE(dis);
    std::string str;
    llvm::raw_string_ostream OS(str);
    dis->disassembleBuffer(OS, bytes, 0, true);
    OS.flush();
    EXPECT_STREQ(
        "00000:  48 8b 04 25 64 00 00 00       movq\t100, %rax\n", str.c_str());
  }

  {
    auto dis = NativeDisassembler::create(
        NativeDisassembler::x86_64_unknown_linux_gnu, 1);
    EXPECT_TRUE(dis);
    std::string str;
    llvm::raw_string_ostream OS(str);
    dis->disassembleBuffer(OS, bytes, 0, true);
    OS.flush();
    EXPECT_STREQ(
        "00000:  48 8b 04 25 64 00 00 00       mov\trax, qword ptr [100]\n",
        str.c_str());
  }
}

#endif

} // anonymous namespace
