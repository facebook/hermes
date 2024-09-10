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

// CHECK:Function<global>(1 params, 2 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "test1"
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    CreateClosure     r0, r1, Function<test1>
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r0, 1, "test1"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r1

// CHECK:Function<test1>(1 params, 18 registers, 4 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0014, lexical 0x0000
// CHECK-NEXT:    LoadConstUInt8    r2, 1
// CHECK-NEXT:    GetGlobalObject   r8
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    LoadConstUInt8    r3, 5
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:L3:
// CHECK-NEXT:    TryGetById        r6, r8, 1, "Math"
// CHECK-NEXT:    GetByIdShort      r7, r6, 2, "random"
// CHECK-NEXT:    Call1             r6, r7, r6
// CHECK-NEXT:    Mov               r7, r0
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    JStrictEqual      L1, r6, r1
// CHECK-NEXT:    TryGetById        r5, r8, 1, "Math"
// CHECK-NEXT:    GetByIdShort      r6, r5, 2, "random"
// CHECK-NEXT:    Call1             r6, r6, r5
// CHECK-NEXT:    JStrictEqual      L2, r6, r3
// CHECK-NEXT:    AddN              r0, r7, r2
// CHECK-NEXT:    Jmp               L3
// CHECK-NEXT:L2:
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Jmp               L2
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r3, 10
// CHECK-NEXT:    Mov               r1, r7
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    JNotGreaterN      L4, r7, r3
// CHECK-NEXT:L5:
// CHECK-NEXT:    SubN              r1, r1, r2
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    JGreaterN         L5, r7, r3
// CHECK-NEXT:L4:
// CHECK-NEXT:    TryGetById        r8, r8, 3, "print"
// CHECK-NEXT:    Call2             r8, r8, r4, r7
// CHECK-NEXT:    Ret               r4

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}debuggercheckbreak.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 5: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:    bc 27: line 21 col 1
// CHECK-NEXT:  0x0014  function idx 1, starts at line 10 col 1
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
// CHECK-NEXT:    bc 76: line 18 col 3
// CHECK-NEXT:    bc 88: line 18 col 3
// CHECK-NEXT:    bc 92: line 20 col 3
// CHECK-NEXT:    bc 98: line 20 col 8
// CHECK-NEXT:    bc 103: line 21 col 1
// CHECK-NEXT:  0x0052  end of debug source table
