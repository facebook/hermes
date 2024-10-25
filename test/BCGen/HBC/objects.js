/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(p) {
  var obj = {a: 1};
  obj.a = 1;
  obj[p] = 1;
  obj["b"] = obj.a;
  obj["2"] = obj[p];
  delete obj["b"];
  delete obj[p];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
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
// CHECK-NEXT:i1[ASCII, 4..4] #00018270: a
// CHECK-NEXT:i2[ASCII, 6..6] #00018E43: b
// CHECK-NEXT:i3[ASCII, 7..9] #9290584E: foo

// CHECK:Literal Value Buffer:
// CHECK-NEXT:[int 1]
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 1]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 1]
// CHECK-NEXT:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] DeclareGlobalVar 3<UInt32>
// CHECK-NEXT:[@ 5] CreateTopLevelEnvironment 1<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 11] CreateClosure 2<Reg8>, 1<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 16] GetGlobalObject 1<Reg8>
// CHECK-NEXT:[@ 18] PutByIdLoose 1<Reg8>, 2<Reg8>, 1<UInt8>, 3<UInt16>
// CHECK-NEXT:[@ 24] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 26] Ret 0<Reg8>

// CHECK:Function<foo>(2 params, 5 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:[@ 0] LoadParam 3<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] NewObjectWithBuffer 2<Reg8>, 0<UInt16>, 0<UInt16>
// CHECK-NEXT:[@ 9] LoadConstUInt8 0<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 12] PutByIdLoose 2<Reg8>, 0<Reg8>, 1<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 18] PutByValLoose 2<Reg8>, 3<Reg8>, 0<Reg8>
// CHECK-NEXT:[@ 22] GetByIdShort 4<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 27] PutByIdLoose 2<Reg8>, 4<Reg8>, 2<UInt8>, 2<UInt16>
// CHECK-NEXT:[@ 33] GetByVal 4<Reg8>, 2<Reg8>, 3<Reg8>
// CHECK-NEXT:[@ 37] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 40] PutByValLoose 2<Reg8>, 0<Reg8>, 4<Reg8>
// CHECK-NEXT:[@ 44] DelByIdLoose 1<Reg8>, 2<Reg8>, 2<UInt16>
// CHECK-NEXT:[@ 49] DelByValLoose 1<Reg8>, 2<Reg8>, 3<Reg8>
// CHECK-NEXT:[@ 53] LoadConstUndefined 1<Reg8>
// CHECK-NEXT:[@ 55] Ret 1<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}objects.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 12: line 12 col 9
// CHECK-NEXT:    bc 18: line 13 col 10
// CHECK-NEXT:    bc 22: line 14 col 17
// CHECK-NEXT:    bc 27: line 14 col 12
// CHECK-NEXT:    bc 33: line 15 col 17
// CHECK-NEXT:    bc 40: line 15 col 12
// CHECK-NEXT:    bc 44: line 16 col 3
// CHECK-NEXT:    bc 49: line 17 col 3
// CHECK-NEXT:  0x0026  end of debug source table
