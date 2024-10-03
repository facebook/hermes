/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(x) {
  this.x = x;
}

function bar() {
  return new foo(1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 4
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
// CHECK-NEXT:i1[ASCII, 6..8] #9B85A7ED: bar
// CHECK-NEXT:i2[ASCII, 9..11] #9290584E: foo
// CHECK-NEXT:i3[ASCII, 12..12] #0001E7F9: x

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] CreateTopLevelEnvironment 2<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 6] DeclareGlobalVar 2<UInt32>
// CHECK-NEXT:[@ 11] DeclareGlobalVar 1<UInt32>
// CHECK-NEXT:[@ 16] CreateClosure 0<Reg8>, 2<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 21] GetGlobalObject 1<Reg8>
// CHECK-NEXT:[@ 23] PutByIdLoose 1<Reg8>, 0<Reg8>, 1<UInt8>, 2<UInt16>
// CHECK-NEXT:[@ 29] CreateClosure 2<Reg8>, 2<Reg8>, 2<UInt16>
// CHECK-NEXT:[@ 34] PutByIdLoose 1<Reg8>, 2<Reg8>, 2<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 40] LoadConstUndefined 2<Reg8>
// CHECK-NEXT:[@ 42] Ret 2<Reg8>

// CHECK:Function<foo>(2 params, 2 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:[@ 0] LoadParam 0<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] LoadThisNS 1<Reg8>
// CHECK-NEXT:[@ 5] PutByIdLoose 1<Reg8>, 0<Reg8>, 1<UInt8>, 3<UInt16>
// CHECK-NEXT:[@ 11] LoadConstUndefined 1<Reg8>
// CHECK-NEXT:[@ 13] Ret 1<Reg8>

// CHECK:Function<bar>(1 params, 12 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0017, lexical 0x0000
// CHECK-NEXT:[@ 0] GetGlobalObject 2<Reg8>
// CHECK-NEXT:[@ 2] GetByIdShort 0<Reg8>, 2<Reg8>, 1<UInt8>, 2<UInt8>
// CHECK-NEXT:[@ 7] CreateThisForNew 1<Reg8>, 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 11] LoadConstUInt8 3<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 14] Mov 4<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 17] Construct 2<Reg8>, 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 21] SelectObject 2<Reg8>, 1<Reg8>, 2<Reg8>
// CHECK-NEXT:[@ 25] Ret 2<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}construct.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 23: line 10 col 1
// CHECK-NEXT:    bc 34: line 10 col 1
// CHECK-NEXT:  0x0010  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 5: line 11 col 10
// CHECK-NEXT:  0x0017  function idx 2, starts at line 14 col 1
// CHECK-NEXT:    bc 2: line 15 col 14
// CHECK-NEXT:    bc 7: line 15 col 17
// CHECK-NEXT:    bc 17: line 15 col 17
// CHECK-NEXT:  0x0024  end of debug source table
