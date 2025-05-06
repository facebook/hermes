/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xdump-functions=global -target=HBC -dump-bytecode -pretty-disassemble=true -O %s | %FileCheckOrRegen --match-full-lines %s

var x = [true, false, 0, 1, undefined, null];
var y = ["foo", "foo", "bar",,,];
var z = [{}];

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 1
// CHECK-NEXT:  String count: 7
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
// CHECK-NEXT:s0[ASCII, 0..2]: bar
// CHECK-NEXT:s1[ASCII, 3..5]: foo
// CHECK-NEXT:s2[ASCII, 6..11]: global
// CHECK-NEXT:i3[ASCII, 11..16] #15A9FF56: length
// CHECK-NEXT:i4[ASCII, 17..17] #0001E7F9: x
// CHECK-NEXT:i5[ASCII, 18..18] #0001E3E8: y
// CHECK-NEXT:i6[ASCII, 19..19] #0001EFDB: z

// CHECK:Literal Value Buffer:
// CHECK-NEXT:true
// CHECK-NEXT:false
// CHECK-NEXT:[int 0]
// CHECK-NEXT:[int 1]
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 0]
// CHECK-NEXT:Function<global>(1 params, 6 registers, 1 numbers, 2 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "x"
// CHECK-NEXT:    DeclareGlobalVar  "y"
// CHECK-NEXT:    DeclareGlobalVar  "z"
// CHECK-NEXT:    NewArrayWithBuffer r3, 6, 4, 0
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    DefineOwnByIndex  r3, r2, 4
// CHECK-NEXT:    LoadConstNull     r1
// CHECK-NEXT:    DefineOwnByIndex  r3, r1, 5
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutByIdLoose      r4, r3, 0, "x"
// CHECK-NEXT:    NewArrayWithBuffer r3, 5, 3, 11
// CHECK-NEXT:    LoadConstUInt8    r0, 5
// CHECK-NEXT:    PutByIdLoose      r3, r0, 1, "length"
// CHECK-NEXT:    PutByIdLoose      r4, r3, 2, "y"
// CHECK-NEXT:    NewArray          r3, 1
// CHECK-NEXT:    NewObject         r5
// CHECK-NEXT:    DefineOwnByIndex  r3, r5, 0
// CHECK-NEXT:    PutByIdLoose      r4, r3, 3, "z"
// CHECK-NEXT:    Ret               r2

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}array.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 5: line 10 col 1
// CHECK-NEXT:    bc 10: line 10 col 1
// CHECK-NEXT:    bc 25: line 10 col 9
// CHECK-NEXT:    bc 31: line 10 col 9
// CHECK-NEXT:    bc 37: line 10 col 7
// CHECK-NEXT:    bc 54: line 11 col 9
// CHECK-NEXT:    bc 60: line 11 col 7
// CHECK-NEXT:    bc 72: line 12 col 9
// CHECK-NEXT:    bc 76: line 12 col 7
// CHECK-NEXT:  0x0022  end of debug source table
