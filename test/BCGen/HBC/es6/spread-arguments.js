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
// CHECK-NEXT:i1[ASCII, 6..8] #9290584E: foo

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateClosure     r1, r1, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "foo"
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r1, r2
// CHECK-NEXT:    Ret               r1

// CHECK:Function<foo>(3 params, 18 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000b
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r3, r3, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    StoreToEnvironment r3, 0, r1
// CHECK-NEXT:    LoadParam         r1, 2
// CHECK-NEXT:    StoreToEnvironment r3, 1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r3, 0
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    Mov               r5, r2
// CHECK-NEXT:    LoadFromEnvironment r2, r3, 1
// CHECK-NEXT:    NewArray          r4, 0
// CHECK-NEXT:    Mov               r6, r5
// CHECK-NEXT:    Mov               r9, r4
// CHECK-NEXT:    Mov               r8, r2
// CHECK-NEXT:    Mov               r7, r6
// CHECK-NEXT:    CallBuiltin       r2, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r5, r2
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r9, r1
// CHECK-NEXT:    Mov               r8, r4
// CHECK-NEXT:    Mov               r7, r2
// CHECK-NEXT:    CallBuiltin       r0, "HermesBuiltin.apply", 4
// CHECK-NEXT:    LoadFromEnvironment r1, r3, 0
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    LoadFromEnvironment r3, r3, 1
// CHECK-NEXT:    NewArray          r2, 0
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    Mov               r9, r2
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    Mov               r7, r5
// CHECK-NEXT:    CallBuiltin       r3, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:    Mov               r4, r3
// CHECK-NEXT:    Mov               r9, r1
// CHECK-NEXT:    Mov               r8, r2
// CHECK-NEXT:    CallBuiltin       r0, "HermesBuiltin.apply", 3
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}spread-arguments.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000b  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 53: line 11 col 5
// CHECK-NEXT:    bc 71: line 11 col 5
// CHECK-NEXT:    bc 104: line 12 col 9
// CHECK-NEXT:    bc 117: line 12 col 9
// CHECK-NEXT:  0x001c  end of debug source table
