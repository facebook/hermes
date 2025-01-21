/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
    return 42;
}

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

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] DeclareGlobalVar 1<UInt32>
// CHECK-NEXT:[@ 5] CreateTopLevelEnvironment 1<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 11] CreateClosure 2<Reg8>, 1<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 16] GetGlobalObject 1<Reg8>
// CHECK-NEXT:[@ 18] PutByIdLoose 1<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 24] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 26] Ret 0<Reg8>

// CHECK:Function<foo>(1 params, 1 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:[@ 0] LoadConstUInt8 0<Reg8>, 42<UInt8>
// CHECK-NEXT:[@ 3] Ret 0<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}return.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  end of debug source table
