/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(a) {
  if (a)
    return 1;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 4;
  else if (a)
    return 5;
  else if (a)
    return 6;
  else if (a)
    return 7;
  else if (a)
    return 8;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else if (a)
    return 4;
  else if (a)
    return 5;
  else if (a)
    return 6;
  else if (a)
    return 7;
  else if (a)
    return 8;
  else if (a)
    return 2;
  else if (a)
    return 3;
  else
    return 9;
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

// CHECK:Function<foo>(2 params, 2 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:[@ 0] LoadParam 1<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] JmpTrueLong 164<Addr32>, 1<Reg8>
// CHECK-NEXT:[@ 9] JmpTrueLong 153<Addr32>, 1<Reg8>
// CHECK-NEXT:[@ 15] JmpTrueLong 142<Addr32>, 1<Reg8>
// CHECK-NEXT:[@ 21] JmpTrueLong 131<Addr32>, 1<Reg8>
// CHECK-NEXT:[@ 27] JmpTrue 120<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 30] JmpTrue 112<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 33] JmpTrue 104<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 36] JmpTrue 96<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 39] JmpTrue 88<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 42] JmpTrue 80<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 45] JmpTrue 72<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 48] JmpTrue 64<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 51] JmpTrue 56<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 54] JmpTrue 48<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 57] JmpTrue 40<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 60] JmpTrue 32<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 63] JmpTrue 24<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 66] JmpTrue 16<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 69] JmpTrue 8<Addr8>, 1<Reg8>
// CHECK-NEXT:[@ 72] LoadConstUInt8 0<Reg8>, 9<UInt8>
// CHECK-NEXT:[@ 75] Ret 0<Reg8>
// CHECK-NEXT:[@ 77] LoadConstUInt8 0<Reg8>, 3<UInt8>
// CHECK-NEXT:[@ 80] Ret 0<Reg8>
// CHECK-NEXT:[@ 82] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 85] Ret 0<Reg8>
// CHECK-NEXT:[@ 87] LoadConstUInt8 0<Reg8>, 8<UInt8>
// CHECK-NEXT:[@ 90] Ret 0<Reg8>
// CHECK-NEXT:[@ 92] LoadConstUInt8 0<Reg8>, 7<UInt8>
// CHECK-NEXT:[@ 95] Ret 0<Reg8>
// CHECK-NEXT:[@ 97] LoadConstUInt8 0<Reg8>, 6<UInt8>
// CHECK-NEXT:[@ 100] Ret 0<Reg8>
// CHECK-NEXT:[@ 102] LoadConstUInt8 0<Reg8>, 5<UInt8>
// CHECK-NEXT:[@ 105] Ret 0<Reg8>
// CHECK-NEXT:[@ 107] LoadConstUInt8 0<Reg8>, 4<UInt8>
// CHECK-NEXT:[@ 110] Ret 0<Reg8>
// CHECK-NEXT:[@ 112] LoadConstUInt8 0<Reg8>, 3<UInt8>
// CHECK-NEXT:[@ 115] Ret 0<Reg8>
// CHECK-NEXT:[@ 117] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 120] Ret 0<Reg8>
// CHECK-NEXT:[@ 122] LoadConstUInt8 0<Reg8>, 3<UInt8>
// CHECK-NEXT:[@ 125] Ret 0<Reg8>
// CHECK-NEXT:[@ 127] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 130] Ret 0<Reg8>
// CHECK-NEXT:[@ 132] LoadConstUInt8 0<Reg8>, 8<UInt8>
// CHECK-NEXT:[@ 135] Ret 0<Reg8>
// CHECK-NEXT:[@ 137] LoadConstUInt8 0<Reg8>, 7<UInt8>
// CHECK-NEXT:[@ 140] Ret 0<Reg8>
// CHECK-NEXT:[@ 142] LoadConstUInt8 0<Reg8>, 6<UInt8>
// CHECK-NEXT:[@ 145] Ret 0<Reg8>
// CHECK-NEXT:[@ 147] LoadConstUInt8 0<Reg8>, 5<UInt8>
// CHECK-NEXT:[@ 150] Ret 0<Reg8>
// CHECK-NEXT:[@ 152] LoadConstUInt8 0<Reg8>, 4<UInt8>
// CHECK-NEXT:[@ 155] Ret 0<Reg8>
// CHECK-NEXT:[@ 157] LoadConstUInt8 0<Reg8>, 3<UInt8>
// CHECK-NEXT:[@ 160] Ret 0<Reg8>
// CHECK-NEXT:[@ 162] LoadConstUInt8 0<Reg8>, 2<UInt8>
// CHECK-NEXT:[@ 165] Ret 0<Reg8>
// CHECK-NEXT:[@ 167] LoadConstUInt8 0<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 170] Ret 0<Reg8>

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}jumps.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  end of debug source table
