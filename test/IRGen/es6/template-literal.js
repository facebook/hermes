/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKIR %s

function f1() {
  return `hello${1 + 1}world`;
}

function f2() {
  return `world`;
}

function f3() {
  return ``;
}

function f4() {
  return `${666}`;
}

function f5(x) {
  return `${x}`;
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "f1": string
// CHKIR-NEXT:       DeclareGlobalVarInst "f2": string
// CHKIR-NEXT:       DeclareGlobalVarInst "f3": string
// CHKIR-NEXT:       DeclareGlobalVarInst "f4": string
// CHKIR-NEXT:       DeclareGlobalVarInst "f5": string
// CHKIR-NEXT:  %5 = CreateFunctionInst (:object) %f1(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f1": string
// CHKIR-NEXT:  %7 = CreateFunctionInst (:object) %f2(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "f2": string
// CHKIR-NEXT:  %9 = CreateFunctionInst (:object) %f3(): functionCode
// CHKIR-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f3": string
// CHKIR-NEXT:  %11 = CreateFunctionInst (:object) %f4(): functionCode
// CHKIR-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f4": string
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %f5(): functionCode
// CHKIR-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "f5": string
// CHKIR-NEXT:        ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function f1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "HermesInternal": string
// CHKIR-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "concat": string
// CHKIR-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, "hello": string, 2: number, "world": string
// CHKIR-NEXT:       ReturnInst %2: any
// CHKIR-NEXT:function_end

// CHKIR:function f2(): string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       ReturnInst "world": string
// CHKIR-NEXT:function_end

// CHKIR:function f3(): string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       ReturnInst "": string
// CHKIR-NEXT:function_end

// CHKIR:function f4(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "HermesInternal": string
// CHKIR-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "concat": string
// CHKIR-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, "": string, 666: number
// CHKIR-NEXT:       ReturnInst %2: any
// CHKIR-NEXT:function_end

// CHKIR:function f5(x: any): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHKIR-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "HermesInternal": string
// CHKIR-NEXT:  %2 = LoadPropertyInst (:any) %1: any, "concat": string
// CHKIR-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, "": string, %0: any
// CHKIR-NEXT:       ReturnInst %3: any
// CHKIR-NEXT:function_end
