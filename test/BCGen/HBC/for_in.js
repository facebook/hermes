/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -pretty-disassemble=false -target=HBC %s -O | %FileCheckOrRegen %s --match-full-lines
// This checks that compilation without optimization at least doesn't fail hard
// RUN: %hermes -dump-bytecode -target=HBC %s
// This tests disassembleBytecode in CompilerDriver.cpp
// RUN: %hermes -emit-binary -O -target=HBC -out %t %s && %hermes -dump-bytecode -pretty-disassemble=false -b %t | %FileCheck %s --match-full-lines

function test_one(x, f) {
  for (var v in x) {
    x[v];
  }
}

test_one(1,2,3)

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
// CHECK-NEXT:i1[ASCII, 6..13] #33D4E32D: test_one

// CHECK:Function<global>(1 params, 17 registers, 3 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] DeclareGlobalVar 1<UInt32>
// CHECK-NEXT:[@ 5] CreateTopLevelEnvironment 4<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 11] CreateClosure 5<Reg8>, 4<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 16] GetGlobalObject 4<Reg8>
// CHECK-NEXT:[@ 18] PutByIdLoose 4<Reg8>, 5<Reg8>, 1<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 24] GetByIdShort 4<Reg8>, 4<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 29] LoadConstUndefined 3<Reg8>
// CHECK-NEXT:[@ 31] LoadConstUInt8 0<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 34] LoadConstUInt8 1<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 37] LoadConstUInt8 2<Reg8>, 3<UInt8>
// CHECK-NEXT:[@ 40] Call4 4<Reg8>, 4<Reg8>, 3<Reg8>, 0<Reg8>, 1<Reg8>, 2<Reg8>
// CHECK-NEXT:[@ 47] Ret 4<Reg8>

// CHECK:Function<test_one>(3 params, 8 registers, 2 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:[@ 0] LoadParam 4<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] Mov 7<Reg8>, 4<Reg8>
// CHECK-NEXT:[@ 6] GetPNameList 6<Reg8>, 7<Reg8>, 0<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 11] JmpUndefined 21<Addr8>, 6<Reg8>
// CHECK-NEXT:[@ 14] GetNextPName 5<Reg8>, 6<Reg8>, 7<Reg8>, 0<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 20] JmpUndefined 12<Addr8>, 5<Reg8>
// CHECK-NEXT:[@ 23] Mov 3<Reg8>, 5<Reg8>
// CHECK-NEXT:[@ 26] GetByVal 3<Reg8>, 4<Reg8>, 3<Reg8>
// CHECK-NEXT:[@ 30] Jmp -16<Addr8>
// CHECK-NEXT:[@ 32] LoadConstUndefined 2<Reg8>
// CHECK-NEXT:[@ 34] Ret 2<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}for_in.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 14 col 1
// CHECK-NEXT:    bc 0: line 14 col 1
// CHECK-NEXT:    bc 18: line 14 col 1
// CHECK-NEXT:    bc 24: line 20 col 1
// CHECK-NEXT:    bc 40: line 20 col 9
// CHECK-NEXT:  0x0010  function idx 1, starts at line 14 col 1
// CHECK-NEXT:    bc 6: line 15 col 3
// CHECK-NEXT:    bc 14: line 15 col 3
// CHECK-NEXT:    bc 26: line 16 col 6
// CHECK-NEXT:  0x001d  end of debug source table
