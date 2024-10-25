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

// LRA:scope %VS0 []

// LRA:function global(): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// LRA-NEXT:                 DeclareGlobalVarInst "foo1": string
// LRA-NEXT:                 DeclareGlobalVarInst "foo2": string
// LRA-NEXT:                 DeclareGlobalVarInst "foo3": string
// LRA-NEXT:                 DeclareGlobalVarInst "foo4": string
// LRA-NEXT:                 DeclareGlobalVarInst "foo5": string
// LRA-NEXT:  {r1}      %6 = CreateFunctionInst (:object) {r0} %0: environment, %foo1(): functionCode
// LRA-NEXT:  {r2}      %7 = HBCGetGlobalObjectInst (:object)
// LRA-NEXT:                 StorePropertyLooseInst {r1} %6: object, {r2} %7: object, "foo1": string
// LRA-NEXT:  {r1}      %9 = CreateFunctionInst (:object) {r0} %0: environment, %foo2(): functionCode
// LRA-NEXT:                 StorePropertyLooseInst {r1} %9: object, {r2} %7: object, "foo2": string
// LRA-NEXT:  {r1}     %11 = CreateFunctionInst (:object) {r0} %0: environment, %foo3(): functionCode
// LRA-NEXT:                 StorePropertyLooseInst {r1} %11: object, {r2} %7: object, "foo3": string
// LRA-NEXT:  {r1}     %13 = CreateFunctionInst (:object) {r0} %0: environment, %foo4(): functionCode
// LRA-NEXT:                 StorePropertyLooseInst {r1} %13: object, {r2} %7: object, "foo4": string
// LRA-NEXT:  {r0}     %15 = CreateFunctionInst (:object) {r0} %0: environment, %foo5(): functionCode
// LRA-NEXT:                 StorePropertyLooseInst {r0} %15: object, {r2} %7: object, "foo5": string
// LRA-NEXT:  {np0}    %17 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:                 ReturnInst {np0} %17: undefined
// LRA-NEXT:function_end

// LRA:function foo1(f: any): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// LRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {r1}      %2 = ImplicitMovInst (:undefined) {np0} %1: undefined
// LRA-NEXT:  {r0}      %3 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined
// LRA-NEXT:                 ReturnInst {np0} %1: undefined
// LRA-NEXT:function_end

// LRA:function foo2(f: any): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// LRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 1: number
// LRA-NEXT:  {r2}      %3 = ImplicitMovInst (:undefined) {np0} %1: undefined
// LRA-NEXT:  {r1}      %4 = ImplicitMovInst (:number) {n0} %2: number
// LRA-NEXT:  {r0}      %5 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined, {n0} %2: number
// LRA-NEXT:                 ReturnInst {np0} %1: undefined
// LRA-NEXT:function_end

// LRA:function foo3(f: any): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// LRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 1: number
// LRA-NEXT:  {n1}      %3 = HBCLoadConstInst (:number) 2: number
// LRA-NEXT:  {r3}      %4 = ImplicitMovInst (:undefined) {np0} %1: undefined
// LRA-NEXT:  {r2}      %5 = ImplicitMovInst (:number) {n0} %2: number
// LRA-NEXT:  {r1}      %6 = ImplicitMovInst (:number) {n1} %3: number
// LRA-NEXT:  {r0}      %7 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined, {n0} %2: number, {n1} %3: number
// LRA-NEXT:                 ReturnInst {np0} %1: undefined
// LRA-NEXT:function_end

// LRA:function foo4(f: any): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// LRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 1: number
// LRA-NEXT:  {n1}      %3 = HBCLoadConstInst (:number) 2: number
// LRA-NEXT:  {n2}      %4 = HBCLoadConstInst (:number) 3: number
// LRA-NEXT:  {r4}      %5 = ImplicitMovInst (:undefined) {np0} %1: undefined
// LRA-NEXT:  {r3}      %6 = ImplicitMovInst (:number) {n0} %2: number
// LRA-NEXT:  {r2}      %7 = ImplicitMovInst (:number) {n1} %3: number
// LRA-NEXT:  {r1}      %8 = ImplicitMovInst (:number) {n2} %4: number
// LRA-NEXT:  {r0}      %9 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined, {n0} %2: number, {n1} %3: number, {n2} %4: number
// LRA-NEXT:                 ReturnInst {np0} %1: undefined
// LRA-NEXT:function_end

// LRA:function foo5(f: any): undefined
// LRA-NEXT:%BB0:
// LRA-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// LRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {r4}      %2 = HBCLoadConstInst (:number) 1: number
// LRA-NEXT:  {r3}      %3 = HBCLoadConstInst (:number) 2: number
// LRA-NEXT:  {r2}      %4 = HBCLoadConstInst (:number) 3: number
// LRA-NEXT:  {r1}      %5 = HBCLoadConstInst (:number) 4: number
// LRA-NEXT:  {r5}      %6 = HBCLoadConstInst (:undefined) undefined: undefined
// LRA-NEXT:  {r0}      %7 = CallInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {r5} %6: undefined, {r4} %2: number, {r3} %3: number, {r2} %4: number, {r1} %5: number
// LRA-NEXT:                 ReturnInst {np0} %1: undefined
// LRA-NEXT:function_end

// BCGEN:Bytecode File Information:
// BCGEN-NEXT:  Bytecode version number: {{.*}}
// BCGEN-NEXT:  Source hash: {{.*}}
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

// BCGEN:Function<global>(1 params, 4 registers, 0 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// BCGEN-NEXT:    CreateTopLevelEnvironment r1, 0
// BCGEN-NEXT:    DeclareGlobalVar  "foo1"
// BCGEN-NEXT:    DeclareGlobalVar  "foo2"
// BCGEN-NEXT:    DeclareGlobalVar  "foo3"
// BCGEN-NEXT:    DeclareGlobalVar  "foo4"
// BCGEN-NEXT:    DeclareGlobalVar  "foo5"
// BCGEN-NEXT:    CreateClosure     r2, r1, Function<foo1>
// BCGEN-NEXT:    GetGlobalObject   r3
// BCGEN-NEXT:    PutByIdLoose      r3, r2, 1, "foo1"
// BCGEN-NEXT:    CreateClosure     r2, r1, Function<foo2>
// BCGEN-NEXT:    PutByIdLoose      r3, r2, 2, "foo2"
// BCGEN-NEXT:    CreateClosure     r2, r1, Function<foo3>
// BCGEN-NEXT:    PutByIdLoose      r3, r2, 3, "foo3"
// BCGEN-NEXT:    CreateClosure     r2, r1, Function<foo4>
// BCGEN-NEXT:    PutByIdLoose      r3, r2, 4, "foo4"
// BCGEN-NEXT:    CreateClosure     r1, r1, Function<foo5>
// BCGEN-NEXT:    PutByIdLoose      r3, r1, 5, "foo5"
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo1>(2 params, 10 registers, 0 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0022, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r1, 1
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    Call1             r1, r1, r0
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<foo2>(2 params, 12 registers, 1 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0029, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r2, 1
// BCGEN-NEXT:    LoadConstUndefined r1
// BCGEN-NEXT:    LoadConstUInt8    r0, 1
// BCGEN-NEXT:    Call2             r2, r2, r1, r0
// BCGEN-NEXT:    Ret               r1

// BCGEN:Function<foo3>(2 params, 14 registers, 2 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0030, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r3, 1
// BCGEN-NEXT:    LoadConstUndefined r2
// BCGEN-NEXT:    LoadConstUInt8    r0, 1
// BCGEN-NEXT:    LoadConstUInt8    r1, 2
// BCGEN-NEXT:    Call3             r3, r3, r2, r0, r1
// BCGEN-NEXT:    Ret               r2

// BCGEN:Function<foo4>(2 params, 16 registers, 3 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0037, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r4, 1
// BCGEN-NEXT:    LoadConstUndefined r3
// BCGEN-NEXT:    LoadConstUInt8    r0, 1
// BCGEN-NEXT:    LoadConstUInt8    r1, 2
// BCGEN-NEXT:    LoadConstUInt8    r2, 3
// BCGEN-NEXT:    Call4             r4, r4, r3, r0, r1, r2
// BCGEN-NEXT:    Ret               r3

// BCGEN:Function<foo5>(2 params, 18 registers, 4 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x003e, lexical 0x0000
// BCGEN-NEXT:    LoadParam         r5, 1
// BCGEN-NEXT:    LoadConstUndefined r4
// BCGEN-NEXT:    LoadConstUInt8    r9, 1
// BCGEN-NEXT:    LoadConstUInt8    r8, 2
// BCGEN-NEXT:    LoadConstUInt8    r7, 3
// BCGEN-NEXT:    LoadConstUInt8    r6, 4
// BCGEN-NEXT:    LoadConstUndefined r10
// BCGEN-NEXT:    Call              r5, r5, 5
// BCGEN-NEXT:    Ret               r4

// BCGEN:Debug filename table:
// BCGEN-NEXT:  0: {{.*}}calln.js

// BCGEN:Debug file table:
// BCGEN-NEXT:  source table offset 0x0000: filename id 0

// BCGEN:Debug source table:
// BCGEN-NEXT:  0x0000  function idx 0, starts at line 13 col 1
// BCGEN-NEXT:    bc 6: line 13 col 1
// BCGEN-NEXT:    bc 11: line 13 col 1
// BCGEN-NEXT:    bc 16: line 13 col 1
// BCGEN-NEXT:    bc 21: line 13 col 1
// BCGEN-NEXT:    bc 26: line 13 col 1
// BCGEN-NEXT:    bc 38: line 13 col 1
// BCGEN-NEXT:    bc 49: line 13 col 1
// BCGEN-NEXT:    bc 60: line 13 col 1
// BCGEN-NEXT:    bc 71: line 13 col 1
// BCGEN-NEXT:    bc 82: line 13 col 1
// BCGEN-NEXT:  0x0022  function idx 1, starts at line 13 col 1
// BCGEN-NEXT:    bc 5: line 13 col 21
// BCGEN-NEXT:  0x0029  function idx 2, starts at line 15 col 1
// BCGEN-NEXT:    bc 8: line 15 col 21
// BCGEN-NEXT:  0x0030  function idx 3, starts at line 17 col 1
// BCGEN-NEXT:    bc 11: line 17 col 21
// BCGEN-NEXT:  0x0037  function idx 4, starts at line 19 col 1
// BCGEN-NEXT:    bc 14: line 19 col 21
// BCGEN-NEXT:  0x003e  function idx 5, starts at line 23 col 1
// BCGEN-NEXT:    bc 19: line 23 col 21
// BCGEN-NEXT:  0x0045  end of debug source table
