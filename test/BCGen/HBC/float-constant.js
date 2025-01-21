/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=true -O %s | %FileCheckOrRegen --match-full-lines %s

var w = 3.14;
var x = -0.00056;
var y = 12345670.89;
var z = 0.0;

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 1
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
// CHECK-NEXT:i1[ASCII, 6..6] #0001DB06: w
// CHECK-NEXT:i2[ASCII, 7..7] #0001E7F9: x
// CHECK-NEXT:i3[ASCII, 8..8] #0001E3E8: y
// CHECK-NEXT:i4[ASCII, 9..9] #0001EFDB: z

// CHECK:Function<global>(1 params, 3 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "w"
// CHECK-NEXT:    DeclareGlobalVar  "x"
// CHECK-NEXT:    DeclareGlobalVar  "y"
// CHECK-NEXT:    DeclareGlobalVar  "z"
// CHECK-NEXT:    LoadConstDouble   r0, 3.14
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r0, 1, "w"
// CHECK-NEXT:    LoadConstDouble   r0, -0.00056
// CHECK-NEXT:    PutByIdLoose      r2, r0, 2, "x"
// CHECK-NEXT:    LoadConstDouble   r0, 12345670.89
// CHECK-NEXT:    PutByIdLoose      r2, r0, 3, "y"
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    PutByIdLoose      r2, r0, 4, "z"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}float-constant.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 5: line 10 col 1
// CHECK-NEXT:    bc 10: line 10 col 1
// CHECK-NEXT:    bc 15: line 10 col 1
// CHECK-NEXT:    bc 32: line 10 col 7
// CHECK-NEXT:    bc 48: line 11 col 7
// CHECK-NEXT:    bc 64: line 12 col 7
// CHECK-NEXT:    bc 72: line 13 col 7
// CHECK-NEXT:  0x001c  end of debug source table
