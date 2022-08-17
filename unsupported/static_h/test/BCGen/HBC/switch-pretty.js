/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode -pretty-disassemble -O %s | %FileCheck --match-full-lines %s

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

//CHECK-LABEL:Function<f>(2 params, 10 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    LoadParam         r0, 1
//CHECK-NEXT:    SwitchImm         r0, 292, L18, 0, 16
//CHECK-NEXT:L17:
//CHECK-NEXT:    LoadConstInt      r1, 3352
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L16:
//CHECK-NEXT:    LoadConstInt      r1, 3523
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L15:
//CHECK-NEXT:    LoadConstInt      r1, 3342
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L14:
//CHECK-NEXT:    LoadConstInt      r1, 3254
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L13:
//CHECK-NEXT:    LoadConstInt      r1, 3243
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L12:
//CHECK-NEXT:    LoadConstInt      r1, 2332
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L11:
//CHECK-NEXT:    LoadConstInt      r1, 3211
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L10:
//CHECK-NEXT:    LoadConstInt      r1, 3642
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L9:
//CHECK-NEXT:    LoadConstInt      r1, 2332
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L8:
//CHECK-NEXT:    LoadConstInt      r1, 3234
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L7:
//CHECK-NEXT:    LoadConstInt      r1, 323
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L6:
//CHECK-NEXT:    LoadConstInt      r1, 362
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L5:
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    GetByIdShort      r2, r1, 1, "g"
//CHECK-NEXT:    LoadConstUndefined r1
//CHECK-NEXT:    Call1             r1, r2, r1
//CHECK-NEXT:    LoadConstInt      r1, 342
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L4:
//CHECK-NEXT:    LoadConstUInt8    r1, 132
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L3:
//CHECK-NEXT:    LoadConstInt      r1, 322
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L2:
//CHECK-NEXT:    LoadConstInt      r1, 342
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L1:
//CHECK-NEXT:    LoadConstUInt8    r1, 32
//CHECK-NEXT:    Ret               r1
//CHECK-NEXT:L18:
//CHECK-NEXT:    SwitchImm         r0, 199, L23, 1, 14
//CHECK-NEXT:L30:
//CHECK-NEXT:    LoadConstInt      r0, 3342
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L29:
//CHECK-NEXT:    LoadConstInt      r0, 3254
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L28:
//CHECK-NEXT:    LoadConstInt      r0, 3243
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L27:
//CHECK-NEXT:    LoadConstInt      r0, 2332
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L26:
//CHECK-NEXT:    LoadConstInt      r0, 3211
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L25:
//CHECK-NEXT:    LoadConstInt      r0, 3642
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L24:
//CHECK-NEXT:    LoadConstInt      r0, 2332
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L22:
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    GetByIdShort      r1, r0, 1, "g"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Call1             r0, r1, r0
//CHECK-NEXT:    LoadConstInt      r0, 342
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L21:
//CHECK-NEXT:    LoadConstUInt8    r0, 132
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L20:
//CHECK-NEXT:    LoadConstInt      r0, 322
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L19:
//CHECK-NEXT:    LoadConstInt      r0, 342
//CHECK-NEXT:    Ret               r0
//CHECK-NEXT:L23:
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    GetByIdShort      r1, r0, 1, "g"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Call1             r1, r1, r0
//CHECK-NEXT:    Ret               r0

//CHECK-LABEL: Jump Tables:
//CHECK-NEXT:  offset 292
//CHECK-NEXT:   0 : L1
//CHECK-NEXT:   1 : L2
//CHECK-NEXT:   2 : L3
//CHECK-NEXT:   3 : L4
//CHECK-NEXT:   4 : L5
//CHECK-NEXT:   5 : L6
//CHECK-NEXT:   6 : L7
//CHECK-NEXT:   7 : L8
//CHECK-NEXT:   8 : L9
//CHECK-NEXT:   9 : L10
//CHECK-NEXT:   10 : L11
//CHECK-NEXT:   11 : L12
//CHECK-NEXT:   12 : L13
//CHECK-NEXT:   13 : L14
//CHECK-NEXT:   14 : L15
//CHECK-NEXT:   15 : L16
//CHECK-NEXT:   16 : L17
//CHECK-NEXT:  offset 199
//CHECK-NEXT:   1 : L19
//CHECK-NEXT:   2 : L20
//CHECK-NEXT:   3 : L21
//CHECK-NEXT:   4 : L22
//CHECK-NEXT:   5 : L23
//CHECK-NEXT:   6 : L23
//CHECK-NEXT:   7 : L23
//CHECK-NEXT:   8 : L24
//CHECK-NEXT:   9 : L25
//CHECK-NEXT:   10 : L26
//CHECK-NEXT:   11 : L27
//CHECK-NEXT:   12 : L28
//CHECK-NEXT:   13 : L29
//CHECK-NEXT:   14 : L30
