/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// Code generation for static proto
function staticProto() {
  return {__proto__: null, a: 2, b: 3, c: 4};
}

function dynamicProto(func, getProto) {
  return {a: func(), b: 10, __proto__: getProto()};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 6
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
// CHECK-NEXT:i1[ASCII, 4..4] #00018270: a
// CHECK-NEXT:i2[ASCII, 6..6] #00018E43: b
// CHECK-NEXT:i3[ASCII, 7..7] #00018A52: c
// CHECK-NEXT:i4[ASCII, 8..19] #E721285C: dynamicProto
// CHECK-NEXT:i5[ASCII, 20..30] #99489473: staticProto

// CHECK:Literal Value Buffer:
// CHECK-NEXT:[int 2]
// CHECK-NEXT:[int 3]
// CHECK-NEXT:[int 4]
// CHECK-NEXT:null
// CHECK-NEXT:[int 10]
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 2]
// CHECK-NEXT:[String 3]
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 2]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 3]
// CHECK-NEXT:1[4, 2]
// CHECK-NEXT:Function<global>(1 params, 4 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    DeclareGlobalVar  "staticProto"
// CHECK-NEXT:    DeclareGlobalVar  "dynamicProto"
// CHECK-NEXT:    CreateClosure     r2, r1, Function<staticProto>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r2, 1, "staticProto"
// CHECK-NEXT:    CreateClosure     r1, r1, Function<dynamicProto>
// CHECK-NEXT:    PutByIdLoose      r3, r1, 2, "dynamicProto"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<staticProto>(1 params, 13 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    LoadConstNull     r3
// CHECK-NEXT:    Mov               r4, r1
// CHECK-NEXT:    CallBuiltin       r2, "HermesBuiltin.silentSetPrototypeOf", 3
// CHECK-NEXT:    Ret               r1

// CHECK:Function<dynamicProto>(3 params, 13 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0017, lexical 0x0000
// CHECK-NEXT:    NewObjectWithBuffer r2, 1, 13
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Call1             r1, r1, r0
// CHECK-NEXT:    PutOwnBySlotIdx   r2, r1, 0
// CHECK-NEXT:    LoadParam         r1, 2
// CHECK-NEXT:    Call1             r3, r1, r0
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    CallBuiltin       r1, "HermesBuiltin.silentSetPrototypeOf", 3
// CHECK-NEXT:    Ret               r2

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}__proto__.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHECK-NEXT:    bc 6: line 11 col 1
// CHECK-NEXT:    bc 11: line 11 col 1
// CHECK-NEXT:    bc 23: line 11 col 1
// CHECK-NEXT:    bc 34: line 11 col 1
// CHECK-NEXT:  0x0010  function idx 1, starts at line 11 col 1
// CHECK-NEXT:    bc 11: line 12 col 10
// CHECK-NEXT:  0x0017  function idx 2, starts at line 15 col 1
// CHECK-NEXT:    bc 11: line 16 col 18
// CHECK-NEXT:    bc 22: line 16 col 48
// CHECK-NEXT:    bc 29: line 16 col 29
// CHECK-NEXT:  0x0024  end of debug source table
