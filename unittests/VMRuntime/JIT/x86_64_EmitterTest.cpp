/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/NativeDisassembler.h"
#include "hermes/VM/JIT/x86-64/Emitter.h"

#include "gtest/gtest.h"

using namespace hermes::vm;
using namespace hermes::vm::x86_64;

namespace {
#ifdef HERMESVM_JIT_DISASSEMBLER

TEST(x86_64_EmitterTest, Test) {
  uint8_t buf[16384];
  Emitter emitter{buf};

  std::string str;
  llvm::raw_string_ostream OS{str};
  // OS.SetUnbuffered();
  auto dis =
      NativeDisassembler::create(NativeDisassembler::x86_64_unknown_linux_gnu);

  auto trim = [](std::string str) {
    // Strip the address
    if (str.compare(0, 6, "00000:") == 0)
      str.erase(0, 6);
    // Strip leading spaces
    while (!str.empty() && isspace(str.front()))
      str.erase(0, 1);

    // Skip trailing comment
    auto com = str.rfind("#");
    if (com != std::string::npos)
      str.erase(com);

    while (!str.empty() && isspace(str.back()))
      str.pop_back();
    for (char &ch : str)
      if (ch == '\t')
        ch = ' ';
    return str;
  };

#define CHECK(expected)                                         \
  str.clear();                                                  \
  dis->disassembleBuffer(                                       \
      OS, llvm::makeArrayRef(buf, emitter.current()), 0, true); \
  emitter = Emitter{buf};                                       \
  EXPECT_STREQ(expected, trim(OS.str()).c_str())

  emitter.movqImmToReg(10, Reg::rax);
  CHECK("48 c7 c0 0a 00 00 00          movq $10, %rax");

  emitter.pushqReg(x86_64::Reg::rbp);
  CHECK("55                            pushq %rbp");
  emitter.movRegToReg<S::Q>(x86_64::Reg::rsp, x86_64::Reg::rbp);
  CHECK("48 89 e5                      movq %rsp, %rbp");
  emitter.movRegToReg<S::Q>(x86_64::Reg::rbp, x86_64::Reg::rsp);
  CHECK("48 89 ec                      movq %rbp, %rsp");
  emitter.movRMToReg<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 100, x86_64::Reg::r12);
  CHECK("4c 8b 63 64                   movq 100(%rbx), %r12");
  emitter.movRegToRM<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::r12, x86_64::Reg::NoIndex, 108);
  CHECK("49 89 5c 24 6c                movq %rbx, 108(%r12)");
  emitter.popqReg(x86_64::Reg::rbp);
  CHECK("5d                            popq %rbp");
  emitter.retq();
  CHECK("c3                            retq");
  emitter.pushqReg(x86_64::Reg::r12);
  CHECK("41 54                         pushq %r12");
  emitter.popqReg(x86_64::Reg::r12);
  CHECK("41 5c                         popq %r12");
  emitter.movRegToReg<S::Q>(x86_64::Reg::rbx, x86_64::Reg::rax);
  CHECK("48 89 d8                      movq %rbx, %rax");
  emitter.movRegToReg<S::Q>(x86_64::Reg::r10, x86_64::Reg::r9);
  CHECK("4d 89 d1                      movq %r10, %r9");
  emitter.movRegToReg<S::Q>(x86_64::Reg::rbx, x86_64::Reg::r9);
  CHECK("49 89 d9                      movq %rbx, %r9");
  emitter.movRegToReg<S::L>(x86_64::Reg::ebx, x86_64::Reg::eax);
  CHECK("89 d8                         movl %ebx, %eax");
  emitter.movRMToReg<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 100, x86_64::Reg::r12);
  CHECK("4c 8b 63 64                   movq 100(%rbx), %r12");
  emitter.movRMToReg<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 1000, x86_64::Reg::r12);
  CHECK("4c 8b a3 e8 03 00 00          movq 1000(%rbx), %r12");
  emitter.movRMToReg<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::NoIndex, 100, x86_64::Reg::ecx);
  CHECK("8b 4b 64                      movl 100(%rbx), %ecx");
  emitter.movRMToReg<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::NoIndex, 0, x86_64::Reg::ecx);
  CHECK("8b 0b                         movl (%rbx), %ecx");
  emitter.movRMToReg<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::NoIndex, 0, x86_64::Reg::r8);
  CHECK("44 8b 03                      movl (%rbx), %r8d");
  emitter.movRMToReg<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0, x86_64::Reg::r8);
  CHECK("4c 8b 03                      movq (%rbx), %r8");

  emitter.movRegToRM<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::rcx, x86_64::Reg::NoIndex, 108);
  CHECK("89 59 6c                      movl %ebx, 108(%rcx)");
  emitter.movRegToRM<S::L, 0>(
      x86_64::Reg::r9, x86_64::Reg::r10, x86_64::Reg::NoIndex, 108);
  CHECK("45 89 4a 6c                   movl %r9d, 108(%r10)");
  emitter.movRegToRM<S::Q, 0>(
      x86_64::Reg::ebx, x86_64::Reg::rcx, x86_64::Reg::NoIndex, 108);
  CHECK("48 89 59 6c                   movq %rbx, 108(%rcx)");
  emitter.movRegToRM<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::r10, x86_64::Reg::NoIndex, 108);
  CHECK("49 89 5a 6c                   movq %rbx, 108(%r10)");
  emitter.movRegToRM<S::Q, 0>(
      x86_64::Reg::rbx, x86_64::Reg::r12, x86_64::Reg::NoIndex, 108);
  CHECK("49 89 5c 24 6c                movq %rbx, 108(%r12)");
  emitter.movRegToRM<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::esp, x86_64::Reg::NoIndex, 108);
  CHECK("89 5c 24 6c                   movl %ebx, 108(%rsp)");
  emitter.movRegToRM<S::L, 0>(
      x86_64::Reg::ebx, x86_64::Reg::rbp, x86_64::Reg::NoIndex, 108);
  CHECK("89 5d 6c                      movl %ebx, 108(%rbp)");
  emitter.movRegToRM<S::Q, 0>(
      x86_64::Reg::ebx, x86_64::Reg::rbp, x86_64::Reg::NoIndex, 108);
  CHECK("48 89 5d 6c                   movq %rbx, 108(%rbp)");

  emitter.movRegToRM<S::L, 2>(
      x86_64::Reg::ebx, x86_64::Reg::rsi, x86_64::Reg::rcx, 108);
  CHECK("89 5c 4e 6c                   movl %ebx, 108(%rsi,%rcx,2)");
  emitter.movRMToReg<S::L, 2>(
      x86_64::Reg::rsi, x86_64::Reg::rcx, 108, x86_64::Reg::ebx);
  CHECK("8b 5c 4e 6c                   movl 108(%rsi,%rcx,2), %ebx");

  emitter.jmp<OffsetType::Auto>(emitter.current() + 1000 + 5);
  CHECK("e9 e8 03 00 00                jmp 1000");
  emitter.jmp<OffsetType::Auto>(emitter.current() - 10 + 2);
  CHECK("eb f6                         jmp -10");
  emitter.jmpRM<0>(x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0);
  CHECK("ff 23                         jmpq *(%rbx)");
  emitter.jmpRM<0>(x86_64::Reg::rbx, x86_64::Reg::NoIndex, 100);
  CHECK("ff 63 64                      jmpq *100(%rbx)");
  emitter.jmpRM<0>(x86_64::Reg::rbx, x86_64::Reg::NoIndex, 1000);
  CHECK("ff a3 e8 03 00 00             jmpq *1000(%rbx)");
  emitter.jmpRM<0>(x86_64::Reg::r8, x86_64::Reg::NoIndex, 0);
  CHECK("41 ff 20                      jmpq *(%r8)");
  emitter.jmpRM<0>(x86_64::Reg::r12, x86_64::Reg::NoIndex, 0);
  CHECK("41 ff 24 24                   jmpq *(%r12)");
  emitter.jmpRM<8>(x86_64::Reg::rbx, x86_64::Reg::rcx, 1000);
  CHECK("ff a4 cb e8 03 00 00          jmpq *1000(%rbx,%rcx,8)");
  emitter.jmpRM<8>(x86_64::Reg::r8, x86_64::Reg::rcx, 1000);
  CHECK("41 ff a4 c8 e8 03 00 00       jmpq *1000(%r8,%rcx,8)");
  emitter.jmpRM<8>(x86_64::Reg::r8, x86_64::Reg::rcx, 100);
  CHECK("41 ff 64 c8 64                jmpq *100(%r8,%rcx,8)");
  emitter.jmpRM<8>(x86_64::Reg::r8, x86_64::Reg::rcx, 0);
  CHECK("41 ff 24 c8                   jmpq *(%r8,%rcx,8)");
  emitter.jmpRM<8>(x86_64::Reg::rax, x86_64::Reg::rcx, 0);
  CHECK("ff 24 c8                      jmpq *(%rax,%rcx,8)");
  emitter.jmpRM<ScaleRIPAddr32>(x86_64::Reg::none, x86_64::Reg::NoIndex, 0);
  CHECK("ff 25 00 00 00 00             jmpq *(%rip)");

  emitter.cjump<x86_64::CCode::Z, OffsetType::Auto>(emitter.current() - 20 + 2);
  CHECK("74 ec                         je -20");
  emitter.cjumpOP<OffsetType::Auto>(
      emitter.ccodeToOpCode(x86_64::CCode::Z), emitter.current() - 20 + 2);
  CHECK("74 ec                         je -20");
  emitter.cjump<x86_64::CCode::GE, OffsetType::Auto>(
      emitter.current() - 2000 + 6);
  CHECK("0f 8d 30 f8 ff ff             jge -2000");
  emitter.cjumpOP<OffsetType::Auto>(
      emitter.ccodeToOpCode(x86_64::CCode::GE), emitter.current() - 2000 + 6);
  CHECK("0f 8d 30 f8 ff ff             jge -2000");

  emitter.call(emitter.current() + 1000 + 5);
  CHECK("e8 e8 03 00 00                callq 1000");
  emitter.callReg(Reg::rax);
  CHECK("ff d0                         callq *%rax");
  emitter.callRM<0>(x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0);
  CHECK("ff 13                         callq *(%rbx)");
  emitter.callRM<8>(x86_64::Reg::r8, x86_64::Reg::rcx, 1000);
  CHECK("41 ff 94 c8 e8 03 00 00       callq *1000(%r8,%rcx,8)");

  emitter.movfpRegToReg(x86_64::Reg::XMM0, x86_64::Reg::XMM1);
  CHECK("f2 0f 10 c8                   movsd %xmm0, %xmm1");
  emitter.movfpRMToReg<0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0, x86_64::Reg::XMM1);
  CHECK("f2 0f 10 0b                   movsd (%rbx), %xmm1");
  emitter.movfpRMToReg<0>(
      x86_64::Reg::r8, x86_64::Reg::NoIndex, 0, x86_64::Reg::XMM1);
  CHECK("f2 41 0f 10 08                movsd (%r8), %xmm1");
  emitter.movfpRMToReg<8>(
      x86_64::Reg::r8, x86_64::Reg::r9, 100, x86_64::Reg::XMM1);
  CHECK("f2 43 0f 10 4c c8 64          movsd 100(%r8,%r9,8), %xmm1");
  emitter.movfpRegToRM<8>(
      x86_64::Reg::XMM1, x86_64::Reg::r8, x86_64::Reg::r9, 100);
  CHECK("f2 43 0f 11 4c c8 64          movsd %xmm1, 100(%r8,%r9,8)");
  emitter.subfpRegFromReg(x86_64::Reg::XMM2, x86_64::Reg::XMM1);
  CHECK("f2 0f 5c ca                   subsd %xmm2, %xmm1");
  emitter.subfpRMFromReg<0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0, x86_64::Reg::XMM1);
  CHECK("f2 0f 5c 0b                   subsd (%rbx), %xmm1");
  emitter.addfpRegToReg(x86_64::Reg::XMM2, x86_64::Reg::XMM1);
  CHECK("f2 0f 58 ca                   addsd %xmm2, %xmm1");
  emitter.addfpRMToReg<0>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0, x86_64::Reg::XMM1);
  CHECK("f2 0f 58 0b                   addsd (%rbx), %xmm1");
  emitter.addfpRegToReg<x86_64::FP::Float>(
      x86_64::Reg::XMM2, x86_64::Reg::XMM1);
  CHECK("f3 0f 58 ca                   addss %xmm2, %xmm1");
  emitter.addfpRMToReg<0, x86_64::FP::Float>(
      x86_64::Reg::rbx, x86_64::Reg::NoIndex, 0, x86_64::Reg::XMM1);
  CHECK("f3 0f 58 0b                   addss (%rbx), %xmm1");

  emitter.movImmToReg<S::B>(10, Reg::al);
  CHECK("b0 0a                         movb $10, %al");
  emitter.movImmToReg<S::B>(10, Reg::ah);
  CHECK("b4 0a                         movb $10, %ah");
  emitter.movImmToReg<S::B>(10, Reg::r8);
  CHECK("41 b0 0a                      movb $10, %r8b");
  emitter.movImmToReg<S::W>(10, Reg::ax);
  CHECK("66 b8 0a 00                   movw $10, %ax");
  emitter.movImmToReg<S::W>(10, Reg::r8);
  CHECK("66 41 b8 0a 00                movw $10, %r8w");
  emitter.movImmToReg<S::L>(10, Reg::eax);
  CHECK("b8 0a 00 00 00                movl $10, %eax");
  emitter.movImmToReg<S::L>(10, Reg::r8d);
  CHECK("41 b8 0a 00 00 00             movl $10, %r8d");
  emitter.movImmToReg<S::Q>(10, Reg::rax);
  CHECK("48 b8 0a 00 00 00 00 00 00 00 movabsq $10, %rax");
  emitter.movImmToReg<S::Q>(10, Reg::r8);
  CHECK("49 b8 0a 00 00 00 00 00 00 00 movabsq $10, %r8");
  emitter.movImmToRM<S::B, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("c6 44 58 20 0a                movb $10, 32(%rax,%rbx,2)");
  emitter.movImmToRM<S::B, 2>(10, Reg::r8, Reg::rbx, 32);
  CHECK("41 c6 44 58 20 0a             movb $10, 32(%r8,%rbx,2)");
  emitter.movImmToRM<S::W, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("66 c7 44 58 20 0a 00          movw $10, 32(%rax,%rbx,2)");
  emitter.movImmToRM<S::W, 2>(10, Reg::r8, Reg::rbx, 32);
  CHECK("66 41 c7 44 58 20 0a 00       movw $10, 32(%r8,%rbx,2)");
  emitter.movImmToRM<S::L, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("c7 44 58 20 0a 00 00 00       movl $10, 32(%rax,%rbx,2)");
  emitter.movImmToRM<S::L, 2>(10, Reg::r8, Reg::rbx, 32);
  CHECK("41 c7 44 58 20 0a 00 00 00    movl $10, 32(%r8,%rbx,2)");
  emitter.movImmToRM<S::SLQ, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("48 c7 44 58 20 0a 00 00 00    movq $10, 32(%rax,%rbx,2)");
  emitter.movImmToRM<S::SLQ, 2>(10, Reg::r8, Reg::rbx, 32);
  CHECK("49 c7 44 58 20 0a 00 00 00    movq $10, 32(%r8,%rbx,2)");
  emitter.movqImmToReg(10, Reg::rax);
  CHECK("48 c7 c0 0a 00 00 00          movq $10, %rax");
  emitter.movqImmToReg(10, Reg::r8);
  CHECK("49 c7 c0 0a 00 00 00          movq $10, %r8");
  emitter.movqImmToReg(0xFFFFFFFF, Reg::rax);
  CHECK("48 b8 ff ff ff ff 00 00 00 00 movabsq $4294967295, %rax");
  emitter.movqImmToReg(0xFFFFFFFFFF, Reg::r8);
  CHECK("49 b8 ff ff ff ff ff 00 00 00 movabsq $1099511627775, %r8");
  emitter.cmpImmToRM<S::B, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("80 7c 58 20 0a                cmpb $10, 32(%rax,%rbx,2)");
  emitter.cmpImmToRM<S::W, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("66 81 7c 58 20 0a 00          cmpw $10, 32(%rax,%rbx,2)");
  emitter.cmpImmToRM<S::L, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("81 7c 58 20 0a 00 00 00       cmpl $10, 32(%rax,%rbx,2)");
  emitter.cmpImmToRM<S::SLQ, 2>(10, Reg::rax, Reg::rbx, 32);
  CHECK("48 81 7c 58 20 0a 00 00 00    cmpq $10, 32(%rax,%rbx,2)");

  emitter.cmpImmToRM<S::SLQ, ScaleRegAccess>(-1, Reg::rbx, Reg::NoIndex, 0);
  CHECK("48 83 fb ff                   cmpq $-1, %rbx");
  emitter.cmpImmToRM<S::L, ScaleRegAccess>(-1, Reg::ebx, Reg::NoIndex, 0);
  CHECK("83 fb ff                      cmpl $-1, %ebx");
  emitter.cmpImmToRM<S::W, ScaleRegAccess>(-1, Reg::bx, Reg::NoIndex, 0);
  CHECK("66 83 fb ff                   cmpw $-1, %bx");
  emitter.cmpImmToRM<S::B, ScaleRegAccess>(-1, Reg::bl, Reg::NoIndex, 0);
  CHECK("80 fb ff                      cmpb $-1, %bl");

  emitter.cmpImmToRM<S::SLQ, ScaleRegAccess>(-1, Reg::rax, Reg::NoIndex, 0);
  CHECK("48 83 f8 ff                   cmpq $-1, %rax");
  emitter.cmpImmToRM<S::L, ScaleRegAccess>(-1, Reg::eax, Reg::NoIndex, 0);
  CHECK("83 f8 ff                      cmpl $-1, %eax");
  emitter.cmpImmToRM<S::W, ScaleRegAccess>(-1, Reg::ax, Reg::NoIndex, 0);
  CHECK("66 83 f8 ff                   cmpw $-1, %ax");

  emitter.cmpImmToRM<S::B, ScaleRegAccess>(-1, Reg::al, Reg::NoIndex, 0);
  CHECK("3c ff                         cmpb $-1, %al");
  emitter.cmpImmToRM<S::W, ScaleRegAccess>(300, Reg::ax, Reg::NoIndex, 0);
  CHECK("66 3d 2c 01                   cmpw $300, %ax");
  emitter.cmpImmToRM<S::L, ScaleRegAccess>(300, Reg::eax, Reg::NoIndex, 0);
  CHECK("3d 2c 01 00 00                cmpl $300, %eax");
  emitter.cmpImmToRM<S::SLQ, ScaleRegAccess>(300, Reg::rax, Reg::NoIndex, 0);
  CHECK("48 3d 2c 01 00 00             cmpq $300, %rax");

  emitter.testImmToRM<S::B, ScaleRegAccess>(-1, Reg::al, Reg::NoIndex, 0);
  CHECK("a8 ff                         testb $-1, %al");
  emitter.testImmToRM<S::W, ScaleRegAccess>(300, Reg::ax, Reg::NoIndex, 0);
  CHECK("66 a9 2c 01                   testw $300, %ax");
  emitter.testImmToRM<S::L, ScaleRegAccess>(300, Reg::eax, Reg::NoIndex, 0);
  CHECK("a9 2c 01 00 00                testl $300, %eax");
  emitter.testImmToRM<S::SLQ, ScaleRegAccess>(300, Reg::rax, Reg::NoIndex, 0);
  CHECK("48 a9 2c 01 00 00             testq $300, %rax");

  emitter.testImmToRM<S::B, ScaleRegAccess>(-1, Reg::bl, Reg::NoIndex, 0);
  CHECK("f6 c3 ff                      testb $-1, %bl");
  emitter.testImmToRM<S::W, ScaleRegAccess>(300, Reg::bx, Reg::NoIndex, 0);
  CHECK("66 f7 c3 2c 01                testw $300, %bx");
  emitter.testImmToRM<S::L, ScaleRegAccess>(300, Reg::ebx, Reg::NoIndex, 0);
  CHECK("f7 c3 2c 01 00 00             testl $300, %ebx");
  emitter.testImmToRM<S::SLQ, ScaleRegAccess>(300, Reg::rbx, Reg::NoIndex, 0);
  CHECK("48 f7 c3 2c 01 00 00          testq $300, %rbx");

  emitter.testRmToReg<S::B>(Reg::rbx, Reg::NoIndex, 0, Reg::al);
  CHECK("84 03                         testb %al, (%rbx)");
  emitter.testRmToReg<S::W>(Reg::rbx, Reg::NoIndex, 0, Reg::ax);
  CHECK("66 85 03                      testw %ax, (%rbx)");
  emitter.testRmToReg<S::L>(Reg::rbx, Reg::NoIndex, 0, Reg::eax);
  CHECK("85 03                         testl %eax, (%rbx)");
  emitter.testRmToReg<S::Q>(Reg::rbx, Reg::NoIndex, 0, Reg::rax);
  CHECK("48 85 03                      testq %rax, (%rbx)");

  emitter.testRegToReg<S::B>(Reg::bl, Reg::al);
  CHECK("84 d8                         testb %bl, %al");
  emitter.testRegToReg<S::W>(Reg::bx, Reg::ax);
  CHECK("66 85 d8                      testw %bx, %ax");
  emitter.testRegToReg<S::L>(Reg::ebx, Reg::eax);
  CHECK("85 d8                         testl %ebx, %eax");
  emitter.testRegToReg<S::Q>(Reg::rbx, Reg::rax);
  CHECK("48 85 d8                      testq %rbx, %rax");

  emitter.xorRegToReg<S::B>(Reg::rax, Reg::rbx);
  CHECK("30 c3                         xorb %al, %bl");
  emitter.xorRegToReg<S::W>(Reg::rax, Reg::rbx);
  CHECK("66 31 c3                      xorw %ax, %bx");
  emitter.xorRegToReg<S::L>(Reg::rax, Reg::rbx);
  CHECK("31 c3                         xorl %eax, %ebx");
  emitter.xorRegToReg<S::L>(Reg::rax, Reg::r8);
  CHECK("41 31 c0                      xorl %eax, %r8d");
  emitter.xorRegToReg<S::Q>(Reg::rax, Reg::rbx);
  CHECK("48 31 c3                      xorq %rax, %rbx");

  emitter.leaRMToReg<S::Q>(Reg::rax, Reg::NoIndex, 10, Reg::rdx);
  CHECK("48 8d 50 0a                   leaq 10(%rax), %rdx");
  emitter.leaRMToReg<S::L>(Reg::rax, Reg::NoIndex, 10, Reg::rdx);
  CHECK("67 8d 50 0a                   leal 10(%eax), %edx");
  emitter.leaRMToReg<S::L, S::Q>(Reg::rax, Reg::NoIndex, 10, Reg::rdx);
  CHECK("8d 50 0a                      leal 10(%rax), %edx");
  emitter.leaRMToReg<S::W>(Reg::rax, Reg::NoIndex, 10, Reg::rdx);
  CHECK("67 66 8d 50 0a                leaw 10(%eax), %dx");
  emitter.leaRMToReg<S::W, S::Q>(Reg::rax, Reg::NoIndex, 10, Reg::rdx);
  CHECK("66 8d 50 0a                   leaw 10(%rax), %dx");

  emitter.movRMToReg<S::L, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 100, Reg::rax);
  CHECK("8b 05 64 00 00 00             movl 100(%rip), %eax");
  emitter.movRMToReg<S::Q, ScaleRIPAddr32>(
      Reg::none, Reg::NoIndex, 100, Reg::rax);
  CHECK("48 8b 05 64 00 00 00          movq 100(%rip), %rax");

  emitter.ucomisRegToReg(Reg::XMM0, Reg::XMM1);
  CHECK("66 0f 2e c8                   ucomisd %xmm0, %xmm1");
  emitter.ucomisRegToReg<FP::Float>(Reg::XMM0, Reg::XMM1);
  CHECK("0f 2e c8                      ucomiss %xmm0, %xmm1");
  emitter.ucomisRMToReg(Reg::rax, Reg::NoIndex, 0, Reg::XMM1);
  CHECK("66 0f 2e 08                   ucomisd (%rax), %xmm1");
}

#endif

} // namespace
