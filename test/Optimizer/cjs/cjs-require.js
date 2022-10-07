/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -fstatic-require -fstatic-builtins -dump-bytecode %s %S/m1.js %S/m2.js -O  | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s
// RUN: %hermesc -commonjs -fstatic-require -fstatic-builtins -dump-bytecode %s %S/m1.js %S/m2.js -Og | %FileCheckOrRegen --match-full-lines --check-prefix=CHKDBG %s

var m1 = require("./m" + "1.js");
m1.foo();

function bar() {
    var m2 = require("./m" + "2.js");
    m2.baz();
}

exports.bar = bar;

// Auto-generated content below. Please do not modify manually.

// CHKOPT:Bytecode File Information:
// CHKOPT-NEXT:  Bytecode version number: 90
// CHKOPT-NEXT:  Source hash: 0000000000000000000000000000000000000000
// CHKOPT-NEXT:  Function count: 7
// CHKOPT-NEXT:  String count: 7
// CHKOPT-NEXT:  BigInt count: 0
// CHKOPT-NEXT:  String Kind Entry count: 2
// CHKOPT-NEXT:  RegExp count: 0
// CHKOPT-NEXT:  Segment ID: 0
// CHKOPT-NEXT:  CommonJS module count: 0
// CHKOPT-NEXT:  CommonJS module count (static): 3
// CHKOPT-NEXT:  Function source count: 0
// CHKOPT-NEXT:  Bytecode options:
// CHKOPT-NEXT:    staticBuiltins: 1
// CHKOPT-NEXT:    cjsModulesStaticallyResolved: 1

// CHKOPT:Global String Table:
// CHKOPT-NEXT:s0[ASCII, 0..-1]:
// CHKOPT-NEXT:s1[ASCII, 0..9]: cjs_module
// CHKOPT-NEXT:s2[ASCII, 10..15]: global
// CHKOPT-NEXT:i3[ASCII, 16..18] #9B85A7ED: bar
// CHKOPT-NEXT:i4[ASCII, 19..21] #9B85C665: baz
// CHKOPT-NEXT:i5[ASCII, 22..24] #9290584E: foo
// CHKOPT-NEXT:i6[ASCII, 25..29] #A689F65B: print

// CHKOPT:CommonJS Modules (Static):
// CHKOPT-NEXT:Module ID 0 -> function ID 1
// CHKOPT-NEXT:Module ID 1 -> function ID 3
// CHKOPT-NEXT:Module ID 2 -> function ID 5

// CHKOPT:Function<global>(1 params, 1 registers, 0 symbols):
// CHKOPT-NEXT:    LoadConstUndefined r0
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<cjs_module>(4 params, 10 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKOPT-NEXT:    LoadConstUInt8    r2, 1
// CHKOPT-NEXT:    CallBuiltin       r1, "HermesBuiltin.requireFast", 2
// CHKOPT-NEXT:    GetByIdShort      r0, r1, 1, "foo"
// CHKOPT-NEXT:    Call1             r0, r0, r1
// CHKOPT-NEXT:    CreateEnvironment r0
// CHKOPT-NEXT:    CreateClosure     r1, r0, Function<bar>
// CHKOPT-NEXT:    LoadParam         r0, 1
// CHKOPT-NEXT:    PutById           r0, r1, 1, "bar"
// CHKOPT-NEXT:    LoadConstUndefined r0
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<bar>(1 params, 10 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHKOPT-NEXT:    LoadConstUInt8    r2, 2
// CHKOPT-NEXT:    CallBuiltin       r1, "HermesBuiltin.requireFast", 2
// CHKOPT-NEXT:    GetByIdShort      r0, r1, 1, "baz"
// CHKOPT-NEXT:    Call1             r0, r0, r1
// CHKOPT-NEXT:    LoadConstUndefined r0
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<cjs_module>(4 params, 2 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x001d, lexical 0x0000
// CHKOPT-NEXT:    CreateEnvironment r0
// CHKOPT-NEXT:    CreateClosure     r1, r0, Function<>
// CHKOPT-NEXT:    LoadParam         r0, 1
// CHKOPT-NEXT:    PutById           r0, r1, 1, "foo"
// CHKOPT-NEXT:    LoadConstUndefined r0
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<>(1 params, 11 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x0024, lexical 0x0000
// CHKOPT-NEXT:    GetGlobalObject   r0
// CHKOPT-NEXT:    TryGetById        r2, r0, 1, "print"
// CHKOPT-NEXT:    LoadConstUndefined r1
// CHKOPT-NEXT:    LoadConstString   r0, "foo"
// CHKOPT-NEXT:    Call2             r0, r2, r1, r0
// CHKOPT-NEXT:    LoadConstUInt8    r0, 1
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<cjs_module>(4 params, 2 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x002e, lexical 0x0000
// CHKOPT-NEXT:    CreateEnvironment r0
// CHKOPT-NEXT:    CreateClosure     r1, r0, Function<>
// CHKOPT-NEXT:    LoadParam         r0, 1
// CHKOPT-NEXT:    PutById           r0, r1, 1, "baz"
// CHKOPT-NEXT:    LoadConstUndefined r0
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Function<>(1 params, 11 registers, 0 symbols):
// CHKOPT-NEXT:Offset in debug table: source 0x0035, lexical 0x0000
// CHKOPT-NEXT:    GetGlobalObject   r0
// CHKOPT-NEXT:    TryGetById        r2, r0, 1, "print"
// CHKOPT-NEXT:    LoadConstUndefined r1
// CHKOPT-NEXT:    LoadConstString   r0, "baz"
// CHKOPT-NEXT:    Call2             r0, r2, r1, r0
// CHKOPT-NEXT:    LoadConstUInt8    r0, 2
// CHKOPT-NEXT:    Ret               r0

// CHKOPT:Debug filename table:
// CHKOPT-NEXT:  0: {{.*}}cjs-require.js
// CHKOPT-NEXT:  1: {{.*}}m1.js
// CHKOPT-NEXT:  2: {{.*}}m2.js

// CHKOPT:Debug file table:
// CHKOPT-NEXT:  source table offset 0x0000: filename id 0
// CHKOPT-NEXT:  source table offset 0x001d: filename id 1
// CHKOPT-NEXT:  source table offset 0x002e: filename id 2

// CHKOPT:Debug source table:
// CHKOPT-NEXT:  0x0000  function idx 1, starts at line 11 col 1
// CHKOPT-NEXT:    bc 3: line 11 col 17
// CHKOPT-NEXT:    bc 7: line 12 col 7
// CHKOPT-NEXT:    bc 12: line 12 col 7
// CHKOPT-NEXT:    bc 26: line 19 col 13
// CHKOPT-NEXT:  0x0010  function idx 2, starts at line 14 col 1
// CHKOPT-NEXT:    bc 3: line 15 col 21
// CHKOPT-NEXT:    bc 7: line 16 col 11
// CHKOPT-NEXT:    bc 12: line 16 col 11
// CHKOPT-NEXT:  0x001d  function idx 3, starts at line 10 col 1
// CHKOPT-NEXT:    bc 10: line 10 col 13
// CHKOPT-NEXT:  0x0024  function idx 4, starts at line 10 col 15
// CHKOPT-NEXT:    bc 2: line 11 col 3
// CHKOPT-NEXT:    bc 14: line 11 col 8
// CHKOPT-NEXT:  0x002e  function idx 5, starts at line 10 col 1
// CHKOPT-NEXT:    bc 10: line 10 col 13
// CHKOPT-NEXT:  0x0035  function idx 6, starts at line 10 col 15
// CHKOPT-NEXT:    bc 2: line 11 col 3
// CHKOPT-NEXT:    bc 14: line 11 col 8
// CHKOPT-NEXT:  0x003f  end of debug source table

// CHKOPT:Debug lexical table:
// CHKOPT-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHKOPT-NEXT:  0x0002  end of debug lexical table

// CHKDBG:Bytecode File Information:
// CHKDBG-NEXT:  Bytecode version number: 90
// CHKDBG-NEXT:  Source hash: 0000000000000000000000000000000000000000
// CHKDBG-NEXT:  Function count: 7
// CHKDBG-NEXT:  String count: 7
// CHKDBG-NEXT:  BigInt count: 0
// CHKDBG-NEXT:  String Kind Entry count: 2
// CHKDBG-NEXT:  RegExp count: 0
// CHKDBG-NEXT:  Segment ID: 0
// CHKDBG-NEXT:  CommonJS module count: 0
// CHKDBG-NEXT:  CommonJS module count (static): 3
// CHKDBG-NEXT:  Function source count: 0
// CHKDBG-NEXT:  Bytecode options:
// CHKDBG-NEXT:    staticBuiltins: 1
// CHKDBG-NEXT:    cjsModulesStaticallyResolved: 1

// CHKDBG:Global String Table:
// CHKDBG-NEXT:s0[ASCII, 0..-1]:
// CHKDBG-NEXT:s1[ASCII, 0..9]: cjs_module
// CHKDBG-NEXT:s2[ASCII, 10..15]: global
// CHKDBG-NEXT:i3[ASCII, 16..18] #9B85A7ED: bar
// CHKDBG-NEXT:i4[ASCII, 19..21] #9B85C665: baz
// CHKDBG-NEXT:i5[ASCII, 22..24] #9290584E: foo
// CHKDBG-NEXT:i6[ASCII, 25..29] #A689F65B: print

// CHKDBG:CommonJS Modules (Static):
// CHKDBG-NEXT:Module ID 0 -> function ID 1
// CHKDBG-NEXT:Module ID 1 -> function ID 3
// CHKDBG-NEXT:Module ID 2 -> function ID 5

// CHKDBG:Function<global>(1 params, 4 registers, 0 symbols):
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    LoadConstUndefined r0
// CHKDBG-NEXT:    Mov               r1, r0
// CHKDBG-NEXT:    Mov               r2, r1
// CHKDBG-NEXT:    Ret               r2

// CHKDBG:Function<cjs_module>(4 params, 21 registers, 5 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    LoadParam         r1, 1
// CHKDBG-NEXT:    LoadParam         r2, 2
// CHKDBG-NEXT:    LoadParam         r3, 3
// CHKDBG-NEXT:    LoadConstUndefined r4
// CHKDBG-NEXT:    LoadConstUInt8    r5, 1
// CHKDBG-NEXT:    StoreToEnvironment r0, 0, r1
// CHKDBG-NEXT:    StoreToEnvironment r0, 1, r2
// CHKDBG-NEXT:    StoreToEnvironment r0, 2, r3
// CHKDBG-NEXT:    StoreNPToEnvironment r0, 3, r4
// CHKDBG-NEXT:    CreateClosure     r6, r0, Function<bar>
// CHKDBG-NEXT:    StoreToEnvironment r0, 4, r6
// CHKDBG-NEXT:    LoadFromEnvironment r7, r0, 1
// CHKDBG-NEXT:    Mov               r13, r5
// CHKDBG-NEXT:    CallBuiltin       r7, "HermesBuiltin.requireFast", 2
// CHKDBG-NEXT:    StoreToEnvironment r0, 3, r7
// CHKDBG-NEXT:    LoadFromEnvironment r8, r0, 3
// CHKDBG-NEXT:    GetByIdShort      r9, r8, 1, "foo"
// CHKDBG-NEXT:    Mov               r14, r8
// CHKDBG-NEXT:    Call              r10, r9, 1
// CHKDBG-NEXT:    LoadFromEnvironment r10, r0, 0
// CHKDBG-NEXT:    LoadFromEnvironment r11, r0, 4
// CHKDBG-NEXT:    PutById           r10, r11, 1, "bar"
// CHKDBG-NEXT:    Ret               r4

// CHKDBG:Function<bar>(1 params, 16 registers, 1 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    LoadConstUndefined r1
// CHKDBG-NEXT:    LoadConstUInt8    r2, 2
// CHKDBG-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHKDBG-NEXT:    GetEnvironment    r3, 0
// CHKDBG-NEXT:    LoadFromEnvironment r4, r3, 1
// CHKDBG-NEXT:    Mov               r8, r2
// CHKDBG-NEXT:    CallBuiltin       r4, "HermesBuiltin.requireFast", 2
// CHKDBG-NEXT:    StoreToEnvironment r0, 0, r4
// CHKDBG-NEXT:    LoadFromEnvironment r5, r0, 0
// CHKDBG-NEXT:    GetByIdShort      r6, r5, 1, "baz"
// CHKDBG-NEXT:    Mov               r9, r5
// CHKDBG-NEXT:    Call              r7, r6, 1
// CHKDBG-NEXT:    Ret               r1

// CHKDBG:Function<cjs_module>(4 params, 8 registers, 3 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x001d, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    LoadParam         r1, 1
// CHKDBG-NEXT:    LoadParam         r2, 2
// CHKDBG-NEXT:    LoadParam         r3, 3
// CHKDBG-NEXT:    LoadConstUndefined r4
// CHKDBG-NEXT:    StoreToEnvironment r0, 0, r1
// CHKDBG-NEXT:    StoreToEnvironment r0, 1, r2
// CHKDBG-NEXT:    StoreToEnvironment r0, 2, r3
// CHKDBG-NEXT:    LoadFromEnvironment r5, r0, 0
// CHKDBG-NEXT:    CreateClosure     r6, r0, Function<>
// CHKDBG-NEXT:    PutById           r5, r6, 1, "foo"
// CHKDBG-NEXT:    Ret               r4

// CHKDBG:Function<>(1 params, 14 registers, 0 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x0024, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    GetGlobalObject   r0
// CHKDBG-NEXT:    LoadConstUndefined r1
// CHKDBG-NEXT:    LoadConstString   r2, "foo"
// CHKDBG-NEXT:    LoadConstUInt8    r3, 1
// CHKDBG-NEXT:    TryGetById        r4, r0, 1, "print"
// CHKDBG-NEXT:    Mov               r7, r1
// CHKDBG-NEXT:    Mov               r6, r2
// CHKDBG-NEXT:    Call              r5, r4, 2
// CHKDBG-NEXT:    Ret               r3

// CHKDBG:Function<cjs_module>(4 params, 8 registers, 3 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x002e, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    LoadParam         r1, 1
// CHKDBG-NEXT:    LoadParam         r2, 2
// CHKDBG-NEXT:    LoadParam         r3, 3
// CHKDBG-NEXT:    LoadConstUndefined r4
// CHKDBG-NEXT:    StoreToEnvironment r0, 0, r1
// CHKDBG-NEXT:    StoreToEnvironment r0, 1, r2
// CHKDBG-NEXT:    StoreToEnvironment r0, 2, r3
// CHKDBG-NEXT:    LoadFromEnvironment r5, r0, 0
// CHKDBG-NEXT:    CreateClosure     r6, r0, Function<>
// CHKDBG-NEXT:    PutById           r5, r6, 1, "baz"
// CHKDBG-NEXT:    Ret               r4

// CHKDBG:Function<>(1 params, 14 registers, 0 symbols):
// CHKDBG-NEXT:Offset in debug table: source 0x0035, lexical 0x0000
// CHKDBG-NEXT:    CreateEnvironment r0
// CHKDBG-NEXT:    GetGlobalObject   r0
// CHKDBG-NEXT:    LoadConstUndefined r1
// CHKDBG-NEXT:    LoadConstString   r2, "baz"
// CHKDBG-NEXT:    LoadConstUInt8    r3, 2
// CHKDBG-NEXT:    TryGetById        r4, r0, 1, "print"
// CHKDBG-NEXT:    Mov               r7, r1
// CHKDBG-NEXT:    Mov               r6, r2
// CHKDBG-NEXT:    Call              r5, r4, 2
// CHKDBG-NEXT:    Ret               r3

// CHKDBG:Debug filename table:
// CHKDBG-NEXT:  0: {{.*}}cjs-require.js
// CHKDBG-NEXT:  1: {{.*}}m1.js
// CHKDBG-NEXT:  2: {{.*}}m2.js

// CHKDBG:Debug file table:
// CHKDBG-NEXT:  source table offset 0x0000: filename id 0
// CHKDBG-NEXT:  source table offset 0x001d: filename id 1
// CHKDBG-NEXT:  source table offset 0x002e: filename id 2

// CHKDBG:Debug source table:
// CHKDBG-NEXT:  0x0000  function idx 1, starts at line 11 col 1
// CHKDBG-NEXT:    bc 48: line 11 col 17
// CHKDBG-NEXT:    bc 60: line 12 col 7
// CHKDBG-NEXT:    bc 68: line 12 col 7
// CHKDBG-NEXT:    bc 80: line 19 col 13
// CHKDBG-NEXT:  0x0010  function idx 2, starts at line 14 col 1
// CHKDBG-NEXT:    bc 21: line 15 col 21
// CHKDBG-NEXT:    bc 33: line 16 col 11
// CHKDBG-NEXT:    bc 41: line 16 col 11
// CHKDBG-NEXT:  0x001d  function idx 3, starts at line 10 col 1
// CHKDBG-NEXT:    bc 34: line 10 col 13
// CHKDBG-NEXT:  0x0024  function idx 4, starts at line 10 col 15
// CHKDBG-NEXT:    bc 13: line 11 col 3
// CHKDBG-NEXT:    bc 25: line 11 col 8
// CHKDBG-NEXT:  0x002e  function idx 5, starts at line 10 col 1
// CHKDBG-NEXT:    bc 34: line 10 col 13
// CHKDBG-NEXT:  0x0035  function idx 6, starts at line 10 col 15
// CHKDBG-NEXT:    bc 13: line 11 col 3
// CHKDBG-NEXT:    bc 25: line 11 col 8
// CHKDBG-NEXT:  0x003f  end of debug source table

// CHKDBG:Debug lexical table:
// CHKDBG-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHKDBG-NEXT:  0x0002  end of debug lexical table
