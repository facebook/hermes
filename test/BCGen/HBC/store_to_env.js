/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
    var myNum = 1234;
    var myBool = true;
    var myString = 'a string';
    var myObj = new Object();
    var myRegExp = /Hermes/i;
    var myUndef = 'temp string';
    var myFunc = function bar(){
        myNum++;
        myBool = false;
        myString = 'new string';
        print(myObj);
        print(myRegExp);
        myUndef = undefined;
    }
    return myFunc;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 8
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 1
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..5]: Hermes
// CHECK-NEXT:s1[ASCII, 6..8]: bar
// CHECK-NEXT:s2[ASCII, 9..14]: global
// CHECK-NEXT:s3[ASCII, 15..15]: i
// CHECK-NEXT:i4[ASCII, 16..21] #9615E9FA: Object
// CHECK-NEXT:i5[ASCII, 22..24] #9290584E: foo
// CHECK-NEXT:i6[ASCII, 25..29] #A689F65B: print
// CHECK-NEXT:i7[ASCII, 30..38] #807C5F3D: prototype

// CHECK:Function<global>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateFunctionEnvironment r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r0
// CHECK-NEXT:    PutByIdLoose      r0, r1, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<foo>(1 params, 11 registers, 3 symbols):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    CreateFunctionEnvironment r0
// CHECK-NEXT:    LoadConstInt      r1, 1234
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    TryGetById        r1, r1, 1, "Object"
// CHECK-NEXT:    GetByIdShort      r2, r1, 2, "prototype"
// CHECK-NEXT:    CreateThis        r2, r2, r1
// CHECK-NEXT:    Mov               r3, r2
// CHECK-NEXT:    Construct         r1, r1, 1
// CHECK-NEXT:    SelectObject      r1, r2, r1
// CHECK-NEXT:    StoreToEnvironment r0, 1, r1
// CHECK-NEXT:    CreateRegExp      r1, "Hermes", "i", 0
// CHECK-NEXT:    StoreToEnvironment r0, 2, r1
// CHECK-NEXT:    CreateClosure     r0, r0, Function<bar>
// CHECK-NEXT:    Ret               r0

// CHECK:Function<bar>(1 params, 14 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0017, lexical 0x0000
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    LoadFromEnvironment r2, r1, 0
// CHECK-NEXT:    LoadConstUInt8    r0, 1
// CHECK-NEXT:    AddN              r0, r2, r0
// CHECK-NEXT:    StoreNPToEnvironment r1, 0, r0
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    TryGetById        r4, r2, 1, "print"
// CHECK-NEXT:    LoadFromEnvironment r3, r1, 1
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Call2             r3, r4, r0, r3
// CHECK-NEXT:    TryGetById        r2, r2, 1, "print"
// CHECK-NEXT:    LoadFromEnvironment r1, r1, 2
// CHECK-NEXT:    Call2             r1, r2, r0, r1
// CHECK-NEXT:    Ret               r0

// CHECK:RegExp Bytecodes:
// CHECK-NEXT:0: /Hermes/i
// CHECK-NEXT:  Header: marked: 0 loops: 0 flags: 1 constraints: 4
// CHECK-NEXT:  0000  MatchNCharICase8: 'HERMES'
// CHECK-NEXT:  0008  Goal

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}store_to_env.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 14: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 14: line 14 col 21
// CHECK-NEXT:    bc 20: line 14 col 27
// CHECK-NEXT:    bc 32: line 14 col 27
// CHECK-NEXT:  0x0017  function idx 2, starts at line 17 col 18
// CHECK-NEXT:    bc 20: line 21 col 9
// CHECK-NEXT:    bc 32: line 21 col 14
// CHECK-NEXT:    bc 37: line 22 col 9
// CHECK-NEXT:    bc 47: line 22 col 14
// CHECK-NEXT:  0x0027  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
