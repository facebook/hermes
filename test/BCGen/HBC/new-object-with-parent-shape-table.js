/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// Show that the shape table is used correctly for NewObjectWithParent.
var a = {__proto__: Math};
var b = {__proto__: JSON};

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
// CHECK-NEXT:  Key buffer size (bytes): 0
// CHECK-NEXT:  Value buffer size (bytes): 0
// CHECK-NEXT:  Shape table count: 1
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
// CHECK-NEXT:i2[ASCII, 6..9] #971CE5C7: JSON
// CHECK-NEXT:i3[ASCII, 10..13] #1C182460: Math
// CHECK-NEXT:i4[ASCII, 14..14] #00018E43: b

// CHECK:Object Shape Table:
// CHECK-NEXT:0[0, 0]

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "a"
// CHECK-NEXT:    DeclareGlobalVar  "b"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    TryGetById        r1, r2, 0, "Math"
// CHECK-NEXT:    NewObjectWithParent r1, r1, 0
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "a"
// CHECK-NEXT:    TryGetById        r1, r2, 1, "JSON"
// CHECK-NEXT:    NewObjectWithParent r1, r1, 0
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "b"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}new-object-with-parent-shape-table.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHECK-NEXT:    bc 0: line 11 col 1
// CHECK-NEXT:    bc 5: line 11 col 1
// CHECK-NEXT:    bc 12: line 11 col 21
// CHECK-NEXT:    bc 25: line 11 col 7
// CHECK-NEXT:    bc 31: line 12 col 21
// CHECK-NEXT:    bc 44: line 12 col 7
// CHECK-NEXT:  0x0017  end of debug source table
