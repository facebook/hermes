/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=HBC
// RUN: %hermesc -O -dump-bytecode -pretty=false %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=HBCRAW

// Verify that switches that do backward branches correctly generate negative
// offsets.  This is shown by the negative offset for case "4" in the
// string switch table of the "raw" disassembly.
function stringSwitch(x) {
  'noinline'
  for (;;) {
    print('hi');
    switch (x) {
        case "0":
            return 32;
        case "1":
            return 342;
        case "2":
            return 322;
        case "3":
            return 132;
        case "4":
            continue;
        case "5":
            return 362;
        case "6":
            return 323;
        case "7":
            return 3234;
        case "8":
            return 2332;
        case "9":
            return 3642;
        case "10":
            return 3211;
        case "11":
            return 2332;
        case "12":
            return 3243;
        case "13":
            return 3254;
        case "14":
            return 3342;
        case "15":
            return 3523;
        case "16":
            return 3352;
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// HBC:Bytecode File Information:
// HBC-NEXT:  Bytecode version number: {{.*}}
// HBC-NEXT:  Source hash: {{.*}}
// HBC-NEXT:  Function count: 2
// HBC-NEXT:  String count: 21
// HBC-NEXT:  BigInt count: 0
// HBC-NEXT:  String Kind Entry count: 2
// HBC-NEXT:  RegExp count: 0
// HBC-NEXT:  StringSwitchImm count: 1
// HBC-NEXT:  Key buffer size (bytes): 0
// HBC-NEXT:  Value buffer size (bytes): 0
// HBC-NEXT:  Shape table count: 0
// HBC-NEXT:  Segment ID: 0
// HBC-NEXT:  CommonJS module count: 0
// HBC-NEXT:  CommonJS module count (static): 0
// HBC-NEXT:  Function source count: 0
// HBC-NEXT:  Bytecode options:
// HBC-NEXT:    staticBuiltins: 0
// HBC-NEXT:    cjsModulesStaticallyResolved: 0

// HBC:Global String Table:
// HBC-NEXT:s0[ASCII, 0..1]: 11
// HBC-NEXT:s1[ASCII, 1..1]: 1
// HBC-NEXT:s2[ASCII, 1..2]: 10
// HBC-NEXT:s3[ASCII, 2..2]: 0
// HBC-NEXT:s4[ASCII, 3..4]: 12
// HBC-NEXT:s5[ASCII, 4..4]: 2
// HBC-NEXT:s6[ASCII, 5..6]: 13
// HBC-NEXT:s7[ASCII, 6..6]: 3
// HBC-NEXT:s8[ASCII, 7..8]: 14
// HBC-NEXT:s9[ASCII, 8..8]: 4
// HBC-NEXT:s10[ASCII, 9..10]: 15
// HBC-NEXT:s11[ASCII, 10..10]: 5
// HBC-NEXT:s12[ASCII, 11..12]: 16
// HBC-NEXT:s13[ASCII, 12..12]: 6
// HBC-NEXT:s14[ASCII, 13..13]: 7
// HBC-NEXT:s15[ASCII, 14..14]: 8
// HBC-NEXT:s16[ASCII, 15..15]: 9
// HBC-NEXT:s17[ASCII, 16..21]: global
// HBC-NEXT:s18[ASCII, 33..34]: hi
// HBC-NEXT:i19[ASCII, 22..33] #646CA4B1: stringSwitch
// HBC-NEXT:i20[ASCII, 35..39] #A689F65B: print

// HBC:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// HBC-NEXT:Offset in debug table: source 0x0000
// HBC-NEXT:    DeclareGlobalVar  "stringSwitch"
// HBC-NEXT:    GetGlobalObject   r2
// HBC-NEXT:    LoadConstUndefined r0
// HBC-NEXT:    CreateClosure     r1, r0, Function<stringSwitch>
// HBC-NEXT:    PutByIdLoose      r2, r1, 0, "stringSwitch"
// HBC-NEXT:    Ret               r0

// HBC:Function<stringSwitch>(2 params, 15 registers, 1 numbers, 1 non-pointers):
// HBC-NEXT:Offset in debug table: source 0x000b
// HBC-NEXT:    LoadParam         r4, 1
// HBC-NEXT:    GetGlobalObject   r3
// HBC-NEXT:    LoadConstString   r2, "hi"
// HBC-NEXT:    LoadConstUndefined r1
// HBC-NEXT:L5:
// HBC-NEXT:    TryGetById        r5, r3, 0, "print"
// HBC-NEXT:    Call2             r5, r5, r1, r2
// HBC-NEXT:    StringSwitchImm   r4, 0, 140, L5, 17
// HBC-NEXT:L17:
// HBC-NEXT:    LoadConstInt      r0, 3352
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L16:
// HBC-NEXT:    LoadConstInt      r0, 3523
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L15:
// HBC-NEXT:    LoadConstInt      r0, 3342
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L14:
// HBC-NEXT:    LoadConstInt      r0, 3254
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L13:
// HBC-NEXT:    LoadConstInt      r0, 3243
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L12:
// HBC-NEXT:    LoadConstInt      r0, 2332
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L11:
// HBC-NEXT:    LoadConstInt      r0, 3211
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L10:
// HBC-NEXT:    LoadConstInt      r0, 3642
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L9:
// HBC-NEXT:    LoadConstInt      r0, 2332
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L8:
// HBC-NEXT:    LoadConstInt      r0, 3234
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L7:
// HBC-NEXT:    LoadConstInt      r0, 323
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L6:
// HBC-NEXT:    LoadConstInt      r0, 362
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L4:
// HBC-NEXT:    LoadConstUInt8    r0, 132
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L3:
// HBC-NEXT:    LoadConstInt      r0, 322
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L2:
// HBC-NEXT:    LoadConstInt      r0, 342
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L1:
// HBC-NEXT:    LoadConstUInt8    r0, 32
// HBC-NEXT:    Ret               r0

// HBC: String Switch Tables:
// HBC-NEXT:  offset 140
// HBC-NEXT:   "0" : L1
// HBC-NEXT:   "1" : L2
// HBC-NEXT:   "2" : L3
// HBC-NEXT:   "3" : L4
// HBC-NEXT:   "4" : L5
// HBC-NEXT:   "5" : L6
// HBC-NEXT:   "6" : L7
// HBC-NEXT:   "7" : L8
// HBC-NEXT:   "8" : L9
// HBC-NEXT:   "9" : L10
// HBC-NEXT:   "10" : L11
// HBC-NEXT:   "11" : L12
// HBC-NEXT:   "12" : L13
// HBC-NEXT:   "13" : L14
// HBC-NEXT:   "14" : L15
// HBC-NEXT:   "15" : L16
// HBC-NEXT:   "16" : L17

// HBC:Debug filename table:
// HBC-NEXT:  0: {{.*}}switch-of-strings-back-branch.js

// HBC:Debug file table:
// HBC-NEXT:  source table offset 0x0000: filename id 0

// HBC:Debug source table:
// HBC-NEXT:  0x0000  function idx 0, starts at line 14 col 1
// HBC-NEXT:    bc 0: line 14 col 1
// HBC-NEXT:    bc 14: line 14 col 1
// HBC-NEXT:  0x000b  function idx 1, starts at line 14 col 1
// HBC-NEXT:    bc 11: line 17 col 5
// HBC-NEXT:    bc 17: line 17 col 10
// HBC-NEXT:  0x0016  end of debug source table

// HBCRAW:Bytecode File Information:
// HBCRAW-NEXT:  Bytecode version number: {{.*}}
// HBCRAW-NEXT:  Source hash: {{.*}}
// HBCRAW-NEXT:  Function count: 2
// HBCRAW-NEXT:  String count: 21
// HBCRAW-NEXT:  BigInt count: 0
// HBCRAW-NEXT:  String Kind Entry count: 2
// HBCRAW-NEXT:  RegExp count: 0
// HBCRAW-NEXT:  StringSwitchImm count: 1
// HBCRAW-NEXT:  Key buffer size (bytes): 0
// HBCRAW-NEXT:  Value buffer size (bytes): 0
// HBCRAW-NEXT:  Shape table count: 0
// HBCRAW-NEXT:  Segment ID: 0
// HBCRAW-NEXT:  CommonJS module count: 0
// HBCRAW-NEXT:  CommonJS module count (static): 0
// HBCRAW-NEXT:  Function source count: 0
// HBCRAW-NEXT:  Bytecode options:
// HBCRAW-NEXT:    staticBuiltins: 0
// HBCRAW-NEXT:    cjsModulesStaticallyResolved: 0

// HBCRAW:Global String Table:
// HBCRAW-NEXT:s0[ASCII, 0..1]: 11
// HBCRAW-NEXT:s1[ASCII, 1..1]: 1
// HBCRAW-NEXT:s2[ASCII, 1..2]: 10
// HBCRAW-NEXT:s3[ASCII, 2..2]: 0
// HBCRAW-NEXT:s4[ASCII, 3..4]: 12
// HBCRAW-NEXT:s5[ASCII, 4..4]: 2
// HBCRAW-NEXT:s6[ASCII, 5..6]: 13
// HBCRAW-NEXT:s7[ASCII, 6..6]: 3
// HBCRAW-NEXT:s8[ASCII, 7..8]: 14
// HBCRAW-NEXT:s9[ASCII, 8..8]: 4
// HBCRAW-NEXT:s10[ASCII, 9..10]: 15
// HBCRAW-NEXT:s11[ASCII, 10..10]: 5
// HBCRAW-NEXT:s12[ASCII, 11..12]: 16
// HBCRAW-NEXT:s13[ASCII, 12..12]: 6
// HBCRAW-NEXT:s14[ASCII, 13..13]: 7
// HBCRAW-NEXT:s15[ASCII, 14..14]: 8
// HBCRAW-NEXT:s16[ASCII, 15..15]: 9
// HBCRAW-NEXT:s17[ASCII, 16..21]: global
// HBCRAW-NEXT:s18[ASCII, 33..34]: hi
// HBCRAW-NEXT:i19[ASCII, 22..33] #646CA4B1: stringSwitch
// HBCRAW-NEXT:i20[ASCII, 35..39] #A689F65B: print

// HBCRAW:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// HBCRAW-NEXT:Offset in debug table: source 0x0000
// HBCRAW-NEXT:[@ 0] DeclareGlobalVar 19<UInt32>
// HBCRAW-NEXT:[@ 5] GetGlobalObject 2<Reg8>
// HBCRAW-NEXT:[@ 7] LoadConstUndefined 0<Reg8>
// HBCRAW-NEXT:[@ 9] CreateClosure 1<Reg8>, 0<Reg8>, 1<UInt16>
// HBCRAW-NEXT:[@ 14] PutByIdLoose 2<Reg8>, 1<Reg8>, 0<UInt8>, 19<UInt16>
// HBCRAW-NEXT:[@ 20] Ret 0<Reg8>

// HBCRAW:Function<stringSwitch>(2 params, 15 registers, 1 numbers, 1 non-pointers):
// HBCRAW-NEXT:Offset in debug table: source 0x000b
// HBCRAW-NEXT:[@ 0] LoadParam 4<Reg8>, 1<UInt8>
// HBCRAW-NEXT:[@ 3] GetGlobalObject 3<Reg8>
// HBCRAW-NEXT:[@ 5] LoadConstString 2<Reg8>, 18<UInt16>
// HBCRAW-NEXT:[@ 9] LoadConstUndefined 1<Reg8>
// HBCRAW-NEXT:[@ 11] TryGetById 5<Reg8>, 3<Reg8>, 0<UInt8>, 20<UInt16>
// HBCRAW-NEXT:[@ 17] Call2 5<Reg8>, 5<Reg8>, 1<Reg8>, 2<Reg8>
// HBCRAW-NEXT:[@ 22] StringSwitchImm 4<Reg8>, 0<UInt32>, 140<UInt32>, -11<Addr32>, 17<UInt32>
// HBCRAW-NEXT:[@ 40] LoadConstInt 0<Reg8>, 3352<Imm32>
// HBCRAW-NEXT:[@ 46] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 48] LoadConstInt 0<Reg8>, 3523<Imm32>
// HBCRAW-NEXT:[@ 54] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 56] LoadConstInt 0<Reg8>, 3342<Imm32>
// HBCRAW-NEXT:[@ 62] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 64] LoadConstInt 0<Reg8>, 3254<Imm32>
// HBCRAW-NEXT:[@ 70] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 72] LoadConstInt 0<Reg8>, 3243<Imm32>
// HBCRAW-NEXT:[@ 78] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 80] LoadConstInt 0<Reg8>, 2332<Imm32>
// HBCRAW-NEXT:[@ 86] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 88] LoadConstInt 0<Reg8>, 3211<Imm32>
// HBCRAW-NEXT:[@ 94] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 96] LoadConstInt 0<Reg8>, 3642<Imm32>
// HBCRAW-NEXT:[@ 102] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 104] LoadConstInt 0<Reg8>, 2332<Imm32>
// HBCRAW-NEXT:[@ 110] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 112] LoadConstInt 0<Reg8>, 3234<Imm32>
// HBCRAW-NEXT:[@ 118] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 120] LoadConstInt 0<Reg8>, 323<Imm32>
// HBCRAW-NEXT:[@ 126] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 128] LoadConstInt 0<Reg8>, 362<Imm32>
// HBCRAW-NEXT:[@ 134] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 136] LoadConstUInt8 0<Reg8>, 132<UInt8>
// HBCRAW-NEXT:[@ 139] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 141] LoadConstInt 0<Reg8>, 322<Imm32>
// HBCRAW-NEXT:[@ 147] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 149] LoadConstInt 0<Reg8>, 342<Imm32>
// HBCRAW-NEXT:[@ 155] Ret 0<Reg8>
// HBCRAW-NEXT:[@ 157] LoadConstUInt8 0<Reg8>, 32<UInt8>
// HBCRAW-NEXT:[@ 160] Ret 0<Reg8>

// HBCRAW: String switch Tables:
// HBCRAW-NEXT:  offset 140
// HBCRAW-NEXT:   0 : 135
// HBCRAW-NEXT:   1 : 127
// HBCRAW-NEXT:   2 : 119
// HBCRAW-NEXT:   3 : 114
// HBCRAW-NEXT:   4 : -11
// HBCRAW-NEXT:   5 : 106
// HBCRAW-NEXT:   6 : 98
// HBCRAW-NEXT:   7 : 90
// HBCRAW-NEXT:   8 : 82
// HBCRAW-NEXT:   9 : 74
// HBCRAW-NEXT:   10 : 66
// HBCRAW-NEXT:   11 : 58
// HBCRAW-NEXT:   12 : 50
// HBCRAW-NEXT:   13 : 42
// HBCRAW-NEXT:   14 : 34
// HBCRAW-NEXT:   15 : 26
// HBCRAW-NEXT:   16 : 18

// HBCRAW:Debug filename table:
// HBCRAW-NEXT:  0: {{.*}}switch-of-strings-back-branch.js

// HBCRAW:Debug file table:
// HBCRAW-NEXT:  source table offset 0x0000: filename id 0

// HBCRAW:Debug source table:
// HBCRAW-NEXT:  0x0000  function idx 0, starts at line 14 col 1
// HBCRAW-NEXT:    bc 0: line 14 col 1
// HBCRAW-NEXT:    bc 14: line 14 col 1
// HBCRAW-NEXT:  0x000b  function idx 1, starts at line 14 col 1
// HBCRAW-NEXT:    bc 11: line 17 col 5
// HBCRAW-NEXT:    bc 17: line 17 col 10
// HBCRAW-NEXT:  0x0016  end of debug source table
