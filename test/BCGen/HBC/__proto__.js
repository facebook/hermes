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

// CHECK:Object Key Buffer:
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 2]
// CHECK-NEXT:[String 3]
// CHECK-NEXT:Object Value Buffer:
// CHECK-NEXT:[int 2]
// CHECK-NEXT:[int 3]
// CHECK-NEXT:[int 4]
// CHECK-NEXT:Function<global>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    DeclareGlobalVar  "staticProto"
// CHECK-NEXT:    DeclareGlobalVar  "dynamicProto"
// CHECK-NEXT:    CreateClosure     r2, r0, Function<staticProto>
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r2, 1, "staticProto"
// CHECK-NEXT:    CreateClosure     r0, r0, Function<dynamicProto>
// CHECK-NEXT:    PutByIdLoose      r1, r0, 2, "dynamicProto"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<staticProto>(1 params, 12 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    NewObjectWithBuffer r0, 3, 3, 0, 0
// CHECK-NEXT:    LoadConstNull     r2
// CHECK-NEXT:    Mov               r3, r0
// CHECK-NEXT:    CallBuiltin       r1, "HermesBuiltin.silentSetPrototypeOf", 3
// CHECK-NEXT:    Ret               r0

// CHECK:Function<dynamicProto>(3 params, 13 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0017, lexical 0x0000
// CHECK-NEXT:    NewObject         r0
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Call1             r1, r1, r2
// CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "a"
// CHECK-NEXT:    LoadConstUInt8    r1, 10
// CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "b"
// CHECK-NEXT:    LoadParam         r1, 2
// CHECK-NEXT:    Call1             r3, r1, r2
// CHECK-NEXT:    Mov               r4, r0
// CHECK-NEXT:    CallBuiltin       r1, "HermesBuiltin.silentSetPrototypeOf", 3
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}__proto__.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHECK-NEXT:    bc 2: line 11 col 1
// CHECK-NEXT:    bc 7: line 11 col 1
// CHECK-NEXT:    bc 19: line 11 col 1
// CHECK-NEXT:    bc 30: line 11 col 1
// CHECK-NEXT:  0x0010  function idx 1, starts at line 11 col 1
// CHECK-NEXT:    bc 15: line 12 col 10
// CHECK-NEXT:  0x0017  function idx 2, starts at line 15 col 1
// CHECK-NEXT:    bc 7: line 16 col 18
// CHECK-NEXT:    bc 11: line 16 col 10
// CHECK-NEXT:    bc 18: line 16 col 10
// CHECK-NEXT:    bc 25: line 16 col 48
// CHECK-NEXT:    bc 32: line 16 col 29
// CHECK-NEXT:  0x002a  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
