/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc %s --dump-bytecode | %FileCheckOrRegen %s

function testForOfFunction() {
    for (const [a, b = function() {  a, b }] of []) {
      (b, (function() {  b() })());
    }
  }

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 3
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..0]: b
// CHECK-NEXT:s1[ASCII, 1..6]: global
// CHECK-NEXT:i2[ASCII, 7..23] #BC01556D: testForOfFunction

// CHECK:Array Buffer:
// CHECK-NEXT:Function<global>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "testForOfFunction"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<testForOfFunction>
// CHECK-NEXT:    GetGlobalObject   r0
// CHECK-NEXT:    PutById           r0, r1, 1, "testForOfFunction"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<testForOfFunction>(1 params, 19 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0009, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r3
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    NewArray          r1, 0
// CHECK-NEXT:    IteratorBegin     r2, r1
// CHECK-NEXT:L8:
// CHECK-NEXT:    IteratorNext      r6, r2, r1
// CHECK-NEXT:    Mov               r5, r2
// CHECK-NEXT:    JStrictEqual      L1, r5, r0
// CHECK-NEXT:L10:
// CHECK-NEXT:    Mov               r8, r6
// CHECK-NEXT:    IteratorBegin     r5, r8
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    LoadConstUndefined r7
// CHECK-NEXT:    IteratorNext      r9, r5, r8
// CHECK-NEXT:    Mov               r10, r5
// CHECK-NEXT:    StrictEq          r10, r10, r0
// CHECK-NEXT:    Mov               r6, r10
// CHECK-NEXT:    JmpTrue           L2, r10
// CHECK-NEXT:    Mov               r7, r9
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUndefined r7
// CHECK-NEXT:    Mov               r9, r6
// CHECK-NEXT:    JmpTrue           L3, r9
// CHECK-NEXT:    IteratorNext      r8, r5, r8
// CHECK-NEXT:    Mov               r9, r5
// CHECK-NEXT:    StrictEq          r9, r9, r0
// CHECK-NEXT:    Mov               r6, r9
// CHECK-NEXT:    JmpTrue           L4, r9
// CHECK-NEXT:    Mov               r7, r8
// CHECK-NEXT:L3:
// CHECK-NEXT:    Mov               r8, r7
// CHECK-NEXT:    JStrictNotEqual   L5, r8, r0
// CHECK-NEXT:L4:
// CHECK-NEXT:    CreateClosure     r7, r3, Function<b>
// CHECK-NEXT:L5:
// CHECK-NEXT:    Mov               r4, r7
// CHECK-NEXT:    JmpTrue           L6, r6
// CHECK-NEXT:    IteratorClose     r5, 0
// CHECK-NEXT:L6:
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    Call1             r5, r5, r0
// CHECK-NEXT:L11:
// CHECK-NEXT:    Jmp               L8
// CHECK-NEXT:L7:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    IteratorClose     r2, 1
// CHECK-NEXT:    Throw             r1
// CHECK-NEXT:L1:
// CHECK-NEXT:    Ret               r0

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L10, end = L11, target = L7

// CHECK:Function<b>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}regress-inlining-scopes.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1 scope offset 0x0000 env r0
// CHECK-NEXT:  0x0009  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 10: line 11 col 5 scope offset 0x0000 env r2
// CHECK-NEXT:    bc 13: line 11 col 5 scope offset 0x0000 env r6
// CHECK-NEXT:    bc 27: line 11 col 5 scope offset 0x0000 env r5
// CHECK-NEXT:    bc 34: line 11 col 5 scope offset 0x0000 env r9
// CHECK-NEXT:    bc 62: line 11 col 5 scope offset 0x0000 env r8
// CHECK-NEXT:    bc 100: line 11 col 5 scope offset 0x0000 env r5
// CHECK-NEXT:    bc 106: line 12 col 27 scope offset 0x0000 env r5
// CHECK-NEXT:    bc 110: line 13 col 5 scope offset 0x0000 env r5
// CHECK-NEXT:    bc 112: line 13 col 5 scope offset 0x0000 env r1
// CHECK-NEXT:    bc 114: line 13 col 5 scope offset 0x0000 env r2
// CHECK-NEXT:    bc 117: line 13 col 5 scope offset 0x0000 env r1
// CHECK-NEXT:  0x0044  end of debug source table

// CHECK:Debug scope descriptor table:
// CHECK-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// CHECK-NEXT:  0x0003  end of debug scope descriptor table

// CHECK:Textified callees table:
// CHECK-NEXT:  0x0000  entries: 0
// CHECK-NEXT:  0x0001  end of textified callees table

// CHECK:Debug string table:
// CHECK-NEXT:  0x0000  end of debug string table
