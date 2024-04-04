/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-lra %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-bytecode %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKBC %s
// RUN: %hermes -O -fstatic-builtins -target=HBC %s | %FileCheck --match-full-lines %s

function foo(x) {
    return Object.keys(x)
}

// Make sure that this isn't incorrectly recognized as a builtin.
function shadows() {
    var Object = {keys: print};
    Object.keys("evil");
}

function checkNonStaticBuiltin() {
  HermesInternal.concat('hello');
}

print(foo({a: 10, b: 20, lastKey:30, 5:6}))
//CHECK: 5,a,b,lastKey

// Auto-generated content below. Please do not modify manually.

// CHKRA:function global(): any
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg1 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "shadows": string
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "checkNonStaticBuiltin": string
// CHKRA-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg1, %foo(): functionCode
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg0, "foo": string
// CHKRA-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg1, %shadows(): functionCode
// CHKRA-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg0, "shadows": string
// CHKRA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg1, %checkNonStaticBuiltin(): functionCode
// CHKRA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg0, "checkNonStaticBuiltin": string
// CHKRA-NEXT:  $Reg2 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// CHKRA-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg0, "foo": string
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) 4: number, "a": string, 10: number, "b": string, 20: number, "lastKey": string, 30: number, 5: number, 6: number
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg1
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:object) $Reg0
// CHKRA-NEXT:  $Reg0 = HBCCallNInst (:any) $Reg3, empty: any, empty: any, $Reg1, $Reg1, $Reg0
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg1
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:any) $Reg0
// CHKRA-NEXT:  $Reg0 = HBCCallNInst (:any) $Reg2, empty: any, empty: any, $Reg1, $Reg1, $Reg0
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function foo(x: any): any
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHKRA-NEXT:  $Reg2 = ImplicitMovInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = CallBuiltinInst (:any) [Object.keys]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, $Reg1
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function shadows(): undefined
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg3 = HBCAllocObjectFromBufferInst (:object) 1: number, "keys": string, null: null
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// CHKRA-NEXT:  $Reg0 = PrStoreInst $Reg0, $Reg3, 0: number, "keys": string, false: boolean
// CHKRA-NEXT:  $Reg2 = LoadPropertyInst (:any) $Reg3, "keys": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:string) "evil": string
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:object) $Reg3
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:string) $Reg1
// CHKRA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg2, empty: any, empty: any, $Reg0, $Reg3, $Reg1
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function checkNonStaticBuiltin(): undefined
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg3 = TryLoadGlobalPropertyInst (:any) $Reg0, "HermesInternal": string
// CHKRA-NEXT:  $Reg2 = LoadPropertyInst (:any) $Reg3, "concat": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:string) "hello": string
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:any) $Reg3
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:string) $Reg1
// CHKRA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg2, empty: any, empty: any, $Reg0, $Reg3, $Reg1
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKBC:Bytecode File Information:
// CHKBC-NEXT:  Bytecode version number: {{.*}}
// CHKBC-NEXT:  Source hash: {{.*}}
// CHKBC-NEXT:  Function count: 4
// CHKBC-NEXT:  String count: 13
// CHKBC-NEXT:  BigInt count: 0
// CHKBC-NEXT:  String Kind Entry count: 2
// CHKBC-NEXT:  RegExp count: 0
// CHKBC-NEXT:  Segment ID: 0
// CHKBC-NEXT:  CommonJS module count: 0
// CHKBC-NEXT:  CommonJS module count (static): 0
// CHKBC-NEXT:  Function source count: 0
// CHKBC-NEXT:  Bytecode options:
// CHKBC-NEXT:    staticBuiltins: 1
// CHKBC-NEXT:    cjsModulesStaticallyResolved: 0

// CHKBC:Global String Table:
// CHKBC-NEXT:s0[ASCII, 0..3]: evil
// CHKBC-NEXT:s1[ASCII, 10..15]: global
// CHKBC-NEXT:s2[ASCII, 16..20]: hello
// CHKBC-NEXT:i3[ASCII, 3..9] #C95C4184: lastKey
// CHKBC-NEXT:i4[ASCII, 14..14] #00018270: a
// CHKBC-NEXT:i5[ASCII, 21..34] #85BBF6F9: HermesInternal
// CHKBC-NEXT:i6[ASCII, 35..35] #00018E43: b
// CHKBC-NEXT:i7[ASCII, 36..56] #B182F5CE: checkNonStaticBuiltin
// CHKBC-NEXT:i8[ASCII, 57..62] #CB8DFA65: concat
// CHKBC-NEXT:i9[ASCII, 63..65] #9290584E: foo
// CHKBC-NEXT:i10[ASCII, 66..69] #973712F5: keys
// CHKBC-NEXT:i11[ASCII, 69..75] #65FB9C2E: shadows
// CHKBC-NEXT:i12[ASCII, 76..80] #A689F65B: print

// CHKBC:Literal Value Buffer:
// CHKBC-NEXT:[int 10]
// CHKBC-NEXT:[int 20]
// CHKBC-NEXT:[int 30]
// CHKBC-NEXT:[int 6]
// CHKBC-NEXT:null
// CHKBC-NEXT:Object Key Buffer:
// CHKBC-NEXT:[String 4]
// CHKBC-NEXT:[String 6]
// CHKBC-NEXT:[String 3]
// CHKBC-NEXT:[int 5]
// CHKBC-NEXT:[String 10]
// CHKBC-NEXT:Function<global>(1 params, 13 registers):
// CHKBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKBC-NEXT:    CreateTopLevelEnvironment r1, 0
// CHKBC-NEXT:    DeclareGlobalVar  "foo"
// CHKBC-NEXT:    DeclareGlobalVar  "shadows"
// CHKBC-NEXT:    DeclareGlobalVar  "checkNonStaticBui"...
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<foo>
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    PutByIdLoose      r0, r2, 1, "foo"
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<shadows>
// CHKBC-NEXT:    PutByIdLoose      r0, r2, 2, "shadows"
// CHKBC-NEXT:    CreateClosure     r1, r1, Function<checkNonStaticBuiltin>
// CHKBC-NEXT:    PutByIdLoose      r0, r1, 3, "checkNonStaticBui"...
// CHKBC-NEXT:    TryGetById        r2, r0, 1, "print"
// CHKBC-NEXT:    GetByIdShort      r3, r0, 2, "foo"
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    NewObjectWithBuffer r0, 4, 4, 0, 0
// CHKBC-NEXT:    Call2             r0, r3, r1, r0
// CHKBC-NEXT:    Call2             r0, r2, r1, r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo>(2 params, 10 registers):
// CHKBC-NEXT:Offset in debug table: source 0x0022, lexical 0x0000
// CHKBC-NEXT:    LoadParam         r1, 1
// CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<shadows>(1 params, 13 registers):
// CHKBC-NEXT:Offset in debug table: source 0x0029, lexical 0x0000
// CHKBC-NEXT:    NewObjectWithBuffer r3, 1, 1, 9, 17
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    TryGetById        r0, r0, 1, "print"
// CHKBC-NEXT:    PutOwnBySlotIdx   r3, r0, 0
// CHKBC-NEXT:    GetByIdShort      r2, r3, 2, "keys"
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    LoadConstString   r1, "evil"
// CHKBC-NEXT:    Call2             r1, r2, r3, r1
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<checkNonStaticBuiltin>(1 params, 13 registers):
// CHKBC-NEXT:Offset in debug table: source 0x0036, lexical 0x0000
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    TryGetById        r3, r0, 1, "HermesInternal"
// CHKBC-NEXT:    GetByIdShort      r2, r3, 2, "concat"
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    LoadConstString   r1, "hello"
// CHKBC-NEXT:    Call2             r1, r2, r3, r1
// CHKBC-NEXT:    Ret               r0

// CHKBC:Debug filename table:
// CHKBC-NEXT:  0: {{.*}}callbuiltin.js

// CHKBC:Debug file table:
// CHKBC-NEXT:  source table offset 0x0000: filename id 0

// CHKBC:Debug source table:
// CHKBC-NEXT:  0x0000  function idx 0, starts at line 12 col 1
// CHKBC-NEXT:    bc 6: line 12 col 1
// CHKBC-NEXT:    bc 11: line 12 col 1
// CHKBC-NEXT:    bc 16: line 12 col 1
// CHKBC-NEXT:    bc 28: line 12 col 1
// CHKBC-NEXT:    bc 39: line 12 col 1
// CHKBC-NEXT:    bc 50: line 12 col 1
// CHKBC-NEXT:    bc 56: line 26 col 1
// CHKBC-NEXT:    bc 62: line 26 col 7
// CHKBC-NEXT:    bc 79: line 26 col 10
// CHKBC-NEXT:    bc 84: line 26 col 6
// CHKBC-NEXT:  0x0022  function idx 1, starts at line 12 col 1
// CHKBC-NEXT:    bc 3: line 13 col 23
// CHKBC-NEXT:  0x0029  function idx 2, starts at line 17 col 1
// CHKBC-NEXT:    bc 12: line 18 col 25
// CHKBC-NEXT:    bc 22: line 19 col 16
// CHKBC-NEXT:    bc 33: line 19 col 16
// CHKBC-NEXT:  0x0036  function idx 3, starts at line 22 col 1
// CHKBC-NEXT:    bc 2: line 23 col 3
// CHKBC-NEXT:    bc 8: line 23 col 24
// CHKBC-NEXT:    bc 19: line 23 col 24
// CHKBC-NEXT:  0x0043  end of debug source table

// CHKBC:Debug lexical table:
// CHKBC-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHKBC-NEXT:  0x0002  end of debug lexical table
