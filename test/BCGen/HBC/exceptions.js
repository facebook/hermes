/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(a) {
  try {
    a();
  } catch (e) {
    try {
      e();
    } catch (e) {
      e();
    }
    finally {
      e();
    }
  }
  finally {
    a();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 2
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
// CHECK-NEXT:i1[ASCII, 6..8] #9290584E: foo

// CHECK:Function<global>(1 params, 2 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    CreateClosure     r0, r1, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r0, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1

// CHECK:Function<foo>(2 params, 12 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:L9:
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call1             r3, r2, r3
// CHECK-NEXT:    Jmp               L2
// CHECK-NEXT:L1:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:L8:
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call1             r3, r1, r3
// CHECK-NEXT:    Jmp               L4
// CHECK-NEXT:L3:
// CHECK-NEXT:    Catch             r0
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call1             r3, r0, r3
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call1             r3, r1, r3
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Call1             r0, r2, r3
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L5:
// CHECK-NEXT:    Catch             r3
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Call1             r0, r1, r0
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L6:
// CHECK-NEXT:    Catch             r3
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Call1             r0, r2, r0
// CHECK-NEXT:    Throw             r3

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L8, end = L3, target = L3
// CHECK-NEXT:1: start = L9, end = L1, target = L1
// CHECK-NEXT:2: start = L8, end = L4, target = L5
// CHECK-NEXT:3: start = L9, end = L2, target = L6
// CHECK-NEXT:4: start = L5, end = L6, target = L6

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}exceptions.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 3: line 11 col 3
// CHECK-NEXT:    bc 5: line 12 col 6
// CHECK-NEXT:    bc 9: line 13 col 3
// CHECK-NEXT:    bc 11: line 13 col 5
// CHECK-NEXT:    bc 13: line 14 col 5
// CHECK-NEXT:    bc 15: line 15 col 8
// CHECK-NEXT:    bc 19: line 16 col 5
// CHECK-NEXT:    bc 21: line 16 col 7
// CHECK-NEXT:    bc 25: line 17 col 8
// CHECK-NEXT:    bc 29: line 18 col 5
// CHECK-NEXT:    bc 31: line 20 col 8
// CHECK-NEXT:    bc 35: line 22 col 3
// CHECK-NEXT:    bc 37: line 24 col 6
// CHECK-NEXT:    bc 43: line 19 col 13
// CHECK-NEXT:    bc 47: line 20 col 8
// CHECK-NEXT:    bc 51: line 21 col 5
// CHECK-NEXT:    bc 53: line 23 col 11
// CHECK-NEXT:    bc 57: line 24 col 6
// CHECK-NEXT:    bc 61: line 25 col 3
// CHECK-NEXT:  0x0047  end of debug source table
