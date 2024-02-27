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

// CHECK:Function<global>(1 params, 7 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateFunctionEnvironment r0
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    Mov               r3, r4
// CHECK-NEXT:    Mov               r5, r3
// CHECK-NEXT:    Ret               r5

// CHECK:Function<foo>(3 params, 32 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 2
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    StoreToEnvironment r1, 1, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r1, 0
// CHECK-NEXT:    LoadConstZero     r6
// CHECK-NEXT:    Mov               r5, r6
// CHECK-NEXT:    LoadFromEnvironment r7, r1, 1
// CHECK-NEXT:    NewArray          r8, 0
// CHECK-NEXT:    Mov               r9, r5
// CHECK-NEXT:    Mov               r23, r8
// CHECK-NEXT:    Mov               r22, r7
// CHECK-NEXT:    Mov               r21, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r5, r10
// CHECK-NEXT:    LoadConstUndefined r11
// CHECK-NEXT:    Mov               r23, r4
// CHECK-NEXT:    Mov               r22, r8
// CHECK-NEXT:    Mov               r21, r11
// CHECK-NEXT:    CallBuiltin       r12, "HermesBuiltin.apply", 4
// CHECK-NEXT:    LoadFromEnvironment r12, r1, 0
// CHECK-NEXT:    LoadConstZero     r14
// CHECK-NEXT:    Mov               r13, r14
// CHECK-NEXT:    LoadFromEnvironment r15, r1, 1
// CHECK-NEXT:    NewArray          r16, 0
// CHECK-NEXT:    Mov               r17, r13
// CHECK-NEXT:    Mov               r23, r16
// CHECK-NEXT:    Mov               r22, r15
// CHECK-NEXT:    Mov               r21, r17
// CHECK-NEXT:    CallBuiltin       r18, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r13, r18
// CHECK-NEXT:    Mov               r23, r12
// CHECK-NEXT:    Mov               r22, r16
// CHECK-NEXT:    CallBuiltin       r19, "HermesBuiltin.apply", 3
// CHECK-NEXT:    LoadConstUndefined r19
// CHECK-NEXT:    Ret               r19

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}spread-arguments.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 53: line 11 col 5
// CHECK-NEXT:    bc 71: line 11 col 5
// CHECK-NEXT:    bc 104: line 12 col 9
// CHECK-NEXT:    bc 117: line 12 col 9
// CHECK-NEXT:  0x001a  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
