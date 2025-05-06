/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

function simple(x, y) {
  // Check that CacheNewObject is used and refers to the shape table entry.
  this.x = x;
  this.y = y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 4
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
// CHECK-NEXT:i1[ASCII, 6..11] #147A1A16: simple
// CHECK-NEXT:i2[ASCII, 12..12] #0001E7F9: x
// CHECK-NEXT:i3[ASCII, 13..13] #0001E3E8: y

// CHECK:Object Key Buffer:
// CHECK-NEXT:[String 2]
// CHECK-NEXT:[String 3]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 2]
// CHECK-NEXT:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "simple"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<simple>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "simple"
// CHECK-NEXT:    Ret               r0

// CHECK:Function<simple>(3 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    LoadThisNS        r2
// CHECK-NEXT:    GetNewTarget      r1
// CHECK-NEXT:    CacheNewObject    r2, r1, 0, 0
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "x"
// CHECK-NEXT:    LoadParam         r1, 2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "y"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}cache-new-object.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 15: line 12 col 10
// CHECK-NEXT:    bc 24: line 13 col 10
// CHECK-NEXT:  0x0014  end of debug source table
