/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s
"use strict";

function foo(x, y, z) { }
foo(0,0,1);

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 2
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
// CHECK-NEXT:i1[ASCII, 6..8] #9290584E: foo

// CHECK:Function<global>(1 params, 16 registers, 2 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateTopLevelEnvironment r3, 0
// CHECK-NEXT:    CreateClosure     r4, r3, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdStrict     r3, r4, 1, "foo"
// CHECK-NEXT:    GetByIdShort      r3, r3, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    Call4             r3, r3, r2, r0, r0, r1
// CHECK-NEXT:    Ret               r3

// CHECK:Function<foo>(4 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}call_function.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 9 col 1
// CHECK-NEXT:    bc 0: line 9 col 1
// CHECK-NEXT:    bc 18: line 9 col 1
// CHECK-NEXT:    bc 24: line 12 col 1
// CHECK-NEXT:    bc 36: line 12 col 4
// CHECK-NEXT:  0x0010  end of debug source table
