/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -strict -fno-inline -dump-bytecode -fstrip-function-names -O %s | %FileCheckOrRegen --match-full-lines %s

Math['function-name-stripped'] = 123;

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 1
// CHECK-NEXT:  String count: 2
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 1
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:i0[ASCII, 0..3] #1C182460: Math
// CHECK-NEXT:i1[ASCII, 4..25] #D7615A1F: function-name-stripped

// CHECK:Function<function-name-stripped>(1 params, 2 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    TryGetById        r1, r1, 1, "Math"
// CHECK-NEXT:    LoadConstUInt8    r0, 123
// CHECK-NEXT:    PutByIdStrict     r1, r0, 1, "function-name-str"...
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}strip_func_name_id.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 32
// CHECK-NEXT:  0x000a  end of debug source table
