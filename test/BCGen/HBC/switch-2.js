/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheckOrRegen --match-full-lines %s

function g() {}

function f(x) {
    switch (x) {
        case 0:
            return 32;
        case 1:
            return 342;
        case 2:
            return 322;
        case 3:
            return 132;
        case 4:
            g();
            return 342;
        case 5:
            return 362;
        case 6:
            return 323;
        case 7:
            return 3234;
        case 8:
            return 2332;
        case 9:
            return 3642;
        case 10:
            return 3211;
        case 11:
            return 2332;
        case 12:
            return 3243;
        case 13:
            return 3254;
        case 14:
            return 3342;
        case 15:
            return 3523;
        case 16:
            return 3352;
    }
    switch (x) {
        case 1:
            return 342;
        case 2:
            return 322;
        case 3:
            return 132;
        case 4:
            g();
            return 342;
        case 8:
            return 2332;
        case 9:
            return 3642;
        case 10:
            return 3211;
        case 11:
            return 2332;
        case 12:
            return 3243;
        case 13:
            return 3254;
        case 14:
            return 3342;
        default:
            g();
            break;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 3
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
// CHECK-NEXT:i1[ASCII, 0..0] #00019A16: g
// CHECK-NEXT:i2[ASCII, 6..6] #00019E07: f

// CHECK:Function<global>(1 params, 4 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:[@ 0] CreateTopLevelEnvironment 1<Reg8>, 0<UInt32>
// CHECK-NEXT:[@ 6] DeclareGlobalVar 1<UInt32>
// CHECK-NEXT:[@ 11] DeclareGlobalVar 2<UInt32>
// CHECK-NEXT:[@ 16] CreateClosure 2<Reg8>, 1<Reg8>, 1<UInt16>
// CHECK-NEXT:[@ 21] GetGlobalObject 3<Reg8>
// CHECK-NEXT:[@ 23] PutByIdStrict 3<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt16>
// CHECK-NEXT:[@ 29] CreateClosure 1<Reg8>, 1<Reg8>, 2<UInt16>
// CHECK-NEXT:[@ 34] PutByIdStrict 3<Reg8>, 1<Reg8>, 2<UInt8>, 2<UInt16>
// CHECK-NEXT:[@ 40] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 42] Ret 0<Reg8>

// CHECK:Function<g>(1 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:[@ 0] LoadConstUndefined 0<Reg8>
// CHECK-NEXT:[@ 2] Ret 0<Reg8>

// CHECK:Function<f>(2 params, 12 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:[@ 0] LoadParam 2<Reg8>, 1<UInt8>
// CHECK-NEXT:[@ 3] SwitchImm 2<Reg8>, 292<UInt32>, 161<Addr32>, 0<UInt32>, 16<UInt32>
// CHECK-NEXT:[@ 21] LoadConstInt 0<Reg8>, 3352<Imm32>
// CHECK-NEXT:[@ 27] Ret 0<Reg8>
// CHECK-NEXT:[@ 29] LoadConstInt 0<Reg8>, 3523<Imm32>
// CHECK-NEXT:[@ 35] Ret 0<Reg8>
// CHECK-NEXT:[@ 37] LoadConstInt 0<Reg8>, 3342<Imm32>
// CHECK-NEXT:[@ 43] Ret 0<Reg8>
// CHECK-NEXT:[@ 45] LoadConstInt 0<Reg8>, 3254<Imm32>
// CHECK-NEXT:[@ 51] Ret 0<Reg8>
// CHECK-NEXT:[@ 53] LoadConstInt 0<Reg8>, 3243<Imm32>
// CHECK-NEXT:[@ 59] Ret 0<Reg8>
// CHECK-NEXT:[@ 61] LoadConstInt 0<Reg8>, 2332<Imm32>
// CHECK-NEXT:[@ 67] Ret 0<Reg8>
// CHECK-NEXT:[@ 69] LoadConstInt 0<Reg8>, 3211<Imm32>
// CHECK-NEXT:[@ 75] Ret 0<Reg8>
// CHECK-NEXT:[@ 77] LoadConstInt 0<Reg8>, 3642<Imm32>
// CHECK-NEXT:[@ 83] Ret 0<Reg8>
// CHECK-NEXT:[@ 85] LoadConstInt 0<Reg8>, 2332<Imm32>
// CHECK-NEXT:[@ 91] Ret 0<Reg8>
// CHECK-NEXT:[@ 93] LoadConstInt 0<Reg8>, 3234<Imm32>
// CHECK-NEXT:[@ 99] Ret 0<Reg8>
// CHECK-NEXT:[@ 101] LoadConstInt 0<Reg8>, 323<Imm32>
// CHECK-NEXT:[@ 107] Ret 0<Reg8>
// CHECK-NEXT:[@ 109] LoadConstInt 0<Reg8>, 362<Imm32>
// CHECK-NEXT:[@ 115] Ret 0<Reg8>
// CHECK-NEXT:[@ 117] GetGlobalObject 3<Reg8>
// CHECK-NEXT:[@ 119] GetByIdShort 3<Reg8>, 3<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 124] LoadConstUndefined 1<Reg8>
// CHECK-NEXT:[@ 126] Call1 3<Reg8>, 3<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 130] LoadConstInt 0<Reg8>, 342<Imm32>
// CHECK-NEXT:[@ 136] Ret 0<Reg8>
// CHECK-NEXT:[@ 138] LoadConstUInt8 0<Reg8>, 132<UInt8>
// CHECK-NEXT:[@ 141] Ret 0<Reg8>
// CHECK-NEXT:[@ 143] LoadConstInt 0<Reg8>, 322<Imm32>
// CHECK-NEXT:[@ 149] Ret 0<Reg8>
// CHECK-NEXT:[@ 151] LoadConstInt 0<Reg8>, 342<Imm32>
// CHECK-NEXT:[@ 157] Ret 0<Reg8>
// CHECK-NEXT:[@ 159] LoadConstUInt8 0<Reg8>, 32<UInt8>
// CHECK-NEXT:[@ 162] Ret 0<Reg8>
// CHECK-NEXT:[@ 164] SwitchImm 2<Reg8>, 199<UInt32>, 116<Addr32>, 1<UInt32>, 14<UInt32>
// CHECK-NEXT:[@ 182] LoadConstInt 0<Reg8>, 3342<Imm32>
// CHECK-NEXT:[@ 188] Ret 0<Reg8>
// CHECK-NEXT:[@ 190] LoadConstInt 0<Reg8>, 3254<Imm32>
// CHECK-NEXT:[@ 196] Ret 0<Reg8>
// CHECK-NEXT:[@ 198] LoadConstInt 0<Reg8>, 3243<Imm32>
// CHECK-NEXT:[@ 204] Ret 0<Reg8>
// CHECK-NEXT:[@ 206] LoadConstInt 0<Reg8>, 2332<Imm32>
// CHECK-NEXT:[@ 212] Ret 0<Reg8>
// CHECK-NEXT:[@ 214] LoadConstInt 0<Reg8>, 3211<Imm32>
// CHECK-NEXT:[@ 220] Ret 0<Reg8>
// CHECK-NEXT:[@ 222] LoadConstInt 0<Reg8>, 3642<Imm32>
// CHECK-NEXT:[@ 228] Ret 0<Reg8>
// CHECK-NEXT:[@ 230] LoadConstInt 0<Reg8>, 2332<Imm32>
// CHECK-NEXT:[@ 236] Ret 0<Reg8>
// CHECK-NEXT:[@ 238] GetGlobalObject 2<Reg8>
// CHECK-NEXT:[@ 240] GetByIdShort 2<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 245] LoadConstUndefined 1<Reg8>
// CHECK-NEXT:[@ 247] Call1 2<Reg8>, 2<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 251] LoadConstInt 0<Reg8>, 342<Imm32>
// CHECK-NEXT:[@ 257] Ret 0<Reg8>
// CHECK-NEXT:[@ 259] LoadConstUInt8 0<Reg8>, 132<UInt8>
// CHECK-NEXT:[@ 262] Ret 0<Reg8>
// CHECK-NEXT:[@ 264] LoadConstInt 0<Reg8>, 322<Imm32>
// CHECK-NEXT:[@ 270] Ret 0<Reg8>
// CHECK-NEXT:[@ 272] LoadConstInt 0<Reg8>, 342<Imm32>
// CHECK-NEXT:[@ 278] Ret 0<Reg8>
// CHECK-NEXT:[@ 280] GetGlobalObject 2<Reg8>
// CHECK-NEXT:[@ 282] GetByIdShort 2<Reg8>, 2<Reg8>, 1<UInt8>, 1<UInt8>
// CHECK-NEXT:[@ 287] LoadConstUndefined 1<Reg8>
// CHECK-NEXT:[@ 289] Call1 2<Reg8>, 2<Reg8>, 1<Reg8>
// CHECK-NEXT:[@ 293] Ret 1<Reg8>

// CHECK: Jump Tables:
// CHECK-NEXT:  offset 292
// CHECK-NEXT:   0 : 156
// CHECK-NEXT:   1 : 148
// CHECK-NEXT:   2 : 140
// CHECK-NEXT:   3 : 135
// CHECK-NEXT:   4 : 114
// CHECK-NEXT:   5 : 106
// CHECK-NEXT:   6 : 98
// CHECK-NEXT:   7 : 90
// CHECK-NEXT:   8 : 82
// CHECK-NEXT:   9 : 74
// CHECK-NEXT:   10 : 66
// CHECK-NEXT:   11 : 58
// CHECK-NEXT:   12 : 50
// CHECK-NEXT:   13 : 42
// CHECK-NEXT:   14 : 34
// CHECK-NEXT:   15 : 26
// CHECK-NEXT:   16 : 18
// CHECK-NEXT:  offset 199
// CHECK-NEXT:   1 : 108
// CHECK-NEXT:   2 : 100
// CHECK-NEXT:   3 : 95
// CHECK-NEXT:   4 : 74
// CHECK-NEXT:   5 : 116
// CHECK-NEXT:   6 : 116
// CHECK-NEXT:   7 : 116
// CHECK-NEXT:   8 : 66
// CHECK-NEXT:   9 : 58
// CHECK-NEXT:   10 : 50
// CHECK-NEXT:   11 : 42
// CHECK-NEXT:   12 : 34
// CHECK-NEXT:   13 : 26
// CHECK-NEXT:   14 : 18

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}switch-2.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 23: line 10 col 1
// CHECK-NEXT:    bc 34: line 10 col 1
// CHECK-NEXT:  0x0010  function idx 2, starts at line 12 col 1
// CHECK-NEXT:    bc 119: line 23 col 13
// CHECK-NEXT:    bc 126: line 23 col 14
// CHECK-NEXT:    bc 240: line 58 col 13
// CHECK-NEXT:    bc 247: line 58 col 14
// CHECK-NEXT:    bc 282: line 75 col 13
// CHECK-NEXT:    bc 289: line 75 col 14
// CHECK-NEXT:  0x002a  end of debug source table
