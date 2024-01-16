/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --check-prefix=CHKIR  --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheckOrRegen --check-prefix=CHKBC  --match-full-lines %s

// Strick equality check on int32 and int32
function test_int_int(x, y) {
  x = x | 0;
  y = y | 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on int32 and uint32
function test_int_uint(x, y) {
  x = x | 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on uint32 and uint32
function test_uint_uint(x, y) {
  x = x >>> 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on values that could be int
function test_could_be_int(func) {
  var x = func() * 100;
  var a = func() ? (x | 0) : undefined;
  var b = x >>> 0;
  if (a === b) {
    return x;
  } else {
    return undefined;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "test_int_int": string
// CHKIR-NEXT:       DeclareGlobalVarInst "test_int_uint": string
// CHKIR-NEXT:       DeclareGlobalVarInst "test_uint_uint": string
// CHKIR-NEXT:       DeclareGlobalVarInst "test_could_be_int": string
// CHKIR-NEXT:  %4 = CreateFunctionInst (:object) %test_int_int(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "test_int_int": string
// CHKIR-NEXT:  %6 = CreateFunctionInst (:object) %test_int_uint(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "test_int_uint": string
// CHKIR-NEXT:  %8 = CreateFunctionInst (:object) %test_uint_uint(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_uint_uint": string
// CHKIR-NEXT:  %10 = CreateFunctionInst (:object) %test_could_be_int(): functionCode
// CHKIR-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "test_could_be_int": string
// CHKIR-NEXT:        ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function test_int_int(x: any, y: any): undefined|number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHKIR-NEXT:  %2 = AsInt32Inst (:number) %0: any
// CHKIR-NEXT:  %3 = AsInt32Inst (:number) %1: any
// CHKIR-NEXT:  %4 = FEqualInst (:boolean) %2: number, %3: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:       ReturnInst %2: number
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function test_int_uint(x: any, y: any): undefined|number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHKIR-NEXT:  %2 = AsInt32Inst (:number) %0: any
// CHKIR-NEXT:  %3 = BinaryUnsignedRightShiftInst (:number) %1: any, 0: number
// CHKIR-NEXT:  %4 = FEqualInst (:boolean) %2: number, %3: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:       ReturnInst %2: number
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function test_uint_uint(x: any, y: any): undefined|number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHKIR-NEXT:  %2 = BinaryUnsignedRightShiftInst (:number) %0: any, 0: number
// CHKIR-NEXT:  %3 = BinaryUnsignedRightShiftInst (:number) %1: any, 0: number
// CHKIR-NEXT:  %4 = FEqualInst (:boolean) %2: number, %3: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:       ReturnInst %2: number
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function test_could_be_int(func: any): undefined|number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHKIR-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHKIR-NEXT:  %2 = BinaryMultiplyInst (:number) %1: any, 100: number
// CHKIR-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHKIR-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %5 = AsInt32Inst (:number) %2: number
// CHKIR-NEXT:       BranchInst %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = PhiInst (:undefined|number) %5: number, %BB1, undefined: undefined, %BB0
// CHKIR-NEXT:  %8 = BinaryUnsignedRightShiftInst (:number) %2: number, 0: number
// CHKIR-NEXT:  %9 = BinaryStrictlyEqualInst (:boolean) %7: undefined|number, %8: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB3, %BB4
// CHKIR-NEXT:%BB3:
// CHKIR-NEXT:        ReturnInst %2: number
// CHKIR-NEXT:%BB4:
// CHKIR-NEXT:        ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKBC:Bytecode File Information:
// CHKBC-NEXT:  Bytecode version number: {{.*}}
// CHKBC-NEXT:  Source hash: {{.*}}
// CHKBC-NEXT:  Function count: 5
// CHKBC-NEXT:  String count: 5
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
// CHKBC-NEXT:i1[ASCII, 6..19] #56AD2899: test_uint_uint
// CHKBC-NEXT:i2[ASCII, 19..31] #738B3606: test_int_uint
// CHKBC-NEXT:i3[ASCII, 31..42] #B424DF91: test_int_int
// CHKBC-NEXT:i4[ASCII, 42..58] #7CF5E44C: test_could_be_int

// CHKBC:Function<global>(1 params, 3 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHKBC-NEXT:    CreateEnvironment r0
// CHKBC-NEXT:    DeclareGlobalVar  "test_int_int"
// CHKBC-NEXT:    DeclareGlobalVar  "test_int_uint"
// CHKBC-NEXT:    DeclareGlobalVar  "test_uint_uint"
// CHKBC-NEXT:    DeclareGlobalVar  "test_could_be_int"
// CHKBC-NEXT:    CreateClosure     r2, r0, Function<test_int_int>
// CHKBC-NEXT:    GetGlobalObject   r1
// CHKBC-NEXT:    PutByIdLoose      r1, r2, 1, "test_int_int"
// CHKBC-NEXT:    CreateClosure     r2, r0, Function<test_int_uint>
// CHKBC-NEXT:    PutByIdLoose      r1, r2, 2, "test_int_uint"
// CHKBC-NEXT:    CreateClosure     r2, r0, Function<test_uint_uint>
// CHKBC-NEXT:    PutByIdLoose      r1, r2, 3, "test_uint_uint"
// CHKBC-NEXT:    CreateClosure     r0, r0, Function<test_could_be_int>
// CHKBC-NEXT:    PutByIdLoose      r1, r0, 4, "test_could_be_int"
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<test_int_int>(3 params, 2 registers, 0 symbols):
// CHKBC-NEXT:    LoadParam         r0, 1
// CHKBC-NEXT:    ToInt32           r0, r0
// CHKBC-NEXT:    LoadParam         r1, 2
// CHKBC-NEXT:    ToInt32           r1, r1
// CHKBC-NEXT:    StrictEq          r1, r0, r1
// CHKBC-NEXT:    JmpTrue           L1, r1
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    Ret               r1
// CHKBC-NEXT:L1:
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<test_int_uint>(3 params, 3 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x001c, lexical 0x0000
// CHKBC-NEXT:    LoadParam         r0, 1
// CHKBC-NEXT:    ToInt32           r0, r0
// CHKBC-NEXT:    LoadParam         r2, 2
// CHKBC-NEXT:    LoadConstZero     r1
// CHKBC-NEXT:    URshift           r1, r2, r1
// CHKBC-NEXT:    StrictEq          r1, r0, r1
// CHKBC-NEXT:    JmpTrue           L1, r1
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    Ret               r1
// CHKBC-NEXT:L1:
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<test_uint_uint>(3 params, 3 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0023, lexical 0x0000
// CHKBC-NEXT:    LoadParam         r0, 1
// CHKBC-NEXT:    LoadConstZero     r2
// CHKBC-NEXT:    URshift           r0, r0, r2
// CHKBC-NEXT:    LoadParam         r1, 2
// CHKBC-NEXT:    URshift           r1, r1, r2
// CHKBC-NEXT:    StrictEq          r1, r0, r1
// CHKBC-NEXT:    JmpTrue           L1, r1
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    Ret               r1
// CHKBC-NEXT:L1:
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<test_could_be_int>(2 params, 12 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x002d, lexical 0x0000
// CHKBC-NEXT:    LoadParam         r2, 1
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    Call1             r3, r2, r1
// CHKBC-NEXT:    LoadConstUInt8    r0, 100
// CHKBC-NEXT:    Mul               r0, r3, r0
// CHKBC-NEXT:    Call1             r2, r2, r1
// CHKBC-NEXT:    LoadConstUndefined r3
// CHKBC-NEXT:    JmpFalse          L1, r2
// CHKBC-NEXT:    ToInt32           r3, r0
// CHKBC-NEXT:L1:
// CHKBC-NEXT:    LoadConstZero     r2
// CHKBC-NEXT:    URshift           r2, r0, r2
// CHKBC-NEXT:    JStrictEqual      L2, r3, r2
// CHKBC-NEXT:    Ret               r1
// CHKBC-NEXT:L2:
// CHKBC-NEXT:    Ret               r0

// CHKBC:Debug filename table:
// CHKBC-NEXT:  0: {{.*}}int-strict-equality.js

// CHKBC:Debug file table:
// CHKBC-NEXT:  source table offset 0x0000: filename id 0

// CHKBC:Debug source table:
// CHKBC-NEXT:  0x0000  function idx 0, starts at line 12 col 1
// CHKBC-NEXT:    bc 2: line 12 col 1
// CHKBC-NEXT:    bc 7: line 12 col 1
// CHKBC-NEXT:    bc 12: line 12 col 1
// CHKBC-NEXT:    bc 17: line 12 col 1
// CHKBC-NEXT:    bc 29: line 12 col 1
// CHKBC-NEXT:    bc 40: line 12 col 1
// CHKBC-NEXT:    bc 51: line 12 col 1
// CHKBC-NEXT:    bc 62: line 12 col 1
// CHKBC-NEXT:  0x001c  function idx 2, starts at line 23 col 1
// CHKBC-NEXT:    bc 11: line 25 col 7
// CHKBC-NEXT:  0x0023  function idx 3, starts at line 34 col 1
// CHKBC-NEXT:    bc 5: line 35 col 7
// CHKBC-NEXT:    bc 12: line 36 col 7
// CHKBC-NEXT:  0x002d  function idx 4, starts at line 45 col 1
// CHKBC-NEXT:    bc 5: line 46 col 15
// CHKBC-NEXT:    bc 12: line 46 col 11
// CHKBC-NEXT:    bc 16: line 47 col 15
// CHKBC-NEXT:  0x003a  end of debug source table

// CHKBC:Debug lexical table:
// CHKBC-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHKBC-NEXT:  0x0002  end of debug lexical table
