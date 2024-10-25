/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode --basic-block-profiling -O %s | %FileCheckOrRegen --match-full-lines %s

var condition = false;
try {
  try {
    print(condition? "yes": "no");
  } finally {
    print("rethrowing");
  }
} catch (e) {
  print(e.stack);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 1
// CHECK-NEXT:  String count: 7
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
// CHECK-NEXT:s0[ASCII, 0..9]: rethrowing
// CHECK-NEXT:s1[ASCII, 9..14]: global
// CHECK-NEXT:s2[ASCII, 23..24]: no
// CHECK-NEXT:s3[ASCII, 25..27]: yes
// CHECK-NEXT:i4[ASCII, 15..23] #431898FB: condition
// CHECK-NEXT:i5[ASCII, 27..31] #834F633C: stack
// CHECK-NEXT:i6[ASCII, 32..36] #A689F65B: print

// CHECK:Function<global>(1 params, 16 registers, 0 numbers, 2 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    ProfilePoint      9
// CHECK-NEXT:    DeclareGlobalVar  "condition"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    LoadConstFalse    r1
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdStrict     r3, r1, 1, "condition"
// CHECK-NEXT:L8:
// CHECK-NEXT:    ProfilePoint      6
// CHECK-NEXT:L6:
// CHECK-NEXT:    ProfilePoint      4
// CHECK-NEXT:    TryGetById        r4, r3, 1, "print"
// CHECK-NEXT:    GetByIdShort      r5, r3, 2, "condition"
// CHECK-NEXT:    LoadConstString   r2, "no"
// CHECK-NEXT:    JmpFalse          L1, r5
// CHECK-NEXT:    ProfilePoint      3
// CHECK-NEXT:    LoadConstString   r2, "yes"
// CHECK-NEXT:L1:
// CHECK-NEXT:    ProfilePoint      2
// CHECK-NEXT:    Call2             r6, r4, r0, r2
// CHECK-NEXT:L7:
// CHECK-NEXT:    ProfilePoint      1
// CHECK-NEXT:    TryGetById        r4, r3, 1, "print"
// CHECK-NEXT:    LoadConstString   r2, "rethrowing"
// CHECK-NEXT:    Call2             r6, r4, r0, r2
// CHECK-NEXT:    Jmp               L3
// CHECK-NEXT:L2:
// CHECK-NEXT:    Catch             r2
// CHECK-NEXT:    ProfilePoint      5
// CHECK-NEXT:    TryGetById        r5, r3, 1, "print"
// CHECK-NEXT:    LoadConstString   r4, "rethrowing"
// CHECK-NEXT:    Call2             r6, r5, r0, r4
// CHECK-NEXT:    Throw             r2
// CHECK-NEXT:L4:
// CHECK-NEXT:    Catch             r2
// CHECK-NEXT:    ProfilePoint      8
// CHECK-NEXT:    TryGetById        r3, r3, 1, "print"
// CHECK-NEXT:    GetByIdShort      r2, r2, 3, "stack"
// CHECK-NEXT:    Call2             r6, r3, r0, r2
// CHECK-NEXT:L3:
// CHECK-NEXT:    ProfilePoint      7
// CHECK-NEXT:    Ret               r6

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L6, end = L7, target = L2
// CHECK-NEXT:1: start = L8, end = L4, target = L4

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}basic_block_profiler.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 3: line 10 col 1
// CHECK-NEXT:    bc 16: line 10 col 15
// CHECK-NEXT:    bc 22: line 11 col 1
// CHECK-NEXT:    bc 25: line 12 col 3
// CHECK-NEXT:    bc 28: line 13 col 5
// CHECK-NEXT:    bc 34: line 13 col 11
// CHECK-NEXT:    bc 56: line 13 col 10
// CHECK-NEXT:    bc 61: line 14 col 3
// CHECK-NEXT:    bc 64: line 15 col 5
// CHECK-NEXT:    bc 74: line 15 col 10
// CHECK-NEXT:    bc 79: line 17 col 1
// CHECK-NEXT:    bc 81: line 14 col 13
// CHECK-NEXT:    bc 86: line 15 col 5
// CHECK-NEXT:    bc 96: line 15 col 10
// CHECK-NEXT:    bc 101: line 16 col 3
// CHECK-NEXT:    bc 103: line 17 col 3
// CHECK-NEXT:    bc 108: line 18 col 3
// CHECK-NEXT:    bc 114: line 18 col 10
// CHECK-NEXT:    bc 119: line 18 col 8
// CHECK-NEXT:  0x003d  end of debug source table
