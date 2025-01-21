/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode -pretty-disassemble -O %s | %FileCheckOrRegen --match-full-lines %s

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
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    DeclareGlobalVar  "g"
// CHECK-NEXT:    DeclareGlobalVar  "f"
// CHECK-NEXT:    CreateClosure     r2, r1, Function<g>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdStrict     r3, r2, 1, "g"
// CHECK-NEXT:    CreateClosure     r1, r1, Function<f>
// CHECK-NEXT:    PutByIdStrict     r3, r1, 2, "f"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<g>(1 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<f>(2 params, 12 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    SwitchImm         r2, 292, L18, 0, 16
// CHECK-NEXT:L17:
// CHECK-NEXT:    LoadConstInt      r0, 3352
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L16:
// CHECK-NEXT:    LoadConstInt      r0, 3523
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L15:
// CHECK-NEXT:    LoadConstInt      r0, 3342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L14:
// CHECK-NEXT:    LoadConstInt      r0, 3254
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L13:
// CHECK-NEXT:    LoadConstInt      r0, 3243
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L12:
// CHECK-NEXT:    LoadConstInt      r0, 2332
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L11:
// CHECK-NEXT:    LoadConstInt      r0, 3211
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L10:
// CHECK-NEXT:    LoadConstInt      r0, 3642
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L9:
// CHECK-NEXT:    LoadConstInt      r0, 2332
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L8:
// CHECK-NEXT:    LoadConstInt      r0, 3234
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstInt      r0, 323
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstInt      r0, 362
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L5:
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    GetByIdShort      r3, r3, 1, "g"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Call1             r3, r3, r1
// CHECK-NEXT:    LoadConstInt      r0, 342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r0, 132
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadConstInt      r0, 322
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstInt      r0, 342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r0, 32
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L18:
// CHECK-NEXT:    SwitchImm         r2, 199, L23, 1, 14
// CHECK-NEXT:L30:
// CHECK-NEXT:    LoadConstInt      r0, 3342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L29:
// CHECK-NEXT:    LoadConstInt      r0, 3254
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L28:
// CHECK-NEXT:    LoadConstInt      r0, 3243
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L27:
// CHECK-NEXT:    LoadConstInt      r0, 2332
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L26:
// CHECK-NEXT:    LoadConstInt      r0, 3211
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L25:
// CHECK-NEXT:    LoadConstInt      r0, 3642
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L24:
// CHECK-NEXT:    LoadConstInt      r0, 2332
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L22:
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    GetByIdShort      r2, r2, 1, "g"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Call1             r2, r2, r1
// CHECK-NEXT:    LoadConstInt      r0, 342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L21:
// CHECK-NEXT:    LoadConstUInt8    r0, 132
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L20:
// CHECK-NEXT:    LoadConstInt      r0, 322
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L19:
// CHECK-NEXT:    LoadConstInt      r0, 342
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L23:
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    GetByIdShort      r2, r2, 1, "g"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Call1             r2, r2, r1
// CHECK-NEXT:    Ret               r1

// CHECK: Jump Tables:
// CHECK-NEXT:  offset 292
// CHECK-NEXT:   0 : L1
// CHECK-NEXT:   1 : L2
// CHECK-NEXT:   2 : L3
// CHECK-NEXT:   3 : L4
// CHECK-NEXT:   4 : L5
// CHECK-NEXT:   5 : L6
// CHECK-NEXT:   6 : L7
// CHECK-NEXT:   7 : L8
// CHECK-NEXT:   8 : L9
// CHECK-NEXT:   9 : L10
// CHECK-NEXT:   10 : L11
// CHECK-NEXT:   11 : L12
// CHECK-NEXT:   12 : L13
// CHECK-NEXT:   13 : L14
// CHECK-NEXT:   14 : L15
// CHECK-NEXT:   15 : L16
// CHECK-NEXT:   16 : L17
// CHECK-NEXT:  offset 199
// CHECK-NEXT:   1 : L19
// CHECK-NEXT:   2 : L20
// CHECK-NEXT:   3 : L21
// CHECK-NEXT:   4 : L22
// CHECK-NEXT:   5 : L23
// CHECK-NEXT:   6 : L23
// CHECK-NEXT:   7 : L23
// CHECK-NEXT:   8 : L24
// CHECK-NEXT:   9 : L25
// CHECK-NEXT:   10 : L26
// CHECK-NEXT:   11 : L27
// CHECK-NEXT:   12 : L28
// CHECK-NEXT:   13 : L29
// CHECK-NEXT:   14 : L30

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}switch-pretty.js

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
