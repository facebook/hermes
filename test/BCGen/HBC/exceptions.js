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
// CHECK-NEXT:  Bytecode version number: 90
// CHECK-NEXT:  Source hash: 0000000000000000000000000000000000000000
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

// CHECK:Function<global>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r0
// CHECK-NEXT:    PutByIdLoose      r0, r1, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<foo>(2 params, 11 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0007, lexical 0x0000
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:L10:
// CHECK-NEXT:    Mov               r0, r1
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:L11:
// CHECK-NEXT:    Jmp               L2
// CHECK-NEXT:L1:
// CHECK-NEXT:    Catch             r3
// CHECK-NEXT:L8:
// CHECK-NEXT:    Mov               r0, r3
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:L9:
// CHECK-NEXT:    Jmp               L4
// CHECK-NEXT:L3:
// CHECK-NEXT:    Catch             r0
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:L4:
// CHECK-NEXT:    Mov               r0, r3
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:L2:
// CHECK-NEXT:    Mov               r0, r1
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L5:
// CHECK-NEXT:    Catch             r0
// CHECK-NEXT:    Call1             r3, r3, r2
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L6:
// CHECK-NEXT:    Catch             r0
// CHECK-NEXT:    Call1             r1, r1, r2
// CHECK-NEXT:    Throw             r0

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L8, end = L9, target = L3
// CHECK-NEXT:1: start = L10, end = L11, target = L1
// CHECK-NEXT:2: start = L8, end = L4, target = L5
// CHECK-NEXT:3: start = L10, end = L2, target = L6
// CHECK-NEXT:4: start = L5, end = L6, target = L6

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}exceptions.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:  0x0007  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 10: line 12 col 6
// CHECK-NEXT:    bc 14: line 13 col 3
// CHECK-NEXT:    bc 16: line 13 col 5
// CHECK-NEXT:    bc 21: line 15 col 8
// CHECK-NEXT:    bc 25: line 16 col 5
// CHECK-NEXT:    bc 27: line 16 col 7
// CHECK-NEXT:    bc 29: line 17 col 8
// CHECK-NEXT:    bc 33: line 18 col 5
// CHECK-NEXT:    bc 36: line 20 col 8
// CHECK-NEXT:    bc 40: line 22 col 3
// CHECK-NEXT:    bc 43: line 24 col 6
// CHECK-NEXT:    bc 49: line 19 col 13
// CHECK-NEXT:    bc 51: line 20 col 8
// CHECK-NEXT:    bc 55: line 21 col 5
// CHECK-NEXT:    bc 57: line 23 col 11
// CHECK-NEXT:    bc 59: line 24 col 6
// CHECK-NEXT:    bc 63: line 25 col 3
// CHECK-NEXT:  0x003e  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
