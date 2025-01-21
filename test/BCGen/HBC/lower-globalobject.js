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

// CHKRA:scope %VS0 []

// CHKRA:function global(): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:                 DeclareGlobalVarInst "x": string
// CHKRA-NEXT:                 DeclareGlobalVarInst "foo": string
// CHKRA-NEXT:  {r0}      %2 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  {r1}      %3 = CreateFunctionInst (:object) {r0} %2: environment, %foo(): functionCode
// CHKRA-NEXT:  {r0}      %4 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:                 StorePropertyLooseInst {r1} %3: object, {r0} %4: object, "foo": string
// CHKRA-NEXT:  {np0}     %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:                 ReturnInst {np0} %6: undefined
// CHKRA-NEXT:function_end

// CHKRA:function foo(): any
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  {r0}      %0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  {r0}      %1 = LoadPropertyInst (:any) {r0} %0: object, "x": string
// CHKRA-NEXT:                 ReturnInst {r0} %1: any
// CHKRA-NEXT:function_end

// CHKBC:Bytecode File Information:
// CHKBC-NEXT:  Bytecode version number: {{.*}}
// CHKBC-NEXT:  Source hash: {{.*}}
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

// CHKBC:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKBC-NEXT:    DeclareGlobalVar  "x"
// CHKBC-NEXT:    DeclareGlobalVar  "foo"
// CHKBC-NEXT:    CreateTopLevelEnvironment r1, 0
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<foo>
// CHKBC-NEXT:    GetGlobalObject   r1
// CHKBC-NEXT:    PutByIdLoose      r1, r2, 1, "foo"
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x000d, lexical 0x0000
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    GetByIdShort      r0, r0, 1, "x"
// CHKBC-NEXT:    Ret               r0

// CHKBC:Debug filename table:
// CHKBC-NEXT:  0: {{.*}}lower-globalobject.js

// CHKBC:Debug file table:
// CHKBC-NEXT:  source table offset 0x0000: filename id 0

// CHKBC:Debug source table:
// CHKBC-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// CHKBC-NEXT:    bc 0: line 11 col 1
// CHKBC-NEXT:    bc 5: line 11 col 1
// CHKBC-NEXT:    bc 23: line 11 col 1
// CHKBC-NEXT:  0x000d  function idx 1, starts at line 13 col 1
// CHKBC-NEXT:    bc 2: line 14 col 10
// CHKBC-NEXT:  0x0014  end of debug source table
