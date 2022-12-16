/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines

function foo(fn, x) {
  fn(...x);
  new fn(...x);
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

// CHECK:Function<global>(1 params, 7 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    CreateClosure     r3, r0, Function<foo>
// CHECK-NEXT:    PutByIdLoose      r1, r3, 1, "foo"
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    Ret               r5

// CHECK:Function<foo>(3 params, 28 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0007, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    LoadParam         r3, 1
// CHECK-NEXT:    StoreToEnvironment r0, 0, r3
// CHECK-NEXT:    LoadParam         r4, 2
// CHECK-NEXT:    StoreToEnvironment r0, 1, r4
// CHECK-NEXT:    LoadFromEnvironment r5, r0, 0
// CHECK-NEXT:    Mov               r6, r1
// CHECK-NEXT:    LoadFromEnvironment r7, r0, 1
// CHECK-NEXT:    NewArray          r8, 0
// CHECK-NEXT:    Mov               r9, r6
// CHECK-NEXT:    Mov               r20, r8
// CHECK-NEXT:    Mov               r19, r7
// CHECK-NEXT:    Mov               r18, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r6, r10
// CHECK-NEXT:    Mov               r20, r5
// CHECK-NEXT:    Mov               r19, r8
// CHECK-NEXT:    Mov               r18, r2
// CHECK-NEXT:    CallBuiltin       r11, "HermesBuiltin.apply", 4
// CHECK-NEXT:    LoadFromEnvironment r11, r0, 0
// CHECK-NEXT:    Mov               r12, r1
// CHECK-NEXT:    LoadFromEnvironment r13, r0, 1
// CHECK-NEXT:    NewArray          r14, 0
// CHECK-NEXT:    Mov               r15, r12
// CHECK-NEXT:    Mov               r20, r14
// CHECK-NEXT:    Mov               r19, r13
// CHECK-NEXT:    Mov               r18, r15
// CHECK-NEXT:    CallBuiltin       r16, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r12, r16
// CHECK-NEXT:    Mov               r20, r11
// CHECK-NEXT:    Mov               r19, r14
// CHECK-NEXT:    CallBuiltin       r17, "HermesBuiltin.apply", 3
// CHECK-NEXT:    Ret               r2

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}spread-arguments.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 16: line 10 col 1
// CHECK-NEXT:  0x0007  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 47: line 11 col 5
// CHECK-NEXT:    bc 63: line 11 col 5
// CHECK-NEXT:    bc 94: line 12 col 9
// CHECK-NEXT:    bc 107: line 12 col 9
// CHECK-NEXT:  0x0017  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
