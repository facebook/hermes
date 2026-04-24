/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// Ensure the shape table has two separate entries.
var a = {x: 1, __proto__: null};
var b = {x: 1, __proto__: Math};

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 1
// CHECK-NEXT:  String count: 5
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  StringSwitchImm count: 0
// CHECK-NEXT:  Key buffer size (bytes): 3
// CHECK-NEXT:  Value buffer size (bytes): 5
// CHECK-NEXT:  Shape table count: 3
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
// CHECK-NEXT:i2[ASCII, 6..9] #1C182460: Math
// CHECK-NEXT:i3[ASCII, 10..10] #00018E43: b
// CHECK-NEXT:i4[ASCII, 11..11] #0001E7F9: x

// CHECK:Literal Value Buffer:
// CHECK-NEXT:[int 1]

// CHECK:Object Key Buffer:
// CHECK-NEXT:[String 4]

// CHECK:Object Shape Table:
// CHECK-NEXT:0[0, 0]
// CHECK-NEXT:1[0, 1]
// CHECK-NEXT:2[0, 1]

// CHECK:Function<global>(1 params, 14 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "a"
// CHECK-NEXT:    DeclareGlobalVar  "b"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    LoadConstNull     r0
// CHECK-NEXT:    NewObjectWithBufferAndParent r1, r0, 1, 0
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "a"
// CHECK-NEXT:    NewObjectWithBuffer r1, 2, 0
// CHECK-NEXT:    TryGetById        r4, r2, 0, "Math"
// CHECK-NEXT:    Mov               r5, r1
// CHECK-NEXT:    CallBuiltin       r3, "HermesBuiltin.silentSetPrototypeOf", 3
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "b"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}shape-table-proto.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHECK-NEXT:    bc 0: line 11 col 1
// CHECK-NEXT:    bc 5: line 11 col 1
// CHECK-NEXT:    bc 25: line 11 col 7
// CHECK-NEXT:    bc 37: line 12 col 27
// CHECK-NEXT:    bc 46: line 12 col 16
// CHECK-NEXT:    bc 50: line 12 col 7
// CHECK-NEXT:  0x0017  end of debug source table
