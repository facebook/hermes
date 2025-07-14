/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -target=HBC %s -O0 -Xg3 | %FileCheckOrRegen %s --match-full-lines

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
// CHECK-NEXT:  StringSwitchImm count: 0
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
// CHECK-NEXT:i3[ASCII, 15..20] #50223B1A: random
// CHECK-NEXT:i4[ASCII, 21..25] #13935A76: test1

// CHECK:Function<global>(1 params, 4 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r2, 0
// CHECK-NEXT:    DeclareGlobalVar  "test1"
// CHECK-NEXT:    CreateClosure     r1, r2, Function<test1>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r1, 0, "test1"
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r1, r3
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r1

// CHECK:Function<test1>(1 params, 14 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x001e
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r2, r1, 1
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r2, 0, r1
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StoreNPToEnvironment r2, 0, r1
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StoreNPToEnvironment r2, 0, r1
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:L3:
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    TryGetById        r1, r1, 0, "Math"
// CHECK-NEXT:    GetByIdShort      r4, r1, 1, "random"
// CHECK-NEXT:    Call1             r4, r4, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StrictEq          r4, r4, r1
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    JmpTrue           L1, r4
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    TryGetById        r4, r4, 0, "Math"
// CHECK-NEXT:    GetByIdShort      r1, r4, 1, "random"
// CHECK-NEXT:    Call1             r1, r1, r4
// CHECK-NEXT:    LoadConstUInt8    r4, 5
// CHECK-NEXT:    StrictEq          r1, r1, r4
// CHECK-NEXT:    JmpTrue           L2, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 0
// CHECK-NEXT:    ToNumeric         r1, r1
// CHECK-NEXT:    Inc               r1, r1
// CHECK-NEXT:    StoreToEnvironment r2, 0, r1
// CHECK-NEXT:    Jmp               L3
// CHECK-NEXT:L2:
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Jmp               L2
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 0
// CHECK-NEXT:    LoadConstUInt8    r4, 10
// CHECK-NEXT:    Greater           r1, r1, r4
// CHECK-NEXT:    JmpFalse          L4, r1
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 0
// CHECK-NEXT:    ToNumeric         r1, r1
// CHECK-NEXT:    Dec               r1, r1
// CHECK-NEXT:    StoreToEnvironment r2, 0, r1
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 0
// CHECK-NEXT:    LoadConstUInt8    r4, 10
// CHECK-NEXT:    Greater           r1, r1, r4
// CHECK-NEXT:    JmpTrue           L5, r1
// CHECK-NEXT:L4:
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    TryGetById        r1, r1, 2, "print"
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 0
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call2             r0, r1, r3, r4
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}debuggercheckbreak.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:    bc 24: line 10 col 1
// CHECK-NEXT:    bc 29: line 10 col 1
// CHECK-NEXT:    bc 30: line 21 col 1
// CHECK-NEXT:  0x001e  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 3: line 10 col 1
// CHECK-NEXT:    bc 12: line 10 col 1
// CHECK-NEXT:    bc 18: line 11 col 13
// CHECK-NEXT:    bc 24: line 12 col 16
// CHECK-NEXT:    bc 31: line 13 col 9
// CHECK-NEXT:    bc 37: line 13 col 20
// CHECK-NEXT:    bc 42: line 13 col 20
// CHECK-NEXT:    bc 49: line 13 col 9
// CHECK-NEXT:    bc 54: line 13 col 5
// CHECK-NEXT:    bc 59: line 15 col 9
// CHECK-NEXT:    bc 65: line 15 col 20
// CHECK-NEXT:    bc 70: line 15 col 20
// CHECK-NEXT:    bc 77: line 15 col 9
// CHECK-NEXT:    bc 81: line 15 col 5
// CHECK-NEXT:    bc 84: line 12 col 22
// CHECK-NEXT:    bc 88: line 12 col 27
// CHECK-NEXT:    bc 91: line 12 col 27
// CHECK-NEXT:    bc 94: line 12 col 27
// CHECK-NEXT:    bc 98: line 12 col 3
// CHECK-NEXT:    bc 100: line 16 col 7
// CHECK-NEXT:    bc 101: line 16 col 7
// CHECK-NEXT:    bc 103: line 18 col 10
// CHECK-NEXT:    bc 110: line 18 col 10
// CHECK-NEXT:    bc 114: line 18 col 3
// CHECK-NEXT:    bc 117: line 19 col 5
// CHECK-NEXT:    bc 121: line 19 col 10
// CHECK-NEXT:    bc 124: line 19 col 10
// CHECK-NEXT:    bc 127: line 19 col 10
// CHECK-NEXT:    bc 132: line 18 col 10
// CHECK-NEXT:    bc 139: line 18 col 10
// CHECK-NEXT:    bc 143: line 18 col 3
// CHECK-NEXT:    bc 148: line 20 col 3
// CHECK-NEXT:    bc 154: line 20 col 9
// CHECK-NEXT:    bc 160: line 20 col 8
// CHECK-NEXT:    bc 167: line 21 col 1
// CHECK-NEXT:  0x00a5  end of debug source table
