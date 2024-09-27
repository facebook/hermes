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

// CHKRA:scope %VS0 []

// CHKRA:function global(): any
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg2 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  $Reg3 = DeclareGlobalVarInst "foo": string
// CHKRA-NEXT:  $Reg3 = DeclareGlobalVarInst "shadows": string
// CHKRA-NEXT:  $Reg3 = DeclareGlobalVarInst "checkNonStaticBuiltin": string
// CHKRA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg2, %foo(): functionCode
// CHKRA-NEXT:  $Reg3 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg3, "foo": string
// CHKRA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg2, %shadows(): functionCode
// CHKRA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg3, "shadows": string
// CHKRA-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg2, %checkNonStaticBuiltin(): functionCode
// CHKRA-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg3, "checkNonStaticBuiltin": string
// CHKRA-NEXT:  $Reg1 = TryLoadGlobalPropertyInst (:any) $Reg3, "print": string
// CHKRA-NEXT:  $Reg0 = LoadPropertyInst (:any) $Reg3, "foo": string
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg3 = HBCAllocObjectFromBufferInst (:object) "a": string, 10: number, "b": string, 20: number, "lastKey": string, 30: number, 5: number, 6: number
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg2
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:object) $Reg3
// CHKRA-NEXT:  $Reg3 = HBCCallNInst (:any) $Reg0, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg3
// CHKRA-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg2
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:any) $Reg3
// CHKRA-NEXT:  $Reg3 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg3
// CHKRA-NEXT:  $Reg3 = ReturnInst $Reg3
// CHKRA-NEXT:function_end

// CHKRA:function foo(x: any): any
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHKRA-NEXT:  $Reg2 = ImplicitMovInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = CallBuiltinInst (:any) [Object.keys]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, $Reg1
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function shadows(): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) "keys": string, null: null
// CHKRA-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg2 = TryLoadGlobalPropertyInst (:any) $Reg2, "print": string
// CHKRA-NEXT:  $Reg2 = PrStoreInst $Reg2, $Reg0, 0: number, "keys": string, false: boolean
// CHKRA-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg0, "keys": string
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:string) "evil": string
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:object) $Reg0
// CHKRA-NEXT:  $Reg3 = ImplicitMovInst (:string) $Reg2
// CHKRA-NEXT:  $Reg2 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg0, $Reg2
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg2 = ReturnInst $Reg2
// CHKRA-NEXT:function_end

// CHKRA:function checkNonStaticBuiltin(): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = TryLoadGlobalPropertyInst (:any) $Reg2, "HermesInternal": string
// CHKRA-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg0, "concat": string
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:string) "hello": string
// CHKRA-NEXT:  $Reg4 = ImplicitMovInst (:any) $Reg0
// CHKRA-NEXT:  $Reg3 = ImplicitMovInst (:string) $Reg2
// CHKRA-NEXT:  $Reg2 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg0, $Reg2
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg2 = ReturnInst $Reg2
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
// CHKBC-NEXT:Object Shape Table:
// CHKBC-NEXT:0[0, 4]
// CHKBC-NEXT:1[9, 1]
// CHKBC-NEXT:Function<global>(1 params, 13 registers, 0 numbers, 0 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKBC-NEXT:    CreateTopLevelEnvironment r2, 0
// CHKBC-NEXT:    DeclareGlobalVar  "foo"
// CHKBC-NEXT:    DeclareGlobalVar  "shadows"
// CHKBC-NEXT:    DeclareGlobalVar  "checkNonStaticBui"...
// CHKBC-NEXT:    CreateClosure     r1, r2, Function<foo>
// CHKBC-NEXT:    GetGlobalObject   r3
// CHKBC-NEXT:    PutByIdLoose      r3, r1, 1, "foo"
// CHKBC-NEXT:    CreateClosure     r1, r2, Function<shadows>
// CHKBC-NEXT:    PutByIdLoose      r3, r1, 2, "shadows"
// CHKBC-NEXT:    CreateClosure     r2, r2, Function<checkNonStaticBuiltin>
// CHKBC-NEXT:    PutByIdLoose      r3, r2, 3, "checkNonStaticBui"...
// CHKBC-NEXT:    TryGetById        r1, r3, 1, "print"
// CHKBC-NEXT:    GetByIdShort      r0, r3, 2, "foo"
// CHKBC-NEXT:    LoadConstUndefined r2
// CHKBC-NEXT:    NewObjectWithBuffer r3, 0, 0
// CHKBC-NEXT:    Call2             r3, r0, r2, r3
// CHKBC-NEXT:    Call2             r3, r1, r2, r3
// CHKBC-NEXT:    Ret               r3

// CHKBC:Function<foo>(2 params, 10 registers, 0 numbers, 0 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x0022, lexical 0x0000
// CHKBC-NEXT:    LoadParam         r1, 1
// CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<shadows>(1 params, 12 registers, 0 numbers, 0 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x0029, lexical 0x0000
// CHKBC-NEXT:    NewObjectWithBuffer r0, 1, 17
// CHKBC-NEXT:    GetGlobalObject   r2
// CHKBC-NEXT:    TryGetById        r2, r2, 1, "print"
// CHKBC-NEXT:    PutOwnBySlotIdx   r0, r2, 0
// CHKBC-NEXT:    GetByIdShort      r1, r0, 2, "keys"
// CHKBC-NEXT:    LoadConstString   r2, "evil"
// CHKBC-NEXT:    Call2             r2, r1, r0, r2
// CHKBC-NEXT:    LoadConstUndefined r2
// CHKBC-NEXT:    Ret               r2

// CHKBC:Function<checkNonStaticBuiltin>(1 params, 12 registers, 0 numbers, 0 non-pointers):
// CHKBC-NEXT:Offset in debug table: source 0x0036, lexical 0x0000
// CHKBC-NEXT:    GetGlobalObject   r2
// CHKBC-NEXT:    TryGetById        r0, r2, 1, "HermesInternal"
// CHKBC-NEXT:    GetByIdShort      r1, r0, 2, "concat"
// CHKBC-NEXT:    LoadConstString   r2, "hello"
// CHKBC-NEXT:    Call2             r2, r1, r0, r2
// CHKBC-NEXT:    LoadConstUndefined r2
// CHKBC-NEXT:    Ret               r2

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
// CHKBC-NEXT:    bc 75: line 26 col 10
// CHKBC-NEXT:    bc 80: line 26 col 6
// CHKBC-NEXT:  0x0022  function idx 1, starts at line 12 col 1
// CHKBC-NEXT:    bc 3: line 13 col 23
// CHKBC-NEXT:  0x0029  function idx 2, starts at line 17 col 1
// CHKBC-NEXT:    bc 8: line 18 col 25
// CHKBC-NEXT:    bc 18: line 19 col 16
// CHKBC-NEXT:    bc 27: line 19 col 16
// CHKBC-NEXT:  0x0036  function idx 3, starts at line 22 col 1
// CHKBC-NEXT:    bc 2: line 23 col 3
// CHKBC-NEXT:    bc 8: line 23 col 24
// CHKBC-NEXT:    bc 17: line 23 col 24
// CHKBC-NEXT:  0x0043  end of debug source table
