/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-lra %s | %FileCheckOrRegen --check-prefix=LRA --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheckOrRegen --check-prefix=BCGEN --match-full-lines %s

// Variants that should produce a HBCCallNInst instruction.

function foo1(f) { f(); }

function foo2(f) { f(1); }

function foo3(f) { f(1, 2); }

function foo4(f) { f(1, 2, 3); }

// This has too many parameters and so will be an ordinary Call instruction, not HBCCallNInst.

function foo5(f) { f(1, 2, 3, 4); }

// Auto-generated content below. Please do not modify manually.

// LRA:function global#0()#1 : undefined
// LRA-NEXT:frame = [], globals = [foo1, foo2, foo3, foo4, foo5]
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg0 @0 [1...11) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// LRA-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCCreateFunctionInst %foo1#0#1()#2 : undefined, %0
// LRA-NEXT:  $Reg1 @2 [3...12) 	%2 = HBCGetGlobalObjectInst
// LRA-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "foo1" : string
// LRA-NEXT:  $Reg2 @4 [5...6) 	%4 = HBCCreateFunctionInst %foo2#0#1()#3 : undefined, %0
// LRA-NEXT:  $Reg2 @5 [empty]	%5 = StorePropertyInst %4 : closure, %2 : object, "foo2" : string
// LRA-NEXT:  $Reg2 @6 [7...8) 	%6 = HBCCreateFunctionInst %foo3#0#1()#4 : undefined, %0
// LRA-NEXT:  $Reg2 @7 [empty]	%7 = StorePropertyInst %6 : closure, %2 : object, "foo3" : string
// LRA-NEXT:  $Reg2 @8 [9...10) 	%8 = HBCCreateFunctionInst %foo4#0#1()#5 : undefined, %0
// LRA-NEXT:  $Reg2 @9 [empty]	%9 = StorePropertyInst %8 : closure, %2 : object, "foo4" : string
// LRA-NEXT:  $Reg0 @10 [11...12) 	%10 = HBCCreateFunctionInst %foo5#0#1()#6 : undefined, %0
// LRA-NEXT:  $Reg0 @11 [empty]	%11 = StorePropertyInst %10 : closure, %2 : object, "foo5" : string
// LRA-NEXT:  $Reg0 @12 [13...14) 	%12 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg0 @13 [empty]	%13 = ReturnInst %12 : undefined
// LRA-NEXT:function_end

// LRA:function foo1#0#1(f)#2 : undefined
// LRA-NEXT:frame = []
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg1 @0 [1...3) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:  $Reg0 @1 [2...4) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg2           	%2 = ImplicitMovInst %1 : undefined
// LRA-NEXT:  $Reg1 @2 [empty]	%3 = HBCCallNInst %0, %1 : undefined
// LRA-NEXT:  $Reg0 @3 [empty]	%4 = ReturnInst %1 : undefined
// LRA-NEXT:function_end

// LRA:function foo2#0#1(f)#2 : undefined
// LRA-NEXT:frame = []
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg2 @0 [1...4) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:  $Reg0 @1 [2...5) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg1 @2 [3...4) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:  $Reg4           	%3 = ImplicitMovInst %1 : undefined
// LRA-NEXT:  $Reg3           	%4 = ImplicitMovInst %2 : number
// LRA-NEXT:  $Reg1 @3 [empty]	%5 = HBCCallNInst %0, %1 : undefined, %2 : number
// LRA-NEXT:  $Reg0 @4 [empty]	%6 = ReturnInst %1 : undefined
// LRA-NEXT:function_end

// LRA:function foo3#0#1(f)#2 : undefined
// LRA-NEXT:frame = []
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg3 @0 [1...5) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:  $Reg0 @1 [2...6) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg2 @2 [3...5) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:  $Reg1 @3 [4...5) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:  $Reg6           	%4 = ImplicitMovInst %1 : undefined
// LRA-NEXT:  $Reg5           	%5 = ImplicitMovInst %2 : number
// LRA-NEXT:  $Reg4           	%6 = ImplicitMovInst %3 : number
// LRA-NEXT:  $Reg1 @4 [empty]	%7 = HBCCallNInst %0, %1 : undefined, %2 : number, %3 : number
// LRA-NEXT:  $Reg0 @5 [empty]	%8 = ReturnInst %1 : undefined
// LRA-NEXT:function_end

// LRA:function foo4#0#1(f)#2 : undefined
// LRA-NEXT:frame = []
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg4 @0 [1...6) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:  $Reg0 @1 [2...7) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg3 @2 [3...6) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:  $Reg2 @3 [4...6) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:  $Reg1 @4 [5...6) 	%4 = HBCLoadConstInst 3 : number
// LRA-NEXT:  $Reg8           	%5 = ImplicitMovInst %1 : undefined
// LRA-NEXT:  $Reg7           	%6 = ImplicitMovInst %2 : number
// LRA-NEXT:  $Reg6           	%7 = ImplicitMovInst %3 : number
// LRA-NEXT:  $Reg5           	%8 = ImplicitMovInst %4 : number
// LRA-NEXT:  $Reg1 @5 [empty]	%9 = HBCCallNInst %0, %1 : undefined, %2 : number, %3 : number, %4 : number
// LRA-NEXT:  $Reg0 @6 [empty]	%10 = ReturnInst %1 : undefined
// LRA-NEXT:function_end

// LRA:function foo5#0#1(f)#2 : undefined
// LRA-NEXT:frame = []
// LRA-NEXT:%BB0:
// LRA-NEXT:  $Reg5 @0 [1...7) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:  $Reg0 @1 [2...8) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg9 @2 [3...7) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:  $Reg8 @3 [4...7) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:  $Reg7 @4 [5...7) 	%4 = HBCLoadConstInst 3 : number
// LRA-NEXT:  $Reg6 @5 [6...7) 	%5 = HBCLoadConstInst 4 : number
// LRA-NEXT:  $Reg10           	%6 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:  $Reg1 @6 [empty]	%7 = CallInst %0, %6 : undefined, %2 : number, %3 : number, %4 : number, %5 : number
// LRA-NEXT:  $Reg0 @7 [empty]	%8 = ReturnInst %1 : undefined
// LRA-NEXT:function_end

// BCGEN:Bytecode File Information:
// BCGEN-NEXT:  Bytecode version number: 90
// BCGEN-NEXT:  Source hash: 0000000000000000000000000000000000000000
// BCGEN-NEXT:  Function count: 6
// BCGEN-NEXT:  String count: 6
// BCGEN-NEXT:  BigInt count: 0
// BCGEN-NEXT:  String Kind Entry count: 2
// BCGEN-NEXT:  RegExp count: 0
// BCGEN-NEXT:  Segment ID: 0
// BCGEN-NEXT:  CommonJS module count: 0
// BCGEN-NEXT:  CommonJS module count (static): 0
// BCGEN-NEXT:  Function source count: 0
// BCGEN-NEXT:  Bytecode options:
// BCGEN-NEXT:    staticBuiltins: 0
// BCGEN-NEXT:    cjsModulesStaticallyResolved: 0

// BCGEN:Global String Table:
// BCGEN-NEXT:s0[ASCII, 0..5]: global
// BCGEN-NEXT:i1[ASCII, 6..9] #D0BD9D2E: foo1
// BCGEN-NEXT:i2[ASCII, 10..13] #D0BD91E2: foo2
// BCGEN-NEXT:i3[ASCII, 14..17] #D0BD95F3: foo3
// BCGEN-NEXT:i4[ASCII, 18..21] #D0BDA900: foo4
// BCGEN-NEXT:i5[ASCII, 22..25] #D0BDAD11: foo5

// BCGEN:Function<global>(1 params, 3 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// BCGEN-NEXT:    DeclareGlobalVar  "foo1"
// BCGEN-NEXT:    DeclareGlobalVar  "foo2"
// BCGEN-NEXT:    DeclareGlobalVar  "foo3"
// BCGEN-NEXT:    DeclareGlobalVar  "foo4"
// BCGEN-NEXT:    DeclareGlobalVar  "foo5"
// BCGEN-NEXT:    CreateEnvironment r0
// BCGEN-NEXT:    CreateClosure     r2, r0, Function<foo1>
// BCGEN-NEXT:    GetGlobalObject   r1
// BCGEN-NEXT:    PutById           r1, r2, 1, "foo1"
// BCGEN-NEXT:    CreateClosure     r2, r0, Function<foo2>
// BCGEN-NEXT:    PutById           r1, r2, 2, "foo2"
// BCGEN-NEXT:    CreateClosure     r2, r0, Function<foo3>
// BCGEN-NEXT:    PutById           r1, r2, 3, "foo3"
// BCGEN-NEXT:    CreateClosure     r2, r0, Function<foo4>
// BCGEN-NEXT:    PutById           r1, r2, 4, "foo4"
// BCGEN-NEXT:    CreateClosure     r0, r0, Function<foo5>
// BCGEN-NEXT:    PutById           r1, r0, 5, "foo5"
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo1>(2 params, 9 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x0013, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r1, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    Call1             r1, r1, r0
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo2>(2 params, 11 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x001a, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r2, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    LoadConstUInt8    r1, 1
// BCGEN-NEXT:    Call2             r1, r2, r0, r1
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo3>(2 params, 13 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x0021, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r3, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    LoadConstUInt8    r2, 1
// BCGEN-NEXT:    LoadConstUInt8    r1, 2
// BCGEN-NEXT:    Call3             r1, r3, r0, r2, r1
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo4>(2 params, 15 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x0028, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r4, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    LoadConstUInt8    r3, 1
// BCGEN-NEXT:    LoadConstUInt8    r2, 2
// BCGEN-NEXT:    LoadConstUInt8    r1, 3
// BCGEN-NEXT:    Call4             r1, r4, r0, r3, r2, r1
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo5>(2 params, 17 registers, 0 symbols):
// BCGEN-NEXT:Offset in debug table: source 0x002f, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r5, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    LoadConstUInt8    r9, 1
// BCGEN-NEXT:    LoadConstUInt8    r8, 2
// BCGEN-NEXT:    LoadConstUInt8    r7, 3
// BCGEN-NEXT:    LoadConstUInt8    r6, 4
// BCGEN-NEXT:    LoadConstUndefined r10
// BCGEN-NEXT:    Call              r1, r5, 5
// BCGEN-NEXT:    Ret               r0

// BCGEN:Debug filename table:
// BCGEN-NEXT:  0: {{.*}}calln.js

// BCGEN:Debug file table:
// BCGEN-NEXT:  source table offset 0x0000: filename id 0

// BCGEN:Debug source table:
// BCGEN-NEXT:  0x0000  function idx 0, starts at line 13 col 1
// BCGEN-NEXT:    bc 34: line 13 col 1
// BCGEN-NEXT:    bc 45: line 13 col 1
// BCGEN-NEXT:    bc 56: line 13 col 1
// BCGEN-NEXT:    bc 67: line 13 col 1
// BCGEN-NEXT:    bc 78: line 13 col 1
// BCGEN-NEXT:  0x0013  function idx 1, starts at line 13 col 1
// BCGEN-NEXT:    bc 5: line 13 col 21
// BCGEN-NEXT:  0x001a  function idx 2, starts at line 15 col 1
// BCGEN-NEXT:    bc 8: line 15 col 21
// BCGEN-NEXT:  0x0021  function idx 3, starts at line 17 col 1
// BCGEN-NEXT:    bc 11: line 17 col 21
// BCGEN-NEXT:  0x0028  function idx 4, starts at line 19 col 1
// BCGEN-NEXT:    bc 14: line 19 col 21
// BCGEN-NEXT:  0x002f  function idx 5, starts at line 23 col 1
// BCGEN-NEXT:    bc 19: line 23 col 21
// BCGEN-NEXT:  0x0036  end of debug source table

// BCGEN:Debug lexical table:
// BCGEN-NEXT:  0x0000  lexical parent: none, variable count: 0
// BCGEN-NEXT:  0x0002  end of debug lexical table
