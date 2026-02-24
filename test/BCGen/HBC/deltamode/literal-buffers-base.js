/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s -O -emit-binary -out %t-base.hbc && %hermes %s -base-bytecode=%t-base.hbc -O -dump-bytecode | %FileCheckOrRegen --match-full-lines %s

// This file acts as a base source code to test compiling other files with
// delta optimizing mode; also compile itself with delta optimizing mode, with
// the normally compiled bytecode as base bytecode, to test the case when no
// new literals are added in the new bytecode.

function a1() {
  return [1, 2, 3];
}

function a2() {
  return [4, 5, 6];
}

function o1() {
  return {
    a: 1,
    b: 2,
    c: 3,
  };
}

function o2() {
  return {
    a: 1,
    b: 2,
    c: 3,
  };
}

function o3() {
  return {
    d: 1,
    e: 2,
    f: 3,
  };
}

function o4() {
  return {
    a: 4,
    b: 5,
  };
}

function o5() {
  return {
    g: 6,
    h: 7,
  };
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 8
// CHECK-NEXT:  String count: 16
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  StringSwitchImm count: 0
// CHECK-NEXT:  Key buffer size (bytes): 24
// CHECK-NEXT:  Value buffer size (bytes): 44
// CHECK-NEXT:  Shape table count: 4
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..5]: global
// CHECK-NEXT:i1[ASCII, 0..0] #00019A16: g
// CHECK-NEXT:i2[ASCII, 4..4] #00018270: a
// CHECK-NEXT:i3[ASCII, 6..7] #061436BB: a1
// CHECK-NEXT:i4[ASCII, 8..9] #06143A88: a2
// CHECK-NEXT:i5[ASCII, 10..10] #00018E43: b
// CHECK-NEXT:i6[ASCII, 11..11] #00018A52: c
// CHECK-NEXT:i7[ASCII, 12..12] #00019625: d
// CHECK-NEXT:i8[ASCII, 13..13] #00019234: e
// CHECK-NEXT:i9[ASCII, 14..14] #00019E07: f
// CHECK-NEXT:i10[ASCII, 15..15] #0001A6E9: h
// CHECK-NEXT:i11[ASCII, 16..17] #06F74514: o1
// CHECK-NEXT:i12[ASCII, 18..19] #06F7493B: o2
// CHECK-NEXT:i13[ASCII, 20..21] #06F74D2A: o3
// CHECK-NEXT:i14[ASCII, 22..23] #06F6B6D9: o4
// CHECK-NEXT:i15[ASCII, 24..25] #06F6B2C8: o5

// CHECK:Literal Value Buffer:
// CHECK-NEXT:[int 1]
// CHECK-NEXT:[int 2]
// CHECK-NEXT:[int 3]
// CHECK-NEXT:[int 4]
// CHECK-NEXT:[int 5]
// CHECK-NEXT:[int 6]
// CHECK-NEXT:[int 4]
// CHECK-NEXT:[int 5]
// CHECK-NEXT:[int 6]
// CHECK-NEXT:[int 7]

// CHECK:Object Key Buffer:
// CHECK-NEXT:[String 2]
// CHECK-NEXT:[String 5]
// CHECK-NEXT:[String 6]
// CHECK-NEXT:[String 7]
// CHECK-NEXT:[String 8]
// CHECK-NEXT:[String 9]
// CHECK-NEXT:[String 2]
// CHECK-NEXT:[String 5]
// CHECK-NEXT:[String 1]
// CHECK-NEXT:[String 10]

// CHECK:Object Shape Table:
// CHECK-NEXT:0[0, 3]
// CHECK-NEXT:1[7, 3]
// CHECK-NEXT:2[14, 2]
// CHECK-NEXT:3[19, 2]

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "a1"
// CHECK-NEXT:    DeclareGlobalVar  "a2"
// CHECK-NEXT:    DeclareGlobalVar  "o1"
// CHECK-NEXT:    DeclareGlobalVar  "o2"
// CHECK-NEXT:    DeclareGlobalVar  "o3"
// CHECK-NEXT:    DeclareGlobalVar  "o4"
// CHECK-NEXT:    DeclareGlobalVar  "o5"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<a1>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "a1"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<a2>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "a2"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<o1>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 2, "o1"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<o2>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 3, "o2"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<o3>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 4, "o3"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<o4>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 5, "o4"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<o5>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 6, "o5"
// CHECK-NEXT:    Ret               r0

// CHECK:Function<a1>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewArrayWithBuffer r0, 3, 3, 0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<a2>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewArrayWithBuffer r0, 3, 3, 13
// CHECK-NEXT:    Ret               r0

// CHECK:Function<o1>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewObjectWithBuffer r0, 0, 0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<o2>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewObjectWithBuffer r0, 0, 0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<o3>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewObjectWithBuffer r0, 1, 0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<o4>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewObjectWithBuffer r0, 2, 26
// CHECK-NEXT:    Ret               r0

// CHECK:Function<o5>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    NewObjectWithBuffer r0, 3, 35
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}literal-buffers-base.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 15 col 1
// CHECK-NEXT:    bc 0: line 15 col 1
// CHECK-NEXT:    bc 5: line 15 col 1
// CHECK-NEXT:    bc 10: line 15 col 1
// CHECK-NEXT:    bc 15: line 15 col 1
// CHECK-NEXT:    bc 20: line 15 col 1
// CHECK-NEXT:    bc 25: line 15 col 1
// CHECK-NEXT:    bc 30: line 15 col 1
// CHECK-NEXT:    bc 44: line 15 col 1
// CHECK-NEXT:    bc 55: line 15 col 1
// CHECK-NEXT:    bc 66: line 15 col 1
// CHECK-NEXT:    bc 77: line 15 col 1
// CHECK-NEXT:    bc 88: line 15 col 1
// CHECK-NEXT:    bc 99: line 15 col 1
// CHECK-NEXT:    bc 110: line 15 col 1
// CHECK-NEXT:  0x002f  end of debug source table
