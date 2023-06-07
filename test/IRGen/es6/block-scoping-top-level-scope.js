/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -bs -g3 -O0 -dump-bytecode %s | %FileCheckOrRegen %s --check-prefix BS
// RUN: %hermesc     -g3 -O0 -dump-bytecode %s | %FileCheckOrRegen %s --check-prefix NOBS

function notStrictNoParamExprs() {
    function testNotStrictNoParamExprs(funcParam) {
        var funcVar;
        let funcLet;
    }
}

function notStrictHasParamExprs() {
    function testNotStrictHasParamExprs(
        funcParam = (
            function funcParamInit(funcParamInitParam) {
                var paramInitVar;
                let paramInitLet;
            }, function funcParamInit2() {
            })) {
        var funcVar;
        let funcLet;
    }
}

function StrictNoParamExprs() {
    "use strict";
    function testStrictNoParamExprs(funcParam) {
        var funcVar;
        let funcLet;
    }
}

function StrictHasParamExprs() {
    "use strict";
    function testStrictHasParamExprs(
        funcParam = (
            function funcParamInit(funcParamInitParam) {
                var paramInitVar;
                let paramInitLet;
            }, function funcParamInit2() {
            })) {
        var funcVar;
        let funcLet;
    }
}

// Auto-generated content below. Please do not modify manually.

// BS:Bytecode File Information:
// BS-NEXT:  Bytecode version number: {{.*}}
// BS-NEXT:  Source hash: {{.*}}
// BS-NEXT:  Function count: 13
// BS-NEXT:  String count: 11
// BS-NEXT:  BigInt count: 0
// BS-NEXT:  String Kind Entry count: 2
// BS-NEXT:  RegExp count: 0
// BS-NEXT:  Segment ID: 0
// BS-NEXT:  CommonJS module count: 0
// BS-NEXT:  CommonJS module count (static): 0
// BS-NEXT:  Function source count: 0
// BS-NEXT:  Bytecode options:
// BS-NEXT:    staticBuiltins: 0
// BS-NEXT:    cjsModulesStaticallyResolved: 0

// BS:Global String Table:
// BS-NEXT:s0[ASCII, 0..12]: funcParamInit
// BS-NEXT:s1[ASCII, 13..26]: funcParamInit2
// BS-NEXT:s2[ASCII, 27..32]: global
// BS-NEXT:s3[ASCII, 33..58]: testNotStrictHasParamExprs
// BS-NEXT:s4[ASCII, 59..83]: testNotStrictNoParamExprs
// BS-NEXT:s5[ASCII, 84..106]: testStrictHasParamExprs
// BS-NEXT:s6[ASCII, 107..128]: testStrictNoParamExprs
// BS-NEXT:i7[ASCII, 129..147] #E81C3D71: StrictHasParamExprs
// BS-NEXT:i8[ASCII, 148..165] #CB99225D: StrictNoParamExprs
// BS-NEXT:i9[ASCII, 166..187] #DC9E65B9: notStrictHasParamExprs
// BS-NEXT:i10[ASCII, 188..208] #689EB6F3: notStrictNoParamExprs

// BS:Function<global>(1 params, 10 registers, 0 symbols):
// BS-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// BS-NEXT:    DeclareGlobalVar  "notStrictNoParamE"...
// BS-NEXT:    DeclareGlobalVar  "notStrictHasParam"...
// BS-NEXT:    DeclareGlobalVar  "StrictNoParamExpr"...
// BS-NEXT:    DeclareGlobalVar  "StrictHasParamExp"...
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    GetGlobalObject   r1
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    CreateClosure     r3, r0, Function<notStrictNoParamExprs>
// BS-NEXT:    PutById           r1, r3, 1, "notStrictNoParamE"...
// BS-NEXT:    CreateClosure     r4, r0, Function<notStrictHasParamExprs>
// BS-NEXT:    PutById           r1, r4, 2, "notStrictHasParam"...
// BS-NEXT:    CreateClosure     r5, r0, Function<StrictNoParamExprs>
// BS-NEXT:    PutById           r1, r5, 3, "StrictNoParamExpr"...
// BS-NEXT:    CreateClosure     r6, r0, Function<StrictHasParamExprs>
// BS-NEXT:    PutById           r1, r6, 4, "StrictHasParamExp"...
// BS-NEXT:    Mov               r7, r2
// BS-NEXT:    Mov               r8, r7
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r8

// BS:Function<notStrictNoParamExprs>(1 params, 5 registers, 1 symbols):
// BS-NEXT:Offset in debug table: source 0x003d, scope 0x0003, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    CreateInnerEnvironment r1, r0, 0
// BS-NEXT:    CreateClosure     r3, r1, Function<testNotStrictNoParamExprs>
// BS-NEXT:    StoreToEnvironment r0, 0, r3
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r2

// BS:Function<testNotStrictNoParamExprs>(2 params, 5 registers, 2 symbols):
// BS-NEXT:Offset in debug table: source 0x0051, scope 0x000a, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r2, 1
// BS-NEXT:    LoadConstUndefined r3
// BS-NEXT:    StoreToEnvironment r0, 0, r2
// BS-NEXT:    StoreNPToEnvironment r0, 1, r3
// BS-NEXT:    CreateInnerEnvironment r1, r0, 1
// BS-NEXT:    StoreNPToEnvironment r1, 0, r3
// BS-NEXT:    StoreNPToEnvironment r1, 0, r3
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r3

// BS:Function<notStrictHasParamExprs>(1 params, 5 registers, 1 symbols):
// BS-NEXT:Offset in debug table: source 0x006f, scope 0x0013, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    CreateInnerEnvironment r1, r0, 0
// BS-NEXT:    CreateClosure     r3, r1, Function<testNotStrictHasParamExprs>
// BS-NEXT:    StoreToEnvironment r0, 0, r3
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r2

// BS:Function<testNotStrictHasParamExprs>(1 params, 12 registers, 2 symbols):
// BS-NEXT:Offset in debug table: source 0x0083, scope 0x001a, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r7, 1
// BS-NEXT:    LoadConstEmpty    r8
// BS-NEXT:    LoadConstUndefined r9
// BS-NEXT:    CreateInnerEnvironment r1, r0, 0
// BS-NEXT:    StoreToEnvironment r0, 0, r8
// BS-NEXT:    StrictNeq         r10, r7, r9
// BS-NEXT:    Mov               r6, r7
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    JmpTrue           L1, r10
// BS-NEXT:    CreateInnerEnvironment r2, r1, 1
// BS-NEXT:    CreateClosure     r7, r2, Function<funcParamInit>
// BS-NEXT:    StoreToEnvironment r2, 0, r7
// BS-NEXT:    CreateInnerEnvironment r3, r1, 1
// BS-NEXT:    CreateClosure     r8, r3, Function<funcParamInit2>
// BS-NEXT:    StoreToEnvironment r3, 0, r8
// BS-NEXT:    Mov               r6, r8
// BS-NEXT:L1:
// BS-NEXT:    StoreToEnvironment r0, 0, r6
// BS-NEXT:    LoadFromEnvironment r7, r0, 0
// BS-NEXT:    ThrowIfEmpty      r8, r7
// BS-NEXT:    StoreToEnvironment r0, 1, r8
// BS-NEXT:    CreateInnerEnvironment r4, r1, 1
// BS-NEXT:    StoreNPToEnvironment r4, 0, r9
// BS-NEXT:    CreateInnerEnvironment r5, r4, 1
// BS-NEXT:    StoreNPToEnvironment r5, 0, r9
// BS-NEXT:    StoreNPToEnvironment r5, 0, r9
// BS-NEXT:    Ret               r9

// BS:Function<funcParamInit>(2 params, 5 registers, 2 symbols):
// BS-NEXT:Offset in debug table: source 0x00d3, scope 0x0034, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r2, 1
// BS-NEXT:    LoadConstUndefined r3
// BS-NEXT:    StoreToEnvironment r0, 0, r2
// BS-NEXT:    StoreNPToEnvironment r0, 1, r3
// BS-NEXT:    CreateInnerEnvironment r1, r0, 1
// BS-NEXT:    StoreNPToEnvironment r1, 0, r3
// BS-NEXT:    StoreNPToEnvironment r1, 0, r3
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r3

// BS:Function<funcParamInit2>(1 params, 4 registers, 0 symbols):
// BS-NEXT:Offset in debug table: source 0x00f1, scope 0x0040, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    CreateInnerEnvironment r1, r0, 0
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r2

// BS:Function<StrictNoParamExprs>(1 params, 4 registers, 1 symbols):
// BS-NEXT:Offset in debug table: source 0x00fb, scope 0x0047, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r1
// BS-NEXT:    CreateClosure     r2, r0, Function<testStrictNoParamExprs>
// BS-NEXT:    StoreToEnvironment r0, 0, r2
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r1

// BS:Function<testStrictNoParamExprs>(2 params, 4 registers, 3 symbols):
// BS-NEXT:Offset in debug table: source 0x0112, scope 0x004c, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r1, 1
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    StoreToEnvironment r0, 0, r1
// BS-NEXT:    StoreNPToEnvironment r0, 1, r2
// BS-NEXT:    StoreNPToEnvironment r0, 2, r2
// BS-NEXT:    StoreNPToEnvironment r0, 2, r2
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r2

// BS:Function<StrictHasParamExprs>(1 params, 4 registers, 1 symbols):
// BS-NEXT:Offset in debug table: source 0x0135, scope 0x0053, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r1
// BS-NEXT:    CreateClosure     r2, r0, Function<testStrictHasParamExprs>
// BS-NEXT:    StoreToEnvironment r0, 0, r2
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r1

// BS:Function<testStrictHasParamExprs>(1 params, 10 registers, 2 symbols):
// BS-NEXT:Offset in debug table: source 0x014c, scope 0x0058, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r5, 1
// BS-NEXT:    LoadConstEmpty    r6
// BS-NEXT:    LoadConstUndefined r7
// BS-NEXT:    StoreToEnvironment r0, 0, r6
// BS-NEXT:    StrictNeq         r8, r5, r7
// BS-NEXT:    Mov               r4, r5
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    JmpTrue           L1, r8
// BS-NEXT:    CreateInnerEnvironment r1, r0, 1
// BS-NEXT:    CreateClosure     r5, r1, Function<funcParamInit>
// BS-NEXT:    StoreToEnvironment r1, 0, r5
// BS-NEXT:    CreateInnerEnvironment r2, r0, 1
// BS-NEXT:    CreateClosure     r6, r2, Function<funcParamInit2>
// BS-NEXT:    StoreToEnvironment r2, 0, r6
// BS-NEXT:    Mov               r4, r6
// BS-NEXT:L1:
// BS-NEXT:    StoreToEnvironment r0, 0, r4
// BS-NEXT:    LoadFromEnvironment r5, r0, 0
// BS-NEXT:    ThrowIfEmpty      r6, r5
// BS-NEXT:    StoreToEnvironment r0, 1, r6
// BS-NEXT:    CreateInnerEnvironment r3, r0, 2
// BS-NEXT:    StoreNPToEnvironment r3, 0, r7
// BS-NEXT:    StoreNPToEnvironment r3, 1, r7
// BS-NEXT:    StoreNPToEnvironment r3, 1, r7
// BS-NEXT:    Ret               r7

// BS:Function<funcParamInit>(2 params, 4 registers, 3 symbols):
// BS-NEXT:Offset in debug table: source 0x01ab, scope 0x0070, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadParam         r1, 1
// BS-NEXT:    LoadConstUndefined r2
// BS-NEXT:    StoreToEnvironment r0, 0, r1
// BS-NEXT:    StoreNPToEnvironment r0, 1, r2
// BS-NEXT:    StoreNPToEnvironment r0, 2, r2
// BS-NEXT:    StoreNPToEnvironment r0, 2, r2
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r2

// BS:Function<funcParamInit2>(1 params, 3 registers, 0 symbols):
// BS-NEXT:Offset in debug table: source 0x01ce, scope 0x007a, textified callees 0x0000
// BS-NEXT:    CreateEnvironment r0
// BS-NEXT:    LoadConstUndefined r1
// BS-NEXT:    AsyncBreakCheck
// BS-NEXT:    Ret               r1

// BS:Debug filename table:
// BS-NEXT:  0: {{.*}}block-scoping-top-level-scope.js

// BS:Debug file table:
// BS-NEXT:  source table offset 0x0000: filename id 0

// BS:Debug source table:
// BS-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// BS-NEXT:    bc 26: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 31: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 37: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 42: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 48: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 53: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 59: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 64: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 70: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 73: line 11 col 1 scope offset 0x0000 env r0
// BS-NEXT:    bc 77: line 51 col 1 scope offset 0x0000 env r0
// BS-NEXT:  0x003d  function idx 1, starts at line 11 col 1
// BS-NEXT:    bc 11: line 11 col 1 scope offset 0x0007 env r1
// BS-NEXT:    bc 16: line 11 col 1 scope offset 0x0007 env r1
// BS-NEXT:    bc 21: line 16 col 1 scope offset 0x0007 env r1
// BS-NEXT:  0x0051  function idx 2, starts at line 12 col 5
// BS-NEXT:    bc 7: line 12 col 5 scope offset 0x000a env r0
// BS-NEXT:    bc 11: line 12 col 5 scope offset 0x000a env r0
// BS-NEXT:    bc 22: line 12 col 5 scope offset 0x000f env r1
// BS-NEXT:    bc 26: line 14 col 13 scope offset 0x000f env r1
// BS-NEXT:    bc 31: line 15 col 5 scope offset 0x000f env r1
// BS-NEXT:  0x006f  function idx 3, starts at line 18 col 1
// BS-NEXT:    bc 11: line 18 col 1 scope offset 0x0017 env r1
// BS-NEXT:    bc 16: line 18 col 1 scope offset 0x0017 env r1
// BS-NEXT:    bc 21: line 29 col 1 scope offset 0x0017 env r1
// BS-NEXT:  0x0083  function idx 4, starts at line 19 col 5
// BS-NEXT:    bc 16: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 20: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 28: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 38: line 21 col 13 scope offset 0x0022 env r2
// BS-NEXT:    bc 43: line 21 col 13 scope offset 0x0022 env r2
// BS-NEXT:    bc 54: line 24 col 16 scope offset 0x0027 env r3
// BS-NEXT:    bc 59: line 24 col 16 scope offset 0x0027 env r3
// BS-NEXT:    bc 66: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 70: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 74: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 77: line 19 col 5 scope offset 0x001f env r1
// BS-NEXT:    bc 88: line 19 col 5 scope offset 0x002c env r4
// BS-NEXT:    bc 99: line 19 col 5 scope offset 0x0030 env r5
// BS-NEXT:    bc 103: line 27 col 13 scope offset 0x0030 env r5
// BS-NEXT:    bc 107: line 28 col 5 scope offset 0x0030 env r5
// BS-NEXT:  0x00d3  function idx 5, starts at line 21 col 13
// BS-NEXT:    bc 7: line 21 col 13 scope offset 0x0034 env r0
// BS-NEXT:    bc 11: line 21 col 13 scope offset 0x0034 env r0
// BS-NEXT:    bc 22: line 21 col 13 scope offset 0x003b env r1
// BS-NEXT:    bc 26: line 23 col 21 scope offset 0x003b env r1
// BS-NEXT:    bc 31: line 24 col 13 scope offset 0x003b env r1
// BS-NEXT:  0x00f1  function idx 6, starts at line 24 col 16
// BS-NEXT:    bc 12: line 25 col 13 scope offset 0x0043 env r1
// BS-NEXT:  0x00fb  function idx 7, starts at line 31 col 1
// BS-NEXT:    bc 4: line 31 col 1 scope offset 0x0047 env r0
// BS-NEXT:    bc 9: line 31 col 1 scope offset 0x0047 env r0
// BS-NEXT:    bc 14: line 37 col 1 scope offset 0x0047 env r0
// BS-NEXT:  0x0112  function idx 8, starts at line 33 col 5
// BS-NEXT:    bc 7: line 33 col 5 scope offset 0x004c env r0
// BS-NEXT:    bc 11: line 33 col 5 scope offset 0x004c env r0
// BS-NEXT:    bc 15: line 33 col 5 scope offset 0x004c env r0
// BS-NEXT:    bc 19: line 35 col 13 scope offset 0x004c env r0
// BS-NEXT:    bc 24: line 36 col 5 scope offset 0x004c env r0
// BS-NEXT:  0x0135  function idx 9, starts at line 39 col 1
// BS-NEXT:    bc 4: line 39 col 1 scope offset 0x0053 env r0
// BS-NEXT:    bc 9: line 39 col 1 scope offset 0x0053 env r0
// BS-NEXT:    bc 14: line 51 col 1 scope offset 0x0053 env r0
// BS-NEXT:  0x014c  function idx 10, starts at line 41 col 5
// BS-NEXT:    bc 9: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 13: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 21: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 31: line 43 col 13 scope offset 0x005e env r1
// BS-NEXT:    bc 36: line 43 col 13 scope offset 0x005e env r1
// BS-NEXT:    bc 47: line 46 col 16 scope offset 0x0064 env r2
// BS-NEXT:    bc 52: line 46 col 16 scope offset 0x0064 env r2
// BS-NEXT:    bc 59: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 63: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 67: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 70: line 41 col 5 scope offset 0x0058 env r0
// BS-NEXT:    bc 81: line 41 col 5 scope offset 0x006a env r3
// BS-NEXT:    bc 85: line 41 col 5 scope offset 0x006a env r3
// BS-NEXT:    bc 89: line 49 col 13 scope offset 0x006a env r3
// BS-NEXT:    bc 93: line 50 col 5 scope offset 0x006a env r3
// BS-NEXT:  0x01ab  function idx 11, starts at line 43 col 13
// BS-NEXT:    bc 7: line 43 col 13 scope offset 0x0070 env r0
// BS-NEXT:    bc 11: line 43 col 13 scope offset 0x0070 env r0
// BS-NEXT:    bc 15: line 43 col 13 scope offset 0x0070 env r0
// BS-NEXT:    bc 19: line 45 col 21 scope offset 0x0070 env r0
// BS-NEXT:    bc 24: line 46 col 13 scope offset 0x0070 env r0
// BS-NEXT:  0x01ce  function idx 12, starts at line 46 col 16
// BS-NEXT:    bc 5: line 47 col 13 scope offset 0x007a env r0
// BS-NEXT:  0x01d8  end of debug source table

// BS:Debug scope descriptor table:
// BS-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// BS-NEXT:  0x0003  lexical parent: 0x0000, flags:    , variable count: 1
// BS-NEXT:    "testNotStrictNoParamExprs"
// BS-NEXT:  0x0007  lexical parent: 0x0003, flags: Is , variable count: 0
// BS-NEXT:  0x000a  lexical parent: 0x0007, flags:    , variable count: 2
// BS-NEXT:    "funcParam"
// BS-NEXT:    "funcVar"
// BS-NEXT:  0x000f  lexical parent: 0x000a, flags: Is , variable count: 1
// BS-NEXT:    "funcLet"
// BS-NEXT:  0x0013  lexical parent: 0x0000, flags:    , variable count: 1
// BS-NEXT:    "testNotStrictHasParamExprs"
// BS-NEXT:  0x0017  lexical parent: 0x0013, flags: Is , variable count: 0
// BS-NEXT:  0x001a  lexical parent: 0x0017, flags:    , variable count: 2
// BS-NEXT:    "funcParam"
// BS-NEXT:    "funcParam"
// BS-NEXT:  0x001f  lexical parent: 0x001a, flags: Is , variable count: 0
// BS-NEXT:  0x0022  lexical parent: 0x001f, flags: Is , variable count: 1
// BS-NEXT:    "?anon_0_funcParamInit"
// BS-NEXT:  0x0027  lexical parent: 0x001f, flags: Is , variable count: 1
// BS-NEXT:    "?anon_1_funcParamInit2"
// BS-NEXT:  0x002c  lexical parent: 0x001f, flags: Is , variable count: 1
// BS-NEXT:    "funcVar"
// BS-NEXT:  0x0030  lexical parent: 0x002c, flags: Is , variable count: 1
// BS-NEXT:    "funcLet"
// BS-NEXT:  0x0034  lexical parent: 0x0022, flags:    , variable count: 2
// BS-NEXT:    "funcParamInitParam"
// BS-NEXT:    "paramInitVar"
// BS-NEXT:  0x003b  lexical parent: 0x0034, flags: Is , variable count: 1
// BS-NEXT:    "paramInitLet"
// BS-NEXT:  0x0040  lexical parent: 0x0027, flags:    , variable count: 0
// BS-NEXT:  0x0043  lexical parent: 0x0040, flags: Is , variable count: 0
// BS-NEXT:  0x0047  lexical parent: 0x0000, flags:    , variable count: 1
// BS-NEXT:    "testStrictNoParamExprs"
// BS-NEXT:  0x004c  lexical parent: 0x0047, flags:    , variable count: 3
// BS-NEXT:    "funcParam"
// BS-NEXT:    "funcVar"
// BS-NEXT:    "funcLet"
// BS-NEXT:  0x0053  lexical parent: 0x0000, flags:    , variable count: 1
// BS-NEXT:    "testStrictHasParamExprs"
// BS-NEXT:  0x0058  lexical parent: 0x0053, flags:    , variable count: 2
// BS-NEXT:    "funcParam"
// BS-NEXT:    "funcParam"
// BS-NEXT:  0x005e  lexical parent: 0x0058, flags: Is , variable count: 1
// BS-NEXT:    "?anon_0_funcParamInit"
// BS-NEXT:  0x0064  lexical parent: 0x0058, flags: Is , variable count: 1
// BS-NEXT:    "?anon_1_funcParamInit2"
// BS-NEXT:  0x006a  lexical parent: 0x0058, flags: Is , variable count: 2
// BS-NEXT:    "funcVar"
// BS-NEXT:    "funcLet"
// BS-NEXT:  0x0070  lexical parent: 0x005e, flags:    , variable count: 3
// BS-NEXT:    "funcParamInitParam"
// BS-NEXT:    "paramInitVar"
// BS-NEXT:    "paramInitLet"
// BS-NEXT:  0x007a  lexical parent: 0x0064, flags:    , variable count: 0
// BS-NEXT:  0x007e  end of debug scope descriptor table

// BS:Textified callees table:
// BS-NEXT:  0x0000  entries: 0
// BS-NEXT:  0x0001  end of textified callees table

// BS:Debug string table:
// BS-NEXT:  0x0000 testNotStrictNoParamExprs
// BS-NEXT:  0x001a funcParam
// BS-NEXT:  0x0024 funcVar
// BS-NEXT:  0x002c funcLet
// BS-NEXT:  0x0034 testNotStrictHasParamExprs
// BS-NEXT:  0x004f ?anon_0_funcParamInit
// BS-NEXT:  0x0065 ?anon_1_funcParamInit2
// BS-NEXT:  0x007c funcParamInitParam
// BS-NEXT:  0x008f paramInitVar
// BS-NEXT:  0x009c paramInitLet
// BS-NEXT:  0x00a9 testStrictNoParamExprs
// BS-NEXT:  0x00c0 testStrictHasParamExprs
// BS-NEXT:  0x00d8  end of debug string table

// NOBS:Bytecode File Information:
// NOBS-NEXT:  Bytecode version number: {{.*}}
// NOBS-NEXT:  Source hash: {{.*}}
// NOBS-NEXT:  Function count: 13
// NOBS-NEXT:  String count: 11
// NOBS-NEXT:  BigInt count: 0
// NOBS-NEXT:  String Kind Entry count: 2
// NOBS-NEXT:  RegExp count: 0
// NOBS-NEXT:  Segment ID: 0
// NOBS-NEXT:  CommonJS module count: 0
// NOBS-NEXT:  CommonJS module count (static): 0
// NOBS-NEXT:  Function source count: 0
// NOBS-NEXT:  Bytecode options:
// NOBS-NEXT:    staticBuiltins: 0
// NOBS-NEXT:    cjsModulesStaticallyResolved: 0

// NOBS:Global String Table:
// NOBS-NEXT:s0[ASCII, 0..12]: funcParamInit
// NOBS-NEXT:s1[ASCII, 13..26]: funcParamInit2
// NOBS-NEXT:s2[ASCII, 27..32]: global
// NOBS-NEXT:s3[ASCII, 33..58]: testNotStrictHasParamExprs
// NOBS-NEXT:s4[ASCII, 59..83]: testNotStrictNoParamExprs
// NOBS-NEXT:s5[ASCII, 84..106]: testStrictHasParamExprs
// NOBS-NEXT:s6[ASCII, 107..128]: testStrictNoParamExprs
// NOBS-NEXT:i7[ASCII, 129..147] #E81C3D71: StrictHasParamExprs
// NOBS-NEXT:i8[ASCII, 148..165] #CB99225D: StrictNoParamExprs
// NOBS-NEXT:i9[ASCII, 166..187] #DC9E65B9: notStrictHasParamExprs
// NOBS-NEXT:i10[ASCII, 188..208] #689EB6F3: notStrictNoParamExprs

// NOBS:Function<global>(1 params, 10 registers, 0 symbols):
// NOBS-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// NOBS-NEXT:    DeclareGlobalVar  "notStrictNoParamE"...
// NOBS-NEXT:    DeclareGlobalVar  "notStrictHasParam"...
// NOBS-NEXT:    DeclareGlobalVar  "StrictNoParamExpr"...
// NOBS-NEXT:    DeclareGlobalVar  "StrictHasParamExp"...
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    GetGlobalObject   r1
// NOBS-NEXT:    LoadConstUndefined r2
// NOBS-NEXT:    CreateClosure     r3, r0, Function<notStrictNoParamExprs>
// NOBS-NEXT:    PutById           r1, r3, 1, "notStrictNoParamE"...
// NOBS-NEXT:    CreateClosure     r4, r0, Function<notStrictHasParamExprs>
// NOBS-NEXT:    PutById           r1, r4, 2, "notStrictHasParam"...
// NOBS-NEXT:    CreateClosure     r5, r0, Function<StrictNoParamExprs>
// NOBS-NEXT:    PutById           r1, r5, 3, "StrictNoParamExpr"...
// NOBS-NEXT:    CreateClosure     r6, r0, Function<StrictHasParamExprs>
// NOBS-NEXT:    PutById           r1, r6, 4, "StrictHasParamExp"...
// NOBS-NEXT:    Mov               r7, r2
// NOBS-NEXT:    Mov               r8, r7
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r8

// NOBS:Function<notStrictNoParamExprs>(1 params, 4 registers, 1 symbols):
// NOBS-NEXT:Offset in debug table: source 0x003d, scope 0x0003, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    CreateClosure     r2, r0, Function<testNotStrictNoParamExprs>
// NOBS-NEXT:    StoreToEnvironment r0, 0, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Function<testNotStrictNoParamExprs>(2 params, 4 registers, 3 symbols):
// NOBS-NEXT:Offset in debug table: source 0x0051, scope 0x0007, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r1, 1
// NOBS-NEXT:    LoadConstUndefined r2
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 1, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r2

// NOBS:Function<notStrictHasParamExprs>(1 params, 4 registers, 1 symbols):
// NOBS-NEXT:Offset in debug table: source 0x006f, scope 0x000d, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    CreateClosure     r2, r0, Function<testNotStrictHasParamExprs>
// NOBS-NEXT:    StoreToEnvironment r0, 0, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Function<testNotStrictHasParamExprs>(1 params, 6 registers, 5 symbols):
// NOBS-NEXT:Offset in debug table: source 0x0083, scope 0x0011, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r2, 1
// NOBS-NEXT:    LoadConstUndefined r3
// NOBS-NEXT:    StrictNeq         r4, r2, r3
// NOBS-NEXT:    Mov               r1, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    JmpTrue           L1, r4
// NOBS-NEXT:    CreateClosure     r2, r0, Function<funcParamInit>
// NOBS-NEXT:    StoreToEnvironment r0, 1, r2
// NOBS-NEXT:    CreateClosure     r4, r0, Function<funcParamInit2>
// NOBS-NEXT:    StoreToEnvironment r0, 2, r4
// NOBS-NEXT:    Mov               r1, r4
// NOBS-NEXT:L1:
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 3, r3
// NOBS-NEXT:    StoreNPToEnvironment r0, 4, r3
// NOBS-NEXT:    StoreNPToEnvironment r0, 4, r3
// NOBS-NEXT:    Ret               r3

// NOBS:Function<funcParamInit>(2 params, 4 registers, 3 symbols):
// NOBS-NEXT:Offset in debug table: source 0x00bf, scope 0x001b, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r1, 1
// NOBS-NEXT:    LoadConstUndefined r2
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 1, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r2

// NOBS:Function<funcParamInit2>(1 params, 3 registers, 0 symbols):
// NOBS-NEXT:Offset in debug table: source 0x00dd, scope 0x0024, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Function<StrictNoParamExprs>(1 params, 4 registers, 1 symbols):
// NOBS-NEXT:Offset in debug table: source 0x00e6, scope 0x0027, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    CreateClosure     r2, r0, Function<testStrictNoParamExprs>
// NOBS-NEXT:    StoreToEnvironment r0, 0, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Function<testStrictNoParamExprs>(2 params, 4 registers, 3 symbols):
// NOBS-NEXT:Offset in debug table: source 0x00fa, scope 0x002c, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r1, 1
// NOBS-NEXT:    LoadConstUndefined r2
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 1, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r2

// NOBS:Function<StrictHasParamExprs>(1 params, 4 registers, 1 symbols):
// NOBS-NEXT:Offset in debug table: source 0x0118, scope 0x0032, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    CreateClosure     r2, r0, Function<testStrictHasParamExprs>
// NOBS-NEXT:    StoreToEnvironment r0, 0, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Function<testStrictHasParamExprs>(1 params, 6 registers, 5 symbols):
// NOBS-NEXT:Offset in debug table: source 0x012c, scope 0x0037, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r2, 1
// NOBS-NEXT:    LoadConstUndefined r3
// NOBS-NEXT:    StrictNeq         r4, r2, r3
// NOBS-NEXT:    Mov               r1, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    JmpTrue           L1, r4
// NOBS-NEXT:    CreateClosure     r2, r0, Function<funcParamInit>
// NOBS-NEXT:    StoreToEnvironment r0, 1, r2
// NOBS-NEXT:    CreateClosure     r4, r0, Function<funcParamInit2>
// NOBS-NEXT:    StoreToEnvironment r0, 2, r4
// NOBS-NEXT:    Mov               r1, r4
// NOBS-NEXT:L1:
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 3, r3
// NOBS-NEXT:    StoreNPToEnvironment r0, 4, r3
// NOBS-NEXT:    StoreNPToEnvironment r0, 4, r3
// NOBS-NEXT:    Ret               r3

// NOBS:Function<funcParamInit>(2 params, 4 registers, 3 symbols):
// NOBS-NEXT:Offset in debug table: source 0x0168, scope 0x0041, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadParam         r1, 1
// NOBS-NEXT:    LoadConstUndefined r2
// NOBS-NEXT:    StoreToEnvironment r0, 0, r1
// NOBS-NEXT:    StoreNPToEnvironment r0, 1, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    StoreNPToEnvironment r0, 2, r2
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r2

// NOBS:Function<funcParamInit2>(1 params, 3 registers, 0 symbols):
// NOBS-NEXT:Offset in debug table: source 0x018b, scope 0x004a, textified callees 0x0000
// NOBS-NEXT:    CreateEnvironment r0
// NOBS-NEXT:    LoadConstUndefined r1
// NOBS-NEXT:    AsyncBreakCheck
// NOBS-NEXT:    Ret               r1

// NOBS:Debug filename table:
// NOBS-NEXT:  0: {{.*}}block-scoping-top-level-scope.js

// NOBS:Debug file table:
// NOBS-NEXT:  source table offset 0x0000: filename id 0

// NOBS:Debug source table:
// NOBS-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// NOBS-NEXT:    bc 26: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 31: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 37: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 42: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 48: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 53: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 59: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 64: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 70: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 73: line 11 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:    bc 77: line 51 col 1 scope offset 0x0000 env r0
// NOBS-NEXT:  0x003d  function idx 1, starts at line 11 col 1
// NOBS-NEXT:    bc 4: line 11 col 1 scope offset 0x0003 env r0
// NOBS-NEXT:    bc 9: line 11 col 1 scope offset 0x0003 env r0
// NOBS-NEXT:    bc 14: line 16 col 1 scope offset 0x0003 env r0
// NOBS-NEXT:  0x0051  function idx 2, starts at line 12 col 5
// NOBS-NEXT:    bc 7: line 12 col 5 scope offset 0x0007 env r0
// NOBS-NEXT:    bc 11: line 12 col 5 scope offset 0x0007 env r0
// NOBS-NEXT:    bc 15: line 12 col 5 scope offset 0x0007 env r0
// NOBS-NEXT:    bc 19: line 14 col 13 scope offset 0x0007 env r0
// NOBS-NEXT:    bc 24: line 15 col 5 scope offset 0x0007 env r0
// NOBS-NEXT:  0x006f  function idx 3, starts at line 18 col 1
// NOBS-NEXT:    bc 4: line 18 col 1 scope offset 0x000d env r0
// NOBS-NEXT:    bc 9: line 18 col 1 scope offset 0x000d env r0
// NOBS-NEXT:    bc 14: line 29 col 1 scope offset 0x000d env r0
// NOBS-NEXT:  0x0083  function idx 4, starts at line 19 col 5
// NOBS-NEXT:    bc 7: line 19 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 15: line 19 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 18: line 21 col 13 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 23: line 21 col 13 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 27: line 24 col 16 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 32: line 24 col 16 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 39: line 19 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 43: line 19 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 47: line 19 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 51: line 27 col 13 scope offset 0x0011 env r0
// NOBS-NEXT:    bc 55: line 28 col 5 scope offset 0x0011 env r0
// NOBS-NEXT:  0x00bf  function idx 5, starts at line 21 col 13
// NOBS-NEXT:    bc 7: line 21 col 13 scope offset 0x001b env r0
// NOBS-NEXT:    bc 11: line 21 col 13 scope offset 0x001b env r0
// NOBS-NEXT:    bc 15: line 21 col 13 scope offset 0x001b env r0
// NOBS-NEXT:    bc 19: line 23 col 21 scope offset 0x001b env r0
// NOBS-NEXT:    bc 24: line 24 col 13 scope offset 0x001b env r0
// NOBS-NEXT:  0x00dd  function idx 6, starts at line 24 col 16
// NOBS-NEXT:    bc 5: line 25 col 13 scope offset 0x0024 env r0
// NOBS-NEXT:  0x00e6  function idx 7, starts at line 31 col 1
// NOBS-NEXT:    bc 4: line 31 col 1 scope offset 0x0027 env r0
// NOBS-NEXT:    bc 9: line 31 col 1 scope offset 0x0027 env r0
// NOBS-NEXT:    bc 14: line 37 col 1 scope offset 0x0027 env r0
// NOBS-NEXT:  0x00fa  function idx 8, starts at line 33 col 5
// NOBS-NEXT:    bc 7: line 33 col 5 scope offset 0x002c env r0
// NOBS-NEXT:    bc 11: line 33 col 5 scope offset 0x002c env r0
// NOBS-NEXT:    bc 15: line 33 col 5 scope offset 0x002c env r0
// NOBS-NEXT:    bc 19: line 35 col 13 scope offset 0x002c env r0
// NOBS-NEXT:    bc 24: line 36 col 5 scope offset 0x002c env r0
// NOBS-NEXT:  0x0118  function idx 9, starts at line 39 col 1
// NOBS-NEXT:    bc 4: line 39 col 1 scope offset 0x0032 env r0
// NOBS-NEXT:    bc 9: line 39 col 1 scope offset 0x0032 env r0
// NOBS-NEXT:    bc 14: line 51 col 1 scope offset 0x0032 env r0
// NOBS-NEXT:  0x012c  function idx 10, starts at line 41 col 5
// NOBS-NEXT:    bc 7: line 41 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 15: line 41 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 18: line 43 col 13 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 23: line 43 col 13 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 27: line 46 col 16 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 32: line 46 col 16 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 39: line 41 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 43: line 41 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 47: line 41 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 51: line 49 col 13 scope offset 0x0037 env r0
// NOBS-NEXT:    bc 55: line 50 col 5 scope offset 0x0037 env r0
// NOBS-NEXT:  0x0168  function idx 11, starts at line 43 col 13
// NOBS-NEXT:    bc 7: line 43 col 13 scope offset 0x0041 env r0
// NOBS-NEXT:    bc 11: line 43 col 13 scope offset 0x0041 env r0
// NOBS-NEXT:    bc 15: line 43 col 13 scope offset 0x0041 env r0
// NOBS-NEXT:    bc 19: line 45 col 21 scope offset 0x0041 env r0
// NOBS-NEXT:    bc 24: line 46 col 13 scope offset 0x0041 env r0
// NOBS-NEXT:  0x018b  function idx 12, starts at line 46 col 16
// NOBS-NEXT:    bc 5: line 47 col 13 scope offset 0x004a env r0
// NOBS-NEXT:  0x0195  end of debug source table

// NOBS:Debug scope descriptor table:
// NOBS-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// NOBS-NEXT:  0x0003  lexical parent: 0x0000, flags:    , variable count: 1
// NOBS-NEXT:    "testNotStrictNoParamExprs"
// NOBS-NEXT:  0x0007  lexical parent: 0x0003, flags:    , variable count: 3
// NOBS-NEXT:    "funcParam"
// NOBS-NEXT:    "funcVar"
// NOBS-NEXT:    "funcLet"
// NOBS-NEXT:  0x000d  lexical parent: 0x0000, flags:    , variable count: 1
// NOBS-NEXT:    "testNotStrictHasParamExprs"
// NOBS-NEXT:  0x0011  lexical parent: 0x000d, flags:    , variable count: 5
// NOBS-NEXT:    "funcParam"
// NOBS-NEXT:    "?anon_0_closure"
// NOBS-NEXT:    "?anon_1_closure"
// NOBS-NEXT:    "funcVar"
// NOBS-NEXT:    "funcLet"
// NOBS-NEXT:  0x001b  lexical parent: 0x0011, flags:    , variable count: 3
// NOBS-NEXT:    "funcParamInitParam"
// NOBS-NEXT:    "paramInitVar"
// NOBS-NEXT:    "paramInitLet"
// NOBS-NEXT:  0x0024  lexical parent: 0x0011, flags:    , variable count: 0
// NOBS-NEXT:  0x0027  lexical parent: 0x0000, flags:    , variable count: 1
// NOBS-NEXT:    "testStrictNoParamExprs"
// NOBS-NEXT:  0x002c  lexical parent: 0x0027, flags:    , variable count: 3
// NOBS-NEXT:    "funcParam"
// NOBS-NEXT:    "funcVar"
// NOBS-NEXT:    "funcLet"
// NOBS-NEXT:  0x0032  lexical parent: 0x0000, flags:    , variable count: 1
// NOBS-NEXT:    "testStrictHasParamExprs"
// NOBS-NEXT:  0x0037  lexical parent: 0x0032, flags:    , variable count: 5
// NOBS-NEXT:    "funcParam"
// NOBS-NEXT:    "?anon_0_closure"
// NOBS-NEXT:    "?anon_1_closure"
// NOBS-NEXT:    "funcVar"
// NOBS-NEXT:    "funcLet"
// NOBS-NEXT:  0x0041  lexical parent: 0x0037, flags:    , variable count: 3
// NOBS-NEXT:    "funcParamInitParam"
// NOBS-NEXT:    "paramInitVar"
// NOBS-NEXT:    "paramInitLet"
// NOBS-NEXT:  0x004a  lexical parent: 0x0037, flags:    , variable count: 0
// NOBS-NEXT:  0x004d  end of debug scope descriptor table

// NOBS:Textified callees table:
// NOBS-NEXT:  0x0000  entries: 0
// NOBS-NEXT:  0x0001  end of textified callees table

// NOBS:Debug string table:
// NOBS-NEXT:  0x0000 testNotStrictNoParamExprs
// NOBS-NEXT:  0x001a funcParam
// NOBS-NEXT:  0x0024 funcVar
// NOBS-NEXT:  0x002c funcLet
// NOBS-NEXT:  0x0034 testNotStrictHasParamExprs
// NOBS-NEXT:  0x004f ?anon_0_closure
// NOBS-NEXT:  0x005f ?anon_1_closure
// NOBS-NEXT:  0x006f funcParamInitParam
// NOBS-NEXT:  0x0082 paramInitVar
// NOBS-NEXT:  0x008f paramInitLet
// NOBS-NEXT:  0x009c testStrictNoParamExprs
// NOBS-NEXT:  0x00b3 testStrictHasParamExprs
// NOBS-NEXT:  0x00cb  end of debug string table
