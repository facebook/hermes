/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s -Xdump-functions=fibonacci | %FileCheckOrRegen --match-full-lines %s

function fibonacci(num) {
  if (num <= 1) return 1;

  return fibonacci(num - 1) + fibonacci(num - 2);
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
// CHECK-NEXT:i1[ASCII, 6..14] #85794EA3: fibonacci

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] DeclareGlobalVar 1<UInt32>
// CHECK-NEXT:[@ 5] CreateTopLevelEnvironment 1<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 11] CreateClosure 2<Reg8>, 1<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 16] GetGlobalObject 1<Reg8>
// CHECK-NEXT:[@ 18] PutByIdLoose 1<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 24] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 26] Ret 0<Reg8>

// CHECK:Function<fibonacci>(2 params, 15 registers, 2 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:[@ 0] LoadParam 5<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] LoadConstUInt8 1<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 6] JLessEqual 45<Addr8>, 5<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 10] GetGlobalObject 3<Reg8>
// CHECK-NEXT:[@ 12] GetByIdShort 4<Reg8>, 3<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 17] Sub 0<Reg8>, 5<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 21] LoadConstUndefined 2<Reg8>
// CHECK-NEXT:[@ 23] Call2 4<Reg8>, 4<Reg8>, 2<Reg8>, 0<Reg8>
// CHECK-NEXT:[@ 28] GetByIdShort 3<Reg8>, 3<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 33] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 36] Sub 0<Reg8>, 5<Reg8>, 0<Reg8>
// CHECK-NEXT:[@ 40] Call2 3<Reg8>, 3<Reg8>, 2<Reg8>, 0<Reg8>
// CHECK-NEXT:[@ 45] Add 3<Reg8>, 4<Reg8>, 3<Reg8>
// CHECK-NEXT:[@ 49] Ret 3<Reg8>
// CHECK-NEXT:[@ 51] Ret 1<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}fibonacci.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 11 col 3
// CHECK-NEXT:    bc 12: line 13 col 10
// CHECK-NEXT:    bc 17: line 13 col 20
// CHECK-NEXT:    bc 23: line 13 col 19
// CHECK-NEXT:    bc 28: line 13 col 31
// CHECK-NEXT:    bc 36: line 13 col 41
// CHECK-NEXT:    bc 40: line 13 col 40
// CHECK-NEXT:    bc 45: line 13 col 10
// CHECK-NEXT:  0x0026  end of debug source table
