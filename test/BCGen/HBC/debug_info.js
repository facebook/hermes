/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-bytecode -target=HBC -g %s | %FileCheckOrRegen %s --match-full-lines
// RUN: cp %s %T/debug_info_à.js && %hermes -O0 -dump-bytecode -target=HBC -g %T/debug_info_à.js | %FileCheckOrRegen %s --match-full-lines --check-prefix=UNICODE

var v1g = "global";

function Fa() {
    var v1a = 3;
    var v2a = 5;
}

function Fb() {
    var v1b = "abc";
}

function Fc() {
    var v1c = undefined;
    function Fcc() {
        var v1cc = 42;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 5
// CHECK-NEXT:  String count: 7
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
// CHECK-NEXT:s0[ASCII, 0..2]: Fcc
// CHECK-NEXT:s1[ASCII, 3..5]: abc
// CHECK-NEXT:s2[ASCII, 6..11]: global
// CHECK-NEXT:i3[ASCII, 12..13] #0462F07A: Fa
// CHECK-NEXT:i4[ASCII, 14..15] #04628D8B: Fb
// CHECK-NEXT:i5[ASCII, 16..17] #04628998: Fc
// CHECK-NEXT:i6[ASCII, 18..20] #913AA8C8: v1g

// CHECK:Function<global>(1 params, 13 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    DeclareGlobalVar  "v1g"
// CHECK-NEXT:    DeclareGlobalVar  "Fa"
// CHECK-NEXT:    DeclareGlobalVar  "Fb"
// CHECK-NEXT:    DeclareGlobalVar  "Fc"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<Fa>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "Fa"
// CHECK-NEXT:    CreateClosure     r3, r0, Function<Fb>
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutByIdLoose      r4, r3, 2, "Fb"
// CHECK-NEXT:    CreateClosure     r5, r0, Function<Fc>
// CHECK-NEXT:    GetGlobalObject   r6
// CHECK-NEXT:    PutByIdLoose      r6, r5, 3, "Fc"
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    Mov               r7, r8
// CHECK-NEXT:    LoadConstString   r9, "global"
// CHECK-NEXT:    GetGlobalObject   r10
// CHECK-NEXT:    PutByIdLoose      r10, r9, 4, "v1g"
// CHECK-NEXT:    Mov               r11, r7
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r11

// CHECK:Function<Fa>(1 params, 7 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0030, lexical 0x0002
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    StoreNPToEnvironment r0, 1, r2
// CHECK-NEXT:    LoadConstUInt8    r3, 3
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r3
// CHECK-NEXT:    LoadConstUInt8    r4, 5
// CHECK-NEXT:    StoreNPToEnvironment r0, 1, r4
// CHECK-NEXT:    LoadConstUndefined r5
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r5

// CHECK:Function<Fb>(1 params, 5 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0045, lexical 0x000c
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstString   r2, "abc"
// CHECK-NEXT:    StoreToEnvironment r0, 0, r2
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r3

// CHECK:Function<Fc>(1 params, 7 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0053, lexical 0x0012
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    StoreNPToEnvironment r0, 1, r2
// CHECK-NEXT:    CreateClosure     r3, r0, Function<Fcc>
// CHECK-NEXT:    StoreToEnvironment r0, 1, r3
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r4
// CHECK-NEXT:    LoadConstUndefined r5
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r5

// CHECK:Function<Fcc>(1 params, 5 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x006b, lexical 0x001c
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstUInt8    r2, 42
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r2
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    AsyncBreakCheck
// CHECK-NEXT:    Ret               r3

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}debug_info.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHECK-NEXT:    bc 2: line 11 col 1
// CHECK-NEXT:    bc 7: line 11 col 1
// CHECK-NEXT:    bc 12: line 11 col 1
// CHECK-NEXT:    bc 17: line 11 col 1
// CHECK-NEXT:    bc 22: line 11 col 1
// CHECK-NEXT:    bc 29: line 11 col 1
// CHECK-NEXT:    bc 35: line 11 col 1
// CHECK-NEXT:    bc 42: line 11 col 1
// CHECK-NEXT:    bc 48: line 11 col 1
// CHECK-NEXT:    bc 55: line 11 col 1
// CHECK-NEXT:    bc 61: line 11 col 1
// CHECK-NEXT:    bc 72: line 11 col 9
// CHECK-NEXT:    bc 78: line 11 col 1
// CHECK-NEXT:    bc 82: line 27 col 1
// CHECK-NEXT:  0x0030  function idx 1, starts at line 13 col 1
// CHECK-NEXT:    bc 4: line 13 col 1
// CHECK-NEXT:    bc 10: line 13 col 1
// CHECK-NEXT:    bc 17: line 14 col 13
// CHECK-NEXT:    bc 24: line 15 col 13
// CHECK-NEXT:    bc 31: line 16 col 1
// CHECK-NEXT:  0x0045  function idx 2, starts at line 18 col 1
// CHECK-NEXT:    bc 4: line 18 col 1
// CHECK-NEXT:    bc 12: line 19 col 13
// CHECK-NEXT:    bc 19: line 20 col 1
// CHECK-NEXT:  0x0053  function idx 3, starts at line 22 col 1
// CHECK-NEXT:    bc 4: line 22 col 1
// CHECK-NEXT:    bc 10: line 22 col 1
// CHECK-NEXT:    bc 14: line 22 col 1
// CHECK-NEXT:    bc 19: line 22 col 1
// CHECK-NEXT:    bc 25: line 23 col 13
// CHECK-NEXT:    bc 32: line 27 col 1
// CHECK-NEXT:  0x006b  function idx 4, starts at line 24 col 5
// CHECK-NEXT:    bc 4: line 24 col 5
// CHECK-NEXT:    bc 11: line 25 col 18
// CHECK-NEXT:    bc 18: line 26 col 5
// CHECK-NEXT:  0x0079  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  lexical parent: 0, variable count: 2
// CHECK-NEXT:    "v1a"
// CHECK-NEXT:    "v2a"
// CHECK-NEXT:  0x000c  lexical parent: 0, variable count: 1
// CHECK-NEXT:    "v1b"
// CHECK-NEXT:  0x0012  lexical parent: 0, variable count: 2
// CHECK-NEXT:    "v1c"
// CHECK-NEXT:    "Fcc"
// CHECK-NEXT:  0x001c  lexical parent: 3, variable count: 1
// CHECK-NEXT:    "v1cc"
// CHECK-NEXT:  0x0023  end of debug lexical table

// UNICODE:Bytecode File Information:
// UNICODE-NEXT:  Bytecode version number: {{.*}}
// UNICODE-NEXT:  Source hash: {{.*}}
// UNICODE-NEXT:  Function count: 5
// UNICODE-NEXT:  String count: 7
// UNICODE-NEXT:  BigInt count: 0
// UNICODE-NEXT:  String Kind Entry count: 2
// UNICODE-NEXT:  RegExp count: 0
// UNICODE-NEXT:  Segment ID: 0
// UNICODE-NEXT:  CommonJS module count: 0
// UNICODE-NEXT:  CommonJS module count (static): 0
// UNICODE-NEXT:  Function source count: 0
// UNICODE-NEXT:  Bytecode options:
// UNICODE-NEXT:    staticBuiltins: 0
// UNICODE-NEXT:    cjsModulesStaticallyResolved: 0

// UNICODE:Global String Table:
// UNICODE-NEXT:s0[ASCII, 0..2]: Fcc
// UNICODE-NEXT:s1[ASCII, 3..5]: abc
// UNICODE-NEXT:s2[ASCII, 6..11]: global
// UNICODE-NEXT:i3[ASCII, 12..13] #0462F07A: Fa
// UNICODE-NEXT:i4[ASCII, 14..15] #04628D8B: Fb
// UNICODE-NEXT:i5[ASCII, 16..17] #04628998: Fc
// UNICODE-NEXT:i6[ASCII, 18..20] #913AA8C8: v1g

// UNICODE:Function<global>(1 params, 13 registers, 0 symbols):
// UNICODE-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// UNICODE-NEXT:    CreateEnvironment r0
// UNICODE-NEXT:    DeclareGlobalVar  "v1g"
// UNICODE-NEXT:    DeclareGlobalVar  "Fa"
// UNICODE-NEXT:    DeclareGlobalVar  "Fb"
// UNICODE-NEXT:    DeclareGlobalVar  "Fc"
// UNICODE-NEXT:    CreateClosure     r1, r0, Function<Fa>
// UNICODE-NEXT:    GetGlobalObject   r2
// UNICODE-NEXT:    PutByIdLoose      r2, r1, 1, "Fa"
// UNICODE-NEXT:    CreateClosure     r3, r0, Function<Fb>
// UNICODE-NEXT:    GetGlobalObject   r4
// UNICODE-NEXT:    PutByIdLoose      r4, r3, 2, "Fb"
// UNICODE-NEXT:    CreateClosure     r5, r0, Function<Fc>
// UNICODE-NEXT:    GetGlobalObject   r6
// UNICODE-NEXT:    PutByIdLoose      r6, r5, 3, "Fc"
// UNICODE-NEXT:    LoadConstUndefined r8
// UNICODE-NEXT:    Mov               r7, r8
// UNICODE-NEXT:    LoadConstString   r9, "global"
// UNICODE-NEXT:    GetGlobalObject   r10
// UNICODE-NEXT:    PutByIdLoose      r10, r9, 4, "v1g"
// UNICODE-NEXT:    Mov               r11, r7
// UNICODE-NEXT:    AsyncBreakCheck
// UNICODE-NEXT:    Ret               r11

// UNICODE:Function<Fa>(1 params, 7 registers, 2 symbols):
// UNICODE-NEXT:Offset in debug table: source 0x0030, lexical 0x0002
// UNICODE-NEXT:    CreateEnvironment r0
// UNICODE-NEXT:    LoadConstUndefined r1
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r1
// UNICODE-NEXT:    LoadConstUndefined r2
// UNICODE-NEXT:    StoreNPToEnvironment r0, 1, r2
// UNICODE-NEXT:    LoadConstUInt8    r3, 3
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r3
// UNICODE-NEXT:    LoadConstUInt8    r4, 5
// UNICODE-NEXT:    StoreNPToEnvironment r0, 1, r4
// UNICODE-NEXT:    LoadConstUndefined r5
// UNICODE-NEXT:    AsyncBreakCheck
// UNICODE-NEXT:    Ret               r5

// UNICODE:Function<Fb>(1 params, 5 registers, 1 symbols):
// UNICODE-NEXT:Offset in debug table: source 0x0045, lexical 0x000c
// UNICODE-NEXT:    CreateEnvironment r0
// UNICODE-NEXT:    LoadConstUndefined r1
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r1
// UNICODE-NEXT:    LoadConstString   r2, "abc"
// UNICODE-NEXT:    StoreToEnvironment r0, 0, r2
// UNICODE-NEXT:    LoadConstUndefined r3
// UNICODE-NEXT:    AsyncBreakCheck
// UNICODE-NEXT:    Ret               r3

// UNICODE:Function<Fc>(1 params, 7 registers, 2 symbols):
// UNICODE-NEXT:Offset in debug table: source 0x0053, lexical 0x0012
// UNICODE-NEXT:    CreateEnvironment r0
// UNICODE-NEXT:    LoadConstUndefined r1
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r1
// UNICODE-NEXT:    LoadConstUndefined r2
// UNICODE-NEXT:    StoreNPToEnvironment r0, 1, r2
// UNICODE-NEXT:    CreateClosure     r3, r0, Function<Fcc>
// UNICODE-NEXT:    StoreToEnvironment r0, 1, r3
// UNICODE-NEXT:    LoadConstUndefined r4
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r4
// UNICODE-NEXT:    LoadConstUndefined r5
// UNICODE-NEXT:    AsyncBreakCheck
// UNICODE-NEXT:    Ret               r5

// UNICODE:Function<Fcc>(1 params, 5 registers, 1 symbols):
// UNICODE-NEXT:Offset in debug table: source 0x006b, lexical 0x001c
// UNICODE-NEXT:    CreateEnvironment r0
// UNICODE-NEXT:    LoadConstUndefined r1
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r1
// UNICODE-NEXT:    LoadConstUInt8    r2, 42
// UNICODE-NEXT:    StoreNPToEnvironment r0, 0, r2
// UNICODE-NEXT:    LoadConstUndefined r3
// UNICODE-NEXT:    AsyncBreakCheck
// UNICODE-NEXT:    Ret               r3

// UNICODE:Debug filename table:
// UNICODE-NEXT:  0: /Users/fbmal7/builds/shdebug/test/BCGen/HBC/Output/debug_info_à.js

// UNICODE:Debug file table:
// UNICODE-NEXT:  source table offset 0x0000: filename id 0

// UNICODE:Debug source table:
// UNICODE-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// UNICODE-NEXT:    bc 2: line 11 col 1
// UNICODE-NEXT:    bc 7: line 11 col 1
// UNICODE-NEXT:    bc 12: line 11 col 1
// UNICODE-NEXT:    bc 17: line 11 col 1
// UNICODE-NEXT:    bc 22: line 11 col 1
// UNICODE-NEXT:    bc 29: line 11 col 1
// UNICODE-NEXT:    bc 35: line 11 col 1
// UNICODE-NEXT:    bc 42: line 11 col 1
// UNICODE-NEXT:    bc 48: line 11 col 1
// UNICODE-NEXT:    bc 55: line 11 col 1
// UNICODE-NEXT:    bc 61: line 11 col 1
// UNICODE-NEXT:    bc 72: line 11 col 9
// UNICODE-NEXT:    bc 78: line 11 col 1
// UNICODE-NEXT:    bc 82: line 27 col 1
// UNICODE-NEXT:  0x0030  function idx 1, starts at line 13 col 1
// UNICODE-NEXT:    bc 4: line 13 col 1
// UNICODE-NEXT:    bc 10: line 13 col 1
// UNICODE-NEXT:    bc 17: line 14 col 13
// UNICODE-NEXT:    bc 24: line 15 col 13
// UNICODE-NEXT:    bc 31: line 16 col 1
// UNICODE-NEXT:  0x0045  function idx 2, starts at line 18 col 1
// UNICODE-NEXT:    bc 4: line 18 col 1
// UNICODE-NEXT:    bc 12: line 19 col 13
// UNICODE-NEXT:    bc 19: line 20 col 1
// UNICODE-NEXT:  0x0053  function idx 3, starts at line 22 col 1
// UNICODE-NEXT:    bc 4: line 22 col 1
// UNICODE-NEXT:    bc 10: line 22 col 1
// UNICODE-NEXT:    bc 14: line 22 col 1
// UNICODE-NEXT:    bc 19: line 22 col 1
// UNICODE-NEXT:    bc 25: line 23 col 13
// UNICODE-NEXT:    bc 32: line 27 col 1
// UNICODE-NEXT:  0x006b  function idx 4, starts at line 24 col 5
// UNICODE-NEXT:    bc 4: line 24 col 5
// UNICODE-NEXT:    bc 11: line 25 col 18
// UNICODE-NEXT:    bc 18: line 26 col 5
// UNICODE-NEXT:  0x0079  end of debug source table

// UNICODE:Debug lexical table:
// UNICODE-NEXT:  0x0000  lexical parent: none, variable count: 0
// UNICODE-NEXT:  0x0002  lexical parent: 0, variable count: 2
// UNICODE-NEXT:    "v1a"
// UNICODE-NEXT:    "v2a"
// UNICODE-NEXT:  0x000c  lexical parent: 0, variable count: 1
// UNICODE-NEXT:    "v1b"
// UNICODE-NEXT:  0x0012  lexical parent: 0, variable count: 2
// UNICODE-NEXT:    "v1c"
// UNICODE-NEXT:    "Fcc"
// UNICODE-NEXT:  0x001c  lexical parent: 3, variable count: 1
// UNICODE-NEXT:    "v1cc"
// UNICODE-NEXT:  0x0023  end of debug lexical table
