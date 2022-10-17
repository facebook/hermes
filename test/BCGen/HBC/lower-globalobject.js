/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKBC %s

var x;

function foo() {
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHKRA:function global#0()#1 : undefined
// CHKRA-NEXT:frame = [], globals = [x, foo]
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHKRA-NEXT:  $Reg1 @1 [2...4) 	%1 = HBCCreateFunctionInst %foo#0#1()#2, %0
// CHKRA-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCGetGlobalObjectInst
// CHKRA-NEXT:  $Reg0 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHKRA-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst undefined : undefined
// CHKRA-NEXT:  $Reg0 @5 [empty]	%5 = ReturnInst %4 : undefined
// CHKRA-NEXT:function_end

// CHKRA:function foo#0#1()#2
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCGetGlobalObjectInst
// CHKRA-NEXT:  $Reg0 @1 [2...3) 	%1 = LoadPropertyInst %0 : object, "x" : string
// CHKRA-NEXT:  $Reg0 @2 [empty]	%2 = ReturnInst %1
// CHKRA-NEXT:function_end

// CHKBC:Bytecode File Information:
// CHKBC-NEXT:  Bytecode version number: 90
// CHKBC-NEXT:  Source hash: 0000000000000000000000000000000000000000
// CHKBC-NEXT:  Function count: 2
// CHKBC-NEXT:  String count: 3
// CHKBC-NEXT:  BigInt count: 0
// CHKBC-NEXT:  String Kind Entry count: 2
// CHKBC-NEXT:  RegExp count: 0
// CHKBC-NEXT:  Segment ID: 0
// CHKBC-NEXT:  CommonJS module count: 0
// CHKBC-NEXT:  CommonJS module count (static): 0
// CHKBC-NEXT:  Function source count: 0
// CHKBC-NEXT:  Bytecode options:
// CHKBC-NEXT:    staticBuiltins: 0
// CHKBC-NEXT:    cjsModulesStaticallyResolved: 0

// CHKBC:Global String Table:
// CHKBC-NEXT:s0[ASCII, 0..5]: global
// CHKBC-NEXT:i1[ASCII, 6..8] #9290584E: foo
// CHKBC-NEXT:i2[ASCII, 9..9] #0001E7F9: x

// CHKBC:Function<global>(1 params, 2 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKBC-NEXT:    DeclareGlobalVar  "x"
// CHKBC-NEXT:    DeclareGlobalVar  "foo"
// CHKBC-NEXT:    CreateEnvironment r0
// CHKBC-NEXT:    CreateClosure     r1, r0, Function<foo>
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    PutById           r0, r1, 1, "foo"
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo>(1 params, 1 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0007, lexical 0x0000
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    GetByIdShort      r0, r0, 1, "x"
// CHKBC-NEXT:    Ret               r0

// CHKBC:Debug filename table:
// CHKBC-NEXT:  0: {{.*}}lower-globalobject.js

// CHKBC:Debug file table:
// CHKBC-NEXT:  source table offset 0x0000: filename id 0

// CHKBC:Debug source table:
// CHKBC-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHKBC-NEXT:    bc 19: line 11 col 1
// CHKBC-NEXT:  0x0007  function idx 1, starts at line 13 col 1
// CHKBC-NEXT:    bc 2: line 14 col 10
// CHKBC-NEXT:  0x000e  end of debug source table

// CHKBC:Debug lexical table:
// CHKBC-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHKBC-NEXT:  0x0002  end of debug lexical table
