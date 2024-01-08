/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -target=HBC %s -O -g | %FileCheckOrRegen %s --match-full-lines

function test1() {
  var count = 0;
  for(var count=0; ; count++) {
    if (Math.random() === 3)
      break;
    if (Math.random() === 5)
      for(;;){} // infinite loop
  }
  while (count > 10)
    count--;
  print(count);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 5
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
// CHECK-NEXT:s0[ASCII, 0..5]: global
// CHECK-NEXT:i1[ASCII, 6..9] #1C182460: Math
// CHECK-NEXT:i2[ASCII, 10..14] #A689F65B: print
// CHECK-NEXT:i3[ASCII, 14..18] #13935A76: test1
// CHECK-NEXT:i4[ASCII, 19..24] #50223B1A: random

// CHECK:Function<global>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "test1"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<test1>
// CHECK-NEXT:    GetGlobalObject   r0
// CHECK-NEXT:    PutByIdLoose      r0, r1, 1, "test1"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r0

// CHECK:Function<test1>(1 params, 18 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0011, lexical 0x0002
// CHECK-NEXT:    LoadConstUInt8    r6, 1
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstUInt8    r4, 5
// CHECK-NEXT:    LoadConstUInt8    r3, 3
// CHECK-NEXT:    LoadConstZero     r5
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:L3:
// CHECK-NEXT:    TryGetById        r7, r1, 1, "Math"
// CHECK-NEXT:    GetByIdShort      r2, r7, 2, "random"
// CHECK-NEXT:    Call1             r7, r2, r7
// CHECK-NEXT:    Mov               r2, r5
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    JStrictEqual      L1, r7, r3
// CHECK-NEXT:    TryGetById        r8, r1, 1, "Math"
// CHECK-NEXT:    GetByIdShort      r7, r8, 2, "random"
// CHECK-NEXT:    Call1             r7, r7, r8
// CHECK-NEXT:    JStrictEqual      L2, r7, r4
// CHECK-NEXT:    AddN              r5, r2, r6
// CHECK-NEXT:    Jmp               L3
// CHECK-NEXT:L2:
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Jmp               L2
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r5, 10
// CHECK-NEXT:    Greater           r3, r2, r5
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    Mov               r2, r4
// CHECK-NEXT:    JmpFalse          L4, r3
// CHECK-NEXT:L5:
// CHECK-NEXT:    SubN              r7, r4, r6
// CHECK-NEXT:    Greater           r3, r7, r5
// CHECK-NEXT:    Mov               r4, r7
// CHECK-NEXT:    Mov               r2, r4
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    JmpTrue           L5, r3
// CHECK-NEXT:L4:
// CHECK-NEXT:    TryGetById        r1, r1, 3, "print"
// CHECK-NEXT:    Call2             r1, r1, r0, r2
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}debuggercheckbreak.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 7: line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:    bc 23: line 21 col 1
// CHECK-NEXT:  0x0011  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 16: line 13 col 9
// CHECK-NEXT:    bc 22: line 13 col 20
// CHECK-NEXT:    bc 27: line 13 col 20
// CHECK-NEXT:    bc 35: line 13 col 5
// CHECK-NEXT:    bc 39: line 15 col 9
// CHECK-NEXT:    bc 45: line 15 col 20
// CHECK-NEXT:    bc 50: line 15 col 20
// CHECK-NEXT:    bc 54: line 15 col 5
// CHECK-NEXT:    bc 62: line 12 col 3
// CHECK-NEXT:    bc 65: line 16 col 7
// CHECK-NEXT:    bc 80: line 18 col 3
// CHECK-NEXT:    bc 98: line 18 col 3
// CHECK-NEXT:    bc 101: line 20 col 3
// CHECK-NEXT:    bc 107: line 20 col 8
// CHECK-NEXT:    bc 112: line 21 col 1
// CHECK-NEXT:  0x0048  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  lexical parent: 0, variable count: 0
// CHECK-NEXT:  0x0004  end of debug lexical table
